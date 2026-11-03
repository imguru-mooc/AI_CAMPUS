/**
 * Vision 기본 모델 Day 4 (객체 탐지) 복습 퀴즈 — Google Form 자동 생성 스크립트
 * HYUNDAI AI Insight Campus 온디바이스 AI · Vision 기본 모델 Day 5 (11/3) 09:00~09:15
 *
 * 사용 방법
 * 1. https://script.google.com 접속 → 새 프로젝트
 * 2. 이 파일 전체를 붙여 넣고 저장
 * 3. 상단 함수 선택에서 createVBDay4Quiz 선택 → ▶ 실행 (최초 1회 권한 승인)
 * 4. 실행 로그의 편집용(editUrl) · 응답용(publishedUrl) URL 확인 — 응답용을 학생에게 공유
 *
 * 퀴즈 모드(문항당 1점, 총 20점) · 이름 필수 · 제출 직후 점수와 해설 공개 · 문항 · 보기 순서 고정
 * 회사 Workspace 계정에서 401 invalid_client 오류가 나면 개인 Google 계정으로 실행
 */

function createVBDay4Quiz() {
  const form = FormApp.create("Vision 기본 모델 Day 4 복습 퀴즈");
  form.setIsQuiz(true)
      .setDescription(
        '범위: Vision 기본 모델 Day 4 객체 탐지 (분류 vs 탐지 · bbox 좌표계 · IoU · NMS · Precision/Recall · AP · mAP · 2-stage vs 1-stage · YOLO 구조 · 영상 파이프라인)\n' +
        '문항당 1점, 총 20점 · 권장 시간 15분\n' +
        '제출 후 점수와 해설이 바로 표시됩니다. 틀린 문항은 해설의 애니메이션 링크로 복습하세요.\n' +
        '애니메이션: https://imguru-mooc.github.io/AI_CAMPUS/VB-day4/animation.html')
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

// 'D4-3' → https://imguru-mooc.github.io/AI_CAMPUS/VB-day4/animation.html#d4-3
function animUrl(anim) {
  return ANIM_BASE + 'VB-day' + anim.charAt(1) + '/animation.html#' + anim.toLowerCase();
}

// 정답 위치 분포 ①~④: 5 · 5 · 5 · 5
var QUESTIONS = [
  // ── D4-1 분류 vs 탐지 (2) ──
  { title: "559×536 이미지를 64×128 창, stride 16 으로 훑는 sliding window 의 창 개수는? (창 크기 하나만)",
    options: ["31", "26", "806", "3,224"],
    answer: 2, why: "가로 (559 − 64) // 16 + 1 = 31, 세로 (536 − 128) // 16 + 1 = 26 → 806 창. 창마다 MobileNetV3-S (약 6 ms) 를 돌리면 약 4.8초/장 → 이미지를 한 번에 보는 탐지기가 필요합니다.", anim: "D4-1" },
  { title: "Penn-Fudan Pedestrian 데이터셋에 대한 설명으로 옳은 것은?",
    options: ["이미지 170장 · 보행자 423명, bbox 와 마스크가 함께 있다", "이미지 423장 · 보행자 170명", "어노테이션이 COCO JSON 파일로 제공된다", "사진 속 모든 사람이 빠짐없이 정답에 들어 있다"],
    answer: 0, why: "Fudan 74 + Penn 96 = 170장. 어노테이션은 텍스트 파일 (정규식 파싱). 너무 작거나 가려진 사람은 정답에 없어 탐지기가 맞게 찾아도 FP 가 될 수 있습니다. 오늘은 그 마스크 PNG 로 세그멘테이션을 합니다.", anim: "D4-1" },

  // ── D4-2 bbox 좌표계 (3) ──
  { title: "COCO JSON 의 annotations[i][\"bbox\"] 형식은?",
    options: ["xyxy (x1, y1, x2, y2)", "cxcywh 정규화 (0~1)", "cxcywh 픽셀", "xywh (왼쪽 위 x, y, 너비, 높이)"],
    answer: 3, why: "xyxy — Pascal VOC · torchvision, xywh — COCO, cxcywh 정규화 — YOLO 라벨. 같은 박스라도 라이브러리마다 형식이 다릅니다.", anim: "D4-2" },
  { title: "박스 [160, 182, 302, 431] (xyxy, 이미지 559×536) 를 YOLO 라벨 (cxcywh 정규화) 로 바꾸면?",
    options: ["[0.2862, 0.3396, 0.5403, 0.8041]", "[0.4132, 0.5718, 0.2540, 0.4646]", "[0.2862, 0.3396, 0.2540, 0.4646]", "[0.4310, 0.5483, 0.2649, 0.4454]"],
    answer: 1, why: "cx = 231 / 559, cy = 306.5 / 536, w = 142 / 559, h = 249 / 536. 오답은 xyxy 를 그대로 나눈 값 · 중심 대신 왼쪽 위를 쓴 값 · W 와 H 를 바꿔 나눈 값.", anim: "D4-2" },
  { title: "박스 [160, 182, 302, 431] (xyxy) 의 넓이 (COCO area) 는?",
    options: ["41,882", "35,358", "35,640", "17,679"],
    answer: 1, why: "w = 302 − 160 = 142, h = 431 − 182 = 249 → 142 × 249 = 35,358. Penn-Fudan 을 COCO 로 바꾸면 annotations 는 사람마다 하나 → 423개.", anim: "D4-2" },

  // ── D4-3 IoU · NMS (4) ──
  { title: "[0, 0, 10, 10] 과 [5, 5, 15, 15] (xyxy) 의 IoU 는?",
    options: ["25 / 200 = 0.125", "25 / 100 = 0.25", "25 / 175 ≈ 0.1429", "50 / 175 ≈ 0.2857"],
    answer: 2, why: "교집합 5 × 5 = 25, 합집합 100 + 100 − 25 = 175. 25 / 200 은 교집합을 빼지 않은 값, 25 / 100 은 한 박스 넓이로 나눈 값.", anim: "D4-3" },
  { title: "[50, 50, 150, 150] 과 [60, 60, 160, 160] 의 IoU 는?",
    options: ["0.6807", "0.8100", "0.9000", "0.7225"],
    answer: 0, why: "교집합 90 × 90 = 8,100, 합집합 10,000 + 10,000 − 8,100 = 11,900 → 0.6807. 10 픽셀씩만 어긋나도 IoU 는 0.68 로 떨어집니다. 오늘은 같은 식을 마스크 픽셀로 (mIoU).", anim: "D4-3" },
  { title: "NMS 예제 (박스 0~4, 점수 0.95 · 0.90 · 0.85 · 0.60 · 0.40, IoU(0,1) 0.8005 · IoU(0,3) 0.7808 · IoU(2,4) 0.7500, 나머지 쌍은 안 겹침) 에 iou_thr 0.5 로 greedy NMS 를 하면 남는 인덱스는?",
    options: ["[0, 1, 2]", "[0, 2, 4]", "[0, 1, 2, 3, 4]", "[0, 2]"],
    answer: 3, why: "0 keep → 1 · 3 제거 (IoU > 0.5) → 2 keep → 4 제거. torchvision.ops.nms 와 같은 결과. iou_thr 0.85 면 5개 모두 남습니다.", anim: "D4-3" },
  { title: "NMS 의 IoU threshold 를 너무 낮게 잡으면 생기는 문제는?",
    options: ["같은 사람에 중복 박스가 많이 남는다", "점수 낮은 박스가 모두 남아 오탐이 늘어난다", "박스 좌표가 원본 이미지 밖으로 나간다", "붙어 서 있는 다른 사람의 박스까지 지울 수 있다"],
    answer: 3, why: "IoU threshold = 얼마나 겹치면 같은 물체로 볼지. 낮으면 조금만 겹쳐도 지워 이웃한 사람을 놓치고, 높으면 중복이 남습니다. 점수로 거르는 것은 conf threshold 의 일.", anim: "D4-3" },

  // ── D4-4 PR 곡선과 AP (3) ──
  { title: "정답 4개, 탐지 5개 (점수 순 TP, TP, FP, TP, FP) 의 AP (all-point 보간) 는?",
    options: ["0.7500", "0.6875", "0.6000", "0.8125"],
    answer: 1, why: "P = 1.00, 1.00, 0.67, 0.75, 0.60 / R = 0.25, 0.50, 0.50, 0.75, 0.75 → envelope 1, 1, 0.75, 0.75, 0.60 → 0.25×1 + 0.25×1 + 0.25×0.75 = 0.6875.", anim: "D4-4" },
  { title: "Precision 과 Recall 의 정의로 옳은 것은?",
    options: ["Precision = TP / 정답 수, Recall = TP / (TP + FP)", "Precision = TP / (TP + FN), Recall = FP / 정답 수", "Precision = TP / (TP + FP), Recall = TP / 정답 수", "둘 다 TP / 전체 탐지 수"],
    answer: 2, why: "Precision 의 분모는 탐지 수, Recall 의 분모는 정답 수. 같은 정답에 두 번째로 매칭된 탐지는 FP, 못 찾은 정답은 FN (Recall 을 낮춤).", anim: "D4-4" },
  { title: "COCO 주 지표 mAP@0.5:0.95 는?",
    options: ["IoU 0.50, 0.55, …, 0.95 (10개) 기준 AP 의 평균", "IoU 0.5 와 0.95 두 기준 AP 의 평균", "conf 0.5 ~ 0.95 에서 구한 Precision 의 평균", "IoU 0.5 기준 AP 에 0.95 를 곱한 값"],
    answer: 0, why: "엄격한 기준이 섞여 박스 위치 정확도까지 봅니다. Day 4 는 클래스가 person 하나라 mAP = AP.", anim: "D4-4" },

  // ── D4-5 Anchor 와 RPN (3) ──
  { title: "559×536 이미지, stride 16 특징맵 (34×35) 의 위치마다 크기 3 × 비율 3 의 anchor 를 두면 anchor 는 모두 몇 개?",
    options: ["10,710", "1,190", "3,570", "11,025"],
    answer: 0, why: "34 × 35 = 1,190 위치 × 9 = 10,710. 모델은 anchor 마다 오프셋 (dx, dy, dw, dh) 과 점수를 예측합니다.", anim: "D4-5" },
  { title: "같은 MobileNetV3-Large backbone 의 Faster R-CNN (2-stage) 과 SSDLite320 (1-stage) 비교로 옳은 것은?",
    options: ["SSDLite320 이 연산도 많고 mAP 도 높다", "둘의 연산량 · mAP 가 같다 (backbone 이 같으므로)", "Faster R-CNN 0.58 GFLOPS · mAP 21.3, SSDLite320 4.49 GFLOPS · mAP 32.8", "Faster R-CNN 4.49 GFLOPS · mAP 32.8, SSDLite320 0.58 GFLOPS · mAP 21.3"],
    answer: 3, why: "2-stage 는 RPN → proposal → RoI Align (7×7) → 분류 · 박스 재보정으로 정확하지만 느리고, 1-stage 는 특징맵에서 한 번에 내 빠르지만 작은 물체에 약합니다.", anim: "D4-5" },
  { title: "person 의 클래스 번호로 옳은 것은?",
    options: ["둘 다 0", "torchvision COCO 탐지기 1 (0 = background), ultralytics YOLOv8 0", "둘 다 1", "torchvision 0, YOLOv8 1"],
    answer: 1, why: "라이브러리마다 다릅니다. torchvision 출력은 이미지마다 dict {boxes (N, 4) xyxy, labels, scores}. 오늘 7교시도 YOLOv8 cls 0 (사람) 만 씁니다.", anim: "D4-5" },

  // ── D4-6 YOLO grid 와 출력 텐서 (3) ──
  { title: "YOLOv8 에 640×640 을 넣으면 예측 (후보 박스) 은 모두 몇 개?",
    options: ["6,400", "2,100", "8,400", "25,200"],
    answer: 2, why: "(640/8)² + (640/16)² + (640/32)² = 6,400 + 1,600 + 400 = 8,400. 6,400 은 stride 8 하나, 2,100 은 320 입력. 오늘 탐지 · 세그 · 포즈 모델 모두 8,400 칸입니다.", anim: "D4-6" },
  { title: "YOLOv8n (COCO) 원시 출력의 모양은? (640 입력, 배치 1)",
    options: ["(1, 85, 8400) — 박스 4 + objectness 1 + 클래스 80", "(1, 80, 8400) — 클래스 80", "(1, 84, 8400) — 박스 4 + 클래스 80", "(1, 8400, 4) — 박스만"],
    answer: 2, why: "YOLOv8 은 objectness 가 없습니다 (85 는 YOLOv5 까지). 오늘 ONNX 로 내보낸 yolov8n.onnx 의 출력 output0 도 (1, 84, 8400) — NMS 가 없어 decode_yolo 로 후처리합니다.", anim: "D4-6" },
  { title: "768×576 영상 프레임을 letterbox(img, size=640) 하면?",
    options: ["scale 1.1111 → 853×640, 좌우를 잘라냄", "640×640 으로 그냥 늘림 (비율 무시)", "scale 0.8333 → 640×480, 좌우에 패딩 80 픽셀씩", "scale 0.8333 → 640×480, 위아래에 회색 (114) 패딩 80 픽셀씩"],
    answer: 3, why: "scale = min(640/576, 640/768) = 0.8333. 비율을 유지해 사람이 찌그러지지 않습니다. 원래 좌표 = (박스 − [left, top, left, top]) / scale.", anim: "D4-6" },

  // ── D4-7 프레임 파이프라인 (1) ──
  { title: "data/people.mp4 (10 FPS) 를 실시간으로 처리하려면 프레임 한 장의 전체 처리 (전처리 ~ 저장) 시간은?",
    options: ["100 ms 이하", "10 ms 이하", "33 ms 이하", "1,000 ms 이하"],
    answer: 0, why: "1000 / 10 = 100 ms (30 FPS 면 33 ms). people.mp4 = 768×576, 10 FPS, 795 프레임 (79.5초). 추론만이 아니라 전처리 · 후처리 · 그리기 · 저장을 모두 더한 시간입니다.", anim: "D4-7" },

  // ── D4-8 탐지기 고르기 (1) ──
  { title: "Day 4 미니 프로젝트 해석으로 옳은 것은?",
    options: ["conf threshold 를 낮추면 Precision 은 오르고 Recall 은 내려간다", "Penn-Fudan 처럼 사람이 크면 작은 모델 · 낮은 해상도로도 AP50 이 거의 같아, mAP@0.5:0.95 (위치 정밀도) 와 속도로 고른다", "AP 를 구할 때는 conf 0.9 처럼 높은 threshold 로 탐지해야 정확하다", "입력을 640 에서 320 으로 줄이면 예측 개수가 절반 (4,200) 이 된다"],
    answer: 1, why: "conf 를 낮추면 Recall↑ Precision↓. AP 는 conf 0.001 로 PR 곡선을 높은 Recall 까지 그려 구합니다. 320 입력은 예측 2,100 (1/4).", anim: "D4-8" }
];
