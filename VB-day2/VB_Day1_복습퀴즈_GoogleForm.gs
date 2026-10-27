/**
 * Vision 기본 모델 Day 1 복습 퀴즈 — Google Form 자동 생성 스크립트
 * HYUNDAI AI Insight Campus 온디바이스 AI · Vision 기본 모델 Day 2 (10/28) 09:00~09:15
 *
 * 사용 방법
 * 1. https://script.google.com 접속 → 새 프로젝트
 * 2. 이 파일 전체를 붙여 넣고 저장
 * 3. 상단 함수 선택에서 createVBDay1Quiz 선택 → ▶ 실행 (최초 1회 권한 승인)
 * 4. 하단 실행 로그에 출력되는 두 URL 확인
 *    - 편집용(editUrl): 강사용, 응답 확인·설정 변경
 *    - 응답용(publishedUrl): 학생에게 공유
 *
 * 생성되는 폼 설정
 * - 퀴즈 모드(자동 채점, 문항당 1점, 총 20점)
 * - 이름 필수 입력, 문항 전부 필수
 * - 제출 직후 점수 공개, 정답·해설·복습할 애니메이션 링크 표시
 * - 문항 순서 고정(Day 1 교시 순), 보기 순서도 고정
 * - 회사 Workspace 계정에서 401 invalid_client 오류가 나면 개인 Google 계정으로 실행
 */

function createVBDay1Quiz() {
  const form = FormApp.create('Vision 기본 모델 Day 1 복습 퀴즈');
  form.setIsQuiz(true)
      .setDescription(
        '범위: Vision 기본 모델 Day 1 전체 (mynn → PyTorch · Tensor · autograd · nn.Module · DataLoader · 학습 루프 · 이미지 텐서 · 영상 파이프라인 · 종이 손글씨 전처리)\n' +
        '문항당 1점, 총 20점 · 권장 시간 15분\n' +
        '제출 후 점수와 해설이 바로 표시됩니다. 틀린 문항은 해설의 애니메이션 링크로 복습하세요.\n' +
        '애니메이션: https://imguru-mooc.github.io/AI_CAMPUS/VB-day1/animation.html')
      .setShuffleQuestions(false)
      .setProgressBar(true);
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
var ANIM_URL = 'https://imguru-mooc.github.io/AI_CAMPUS/VB-day1/animation.html';

var QUESTIONS = [
  { title: "NN 과정 mynn.py 의 SoftmaxWithLoss 에 해당하는 PyTorch 부품과 그 입력은?",
    options: ["nn.Softmax — 확률을 입력", "nn.CrossEntropyLoss — softmax 전 점수(logits)와 정수 레이블", "nn.MSELoss — one-hot 레이블", "nn.NLLLoss — softmax 확률과 one-hot 레이블"],
    answer: 1,
    why: "CrossEntropyLoss 안에 softmax(정확히는 log_softmax) 가 들어 있으므로 모델 끝에 softmax 를 또 붙이지 않고 logits 를 그대로 넣습니다.",
    anim: "D1-1" },
  { title: "PyTorch 로 다시 만든 SimpleConvNet (Conv 1→30 5×5, Pool, Linear 4320→100, Linear 100→10) 의 파라미터 수는?",
    options: ["79,510", "62,006", "433,890 — NumPy 판과 같다", "4,320"],
    answer: 2,
    why: "780 + 432,100 + 1,010 = 433,890. 같은 모델을 다른 도구로 쓴 것이라 숫자가 정확히 같습니다.",
    anim: "D1-1" },
  { title: "a = np.zeros(3); b = torch.from_numpy(a); c = torch.tensor(a); b[0] = 7; c[1] = 9 실행 후 a 는?",
    options: ["[7, 9, 0]", "[0, 0, 0]", "[0, 9, 0]", "[7, 0, 0]"],
    answer: 3,
    why: "from_numpy 는 같은 메모리를 공유하므로 b[0] = 7 이 a 에 보이고, torch.tensor 는 복사본이라 c[1] = 9 는 a 에 영향이 없습니다.",
    anim: "D1-2" },
  { title: "NumPy 기본 실수 배열을 torch.from_numpy 로 바꿔 nn.Linear 에 넣었더니 'expected ... Float but found Double' 오류가 났다. 해결책은?",
    options: [".float() 로 float32 로 바꾼다", ".long() 으로 정수로 바꾼다", ".cuda() 로 GPU 에 올린다", ".detach() 를 붙인다"],
    answer: 0,
    why: "NumPy 기본은 float64(Double), PyTorch 가중치는 float32(Float) 입니다. 정수 레이블 쪽은 반대로 int64(long) 이 필요합니다.",
    anim: "D1-2" },
  { title: "x = torch.arange(24).reshape(2, 3, 4); p = x.permute(0, 2, 1) 일 때 p 의 shape 와 p[1, 3, 2] 와 같은 원소는?",
    options: ["(2, 3, 4), x[1, 3, 2]", "(4, 3, 2), x[2, 3, 1]", "(2, 4, 3), x[1, 2, 3]", "(2, 12), x[1, 3, 2]"],
    answer: 2,
    why: "permute(0, 2, 1) 은 마지막 두 축의 순서를 바꿉니다. 원소 배치가 실제로 바뀌는 점이 모양만 바꾸는 reshape · view 와 다릅니다.",
    anim: "D1-2" },
  { title: "f = x² + 3y² 를 x = 1, y = 2 에서 f.backward() 하면 x.grad, y.grad 는?",
    options: ["1, 4", "2, 12", "2, 6", "13, 13"],
    answer: 1,
    why: "∂f/∂x = 2x = 2, ∂f/∂y = 6y = 12 (f = 13). autograd 가 계산 그래프를 거꾸로 따라가며 국소 미분을 곱합니다.",
    anim: "D1-3" },
  { title: "x = torch.tensor(1.0, requires_grad=True) 로 (x**2).backward() 를 두 번 실행하면 x.grad 는?",
    options: ["2.0 — 마지막 결과로 덮어씀", "1.0", "0.0", "4.0 — 기울기가 누적(더해짐)"],
    answer: 3,
    why: "기울기는 .grad 에 더해집니다. 그래서 학습 루프는 매 step optimizer.zero_grad() 로 기울기를 비우고 시작합니다.",
    anim: "D1-3" },
  { title: "PyTorch 학습 한 step 의 올바른 순서는?",
    options: ["loss.backward() → opt.zero_grad() → opt.step()", "opt.step() → loss.backward() → opt.zero_grad()", "opt.zero_grad() → loss.backward() → opt.step()", "opt.zero_grad() → opt.step() → loss.backward()"],
    answer: 2,
    why: "비우고(zero_grad) → 기울기 계산(backward) → 갱신(step). NN 과정의 optimizer.update(params, grads) 가 step() 으로 바뀌었습니다.",
    anim: "D1-3" },
  { title: "autograd 로 경사하강을 직접 할 때 p -= lr * p.grad 를 with torch.no_grad(): 안에서 하는 이유는?",
    options: ["갱신 연산까지 계산 그래프에 기록되면 안 되기 때문 (잎 텐서의 in-place 연산 오류)", "GPU 에서만 동작하기 때문", "속도를 높이기 위해서일 뿐 결과는 같다", "기울기를 0 으로 만들기 위해"],
    answer: 0,
    why: "no_grad 없이 requires_grad 잎 텐서를 제자리 갱신하면 'a leaf Variable that requires grad is being used in an in-place operation' 오류가 납니다.",
    anim: "D1-3" },
  { title: "MNIST train 60,000장을 DataLoader(batch_size=100, shuffle=True) 로 읽을 때 len(loader) 와 배치 xb 의 shape 는?",
    options: ["60,000, (1, 28, 28)", "100, (600, 1, 28, 28)", "600, (100, 784)", "600, (100, 1, 28, 28)"],
    answer: 3,
    why: "60,000 / 100 = 600 iteration/epoch. ToTensor 가 (1, 28, 28) 로 만들고 DataLoader 가 100장을 쌓아 (100, 1, 28, 28) 이 됩니다.",
    anim: "D1-4" },
  { title: "학습 전 10 클래스 분류 모델의 CrossEntropy 손실이 약 2.30 인 이유는?",
    options: ["학습률이 너무 커서", "예측이 거의 균등(1/10) 이라 −ln(1/10) = ln 10 ≈ 2.30 이기 때문", "레이블이 one-hot 이 아니어서", "softmax 를 두 번 적용해서"],
    answer: 1,
    why: "실행 예 2.3010. 학습 전 손실이 ln(클래스 수) 근처인지 보는 것은 손실 함수 · 레이블이 제대로 연결됐는지 확인하는 좋은 습관입니다.",
    anim: "D1-4" },
  { title: "평가 함수에서 model.eval() 과 with torch.no_grad() 에 대한 설명으로 옳은 것은?",
    options: ["둘은 같은 일을 하므로 하나만 쓰면 된다", "eval() 은 기울기 기록을 끄고, no_grad() 는 Dropout 을 끈다", "eval() 은 Dropout · BatchNorm 동작을 추론 모드로, no_grad() 는 연산 기록을 끈다 — 둘 다 쓴다", "no_grad() 만 쓰면 BatchNorm 도 추론 모드가 된다"],
    answer: 2,
    why: "하는 일이 다릅니다. Day 2 부터 BatchNorm 이 들어간 모델을 쓰므로 eval() 을 빼먹으면 결과가 배치마다 달라집니다.",
    anim: "D1-5" },
  { title: "torch.save(model.state_dict(), 'mlp.pt') 로 저장한 784-100-10 MLP 파일이 약 320 KB 인 이유는?",
    options: ["파라미터 79,510 개 × 4 bytes (float32) ≈ 318 KB + 이름 정보", "모델 코드 전체가 저장되기 때문", "학습 기록(hist) 이 함께 저장되기 때문", "MNIST 데이터 일부가 저장되기 때문"],
    answer: 0,
    why: "state_dict 는 '이름 → 텐서' 딕셔너리입니다. 실행 예 320,309 bytes. Quantization 과목에서 이 숫자를 1/4 로 줄입니다.",
    anim: "D1-5" },
  { title: "cv2.imread 로 읽은 사진을 matplotlib 에 그대로 그리면 파란 잉크가 갈색으로 보인다. 이유와 해결책은?",
    options: ["JPEG 압축 때문 — PNG 로 저장", "uint8 overflow 때문 — float 로 변환", "OpenCV 는 BGR 순서 — img[:, :, ::-1] 로 RGB 로", "정규화를 안 해서 — /255"],
    answer: 2,
    why: "OpenCV 는 (H, W, C) uint8 을 BGR 순서로 읽습니다. 사전학습 모델도 RGB 를 기대하므로 반드시 바꿔야 하고, 안 바꿔도 오류는 나지 않아 더 위험합니다.",
    anim: "D1-6" },
  { title: "uint8 배열 [[200, 250]] 에 np.uint8 100 을 더하면? (cv2.add 로 더하면 [[255, 255]])",
    options: ["[[300, 350]]", "[[255, 255]]", "오류", "[[44, 94]] — 255 를 넘으면 0 부터 다시 셈"],
    answer: 3,
    why: "300 − 256 = 44, 350 − 256 = 94 (overflow). OpenCV 는 255 에서 멈추고(saturation), float 로 바꾸면 [300, 350]. 이미지 계산은 float 로 바꾼 뒤에.",
    anim: "D1-6" },
  { title: "OpenCV 이미지 rgb (640, 480, 3) uint8 을 PyTorch 모델 입력 텐서로 바꾸는 코드로 옳은 것은?",
    options: ["torch.from_numpy(rgb).float()", "torch.from_numpy(rgb.copy()).permute(2, 0, 1).float().div(255).unsqueeze(0)", "torch.from_numpy(rgb).reshape(3, 640, 480) / 255", "torch.tensor(rgb).permute(0, 1, 2).unsqueeze(0)"],
    answer: 1,
    why: "HWC → CHW (permute(2, 0, 1)), uint8 → float32, ÷255, 배치 축 추가 → (1, 3, 640, 480). reshape 는 원소 순서를 바꾸지 않아 이미지가 뒤섞입니다.",
    anim: "D1-6" },
  { title: "cv2.VideoWriter('out.mp4', fourcc, 30, (120, 320)) 로 120×320 (H×W) 프레임을 썼더니 오류 없이 거의 빈 파일이 됐다. 원인은?",
    options: ["크기 인자는 (W, H) 순서라 (320, 120) 이어야 한다 — 맞지 않는 프레임은 조용히 버려진다", "FPS 가 너무 높다", "fourcc 가 틀렸다", "release() 를 너무 일찍 불렀다"],
    answer: 0,
    why: "cv2.resize 와 VideoWriter 의 크기 인자는 (가로 W, 세로 H). shape 의 (H, W) 와 반대라 자주 틀립니다.",
    anim: "D1-7" },
  { title: "영상 파일을 프레임 단위로 처리하는 기본 루프로 옳은 것은?",
    options: ["for frame in cap: process(frame)", "while True: ok, frame = cap.read(); if not ok: break; ...; writer.write(frame) → 끝나면 release()", "frames = cap.read_all(); for f in frames: ...", "cap.get(cv2.CAP_PROP_FRAME_COUNT) 번 cap.read() 만 하면 release() 는 필요 없다"],
    answer: 1,
    why: "read() 는 (성공 여부, 프레임) 을 돌려주고 영상이 끝나면 ok 가 False. release() 를 해야 파일이 완성됩니다. 카메라를 열어도 같은 루프입니다.",
    anim: "D1-7" },
  { title: "MNIST 로 98% 를 내는 CNN 에 종이 손글씨 사진을 '흑백 → 28×28 → ÷255' 만 해서 넣었더니 30장 중 3장만 맞혔다 (대부분 '8'). 가장 큰 이유는?",
    options: ["모델이 과적합되어서", "사진 해상도가 너무 높아서", "학습 epoch 이 부족해서", "MNIST(검은 배경 · 흰 글씨 · 20×20 · 무게중심 가운데) 와 입력의 분포가 달라서 (domain shift)"],
    answer: 3,
    why: "naive 0.100 → 전처리 0.967. 입력을 학습 데이터와 같은 모양으로 맞추는 것이 전처리입니다. Day 2 데이터 증강 · Day 3 전이학습은 반대로 모델 쪽을 실제 데이터에 맞춥니다.",
    anim: "D1-8" },
  { title: "그림자가 진 종이 사진에서 Otsu 이진화가 그림자를 글씨로 잡는 문제를 푼 단계는?",
    options: ["GaussianBlur (5, 5) 를 두 번 적용", "가장 큰 덩어리 대신 가장 작은 덩어리 선택", "크게 흐린 배경으로 나누는 조명 보정 cv2.divide(g, GaussianBlur(g, (0, 0), 25), scale=255)", "threshold 기준값을 128 로 고정"],
    answer: 2,
    why: "아주 크게 흐린 이미지 = 조명 · 그림자만 남은 배경. 원본을 배경으로 나누면 밝기가 평평해집니다. 실행 예: 조명 보정 없이 0.600, 있으면 0.967.",
    anim: "D1-8" }
];
