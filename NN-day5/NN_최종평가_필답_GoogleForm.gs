/**
 * Neural Network 최종 평가 (필답) — Google Form 자동 생성 스크립트
 * HYUNDAI AI Insight Campus 온디바이스 AI · Neural Network Day 5 (10/26) 17:00~17:25
 *
 * 사용 방법
 * 1. https://script.google.com 접속 → 새 프로젝트
 * 2. 이 파일 전체를 붙여 넣고 저장
 * 3. 상단 함수 선택에서 createNNFinalExam 선택 → ▶ 실행 (최초 1회 권한 승인)
 * 4. 하단 실행 로그에 출력되는 두 URL 확인
 *    - 편집용(editUrl): 강사용, 응답 · 점수 확인, 스프레드시트 연결
 *    - 응답용(publishedUrl): 학생에게 공유
 *
 * 생성되는 폼 설정
 * - 퀴즈 모드(자동 채점), 30문항 × 1점 = 30점 → 과정 성적 30% 환산은 강사가 한다
 * - Part A 객관식 20 (Day 1~5 각 4문항) + Part B 계산 · 유도 10 (shape · 파라미터 수 · 역전파 빈칸)
 *   Part B 는 FormApp 의 단답형(TextItem)에 정답 키를 넣을 수 없어 숫자 · 식 4지선다로 자동 채점한다
 * - 이름 · 학번 필수, 문항 전부 필수, 문항 순서 무작위(응답자마다 다름)
 * - 제출 직후 점수 공개, 문항별 정답 · 해설 · 복습 애니메이션 링크 표시
 */

function createNNFinalExam() {
  const form = FormApp.create('Neural Network 최종 평가 (필답)');
  form.setIsQuiz(true)
      .setDescription(
        '범위: Neural Network 5일 전체 (Day 1 NumPy · 미분 · 경사하강 / Day 2 순전파 · 손실 / Day 3 역전파 / Day 4 학습 기법 / Day 5 CNN)\n' +
        '구성: Part A 객관식 20문항 + Part B 계산 · 유도 10문항 (shape 계산 · 파라미터 수 · 역전파 유도 빈칸) — 모두 4지선다\n' +
        '배점: 문항당 1점, 총 30점. 과정 성적의 30% 이며, 30% 환산은 강사가 합니다.\n' +
        '시간: 25분 · 강의 자료 · 노트북 · 검색 참고 없이 풉니다. 문항 순서는 사람마다 다릅니다.\n' +
        '제출 직후 점수와 문항별 해설을 확인할 수 있습니다.')
      .setShuffleQuestions(true)
      .setProgressBar(true);
  // 조직 계정 배포 시 1인 1회 제한을 쓰려면 주석 해제 (Google 로그인 필요)
  // form.setLimitOneResponsePerUser(true);
  // form.setCollectEmail(true);

  form.addTextItem().setTitle('이름').setRequired(true);
  form.addTextItem().setTitle('학번').setHelpText('수강생 번호(학번)를 입력하세요').setRequired(true);
  form.addPageBreakItem().setTitle('문항 (30) — 각 1점')
      .setHelpText('[Day N] 은 객관식, [shape 계산] · [파라미터 수] · [역전파 유도] 는 계산 · 유도 문항입니다.');

  QUESTIONS.forEach(function (q) {
    const item = form.addMultipleChoiceItem();
    item.setTitle('[' + q.tag + '] ' + q.title)
        .setPoints(1)
        .setRequired(true);
    item.setChoices(q.options.map(function (opt, j) {
      return item.createChoice(opt, j === q.answer);
    }));
    const review = '  [복습: ' + q.anim + ' ' + q.url + ']';
    item.setFeedbackForCorrect(
      FormApp.createFeedback().setText('정답입니다. ' + q.why + review).build());
    item.setFeedbackForIncorrect(
      FormApp.createFeedback().setText('정답: ' + LABEL[q.answer] + ' ' + q.options[q.answer] + ' — ' + q.why + review).build());
  });

  Logger.log('편집용  URL: ' + form.getEditUrl());
  Logger.log('응답용  URL: ' + form.getPublishedUrl());
}

var LABEL = ['①', '②', '③', '④'];

var QUESTIONS = [
  { part: "A", tag: "Day 1",
    title: "A = np.zeros((3, 4)) 일 때 A.sum(axis=1, keepdims=True) 의 shape 은?",
    options: ["(3, 1)", "(3,)", "(4,)", "(1, 4)"],
    answer: 0,
    why: "axis=1(열 방향)을 없애 행마다 합 → (3,), keepdims=True 면 줄인 축을 크기 1 로 남겨 (3, 1). (3, 1) 은 (3, 4) 와 다시 브로드캐스팅되므로 Day 2 softmax 의 최댓값 빼기에 씁니다.",
    anim: "D1-4", url: "https://imguru-mooc.github.io/AI_CAMPUS/NN-day1/animation.html#d1-4" },
  { part: "A", tag: "Day 1",
    title: "다음 중 NumPy 브로드캐스팅이 되지 않아 오류가 나는 연산은?",
    options: ["(3, 4) + (1, 4)", "(3, 4) + (4,)", "(3, 1) + (1, 4)", "(3, 4) + (3,)"],
    answer: 3,
    why: "shape 를 오른쪽 끝부터 비교해 같거나 한쪽이 1 이어야 합니다. (3, 4) + (3,) 은 4 와 3 이 달라 오류. (3, 4) + (4,) 는 모든 행에 더하기 — 신경망의 '+ 편향'.",
    anim: "D1-3", url: "https://imguru-mooc.github.io/AI_CAMPUS/NN-day1/animation.html#d1-3" },
  { part: "A", tag: "Day 1",
    title: "Day 1 에서 구현한 수치 미분(중앙 차분) 식과 기본 h 로 옳은 것은?",
    options: ["(f(x + h) − f(x − h)) / h,  h = 1e-4", "(f(x + h) − f(x)) / h,  h = 1e-50", "(f(x + h) − f(x − h)) / (2h),  h = 1e-50", "(f(x + h) − f(x − h)) / (2h),  h = 1e-4"],
    answer: 3,
    why: "중앙 차분은 전진 차분보다 오차가 작습니다. h 를 1e-50 처럼 너무 작게 하면 부동소수점 반올림으로 0 이 되어(np.float32(1e-50) = 0.0) 오히려 틀립니다.",
    anim: "D1-7", url: "https://imguru-mooc.github.io/AI_CAMPUS/NN-day1/animation.html#d1-7" },
  { part: "A", tag: "Day 1",
    title: "f(x, y) = x² + 3y² 에 경사하강 p ← p − η∇f(p) 를 쓸 때, 발산하지 않고 수렴하는 학습률 범위는?",
    options: ["0 < η < 1", "0 < η < 2/3", "0 < η < 1/3", "0 < η < 1/6"],
    answer: 2,
    why: "∇f = (2x, 6y) → y ← (1 − 6η)y, |1 − 6η| < 1 ⇒ η < 1/3 (x 방향 조건 η < 1 보다 엄격). η = 0.3 은 −0.8배 지그재그로 수렴, 0.35 는 −1.1배로 발산 — 가장 가파른 방향이 학습률 상한을 정합니다.",
    anim: "D1-8", url: "https://imguru-mooc.github.io/AI_CAMPUS/NN-day1/animation.html#d1-8" },
  { part: "A", tag: "Day 2",
    title: "단층 퍼셉트론이 XOR 을 풀지 못하는 이유와 해결 방법으로 옳은 것은?",
    options: ["계단 함수를 씀 → sigmoid 로 바꾼 단층 망 사용", "직선 하나로 분리 불가 → AND(NAND, OR) 2층", "가중치가 정수가 아님 → 학습률을 더 크게 설정", "편향 b 가 없음 → b 를 추가한 단층 퍼셉트론"],
    answer: 1,
    why: "XOR 은 선형 분리가 안 됩니다 (학습 규칙 W ← W − η(y − t)x 로 50 epoch 돌려도 정확도 0.5). 은닉층 NAND · OR 이 공간을 바꿔 (0,0)→(1,0), (1,1)→(0,1) 등으로 옮기면 AND 한 번으로 분리됩니다.",
    anim: "D2-3", url: "https://imguru-mooc.github.io/AI_CAMPUS/NN-day2/animation.html#d2-3" },
  { part: "A", tag: "Day 2",
    title: "sigmoid 함수 s(x) 의 도함수와 그 최댓값으로 옳은 것은?",
    options: ["s(1 + s), 최댓값 0.50 (x = 0)", "s(1 − s), 최댓값 1.0 (x → ∞)", "s(1 − s), 최댓값 0.25 (x = 0)", "1 − s², 최댓값 1 (x = 0)"],
    answer: 2,
    why: "sigmoid' = s(1 − s), x = 0 에서 0.5 × 0.5 = 0.25 가 최대. 층마다 곱해지면 0.25¹⁰ ≈ 9.5e-7 — 기울기 소실. 1 − t² 은 tanh 의 도함수입니다.",
    anim: "D2-4", url: "https://imguru-mooc.github.io/AI_CAMPUS/NN-day2/animation.html#d2-4" },
  { part: "A", tag: "Day 2",
    title: "softmax([1010, 1000, 990]) 를 그대로 계산하면 nan 이 된다. 수치 안정화 방법은?",
    options: ["exp(x) 에 1e-7 을 더한 뒤 합으로 나누기", "x − x.max(axis=1, keepdims=True) 를 한 뒤 exp", "x − x.min(axis=1, keepdims=True) 를 한 뒤 exp", "x / x.sum(axis=1, keepdims=True) 를 한 뒤 exp"],
    answer: 1,
    why: "최댓값을 빼도 softmax 값은 같고 exp 의 최대 입력이 0 이 되어 overflow 가 없습니다 → [9.9995e-01, 4.5398e-05, 2.0611e-09]. 최솟값을 빼면 exp(20) 처럼 오히려 커집니다.",
    anim: "D2-6", url: "https://imguru-mooc.github.io/AI_CAMPUS/NN-day2/animation.html#d2-6" },
  { part: "A", tag: "Day 2",
    title: "선형 회귀 Y = XW + b, 손실 L = ½·mean((Y − T)²) 의 기울기로 옳은 것은? (N = 샘플 수)",
    options: ["dW = (Y − T) @ X.T / N,  db = mean(Y − T)", "dW = 2 · X.T @ (Y − T),  db = sum(Y − T)", "dW = X.T @ (Y − T) / N,  db = mean(Y − T)", "dW = X.T @ (T − Y) / N,  db = mean(T − Y)"],
    answer: 2,
    why: "½ 이 미분의 2 를 상쇄 → 샘플 하나로 보면 (y − t)x. 로지스틱 회귀 dW = X.T @ (P − T)/N 도 같은 모양이고, Day 3 에서 Affine dW = X.T @ dout 에 dout = (Y − T)/N 을 넣은 것과 같습니다. lr 1.0, 200 epoch → W ≈ 2.0523, b ≈ 0.9757.",
    anim: "D2-8", url: "https://imguru-mooc.github.io/AI_CAMPUS/NN-day2/animation.html#d2-8" },
  { part: "A", tag: "Day 3",
    title: "사과 100원 × 2개, 귤 150원 × 3개, 소비세 1.1 의 계산 그래프를 dout = 1 로 역전파했을 때 '귤 가격' 의 기울기는?",
    options: ["1.1", "3", "3.3", "165"],
    answer: 2,
    why: "귤 가격 → ×3 → +사과 → ×1.1 이므로 3 × 1.1 = 3.3. 165 는 '귤 개수' 의 기울기 (150 × 1.1), 1.1 은 합계 노드의 기울기. 최종 금액 715, 사과 가격 기울기 2.2.",
    anim: "D3-2", url: "https://imguru-mooc.github.io/AI_CAMPUS/NN-day3/animation.html#d3-2" },
  { part: "A", tag: "Day 3",
    title: "784-10-10 TwoLayerNet 의 Affine backward 에 dX = dout @ W (전치 누락) 버그를 넣었는데 오류 없이 실행됐다. 이유와 gradient check 결과는?",
    options: ["W2 가 (10, 10) 이라 shape 가 맞음 → W2 · b2 만 틀림", "W2 가 (10, 10) 이라 shape 가 맞음 → W1 · b1 만 틀림", "브로드캐스팅으로 shape 가 맞음 → 모든 기울기가 틀림", "NumPy 가 자동 전치를 해 줌 → 모든 기울기가 맞음"],
    answer: 1,
    why: "dX 는 앞 층으로 내려가는 기울기라 앞쪽 층만 틀립니다 (W1 0.63, b1 0.56, 판정 기준 < 1e-7 통과 · > 1e-2 버그). shape 가 맞아도 값이 틀릴 수 있다 → 새 계층마다 gradient check.",
    anim: "D3-7", url: "https://imguru-mooc.github.io/AI_CAMPUS/NN-day3/animation.html#d3-7" },
  { part: "A", tag: "Day 3",
    title: "MNIST 60,000장을 batch 100 으로 학습할 때 iters_per_epoch 와 10 epoch 의 총 iteration 수는?",
    options: ["6,000, 60,000", "600, 6,000", "100, 1,000", "600, 60,000"],
    answer: 1,
    why: "iters_per_epoch = N // batch_size = 60,000 // 100 = 600, 10 epoch = 6,000 iteration. 매 epoch rng.permutation(N) 으로 섞고 params 를 제자리(-=) 갱신합니다.",
    anim: "D3-8", url: "https://imguru-mooc.github.io/AI_CAMPUS/NN-day3/animation.html#d3-8" },
  { part: "A", tag: "Day 3",
    title: "Day 3 MNIST 784-100-10 (가중치 std 0.01) 의 학습 전 첫 손실이 ≈ 2.3026 (= ln 10) 인 이유는?",
    options: ["softmax 가 최댓값을 빼서 손실이 ln 10 으로 고정돼서", "cross-entropy 에 더한 1e-12 가 손실을 지배해서", "가중치가 작아 10 클래스 확률이 거의 균등해서", "레이블 0~9 의 평균 5 에 log 를 취한 값이라서"],
    answer: 2,
    why: "모든 확률 ≈ 0.1 → −log 0.1 = ln 10 = 2.3026. 첫 손실이 이 값 근처면 초기화 · 손실 함수가 제대로 연결된 것입니다. 10 epoch 후 test 0.9694.",
    anim: "D3-9", url: "https://imguru-mooc.github.io/AI_CAMPUS/NN-day3/animation.html#d3-9" },
  { part: "A", tag: "Day 4",
    title: "Adam (β1 = 0.9) 에서 m 을 0 으로 시작해 첫 step(t = 1) 에 기울기 g 를 받으면, 편향 보정한 m̂ = m / (1 − β1ᵗ) 은?",
    options: ["g", "0.9·g", "0.1·g", "10·g"],
    answer: 0,
    why: "m = 0.9·0 + 0.1·g = 0.1·g, 1 − 0.9¹ = 0.1 로 나누면 m̂ = g. 보정이 없으면 초반 m, v 가 0 쪽으로 치우칩니다. t 가 커지면 분모 → 1.",
    anim: "D4-2", url: "https://imguru-mooc.github.io/AI_CAMPUS/NN-day4/animation.html#d4-2" },
  { part: "A", tag: "Day 4",
    title: "Affine(784, 100) 뒤에 ReLU 를 쓸 때 He 초기화의 표준편차는?",
    options: ["√(2/100) ≈ 0.1414", "√(2/784) ≈ 0.0505", "√(1/784) ≈ 0.0357", "0.01 (고정)"],
    answer: 1,
    why: "He = √(2/n_in), n_in = 앞 층 노드 수 784. √(1/n_in) 은 sigmoid · tanh 용 Xavier. MNIST 4층 SGD 0.01 2000 iteration: std 0.01 0.1135 · Xavier 0.9179 · He 0.9378. Day 5 Convolution 은 n_in = C·FH·FW.",
    anim: "D4-3", url: "https://imguru-mooc.github.io/AI_CAMPUS/NN-day4/animation.html#d4-3" },
  { part: "A", tag: "Day 4",
    title: "학습 중 과적합(overfitting)이 일어나고 있다는 신호로 가장 알맞은 것은?",
    options: ["train 정확도는 오르는데 test 손실이 다시 증가", "validation 정확도가 test 정확도보다 조금 높음", "train 정확도가 0.1 근처에서 전혀 오르지 않음", "train 손실과 test 손실이 함께 꾸준히 감소"],
    answer: 0,
    why: "300장 · 200 epoch 실험: train 1.000 / test 0.786, test 손실 0.79 → 1.12 증가 (틀린 답을 과신). 처방: 데이터 ↑ (3000장 → 0.926), weight decay (λ 0.01 → 0.810), Dropout (0.2 → 0.800).",
    anim: "D4-5", url: "https://imguru-mooc.github.io/AI_CAMPUS/NN-day4/animation.html#d4-5" },
  { part: "A", tag: "Day 4",
    title: "hyperparameter (lr, λ) 를 test 데이터로 고르면 안 되는 이유는?",
    options: ["test 에 맞춰 고르게 되어 test 성능이 과대평가됨", "test 로 고르면 학습 시간이 두 배 이상으로 늘어남", "test 데이터는 레이블이 없어 정확도를 구할 수 없음", "test 는 크기가 작아 로그 스케일 탐색이 안 됨"],
    answer: 0,
    why: "고르는 데는 validation (1000장 중 p[:200]), 보고는 test 한 번. 30회 로그 스케일 랜덤 탐색 lr = 10^U(−4, 0) → 1위 lr 0.242, val 0.885.",
    anim: "D4-7", url: "https://imguru-mooc.github.io/AI_CAMPUS/NN-day4/animation.html#d4-7" },
  { part: "A", tag: "Day 5",
    title: "MNIST 이미지의 픽셀 순서를 모두 같은 방식으로 뒤섞어도 MLP 정확도가 같았다 (원본 0.9436, 뒤섞음 0.9455). 이 실험이 보여 주는 것은?",
    options: ["Adam 이 뒤섞인 픽셀 순서를 스스로 복원한다", "뒤섞으면 데이터가 늘어나 성능이 조금 오른다", "MLP 는 이미 이동 불변성을 갖추고 있다", "MLP 는 픽셀 사이의 공간 관계를 쓰지 않는다"],
    answer: 3,
    why: "784 로 펼치면 이웃 관계가 사라지므로 순서를 바꿔도 MLP 에게는 같은 문제입니다. 합성곱은 지역성 · 가중치 공유로 공간 구조를 씁니다.",
    anim: "D5-1", url: "https://imguru-mooc.github.io/AI_CAMPUS/NN-day5/animation.html#d5-1" },
  { part: "A", tag: "Day 5",
    title: "Max Pooling (2×2, stride 2) 계층의 backward 동작으로 옳은 것은?",
    options: ["저장한 argmax 자리로만 dout 을 보낸다", "영역 4칸에 dout / 4 씩 똑같이 나누어 보낸다", "영역 4칸 모두에 dout 을 그대로 복사해 보낸다", "argmax 자리만 0 으로, 나머지 3칸에 보낸다"],
    answer: 0,
    why: "최댓값을 만든 입력만 출력에 영향을 주므로 그 자리로만 기울기가 갑니다 (ReLU mask 와 비슷). Pooling 은 파라미터가 없습니다. dout / 4 는 Average Pooling.",
    anim: "D5-5", url: "https://imguru-mooc.github.io/AI_CAMPUS/NN-day5/animation.html#d5-5" },
  { part: "A", tag: "Day 5",
    title: "합성곱을 4중 for 루프 대신 im2col + 행렬곱으로 구현할 때의 특징은?",
    options: ["stride 1 · pad 0 인 합성곱에서만 쓸 수 있다", "결과가 근사값이라 정확도가 조금 떨어진다", "메모리도 줄고 속도도 빨라져 손해가 전혀 없다", "메모리를 더 쓰는 대신 행렬곱 하나로 훨씬 빠르다"],
    answer: 3,
    why: "MNIST 배치 100, 5×5 필터 30개: col (57,600, 25) = 입력의 18.4배 메모리, 속도 6.9 초 → 0.039 초 (약 177배), 결과는 allclose 로 동일.",
    anim: "D5-4", url: "https://imguru-mooc.github.io/AI_CAMPUS/NN-day5/animation.html#d5-4" },
  { part: "A", tag: "Day 5",
    title: "test 이미지를 오른쪽으로 2 픽셀 밀었을 때 (np.roll axis=3) 정확도로 옳은 것은? (원래 CNN 0.9810, MLP 0.9757)",
    options: ["CNN 0.5619, MLP 0.7484 — 둘 다 크게 떨어진다", "CNN 0.7975, MLP 0.9119 — MLP 가 덜 떨어진다", "CNN 0.9810, MLP 0.9757 — 둘 다 변화 없다", "CNN 0.9119, MLP 0.7975 — CNN 이 덜 떨어진다"],
    answer: 3,
    why: "1 · 2 · 3 픽셀: CNN 0.9734 / 0.9119 / 0.7484, Day 4 MLP 0.9551 / 0.7975 / 0.5619. 풀링 · 가중치 공유 덕분에 CNN 이 덜 떨어지지만 완벽한 불변성은 아닙니다.",
    anim: "D5-7", url: "https://imguru-mooc.github.io/AI_CAMPUS/NN-day5/animation.html#d5-7" },
  { part: "B", tag: "shape 계산",
    title: "입력 x (N, 3, 32, 32) 에 Convolution(in 3, out 16, filter 5, stride 1, pad 0) 을 적용한 출력 shape 은?",
    options: ["(N, 3, 28, 28)", "(N, 16, 32, 32)", "(N, 16, 28, 28)", "(N, 16, 27, 27)"],
    answer: 2,
    why: "OH = (H + 2P − FH) // S + 1 = (32 + 0 − 5) // 1 + 1 = 28. 출력 채널 수 = 필터 개수 16. pad 2 였다면 32 (same).",
    anim: "D5-2", url: "https://imguru-mooc.github.io/AI_CAMPUS/NN-day5/animation.html#d5-2" },
  { part: "B", tag: "shape 계산",
    title: "X (100, 784), W1 (784, 50), b1 (50,) 일 때 A1 = X @ W1 + b1 의 shape 과, Affine backward 의 dW1 = X.T @ dout 의 shape 은?",
    options: ["(100, 50),  (784, 50)", "(50, 100),  (50, 784)", "(784, 50),  (100, 784)", "(100, 50),  (100, 50)"],
    answer: 0,
    why: "(100, 784) @ (784, 50) = (100, 50), b1 은 모든 행에 브로드캐스팅. dW1 = X.T (784, 100) @ dout (100, 50) = (784, 50) — 언제나 W1 과 같은 shape.",
    anim: "D3-5", url: "https://imguru-mooc.github.io/AI_CAMPUS/NN-day3/animation.html#d3-5" },
  { part: "B", tag: "shape 계산",
    title: "MNIST 배치 x (100, 1, 28, 28) 에 5×5 필터 30개 (stride 1, pad 0) 를 쓸 때 im2col 결과 col 의 shape 은?",
    options: ["(57,600, 750)", "(57,600, 25)", "(78,400, 25)", "(57,600, 30)"],
    answer: 1,
    why: "col = (N·OH·OW, C·FH·FW) = (100·24·24, 1·5·5) = (57,600, 25). 필터 개수 30 은 col_W (25, 30) 쪽에 들어갑니다. 입력의 18.4배 메모리.",
    anim: "D5-4", url: "https://imguru-mooc.github.io/AI_CAMPUS/NN-day5/animation.html#d5-4" },
  { part: "B", tag: "파라미터 수",
    title: "MNIST 2층 MLP 784-100-10 (편향 포함) 의 파라미터 수는?",
    options: ["80,510", "79,510", "79,400", "78,500"],
    answer: 1,
    why: "784·100 + 100 + 100·10 + 10 = 79,510. 79,400 은 편향을 뺀 값, 78,500 은 1층만. 수치 미분이면 step 당 손실 2P = 159,020번 → 역전파가 필요한 이유.",
    anim: "D2-5", url: "https://imguru-mooc.github.io/AI_CAMPUS/NN-day2/animation.html#d2-5" },
  { part: "B", tag: "파라미터 수",
    title: "SimpleConvNet: Convolution(1→30, 5×5) → Relu → Pooling(2×2) → Affine(4320→100) → Relu → Affine(100→10) 의 전체 파라미터 수는?",
    options: ["433,890", "433,860", "432,100", "433,110"],
    answer: 0,
    why: "conv 30·1·5·5 + 30 = 780, affine1 4320·100 + 100 = 432,100, affine2 100·10 + 10 = 1,010 → 433,890. 433,110 은 conv 를 빠뜨린 값, 433,860 은 conv 편향 30 을 빠뜨린 값. 파라미터는 Affine 에 몰립니다.",
    anim: "D5-6", url: "https://imguru-mooc.github.io/AI_CAMPUS/NN-day5/animation.html#d5-6" },
  { part: "B", tag: "파라미터 수",
    title: "Convolution(in_channels 16, out_channels 32, filter_size 3) 계층 하나의 파라미터 수는? (편향 포함)",
    options: ["4,608", "4,624", "160", "4,640"],
    answer: 3,
    why: "W (FN, C, FH, FW) = 32·16·3·3 = 4,608, 편향 (FN,) = 32 → 4,640. 이미지 크기와 무관합니다 (가중치 공유). 160 은 Conv(1→16, 3) 의 값.",
    anim: "D5-8", url: "https://imguru-mooc.github.io/AI_CAMPUS/NN-day5/animation.html#d5-8" },
  { part: "B", tag: "역전파 유도",
    title: "곱셈 노드 z = x · y 의 backward: 위에서 dout 이 내려올 때 dx = (    ). 빈칸은?",
    options: ["dout", "dout · x", "dout · y", "dout · x · y"],
    answer: 2,
    why: "∂z/∂x = y → dx = dout · y, dy = dout · x (서로 바꿔 곱함). 그래서 MulLayer 는 forward 때 x, y 를 저장합니다. dout 그대로는 덧셈 노드.",
    anim: "D3-3", url: "https://imguru-mooc.github.io/AI_CAMPUS/NN-day3/animation.html#d3-3" },
  { part: "B", tag: "역전파 유도",
    title: "Affine 계층 Y = XW + b 의 backward: X (N, D), W (D, M), dout (N, M) 일 때 dX = (    ). 빈칸은?",
    options: ["X.T @ dout", "W.T @ dout", "dout @ W", "dout @ W.T"],
    answer: 3,
    why: "dX 는 X 와 같은 (N, D) 여야 하므로 (N, M) @ (M, D) = dout @ W.T 하나로 정해집니다. X.T @ dout 은 dW (D, M), db = dout.sum(axis=0).",
    anim: "D3-5", url: "https://imguru-mooc.github.io/AI_CAMPUS/NN-day3/animation.html#d3-5" },
  { part: "B", tag: "역전파 유도",
    title: "SoftmaxWithLoss (배치 N, 정답 one-hot T, softmax 출력 Y) 의 backward 결과 dx = (    ). 빈칸은?",
    options: ["(Y − T) / N", "(T − Y) / N", "Y(1 − Y) / N", "(Y − T) · N"],
    answer: 0,
    why: "softmax 와 cross-entropy 의 미분이 상쇄되어 '(예측 − 정답)' 만 남고, 배치 평균 손실이라 N 으로 나눕니다. 예: x = [[0.3, 2.9, 4.0]], t = [2] → [0.0182, 0.2452, −0.2634].",
    anim: "D3-6", url: "https://imguru-mooc.github.io/AI_CAMPUS/NN-day3/animation.html#d3-6" },
  { part: "B", tag: "역전파 유도",
    title: "(ㄱ) Sigmoid 계층 (출력 y 저장) 의 backward dx = (ㄱ), (ㄴ) BatchNorm 의 dβ = (ㄴ) (Σ 는 배치 방향 axis=0 합). 두 빈칸으로 옳은 것은?",
    options: ["(ㄱ) dout · y(1 − y),  (ㄴ) Σ(x̂ · dout)", "(ㄱ) dout · y(1 − y),  (ㄴ) Σdout", "(ㄱ) dout · y(1 + y),  (ㄴ) γ · Σdout", "(ㄱ) dout · (1 − y),  (ㄴ) Σdout"],
    answer: 1,
    why: "Sigmoid: dy/dx = y(1 − y) 를 저장한 출력으로 계산. BN: y = γx̂ + β 에서 β 는 더하기라 dβ = Σdout (배치 방향 합), dγ = Σ(x̂ · dout).",
    anim: "D3-4", url: "https://imguru-mooc.github.io/AI_CAMPUS/NN-day3/animation.html#d3-4" }
];
