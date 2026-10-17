/* dht.h — 커널 dht11 드라이버(IIO) 읽기
 *
 *   config.txt: dtoverlay=dht11,gpiopin=4  → /sys/bus/iio/devices/iio:deviceN/
 *     in_temp_input               밀리 °C   (23000 = 23.0 °C)
 *     in_humidityrelative_input   밀리 %    (45000 = 45 %)
 *
 *   dht_t d;  dht_open(&d);  dht_read(&d, &t, &h);
 */
#ifndef DHT_H
#define DHT_H

typedef struct {
    char dir[96];          /* 찾은 IIO 장치 디렉터리 */
    int fails;             /* 연속 실패 횟수 */
} dht_t;

int dht_open(dht_t *d);                              /* 0 = 찾음, -1 = 없음 (overlay 확인) */
int dht_read(dht_t *d, double *temp_c, double *humi); /* 0 = 성공, -1 = 3번 재시도 후 실패 */

#endif
