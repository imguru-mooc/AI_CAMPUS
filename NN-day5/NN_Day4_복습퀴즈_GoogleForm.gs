/**
 * Neural Network Day 4 복습 퀴즈 — Google Form 자동 생성 스크립트
 * HYUNDAI AI Insight Campus 온디바이스 AI · Neural Network Day 5 (10/26) 09:00~09:15
 *
 * 사용 방법
 * 1. https://script.google.com 접속 → 새 프로젝트
 * 2. 이 파일 전체를 붙여 넣고 저장
 * 3. 상단 함수 선택에서 createNNDay4Quiz 선택 → ▶ 실행 (최초 1회 권한 승인)
 * 4. 하단 실행 로그에 출력되는 두 URL 확인
 *    - 편집용(editUrl): 강사용, 응답 확인·설정 변경
 *    - 응답용(publishedUrl): 학생에게 공유
 *
 * 생성되는 폼 설정
 * - 퀴즈 모드(자동 채점, 문항당 1점, 총 20점)
 * - 이름 필수 입력, 문항 전부 필수
 * - 제출 직후 점수 공개, 정답·해설·복습할 애니메이션 링크 표시
 * - 문항 순서 고정(Day 4 교시 순), 보기 순서도 고정
 */

function createNNDay4Quiz() {
  const form = FormApp.create('Neural Network Day 4 복습 퀴즈');
  form.setIsQuiz(true)
      .setDescription(
        '범위: Day 4 전체 (SGD 의 약점 · Momentum · AdaGrad · RMSProp · Adam · Xavier · He · Batch Normalization · 과적합 · weight decay · Dropout · hyperparameter 탐색 · mynn.py)\n' +
        '문항당 1점, 총 20점 · 권장 시간 15분\n' +
        '제출 후 점수와 해설이 바로 표시됩니다. 틀린 문항은 해설의 애니메이션 링크로 복습하세요.\n' +
        '애니메이션: https://imguru-mooc.github.io/AI_CAMPUS/NN-day4/animation.html')
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
var ANIM_URL = 'https://imguru-mooc.github.io/AI_CAMPUS/NN-day4/animation.html';

var QUESTIONS = [
  { title: "f(x, y) = x²/20 + y² 에서 SGD 를 쓰면 y 갱신은 y ← (1 − 2·lr)·y 이다. y 방향이 진동하며 커지지 않고 수렴하는 조건은?",
    options: ["lr < 1", "lr < 0.1", "lr < 2", "lr < 20"],
    answer: 0,
    why: "|1 − 2·lr| < 1 → 0 < lr < 1. lr 0.95 는 매 step 부호가 바뀌는 지그재그(−0.9배), lr 1.0 은 ±2 에서 영원히 진동, lr 1.05 는 발산(y = 34.9).",
    anim: "D4-1" },
  { title: "같은 함수, 시작 (−7, 2) 에서 SGD lr 0.1 로 30 step 진행한 결과는?",
    options: ["y 가 ±2 사이를 오가며 영원히 진동함", "y 가 34.9 까지 커지며 발산해 버림", "끝 (−0.350, 0.085) — y 가 매 step 부호를 바꿈", "끝 (−5.178, 0.002) — y 는 수렴, x 는 느림"],
    answer: 3,
    why: "x 방향 기울기 x/10 이 작아 lr 0.1 로는 거의 못 움직입니다. (−0.350, 0.085) 는 lr 0.95, ±2 진동은 lr 1.0, 발산은 lr 1.05 의 결과 — 학습률 하나로 두 방향을 다 만족시킬 수 없습니다.",
    anim: "D4-1" },
  { title: "Momentum (α = 0.9) 의 갱신식으로 옳은 것은? (g = ∂L/∂W, η = lr)",
    options: ["v ← αv + η·g,  W ← W + v", "h ← h + g²,  W ← W − η·g / √h", "v ← αv − η·g,  W ← W − v", "v ← αv − η·g,  W ← W + v"],
    answer: 3,
    why: "속도 v 에 −η·g 를 누적하고 W 에 더합니다. h ← h + g² 는 AdaGrad (RMSProp 은 h ← ρh + (1 − ρ)g²). 모든 옵티마이저는 update(params, grads) 로 제자리 갱신.",
    anim: "D4-2" },
  { title: "Adam 의 m̂ = m / (1 − β1ᵗ), v̂ = v / (1 − β2ᵗ) (편향 보정)이 필요한 이유는?",
    options: ["기울기의 부호를 매 step 뒤집어 지그재그를 없애려고", "β1, β2 가 커서 학습 후반에 기울기가 발산하기 쉬워서", "m, v 가 0 에서 시작해 초반 값이 0 쪽으로 치우쳐서", "학습률 η 를 매 step 1/t 비율로 서서히 줄이려고"],
    answer: 2,
    why: "m ← β1·m + (1 − β1)g 를 0 에서 시작하면 t = 1 일 때 m = 0.1·g 로 작습니다. 1 − β1¹ = 0.1 로 나누면 m̂ = g. t 가 커지면 분모 → 1 이라 보정이 사라집니다.",
    anim: "D4-2" },
  { title: "MNIST 784-100×4-10 · He · batch 128 · 2000 iteration 옵티마이저 비교 결과로 옳은 것은?",
    options: ["다섯 옵티마이저가 모두 0.97 이상으로 거의 같다", "SGD 0.9378 최저, RMSProp 0.9745 · Adam 0.9740", "Adam 0.9745 가 가장 높고, SGD 0.9644 가 가장 낮다", "SGD 0.9378 이 가장 낮고, Momentum 이 0.9745 로 1위"],
    answer: 1,
    why: "test: SGD(0.01) 0.9378 · Momentum(0.01) 0.9644 · AdaGrad(0.01) 0.9726 · RMSProp(0.001) 0.9745 · Adam(0.001) 0.9740. 마지막 100 iteration 평균 손실 0.227 / 0.074 / 0.059 / 0.046 / 0.046.",
    anim: "D4-2" },
  { title: "784-10-10 망의 W 를 전부 0.01 (같은 값)으로 두고 SGD 0.1 로 300 iteration 학습하면?",
    options: ["무작위 초기화와 거의 같은 정확도로 수렴한다", "출력층만 학습되고 W1 은 모두 0 으로 줄어든다", "은닉 뉴런이 똑같이 갱신돼 W1 의 열이 모두 같다", "첫 step 에 기울기가 발산해 손실이 nan 이 된다"],
    answer: 2,
    why: "같은 값에서 출발하면 모든 은닉 뉴런이 같은 기울기를 받아 대칭이 깨지지 않습니다 → test 0.2714. W 를 0 으로 두면 dW1 = 0, dW2 = 0 이라 아예 학습되지 않습니다.",
    anim: "D4-3" },
  { title: "활성화 함수에 맞는 가중치 초기화 표준편차로 옳은 것은? (n_in = 앞 층 노드 수)",
    options: ["sigmoid · tanh · ReLU 모두 √(2/n_in) 로 같다", "sigmoid · tanh 는 √(1/n_in), ReLU 는 √(2/n_in)", "sigmoid · tanh 는 √(2/n_in), ReLU 는 √(1/n_in)", "sigmoid · tanh 는 0.01 고정, ReLU 는 √(1/n_in)"],
    answer: 1,
    why: "Xavier √(1/n_in) 는 sigmoid · tanh, He √(2/n_in) 는 ReLU (음수 절반이 0 이 되므로 2배). ReLU 5층 활성값 std: He 0.82 → 1.00 유지, Xavier 0.583 → 0.177 감소.",
    anim: "D4-3" },
  { title: "MNIST 4층 은닉 · SGD 0.01 · 2000 iteration 에서 초기화 std 0.01 / Xavier / He 의 test 정확도는?",
    options: ["0.9378 / 0.9179 / 0.1135", "0.1135 / 0.9378 / 0.9179", "0.1135 / 0.9179 / 0.9378", "0.2714 / 0.9179 / 0.9378"],
    answer: 2,
    why: "std 0.01 은 층마다 신호가 사라져 손실이 2.301 그대로 (0.1135 ≈ 찍기). ReLU 망에서는 He 가 가장 좋습니다. 0.2714 는 상수 초기화 대칭 실험의 값.",
    anim: "D4-3" },
  { title: "BatchNorm forward 로 옳은 것은? (ε = 1e-7)",
    options: ["axis=1 의 μ, σ² 로 x̂ = (x − μ)/√(σ² + ε), y = γx̂ + β", "axis=0 의 μ, σ² 로 x̂ = (x − β)/γ, 그 뒤 μ, σ 를 학습", "axis=0 의 μ, σ² 로 x̂ = (x − μ)/√(σ² + ε), y = γx̂ + β", "axis=0 의 μ, σ² 로 x̂ = (x − μ)/(σ² + ε), y = γx̂ + β"],
    answer: 2,
    why: "미니배치 방향(axis=0, 특징마다)으로 평균 0 · 분산 1 을 맞춘 뒤 γ 로 늘이고 β 로 옮깁니다. 예: γ = [1, 2, 0.5, 1.5], β = [0, 1, −1, 0.5] → 출력 열 평균 = β, 표준편차 = γ.",
    anim: "D4-4" },
  { title: "BatchNorm 의 학습 / 추론 동작으로 옳은 것은?",
    options: ["추론은 γ, β 없이 사용, running_mean 은 저장만 한다", "추론은 running 값 사용, running_mean = 0.9·rm + 0.1·μ", "추론도 배치 통계 사용, running_mean = 0.9·rm + 0.1·μ", "추론은 running 값 사용, running_mean = 0.1·rm + 0.9·μ"],
    answer: 1,
    why: "학습(train_flg=True)은 배치 μ, σ² 를 쓰고 running 값을 0.9 · 0.1 비율로 갱신, 추론(train_flg=False)은 running 값을 씁니다. N(5, 2²) 배치를 50번 넣으면 running_mean 5.03, running_var 3.83.",
    anim: "D4-4" },
  { title: "BatchNorm backward 의 dx 를 열(axis=0) 방향으로 더하면 0 이 된다. 여기서 나오는 결론은?",
    options: ["BN 뒤 Relu 의 기울기가 모두 0 이 되어 버린다", "BN 앞 Affine 의 db 가 0 이다 (편향이 지워짐)", "dβ = Σdout 도 0 이라 β 는 처음 값 그대로 고정된다", "dγ 가 항상 0 이라 γ 는 학습되지 않는다"],
    answer: 1,
    why: "BN 은 평균을 빼므로 앞 층 편향 b 를 얼마로 바꿔도 출력이 같습니다 → db = Σdx = 0. dγ = Σ(x̂·dout), dβ = Σdout 은 0 이 아닙니다. gradient check: dx 3.8e-10 · dγ 9.1e-13 · dβ 6.8e-14.",
    anim: "D4-4" },
  { title: "학습 300장 · 784-100×6-10 · 200 epoch 학습에서 train 1.000 / test 0.786 이 나왔다. test 손실은 25 → 200 epoch 동안?",
    options: ["0.79 근처로 유지 (정확도처럼 정체)", "1.12 → 0.79 로 감소 (train 과 함께 개선)", "0.79 → 1.12 로 증가 (틀린 답을 과신)", "0.79 → 0.12 로 감소 (정확도와 함께 개선)"],
    answer: 2,
    why: "정확도는 멈춰 있지만 test 손실은 오히려 오릅니다 — 틀린 샘플에 대한 확신이 커지기 때문. 정확도만 보면 과적합을 놓칩니다. 간극 1.000 − 0.786 = 0.214.",
    anim: "D4-5" },
  { title: "같은 과적합 실험에서 학습 데이터를 3000장으로 늘리면 (같은 600 iteration = 20 epoch)?",
    options: ["train 1.000 / test 0.926 — 간극이 크게 준다", "train 1.000 / test 0.802 — 모델 축소와 같다", "train 1.000 / test 0.786 — 아무 변화가 없다", "train 0.926 / test 0.786 — train 만 떨어진다"],
    answer: 0,
    why: "데이터를 늘리는 것이 과적합 처방 1순위입니다 (0.786 → 0.926). 0.802 는 모델을 784-100-10 으로 줄였을 때의 test 값.",
    anim: "D4-5" },
  { title: "weight decay (L2) 의 손실 · 기울기 식으로 옳은 것은?",
    options: ["손실 + λΣ|W|, 기울기 + λ·sign(W) (편향 · γ · β 포함)", "손실 + ½λΣW², 기울기 + λW (편향 · γ · β 제외)", "손실 + ½λΣW², 기울기 + ½λW (편향 포함)", "손실 + λΣW², 기울기 + λW (가중치 · 편향 모두)"],
    answer: 1,
    why: "½ 을 붙이면 미분할 때 2 가 상쇄되어 λW 가 됩니다. 5교시 설정에 λ 0.01 → train 1.000 / test 0.810, test 손실 0.74 → 0.65.",
    anim: "D4-6" },
  { title: "Dropout(ratio) 계층의 동작으로 옳은 것은?",
    options: ["학습: x * mask, 추론: 모두 켜고 × (1 − ratio)", "학습: x * mask, 추론: 같은 mask 로 일부를 끈다", "학습: x * mask, 추론: 모두 켜고 × ratio 를 곱함", "학습: x * mask, 추론: 모두 켜고 ÷ (1 − ratio)"],
    answer: 0,
    why: "학습: mask = rng.random(x.shape) > ratio, out = x * mask, backward dout * mask. 추론: 평균 출력 크기를 맞추려고 (1 − ratio) 배. Dropout 0.2 → test 0.800 (25 epoch train 0.53, 초반이 느림).",
    anim: "D4-6" },
  { title: "과적합 설정(규제 없음 test 0.786)에 규제를 적용한 결과로 옳은 것은?",
    options: ["λ 0.01 → 0.810, Dropout 0.2 → 0.800, λ 0.1 → 0.641", "λ 0.01 → 0.800, Dropout 0.2 → 0.810, λ 0.1 → 0.926", "λ 0.01 → 0.641, Dropout 0.2 → 0.800, λ 0.1 → 0.810", "λ 0.01 → 0.810, Dropout 0.2 → 0.641, λ 0.1 → 0.800"],
    answer: 0,
    why: "적당한 규제는 test 를 올리지만 λ 0.1 은 너무 강해 train 0.767 / test 0.641 (과소적합). λ · ratio 도 골라야 하는 값 → 7교시 hyperparameter 탐색.",
    anim: "D4-6" },
  { title: "hyperparameter 탐색 방법으로 옳은 것은? (U = rng.uniform)",
    options: ["validation 으로 고르고 lr = U(1e-4, 1)", "train 손실로 고르고 lr 은 0.1 간격 그리드 탐색", "test 로 고르고 lr = 10 ** U(−4, 0)", "validation 으로 고르고 lr = 10 ** U(−4, 0)"],
    answer: 3,
    why: "test 로 고르면 test 에 과적합됩니다 (보고는 test 한 번). 학습 1000장 → p = default_rng(0).permutation(1000), val = p[:200], train = p[200:]. 로그 스케일로 뽑아야 작은 lr 영역도 고르게 봅니다.",
    anim: "D4-7" },
  { title: "30회 랜덤 탐색 (lr = 10^U(−4, 0), λ = 10^U(−6, −1)) 결과의 해석으로 옳은 것은?",
    options: ["1위 lr 0.242 · val 0.885, λ 는 영향이 작다", "1위 lr 0.242 · val 0.885, λ 가 결정적이다", "1위 lr 0.0003 · val 0.885, λ 는 영향이 작다", "1위 lr 0.588 · val 0.140, lr 은 영향이 작다"],
    answer: 0,
    why: "상위: lr 0.242 (λ 4.8e-6) 0.885 · lr 0.240 0.875 · lr 0.588 0.870. lr 3e-4 ~ 6e-4 근처는 val 0.08 ~ 0.14 (학습 안 됨). 다음엔 lr = 10^U(−1.3, −0.2) 로 좁혀 다시.",
    anim: "D4-7" },
  { title: "mynn.Sequential 의 params() 와 옵티마이저 갱신에 대한 설명으로 옳은 것은?",
    options: ["키 \"0.W\" · \"1.gamma\", 복사본을 모아 → 새 배열 대입", "키 \"W1\" · \"gamma1\", 계층 배열 그대로 → 제자리", "키 \"Affine0\" · \"BN1\", 복사본을 모아 → 새 배열 대입", "키 \"0.W\" · \"1.gamma\", 계층 배열 그대로 → 제자리"],
    answer: 3,
    why: "params() 는 \"계층 번호.이름\" 키로 계층의 배열 객체를 그대로 모읍니다. 옵티마이저가 params[k] -= … 로 제자리 갱신해야 계층에 바로 반영됩니다. load_params 도 p[...] = data[k] 로 제자리 복사.",
    anim: "D4-8" },
  { title: "Adam 5 epoch 으로 test 0.9757 을 낸 기준 5층 MLP (Affine + BatchNorm + Relu) 의 count_params() 와, save_params 가 params 와 함께 저장하는 것은?",
    options: ["79,510 · BN running_mean / running_var", "109,810 · BN running_mean / running_var", "110,610 · Adam 의 m / v 상태", "110,610 · BN running_mean / running_var"],
    answer: 3,
    why: "784·100+100 + 3·(100·100+100) + 100·10+10 = 109,810 에 BN γ · β 800 을 더해 110,610. state() = params + buffers(running 평균 · 분산) — 저장하지 않으면 다시 불러온 뒤 추론 정확도가 달라집니다.",
    anim: "D4-8" }
];
