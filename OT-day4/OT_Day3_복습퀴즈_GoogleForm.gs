/**
 * ONNX·TensorRT Day 3 복습 퀴즈 — Google Form 자동 생성 스크립트
 * HYUNDAI AI Insight Campus 온디바이스 AI · ONNX·TensorRT Day 4 (11/9) 09:00~09:15
 *
 * 사용 방법
 * 1. https://script.google.com 접속 → 새 프로젝트
 * 2. 이 파일 전체를 붙여 넣고 저장
 * 3. 상단 함수 선택에서 createOTDay3Quiz 선택 → ▶ 실행 (최초 1회 권한 승인)
 * 4. 하단 실행 로그에 출력되는 두 URL 확인
 *    - 편집용(editUrl): 강사용, 응답 확인·설정 변경
 *    - 응답용(publishedUrl): 학생에게 공유
 *
 * 생성되는 폼 설정
 * - 퀴즈 모드(자동 채점, 문항당 1점, 총 20점)
 * - 이름 필수 입력, 문항 전부 필수
 * - 제출 직후 점수 공개, 정답·해설·복습할 애니메이션 링크 표시
 * - 문항 순서 고정(Day 3 교시 순), 보기 순서도 고정
 * - 회사 Workspace 계정에서 401 invalid_client 오류가 나면 개인 Google 계정으로 실행
 */

function createOTDay3Quiz() {
  const form = FormApp.create('ONNX·TensorRT Day 3 복습 퀴즈');
  form.setIsQuiz(true)
      .setDescription(
        '범위: ONNX·TensorRT Day 3 전체 (providers · warm-up · p95 · bench() · 스레드 수 · 그래프 최적화 레벨 · CUDAExecutionProvider · 지연 vs 처리량 · profiling · 카메라 4대 미니 프로젝트)\n' +
        '문항당 1점, 총 20점 · 권장 시간 15분\n' +
        '제출 후 점수와 해설이 바로 표시됩니다. 틀린 문항은 해설의 애니메이션 링크로 복습하세요.\n' +
        '애니메이션: https://imguru-mooc.github.io/AI_CAMPUS/OT-day3/animation.html')
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
var ANIM_URL = 'https://imguru-mooc.github.io/AI_CAMPUS/OT-day3/animation.html';

var QUESTIONS = [
  { title: "InferenceSession(path, providers=[\"CUDAExecutionProvider\", \"CPUExecutionProvider\"]) 의 providers 목록에 대해 옳은 것은?",
    options: ["적힌 실행기를 모두 동시에 나눠서 사용한다", "앞에서부터 시도하고, 안 되면 조용히 다음 것으로 넘어간다", "CUDA 를 쓸 수 없으면 세션 생성 단계에서 오류로 멈춘다", "순서와 상관없이 ORT 가 가장 빠른 것을 고른다"],
    answer: 1,
    why: "providers 는 희망 목록입니다. 앞에서부터 시도하고 안 되면 다음 것으로 조용히 넘어가므로 'GPU 로 돌린 줄 알았는데 CPU 였다' 는 일이 흔합니다.",
    anim: "D3-1" },
  { title: "세션이 실제로 GPU 에서 돌아가는지 확인하는 방법은?",
    options: ["ort.get_available_providers() 에 CUDA 가 있는지 본다", "providers 에 CUDA 를 넣었으니 따로 확인할 필요 없다", "ort.__version__ 문자열에 gpu 가 들어 있는지 본다", "세션을 만든 뒤 sess.get_providers()[0] 을 본다"],
    answer: 3,
    why: "get_available_providers() 는 '설치된' 실행기 목록일 뿐입니다. 실제로 쓰이는 실행기는 세션을 만든 뒤 sess.get_providers() 로 확인합니다 (1-2 의 GPU_OK).",
    anim: "D3-1" },
  { title: "새로 만든 세션으로 5번 연속 재면 첫 실행이 가장 느린 경우가 많습니다. 그래서 하는 일은?",
    options: ["warm-up 몇 번은 버리고 반복해서 잰다", "첫 실행 시간을 그대로 대표값으로 기록해 둔다", "세션을 매번 새로 만들어 첫 실행만 골라 잰다", "time.time() 으로 바꿔 딱 한 번만 잰다"],
    answer: 0,
    why: "첫 실행에는 메모리 확보 · 캐시 채우기 · 커널 선택 같은 준비가 섞입니다. warm-up 10번 버리기 → 100번 재기 → 평균 · p50 · p95 · FPS.",
    anim: "D3-2" },
  { title: "t0 = time.perf_counter(); sess.run(...); dt = time.perf_counter() - t0 — dt 를 ms 로 바꾸는 식은?",
    options: ["dt / 1000", "dt * 1e6", "dt * 1000", "dt (이미 ms)"],
    answer: 2,
    why: "perf_counter() 는 초 단위 시계입니다. 차이 × 1000 = ms. time.time() 보다 짧은 시간을 더 정확히 잽니다.",
    anim: "D3-2" },
  { title: "실시간 카메라 시스템에서 추론 시간을 판단할 때 가장 중요하게 보는 숫자는?",
    options: ["최대값 — 가장 나빴던 한 번의 시간", "평균 — 전체를 대표하는 값", "첫 실행 시간 — 시작 지연", "p95 — 튀는 지연까지 반영"],
    answer: 3,
    why: "평균 30 FPS 라도 가끔 튀면 그 순간 프레임을 놓칩니다. 최대값은 한 번의 사고 (다른 프로그램 · 메모리 정리) 에 크게 흔들려 비교용으로 쓰지 않습니다. p95 = np.percentile(times, 95).",
    anim: "D3-2" },
  { title: "bench(sess, x) 에 배치 4 입력 (shape (4, 3, 224, 224)) 을 넣었더니 mean 이 50 ms 였습니다. bench 가 돌려주는 fps 는?",
    options: ["80 (장/초)", "20 (장/초)", "200 (장/초)", "12.5 (장/초)"],
    answer: 0,
    why: "fps = 1000 / mean × x.shape[0] = 1000 / 50 × 4 = 80. 배치면 한 번에 b 장이므로 × b → '초당 장 수' (6교시의 처리량).",
    anim: "D3-2" },
  { title: "스레드 수를 바꾸는 올바른 순서는?",
    options: ["세션을 만든 뒤 sess.intra_op_num_threads = 2 로 바꾼다", "SessionOptions 에 intra_op_num_threads = 2 → 세션 생성", "sess.run(None, feed, threads=2) 처럼 실행 때 넘긴다", "os.environ 에 스레드 수를 적은 뒤 import onnxruntime"],
    answer: 1,
    why: "옵션 → 세션 순서. 이미 만든 세션에는 옵션 변경이 적용되지 않습니다. intra_op_num_threads = 연산 하나를 몇 명이 나눠 계산할지.",
    anim: "D3-3" },
  { title: "CPU 코어가 2개인 Colab 에서 스레드를 4로 늘리면 일반적으로 어떤 일이 생길까요?",
    options: ["코어보다 일꾼이 많아 p95 부터 나빠진다", "일꾼이 2배라 평균 시간이 절반으로 준다", "코어 수를 넘는 값이라 세션 생성이 오류로 멈춘다", "ORT 가 자동으로 2로 줄여 아무 차이가 없다"],
    answer: 0,
    why: "스레드 = 일꾼, 코어 = 작업대. 코어 수가 상한이고, 넘으면 평균보다 p95 가 먼저 나빠집니다. 작은 연산은 나누는 비용이 더 클 수도 있습니다.",
    anim: "D3-3" },
  { title: "intra_op_num_threads 의 기본값 0 의 뜻은?",
    options: ["스레드를 쓰지 않는다 (실행 불가)", "코어 수와 상관없이 항상 1 스레드", "CPU 대신 GPU 스레드만 사용한다", "ORT 가 코어 수에 맞춰 정한다"],
    answer: 3,
    why: "0 = 자동. 그래서 대부분은 기본값이 좋고, 다른 프로그램과 CPU 를 나눠 써야 할 때 줄입니다. make_session(threads=0) 도 같은 뜻.",
    anim: "D3-3" },
  { title: "graph_optimization_level 4단계와 하는 일의 짝으로 옳은 것은?",
    options: ["BASIC = 노드 합치기, EXTENDED = 상수 접기", "DISABLE = 레이아웃 최적화, ALL = 아무것도 안 함", "BASIC = 상수 접기, EXTENDED = 노드 합치기, ALL = 레이아웃", "BASIC = 레이아웃, EXTENDED = 상수 접기, ALL = 노드 합치기"],
    answer: 2,
    why: "DISABLE → BASIC (상수 접기) → EXTENDED (노드 합치기) → ALL (레이아웃, 기본값). 합치기 = 메모리 왕복 감소, 레이아웃 = CPU SIMD 명령에 맞는 데이터 배치.",
    anim: "D3-4" },
  { title: "SimpleConvNet 을 EXTENDED 로 열어 저장한 그래프를 describe_onnx 로 보면 FusedConv · FusedGemm 의 domain 은 com.microsoft 입니다. 이 뜻은?",
    options: ["ORT 전용 연산이라 다른 실행기는 읽지 못한다", "ONNX 표준 opset 17 에 들어 있는 연산이다", "Windows 용 ORT 에서만 실행되는 연산이다", "Netron 화면에서만 보이는 표시용 노드다"],
    answer: 0,
    why: "com.microsoft = ORT 전용 도메인. Day 2 5-3 의 오류 메시지에 'FusedGemm' 이 나온 이유도 ORT 가 실행 전에 합쳐 놓았기 때문입니다.",
    anim: "D3-4" },
  { title: "optimized_model_filepath 로 저장한 ALL 레벨 그래프에 대해 옳은 것은?",
    options: ["원래 ONNX 보다 정확도가 높아 이 파일을 배포한다", "장치 전용 레이아웃이라 원래 ONNX 를 배포한다", "어느 컴퓨터에서나 가장 빠르므로 이 파일을 배포한다", "레벨을 낮춘 그래프와 결과가 달라 다시 학습해야 한다"],
    answer: 1,
    why: "ALL 의 레이아웃 최적화는 그 장치 전용입니다 (ORT 도 경고). 레벨이 달라도 결과는 같고 (allclose) 속도만 다릅니다.",
    anim: "D3-4" },
  { title: "Colab 설치 셀에서 %pip uninstall -y onnxruntime 을 먼저 한 뒤 onnxruntime-gpu 를 설치하는 이유는?",
    options: ["onnxruntime-gpu 의 설치 용량이 더 작아서", "CPU 판에는 InferenceSession 이 없어서", "두 판을 같이 깔면 import 가 꼬여 GPU 가 안 보여서", "두 판의 라이선스가 서로 달라 함께 쓸 수 없어서"],
    answer: 2,
    why: "두 판은 같은 import 이름 (onnxruntime) 을 씁니다. 둘 다 깔면 충돌하므로 CPU 판을 지우고 GPU 판만 설치합니다.",
    anim: "D3-5" },
  { title: "GPU 세션의 첫 실행이 수백 ms ~ 수 초 걸리는 이유와 대응으로 옳은 것은?",
    options: ["GPU 가 고장 났다 — CPU 세션으로 바꿔 잰다", "CUDA 초기화 · cuDNN 탐색 — warm-up 후 잰다", "모델이 너무 크다 — 실행할 때마다 이 시간이 걸린다", "GPU 의 실제 성능이다 — 첫 실행 시간을 그대로 기록한다"],
    answer: 1,
    why: "GPU 는 처음에 준비할 것이 훨씬 많아 warm-up 이 CPU 보다 더 중요합니다. warm-up 없이 첫 실행을 기록하면 수백 배 느리게 보입니다.",
    anim: "D3-5" },
  { title: "MobileNetV3 를 CPU 와 GPU (T4) 에서 배치 1 · 배치 32 로 비교했을 때 일반적인 경향은?",
    options: ["배치와 상관없이 GPU 의 이득은 거의 일정하게 유지된다", "배치 1 에서 GPU 가 가장 크게 이기고, 배치 32 에서는 CPU 가 이긴다", "GPU 는 메모리가 모자라 배치 32 를 처리할 수 없다", "배치 1 은 GPU 이득이 작고, 배치 32 에서 처리량 차이가 크게 벌어진다"],
    answer: 3,
    why: "GPU = 계산은 빠르지만 CPU ↔ GPU 복사 · 준비 비용이 있습니다. 일이 많을수록 (배치가 클수록) 수천 개 코어가 일할 거리가 생겨 유리합니다. 결과 비교는 atol=1e-3.",
    anim: "D3-5" },
  { title: "배치 16 의 지연 (한 번 run) 이 200 ms 입니다. 처리량과 장당 시간은?",
    options: ["처리량 5 장/초, 장당 200 ms", "처리량 3,200 장/초, 장당 12.5 ms", "처리량 80 장/초, 장당 12.5 ms", "처리량 80 장/초, 장당 200 ms"],
    answer: 2,
    why: "처리량 = 배치 × 1000 ÷ 지연 = 16 × 1000 ÷ 200 = 80 장/초. 장당 시간 = 지연 ÷ 배치 = 12.5 ms (ms_per_img × imgs_per_s = 1000). 배치 시간을 그대로 '한 장 시간' 으로 쓰면 안 됩니다.",
    anim: "D3-6" },
  { title: "어떤 설정의 처리량이 130 장/초입니다. 카메라 한 대가 30 FPS 라면 감당할 수 있는 카메라 수 (cams = imgs_per_s // 30) 는?",
    options: ["3대", "4대", "5대", "4.33대"],
    answer: 1,
    why: "130 // 30 = 4 (// 는 정수 나눗셈 · 몫). 단, 여러 카메라 프레임을 모아 큰 배치로 처리하면 처리량은 늘어도 모으는 시간 + 배치 지연만큼 답이 늦어집니다. 실시간 → 배치 1 · p95.",
    anim: "D3-6" },
  { title: "ORT profiling 의 순서로 옳은 것은?",
    options: ["run 만 반복하면 JSON 파일이 자동으로 완성된다", "sess.start_profiling() → run → 파일 자동 저장", "run 후 time.perf_counter() 로 노드별 시간을 잰다", "enable_profiling 켜기 → run → end_profiling()"],
    answer: 3,
    why: "end_profiling() 을 부르지 않으면 파일이 닫히지 않아 내용이 없습니다. JSON 은 chrome://tracing · ui.perfetto.dev 에서 시간 막대로도 볼 수 있습니다.",
    anim: "D3-7" },
  { title: "profiling JSON 에서 연산 종류별 시간 비율을 구하는 방법으로 옳은 것은?",
    options: ["_kernel_time 기록만 op_name 별로 dur 을 더한다", "모든 기록의 dur 을 ms 단위로 보고 전부 그대로 더한다", "profiling 을 켠 그 세션으로 bench 까지 함께 잰다", "노드 이름 (name) 별로 첫 번째 기록 하나만 골라 본다"],
    answer: 0,
    why: "dur 단위는 µs (1000 µs = 1 ms). profiling 은 기록하느라 느려지므로 재는 세션과 기록하는 세션은 따로. 실행 예: MobileNetV3 1위 = Conv, SimpleConvNet 1위 = FusedGemm — 병목은 모델마다 다릅니다.",
    anim: "D3-7" },
  { title: "미니 프로젝트 과제 2 — 스레드 {1, 2} × 레벨 {DISABLE, ALL} 실험표 grid 에서 카메라용 최선 설정 best 를 고르는 코드는?",
    options: ["grid.loc[grid[\"mean\"].idxmax()]", "grid.loc[grid[\"fps\"].idxmin()]", "grid.loc[grid[\"p95\"].idxmin()]", "grid.iloc[-1] (마지막 줄)"],
    answer: 2,
    why: "실시간이면 p95 가 가장 작은 줄. 채점은 숫자 값이 아니라 '최선이 정말 표에서 가장 작은 p95 인가' 를 봅니다. 과제 3 은 120 장/초 (4대 × 30) 를 감당하는 배치 중 지연이 가장 짧은 배치.",
    anim: "D3-8" }
];
