# C Day 2 강의 자료 — 포인터·배열·문자열·동적 메모리

**HYUNDAI AI Insight Campus 온디바이스 AI · 프로그래밍 언어(C) 2/4 · 9/8(화)**

| 교시 | 주제 | 애니메이션 (index_day2.html) |
|---|---|---|
| 1 | 포인터 기초 — Day 1의 빚 갚기 | D2-1 포인터 화살표 뷰어 |
| 2 | 포인터 산술과 const | D2-2 p+1 은 몇 바이트 이동하는가 |
| 3 | 배열과 포인터 | D2-3 배열 감쇠(decay) |
| 4 | 정렬과 검색 | D2-4 버블 정렬 · 이진 검색 |
| 5 | 문자열 — char 배열과 널 종료 | D2-5 char[] vs char * · 버퍼 오버플로 |
| 6 | 안전한 문자열 처리와 파싱 | D2-6 strtok 분리 |
| 7 | 동적 메모리 — malloc/free | D2-7 힙 할당·해제 |
| 8 | 종합 실습 — 센서 로그 분석기 · 정리 | D2-8 로그 분석기 흐름 |

### 이 자료 사용법

- **▶ 예제**는 파일을 만들고, 적힌 명령으로 컴파일·실행한 뒤 출력을 함께 읽습니다. 주석의 `// →` 뒤가 기대 출력입니다. 주소값(`0x7ffc…`)은 실행마다 다르니 **끝자리 차이**만 보세요.
- **✏️ 빈칸 채우기**는 `____` 를 채워 컴파일합니다.
- **✏️ 괄호 넣기**는 ( ) 에 들어갈 말을 먼저 생각한 뒤 정답을 펼쳐 확인합니다.
- **🔒 정답 보기**는 접혀 있습니다. 강사가 신호를 준 뒤 펼치세요.
- **⚠ 함정** 표시는 오늘 반드시 한 번은 직접 밟아 봐야 하는 실수입니다. 오늘 함정은 대부분 **세그폴트**로 끝납니다 — 그게 정상입니다.
- 어제 만든 `Reading` 구조체와 센서 프레임 `$T=25.3,H=60,D=120*` 을 오늘 문자열로 파싱하고 힙에 쌓습니다.

---

## ⚙️ 준비 — 가장 먼저 실행

```bash
mkdir -p ~/c_day2 && cd ~/c_day2
gcc --version
valgrind --version      # 없으면: sudo apt install valgrind  (7교시 전까지)
```

오늘 컴파일 명령에는 `-g` 를 붙입니다. 세그폴트가 났을 때 gdb·valgrind 가 **몇 번째 줄**인지 알려주려면 필요합니다.

```bash
gcc -Wall -Wextra -g -o 프로그램이름 소스파일.c && ./프로그램이름
```

세그폴트가 나면 이 한 줄로 위치를 찾습니다:

```bash
gdb -batch -ex run -ex bt ./프로그램이름 2>&1 | tail -5
```

---

# 1교시 · 포인터 기초 — Day 1의 빚 갚기 (09:00–09:50)

**학습 목표**
- `&`(주소)와 `*`(역참조)를 그림으로 설명한다
- 함수가 호출자의 변수를 바꾸게 하는 방법을 안다
- 초기화 안 된 포인터가 왜 죽는지 실행으로 확인한다

🎬 **D2-1 포인터 화살표 뷰어** — 변수 칸 위의 화살표를 따라가며 진행

### ▶ 예제 1-1 · 주소는 그냥 숫자다

`addr.c`
```c
#include <stdio.h>

int main(void) {
    int x = 5;
    int *p = &x;            // p 는 "x 의 주소"를 담는다

    printf("x   = %d\n", x);            // → 5
    printf("&x  = %p\n", (void *)&x);   // → 0x7ffc...20  (실행마다 다름)
    printf("p   = %p\n", (void *)p);    // → 같은 값
    printf("*p  = %d\n", *p);           // → 5   (p 가 가리키는 곳의 값)

    *p = 42;                            // p 를 통해 x 를 바꾼다
    printf("x   = %d\n", x);            // → 42
    return 0;
}
```

```bash
gcc -Wall -Wextra -g addr.c -o addr && ./addr
```

`int *p` 는 "p 를 역참조(`*p`)하면 int 가 나온다" 로 읽습니다. `p` 자체는 8바이트 주소, `*p` 는 그 주소에 있는 int. 어제 `sizeof(int *)` 가 8 이었던 이유입니다.

### ▶ 예제 1-2 · Day 1 의 inc 가 실패한 자리에서 swap 을 성공시키기

```c
#include <stdio.h>

void inc_wrong(int x)  { x++; }          // Day 1: 복사본만 바뀜
void inc_right(int *p) { (*p)++; }       // 주소를 받아 원본을 바꿈

void swap(int *a, int *b) {
    int t = *a;
    *a = *b;
    *b = t;
}

int main(void) {
    int x = 5, y = 9;
    inc_wrong(x);  printf("%d\n", x);    // → 5
    inc_right(&x); printf("%d\n", x);    // → 6
    swap(&x, &y);  printf("%d %d\n", x, y);   // → 9 6
    return 0;
}
```

어제 `judge(ans, g, &s, &b)` 가 strike/ball 두 값을 돌려줄 수 있었던 이유가 바로 `inc_right` 입니다. **값을 돌려받고 싶은 변수의 주소를 넘긴다.**

### ▶ 예제 1-3 · ⚠ 함정 — 어디도 가리키지 않는 포인터

```c
#include <stdio.h>

int main(void) {
    int *p;                 // 초기화 안 됨: 쓰레기 주소
    *p = 10;                // → Segmentation fault (대부분)
    printf("%d\n", *p);
    return 0;
}
```

```bash
gcc -Wall -Wextra -g wild.c -o wild
# warning: 'p' is used uninitialized [-Wuninitialized]
./wild
# Segmentation fault (core dumped)
gdb -batch -ex run -ex bt ./wild 2>&1 | tail -3
# #0  main () at wild.c:5      ← 죽은 줄 번호
```

포인터는 **선언만으로는 아무 곳도 가리키지 않습니다.** 반드시 `&변수`, 배열, `malloc` 결과, 또는 `NULL` 로 초기화하세요. `NULL` 을 역참조해도 죽지만, 적어도 "가리키는 곳이 없다"를 `if (p == NULL)` 로 검사할 수 있습니다.

### ▶ 예제 1-4 · ⚠ 함정 — `*p++` 와 `(*p)++`

```c
#include <stdio.h>

int main(void) {
    int arr[3] = {3, 1, 2};
    int *p = arr;
    printf("%d ", *p++);     // → 3   : *p 를 쓴 뒤 p 를 다음 칸으로
    printf("%d ", *p);       // → 1   : p 는 이제 arr[1]
    (*p)++;                  //        arr[1] 을 1 증가
    printf("%d\n", arr[1]);  // → 2
    return 0;
}
```

후위 `++` 가 `*` 보다 우선순위가 높아 `*p++` 는 `*(p++)` 입니다. 어제 숫자 야구 정답의 `(*strike)++` 에 괄호가 있었던 이유.

#### ✏️ 빈칸 채우기 1-1

두 값 중 큰 값을 `*out` 에 써 주고, 같으면 0 을 반환합니다.

```c
#include <stdio.h>

int max_to(int a, int b, int ____ out) {
    if (a == b) return 0;
    ____ = (a > b) ? a : b;
    return 1;
}

int main(void) {
    int m = 0;
    if (max_to(25, 37, ____)) printf("max = %d\n", m);   // → max = 37
    return 0;
}
```

<details><summary>🔒 정답 보기 1-1</summary>

```c
int max_to(int a, int b, int *out) {
    if (a == b) return 0;
    *out = (a > b) ? a : b;
    return 1;
}
    if (max_to(25, 37, &m)) printf("max = %d\n", m);
```
반환값은 성공/실패, 결과는 포인터로 — C 라이브러리 함수의 흔한 모양입니다 (`scanf`, `strtol`).
</details>

#### ✏️ 괄호 넣기 1-2

1. `&x` 는 x 의 (　　　)를, `*p` 는 p 가 가리키는 곳의 (　　　)을 뜻한다.
2. `int *p;` 에서 `p` 의 크기는 64비트 시스템에서 (　　　)바이트, `*p` 의 크기는 (　　　)바이트다.
3. 함수가 호출자의 변수를 바꾸려면 값 대신 (　　　)를 넘겨야 한다.
4. 선언만 하고 초기화하지 않은 포인터를 역참조하면 보통 (　　　)가 난다.
5. `*p++` 는 (`(*p)++` / `*(p++)`) 와 같다.

<details><summary>🔒 정답 보기 1-2</summary>

1. 주소 / 값
2. 8 / 4
3. 주소
4. 세그폴트 (Segmentation fault)
5. `*(p++)`
</details>

---

# 2교시 · 포인터 산술과 const (10:00–10:50)

**학습 목표**
- `p + 1` 이 몇 바이트 이동하는지 타입별로 안다
- 포인터끼리 뺄셈이 요소 개수임을 안다
- `const int *` 와 `int *const` 를 구분한다

🎬 **D2-2 p+1 은 몇 바이트** — 타입을 바꿔 가며 화살표 이동 거리 비교

### ▶ 예제 2-1 · p + 1 은 1바이트가 아니다

```c
#include <stdio.h>

int main(void) {
    int    arr[5] = {10, 20, 30, 40, 50};
    double d[3];
    char   c[4];

    int *p = arr;
    printf("%p\n%p\n%p\n", (void *)p, (void *)(p + 1), (void *)(p + 2));
    // → ...90 / ...94 / ...98   : 4씩 증가

    printf("int    +1 = %ld bytes\n", (long)((char *)(p + 1)   - (char *)p));    // → 4
    printf("double +1 = %ld bytes\n", (long)((char *)(d + 1)   - (char *)d));    // → 8
    printf("char   +1 = %ld bytes\n", (long)((char *)(c + 1)   - (char *)c));    // → 1

    printf("%d %d\n", *(p + 2), *(arr + 2));     // → 30 30
    printf("%ld\n", (long)(&arr[4] - &arr[0]));  // → 4  (바이트가 아니라 요소 개수)
    return 0;
}
```

`p + n` 은 `n * sizeof(*p)` 바이트 이동합니다. 그래서 같은 주소를 `int *` 로 보느냐 `char *` 로 보느냐에 따라 `+1` 의 의미가 달라집니다.

### ▶ 예제 2-2 · 바이트 배열을 uint16_t 로 읽기 — Day 1 엔디안 회수

```c
#include <stdio.h>
#include <stdint.h>

int main(void) {
    // 센서가 보낸 4바이트: 거리 300(0x012C), 습도 60(0x003C), 리틀 엔디안
    uint8_t buf[4] = {0x2C, 0x01, 0x3C, 0x00};

    // 방법 1: 포인터 캐스팅 (정렬이 맞을 때만, x86/ARM 대부분 OK)
    uint16_t *w = (uint16_t *)buf;
    printf("%u %u\n", w[0], w[1]);            // → 300 60

    // 방법 2: 바이트 조립 (이식성 100%, 임베디드 권장)
    uint16_t dist = buf[0] | (buf[1] << 8);
    printf("%u\n", dist);                     // → 300
    return 0;
}
```

방법 2 가 Day 3 비트 연산의 출발점입니다. 정렬(alignment)이 맞지 않는 주소를 `uint32_t *` 로 캐스팅하면 일부 MCU 에서는 죽습니다.

### ▶ 예제 2-3 · const 는 오른쪽에서 왼쪽으로 읽는다

```c
#include <stdio.h>

int main(void) {
    int a = 1, b = 2;

    const int *p1 = &a;     // "p1 은 const int 를 가리킨다"  → *p1 = 9 불가, p1 = &b 가능
    int *const p2 = &a;     // "p2 는 int 를 가리키는 const" → *p2 = 9 가능, p2 = &b 불가
    const int *const p3 = &a;   // 둘 다 불가

    // *p1 = 9;      // error: assignment of read-only location
    p1 = &b;         // OK
    *p2 = 9;         // OK   (a 가 9 가 됨)
    // p2 = &b;      // error: assignment of read-only variable
    printf("%d %d %d\n", a, *p1, *p3);   // → 9 2 9
    return 0;
}
```

함수 인자에 `const int *a` 를 쓰면 "이 함수는 배열을 읽기만 한다" 는 약속이자 컴파일러 검사입니다. 어제 `is_valid(const int g[3])` 이 그것.

#### ✏️ 빈칸 채우기 2-1

```c
#include <stdio.h>
#include <stdint.h>

int main(void) {
    uint8_t frame[6] = {0x2C, 0x01, 0x3C, 0x00, 0x78, 0x00};   // 거리, 습도, 온도×?  (각 2바이트, LE)
    uint16_t *w = (uint16_t *)frame;
    printf("%u %u %u\n", w[0], w[1], ____);          // → 300 60 120
    printf("%ld\n", (long)(____ - w));               // → 3   (uint16_t 요소 3개 = 6바이트)
    uint8_t *end = (uint8_t *)(w + 3);
    printf("%ld\n", (long)(end - frame));            // → ____
    return 0;
}
```

<details><summary>🔒 정답 보기 2-1</summary>

```c
    printf("%u %u %u\n", w[0], w[1], w[2]);
    printf("%ld\n", (long)((uint16_t *)(frame + 6) - w));   // 또는 (w + 3) - w
    printf("%ld\n", (long)(end - frame));            // → 6
```
같은 메모리를 `uint16_t *` 로 재면 3, `uint8_t *` 로 재면 6 입니다.
</details>

#### ✏️ 괄호 넣기 2-2

1. `int *p` 에서 `p + 1` 은 (　　　)바이트, `double *q` 에서 `q + 1` 은 (　　　)바이트 이동한다.
2. `&arr[4] - &arr[0]` 의 값은 바이트 수가 아니라 (　　　)이다.
3. `const int *p` 는 (포인터 / 가리키는 값) 을 바꿀 수 없고, `int *const p` 는 (포인터 / 가리키는 값) 을 바꿀 수 없다.
4. 함수 매개변수를 `const int *a` 로 선언하면 함수가 배열을 (　　　)만 한다는 약속이다.

<details><summary>🔒 정답 보기 2-2</summary>

1. 4 / 8
2. 요소 개수 (4)
3. 가리키는 값 / 포인터
4. 읽기
</details>

---

# 3교시 · 배열과 포인터 (11:00–11:50)

**학습 목표**
- 배열 이름이 첫 요소 주소로 감쇠(decay)한다는 것을 안다
- 함수에 배열을 넘기면 `sizeof` 가 왜 8이 되는지 안다
- 2차원 배열의 메모리 배치를 그린다

🎬 **D2-3 배열 감쇠** — `sizeof(arr)` 가 함수 안에서 8이 되는 순간

### ▶ 예제 3-1 · arr[i] 는 *(arr + i) 의 설탕

```c
#include <stdio.h>

int main(void) {
    int arr[5] = {10, 20, 30, 40, 50};
    printf("%d %d %d\n", arr[2], *(arr + 2), 2[arr]);   // → 30 30 30  (셋 다 같은 뜻!)
    printf("%p %p\n", (void *)arr, (void *)&arr[0]);   // → 같은 주소
    printf("%zu\n", sizeof(arr));                       // → 20  (5 × 4)
    printf("%zu\n", sizeof(arr) / sizeof(arr[0]));      // → 5   (요소 개수 구하는 관용구)
    return 0;
}
```

`2[arr]` 가 되는 이유: `arr[2]` = `*(arr + 2)` = `*(2 + arr)` = `2[arr]`. 쓰지는 마세요, 이해만.

### ▶ 예제 3-2 · ⚠ 함정 — 함수 안에서 sizeof(arr) 는 8

```c
#include <stdio.h>

void show(int arr[]) {                   // 사실은 int *arr
    printf("in func : %zu\n", sizeof(arr));           // → 8   (포인터 크기!)
}

int sum(const int *a, int n) {           // 그래서 길이는 따로 받는다
    int s = 0;
    for (int i = 0; i < n; i++) s += a[i];
    return s;
}

int main(void) {
    int arr[5] = {10, 20, 30, 40, 50};
    printf("in main : %zu\n", sizeof(arr));           // → 20
    show(arr);
    printf("%d\n", sum(arr, sizeof(arr) / sizeof(arr[0])));   // → 150
    return 0;
}
```

```
warning: 'sizeof' on array function parameter 'arr' will return size of 'int *'
```

배열을 함수에 넘기는 순간 **첫 요소의 주소 하나**만 전달됩니다(감쇠). 길이 정보는 사라지므로 `(배열, 길이)` 를 항상 함께 넘깁니다. C 라이브러리가 `fwrite(buf, size, n, fp)` 처럼 생긴 이유.

### ▶ 예제 3-3 · 2차원 배열 — 행 우선, 한 덩어리

```c
#include <stdio.h>

int main(void) {
    int m[2][3] = {{1, 2, 3}, {4, 5, 6}};
    printf("%zu\n", sizeof(m));                                    // → 24
    printf("%ld\n", (long)((char *)&m[1][0] - (char *)&m[0][0]));  // → 12  (한 행 = 3 × 4)
    int *flat = &m[0][0];
    for (int i = 0; i < 6; i++) printf("%d ", flat[i]);           // → 1 2 3 4 5 6
    printf("\n");
    return 0;
}
```

`m[i][j]` 의 주소 = 시작 + `(i * 3 + j) * 4`. 함수에 넘길 때는 열 수를 알려야 이 계산을 할 수 있어 `int m[][3]` 으로 씁니다.

### ▶ 예제 3-4 · 센서 로그 통계 — 배열을 함수에 넘기는 연습

```c
#include <stdio.h>

double avg(const double *a, int n) {
    double s = 0;
    for (int i = 0; i < n; i++) s += a[i];
    return n ? s / n : 0;
}

int max_index(const double *a, int n) {
    int mi = 0;
    for (int i = 1; i < n; i++) if (a[i] > a[mi]) mi = i;
    return mi;
}

int main(void) {
    double t[] = {25.3, 25.1, 26.0, 37.2, 25.4, 25.2};
    int n = sizeof(t) / sizeof(t[0]);
    printf("avg=%.2f max=%.1f at %d\n", avg(t, n), t[max_index(t, n)], max_index(t, n));
    // → avg=27.37 max=37.2 at 3
    return 0;
}
```

#### ✏️ 빈칸 채우기 3-1

3×3 행렬 곱. 2차원 배열을 함수에 넘깁니다.

```c
#include <stdio.h>

void matmul(const int a[][3], const int b[][3], int c____) {
    for (int i = 0; i < 3; i++)
        for (int j = 0; j < 3; j++) {
            c[i][j] = 0;
            for (int k = 0; k < 3; k++) c[i][j] += a[i][k] * ____;
        }
}

int main(void) {
    int a[3][3] = {{1,0,0},{0,1,0},{0,0,1}};      // 단위행렬
    int b[3][3] = {{1,2,3},{4,5,6},{7,8,9}};
    int c[3][3];
    matmul(a, b, c);
    printf("%d %d %d\n", c[0][0], c[1][1], c[2][2]);   // → 1 5 9
    return 0;
}
```

<details><summary>🔒 정답 보기 3-1</summary>

```c
void matmul(const int a[][3], const int b[][3], int c[][3]) {
            for (int k = 0; k < 3; k++) c[i][j] += a[i][k] * b[k][j];
```
</details>

#### ✏️ 괄호 넣기 3-2

1. `arr[i]` 는 (　　　) 와 같은 뜻이다.
2. 함수 매개변수 `int arr[]` 는 실제로는 (　　　) 타입이다.
3. 요소 개수를 구하는 관용구는 `sizeof(arr) / (　　　)` 이다.
4. 배열을 함수에 넘길 때 길이 정보가 (남는다 / 사라진다).
5. `int m[2][3]` 에서 `m[1][0]` 은 시작 주소에서 (　　　)바이트 뒤에 있다.

<details><summary>🔒 정답 보기 3-2</summary>

1. `*(arr + i)`
2. `int *`
3. `sizeof(arr[0])`
4. 사라진다
5. 12
</details>

---

# 4교시 · 정렬과 검색 (13:00–13:50)

**학습 목표**
- 버블 정렬·선택 정렬을 포인터 swap 으로 구현한다
- 이진 검색이 정렬을 전제로 하는 이유를 안다
- off-by-one 으로 배열 밖을 건드리는 순간을 본다

🎬 **D2-4 버블 정렬 · 이진 검색** — 한 패스씩 진행

### ▶ 예제 4-1 · 버블 정렬 — 1교시 swap 재사용

```c
#include <stdio.h>

void swap(int *a, int *b) { int t = *a; *a = *b; *b = t; }

int bubble(int *a, int n) {
    int swaps = 0;
    for (int i = 0; i < n - 1; i++)
        for (int j = 0; j < n - 1 - i; j++)      // 뒤쪽 i 개는 이미 자리 잡음
            if (a[j] > a[j + 1]) { swap(&a[j], &a[j + 1]); swaps++; }
    return swaps;
}

int main(void) {
    int t[] = {26, 25, 37, 25, 24};
    int n = sizeof(t) / sizeof(t[0]);
    int s = bubble(t, n);
    for (int i = 0; i < n; i++) printf("%d ", t[i]);   // → 24 25 25 26 37
    printf("(%d swaps)\n", s);                          // → (7 swaps)
    return 0;
}
```

`j < n - 1 - i` 의 `-1` 을 빼면 `a[j + 1]` 이 `a[n]` 을 읽습니다 — 배열 밖. 실행해 보세요(오늘의 off-by-one).

### ▶ 예제 4-2 · 이진 검색 — 정렬돼 있어야 한다

```c
#include <stdio.h>

int bsearch_int(const int *a, int n, int key) {
    int lo = 0, hi = n - 1;
    while (lo <= hi) {
        int mid = lo + (hi - lo) / 2;      // (lo+hi)/2 는 오버플로 가능
        if (a[mid] == key) return mid;
        if (a[mid] < key)  lo = mid + 1;
        else               hi = mid - 1;
    }
    return -1;
}

int main(void) {
    int a[] = {24, 25, 25, 26, 37};
    printf("%d %d\n", bsearch_int(a, 5, 26), bsearch_int(a, 5, 30));   // → 3 -1
    return 0;
}
```

"없음" 을 -1 로 돌려주는 관례 — 인덱스는 0 이상이라 겹치지 않습니다. 반환값 하나로 결과와 실패를 같이 표현하는 흔한 방법.

### ▶ 예제 4-3 · ⚠ 함정 — 배열 밖은 조용히 남의 것을 덮는다

```c
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

struct Dev { char name[8]; int level; };

int main(void) {
    struct Dev *d = malloc(sizeof *d);
    strcpy(d->name, "ESP"); d->level = 1;
    printf("before: name=%s level=%d\n", d->name, d->level);     // → ESP 1
    strcpy(d->name, "ESP32-SENSOR");     // 13글자 → 8칸을 넘침
    printf("after : name=%s level=%d (0x%08X)\n", d->name, d->level, d->level);
    // → after : name=ESP32-SENSOR level=1380930382 (0x524F534E)
    free(d);
    return 0;
}
```

`0x524F534E` 을 ASCII 로 읽으면 `N S O R` — 넘친 글자가 옆 필드 `level` 에 그대로 들어갔습니다. 컴파일러가 경고를 주지만 **실행은 됩니다**. 이 구조체를 스택에 두면 glibc 가 `*** stack smashing detected ***` 로 잡아 주기도 하지만, MCU 에는 그런 보호가 없습니다.

#### ✏️ 빈칸 채우기 4-1

정렬된 배열의 중앙값. 짝수 개면 가운데 둘의 평균.

```c
#include <stdio.h>

double median(const int *a, int n) {       // a 는 정렬돼 있다고 가정
    if (n % 2 == 1) return a[____];
    return (a[n / 2 - 1] + a[____]) / ____;
}

int main(void) {
    int odd[]  = {24, 25, 25, 26, 37};
    int even[] = {24, 25, 26, 37};
    printf("%.1f %.1f\n", median(odd, 5), median(even, 4));   // → 25.0 25.5
    return 0;
}
```

<details><summary>🔒 정답 보기 4-1</summary>

```c
    if (n % 2 == 1) return a[n / 2];
    return (a[n / 2 - 1] + a[n / 2]) / 2.0;
```
`/ 2` 로 쓰면 어제 5교시의 정수 나눗셈 함정 — 25.5 가 25.0 이 됩니다.
</details>

#### ✏️ 괄호 넣기 4-2

1. 버블 정렬 안쪽 루프의 상한이 `n - 1 - i` 인 이유는 뒤쪽 (　　　)개가 이미 자리를 잡았기 때문이다.
2. 이진 검색은 배열이 (　　　)돼 있어야 동작한다.
3. `mid = (lo + hi) / 2` 대신 `lo + (hi - lo) / 2` 를 쓰는 이유는 (　　　)를 피하기 위해서다.
4. 검색 실패를 (　　　)로 돌려주는 관례는 유효한 인덱스와 겹치지 않기 때문이다.
5. 배열 밖에 쓰면 컴파일 에러가 (난다 / 나지 않는다).

<details><summary>🔒 정답 보기 4-2</summary>

1. i
2. 정렬
3. 정수 오버플로
4. −1
5. 나지 않는다 (경고는 날 수 있음)
</details>

---

# 5교시 · 문자열 — char 배열과 널 종료 (14:00–14:50)

**학습 목표**
- C 문자열이 `'\0'` 으로 끝나는 char 배열임을 안다
- `char s[]` 와 `char *s` 의 차이를 실행으로 확인한다
- `strlen/strcpy/strcmp` 를 포인터 순회로 직접 구현한다

🎬 **D2-5 char[] vs char \*** — 두 선언의 메모리 배치와 리터럴 쓰기 시 세그폴트

### ▶ 예제 5-1 · "ABC" 는 4바이트

```c
#include <stdio.h>
#include <string.h>

int main(void) {
    char s1[] = "ABC";        // 스택에 {'A','B','C','\0'} 복사 — 수정 가능
    char *s2  = "ABC";        // 읽기 전용 리터럴을 가리키는 포인터

    printf("%zu %zu %zu\n", sizeof(s1), sizeof(s2), strlen(s1));   // → 4 8 3
    s1[0] = 'X';
    printf("%s\n", s1);       // → XBC
    // s2[0] = 'X';           // → Segmentation fault (리터럴은 읽기 전용 영역)
    return 0;
}
```

`sizeof(s1)` 은 널 포함 4, `sizeof(s2)` 는 포인터 크기 8, `strlen` 은 널 전까지 3. 세 숫자를 구분하는 것이 오늘의 절반입니다. 리터럴에 쓰려면 `char *s2` 대신 `const char *s2` 로 선언해 **컴파일 에러**로 바꾸세요.

### ▶ 예제 5-2 · 라이브러리 함수를 직접 만들어 보기

```c
#include <stdio.h>

size_t my_strlen(const char *s) {
    const char *p = s;
    while (*p) p++;               // '\0' (값 0) 을 만날 때까지
    return p - s;                 // 포인터 뺄셈 = 글자 수
}

char *my_strcpy(char *d, const char *s) {
    char *r = d;
    while ((*d++ = *s++) != '\0') {}   // 복사하면서 전진, '\0' 까지 복사
    return r;
}

int my_strcmp(const char *a, const char *b) {
    while (*a && *a == *b) { a++; b++; }
    return (unsigned char)*a - (unsigned char)*b;   // 0 이면 같음
}

int main(void) {
    char buf[16];
    my_strcpy(buf, "sensor");
    printf("%zu %s %d %d\n", my_strlen(buf), buf,
           my_strcmp("abc", "abc"), my_strcmp("abc", "abd"));   // → 6 sensor 0 -1
    return 0;
}
```

`*d++ = *s++` 한 줄이 1교시 `*p++` 의 응용입니다. `strcmp` 가 0 이면 **같다** — `if (strcmp(a, b))` 는 "다르면" 이라는 뜻이라 자주 틀립니다.

### ▶ 예제 5-3 · 프레임에서 값 찾기 — strchr, strstr

```c
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

int main(void) {
    const char *frame = "$T=25.3,H=60,D=120*";
    const char *t = strstr(frame, "T=");       // "T=" 가 시작하는 위치
    const char *h = strstr(frame, "H=");
    const char *e = strchr(frame, '*');        // '*' 위치
    printf("%s\n", t + 2);                     // → 25.3,H=60,D=120*   (끝까지 출력됨)
    printf("%.1f %d\n", atof(t + 2), atoi(h + 2));   // → 25.3 60   (숫자 아닌 곳에서 멈춤)
    printf("%ld\n", (long)(e - frame));        // → 18  ('*' 의 인덱스)
    return 0;
}
```

`atof/atoi` 는 숫자가 끝나는 곳에서 조용히 멈춥니다 — 편하지만 실패를 알 수 없습니다. 6교시에서 `strtol` 로 바꿉니다.

### ▶ 예제 5-4 · ⚠ 함정 — 8칸에 13글자

```c
#include <stdio.h>
#include <string.h>

int main(void) {
    char buf[8];
    strcpy(buf, "ESP32-SENSOR");     // 13 + '\0' = 14바이트를 8칸에
    printf("%s\n", buf);
    return 0;
}
```

```bash
gcc -Wall -Wextra -g ovf.c -o ovf
# warning: '__builtin_memcpy' writing 14 bytes into a region of size 8 overflows the destination
./ovf
# ESP32-SENSOR
# *** stack smashing detected ***: terminated
```

경고 → 실행은 됨 → 종료 직전에 죽음. 4교시 예제 4-3 과 같은 현상이고, **입력이 사용자에게서 오면** 이게 보안 취약점(buffer overflow)이 됩니다.

#### ✏️ 빈칸 채우기 5-1

문자열을 제자리에서 뒤집기.

```c
#include <stdio.h>
#include <string.h>

void reverse(char *s) {
    char *a = s, *b = s + strlen(s) ____;     // 마지막 글자 ('\0' 앞)
    while (a < b) {
        char t = *a; *a = *b; *b = t;
        ____; ____;
    }
}

int main(void) {
    char s[] = "sensor";
    reverse(s);
    printf("%s\n", s);      // → rosnes
    return 0;
}
```

<details><summary>🔒 정답 보기 5-1</summary>

```c
    char *a = s, *b = s + strlen(s) - 1;
        a++; b--;
```
`char *s = "sensor"` 로 선언했다면 리터럴을 수정하려다 세그폴트가 납니다.
</details>

#### ✏️ 괄호 넣기 5-2

1. `"ABC"` 가 차지하는 바이트 수는 (　　　)이고, `strlen("ABC")` 는 (　　　)이다.
2. `char s[] = "ABC"` 는 (스택 / 읽기 전용 영역)에 복사본을 만들고, `char *s = "ABC"` 는 (스택 / 읽기 전용 영역)의 리터럴을 가리킨다.
3. `strcmp(a, b)` 가 (　　　)을 반환하면 두 문자열이 같다.
4. `while (*p) p++;` 는 값이 (　　　)인 문자를 만날 때까지 전진한다.
5. `char buf[8]` 에 13글자를 `strcpy` 하면 컴파일러는 (에러 / 경고)를 내고 실행은 (된다 / 안 된다).

<details><summary>🔒 정답 보기 5-2</summary>

1. 4 / 3
2. 스택 / 읽기 전용 영역
3. 0
4. 0 (`'\0'`)
5. 경고 / 된다
</details>

---

# 6교시 · 안전한 문자열 처리와 파싱 (15:00–15:50)

**학습 목표**
- `fgets` 로 줄을 안전하게 읽고 끝의 `\n` 을 처리한다
- `strncpy` 의 함정을 알고 `snprintf` 를 쓴다
- `strtok` + `strtol` 로 센서 프레임을 구조체로 파싱하고, 오류를 감지한다

🎬 **D2-6 strtok 분리** — 콤마가 `'\0'` 로 바뀌며 토큰이 잘리는 모습

### ▶ 예제 6-1 · fgets — scanf("%s") 를 버리는 이유

```c
#include <stdio.h>
#include <string.h>

int main(void) {
    char line[32];
    printf("프레임 > ");
    if (fgets(line, sizeof line, stdin) == NULL) return 1;   // EOF 나 오류
    line[strcspn(line, "\n")] = '\0';        // 끝의 개행 제거 (없으면 그대로)
    printf("[%s] %zu\n", line, strlen(line));
    return 0;
}
```

`fgets` 는 **버퍼 크기를 알고** 그 안에서만 읽습니다. `scanf("%s", buf)` 는 크기를 모르므로 5교시의 오버플로가 사용자 입력으로 재현됩니다. 31글자 넘게 입력해 보세요 — 잘리기만 하고 죽지 않습니다.

### ▶ 예제 6-2 · ⚠ 함정 — strncpy 는 널을 보장하지 않는다

```c
#include <stdio.h>
#include <string.h>

int main(void) {
    char a[4], b[4];
    strncpy(a, "ABCDEFG", 4);           // 4글자 채우고 끝 — '\0' 없음!
    printf("%c%c%c%c|\n", a[0], a[1], a[2], a[3]);   // → ABCD|
    // printf("%s\n", a);                // '\0' 이 없어 옆 메모리까지 출력 (정의되지 않은 동작)

    snprintf(b, sizeof b, "%s", "ABCDEFG");    // 크기 안에서 자르고 항상 '\0'
    printf("%s\n", b);                          // → ABC
    return 0;
}
```

`strncpy` 는 이름과 달리 안전하지 않습니다. 잘라서 복사할 때는 `snprintf(dst, sizeof dst, "%s", src)` 를 습관으로.

### ▶ 예제 6-3 · strtok 과 strtol — 프레임을 조각내고 숫자로

```c
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

int main(void) {
    char line[] = "$T=25.3,H=60,D=120*";         // strtok 은 원본을 고치므로 배열이어야 함

    char *tok = strtok(line + 1, ",");           // '$' 건너뛰고 첫 토큰
    while (tok) {
        printf("[%s] ", tok);                    // → [T=25.3] [H=60] [D=120*]
        tok = strtok(NULL, ",");                 // 다음 토큰은 NULL 로 이어서
    }
    printf("\n");

    char *end;
    long v = strtol("120*", &end, 10);
    printf("%ld '%s'\n", v, end);                // → 120 '*'   : 숫자가 끝난 곳을 알려줌
    v = strtol("abc", &end, 10);
    printf("%ld %d\n", v, end == "abc" ? 1 : 0); // 실패하면 end 가 시작 위치 그대로
    return 0;
}
```

🎬 D2-6: `strtok` 은 콤마 자리에 `'\0'` 을 써 넣고 그 앞 조각의 시작 주소를 돌려줍니다. 그래서 문자열 리터럴(`char *`)에는 못 쓰고, 한 번 자르면 원본은 사라집니다.

### ▶ 예제 6-4 · parse_frame — 어제의 Reading 을 채우는 함수

`frame.h`
```c
#ifndef FRAME_H
#define FRAME_H
typedef struct { double t; int h; int d; } Reading;
int parse_frame(const char *line, Reading *out);   // 성공 1, 실패 0
#endif
```

`frame.c`
```c
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "frame.h"

static int get_num(const char *src, const char *key, double *val) {
    const char *p = strstr(src, key);
    if (!p) return 0;
    char *end;
    *val = strtod(p + strlen(key), &end);
    return end != p + strlen(key);               // 숫자가 하나도 없으면 실패
}

int parse_frame(const char *line, Reading *out) {
    if (line[0] != '$' || strchr(line, '*') == NULL) return 0;
    double t, h, d;
    if (!get_num(line, "T=", &t)) return 0;
    if (!get_num(line, "H=", &h)) return 0;
    if (!get_num(line, "D=", &d)) return 0;
    out->t = t; out->h = (int)h; out->d = (int)d;
    return 1;
}
```

`test_frame.c`
```c
#include <stdio.h>
#include "frame.h"

int main(void) {
    const char *tests[] = {
        "$T=25.3,H=60,D=120*",   // 정상
        "$T=36.9,H=70,D=150*",   // 정상
        "T=25.3,H=60,D=120*",    // '$' 없음
        "$T=25.3,H=60*",         // D 없음
        "$T=abc,H=60,D=120*",    // 숫자 아님
        "$T=25.3,H=60,D=120",    // '*' 없음
    };
    for (int i = 0; i < 6; i++) {
        Reading r;
        if (parse_frame(tests[i], &r)) printf("OK  T=%.1f H=%d D=%d\n", r.t, r.h, r.d);
        else                            printf("BAD %s\n", tests[i]);
    }
    return 0;
}
```

```bash
gcc -Wall -Wextra -g test_frame.c frame.c -o test_frame && ./test_frame
# OK  T=25.3 H=60 D=120
# OK  T=36.9 H=70 D=150
# BAD T=25.3,H=60,D=120*
# BAD $T=25.3,H=60*
# BAD $T=abc,H=60,D=120*
# BAD $T=25.3,H=60,D=120
```

`static int get_num` — 어제 7교시의 "파일 밖에서 안 보이는 static". 이 함수는 8교시 과제의 부품입니다. **`out` 은 성공했을 때만 채웁니다** — 실패 시 반쯤 채워진 구조체를 남기지 않는 것이 규칙.

#### ✏️ 빈칸 채우기 6-1

어제 숫자 야구의 `read_guess` 를 `fgets` + `strtol` 로 다시 씁니다.

```c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int read_guess(int g[3]) {
    char line[32];
    if (fgets(line, ____, stdin) == NULL) return 0;
    char *p = line;
    for (int i = 0; i < 3; i++) {
        char *end;
        g[i] = (int)strtol(p, ____, 10);
        if (end == ____) return 0;      // 숫자를 하나도 못 읽음
        p = end;
    }
    return 1;
}

int main(void) {
    int g[3];
    while (read_guess(g)) printf("%d %d %d\n", g[0], g[1], g[2]);
    return 0;
}
```

<details><summary>🔒 정답 보기 6-1</summary>

```c
    if (fgets(line, sizeof line, stdin) == NULL) return 0;
        g[i] = (int)strtol(p, &end, 10);
        if (end == p) return 0;
```
`strtol` 은 앞의 공백을 건너뛰므로 `"1 2 3"` 도 `"1,2,3"` 도 아닌 `"1 2 3"` 만 통과합니다. 콤마도 허용하려면 `p = end; if (*p == ',') p++;`.
</details>

#### ✏️ 괄호 넣기 6-2

1. `fgets(buf, n, stdin)` 은 최대 (　　　)글자 + 널을 읽으며, 줄 끝의 (　　　) 문자도 버퍼에 남긴다.
2. `strncpy` 는 길이를 다 채우면 (　　　)을 붙이지 않는다. 대신 (　　　)를 쓴다.
3. `strtok` 은 구분자 자리에 (　　　)을 써 넣으므로 원본이 (보존된다 / 바뀐다).
4. `strtol(s, &end, 10)` 에서 `end == s` 이면 (　　　)했다는 뜻이다.
5. `parse_frame` 은 실패했을 때 `out` 을 (채운다 / 건드리지 않는다).

<details><summary>🔒 정답 보기 6-2</summary>

1. n − 1 / `\n`
2. `'\0'` / `snprintf`
3. `'\0'` / 바뀐다
4. 숫자를 하나도 읽지 못해 실패
5. 건드리지 않는다
</details>

---

# 7교시 · 동적 메모리 — malloc/free (16:00–16:50)

**학습 목표**
- 스택 대신 힙을 써야 하는 두 가지 경우를 안다
- `malloc → NULL 검사 → 사용 → free` 를 손에 익힌다
- `realloc` 으로 배열을 키우고, valgrind 로 누수를 확인한다

🎬 **D2-7 힙 할당·해제** — 블록이 생기고 사라지는 모습, 누수·이중 해제

### ▶ 예제 7-1 · 스택으로 안 되는 두 가지

```c
#include <stdio.h>
#include <stdlib.h>

int *make_array(int n) {              // ① 함수가 끝나도 살아 있어야 한다
    // int a[n]; return a;             //    ← 스택 배열의 주소를 돌려주면 죽은 메모리
    int *a = malloc(n * sizeof(int));
    if (a == NULL) return NULL;       //    항상 검사 (MCU 에서는 정말 실패함)
    for (int i = 0; i < n; i++) a[i] = i * 10;
    return a;                         //    호출자가 free 할 책임
}

int main(void) {
    int n;
    printf("개수 > "); if (scanf("%d", &n) != 1) return 1;   // ② 크기를 실행 중에 정한다
    int *a = make_array(n);
    if (!a) { printf("메모리 부족\n"); return 1; }
    for (int i = 0; i < n; i++) printf("%d ", a[i]);
    printf("\n");
    free(a);                          // 다 썼으면 돌려준다
    a = NULL;                         // 습관: 해제 후 NULL
    return 0;
}
```

**소유권 규칙**: 누가 `free` 하는지를 함수 주석에 적습니다. `make_array` 는 "반환값은 호출자가 free" — 이걸 안 적어서 생기는 누수가 실제 코드에서 가장 흔합니다.

### ▶ 예제 7-2 · realloc — 줄 수를 모를 때

```c
#include <stdio.h>
#include <stdlib.h>

int main(void) {
    int cap = 2, n = 0;
    int *a = malloc(cap * sizeof(int));
    if (!a) return 1;

    int v;
    while (scanf("%d", &v) == 1) {              // Ctrl-D 로 종료
        if (n == cap) {
            cap *= 2;                            // 두 배씩 키우기
            int *tmp = realloc(a, cap * sizeof(int));
            if (!tmp) { free(a); return 1; }    // 실패 시 원본은 살아 있다
            a = tmp;
            printf("(cap → %d)\n", cap);
        }
        a[n++] = v;
    }
    for (int i = 0; i < n; i++) printf("%d ", a[i]);
    printf("\n%d개\n", n);
    free(a);
    return 0;
}
```

```bash
printf "25 26 37 25 24\n" | ./grow
# (cap → 4)
# (cap → 8)
# 25 26 37 25 24
# 5개
```

`a = realloc(a, ...)` 로 바로 받으면 실패 시 원본 주소를 잃어 누수가 됩니다. `tmp` 로 받는 것이 관용구.

### ▶ 예제 7-3 · valgrind — 세 가지 실수 잡기

`leak.c`
```c
#include <stdlib.h>
#include <stdio.h>

int main(void) {
    int *p = malloc(10 * sizeof(int));
    p[0] = 1;
    printf("%d\n", p[0]);
    return 0;                       // free 없음
}
```

```bash
gcc -Wall -Wextra -g leak.c -o leak
valgrind --leak-check=full ./leak
#   in use at exit: 40 bytes in 1 blocks
#   definitely lost: 40 bytes in 1 blocks
#     at malloc ...  by main (leak.c:5)        ← 어디서 할당했는지
```

같은 방법으로 아래 두 개도 실행해 메시지를 읽어 두세요:

```c
free(p); p[0] = 2;      // Invalid write of size 4 ... Address ... is 0 bytes inside a block ... free'd
free(p); free(p);       // Invalid free() / delete / delete[]
```

valgrind 가 없는 환경(MCU)에서는 이 세 실수가 조용히 시스템을 망가뜨립니다. 오늘 PC 에서 메시지를 눈에 익혀 두는 것이 목적.

#### ✏️ 빈칸 채우기 7-1

문자열을 힙에 복사하는 `my_strdup`. (표준 `strdup` 을 직접 구현)

```c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char *my_strdup(const char *s) {          // 반환값은 호출자가 free
    size_t n = strlen(s) ____;            // '\0' 자리
    char *d = malloc(____);
    if (d == NULL) return NULL;
    memcpy(d, s, n);
    return d;
}

int main(void) {
    char *copy = my_strdup("$T=25.3,H=60,D=120*");
    if (!copy) return 1;
    copy[0] = '#';
    printf("%s\n", copy);      // → #T=25.3,H=60,D=120*
    ____(copy);
    return 0;
}
```

<details><summary>🔒 정답 보기 7-1</summary>

```c
    size_t n = strlen(s) + 1;
    char *d = malloc(n);
    free(copy);
```
`+ 1` 을 빼면 `'\0'` 이 안 들어가 5교시 함정이 힙에서 재현됩니다. valgrind 가 "Invalid write of size 1" 로 잡습니다.
</details>

#### ✏️ 괄호 넣기 7-2

1. 함수가 끝나도 살아 있어야 하는 데이터는 (스택 / 힙)에 둔다.
2. `malloc` 의 반환값은 사용하기 전에 반드시 (　　　)인지 검사한다.
3. `a = realloc(a, ...)` 대신 `tmp` 로 받는 이유는 실패 시 (　　　)를 잃지 않기 위해서다.
4. `free(p)` 뒤에 `p` 를 (　　　)로 만드는 습관은 해제 후 사용을 막는다.
5. valgrind 의 "definitely lost" 는 (　　　)를 뜻한다.

<details><summary>🔒 정답 보기 7-2</summary>

1. 힙
2. NULL
3. 원본 주소 (기존 메모리)
4. NULL
5. 메모리 누수 (free 하지 않은 블록)
</details>

---

# 8교시 · 종합 실습 — 센서 로그 분석기 · 정리 (17:00–17:50)

🎬 **D2-8 로그 분석기 흐름** — 입력 → 파싱 → 힙 누적 → 정렬 → 통계 → 해제

**과제** — 표준 입력으로 센서 프레임을 한 줄씩 받아 통계를 냅니다. 파일 4개(`main.c`, `frame.c`, `frame.h`, `Makefile`), `-Wall -Wextra` 경고 0개, **valgrind 누수 0**.

| 단계 | 함수 | 오늘 배운 것 |
|---|---|---|
| 읽기 | `fgets` 루프 | 6교시 · 안전 입력, `\n` 제거 |
| 파싱 | `parse_frame(line, &r)` | 6교시 · 실패 시 줄 번호와 함께 `BAD` 출력하고 계속 |
| 누적 | `Reading *log`, `realloc` 두 배 | 7교시 · tmp 관용구 |
| 정렬 | 온도 기준 버블 정렬 (`Reading` 통째로 swap) | 4교시 · 구조체 swap 은 `Reading t = a[j]; …` |
| 통계 | 최솟값·최댓값·중앙값·평균, ALERT(35℃ 초과) 개수 | 3·4교시 |
| 해제 | `free(log)` | 7교시 |

### ▶ 뼈대 — 여기서 시작

`main.c`
```c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "frame.h"

static void sort_by_temp(Reading *a, int n) {
    for (int i = 0; i < n - 1; i++)
        for (int j = 0; j < n - 1 - i; j++)
            if (a[j].t > a[j + 1].t) { Reading t = a[j]; a[j] = a[j + 1]; a[j + 1] = t; }
}

int main(void) {
    int cap = 4, n = 0, lineno = 0, bad = 0;
    Reading *log = malloc(cap * sizeof *log);
    if (!log) return 1;

    char line[64];
    while (fgets(line, sizeof line, stdin)) {
        lineno++;
        line[strcspn(line, "\n")] = '\0';
        if (line[0] == '\0') continue;                    // 빈 줄 무시
        Reading r;
        if (!parse_frame(line, &r)) { printf("BAD line %d: %s\n", lineno, line); bad++; continue; }
        if (n == cap) {
            // ____ : realloc 으로 cap 두 배 (tmp 관용구)
        }
        log[n++] = r;
    }

    if (n == 0) { printf("데이터 없음\n"); free(log); return 0; }
    sort_by_temp(log, n);

    // ____ : 최솟값 log[0].t, 최댓값 log[n-1].t, 중앙값(4교시 median 을 Reading 용으로), 평균, ALERT 개수
    printf("n=%d bad=%d\n", n, bad);

    free(log);
    return 0;
}
```

`frame.h`, `frame.c` 는 6교시 예제 6-4 그대로. `Makefile` 은 어제 것에 `-g` 추가.

테스트 입력 `log.txt`
```
$T=25.3,H=60,D=120*
$T=26.0,H=61,D=118*
$T=37.2,H=70,D=150*
T=25.1,H=60,D=120*
$T=24.8,H=59,D=121*
$T=abc,H=60,D=120*
$T=36.1,H=68,D=140*
```

```bash
make && ./analyzer < log.txt
# BAD line 4: T=25.1,H=60,D=120*
# BAD line 6: $T=abc,H=60,D=120*
# min=24.8 max=37.2 median=26.0 avg=29.88 alert=2
# n=5 bad=2
valgrind --leak-check=full ./analyzer < log.txt 2>&1 | grep -E "definitely|ERROR SUMMARY"
# definitely lost: 0 bytes in 0 blocks
# ERROR SUMMARY: 0 errors
```

<details><summary>🔒 정답 보기 8-A (리뷰 시간에 함께 엽니다)</summary>

```c
        if (n == cap) {
            cap *= 2;
            Reading *tmp = realloc(log, cap * sizeof *log);
            if (!tmp) { free(log); return 1; }
            log = tmp;
        }
```
```c
    double sum = 0; int alert = 0;
    for (int i = 0; i < n; i++) { sum += log[i].t; if (log[i].t > 35.0) alert++; }
    double med = (n % 2) ? log[n / 2].t : (log[n / 2 - 1].t + log[n / 2].t) / 2.0;
    printf("min=%.1f max=%.1f median=%.1f avg=%.2f alert=%d\n",
           log[0].t, log[n - 1].t, med, sum / n, alert);
```
</details>

**제출 체크리스트**
- [ ] `make clean && make` 성공, `-Wall -Wextra` 경고 0개
- [ ] `BAD` 줄이 있어도 죽지 않고 끝까지 처리
- [ ] 입력 100줄(`cap` 을 여러 번 넘김)에서도 정상
- [ ] `valgrind` — definitely lost 0 bytes, ERROR SUMMARY 0
- [ ] 미완성이어도 `parse_frame` 통합 + `realloc` 루프가 있으면 통과

### 오늘의 여섯 문장

1. `&` 는 주소, `*` 는 그 주소의 값. 원본을 바꾸려면 주소를 넘긴다 (D2-1)
2. `p + 1` 은 `sizeof(*p)` 바이트 이동. 포인터 뺄셈은 요소 개수 (D2-2)
3. 배열은 함수에 넘기는 순간 포인터가 된다 — 길이를 같이 넘겨라 (D2-3)
4. 배열 밖에 쓰면 컴파일러는 막지 않는다. 옆 변수가 조용히 바뀐다 (D2-4·D2-5)
5. 문자열은 `'\0'` 까지. `char[]` 는 수정 가능, `char *` 리터럴은 읽기 전용. `fgets`·`snprintf`·`strtol` 을 쓴다 (D2-5·D2-6)
6. `malloc → NULL 검사 → 사용 → free`. 누가 free 하는지 적어라. valgrind 로 확인 (D2-7)

- 개념 워크시트: `worksheet_day2.html` 8교시까지 채점 후 결과 코드 제출
- 제출: `c_day2/analyzer/` 폴더 (4개 파일) + valgrind 결과 캡처 + 워크시트 캡처
- **Day 3 예고**: 구조체 패딩 (오늘 `Reading` 이 왜 16바이트인지), 비트 연산 (2교시 `buf[0] | buf[1] << 8` 의 확장), 함수 포인터 (오늘 버블 정렬을 `qsort` + 비교 함수로 바꾸기)
