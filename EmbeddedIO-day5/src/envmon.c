/* envmon.c — 스마트 환경 모니터 (Day 5 종합 프로젝트)
 *
 *   sensor    2초마다 온습도(DHT11 · IIO) + 조도(PCF8591 IN0)
 *   proximity 약 0.3초마다 거리(HC-SR04 중앙값 3회) — 30 cm 안이면 접근 경고
 *   control   버튼 A(GPIO5) 짧게 = LCD 화면 전환 · 길게(1초) = 부저 음소거
 *             온도 ≥ 28 °C → 릴레이(팬) ON, ≤ 27 °C → OFF · 접근 → 부저 · 경보 LED
 *   display   0.5초마다(또는 화면 전환 즉시) LCD 갱신, 1분마다 envmon.csv 기록
 *
 *   ./envmon               (Ctrl+C 또는 systemctl stop → 모두 끄고 반납)
 *   ./envmon -c 경로.csv
 *
 *   핀: DHT11 4 · I2C 2/3 · 버튼 5 · LCD 12/16/20/21/26/22 · 부저 13 · LED 17(상태)/27(경보)
 *       HC-SR04 23/24 · 릴레이 25
 */
#include <pthread.h>
#include <poll.h>
#include <stdlib.h>
#include <time.h>
#include "gpio_common.h"
#include "pcf8591.h"
#include "dht.h"
#include "sonar.h"
#include "lcd.h"

#define PIN_LED_OK 17
#define PIN_LED_AL 27
#define PIN_RELAY  25
#define PIN_BUZ    13
#define PIN_BTN     5
#define FAN_ON   28.0
#define FAN_OFF  27.0
#define NEAR_CM  30.0
#define NSCREEN   4

/* ── 공유 상태: 모든 쓰레드가 이 구조체 하나만 주고받는다 ── */
typedef struct {
    double t, h;   int th_ok;      /* 온습도 · 유효 */
    int light;                     /* 조도 % (-1 = 실패) */
    double cm;                     /* 거리 (-1 = 실패) */
    int screen, mute, fan, near;
    int err_dht, err_pcf, err_sonar;
} env_t;

static struct {
    pthread_mutex_t mx;
    pthread_cond_t  cv;            /* 화면을 바로 다시 그려 달라는 신호 */
    env_t d;
} S = { PTHREAD_MUTEX_INITIALIZER, PTHREAD_COND_INITIALIZER, { .light = -1, .cm = -1 } };
static const char *g_csv = "envmon.csv";
static long long g_start;

static void log_msg(const char *fmt, const char *a)
{
    time_t now = time(NULL);
    char ts[16]; strftime(ts, sizeof ts, "%H:%M:%S", localtime(&now));
    printf("[%s] ", ts); printf(fmt, a); printf("\n");          /* journalctl 에 그대로 남는다 */
}

/* ── sensor: 온습도 + 조도 ── */
static void *sensor_thread(void *arg)
{
    (void)arg;
    dht_t d; int have_dht = dht_open(&d) == 0;
    int pcf = pcf_open("/dev/i2c-1", PCF_ADDR);
    int was_dht = 1, was_pcf = 1;
    while (!g_stop) {
        double t = 0, h = 0;
        if (!have_dht) have_dht = dht_open(&d) == 0;
        int ok = have_dht && dht_read(&d, &t, &h) == 0;
        if (pcf < 0) pcf = pcf_open("/dev/i2c-1", PCF_ADDR);
        int v = pcf >= 0 ? pcf_read(pcf, 0) : -1;

        pthread_mutex_lock(&S.mx);
        if (ok) { S.d.t = t; S.d.h = h; S.d.th_ok = 1; } else { S.d.th_ok = 0; S.d.err_dht++; }
        S.d.light = v < 0 ? -1 : v * 100 / 255;
        if (v < 0) S.d.err_pcf++;
        pthread_mutex_unlock(&S.mx);

        if (ok != was_dht) log_msg("DHT11 %s", ok ? "복구" : "읽기 실패 — 선 확인");
        if ((v >= 0) != was_pcf) log_msg("PCF8591 %s", v >= 0 ? "복구" : "응답 없음 — I2C 선 확인");
        was_dht = ok; was_pcf = v >= 0;
        for (int i = 0; i < 20 && !g_stop; i++) sleep_ms(100);   /* 2초 — Ctrl+C 에 빨리 반응 */
    }
    if (pcf >= 0) close(pcf);
    return NULL;
}

/* ── proximity: 거리 ── */
static void *proximity_thread(void *arg)
{
    sonar_t *so = arg;
    int was_near = 0;
    while (!g_stop) {
        double cm = so ? sonar_median(so, 3) : -1;
        pthread_mutex_lock(&S.mx);
        S.d.cm = cm;
        if (cm < 0) S.d.err_sonar++;
        S.d.near = cm > 0 && cm < NEAR_CM;
        int near = S.d.near;
        if (near != was_near) pthread_cond_signal(&S.cv);       /* 경고 화면을 바로 */
        pthread_mutex_unlock(&S.mx);
        if (near && !was_near) log_msg("접근 감지%s", "");
        was_near = near;
        sleep_ms(100);
    }
    return NULL;
}

/* ── control: 버튼 이벤트 + 판단 + 출력 ── */
static struct gpiod_line_request *g_out;
static const unsigned int OUTS[4] = { PIN_LED_OK, PIN_LED_AL, PIN_RELAY, PIN_BUZ };

static void *control_thread(void *arg)
{
    struct gpiod_line_request *btn = arg;
    struct gpiod_edge_event_buffer *buf = gpiod_edge_event_buffer_new(8);
    struct pollfd p = { .fd = gpiod_line_request_get_fd(btn), .events = POLLIN };
    long long down_at = 0;
    while (!g_stop) {
        if (poll(&p, 1, 50) > 0) {                               /* 50 ms 마다 출력 갱신 */
            int n = gpiod_line_request_read_edge_events(btn, buf, 8);
            for (int i = 0; i < n; i++) {
                struct gpiod_edge_event *ev = gpiod_edge_event_buffer_get_event(buf, i);
                long long ts = (long long)gpiod_edge_event_get_timestamp_ns(ev);
                if (gpiod_edge_event_get_event_type(ev) == GPIOD_EDGE_EVENT_FALLING_EDGE) down_at = ts;
                else if (down_at) {
                    long long ms = (ts - down_at) / 1000000;  down_at = 0;
                    pthread_mutex_lock(&S.mx);
                    if (ms >= 1000) S.d.mute = !S.d.mute; else S.d.screen = (S.d.screen + 1) % NSCREEN;
                    pthread_cond_signal(&S.cv);
                    pthread_mutex_unlock(&S.mx);
                    log_msg(ms >= 1000 ? "버튼 길게 — 음소거 전환%s" : "버튼 — 화면 전환%s", "");
                }
            }
        }
        pthread_mutex_lock(&S.mx);
        if (S.d.th_ok && S.d.t >= FAN_ON)  S.d.fan = 1;                  /* 히스테리시스 */
        if (S.d.th_ok && S.d.t <= FAN_OFF) S.d.fan = 0;
        int fan = S.d.fan, near = S.d.near, mute = S.d.mute, err = !S.d.th_ok || S.d.light < 0 || S.d.cm < 0;
        pthread_mutex_unlock(&S.mx);

        long long ms = (now_ns() - g_start) / 1000000;
        enum gpiod_line_value v[4] = {
            (ms % (err ? 250 : 1000)) < (err ? 125 : 100),         /* 상태 LED: 정상 1초에 한 번 · 오류 빠르게 */
            near,                                                    /* 경보 LED */
            fan,                                                     /* 릴레이 = 팬 */
            near && !mute && (ms % 500) < 100,                       /* 부저 삑 */
        };
        gpiod_line_request_set_values(g_out, v);
    }
    gpiod_edge_event_buffer_free(buf);
    return NULL;
}

/* ── display: LCD + CSV ── */
static void *display_thread(void *arg)
{
    lcd_t *l = arg;
    static const uint8_t DEG[8] = { 0x06, 0x09, 0x09, 0x06, 0, 0, 0, 0 };
    lcd_custom(l, 1, DEG);
    long long last_csv = 0;
    pthread_mutex_lock(&S.mx);
    while (!g_stop) {
        env_t c = S.d;                                           /* 잠근 채 복사 → 풀고 그린다 */
        pthread_mutex_unlock(&S.mx);

        char a[32], b[32];                                       /* lcd_printf 가 16칸으로 자른다 */
        if (c.near) { snprintf(a, sizeof a, "!! APPROACH !!"); snprintf(b, sizeof b, "dist %5.1f cm", c.cm); }
        else switch (c.screen) {
        case 0:
            if (c.th_ok) snprintf(a, sizeof a, "T %4.1f\x01" "C H %2.0f%%", c.t, c.h);
            else snprintf(a, sizeof a, "T --.-  H --");
            snprintf(b, sizeof b, "Fan %-3s Mute %s", c.fan ? "ON" : "OFF", c.mute ? "Y" : "N");
            break;
        case 1:
            if (c.light < 0) snprintf(a, sizeof a, "Light  --");
            else snprintf(a, sizeof a, "Light %3d %%", c.light);
            { int n = c.light < 0 ? 0 : c.light * 16 / 100; for (int i = 0; i < 16; i++) b[i] = i < n ? (char)0xFF : ' '; b[16] = 0; }
            break;
        case 2:
            if (c.cm < 0) snprintf(a, sizeof a, "Dist  ---.- cm");
            else snprintf(a, sizeof a, "Dist %6.1f cm", c.cm);
            snprintf(b, sizeof b, "near < %.0f cm", NEAR_CM);
            break;
        default:
            snprintf(a, sizeof a, "up %lld s", (now_ns() - g_start) / 1000000000LL);
            snprintf(b, sizeof b, "E D%d P%d S%d", c.err_dht % 100, c.err_pcf % 100, c.err_sonar % 100);
        }
        lcd_printf(l, 0, "%s", a);
        lcd_printf(l, 1, "%s", b);

        long long now = now_ns();
        if (now - last_csv >= 60000000000LL) {                   /* 1분마다 */
            last_csv = now;
            FILE *f = fopen(g_csv, "a");
            if (f) {
                time_t tt = time(NULL); char ts[32];
                strftime(ts, sizeof ts, "%Y-%m-%d %H:%M:%S", localtime(&tt));
                fprintf(f, "%s,%.1f,%.0f,%d,%.1f,%d,%d\n", ts, c.th_ok ? c.t : -1, c.th_ok ? c.h : -1,
                        c.light, c.cm, c.fan, c.near);
                fclose(f);
            }
        }
        /* 0.5초 기다리되, 화면 전환 · 접근 신호가 오면 바로 깨어난다 */
        struct timespec dl; clock_gettime(CLOCK_REALTIME, &dl);
        dl.tv_nsec += 500000000L; if (dl.tv_nsec >= 1000000000L) { dl.tv_sec++; dl.tv_nsec -= 1000000000L; }
        pthread_mutex_lock(&S.mx);
        pthread_cond_timedwait(&S.cv, &S.mx, &dl);
    }
    pthread_mutex_unlock(&S.mx);
    return NULL;
}

int main(int argc, char **argv)
{
    for (int i = 1; i < argc; i++)
        if (!strcmp(argv[i], "-c") && i + 1 < argc) g_csv = argv[++i];
    setvbuf(stdout, NULL, _IOLBF, 0);          /* systemd 아래에서는 stdout 이 파이프 → 줄 단위로 내보내기 */
    gpio_install_sigint();                     /* SIGINT · SIGTERM(systemctl stop) → g_stop */
    g_start = now_ns();

    lcd_t *lcd = lcd_open();
    g_out = gpio_request_output("envmon", OUTS, 4);
    unsigned int bpin = PIN_BTN;
    struct gpiod_line_request *btn = gpio_request_input("envmon", &bpin, 1, GPIOD_LINE_EDGE_BOTH, 20000);
    if (!lcd || !g_out || !btn) {               /* 이것들이 없으면 시작하지 않는다 → systemd 가 재시작 */
        fprintf(stderr, "LCD · 출력 · 버튼 요청 실패 — overlay(eio) 를 내렸는지, 다른 프로그램이 쓰는지 확인\n");
        return EXIT_FAILURE;
    }
    sonar_t *so = sonar_open(23, 24);           /* 센서는 없어도 계속 — 화면에 -- */
    if (!so) log_msg("HC-SR04 요청 실패%s — 거리 없이 계속", "");
    lcd_printf(lcd, 0, "envmon start");
    log_msg("envmon 시작 · CSV %s", g_csv);

    pthread_t th[4];
    pthread_create(&th[0], NULL, sensor_thread, NULL);
    pthread_create(&th[1], NULL, proximity_thread, so);
    pthread_create(&th[2], NULL, control_thread, btn);
    pthread_create(&th[3], NULL, display_thread, lcd);

    while (!g_stop) sleep_ms(200);              /* 메인은 신호만 기다린다 */
    pthread_mutex_lock(&S.mx); pthread_cond_broadcast(&S.cv); pthread_mutex_unlock(&S.mx);
    for (int i = 0; i < 4; i++) pthread_join(th[i], NULL);

    enum gpiod_line_value off[4] = { 0, 0, 0, 0 };            /* 릴레이 · 부저 반드시 끄기 */
    gpiod_line_request_set_values(g_out, off);
    gpiod_line_request_release(g_out);
    gpiod_line_request_release(btn);
    sonar_close(so);
    lcd_printf(lcd, 0, "envmon stopped");
    lcd_printf(lcd, 1, "%s", "");
    lcd_release(lcd);                           /* 화면은 남기고 라인만 반납 */
    log_msg("envmon 종료%s", "");
    return EXIT_SUCCESS;
}
