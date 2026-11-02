/**
 * Vision 기본 모델 Day 1~3 복습 퀴즈 — Google Form 자동 생성 스크립트
 * HYUNDAI AI Insight Campus 온디바이스 AI · Vision 기본 모델 Day 4 (11/2) 09:00~09:15
 * (10/30 특강으로 하루 공백 — Day 1 4문항 · Day 2 6문항 · Day 3 10문항)
 *
 * 사용 방법
 * 1. https://script.google.com 접속 → 새 프로젝트
 * 2. 이 파일 전체를 붙여 넣고 저장
 * 3. 상단 함수 선택에서 createVBDay3Quiz 선택 → ▶ 실행 (최초 1회 권한 승인)
 * 4. 실행 로그의 편집용(editUrl) · 응답용(publishedUrl) URL 확인 — 응답용을 학생에게 공유
 *
 * 퀴즈 모드(문항당 1점, 총 20점) · 이름 필수 · 제출 직후 점수와 해설 공개 · 문항 · 보기 순서 고정
 * 회사 Workspace 계정에서 401 invalid_client 오류가 나면 개인 Google 계정으로 실행
 */

function createVBDay3Quiz() {
  const form = FormApp.create('Vision 기본 모델 Day 1~3 복습 퀴즈');
  form.setIsQuiz(true)
      .setDescription(
        '범위: Vision 기본 모델 Day 1 ~ 3 (Day 1 PyTorch · 이미지 · 영상 4문항 / Day 2 Conv · VGG · BatchNorm · ResNet 6문항 / Day 3 연산량 · Depthwise Separable · MobileNet · 전이학습 · Grad-CAM 10문항)\n' +
        '문항당 1점, 총 20점 · 권장 시간 15분\n' +
        '제출 후 점수와 해설이 바로 표시됩니다. 틀린 문항은 해설의 애니메이션 링크로 복습하세요.\n' +
        '애니메이션: https://imguru-mooc.github.io/AI_CAMPUS/VB-day1/animation.html · VB-day2/animation.html · VB-day3/animation.html')
      .setShuffleQuestions(false)
      .setProgressBar(true);

  form.addTextItem().setTitle('이름').setRequired(true);
  form.addPageBreakItem().setTitle('문항 (20)');

  QUESTIONS.forEach(function (q, i) {
    const item = form.addMultipleChoiceItem();
    item.setTitle((i + 1) + '. ' + q.title).setPoints(1).setRequired(true);
    item.setChoices(q.options.map(function (opt, j) { return item.createChoice(opt, j === q.answer); }));
    const review = q.anim ? '  [복습: ' + q.anim + ' ' + animUrl(q.anim) + ']' : '';
    item.setFeedbackForCorrect(FormApp.createFeedback().setText('정답입니다. ' + q.why + review).build());
    item.setFeedbackForIncorrect(FormApp.createFeedback().setText('정답: ' + LABEL[q.answer] + ' ' + q.options[q.answer] + ' — ' + q.why + review).build());
  });

  Logger.log('편집용  URL: ' + form.getEditUrl());
  Logger.log('응답용  URL: ' + form.getPublishedUrl());
}

var LABEL = ['①', '②', '③', '④'];
var ANIM_BASE = 'https://imguru-mooc.github.io/AI_CAMPUS/';

// 'D2-4' → https://imguru-mooc.github.io/AI_CAMPUS/VB-day2/animation.html#d2-4
function animUrl(anim) {
  return ANIM_BASE + 'VB-day' + anim.charAt(1) + '/animation.html#' + anim.toLowerCase();
}

var QUESTIONS = [
  // ── Day 1 (4) ──
  { title: "a = np.zeros(3); b = torch.from_numpy(a); c = torch.tensor(a); b[0] = 7; c[1] = 9 를 실행한 뒤 a 의 값은?",
    options: ["[0, 0, 0]", "[7, 9, 0]", "[7, 0, 0]", "[0, 9, 0]"],
    answer: 2, why: "from_numpy 는 메모리를 공유하고 torch.tensor 는 복사합니다. 그래서 b 를 바꾸면 a 도 바뀌고 c 는 따로 → c = [0, 9, 0].", anim: "D1-2" },
  { title: "cv2.imread 로 읽은 사진을 PyTorch 모델 입력으로 바꿀 때의 설명으로 옳은 것은?",
    options: ["OpenCV 는 (H, W, C) uint8 0~255 BGR → RGB 로 바꾸고 permute 로 (C, H, W), float32 0~1, 배치 축을 붙여 (N, C, H, W)", "OpenCV 는 (C, H, W) RGB 라 그대로 넣으면 된다", "OpenCV 는 (W, H, C) float32 0~1 이다", "BGR 그대로 넣어도 색 순서는 모델이 알아서 맞춘다"],
    answer: 0, why: "img[:, :, ::-1] (BGR→RGB) → permute(2, 0, 1) → .float() / 255 → unsqueeze(0). 오늘 YOLO 전처리 (letterbox → BGR→RGB → /255 → CHW) 와 같은 흐름입니다.", anim: "D1-6" },
  { title: "PyTorch 학습 루프에서 배치마다 부르는 순서로 옳은 것은?",
    options: ["backward → zero_grad → step", "step → backward → zero_grad", "zero_grad → step → backward", "zero_grad → backward → step"],
    answer: 3, why: "기울기는 .grad 에 누적되므로 먼저 0 으로 지우고, 역전파로 기울기를 구한 뒤, 옵티마이저가 갱신합니다.", anim: "D1-5" },
  { title: "cv2.VideoWriter(path, fourcc, fps, size) 의 size 인자에 대한 설명으로 옳은 것은?",
    options: ["(H, W) 순서이고 틀리면 바로 오류가 난다", "(W, H) 순서이고 틀리면 오류 없이 빈 영상이 만들어진다", "(C, H, W) 순서다", "생략하면 첫 프레임 크기로 자동 설정된다"],
    answer: 1, why: "cv2.resize 와 마찬가지로 (W, H). write() 한 프레임 크기와 다르면 조용히 버려집니다. 오늘 7교시 영상 탐지 결과 저장에 다시 씁니다.", anim: "D1-7" },

  // ── Day 2 (6) ──
  { title: "224×224 입력에 nn.Conv2d(3, 64, kernel_size=7, stride=2, padding=3) 을 적용하면 출력 크기는?",
    options: ["224×224", "109×109", "112×112", "56×56"],
    answer: 2, why: "O = ⌊(N + 2P − K) / S⌋ + 1 = ⌊(224 + 6 − 7) / 2⌋ + 1 = 112. ResNet 첫 층입니다. 오늘 YOLO 의 stride 8 · 16 · 32 도 같은 원리로 80 · 40 · 20 칸이 됩니다.", anim: "D2-2" },
  { title: "nn.Conv2d(64, 128, 3) (bias 포함) 의 파라미터 수는?",
    options: ["73,856", "73,728", "8,320", "36,928"],
    answer: 0, why: "Cout · Cin · K² + Cout = 128 · 64 · 9 + 128 = 73,856. bias 를 빼면 73,728 (Day 3 의 일반 Conv 64→128).", anim: "D2-2" },
  { title: "VGG 가 5×5 한 층 대신 3×3 을 두 층 쌓는 이유로 옳지 않은 것은?",
    options: ["같은 5×5 수용 영역을 본다", "파라미터가 18C² vs 25C² 로 약 28% 적다", "ReLU 가 두 번 들어가 더 복잡한 모양을 표현한다", "출력 해상도가 두 배로 커진다"],
    answer: 3, why: "padding=1 이면 크기는 그대로입니다. 세 층 쌓으면 7×7 수용 영역 (27C² vs 49C²).", anim: "D2-4" },
  { title: "BatchNorm2d 가 있는 모델을 평가할 때 model.eval() 을 부르는 이유는?",
    options: ["기울기 계산을 끄기 위해서", "BN 이 배치 통계 대신 학습 중 모은 running 평균 · 분산을 쓰게 하려고", "학습률을 0 으로 만들려고", "가중치를 float16 으로 바꾸려고"],
    answer: 1, why: "train 모드의 BN 은 현재 배치의 평균 · 분산을 써서 배치 구성에 따라 결과가 달라집니다. 기울기 끄기는 torch.no_grad() — 다른 일입니다.", anim: "D2-5" },
  { title: "ResNet-20 (CIFAR, 32×32 입력) 의 layer3 출력 크기는? (배치 축 제외)",
    options: ["(16, 32, 32)", "(32, 16, 16)", "(64, 8, 8)", "(128, 4, 4)"],
    answer: 2, why: "layer1 16ch 32×32 → layer2 32ch 16×16 → layer3 64ch 8×8 → GAP → Linear(64→10). 파라미터 272,474. Day 3 Grad-CAM 이 이 8×8 을 썼습니다.", anim: "D2-6" },
  { title: "데이터 증강 RandomCrop(32, padding=4) + RandomHorizontalFlip() 에 대한 설명으로 옳은 것은?",
    options: ["train 데이터에만 적용하고 test 는 원본 그대로 평가한다", "test 에도 똑같이 적용해야 공정하다", "MNIST 숫자에도 좌우 반전은 항상 안전하다", "한 번 증강한 이미지를 저장해 매 epoch 같은 그림을 쓴다"],
    answer: 0, why: "꺼낼 때마다 다른 그림 (최대 ±4 픽셀 이동, 50% 좌우 반전). 정답이 바뀌는 변형 (6 ↔ 9 등) 은 쓰지 않습니다.", anim: "D2-7" },

  // ── Day 3 (10) ──
  { title: "Depthwise Separable 합성곱의 연산량 ÷ 일반 합성곱의 연산량은? (커널 K×K, 출력 채널 Cout)",
    options: ["1/K²", "1/(Cout · K²)", "1/Cin + 1/K", "1/Cout + 1/K²"],
    answer: 3, why: "depthwise (채널마다 공간만) + pointwise 1×1 (채널만). K = 3 이면 약 1/9 ~ 1/8. 64→128 에서 0.1189 = 1/128 + 1/9.", anim: "D3-3" },
  { title: "64 → 128 채널 3×3 합성곱 (bias 없음) 을 Depthwise Separable 로 바꾸면 가중치 수는?",
    options: ["73,728 → 8,192", "73,728 → 8,768", "73,856 → 576", "36,864 → 8,768"],
    answer: 1, why: "depthwise 64 · 9 = 576 + pointwise 64 · 128 = 8,192 → 8,768 (일반의 11.9%). DS 연산의 약 93% 가 pointwise 1×1 입니다.", anim: "D3-3" },
  { title: "ResNet-50 bottleneck (256 채널, bias 없음) 이 1×1 256→64 → 3×3 64→64 → 1×1 64→256 으로 하는 이유와 가중치 수는?",
    options: ["활성화를 줄이려고 — 1,179,648", "정확도를 올리려고 — 36,864", "비싼 3×3 을 채널이 적은 곳에서 하려고 — 69,632 (3×3 두 층 1,179,648 의 약 1/17)", "출력 크기를 줄이려고 — 16,384"],
    answer: 2, why: "16,384 + 36,864 + 16,384 = 69,632. 1×1 Conv 는 위치마다 채널만 섞는 압축기입니다.", anim: "D3-2" },
  { title: "ResNet-50 (25,557,032 파라미터) 을 float32 로 저장할 때 가중치 메모리는?",
    options: ["약 102 MB", "약 25.6 MB", "약 204 MB", "약 818 MB"],
    answer: 0, why: "float32 = 4 bytes → 25,557,032 × 4 / 1e6 = 102.2 MB. 25.6 MB 는 1 byte (int8) 로 센 값 — Quantization (11/16~) 에서 다룹니다.", anim: "D3-2" },
  { title: "온디바이스 지연시간을 잴 때의 규칙으로 옳지 않은 것은?",
    options: ["CPU 에서 torch.set_num_threads(1) 로 스레드 1개", "배치 1장 (카메라 프레임처럼)", "warm-up 3회는 버리고 여러 번 잰 평균", "GPU 에서 배치 128 로 잰 시간을 128 로 나눠 한 장 지연시간으로 쓴다"],
    answer: 3, why: "배치로 나눈 처리량 (throughput) 은 한 장이 들어와 결과가 나오기까지의 지연시간 (latency) 과 다릅니다. 30 FPS = 33 ms/장.", anim: "D3-1" },
  { title: "MobileNetV2 inverted residual 블록 (t = 6) 의 구조로 옳은 것은?",
    options: ["1×1 줄이기 → 3×3 → 1×1 늘리기, 모든 Conv 뒤 ReLU", "1×1 늘리기 (×6) → depthwise 3×3 → 1×1 줄이기, 마지막 1×1 뒤에는 활성화 없음", "depthwise 3×3 두 번, 지름길 없음", "3×3 → 3×3 basic 블록 + SE"],
    answer: 1, why: "24 → 144 → 24 처럼 늘렸다 줄여 inverted. 얇은 출력에 ReLU 를 걸면 정보 손실이 커서 linear bottleneck. 지름길은 stride 1 이고 cin == cout 일 때만.", anim: "D3-4" },
  { title: "MobileNetV3 의 h-swish(x) 정의로 옳은 것은?",
    options: ["x · σ(x)", "max(0, x)", "x · ReLU6(x + 3) / 6", "ReLU6(x) / 6"],
    answer: 2, why: "x ≤ −3 이면 0, x ≥ 3 이면 x. sigmoid 없이 swish (x · σ(x)) 를 근사해 장치에서 계산이 쌉니다 (nn.Hardswish).", anim: "D3-4" },
  { title: "사전학습 MobileNetV3-Small 의 classifier[3] 을 nn.Linear(1024, 10) 으로 바꾸고 features 를 동결 (feature extraction) 할 때 학습되는 파라미터는?",
    options: ["601,098 개 (classifier 전체)", "10,250 개 (마지막 Linear 만)", "927,008 개 (features)", "1,528,106 개 (전체)"],
    answer: 0, why: "전체 1,528,106 = features 927,008 + classifier 601,098. features 의 requires_grad = False. 전체를 학습하면 fine-tuning.", anim: "D3-5" },
  { title: "datasets.ImageFolder(\"safety/train\") 의 하위 폴더가 helmet/, no_person/, no_helmet/ 일 때 레이블 1 은?",
    options: ["helmet", "no_person", "탐색기에서 두 번째로 만든 폴더", "no_helmet"],
    answer: 3, why: "폴더 이름을 정렬한 순서 (classes) 가 레이블 번호: helmet 0 · no_helmet 1 · no_person 2.", anim: "D3-6" },
  { title: "MobileNetV3-Small 에 96×96 을 넣고 features[-1] 로 Grad-CAM 을 구하면 CAM 크기는? (그리고 오늘 Day 4 와의 연결)",
    options: ["96×96 — 입력과 같아 위치를 정확히 안다", "3×3 — stride 32 라 거칠다 → 물체 위치가 중요하면 객체 탐지가 필요", "7×7 — 박스를 바로 얻을 수 있다", "8×8 — ResNet-20 과 같다"],
    answer: 1, why: "96 / 32 = 3. α = 기울기의 공간 평균, CAM = ReLU(Σ α_k A^k). 분류는 '무엇이' 까지 — 오늘은 '무엇이 어디에' (bbox · IoU · NMS · YOLO).", anim: "D3-7" }
];
