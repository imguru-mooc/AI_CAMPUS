/**
 * ONNX·TensorRT Day 4 복습 퀴즈 — Google Form 자동 생성 스크립트
 * HYUNDAI AI Insight Campus 온디바이스 AI · ONNX·TensorRT Day 5 (11/10) 09:00~09:15
 *
 * 사용 방법
 * 1. https://script.google.com 접속 → 새 프로젝트
 * 2. 이 파일 전체를 붙여 넣고 저장
 * 3. 상단 함수 선택에서 createOTDay4Quiz 선택 → ▶ 실행 (최초 1회 권한 승인)
 * 4. 하단 실행 로그에 출력되는 두 URL 확인
 *    - 편집용(editUrl): 강사용, 응답 확인·설정 변경
 *    - 응답용(publishedUrl): 학생에게 공유
 *
 * 생성되는 폼 설정
 * - 퀴즈 모드(자동 채점, 문항당 1점, 총 20점)
 * - 이름 필수 입력, 문항 전부 필수
 * - 제출 직후 점수 공개, 정답·해설·복습할 애니메이션 링크 표시
 * - 문항 순서 고정(Day 4 교시 순), 보기 순서도 고정
 * - 회사 Workspace 계정에서 401 invalid_client 오류가 나면 개인 Google 계정으로 실행
 */

function createOTDay4Quiz() {
  const form = FormApp.create('ONNX·TensorRT Day 4 복습 퀴즈');
  form.setIsQuiz(true)
      .setDescription(
        '범위: ONNX·TensorRT Day 4 전체 (TensorRT 엔진 · 배치 1 고정 ONNX · polygraphy convert · polygraphy run 비교 · Python 빌드 build_engine() · Python 실행 trt_runner() · FP16 · 엔진 레이어 합치기 · 실행기 4가지 성능표 미니 프로젝트)\n' +
        '문항당 1점, 총 20점 · 권장 시간 15분\n' +
        '제출 후 점수와 해설이 바로 표시됩니다. 틀린 문항은 해설의 애니메이션 링크로 복습하세요.\n' +
        '애니메이션: https://imguru-mooc.github.io/AI_CAMPUS/OT-day4/animation.html')
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
var ANIM_URL = 'https://imguru-mooc.github.io/AI_CAMPUS/OT-day4/animation.html';

var QUESTIONS = [
  { title: "ONNX · ONNX Runtime · TensorRT 엔진에 대한 설명으로 옳은 것은?",
    options: ["셋 다 같은 파일 형식이고 확장자만 다르다", "ONNX = 설계도, ONNX Runtime = 읽어서 바로 돌리는 기계, TensorRT 엔진 = 이 GPU 전용으로 다시 조립한 완성품 — 엔진은 GPU 세대 (compute capability) · TensorRT 버전이 바뀌면 다시 빌드하므로 배포할 것은 .onnx", "TensorRT 엔진은 한 번 만들면 CPU · 어떤 GPU 에서도 그대로 돌아가므로 배포할 것은 .engine", "ONNX Runtime 도 빌드 결과를 .engine 파일로 저장해 두고 다음에 재사용한다"],
    answer: 1,
    why: "ORT 는 세션 만들 때마다 정해진 규칙으로 최적화하고, TensorRT 는 빌드할 때 한 번 이 GPU 에서 커널을 직접 재 보고 골라 .engine 으로 저장합니다. T4 = 7.5 (sm_75), Jetson Orin = 8.7 — 다른 장치에는 .onnx 를 가져가 다시 빌드.",
    anim: "D4-1" },
  { title: "오늘 mnv3s_b1.onnx 를 dynamic_axes 없이 (배치 1 고정) 내보낸 이유는?",
    options: ["TensorRT 는 배치 1 만 지원하기 때문", "dynamic_axes 를 주면 ONNX 파일이 깨지기 때문", "배치 1 이 언제나 처리량 (FPS) 이 가장 높기 때문", "입력 크기가 정해져 있으면 그 크기에 맞는 커널을 고르고, 배치가 자유로우면 최소 · 최적 · 최대 크기 설정 (optimization profile) 이 따로 필요하기 때문"],
    answer: 3,
    why: "Day 3 의 mnv3s.onnx 는 ['batch', 3, 224, 224], 오늘 것은 [1, 3, 224, 224] — 숫자만 있습니다. 배치가 자유로운 ONNX 를 그대로 빌드하면 optimization profile 이 없다는 오류가 납니다.",
    anim: "D4-1" },
  { title: "mnv3s_b1.onnx 를 FP32 TensorRT 엔진 파일로 바꾸는 명령으로 옳은 것은?",
    options: ["polygraphy convert mnv3s_b1.onnx -o mnv3s_fp32_cli.engine", "polygraphy run mnv3s_b1.onnx --trt", "polygraphy inspect model mnv3s_b1.onnx -o mnv3s_fp32_cli.engine", "polygraphy build mnv3s_b1.onnx > mnv3s_fp32_cli.engine"],
    answer: 0,
    why: "하위 명령은 convert, 출력 파일은 -o. 같은 명령에 --fp16 만 붙이면 FP16 엔진입니다. (trtexec 는 pip 판 TensorRT 에 들어 있지 않아 Colab 에서는 Polygraphy 를 씁니다.)",
    anim: "D4-2" },
  { title: "엔진 빌드 (파싱 → 최적화 → 커널 고르기 → 저장) 가 수십 초씩 걸리는 가장 큰 이유는?",
    options: ["ONNX 파일을 인터넷에서 다시 내려받기 때문", "빌드하면서 가중치를 다시 학습하기 때문", "레이어마다 후보 커널 (tactic) 을 이 GPU 에서 직접 돌려 보고 가장 빠른 것을 고르기 때문", "CPU 용 엔진도 함께 만들기 때문"],
    answer: 2,
    why: "③ 커널 고르기가 오래 걸립니다. 그래서 '빌드는 느리지만 실행은 빠른' 엔진이 됩니다. 빌드 셀은 한 번만 누르고 기다리세요 — 여러 번 누르면 빌드가 겹쳐 더 느려지고 GPU 메모리가 모자랄 수 있습니다.",
    anim: "D4-2" },
  { title: "polygraphy convert 로그의 [W] 줄과 [E] 줄에 대한 설명으로 옳은 것은?",
    options: ["[W] 가 한 줄이라도 있으면 엔진을 쓸 수 없다", "[E] 는 '끝 (End)' 표시라 무시해도 된다", "[W] 는 경고 (계속 진행), [E] 는 오류 (빌드 실패) — 성공 여부는 종료 코드 0 과 [E] 로 판단한다", "[W] 는 FP16 으로 바뀐 레이어를 표시한다"],
    answer: 2,
    why: "흔한 경고 'INT64 가중치를 INT32 로 바꿨다' 는 무시해도 됩니다. 로그에서 볼 곳 세 가지: [E] · [W] · 빌드 시간.",
    anim: "D4-2" },
  { title: "polygraphy run mnv3s_b1.onnx --onnxrt --trt --atol 1e-3 --rtol 1e-3 이 하는 일은?",
    options: ["같은 무작위 입력을 ONNX Runtime 과 TensorRT 에 넣어 출력을 비교하고 PASSED / FAILED 를 알려 준다", "두 실행기의 엔진 파일 크기를 비교한다", "MNIST 데이터로 두 실행기의 정확도를 잰다", "이미 만든 mnv3s_fp32_cli.engine 을 읽어 속도만 잰다"],
    answer: 0,
    why: "--trt 를 주면 polygraphy 가 안에서 엔진을 새로 빌드한 뒤 비교합니다 (그래서 또 수십 초). --onnxrt 를 빠뜨리면 비교 대상이 없어 실행만 합니다.",
    anim: "D4-3" },
  { title: "polygraphy 비교의 판정 식과 FP16 결과 해석으로 옳은 것은?",
    options: ["|a − b| ≤ atol · rtol — FAILED 면 엔진이 틀린 것이므로 버린다", "|a − b| ≤ atol 만 본다 — rtol 은 무시된다", "|a| ≤ atol + |b| — 무작위 입력으로 한 번 PASSED 면 실제 데이터 확인은 필요 없다", "|a − b| ≤ atol + rtol · |b| — FP16 은 유효숫자 약 3자리라 1e-3 기준에서 FAILED 가 흔하며, FAILED 는 '이 기준보다 차이가 크다' 는 뜻"],
    answer: 3,
    why: "기준을 정하는 것은 사람입니다. FP32 는 엄격하게, FP16 은 느슨한 기준 + 정확도 (6교시 MNIST) 로 판단합니다.",
    anim: "D4-3" },
  { title: "Python API 로 엔진을 빌드하는 순서로 옳은 것은?",
    options: ["Builder → Logger → OnnxParser → create_network(0) → Config → 저장", "Logger → Builder → create_network(0) → OnnxParser.parse_from_file → Config → build_serialized_network → 파일 저장", "OnnxParser → Config → Logger → Builder → create_network(0) → 저장", "Logger → Config → build_serialized_network → OnnxParser.parse_from_file → 저장"],
    answer: 1,
    why: "① Logger (TensorRT 가 하는 말을 받는 곳) ② Builder (공장) ③ Network (빈 설계판, 0 = explicit batch) ④ OnnxParser 로 설계판 채우기 ⑤ Config ⑥ 빌드 · 저장 — polygraphy convert 가 안에서 하던 일입니다.",
    anim: "D4-4" },
  { title: "Config 의 set_memory_pool_limit(trt.MemoryPoolType.WORKSPACE, 1 << 30) 와 profiling_verbosity = DETAILED 의 역할로 옳은 것은?",
    options: ["WORKSPACE = 엔진 파일 크기 상한, DETAILED = 빌드 로그를 더 많이 출력", "WORKSPACE = CPU 스레드 수, DETAILED = FP16 켜기", "둘 다 빌드 시간에만 영향을 주고 엔진 내용과는 무관하다", "WORKSPACE = 커널이 계산 중 잠깐 쓰는 GPU 메모리 상한 (너무 작으면 빠른 커널을 못 고름), DETAILED = 나중에 엔진 레이어 정보 (LayerType 등) 를 볼 수 있게 함"],
    answer: 3,
    why: "1 << 30 = 1 GB. DETAILED 로 빌드해 두었기 때문에 7교시에 엔진 인스펙터로 레이어 이름 · 종류를 꺼내 볼 수 있습니다. FP16 은 따로 config.set_flag(trt.BuilderFlag.FP16) 한 줄.",
    anim: "D4-4" },
  { title: "build_engine() 에서 빌드 실패를 다루는 방법으로 옳은 것은?",
    options: ["실패해도 빈 엔진 파일이 저장되므로 파일 크기로만 확인한다", "parse_from_file 결과를 확인한 뒤 빌드하고, build_serialized_network 가 None 을 돌려주면 실패 — 위쪽 로그 [E] 를 본다", "parse_from_file 이 실패해도 빌드는 정상 진행된다", "Builder · Network · Config 를 전역 변수로 두고 계속 재사용하면 실패가 줄어든다"],
    answer: 1,
    why: "파싱 → 확인 → 빌드. 파싱 오류의 대부분은 'TensorRT 가 모르는 연산' 입니다. Builder · Network · Config 를 재사용하면 이전 설계가 섞이므로 빌드마다 새로 만듭니다.",
    anim: "D4-4" },
  { title: "엔진 파일을 읽어 실행 준비를 하는 순서로 옳은 것은?",
    options: ["engine.create_execution_context() → trt.Runtime(LOGGER) → deserialize_cuda_engine", "trt.Builder(LOGGER) → build_serialized_network → get_tensor_name", "trt.Runtime(LOGGER).deserialize_cuda_engine(엔진 바이트) → engine.create_execution_context() → engine.get_tensor_name(i) 로 입출력 이름 · shape 확인", "ort.InferenceSession(엔진 파일) → sess.run()"],
    answer: 2,
    why: "Runtime 은 빌드 때의 Builder 에 해당하는 '읽는 도구', Context 는 실행 한 번에 필요한 상태 (입출력 주소 등). 이름은 export 때 붙인 input · logits 그대로 — TensorRT 도 이름으로 입출력을 찾습니다.",
    anim: "D4-5" },
  { title: "context.set_tensor_address(name, 주소) 에 넘겨야 하는 것은?",
    options: ["PyTorch CUDA 텐서 (device=\"cuda\") 의 data_ptr() — GPU 메모리 주소", "NumPy 배열 그대로", "NumPy 배열의 .tobytes() 결과", "엔진 파일 경로 문자열"],
    answer: 0,
    why: "ONNX Runtime 은 NumPy 를 그대로 받았지만 TensorRT 는 GPU 메모리 주소만 받습니다. 입력은 torch.from_numpy(x1).to(\"cuda\"), 출력 자리는 torch.empty(..., device=\"cuda\"). 입력은 float32 로 (np.random.rand 기본은 float64).",
    anim: "D4-5" },
  { title: "execute_async_v3 와 trt_runner() 에 대한 설명으로 옳은 것은?",
    options: ["execute_async_v3 는 '시작해 둬' 만 하고 바로 돌아오므로 torch.cuda.synchronize() 로 기다린 뒤 결과를 읽는다 — 버퍼 · 주소는 한 번만 준비하고 run(x) 는 복사 → 실행 → 기다리기 → 가져오기만", "execute_async_v3 는 실행이 끝날 때까지 기다린 뒤 돌아온다", "run(x) 할 때마다 torch.empty(..., device=\"cuda\") 로 버퍼를 새로 만들고 주소를 다시 알려 줘야 정확하다", "synchronize 없이 시간을 재야 실제 속도가 나온다"],
    answer: 0,
    why: "기다리지 않고 시간을 재면 실행이 끝나기 전 시간이라 엄청 빠르게 보입니다. 매번 버퍼를 새로 만들면 느립니다 — 버퍼는 한 번.",
    anim: "D4-5" },
  { title: "FP16 에 대한 설명으로 옳은 것은?",
    options: ["4바이트, 최대 약 3.4e38, 유효숫자 약 7자리", "1바이트, 최대 255, 정수만 표현", "2바이트, 최대 65504, eps ≈ 0.001 (유효숫자 약 3자리)", "2바이트, 최대 65536, 유효숫자 약 5자리"],
    answer: 2,
    why: "FP16 은 메모리가 FP32 의 절반이고 T4 의 Tensor Core 가 빨리 계산하지만, 범위와 정밀도가 줄어듭니다. 1000 근처에서는 간격이 0.5 (1000.3 → 1000.5).",
    anim: "D4-6" },
  { title: "float(np.float16(0.1)) 과 np.float16(70000.0) 의 결과는?",
    options: ["0.1 · 70000.0", "0.0999755859375 · inf", "0.1 · 65504.0", "0.0999755859375 · 65504.0"],
    answer: 1,
    why: "0.1 도 FP16 에 정확히 저장되지 않고 (0.0999755859375), 최대 65504 를 넘으면 inf 가 됩니다.",
    anim: "D4-6" },
  { title: "config.set_flag(trt.BuilderFlag.FP16) 로 만든 엔진에 대한 설명으로 옳은 것은?",
    options: ["모든 레이어가 반드시 FP16 으로 바뀌고, 입력도 float16 으로 바꿔 넣어야 한다", "항상 FP32 엔진의 정확히 2배 빠르다", "max_diff 가 FP32 보다 크면 무조건 사용할 수 없다", "FP16 은 '허락' — TensorRT 가 레이어마다 재 보고 FP32 로 남기기도 하며, 오늘 엔진의 입출력은 FP32 그대로; 차이가 커져도 정확도 (top-1 · MNIST) 로 판단한다"],
    answer: 3,
    why: "안쪽 계산만 FP16 이고 입출력 dtype 은 FP32 (5-1 표). 복사 · 작은 레이어 · FP32 로 남은 레이어 때문에 속도 이득은 모델마다 다르니 재 보고 말합니다. 빨라도 틀리면 못 쓰므로 성능표에 정확도 열을 함께.",
    anim: "D4-6" },
  { title: "엔진 안의 레이어 목록을 꺼내 보는 방법과 그 해석으로 옳은 것은?",
    options: ["onnx.load(\"mnv3s_fp32.engine\").graph.node 로 읽는다", "engine.create_engine_inspector().get_engine_information(trt.LayerInformationFormat.JSON) — 이름에 \" + \" 가 있으면 여러 ONNX 노드를 합친 레이어", "엔진 파일을 텍스트 편집기로 열어 읽는다", "polygraphy run --trt 의 PASSED 줄 수가 레이어 수다"],
    answer: 1,
    why: "합쳐진 레이어는 'A + B' 처럼 원래 ONNX 노드 이름을 이어 붙인 모양 (보통 Conv + 활성 함수, Conv + Add). CLI 로는 polygraphy inspect model 엔진 --show layers.",
    anim: "D4-7" },
  { title: "엔진을 인스펙트했더니 레이어 이름만 나오고 LayerType 이 없습니다. 원인과, 7교시에서 정리한 '빨라진 이유' 로 옳은 것은?",
    options: ["FP16 으로 빌드했기 때문 / 레이어 수가 적을수록 무조건 빠르다", "WORKSPACE 를 1 GB 로 줬기 때문 / CPU 스레드 수가 늘었다", "배치 1 로 고정했기 때문 / ONNX 노드 수가 늘었다", "profiling_verbosity = DETAILED 없이 빌드했기 때문 / ① 레이어 합치기 (메모리 왕복 ↓) ② 이 GPU 에서 재 보고 고른 커널 ③ FP16 + Tensor Core"],
    answer: 3,
    why: "그래서 build_engine 에 DETAILED 를 넣어 두었습니다. 레이어 수가 적다고 무조건 빠른 것은 아니고 합치기 + 커널 선택 + FP16 이 함께 작용하므로 결국 bench 로 확인합니다.",
    anim: "D4-7" },
  { title: "미니 프로젝트 성능표 cmp 에 precision 열을 두고, Day 3 ot_baseline.csv 와 이어 붙일 때의 처리로 옳은 것은?",
    options: ["같은 TensorRT 라도 FP32 · FP16 을 구분해 속도와 정확도를 함께 보기 위해 — Day 3 줄에는 precision 열이 없으므로 \"FP32\" 로 채운 뒤 cmp 를 이어 붙인다", "precision 은 장식용이라 Day 3 줄은 비워 둬도 채점에 영향이 없다", "Day 3 줄을 모두 지우고 오늘 4줄만 저장한다", "Day 3 줄의 precision 은 \"FP16\" 으로 채운다"],
    answer: 0,
    why: "성능표 = 조건 (provider · precision · batch) + 속도 (mean · p95 · fps) + 정확도 (max_diff · top1_same · mnist_acc). Day 3 와 같은 열 이름이라 pd.concat 으로 이어 붙이고, Day 5 에서 YOLOv8n TensorRT 줄을 더합니다.",
    anim: "D4-8" },
  { title: "과제 4 — ORT CPU 대비 speedup 열과 가장 빠른 실행기 fastest 를 구하는 코드로 옳은 것은? (cpu_mean = ORT CPU 의 mean)",
    options: ["cmp[\"speedup\"] = cmp[\"mean\"] / cpu_mean · fastest = cmp.loc[cmp[\"fps\"].idxmin(), \"provider\"]", "cmp[\"speedup\"] = cmp[\"p95\"] - cpu_mean · fastest = cmp.provider.iloc[-1]", "cmp[\"speedup\"] = cpu_mean / cmp[\"mean\"] · fastest = cmp.loc[cmp[\"p95\"].idxmin(), \"provider\"]", "cmp[\"speedup\"] = cpu_mean * cmp[\"fps\"] · fastest = cmp.loc[cmp[\"mean\"].idxmax(), \"provider\"]"],
    answer: 2,
    why: "speedup = ORT CPU 평균 ÷ 각 평균 (ORT CPU 줄은 1). 실시간이면 p95 가 가장 작은 실행기가 '가장 빠른' 것. 채점은 숫자 값이 아니라 계산 방법을 봅니다.",
    anim: "D4-8" }
];
