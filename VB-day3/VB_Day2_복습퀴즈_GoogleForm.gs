/**
 * Vision 기본 모델 Day 2 복습 퀴즈 — Google Form 자동 생성 스크립트
 * HYUNDAI AI Insight Campus 온디바이스 AI · Vision 기본 모델 Day 3 (10/29) 09:00~09:15
 *
 * 사용 방법
 * 1. https://script.google.com 접속 → 새 프로젝트
 * 2. 이 파일 전체를 붙여 넣고 저장
 * 3. 상단 함수 선택에서 createVBDay2Quiz 선택 → ▶ 실행 (최초 1회 권한 승인)
 * 4. 실행 로그의 편집용(editUrl) · 응답용(publishedUrl) URL 확인 — 응답용을 학생에게 공유
 *
 * 퀴즈 모드(문항당 1점, 총 20점) · 이름 필수 · 제출 직후 점수와 해설 공개 · 문항 · 보기 순서 고정
 * 회사 Workspace 계정에서 401 invalid_client 오류가 나면 개인 Google 계정으로 실행
 */

function createVBDay2Quiz() {
  const form = FormApp.create('Vision 기본 모델 Day 2 복습 퀴즈');
  form.setIsQuiz(true)
      .setDescription(
        '범위: Vision 기본 모델 Day 2 전체 (CIFAR-10 · MLP 의 한계 · Conv 출력 크기 · 파라미터 · 수용 영역 · LeNet-5 · AlexNet · VGG · BatchNorm · ResNet · 데이터 증강 · OneCycleLR · 혼동 행렬 · MAC)\n' +
        '문항당 1점, 총 20점 · 권장 시간 15분\n' +
        '제출 후 점수와 해설이 바로 표시됩니다. 틀린 문항은 해설의 애니메이션 링크로 복습하세요.\n' +
        '애니메이션: https://imguru-mooc.github.io/AI_CAMPUS/VB-day2/animation.html')
      .setShuffleQuestions(false)
      .setProgressBar(true);

  form.addTextItem().setTitle('이름').setRequired(true);
  form.addPageBreakItem().setTitle('문항 (20)');

  QUESTIONS.forEach(function (q, i) {
    const item = form.addMultipleChoiceItem();
    item.setTitle((i + 1) + '. ' + q.title).setPoints(1).setRequired(true);
    item.setChoices(q.options.map(function (opt, j) { return item.createChoice(opt, j === q.answer); }));
    const review = q.anim ? '  [복습: ' + q.anim + ' ' + ANIM_URL + '#' + q.anim.toLowerCase() + ']' : '';
    item.setFeedbackForCorrect(FormApp.createFeedback().setText('정답입니다. ' + q.why + review).build());
    item.setFeedbackForIncorrect(FormApp.createFeedback().setText('정답: ' + LABEL[q.answer] + ' ' + q.options[q.answer] + ' — ' + q.why + review).build());
  });

  Logger.log('편집용  URL: ' + form.getEditUrl());
  Logger.log('응답용  URL: ' + form.getPublishedUrl());
}

var LABEL = ['①', '②', '③', '④'];
var ANIM_URL = 'https://imguru-mooc.github.io/AI_CAMPUS/VB-day2/animation.html';

var QUESTIONS = [
  { title: "CIFAR-10 에 대한 설명으로 옳은 것은?",
    options: ["28×28 흑백, 10 클래스, train 60,000장", "32×32 컬러(RGB), 10 클래스, train 50,000 · test 10,000장", "224×224 컬러, 1,000 클래스", "32×32 컬러, 100 클래스, train 50,000장"],
    answer: 1, why: "한 장 3072 값 (MNIST 784 의 약 4배), 클래스당 train 5,000 · test 1,000장.", anim: "D2-1" },
  { title: "정규화 상수 MEAN, STD 를 구할 때 옳은 방법은?",
    options: ["train 데이터만으로 채널별 평균 · 표준편차를 구해 test 에도 같은 값을 쓴다", "test 데이터로 구한다", "train 과 test 를 합쳐서 구한다", "이미지마다 따로 구해 정규화한다"],
    answer: 0, why: "test 를 섞으면 시험 문제를 미리 본 것과 같습니다. axis=(0, 1, 2) 로 평균해 채널 3개만 남깁니다.", anim: "D2-1" },
  { title: "MLP(3072-512-256-10) 가 파라미터 170만 개로도 CIFAR-10 에서 약 50% 에 머무는 가장 큰 이유는?",
    options: ["학습률이 너무 작아서", "출력층이 10개뿐이라서", "이미지를 한 줄로 펼쳐 이웃 픽셀 관계(공간 구조) 를 잃기 때문", "ReLU 를 썼기 때문"],
    answer: 2, why: "바로 아래 픽셀이 줄에서는 32칸 떨어지고, 물체가 1픽셀만 움직여도 입력 대부분이 바뀝니다. 실행 예 3 epoch 0.4994.", anim: "D2-1" },
  { title: "입력 32×32 에 nn.Conv2d(64, 128, kernel_size=3, stride=2, padding=1) 를 적용하면 출력 크기는?",
    options: ["32×32", "15×15", "8×8", "16×16"],
    answer: 3, why: "(32 + 2·1 − 3) // 2 + 1 = 16. 크기 절반 · 채널 2배는 VGG · ResNet 의 공통 패턴입니다.", anim: "D2-2" },
  { title: "nn.Conv2d(64, 64, 3, groups=64) (depthwise) 의 파라미터 수는? (bias 포함)",
    options: ["640", "36,928", "576", "4,160"],
    answer: 0, why: "가중치 64 · (64/64) · 9 = 576 + bias 64 = 640. 일반 3×3 (36,928) 의 약 1/58 — 오늘(Day 3) MobileNet 의 핵심입니다.", anim: "D2-2" },
  { title: "3×3 합성곱(stride 1) 을 세 층 쌓으면 출력 한 칸의 수용 영역은?",
    options: ["3×3", "9×9", "7×7", "5×5"],
    answer: 2, why: "rf ← rf + (K−1)·jump: 1 → 3 → 5 → 7. 3×3 세 층은 7×7 한 층과 같은 영역을 보면서 파라미터는 45% 적습니다.", anim: "D2-2" },
  { title: "CIFAR-10 용 LeNet-5 (Conv 3→6 5×5, Pool, Conv 6→16 5×5, Pool, FC 400-120-84-10) 의 파라미터 수는?",
    options: ["62,006", "1,707,274", "61,100,840", "307,946"],
    answer: 0, why: "456 + 2,416 + 48,120 + 10,164 + 850 = 62,006 — MLP 의 1/27 인데 3 epoch test 는 0.5489 로 MLP(0.4994) 보다 높았습니다.", anim: "D2-3" },
  { title: "AlexNet (2012) 에 대한 설명으로 옳지 않은 것은?",
    options: ["ILSVRC 2012 top-5 오류 15.3% 로 2위(26.2%) 를 크게 앞섰다", "ReLU · Dropout · 데이터 증강 · GPU 학습을 썼다", "파라미터의 대부분이 합성곱 층에 있다", "torchvision AlexNet 은 약 6,110만 파라미터다"],
    answer: 2, why: "classifier.1 = Linear(9216→4096) 하나가 37,748,736 개(61.8%). 합성곱 5층은 2,469,696 개(4%) 뿐 — 파라미터는 FC 에, 연산은 Conv 에.", anim: "D2-3" },
  { title: "VGG 가 5×5 대신 3×3 을 두 층 쌓는 이유로 옳지 않은 것은?",
    options: ["같은 5×5 수용 영역을 본다", "파라미터가 약 28% 적다 (18C² vs 25C²)", "ReLU 가 두 번 들어가 더 복잡한 모양을 표현한다", "출력 해상도가 두 배로 커진다"],
    answer: 3, why: "padding=1 이면 크기는 그대로입니다. C=64 에서 5×5 102,400 vs 3×3×2 73,728.", anim: "D2-4" },
  { title: "VGG-mini (vgg_block 3개 + Linear 2048→10) 를 Adam 3 epoch 학습한 결과에 대한 설명으로 옳은 것은? (실행 예)",
    options: ["LeNet-5 보다 낮았다", "test 약 0.75 로 LeNet-5(0.55) 보다 크게 높았다", "MLP 와 같았다", "학습이 발산했다"],
    answer: 1, why: "실행 예 0.7526. 깊이(합성곱 6층) · 3×3 · BN 의 효과. 파라미터 307,946 (conv 286,560 + BN 896 + Linear 20,490).", anim: "D2-4" },
  { title: "BN 없는 plain-20 에서 학습 전 층별 기울기를 재면 입력 쪽 첫 층 기울기는?",
    options: ["마지막 층보다 수백만 배 이상 작다 (기울기 소실)", "마지막 층과 비슷하다", "마지막 층보다 훨씬 크다 (폭발)", "정확히 0 이다"],
    answer: 0, why: "실행 예: 첫 층 1.1e-09 vs 마지막 층 2.2e-02. 층마다 1 보다 작은 값이 곱해져 입력 쪽으로 갈수록 사라집니다. BN 을 넣으면 3.4e-01.", anim: "D2-5" },
  { title: "5교시 실험(10,000장, SGD 0.05, 3 epoch) 결과로 옳은 것은? (실행 예)",
    options: ["BN 이 없어도 plain-8 은 잘 학습됐다", "BN 을 넣으면 plain-20 이 plain-8 보다 train 손실이 낮았다", "BN 이 없으면 8층 · 20층 모두 손실 2.30 근처에서 움직이지 않았고, BN 을 넣어도 20층이 8층보다 train 손실이 높았다", "BN 을 넣으면 두 모델 모두 test 0.9 이상"],
    answer: 2, why: "BN 없음: 2.304 · 2.303 (= ln 10). BN 있음: plain-8 1.699 · plain-20 1.785 → 깊을수록 최적화가 어려운 성능 저하(degradation) → ResNet.", anim: "D2-5" },
  { title: "nn.BatchNorm2d(16) 의 학습 파라미터 수와, 학습되지 않지만 state_dict 에 저장되는 buffer 는?",
    options: ["16, running_mean", "32, running_mean · running_var · num_batches_tracked", "64, 없음", "32, weight · bias"],
    answer: 1, why: "weight γ 16 + bias β 16 = 32. running 통계는 추론(model.eval()) 때 쓰므로 함께 저장됩니다.", anim: "D2-5" },
  { title: "Residual block y = ReLU(F(x) + x) 에서 지름길이 역전파에 하는 일은?",
    options: ["기울기를 0 으로 만든다", "기울기를 절반으로 나눈다", "F 의 기울기만 통과시킨다", "덧셈 노드가 기울기를 그대로 두 갈래로 보내 ∂y/∂x = F'(x) + 1 — 기울기가 줄지 않고 흐른다"],
    answer: 3, why: "F' 가 작아져도 1 이 남습니다. 블록이 쌓이면 출력에서 입력까지 '고속도로' 가 생깁니다.", anim: "D2-6" },
  { title: "BasicBlock(16, 32, stride=2) 의 지름길(shortcut) 은?",
    options: ["nn.Identity()", "1×1 Conv(16→32, stride 2) + BN — 크기와 채널을 맞추기 위해", "3×3 Conv 두 개", "MaxPool2d(2)"],
    answer: 1, why: "x 의 모양 (16, 32, 32) 을 (32, 16, 16) 으로 맞춰야 더할 수 있습니다. 블록 파라미터 14,528.", anim: "D2-6" },
  { title: "ResNet-20 (CIFAR) 에 대한 설명으로 옳은 것은?",
    options: ["6n + 2 에서 n = 3, 파라미터 272,474, MAC 40,813,184", "파라미터가 MLP 보다 많다", "FC 층이 4096 뉴런 두 개다", "입력을 224×224 로 키워 넣는다"],
    answer: 0, why: "stem 1 + 블록 9×2 + Linear 1 = 20 층. GAP 덕분에 FC 는 650 개뿐 — 파라미터는 MLP 의 1/6, MAC 은 약 24배.", anim: "D2-6" },
  { title: "데이터 증강 RandomCrop(32, padding=4) + RandomHorizontalFlip() 에 대한 설명으로 옳지 않은 것은?",
    options: ["최대 ±4 픽셀 이동 효과가 있다", "test 데이터에도 똑같이 적용해 평가한다", "50% 확률로 좌우 반전한다", "같은 사진을 꺼낼 때마다 다른 그림이 나온다"],
    answer: 1, why: "증강은 train 에만. test 는 언제나 원본 그대로 평가합니다.", anim: "D2-7" },
  { title: "OneCycleLR(opt, max_lr=0.1, epochs=15, steps_per_epoch=391) 의 학습률 변화로 옳은 것은?",
    options: ["0.1 로 시작해 계속 줄어든다", "0.004 에서 시작해 전체의 30% 지점에서 0.1, 끝에는 거의 0 (약 4e-7)", "0.1 로 고정", "0 에서 시작해 끝에 0.1"],
    answer: 1, why: "시작 = max_lr / 25 = 0.004, 최대 지점 ≈ 5,865 × 0.3 ≈ 1,759 step. 배치마다 scheduler.step() 을 부릅니다.", anim: "D2-7" },
  { title: "혼동 행렬을 np.bincount(true * 10 + pred, minlength=100).reshape(10, 10) 로 만들 때, 칸 번호 k = 35 의 의미는?",
    options: ["정답 3 (cat) 을 5 (dog) 로 예측한 개수", "정답 5 를 3 으로 예측한 개수", "35번째 이미지", "정답 35"],
    answer: 0, why: "k = 정답 × 10 + 예측 → 정답 = k // 10 = 3, 예측 = k % 10 = 5. VGG-mini 실행 예에서 가장 많이 헷갈린 쌍도 cat → dog (180장).", anim: "D2-8" },
  { title: "8교시 미니 프로젝트 ResNet-20 학습 설정을 20 epoch 으로 정한 이유는? (강사 사전 실행)",
    options: ["15 epoch 은 0.8479 로 목표 0.85 에 조금 못 미쳤고, 20 epoch 에서 0.8587", "GPU 가 없어서", "15 epoch 에서 과적합이 생겨서", "OneCycleLR 이 20 epoch 만 지원해서"],
    answer: 0, why: "같은 설정(SGD nesterov · wd 5e-4 · OneCycle 0.1 · 증강) 에서 epoch 만 늘렸습니다. 이 가중치를 오늘 1교시에 불러와 씁니다.", anim: "D2-8" }
];
