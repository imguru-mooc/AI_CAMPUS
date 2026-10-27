/**
 * Neural Network Day 5 복습 퀴즈 — Google Form 자동 생성 스크립트
 * HYUNDAI AI Insight Campus 온디바이스 AI · Vision 기본 모델 Day 1 (10/27) 09:00~09:15
 *
 * 사용 방법
 * 1. https://script.google.com 접속 → 새 프로젝트
 * 2. 이 파일 전체를 붙여 넣고 저장
 * 3. 상단 함수 선택에서 createNNDay5Quiz 선택 → ▶ 실행 (최초 1회 권한 승인)
 * 4. 하단 실행 로그에 출력되는 두 URL 확인
 *    - 편집용(editUrl): 강사용, 응답 확인·설정 변경
 *    - 응답용(publishedUrl): 학생에게 공유
 *
 * 생성되는 폼 설정
 * - 퀴즈 모드(자동 채점, 문항당 1점, 총 20점)
 * - 이름 필수 입력, 문항 전부 필수
 * - 제출 직후 점수 공개, 정답·해설·복습할 애니메이션 링크 표시
 * - 문항 순서 고정(Day 5 교시 순), 보기 순서도 고정
 */

function createNNDay5Quiz() {
  const form = FormApp.create('Neural Network Day 5 복습 퀴즈');
  form.setIsQuiz(true)
      .setDescription(
        '범위: Neural Network Day 5 전체 (완전연결층의 한계 · 합성곱 · 패딩 · 스트라이드 · 다채널 · im2col · Convolution · Pooling · SimpleConvNet · 필터 · 모델 비용)\n' +
        '문항당 1점, 총 20점 · 권장 시간 15분\n' +
        '제출 후 점수와 해설이 바로 표시됩니다. 틀린 문항은 해설의 애니메이션 링크로 복습하세요.\n' +
        '애니메이션: https://imguru-mooc.github.io/AI_CAMPUS/NN-day5/animation.html')
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
var ANIM_URL = 'https://imguru-mooc.github.io/AI_CAMPUS/NN-day5/animation.html';

var QUESTIONS = [
  { title: "모든 MNIST 이미지의 784 픽셀 순서를 같은 방식으로 뒤섞은 뒤 MLP(784-100-10)를 학습하면 정확도는?",
    options: ["0.10 근처로 떨어진다 (찍기 수준)", "원본과 거의 같다 (0.9436 vs 0.9455)", "절반 정도로 떨어진다", "뒤섞으면 학습이 발산한다"],
    answer: 1,
    why: "완전연결층은 픽셀의 '이웃 관계'를 쓰지 않으므로 순서를 바꿔도 결과가 같습니다. 즉 이미지의 공간 정보를 버리고 있다는 증거입니다.",
    anim: "D5-1" },
  { title: "224×224×3 이미지를 펼쳐 은닉 1000 의 완전연결층에 넣을 때 가중치 W 의 개수는?",
    options: ["1,792개", "78,400개", "150,528,000개", "150,528개"],
    answer: 2,
    why: "224·224·3 = 150,528 입력 × 1000 = 150,528,000. 같은 RGB 입력에 3×3 필터 64개 합성곱은 64·3·3·3 + 64 = 1,792 개로, 이미지 크기와 무관합니다.",
    anim: "D5-1" },
  { title: "합성곱이 파라미터를 크게 줄이는 두 가지 아이디어는?",
    options: ["지역성 · 가중치 공유", "정규화 · 드롭아웃", "패딩 · 스트라이드", "풀링 · 배치 정규화"],
    answer: 0,
    why: "필터는 작은 영역만 보고(지역성), 같은 필터를 모든 위치에 다시 씁니다(가중치 공유). 그래서 파라미터 수가 입력 크기와 상관없습니다.",
    anim: "D5-1" },
  { title: "입력 28, 필터 5, 스트라이드 1, 패딩 0 일 때 출력 크기는? (OH = (H + 2P − FH) / S + 1)",
    options: ["28", "26", "23", "24"],
    answer: 3,
    why: "(28 + 0 − 5) / 1 + 1 = 24. SimpleConvNet 첫 층 출력이 (N, 30, 24, 24) 인 이유입니다.",
    anim: "D5-2" },
  { title: "입력 28, 필터 3, 스트라이드 2, 패딩 1 일 때 출력 크기는?",
    options: ["14", "13", "28", "15"],
    answer: 0,
    why: "(28 + 2 − 3) // 2 + 1 = 13 + 1 = 14. 스트라이드 2 는 크기를 절반으로 줄입니다. (28, 3, 1, 1) 이면 28 그대로 (same).",
    anim: "D5-2" },
  { title: "x = arange(16).reshape(4, 4), 커널 [[1, 0], [0, −1]] 로 합성곱(스트라이드 1)한 3×3 출력의 값은?",
    options: ["모두 5", "모두 −5", "모두 0", "왼쪽 위부터 0, 1, 2, …"],
    answer: 1,
    why: "각 위치에서 x[i, j] − x[i+1, j+1] = −(4 + 1) = −5. 대각선 방향 밝기 차이를 보는 필터라 일정한 경사에서는 값이 모두 같습니다.",
    anim: "D5-2" },
  { title: "입력 채널이 3 이고 필터 하나의 크기가 3×3 일 때, 필터 하나의 출력과 필터 모양은?",
    options: ["출력 채널 3장, 필터 (3, 3)", "출력 채널 1장, 필터 (3, 3)", "출력 채널 1장, 필터 (3, 3, 3) — 채널별 곱을 모두 더함", "출력 채널 9장, 필터 (3, 3, 3)"],
    answer: 2,
    why: "필터는 입력 채널 수만큼 깊이를 가지고, 채널별 곱을 모두 더해 한 장을 만듭니다. 필터 FN 개 → 출력 채널 FN 장.",
    anim: "D5-3" },
  { title: "im2col 이 (N, C, H, W) 입력을 바꾸는 모양은?",
    options: ["(N·OH·OW, C·FH·FW) — 필터가 닿는 영역 하나가 행 하나", "(C·FH·FW, N) — 이미지 한 장이 열 하나", "(N, C·H·W) — 이미지를 한 줄로 펼침", "(FN, C·FH·FW) — 필터를 한 줄로"],
    answer: 0,
    why: "영역마다 한 행으로 펼치면 합성곱 = col @ W_col 행렬곱이 됩니다. 필터 쪽을 (FN, C·FH·FW) 로 펼치는 것은 W 의 reshape 입니다.",
    anim: "D5-4" },
  { title: "MNIST 배치 100, 5×5 필터일 때 col 은 (57,600, 25) 입니다. 입력과 비교한 메모리, 4중 for 루프와 비교한 속도는?",
    options: ["메모리 같음, 약 2배 빠름", "메모리 약 18.4배, 약 177배 빠름", "메모리 약 25배, 약 10배 빠름", "메모리 절반, 약 177배 빠름"],
    answer: 1,
    why: "겹치는 영역을 중복 저장하므로 입력의 18.4배 메모리를 쓰는 대신, 최적화된 행렬곱으로 for 루프 6.9 초 → 0.039 초 (서버 기준)가 됩니다.",
    anim: "D5-4" },
  { title: "Convolution.backward 에서 dcol 을 입력 모양 dx 로 되돌리는 함수와 그 동작은?",
    options: ["im2col — 겹치는 자리는 평균", "reshape — 그대로 모양만", "col2im — 겹치는 자리는 더한다", "transpose — 축만 바꾼다"],
    answer: 2,
    why: "한 입력 픽셀이 여러 영역에 쓰였으므로 각 영역에서 온 기울기를 모두 더해야 합니다 (연쇄 법칙의 분기 = 합).",
    anim: "D5-5" },
  { title: "Max Pooling(2×2) 의 역전파는?",
    options: ["4칸에 기울기를 1/4 씩 나눈다", "기울기를 그대로 4칸 모두에 복사한다", "파라미터 W 의 기울기를 계산한다", "순전파 때 최댓값이던 자리(argmax)로만 dout 을 보내고 나머지는 0"],
    answer: 3,
    why: "ReLU 의 mask 와 같은 원리입니다. 풀링은 파라미터가 없고, 1/4 씩 나누는 것은 Average Pooling 입니다.",
    anim: "D5-5" },
  { title: "SimpleConvNet 의 shape 흐름으로 옳은 것은?",
    options: ["(N,1,28,28) → (N,30,24,24) → (N,30,12,12) → 4320 → 100 → 10", "(N,1,28,28) → (N,30,28,28) → (N,30,14,14) → 5880 → 100 → 10", "(N,1,28,28) → (N,30,24,24) → 17280 → 100 → 10", "(N,784) → (N,30,24,24) → (N,30,12,12) → 100 → 10"],
    answer: 0,
    why: "Conv(1→30, 5×5) 로 24×24, Pool 2×2 로 12×12, 펼치면 30·12·12 = 4320. 내일 PyTorch 의 nn.Flatten 이 이 펼치기를 합니다.",
    anim: "D5-6" },
  { title: "SimpleConvNet 의 파라미터 433,890 개 중 가장 많은 부분을 차지하는 층은?",
    options: ["Conv(1→30, 5×5) — 780개", "Affine(4320→100) — 432,100개", "Affine(100→10) — 1,010개", "Pooling — 4,320개"],
    answer: 1,
    why: "780 + 432,100 + 1,010 = 433,890. 파라미터는 Affine 에, 연산(MAC)은 Conv 와 Affine 에 432,000 씩 반반입니다.",
    anim: "D5-8" },
  { title: "SimpleConvNet 을 Adam(0.001), 배치 100 으로 1 epoch 학습한 결과는?",
    options: ["test 0.9694 — Day 3 MLP 와 같다", "test 0.9310 — 1 epoch 로는 부족하다", "test 0.9810 — Day 4 MLP 5 epoch(0.9757) 보다 높다", "test 0.9995 — 과적합이 없다"],
    answer: 2,
    why: "train 0.9825, test 0.9810, 오답 190 / 10,000. 1 epoch 만으로 5 epoch MLP 를 넘었습니다. 오늘(Vision Day 1) PyTorch 로 같은 모델을 다시 학습합니다.",
    anim: "D5-6" },
  { title: "학습된 1층 필터 30개를 그려 보면?",
    options: ["학습 전과 똑같이 무작위 잡음이다", "모두 같은 모양으로 수렴한다", "숫자 0~9 모양이 그대로 보인다", "에지 · 획 방향을 검출하는 모양으로 바뀐다"],
    answer: 3,
    why: "학습 전 무작위 필터가 학습 후 특정 방향의 밝기 변화(에지 · 획)에 반응하는 모양이 됩니다. 2교시 Sobel 필터와 비슷한 것이 저절로 생깁니다.",
    anim: "D5-7" },
  { title: "test 이미지를 오른쪽으로 2 픽셀 밀었을 때 정확도 (CNN vs Day 4 MLP) 는?",
    options: ["CNN 0.9119 vs MLP 0.7975 — CNN 이 덜 떨어진다", "CNN 0.7975 vs MLP 0.9119 — MLP 가 덜 떨어진다", "둘 다 그대로 0.98 근처", "둘 다 0.10 으로 떨어진다"],
    answer: 0,
    why: "1 · 2 · 3 픽셀: CNN 0.9734 / 0.9119 / 0.7484, MLP 0.9551 / 0.7975 / 0.5619. 가중치 공유와 풀링 덕분에 CNN 이 위치 이동에 더 강합니다.",
    anim: "D5-7" },
  { title: "합성곱 층의 MAC(곱셈-덧셈) 수 공식은?",
    options: ["FN · C · FH · FW", "FN · OH · OW · C · FH · FW", "OH · OW · FN", "C · H · W · FN"],
    answer: 1,
    why: "출력 원소 하나(FN·OH·OW 개)마다 C·FH·FW 번 곱하고 더합니다. ①은 파라미터 수(편향 제외)입니다.",
    anim: "D5-8" },
  { title: "SimpleConvNet 의 float32 가중치 크기와 int8 로 바꿨을 때 크기는?",
    options: ["0.43 MB → 0.11 MB", "433 KB → 433 KB (변하지 않음)", "17.4 MB → 4.3 MB", "약 1.74 MB → 약 0.43 MB"],
    answer: 3,
    why: "433,890 × 4 bytes = 1,735,560 B ≈ 1.74 MB, int8 은 1 byte 라 1/4. Quantization·Pruning 과목에서 실제로 해 봅니다.",
    anim: "D5-8" },
  { title: "경량 CNN Conv(1→16, 3, P1) → Pool → Conv(16→32, 3, P1) → Pool → Affine(1568→10) 에 대한 설명으로 옳은 것은?",
    options: ["파라미터 20,490 개로 21배 적지만 MAC 은 약 103만으로 비슷하다", "파라미터와 MAC 이 모두 21배 적다", "파라미터는 같고 MAC 만 21배 적다", "파라미터 433,890 개로 같다"],
    answer: 0,
    why: "160 + 4,640 + 15,690 = 20,490. 큰 Affine 을 없애 파라미터가 줄었지만, 합성곱 층이 늘어 연산은 1,031,744 MAC 으로 비슷합니다. 경량화 = 파라미터와 연산을 따로 봐야 합니다.",
    anim: "D5-8" },
  { title: "224×224×3 입력에 3×3 필터 64개 합성곱(패딩 1) 한 층의 파라미터와 MAC 은?",
    options: ["1,792개 · 1,792 MAC", "86,704,128개 · 1,792 MAC", "1,792개 · 86,704,128 MAC", "150,528개 · 3,211,264 MAC"],
    answer: 2,
    why: "파라미터는 64·3·3·3 + 64 = 1,792 로 작지만, 출력 224·224·64 = 3,211,264 원소마다 27번 곱해 86,704,128 MAC. 큰 이미지에서는 연산이 병목 → Day 3 경량 CNN.",
    anim: "D5-8" }
];
