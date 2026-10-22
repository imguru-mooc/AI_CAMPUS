/**
 * Neural Network Day 3 복습 퀴즈 — Google Form 자동 생성 스크립트
 * HYUNDAI AI Insight Campus 온디바이스 AI · Neural Network Day 4 (10/23) 09:00~09:15
 *
 * 사용 방법
 * 1. https://script.google.com 접속 → 새 프로젝트
 * 2. 이 파일 전체를 붙여 넣고 저장
 * 3. 상단 함수 선택에서 createNNDay3Quiz 선택 → ▶ 실행 (최초 1회 권한 승인)
 * 4. 하단 실행 로그에 출력되는 두 URL 확인
 *    - 편집용(editUrl): 강사용, 응답 확인·설정 변경
 *    - 응답용(publishedUrl): 학생에게 공유
 *
 * 생성되는 폼 설정
 * - 퀴즈 모드(자동 채점, 문항당 1점, 총 20점)
 * - 이름 필수 입력, 문항 전부 필수
 * - 제출 직후 점수 공개, 정답·해설·복습할 애니메이션 링크 표시
 * - 문항 순서 고정(Day 3 교시 순), 보기 순서도 고정
 */

function createNNDay3Quiz() {
  const form = FormApp.create('Neural Network Day 3 복습 퀴즈');
  form.setIsQuiz(true)
      .setDescription(
        '범위: Day 3 전체 (수치 미분의 한계 · 계산 그래프 · 연쇄 법칙 · Relu · Sigmoid · Affine · SoftmaxWithLoss · gradient check · 미니배치 SGD · MNIST)\n' +
        '문항당 1점, 총 20점 · 권장 시간 15분\n' +
        '제출 후 점수와 해설이 바로 표시됩니다. 틀린 문항은 해설의 애니메이션 링크로 복습하세요.\n' +
        '애니메이션: https://imguru-mooc.github.io/AI_CAMPUS/NN-day3/animation.html')
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
    const review = q.anim ? '  [복습: ' + q.anim + ' ' + ANIM_URL + '#' + q.anim.toLowerCase() + ']' : '';
    item.setFeedbackForCorrect(
      FormApp.createFeedback().setText('정답입니다. ' + q.why + review).build());
    item.setFeedbackForIncorrect(
      FormApp.createFeedback().setText('정답: ' + LABEL[q.answer] + ' ' + q.options[q.answer] + ' — ' + q.why + review).build());
  });

  Logger.log('편집용  URL: ' + form.getEditUrl());
  Logger.log('응답용  URL: ' + form.getPublishedUrl());
}

var LABEL = ['①', '②', '③', '④'];
var ANIM_URL = 'https://imguru-mooc.github.io/AI_CAMPUS/NN-day3/animation.html';

var QUESTIONS = [
  { title: "784-100-10 망(P = 79,510)을 수치 미분(중앙 차분)으로 학습하면 step 당 손실 계산 횟수는?",
    options: ["159,020번 (2P)", "2번 (순전파 + 역방향)", "318,040번 (4P)", "79,510번 (P)"],
    answer: 0,
    why: "파라미터 하나마다 f(x + h), f(x − h) 두 번 → 2P = 159,020번. 서버에서 step 당 약 69 초, 역전파는 약 2 ms 입니다.",
    anim: "D3-1" },
  { title: "수치 미분 한 step 의 전체 비용이 P² 에 비례하는 이유는?",
    options: ["가중치 행렬 W 의 원소 수가 정확히 P² 개라서", "중앙 차분 h 때문에 오차가 P² 배로 커져서", "손실 2P 번 × 순전파 한 번도 P 에 비례", "파라미터마다 2차 미분까지 함께 계산해야 해서"],
    answer: 2,
    why: "(손실 계산 2P 번) × (순전파 한 번 ∝ P) → ∝ P². 역전파는 순전파 1번 + 역방향 1번(≈ 순전파 2~3배)이면 끝납니다.",
    anim: "D3-1" },
  { title: "사과 100원 × 2개, 귤 150원 × 3개, 소비세 1.1 의 계산 그래프. 최종 금액과 '사과 가격'에 대한 기울기는?",
    options: ["715원, 1.1", "715원, 110", "715원, 2.2", "650원, 2.2"],
    answer: 2,
    why: "(200 + 450) × 1.1 = 715. 사과 가격 → ×2 → +귤 → ×1.1 이므로 기울기 2 × 1.1 = 2.2 — 사과 가격이 1원 오르면 최종 금액이 2.2원 오릅니다.",
    anim: "D3-2" },
  { title: "같은 그래프를 dout = 1 로 역전파할 때 'd사과개수'와 'd소비세'는?",
    options: ["110, 715", "110, 650", "100, 650", "2.2, 715"],
    answer: 1,
    why: "d사과개수 = 사과 가격 100 × 1.1 = 110, d소비세 = 소비세를 곱하기 전 합계 650. 역방향 한 번으로 모든 입력의 기울기가 동시에 나옵니다.",
    anim: "D3-2" },
  { title: "z = t², t = x + y, x = 3, y = 1 일 때 연쇄 법칙으로 구한 ∂z/∂x 는?",
    options: ["16", "4", "6", "8"],
    answer: 3,
    why: "t = 4, ∂z/∂t = 2t = 8, ∂t/∂x = 1 → ∂z/∂x = 8 × 1 = 8 (∂z/∂y 도 8). 합성 함수의 미분 = 국소 미분의 곱.",
    anim: "D3-3" },
  { title: "덧셈 노드와 곱셈 노드의 backward 규칙을 바르게 짝지은 것은?",
    options: ["덧셈: dout 그대로 양쪽 · 곱셈: dout × 자기 입력", "덧셈: dout × 상대편 입력 · 곱셈: dout 그대로 양쪽", "덧셈: dout 그대로 양쪽 · 곱셈: dout × 상대편 입력", "덧셈: dout 을 반씩 나눔 · 곱셈: dout × 자기 입력"],
    answer: 2,
    why: "z = x + y → dx = dy = dout · 1. z = x·y → dx = dout · y, dy = dout · x (서로 바꿔 곱함). 그래서 MulLayer 는 forward 때 x, y 를 저장합니다.",
    anim: "D3-3" },
  { title: "Relu 계층에 X = [[1.0, −0.5], [−2.0, 3.0]] 을 forward 한 뒤 backward(ones) 의 결과는?",
    options: ["[[1, 0], [0, 3]]", "[[0, 1], [1, 0]]", "[[1, 0], [0, 1]]", "[[1, 1], [1, 1]]"],
    answer: 2,
    why: "forward 에서 mask = (X <= 0) = [[F, T], [T, F]] 를 저장하고 backward 에서 mask 자리를 0 으로. [[1, 0], [0, 3]] 은 forward 출력입니다.",
    anim: "D3-4" },
  { title: "Sigmoid 계층이 forward 때 저장하는 값과 backward 식은?",
    options: ["출력 out 저장, dout * out * (1 − out)", "입력 x 저장, dout * x * (1 − x)", "mask = (x <= 0) 저장, mask 자리를 0 으로", "출력 out 저장, dout * out * (1 + out)"],
    answer: 0,
    why: "Day 2 의 sigmoid 도함수 s(1 − s) 를 x 로 다시 계산하지 않고 저장한 out 으로 씁니다. 예: x = [1.0, −2.0] → out(1 − out) = [0.1966, 0.1050], 최대 0.25.",
    anim: "D3-4" },
  { title: "Affine 계층 Y = XW + b 의 backward 로 옳은 것은?",
    options: ["dX = W.T @ dout,  dW = dout @ X.T", "dX = dout @ W,  dW = X @ dout", "dX = dout @ W.T,  dW = dout @ X.T", "dX = dout @ W.T,  dW = X.T @ dout"],
    answer: 3,
    why: "X (N,D), W (D,M), dout (N,M) → dX = (N,M)@(M,D) = (N,D), dW = (D,N)@(N,M) = (D,M), db = dout.sum(axis=0) (M,). shape 를 맞추면 식이 하나로 정해집니다.",
    anim: "D3-5" },
  { title: "MNIST 1층 (배치 N = 100): X (100, 784), W1 (784, 100), dout (100, 100). dW1 과 db1 의 shape 은?",
    options: ["(784, 100), (100,)", "(784, 100), (100, 100)", "(100, 100), (784,)", "(100, 784), (100,)"],
    answer: 0,
    why: "dW1 = X.T (784, 100) @ dout (100, 100) = (784, 100) — W1 과 같은 shape. db1 = dout 을 행 방향(axis=0)으로 합한 (100,). 배치 크기와 은닉 크기가 둘 다 100 이라 헷갈리기 쉬운 예.",
    anim: "D3-5" },
  { title: "x = [[0.3, 2.9, 4.0]], t = [2] 일 때 SoftmaxWithLoss 의 손실과 정답 자리의 backward 값은?",
    options: ["0.3057, 0.7366", "0.3057, −0.2634", "0.7366, −0.2634", "0.2634, −0.3057"],
    answer: 1,
    why: "softmax y = [0.0182, 0.2452, 0.7366], 손실 = −log 0.7366 = 0.3057. backward = (y − t)/N = [0.0182, 0.2452, 0.7366 − 1 = −0.2634].",
    anim: "D3-6" },
  { title: "Affine 의 dW = X.T @ dout 에 SoftmaxWithLoss 의 dout = (Y − T)/N 을 넣으면?",
    options: ["N 으로 나누지 않는 Xᵀ(Y − T) 인 식", "Day 2 의 dW = Xᵀ(Y − T)/N 과 같은 식", "Day 2 식에 Y(1 − Y) 가 더 곱해진 식", "Day 2 식과 부호가 반대인 Xᵀ(T − Y)/N"],
    answer: 1,
    why: "Day 2 로지스틱 · 선형 회귀의 '(예측 − 정답) × 입력' 이 계층 두 개(SoftmaxWithLoss → Affine)의 연쇄 법칙으로 똑같이 나옵니다.",
    anim: "D3-6" },
  { title: "TwoLayerNet 이 계층을 OrderedDict 에 담고 gradient() 에서 reversed 로 도는 이유는?",
    options: ["추가한 순서를 기억해 backward 를 정확히 역순으로 하려고", "계층마다 서로 다른 학습률을 따로따로 적용하려고", "계층 이름을 알파벳순으로 정렬해 이름으로 빨리 찾으려고", "역순으로 돌아야 계산 그래프 메모리를 절반만 쓰므로"],
    answer: 0,
    why: "forward 는 Affine1 → Relu1 → Affine2, backward 는 lastLayer.backward(1) 다음 Affine2 → Relu1 → Affine1. 순서가 보장되는 OrderedDict 를 reversed(list(...)) 로 돕니다.",
    anim: "D3-7" },
  { title: "rel_error = ‖a − b‖ / (‖a‖ + ‖b‖ + 1e-12) 로 gradient check 할 때 판정 기준은?",
    options: ["< 1e-2 통과, 1e-2 ~ 1e-1 의심, > 1e-1 버그", "< 1e-7 통과, 1e-7 ~ 1e-4 의심, > 1e-2 버그", "< 1e-12 통과, 1e-12 ~ 1e-7 의심, > 1e-7 버그", "< 1e-4 통과, 1e-4 ~ 1e-2 의심, > 1 버그"],
    answer: 1,
    why: "784-10-10 · MNIST 3장 예: W1 2.1e-10, b2 3.9e-10 → 모두 1e-7 미만이라 통과.",
    anim: "D3-7" },
  { title: "cross-entropy 의 log 에 1e-12 대신 Day 2 처럼 1e-7 을 더하면 gradient check 결과는?",
    options: ["W1 만 상대 오차가 0.5 가 되어 버그로 판정된다", "모든 파라미터 상대 오차가 ≈ 5e-7 로 일정하게 커진다", "상대 오차가 1e-12 아래로 오히려 더 작아진다", "log(0) 이 생겨 모든 파라미터 상대 오차가 nan 이 된다"],
    answer: 1,
    why: "버그가 아니라 손실 식이 미세하게 달라진 것입니다(역전파의 (Y − T)/N 은 1e-7 을 모름). 그래서 Day 3 부터 1e-12 를 씁니다.",
    anim: "D3-7" },
  { title: "버그 ①: Affine backward 에 db = dout.mean(axis=0) 을 넣고 gradient check 를 하면?",
    options: ["평균도 합에 비례하니 1e-7 미만으로 통과한다", "W1 · b1 · W2 · b2 모두 상대 오차가 0.5 다", "shape 이 (M,) 가 아니라서 실행 중 오류가 난다", "b2 만 상대 오차 0.5, 오류 없이 실행된다"],
    answer: 3,
    why: "mean 은 sum 을 N(= 3)으로 나눈 값 → (1 − 1/3)/(1 + 1/3) = 0.5. shape 는 (M,) 로 같아서 오류 메시지가 나지 않습니다.",
    anim: "D3-7" },
  { title: "버그 ②: dX = dout @ W.T 에서 .T 를 빼먹었는데 784-10-10 망에서 오류 없이 실행됐다. 이유와 결과는?",
    options: ["NumPy 가 자동으로 전치해 주어 기울기가 모두 맞다", "W2 가 (10, 10) 이라 shape 가 맞고, W2 · b2 가 틀린다", "브로드캐스팅으로 shape 가 맞고, 기울기가 모두 틀린다", "W2 가 (10, 10) 이라 shape 가 맞고, W1 · b1 이 틀린다"],
    answer: 3,
    why: "dX 는 앞 층으로 내려가는 기울기라 앞쪽 층만 틀립니다 (W1 0.63, b1 0.56). shape 가 맞아도 값은 틀릴 수 있다 → 새 계층마다 gradient check.",
    anim: "D3-7" },
  { title: "나선 데이터 train 240점 · batch 30, MNIST 60,000장 · batch 100 의 iters_per_epoch 는 각각?",
    options: ["80, 600", "30, 100", "8, 600", "8, 6,000"],
    answer: 2,
    why: "iters_per_epoch = N // batch_size → 240 // 30 = 8 (300 epoch 면 2,400 iteration), 60,000 // 100 = 600.",
    anim: "D3-8" },
  { title: "Day 3 미니배치 SGD 학습 루프에 대한 설명으로 옳은 것은?",
    options: ["섞지 않고 순서대로 자르고, params 를 제자리(-=) 갱신", "매 iteration 섞고, params[k] = params[k] − lr * g 로 갱신", "처음 한 번만 섞고, 매 step 새 params 딕셔너리를 만들어 갱신", "매 epoch rng.permutation(N) 으로 섞고, params 를 제자리(-=) 갱신"],
    answer: 3,
    why: "계층(Affine)이 params 의 배열을 그대로 참조하므로 net.params[k] -= lr * g 처럼 제자리로 바꿔야 계층에 반영됩니다. 새 배열을 대입하면 연결이 끊깁니다.",
    anim: "D3-8" },
  { title: "Day 3 MNIST 784-100-10 (std 0.01, SGD lr 0.1, batch 100) 학습에 대한 설명으로 옳은 것은?",
    options: ["첫 손실 ≈ ln 10, 10 epoch test 0.9694, int8 로도 0.9692", "첫 손실 ≈ 1.0, 10 epoch test 0.9531, int8 로도 0.9692", "첫 손실 ≈ ln 10, 10 epoch test 0.9694, int8 로는 0.81", "첫 손실 ≈ ln 2, 10 epoch test 0.9694, int8 로도 0.9692"],
    answer: 0,
    why: "가중치가 작아 학습 전 확률이 거의 균등 → 손실 ≈ ln 10 = 2.3026. 10 epoch(6,000 iteration) test 0.9694 (5 epoch 0.9531), int8 양자화 가중치로도 0.9692.",
    anim: "D3-9" }
];
