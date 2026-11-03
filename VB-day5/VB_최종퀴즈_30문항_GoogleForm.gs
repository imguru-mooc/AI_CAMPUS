/**
 * Vision 기본 모델 최종 필기 퀴즈 (30문항, Day 1~5) — Google Form 자동 생성 스크립트
 * HYUNDAI AI Insight Campus 온디바이스 AI · Vision 기본 모델 Day 5 (11/3) 8교시 17:25~17:45
 * (Day 1 5문항 · Day 2 5문항 · Day 3 6문항 · Day 4 7문항 · Day 5 7문항)
 *
 * 사용 방법
 * 1. https://script.google.com 접속 → 새 프로젝트
 * 2. 이 파일 전체를 붙여 넣고 저장
 * 3. 상단 함수 선택에서 createVBFinalQuiz 선택 → ▶ 실행 (최초 1회 권한 승인)
 * 4. 실행 로그의 편집용(editUrl) · 응답용(publishedUrl) URL 확인 — 응답용을 학생에게 공유
 *
 * 퀴즈 모드(문항당 1점, 총 30점) · 이름 필수 · 제출 직후 점수와 해설 공개 · 문항 · 보기 순서 고정
 * 회사 Workspace 계정에서 401 invalid_client 오류가 나면 개인 Google 계정으로 실행
 */

function createVBFinalQuiz() {
  const form = FormApp.create("Vision 기본 모델 최종 필기 퀴즈 (30문항)");
  form.setIsQuiz(true)
      .setDescription(
        '범위: Vision 기본 모델 Day 1 ~ 5 (Day 1 PyTorch · 이미지 · 영상 5문항 / Day 2 Conv · VGG · BatchNorm · ResNet 5문항 / Day 3 경량 CNN · 전이학습 · Grad-CAM 6문항 / Day 4 객체 탐지 7문항 / Day 5 세그 · 포즈 · ONNX · 성능 측정 · 침입 감지 7문항)\n' +
        '문항당 1점, 총 30점 · 권장 시간 20분\n' +
        '제출 후 점수와 해설이 바로 표시됩니다. 틀린 문항은 해설의 애니메이션 링크로 복습하세요.\n' +
        '애니메이션: https://imguru-mooc.github.io/AI_CAMPUS/VB-day1/animation.html · VB-day2 · VB-day3 · VB-day4 · VB-day5/animation.html')
      .setShuffleQuestions(false)
      .setProgressBar(true);

  form.addTextItem().setTitle('이름').setRequired(true);
  form.addPageBreakItem().setTitle('문항 (30)');
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

// 'D4-3' → https://imguru-mooc.github.io/AI_CAMPUS/VB-day4/animation.html#d4-3
function animUrl(anim) {
  return ANIM_BASE + 'VB-day' + anim.charAt(1) + '/animation.html#' + anim.toLowerCase();
}

// 정답 위치 분포 ①~④: 8 · 7 · 7 · 8
var QUESTIONS = [
  // ── Day 1 (5) ──
  { title: "a = np.zeros(3); b = torch.from_numpy(a); c = torch.tensor(a); b[0] = 7; c[1] = 9 를 실행한 뒤 a 의 값은?",
    options: ["[0, 0, 0]", "[7, 9, 0]", "[7, 0, 0]", "[0, 9, 0]"],
    answer: 2, why: "from_numpy 는 메모리를 공유하고 torch.tensor 는 복사합니다. b 를 바꾸면 a 도 바뀌고, c = [0, 9, 0] 은 따로.", anim: "D1-2" },
  { title: "PyTorch 로 옮긴 SimpleConvNet — Conv2d(1, 30, 5) · Linear(4320, 100) · Linear(100, 10) — 의 전체 파라미터 수는?",
    options: ["433,890", "432,100", "433,110", "79,510"],
    answer: 0, why: "780 + 432,100 + 1,010 = 433,890 (NumPy 판과 같음). 79,510 은 MLP 784-100-10.", anim: "D1-1" },
  { title: "x = torch.tensor(1.0, requires_grad=True) 에서 (x**2).backward() 를 두 번 호출하면 x.grad 는?",
    options: ["2 — 매번 새로 계산된다", "0 — 두 번째 호출이 지운다", "오류 — 한 번만 호출할 수 있다", "4 — 기울기가 .grad 에 누적된다"],
    answer: 3, why: "기울기는 더해집니다 (2 + 2). 그래서 학습 루프는 zero_grad → backward → step 순서로 매 배치 지웁니다.", anim: "D1-3" },
  { title: "uint8 배열 [[200, 250]] 에 NumPy 로 100 을 더하면? (dtype 유지)",
    options: ["[[255, 255]] — 자동으로 255 에 고정", "[[44, 94]] — 256 을 넘으면 다시 0 부터 (overflow)", "[[300, 350]]", "오류가 난다"],
    answer: 1, why: "uint8 은 0~255. cv2.add 는 [[255, 255]] (saturation), float 로 바꾸면 [300, 350]. 그래서 이미지 연산 전에 float32 로 바꿉니다.", anim: "D1-6" },
  { title: "cv2.VideoWriter(path, fourcc, fps, size) 의 size 인자에 대한 설명으로 옳은 것은?",
    options: ["(W, H) 순서이고, 틀리면 오류 없이 빈 영상이 만들어진다", "(H, W) 순서이고, 틀리면 바로 오류가 난다", "(C, H, W) 순서다", "생략하면 첫 프레임 크기로 자동 설정된다"],
    answer: 0, why: "cv2.resize 와 같이 (W, H). Day 4 인원 카운트 · Day 5 침입 감지 결과 영상도 같은 VideoWriter 로 저장합니다.", anim: "D1-7" },

  // ── Day 2 (5) ──
  { title: "224×224 입력에 nn.Conv2d(3, 64, kernel_size=7, stride=2, padding=3) 을 적용하면 출력 크기는?",
    options: ["224×224", "112×112", "109×109", "56×56"],
    answer: 1, why: "O = ⌊(N + 2P − K) / S⌋ + 1 = ⌊(224 + 6 − 7) / 2⌋ + 1 = 112. ResNet 첫 층. Day 5 의 Transposed Conv 는 이 식을 거꾸로 푼 것입니다.", anim: "D2-2" },
  { title: "nn.Conv2d(64, 128, 3) (bias 포함) 의 파라미터 수는?",
    options: ["73,728", "73,856", "8,320", "36,928"],
    answer: 1, why: "Cout · Cin · K² + Cout = 128 · 64 · 9 + 128 = 73,856. bias 를 빼면 73,728.", anim: "D2-2" },
  { title: "VGG 가 5×5 한 층 대신 3×3 을 두 층 쌓는 이유로 옳지 않은 것은?",
    options: ["같은 5×5 수용 영역을 본다", "파라미터가 18C² vs 25C² 로 약 28% 적다", "ReLU 가 두 번 들어가 더 복잡한 모양을 표현한다", "출력 해상도가 두 배로 커진다"],
    answer: 3, why: "padding=1 이면 크기는 그대로입니다. 세 층이면 7×7 수용 영역 (27C² vs 49C²).", anim: "D2-4" },
  { title: "BatchNorm2d 가 있는 모델을 평가할 때 model.eval() 을 부르는 이유는?",
    options: ["기울기 계산을 끄기 위해서", "학습률을 0 으로 만들려고", "BN 이 배치 통계 대신 학습 중 모은 running 평균 · 분산을 쓰게 하려고", "가중치를 float16 으로 바꾸려고"],
    answer: 2, why: "기울기 끄기는 torch.no_grad() — 다른 일입니다. ONNX export 전에도 eval() 을 불러야 BN · Dropout 이 추론 모드로 기록됩니다 (Day 5).", anim: "D2-5" },
  { title: "ResNet 의 residual 블록 y = ReLU(F(x) + x) 에서 지름길이 깊은 망 학습을 돕는 이유는?",
    options: ["파라미터 수가 절반으로 줄어든다", "BatchNorm 이 필요 없어진다", "출력 해상도를 2배로 키운다", "덧셈 노드가 기울기를 그대로 흘려 ∂y/∂x = F'(x) + 1 이 되어 기울기가 사라지지 않는다"],
    answer: 3, why: "항등 함수를 배우기 쉬워져 56층 plain 이 20층보다 나빠지는 성능 저하 (degradation) 를 풉니다. ResNet-20 파라미터 272,474.", anim: "D2-6" },

  // ── Day 3 (6) ──
  { title: "Depthwise Separable 합성곱의 연산량 ÷ 일반 합성곱의 연산량은? (커널 K×K, 출력 채널 Cout)",
    options: ["1/K²", "1/(Cout · K²)", "1/Cin + 1/K", "1/Cout + 1/K²"],
    answer: 3, why: "depthwise (채널마다 공간만) + pointwise 1×1 (채널만). 64→128, K=3 이면 1/128 + 1/9 = 0.1189.", anim: "D3-3" },
  { title: "64 → 128 채널 3×3 합성곱 (bias 없음) 을 Depthwise Separable 로 바꾸면 가중치 수는?",
    options: ["73,728 → 8,768", "73,728 → 8,192", "73,856 → 576", "36,864 → 8,768"],
    answer: 0, why: "depthwise 64 · 9 = 576 + pointwise 64 · 128 = 8,192 → 8,768 (11.9%). DS 연산의 약 93% 가 pointwise 1×1.", anim: "D3-3" },
  { title: "MobileNetV2 inverted residual 블록 (t = 6) 의 구조로 옳은 것은?",
    options: ["1×1 늘리기 (×6) → depthwise 3×3 → 1×1 줄이기, 마지막 1×1 뒤 활성화 없음", "1×1 줄이기 → 3×3 → 1×1 늘리기, 모든 Conv 뒤 ReLU", "depthwise 3×3 두 번, 지름길 없음", "3×3 → 3×3 basic 블록 + SE"],
    answer: 0, why: "24 → 144 → 24 처럼 늘렸다 줄여 inverted, 얇은 출력에 ReLU 를 걸지 않아 linear bottleneck. 지름길은 stride 1 이고 cin == cout 일 때만.", anim: "D3-4" },
  { title: "MobileNetV3 의 h-swish(x) 정의로 옳은 것은?",
    options: ["x · σ(x)", "max(0, x)", "x · ReLU6(x + 3) / 6", "ReLU6(x) / 6"],
    answer: 2, why: "sigmoid 없이 swish 를 근사해 장치에서 계산이 쌉니다 (nn.Hardswish). Day 5 의 MobileNetV3-S ONNX 그래프에도 HardSwish 노드가 보입니다.", anim: "D3-4" },
  { title: "사전학습 MobileNetV3-Small 의 classifier[3] 을 nn.Linear(1024, 10) 으로 바꾸고 features 를 동결 (feature extraction) 하면 학습되는 파라미터는?",
    options: ["10,250 개 (마지막 Linear 만)", "601,098 개 (classifier 전체)", "927,008 개 (features)", "1,528,106 개 (전체)"],
    answer: 1, why: "전체 1,528,106 = features 927,008 + classifier 601,098. 전체를 학습하면 fine-tuning.", anim: "D3-5" },
  { title: "Grad-CAM 계산으로 옳은 것은?",
    options: ["CAM = 입력 이미지의 기울기 절댓값", "α_k = 가중치 W 의 평균, CAM = softmax(Σ α_k A^k)", "α_k = 클래스 점수의 A^k 기울기 공간 평균, CAM = ReLU(Σ α_k A^k)", "CAM = 마지막 conv 출력 A 의 채널 최댓값"],
    answer: 2, why: "ReLU 는 그 클래스 점수를 올리는 위치만 남기려고. MobileNetV3-S 96 입력의 CAM 은 3×3 (stride 32) 이라 거칠어, 위치가 중요하면 탐지 (Day 4) · 세그 (Day 5).", anim: "D3-7" },

  // ── Day 4 (7) ──
  { title: "[0, 0, 10, 10] 과 [5, 5, 15, 15] (xyxy) 의 IoU 는?",
    options: ["25 / 200 = 0.125", "25 / 100 = 0.25", "25 / 175 ≈ 0.1429", "50 / 175 ≈ 0.2857"],
    answer: 2, why: "교집합 5 × 5 = 25, 합집합 100 + 100 − 25 = 175.", anim: "D4-3" },
  { title: "NMS 예제 (박스 0~4, 점수 0.95 · 0.90 · 0.85 · 0.60 · 0.40, IoU(0,1) 0.8005 · IoU(0,3) 0.7808 · IoU(2,4) 0.7500, 나머지 쌍은 안 겹침) 에 iou_thr 0.5 로 NMS 를 하면 남는 인덱스는?",
    options: ["[0, 2]", "[0, 1, 2]", "[0, 2, 4]", "[0, 1, 2, 3, 4]"],
    answer: 0, why: "0 keep → 1 · 3 제거 → 2 keep → 4 제거.", anim: "D4-3" },
  { title: "정답 4개, 탐지 5개 (점수 순 TP, TP, FP, TP, FP) 의 AP (all-point 보간) 는?",
    options: ["0.7500", "0.6000", "0.8125", "0.6875"],
    answer: 3, why: "envelope 1, 1, 0.75, 0.75, 0.60 → 0.25×1 + 0.25×1 + 0.25×0.75 = 0.6875.", anim: "D4-4" },
  { title: "mAP@0.5:0.95 에서 평균하는 IoU 기준은 몇 개?",
    options: ["2개 (0.5, 0.95)", "5개 (0.5, 0.6, …, 0.9)", "10개 (0.50, 0.55, …, 0.95)", "46개 (0.50, 0.51, …, 0.95)"],
    answer: 2, why: "COCO 주 지표 — 엄격한 기준이 섞여 박스 위치 정확도까지 봅니다.", anim: "D4-4" },
  { title: "YOLOv8n 에 640×640 을 넣었을 때 원시 출력의 모양은?",
    options: ["(1, 84, 8400)", "(1, 85, 8400)", "(1, 84, 6400)", "(1, 80, 8400)"],
    answer: 0, why: "8,400 = 6,400 + 1,600 + 400 (stride 8 · 16 · 32), 84 = 박스 4 + 클래스 80. objectness 없음.", anim: "D4-6" },
  { title: "박스 형식과 쓰는 곳의 짝으로 옳은 것은?",
    options: ["YOLO 라벨 = 픽셀 단위 xyxy", "torchvision 출력 boxes = cxcywh 정규화", "Pascal VOC = xywh 정규화", "COCO JSON bbox = xywh (왼쪽 위 x, y, 너비, 높이)"],
    answer: 3, why: "xyxy — Pascal VOC · torchvision, xywh — COCO, cxcywh 정규화 (0~1) — YOLO 라벨.", anim: "D4-2" },
  { title: "Faster R-CNN (2-stage) 의 처리 순서로 옳은 것은?",
    options: ["backbone → 특징맵마다 클래스 + 박스를 한 번에 → NMS", "backbone → RPN (anchor 마다 물체? + 보정) → proposal → RoI Align (7×7) → 분류 + 박스 재보정", "RoI Align → backbone → RPN → 분류", "이미지를 grid 로 나눠 칸마다 예측 → anchor-free 박스"],
    answer: 1, why: "두 번 박스를 다듬어 정확하지만 느립니다. 특징맵에서 한 번에 내는 것은 1-stage (SSD · YOLO).", anim: "D4-5" },

  // ── Day 5 (7) ──
  { title: "8×8 특징맵에 Transposed Conv (kernel 4, stride 2, padding 1) 를 적용하면 출력 크기는?",
    options: ["15×15", "17×17", "32×32", "16×16"],
    answer: 3, why: "(입력 − 1) × stride − 2 × padding + kernel = 7 × 2 − 2 + 4 = 16. k=2, s=2, p=0 도 16.", anim: "D5-2" },
  { title: "4×4 마스크에서 정답 사람 = 왼쪽 2열 (8픽셀), 예측 사람 = 왼쪽 3열 (12픽셀) 일 때 mIoU (배경 · 사람) 는?",
    options: ["0.6667", "0.5000", "0.5833", "0.7500"],
    answer: 2, why: "사람 IoU 8/12 = 0.6667, 배경 IoU 4/8 = 0.5000 → 평균 0.5833. 0.7500 은 픽셀 정확도.", anim: "D5-2" },
  { title: "Day 5 쓰러짐 규칙으로 옳은 것은?",
    options: ["엉덩이 중점 (11, 12) → 어깨 중점 (5, 6) 벡터와 위쪽 수직 (0, −1) 사이 각 > 60° 이면 쓰러짐 (4개 관절 conf ≥ 0.5 일 때만)", "박스 h/w 가 1 보다 작으면 항상 쓰러짐", "코 (0) 의 y 좌표가 발목 (15, 16) 보다 크면 쓰러짐", "몸통 벡터와 (0, 1) 사이 각 < 60° 이면 쓰러짐"],
    answer: 0, why: "이미지 y 는 아래로 증가 → 위쪽 = (0, −1). 서 있음 ≈ 0°, 누움 ≈ 90°. 박스 비율은 허리 숙임 · 카메라 각도에도 바뀌어 애매합니다.", anim: "D5-3" },
  { title: "같은 YOLOv8n 몸통에 head 만 다른 세 모델의 출력 채널 (640 입력, 8,400 칸) 으로 옳은 것은?",
    options: ["탐지 84 · 세그 84 · 포즈 84", "탐지 84 · 세그 116 (+ prototype 32×160×160) · 포즈 56", "탐지 85 · 세그 117 · 포즈 57", "탐지 84 · 세그 32 · 포즈 17"],
    answer: 1, why: "세그 116 = 4 + 80 + 마스크 계수 32, 포즈 56 = 4 + 사람 점수 1 + 17 × (x, y, conf).", anim: "D5-4" },
  { title: "ONNX export · 실행에 대한 설명으로 옳은 것은?",
    options: ["opset_version 은 입력 이미지 크기를 정한다", "ONNX 파일은 Jetson 에서만 실행할 수 있다", "export 한 모델은 PyTorch 출력과 비교할 필요가 없다", "dynamic_axes 로 배치 축을 열어 두면 같은 .onnx 에 배치 4 도 넣을 수 있고, yolov8n.onnx 출력에는 NMS 가 없다"],
    answer: 3, why: "torch.onnx.export 는 예시 입력으로 tracing. ORT 출력과 PyTorch 출력의 최대 차이 < 1e-4 로 검증. MobileNetV3-S 노드 159개 · 약 10.2 MB.", anim: "D5-5" },
  { title: "온디바이스 성능 측정 규칙으로 옳은 것은?",
    options: ["warm-up 5회를 버리고 50회 측정 → 평균 · p50 · p95, FPS = 1000 / 평균 ms", "첫 실행 한 번만 재서 적는다", "FPS = 평균 ms / 1000", "p95 는 평균의 95% 값이다"],
    answer: 0, why: "p95 = 100번 중 95번은 이보다 빠름 (꼬리). 스레드 1 · 2 · 전체로 바꿔 재고, 결과는 baseline.csv 로 다음 과목 (TensorRT) 과 비교합니다.", anim: "D5-6" },
  { title: "위험구역 침입 감지에서 박스 [100, 200, 160, 380] 인 사람의 판정에 쓰는 점과 경보 조건은?",
    options: ["박스 중심 (130, 290) 이 구역 안이면 즉시 경보", "발 위치 (130, 380) 를 cv2.pointPolygonTest(poly, pt, False) >= 0 으로 판정, 3 프레임 연속 구역 안이면 경보", "박스가 구역과 조금이라도 겹치면 즉시 경보", "발 위치 (100, 380) 가 10 프레임 연속 밖이면 경보"],
    answer: 1, why: "발 위치 = 박스 아래 중앙 ((x1 + x2) / 2, y2). debounce (3 프레임) 로 한두 프레임 흔들림에 경보가 깜빡이지 않게 합니다.", anim: "D5-7" }
];
