/* sonar.h — HC-SR04 모듈 (Day 3 09_sonar 를 모듈로)
 *   Trig → GPIO23 · Echo → 1kΩ → GPIO24 (GPIO24 → 2kΩ → GND 분압)
 *   sonar_t *s = sonar_open(23, 24);  double cm = sonar_median(s, 3);   (-1 = 측정 실패)
 */
#ifndef SONAR_H
#define SONAR_H

typedef struct sonar sonar_t;

sonar_t *sonar_open(unsigned int trig, unsigned int echo);
double sonar_measure(sonar_t *s);              /* 한 번 — cm 또는 -1 */
double sonar_median(sonar_t *s, int n);        /* n 번(≤ 7) 중앙값 — cm 또는 -1 */
void sonar_close(sonar_t *s);

#endif
