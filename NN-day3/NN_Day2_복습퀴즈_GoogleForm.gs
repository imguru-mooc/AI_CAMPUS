/**
 * Neural Network Day 2 복습 퀴즈 — Google Form 자동 생성 스크립트
 * HYUNDAI AI Insight Campus 온디바이스 AI · Neural Network Day 3 (10/22) 09:00~09:15
 *
 * 사용 방법
 * 1. https://script.google.com 접속 → 새 프로젝트
 * 2. 이 파일 전체를 붙여 넣고 저장
 * 3. 상단 함수 선택에서 createNNDay2Quiz 선택 → ▶ 실행 (최초 1회 권한 승인)
 * 4. 하단 실행 로그에 출력되는 두 URL 확인
 *    - 편집용(editUrl): 강사용, 응답 확인·설정 변경
 *    - 응답용(publishedUrl): 학생에게 공유
 *
 * 생성되는 폼 설정
 * - 퀴즈 모드(자동 채점, 문항당 1점, 총 20점)
 * - 이름 필수 입력, 문항 전부 필수
 * - 제출 직후 점수 공개, 정답·해설·복습할 애니메이션 표시
 * - 문항 순서 고정(Day 2 교시 순), 보기 순서도 고정
 */

function createNNDay2Quiz() {
  const form = FormApp.create('Neural Network Day 2 복습 퀴즈');
  form.setIsQuiz(true)
      .setDescription(
        '범위: Day 2 전체 (퍼셉트론 · 학습 규칙 · XOR · MLP · 활성화 함수 · 순전파 · softmax · 손실 · 선형 회귀 · 로지스틱 회귀)\n' +
        '문항당 1점, 총 20점 · 권장 시간 15분\n' +
        '제출 후 점수와 해설이 바로 표시됩니다. 틀린 문항은 해설의 애니메이션으로 복습하세요.\n' +
        '애니메이션: https://imguru-mooc.github.io/AI_CAMPUS/NN-day2/animation.html')
      .setShuffleQuestions(false)
      .setProgressBar(true);
  // 조직 계정 배포 시 1인 1회 제한을 쓰려면 주석 해제 (Google 로그인 필요)
  // form.setLimitOneResponsePerUser(true);
  // form.setCollectEmail(true);

  form.addTextItem().setTitle('이름').setRequired(true);
  form.addPageBreakItem().setTitle('문항 (20)');

  QUESTIONS.forEach(function (q, i) {
    const item = form.addMultipleChoiceItem();
    item.setTitle((i + 1) + '. ' + q.title)
        .setPoints(1)
        .setRequired(true);
    item.setChoices(q.options.map(function (opt, j) {
      return item.createChoice(opt, j === q.answer);
    }));
    item.setFeedbackForCorrect(
      FormApp.createFeedback().setText('정답입니다. ' + q.why + (q.anim ? '  [복습: ' + q.anim + ']' : '')).build());
    item.setFeedbackForIncorrect(
      FormApp.createFeedback().setText('정답: ' + LABEL[q.answer] + ' ' + q.options[q.answer] + ' — ' + q.why + (q.anim ? '  [복습: ' + q.anim + ']' : '')).build());
  });

  Logger.log('편집용  URL: ' + form.getEditUrl());
  Logger.log('응답용  URL: ' + form.getPublishedUrl());
}

var LABEL = ['①', '②', '③', '④'];

var QUESTIONS = [
  { title: "퍼셉트론 y = step(xW + b) 로 AND 게이트를 만드는 파라미터는?",
    options: ["W = (0.5, 0.5), b = −0.7", "W = (−0.5, −0.5), b = 0.7", "W = (0.5, 0.5), b = 0.2", "W = (0.5, 0.5), b = −0.2"],
    answer: 0,
    why: "두 입력이 모두 1 일 때만 0.5 + 0.5 − 0.7 = 0.3 > 0 입니다. b = −0.2 는 OR, (−0.5, −0.5)·0.7 은 NAND — 같은 함수, 다른 숫자.",
    anim: "D2-1" },
  { title: "결정 직선 x1·W1 + x2·W2 + b = 0 에서 W 는 그대로 두고 b 만 바꾸면?",
    options: ["직선은 그대로, 0 과 1 출력만 뒤바뀐다", "직선의 기울기만 바뀌고 위치는 그대로다", "직선이 원점을 중심으로 회전한다", "기울기는 그대로, 직선이 평행 이동한다"],
    answer: 3,
    why: "가중치 W 는 직선의 방향(기울기), 편향 b 는 직선의 위치를 정합니다.",
    anim: "D2-1" },
  { title: "Day 2 에서 쓴 퍼셉트론 학습 규칙은?",
    options: ["W ← W − η(y − t)x", "W ← W − η(y − t)", "W ← W − η·y·x", "W ← W + η(y − t)x"],
    answer: 0,
    why: "(예측 − 정답) × 입력 만큼 반대로 고칩니다. 맞히면 y − t = 0 이라 바뀌지 않습니다. 편향은 b ← b − η(y − t).",
    anim: "D2-2" },
  { title: "예측 y = 1, 정답 t = 0, 입력 x = (1, 1), η = 0.1 일 때 학습 규칙을 한 번 적용하면 W 는?",
    options: ["두 성분이 모두 0 으로 초기화된다", "두 성분이 모두 0.1 씩 늘어난다", "W 는 그대로이고 b 만 바뀐다", "두 성분이 모두 0.1 씩 줄어든다"],
    answer: 3,
    why: "W − 0.1·(1 − 0)·(1, 1) = W − (0.1, 0.1). 1 이라고 잘못 말했으니 점수를 낮추는 쪽으로 고칩니다.",
    anim: "D2-2" },
  { title: "퍼셉트론 하나로 XOR 을 50 epoch 학습해도 정확도가 0.5 에서 맴도는 이유는?",
    options: ["학습률 η = 0.1 이 너무 작아서", "XOR 은 직선 하나로 나눌 수 없어서", "epoch 을 충분히 늘리지 않아서", "초기 가중치를 0 으로 잡아서"],
    answer: 1,
    why: "학습 방법이 아니라 모델(직선 하나)이 XOR 을 표현할 수 없습니다 — 선형 분리 불가능.",
    anim: "D2-2" },
  { title: "XOR 2층 망의 은닉층 h = (NAND, OR) 은 입력 (1, 0) 을 어떤 좌표로 옮기나?",
    options: ["(0, 0)", "(1, 0)", "(1, 1)", "(0, 1)"],
    answer: 2,
    why: "NAND(1,0) = 1, OR(1,0) = 1. (0,1) 과 (1,0) 이 모두 (1,1) 로 모여 직선 하나(AND)로 나눌 수 있게 됩니다 — 은닉층 = 공간 변환.",
    anim: "D2-3" },
  { title: "sigmoid 의 도함수 s(1 − s) 의 최댓값은?",
    options: ["x = 0 에서 1.0", "x → ∞ 에서 1.0", "x = 0 에서 0.5", "x = 0 에서 0.25"],
    answer: 3,
    why: "s(0) = 0.5 → 0.5 × 0.5 = 0.25. 층마다 최대 0.25 배라 깊이 쌓으면 기울기가 사라집니다 (0.25 의 10제곱 ≈ 9.5e-7).",
    anim: "D2-4" },
  { title: "ReLU 의 도함수를 NumPy 로 쓴 것은?",
    options: ["(x < 0).astype(float)", "x * (1 - x)", "(x > 0).astype(float)", "np.maximum(0, x)"],
    answer: 2,
    why: "양수 구간 기울기 1, 음수 구간 0. np.maximum(0, x) 는 ReLU 자체, x(1 − x) 는 sigmoid 출력으로 쓴 도함수 모양입니다.",
    anim: "D2-4" },
  { title: "활성화 함수 없이 선형 층 두 개 (X @ Wa) @ Wb 를 쌓으면?",
    options: ["XOR 같은 비선형 문제도 풀린다", "출력이 항상 0 과 1 사이가 된다", "W = Wa @ Wb 인 층 하나와 같다", "표현력이 두 배인 2층 망이 된다"],
    answer: 2,
    why: "(X @ Wa) @ Wb = X @ (Wa @ Wb). 비선형 활성화가 있어야 층을 쌓는 의미가 생깁니다.",
    anim: "D2-4" },
  { title: "MNIST 784-100-10 망에 배치 100 을 넣을 때 A1 = X @ W1 + b1 의 shape 은?",
    options: ["(100, 100)", "(100, 784)", "(784, 100)", "(100, 10)"],
    answer: 0,
    why: "X (100, 784) @ W1 (784, 100) = (100, 100). 행 = 샘플, W shape = (이전 층 뉴런 수, 이번 층 뉴런 수).",
    anim: "D2-5" },
  { title: "2-3-2-2 망의 파라미터는 (2·3+3)+(3·2+2)+(2·2+2) = 23개다. 784-100-10 망은?",
    options: ["79,510개", "79,400개", "78,400개", "80,510개"],
    answer: 0,
    why: "784·100 + 100 + 100·10 + 10 = 79,510. 편향을 빼먹으면 79,400. Day 3 에서 수치 미분 비용(2P = 159,020번)을 이 수로 계산합니다.",
    anim: "D2-5" },
  { title: "softmax 에서 A − A.max(axis=1, keepdims=True) 를 먼저 하는 이유는?",
    options: ["exp 오버플로(inf → nan)를 막으려고", "가장 큰 점수를 정답으로 만들려고", "음수 점수를 모두 없애려고", "확률의 합을 1 로 맞추려고"],
    answer: 0,
    why: "모든 점수에서 같은 상수를 빼도 softmax 결과는 같습니다. naive softmax([1010, 1000, 990]) 은 nan, 안정화하면 [0.99995, 4.5e-05, 2.1e-09].",
    anim: "D2-6" },
  { title: "10클래스에서 np.eye(10)[t] 에 t = 3 을 넣은 결과는?",
    options: ["앞에서 3개가 1 인 길이 10 벡터", "인덱스 3 만 0 인 길이 10 벡터", "인덱스 3 만 1 인 길이 10 벡터", "값이 3 인 길이 1 의 벡터"],
    answer: 2,
    why: "단위행렬의 t 번째 행 = one-hot. 배치 레이블 t = [3, 0, ...] 을 넣으면 한 번에 (N, 10) 이 됩니다.",
    anim: "D2-6" },
  { title: "정답 클래스 확률이 0.6 일 때 cross-entropy 손실은?",
    options: ["(1 − 0.6)² = 0.16", "−log 0.4 ≈ 0.9163", "1 − 0.6 = 0.4", "−log 0.6 ≈ 0.5108"],
    answer: 3,
    why: "CE 는 one-hot 덕분에 정답 자리만 남아 −log(정답 확률) 입니다. 같은 예에서 MSE 는 ≈ 0.0195.",
    anim: "D2-7" },
  { title: "10클래스 분류에서 정답 확률이 0.1 이면 CE ≈ 2.3026 이다. 이 값의 의미는?",
    options: ["학습률을 2.3 배로 키우라는 신호", "10개 중 균등하게 찍은 수준 (= ln 10)", "모든 샘플을 확신하며 맞힌 상태", "cross-entropy 가 가질 수 있는 최댓값"],
    answer: 1,
    why: "−log 0.1 = ln 10 ≈ 2.3026. 정답 확률이 0 에 가까우면 CE 는 끝없이 커집니다. Day 3 MNIST 의 학습 전 손실도 이 근처입니다.",
    anim: "D2-7" },
  { title: "선형 회귀 Y = XW + b, L = ½·mean((Y − T)²) 의 W 기울기는?",
    options: ["X @ (Y − T) / N", "X.T @ (Y − T) / N", "(Y − T) @ X.T / N", "X.T @ (T − Y) / N"],
    answer: 1,
    why: "½ 이 미분의 2 를 상쇄합니다. 샘플 하나로 보면 (y − t)x — '(예측 − 정답) × 입력'. shape (1, N) @ (N, 1) = (1, 1) 로 W 와 같습니다.",
    anim: "D2-8" },
  { title: "Day 2 선형 회귀 데이터에서 학습률을 lr 1.0 → 2.0 으로 올리면?",
    options: ["lr 0.2 일 때보다 덜 간다", "발산한다 (안정 상한 ≈ 1.54)", "더 빨리 W ≈ 2.05 에 수렴한다", "lr 1.0 과 결과가 똑같다"],
    answer: 1,
    why: "lr 1.0 은 200 epoch 에 W ≈ 2.05, b ≈ 0.98 로 수렴, lr 0.2 는 덜 감, 상한 ≈ 1.54 를 넘으면 발산합니다.",
    anim: "D2-8" },
  { title: "로지스틱 회귀 P = sigmoid(XW + b) 에 BCE 손실을 쓰면 dW 는?",
    options: ["X.T @ (T − P) / N", "X.T @ (P − T) / N", "X.T @ log(P) / N", "X.T @ (P − T) * P * (1 − P) / N"],
    answer: 1,
    why: "sigmoid 의 미분 P(1 − P) 와 BCE 의 미분이 상쇄되어 선형 회귀와 똑같은 (y − t)x 모양이 됩니다. Day 3 SoftmaxWithLoss 의 (Y − T)/N 도 같은 원리.",
    anim: "D2-9" },
  { title: "BCE = −mean(T·log P + (1 − T)·log(1 − P)) 에서 정답 T = 0 인 샘플의 손실은?",
    options: ["P · log P", "−log P", "−log(1 − P)", "−(1 − P)"],
    answer: 2,
    why: "T = 0 이면 앞 항이 사라집니다. P 가 1 에 가까울수록(확신하며 틀릴수록) 큰 벌점 — CE 의 2클래스 버전.",
    anim: "D2-9" },
  { title: "lr 0.5, 300 step 학습 결과 W ≈ [3.30, 3.41], b ≈ 0.31, 정확도 0.995 였다. 결정 경계는?",
    options: ["x1 = x2 를 지나는 원점 대칭 곡선", "3.30·x1 + 3.41·x2 = 0.995 인 직선", "P = 1 이 되는 점들을 잇는 곡선", "3.30·x1 + 3.41·x2 + 0.31 = 0 인 직선"],
    answer: 3,
    why: "P = 0.5 ⇔ sigmoid 입력 = 0 ⇔ x1·W1 + x2·W2 + b = 0. sigmoid 를 써도 경계는 직선입니다.",
    anim: "D2-9" }
];
