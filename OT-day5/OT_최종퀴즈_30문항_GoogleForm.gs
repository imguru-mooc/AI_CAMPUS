/**
 * ONNX·TensorRT 최종 필기 퀴즈 (30문항, Day 1~5) — Google Form 자동 생성 스크립트
 * HYUNDAI AI Insight Campus 온디바이스 AI · ONNX·TensorRT 5일 종합 필기 퀴즈 · Day 5 8교시 (11/10) 17:00~17:25
 * (Day 1 · 2 · 3 · 4 · 5 각 6문항)
 *
 * 사용 방법
 * 1. https://script.google.com 접속 → 새 프로젝트
 * 2. 이 파일 전체를 붙여 넣고 저장
 * 3. 상단 함수 선택에서 createOTFinalQuiz 선택 → ▶ 실행 (최초 1회 권한 승인)
 * 4. 실행 로그의 편집용(editUrl) · 응답용(publishedUrl) URL 확인 — 응답용을 학생에게 공유
 *
 * 퀴즈 모드(문항당 1점, 총 30점) · 이름 필수 · 제출 직후 점수와 해설 공개 · 문항 · 보기 순서 고정
 * 회사 Workspace 계정에서 401 invalid_client 오류가 나면 개인 Google 계정으로 실행
 */

function createOTFinalQuiz() {
  const form = FormApp.create("ONNX·TensorRT 최종 필기 퀴즈 (30문항)");
  form.setIsQuiz(true)
      .setDescription(
        '범위: ONNX·TensorRT Day 1 ~ 5 (Day 1 ONNX 첫걸음 · Day 2 ONNX 속 들여다보기 · Day 3 ONNX Runtime 으로 빠르게 · Day 4 TensorRT 첫걸음 · Day 5 탐지 모델 TensorRT · 종합 프로젝트, 각 6문항)\n' +
        '문항당 1점, 총 30점 · 권장 시간 20분\n' +
        '제출 후 점수와 해설이 바로 표시됩니다. 틀린 문항은 해설의 애니메이션 링크로 복습하세요.\n' +
        '애니메이션: https://imguru-mooc.github.io/AI_CAMPUS/OT-day1/animation.html · OT-day2 · OT-day3 · OT-day4 · OT-day5/animation.html')
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

// 'D4-3' → https://imguru-mooc.github.io/AI_CAMPUS/OT-day4/animation.html#d4-3
function animUrl(anim) {
  return ANIM_BASE + 'OT-day' + anim.charAt(1) + '/animation.html#' + anim.toLowerCase();
}

// 정답 위치 분포 ①~④: 8 · 7 · 7 · 8
var QUESTIONS = [
  // ── Day 1 (6) ──
  { title: "PyTorch 의 tiny.pt (state_dict) 와 torch.onnx.export 로 만든 tiny.onnx 의 차이로 옳은 것은?",
    options: ["둘 다 그래프와 가중치를 담고 확장자만 다르다", ".pt 는 가중치만 — 계산 순서는 파이썬 코드에 있고, .onnx 는 계산 순서(그래프) + 가중치를 한 파일에 담는다", ".onnx 는 그래프만 담고 가중치는 .pt 파일에서 따로 읽는다", ".pt 는 어느 장치에서나 Python 없이 실행되고 .onnx 는 PyTorch 가 있어야 실행된다"],
    answer: 1, why: "ONNX = 누구나 읽을 수 있는 설계도. export 는 더미 입력을 한 번 흘려 보내며 지나간 연산을 '녹화' 합니다 — 더미의 shape · dtype 이 그대로 파일의 입력이 됩니다.", anim: "D1-2" },
  { title: "Netron 으로 nn.Linear(3, 2) 를 내보낸 tiny.onnx 를 열면 상자 Gemm 과 B ⟨2×3⟩ · C ⟨2⟩ 가 보입니다. 옳은 설명은?",
    options: ["Gemm = 활성 함수, B · C = 입력과 출력", "B · C 는 Netron 이 그림을 그리려고 만든 임시 값이라 파일에는 없다", "Gemm 은 학습용 노드라 추론 때는 실행되지 않는다", "Gemm = 행렬곱 + 덧셈, B = weight · C = bias (파일 안의 가중치 initializer)"],
    answer: 3, why: "y = x·Bᵀ + C (transB = 1). nn.Linear 하나가 Gemm 노드 하나가 되고, B 를 클릭하면 가중치 숫자가 보입니다. 코드로는 onnx.load(path).graph.initializer.", anim: "D1-3" },
  { title: "input_names=[\"input\"], output_names=[\"logits\"] 로 내보낸 모델을 ONNX Runtime 으로 실행하는 올바른 코드는?",
    options: ["sess = ort.InferenceSession(path, providers=[\"CPUExecutionProvider\"]); out = sess.run(None, {\"input\": x})[0]  (x 는 NumPy float32)", "sess = ort.InferenceSession(path); out = sess(x)  (x 는 torch.Tensor)", "out = ort.run(path, x)  (x 는 float64 NumPy)", "sess = ort.InferenceSession(path); out = sess.run([\"input\"], x)[\"logits\"]"],
    answer: 0, why: "입력은 {이름: NumPy 배열} 딕셔너리, 출력은 리스트라 [0] 으로 꺼냅니다. 첫 인자 None = '출력 전부'. float64 를 넣으면 tensor(double) 오류 — .astype(np.float32).", anim: "D1-4" },
  { title: "PyTorch 결과 ref 와 ONNX Runtime 결과 out 을 비교하는 방법으로 옳은 것은?",
    options: ["(out == ref).all() — 한 비트라도 다르면 export 실패", "out.argmax() == ref.argmax() 만 보면 충분하다", "np.allclose(out, ref, atol=1e-5) — float32 끝자리 차이는 정상이므로 허용 오차로 비교", "출력 shape 만 같으면 값은 비교하지 않아도 된다"],
    answer: 2, why: "두 프로그램은 같은 식을 다른 순서 · 다른 라이브러리로 계산해 끝자리가 다를 수 있습니다. '내보낸 뒤 반드시 비교한다' 가 Day 1 의 가장 중요한 습관 — Day 4 · 5 의 TensorRT 비교도 같은 원칙.", anim: "D1-4" },
  { title: "cv2.imread 로 읽은 컬러 사진 (H, W, 3) 을 MobileNetV3 입력 (1, 3, 224, 224) 로 만드는 순서로 옳은 것은?",
    options: ["resize → reshape(1, 3, 224, 224) → /255", "transpose(2, 0, 1) → BGR→RGB → [None] → resize", "/255 → [None] → reshape(3, 224, 224) → BGR→RGB", "BGR→RGB → resize (224, 224) → /255 → (x − MEAN) / STD → transpose(2, 0, 1) → [None]"],
    answer: 3, why: "OpenCV 는 (H, W, C) · BGR · uint8, 모델 입력은 NCHW · RGB · float32. reshape 는 모양만 바꿔 픽셀이 뒤섞입니다. 전처리는 학습 때와 같은 순서 · 같은 값 — Day 5 의 detect 도 BGR→RGB · /255 · CHW.", anim: "D1-7" },
  { title: "PyTorch 2.15 에서 BatchNorm 이 있는 모델을 eval() 없이 export 한 뒤 ORT 결과를 PyTorch 결과와 비교하면?",
    options: ["ONNX 파일은 같지만 (export 는 기본값으로 실행 모드 그래프를 녹화), 비교 기준인 PyTorch 결과가 BN 배치 통계로 계산되어 비교가 실패한다", "ONNX 파일에 학습 모드 BN 이 녹화되어 ORT 결과가 틀린다", "export 단계에서 바로 오류가 나서 파일이 만들어지지 않는다", "BN 은 export 에 영향이 없으므로 PyTorch · ORT 결과가 언제나 같다"],
    answer: 0, why: "달라지는 것은 ONNX 파일이 아니라 비교 기준인 PyTorch 쪽 — running 평균까지 바뀝니다. 그래서 4단계의 0번은 언제나 eval().", anim: "D1-8" },

  // ── Day 2 (6) ──
  { title: "g = onnx.load(\"mnist_cnn.onnx\").graph 에서 conv.weight 같은 가중치가 들어 있는 목록은?",
    options: ["g.input", "g.node", "g.initializer", "g.output"],
    answer: 2, why: "그래프 안의 네 목록 = input · node · initializer · output. 가중치는 파일 안에 이미 들어 있으므로 initializer 이고, numpy_helper.to_array 로 꺼냅니다. 노드는 input · output 이름이 같으면 이어집니다.", anim: "D2-2" },
  { title: "nn.GELU 가 든 모델을 opset 13 과 opset 20 으로 내보낸 결과로 옳은 것은?",
    options: ["opset 13 은 GELU 를 지원하지 않아 export 가 실패한다", "opset 13 은 Erf · Div · Mul 등으로 풀어 쓰고 opset 20 은 Gelu 한 노드 — 계산 결과는 같다", "opset 20 이 결과가 더 정확하다", "opset 은 파일 저장 형식 (IR) 버전이라 노드 구성과는 상관없다"],
    answer: 1, why: "opset = 연산 사전의 판. 사전에 단어가 있으면 한 노드, 없으면 풀어 씁니다. 실행기가 모르는 높은 opset 은 '모르는 단어' 오류 — 배포할 실행기의 지원 판부터 확인.", anim: "D2-4" },
  { title: "입력 input · 출력 logits 의 배치 축을 열어 주는 올바른 export 옵션은?",
    options: ["dynamic_axes={\"input\": {0: \"batch\"}, \"logits\": {0: \"batch\"}}", "dynamic_axes={0: \"batch\"}", "dynamic_axes=[\"input\", \"logits\"]", "dynamic_axes={\"batch\": [\"input\", \"logits\"]}"],
    answer: 0, why: "{이름: {축 번호: 축 이름}} — 세션 입력 shape 가 ['batch', 1, 28, 28] 가 됩니다. 단, TensorRT 로 배치가 자유로운 ONNX 를 빌드하려면 optimization profile 이 필요해 Day 4 · 5 는 배치 1 고정으로 내보냈습니다.", anim: "D2-5" },
  { title: "forward 안에 'if x.mean() > 0: return x * 2 else: return x - 1' 이 있는 모델을 torch.ones 더미로 export 했습니다. 음수 입력을 넣으면?",
    options: ["ONNX 에 If 노드가 자동으로 생겨 x - 1 길로 정확히 계산된다", "ONNX Runtime 이 실행 중 오류를 낸다", "export 단계에서 반드시 오류가 나 파일이 만들어지지 않는다", "오류 없이 x * 2 길로 계산된다 — 그래프에는 녹화된 길만 남는다"],
    answer: 3, why: "export 는 '실행된 길만' 녹화합니다. 고치는 법: torch.where(조건, x * 2, x - 1). TracerWarning 은 힌트일 뿐이라 더미와 다른 입력으로 결과를 비교해야 잡힙니다.", anim: "D2-6" },
  { title: "조용히 틀리는 export 를 잡아내는 verify(model, path, cases) 의 사용법으로 옳은 것은?",
    options: ["export 때 쓴 더미 입력 하나로만 비교한다", "더미와 다른 입력 (다른 값 · 음수 · 다른 배치 크기) 여러 개로 PyTorch 결과와 allclose 비교한다", "onnx.checker.check_model 이 통과하면 비교는 필요 없다", "Netron 으로 노드 수만 세어 본다"],
    answer: 1, why: "더미 하나로만 돌리면 녹화된 길과 같은 입력이라 늘 통과합니다. checker 는 형식 검사, shape_inference 는 shape 채우기 — 둘 다 '결과가 맞는지' 는 말해 주지 않습니다.", anim: "D2-6" },
  { title: "onnxslim 으로 모델을 단순화한 뒤 해야 할 일과 그 이유로 옳은 것은?",
    options: ["다시 학습한다 — 단순화하면 가중치가 바뀐다", "opset 을 낮춘다 — 단순화하면 opset 이 올라간다", "원래 모델과 결과를 다시 비교한다 — 단순화는 같은 계산을 더 짧게 적는 것이라 결과가 같아야 한다", "아무것도 하지 않는다 — onnxslim 은 결과를 보장한다"],
    answer: 2, why: "상수 접기 등으로 Shape · Gather · Concat 같은 노드가 접혀 노드가 줄고 읽기 쉬워집니다 (TensorRT 변환도 덜 까다로움). 그래도 '바꾸면 비교' 는 언제나.", anim: "D2-7" },

  // ── Day 3 (6) ──
  { title: "InferenceSession(path, providers=[\"CUDAExecutionProvider\", \"CPUExecutionProvider\"]) 로 만든 세션이 실제로 GPU 에서 도는지 확인하는 방법은?",
    options: ["ort.get_available_providers() 에 CUDA 가 있으면 GPU 로 돈다", "providers 목록 첫 칸에 CUDA 를 적었으면 무조건 GPU 로 돈다", "세션을 만든 뒤 sess.get_providers()[0] 이 CUDAExecutionProvider 인지 본다", "세션을 만들 때 오류가 없으면 GPU 로 돈다"],
    answer: 2, why: "providers 는 '희망 목록' — 안 되면 오류 없이 다음 실행기로 넘어갑니다. get_available_providers() 는 설치된 목록일 뿐.", anim: "D3-1" },
  { title: "추론 시간을 잴 때 앞의 몇 번 (warm-up) 을 버리는 이유는?",
    options: ["첫 실행에는 메모리 확보 · 캐시 채우기 · 커널 선택 같은 준비가 섞여 느리게 나오기 때문", "앞의 실행은 결과가 틀리기 때문", "warm-up 을 해야 모델 가중치가 GPU 로 학습되기 때문", "평균을 일부러 작게 보이게 하려고"],
    answer: 0, why: "warm-up 10번 버리기 → 100번 재기 → mean · p50 · p95 · FPS (bench). GPU · TensorRT 는 처음 준비할 것이 더 많아 warm-up 이 더 중요합니다 (Day 5 4교시도 5 프레임 warm-up).", anim: "D3-2" },
  { title: "실시간 카메라 시스템의 추론 시간을 비교할 때 가장 중요하게 보는 숫자는?",
    options: ["최소값 — 가장 빠를 때의 실력", "최대값 — 가장 느렸던 한 번", "첫 실행 시간", "p95 — 가끔 튀는 지연 (오른쪽 꼬리) 까지 반영한다"],
    answer: 3, why: "평균이 빨라도 가끔 튀면 그 순간 프레임을 놓칩니다. 최대값은 한 번의 사고에 크게 흔들려 비교용으로 쓰지 않습니다. p95 = np.percentile(times, 95).", anim: "D3-2" },
  { title: "CPU 코어가 2개인 Colab 에서 intra_op_num_threads 를 4 로 늘리면 일반적으로?",
    options: ["스레드 수에 비례해 정확히 2배 빨라진다", "코어보다 일꾼이 많아 작업대를 번갈아 써서 p95 부터 나빠진다", "자동으로 GPU 를 함께 쓴다", "옵션이 무시되고 항상 1 스레드로 돈다"],
    answer: 1, why: "스레드 = 일꾼, 코어 = 작업대 — 코어 수가 상한. 기본값 0 = 자동. 옵션은 SessionOptions 에 정한 뒤 세션을 만들어야 적용됩니다.", anim: "D3-3" },
  { title: "ORT graph_optimization_level 의 단계와 하는 일 짝으로 옳은 것은?",
    options: ["BASIC = 노드 합치기, EXTENDED = 상수 접기, ALL = FP16 변환", "ALL = 모든 최적화를 끈다", "DISABLE 이 기본값이고 ALL 은 GPU 에서만 쓸 수 있다", "BASIC = 상수 접기, EXTENDED = 노드 합치기 (FusedConv 등), ALL = 레이아웃 (NCHWc) 까지 · 기본값"],
    answer: 3, why: "합치기 = 메모리 왕복 감소. ALL 의 레이아웃 최적화는 그 장치 전용이라 배포는 원래 ONNX 를 주고 장치에서 세션을 만들 때 최적화합니다 — TensorRT 엔진을 장치에서 다시 빌드하는 것과 같은 생각.", anim: "D3-4" },
  { title: "배치 16 의 한 번 run 지연이 200 ms 입니다 (가정). 처리량과 장당 시간은?",
    options: ["처리량 5 장/초, 장당 200 ms", "처리량 80 장/초, 장당 12.5 ms", "처리량 3,200 장/초, 장당 0.3 ms", "처리량 16 장/초, 장당 62.5 ms"],
    answer: 1, why: "처리량 = 배치 × 1000 ÷ 지연 = 16 × 1000 ÷ 200 = 80 장/초, 장당 = 200 ÷ 16 = 12.5 ms. 처리량은 배치가 유리하지만 실시간 응답은 배치 1 · p95 로 판단합니다.", anim: "D3-6" },

  // ── Day 4 (6) ──
  { title: "Colab T4 (sm_75) 에서 만든 TensorRT 엔진을 Jetson Orin (sm_87) 에서 쓰려면?",
    options: [".onnx 를 가져가 Jetson 에서 그 장치의 TensorRT 로 엔진을 다시 빌드한다", ".engine 파일을 복사하면 그대로 읽힌다", ".engine 을 ONNX Runtime 으로 열면 어느 GPU 에서나 돈다", "엔진 파일 이름만 sm_87 로 바꾼다"],
    answer: 0, why: "엔진 = 만든 GPU 세대 (compute capability) · TensorRT 버전 전용 완성품. 배포할 것은 설계도 (.onnx) — Day 5 다음 과목의 Jetson 도 같은 원칙.", anim: "D4-1" },
  { title: "polygraphy run model.onnx --onnxrt --trt --atol 1e-3 --rtol 1e-3 의 판정 식은? (a = TensorRT 출력, b = ONNX Runtime 출력)",
    options: ["|a − b| ≤ atol × rtol", "|a| ≤ atol 이고 |b| ≤ rtol", "|a − b| ≤ atol + rtol · |b|", "|a − b| ≤ rtol − atol · |b|"],
    answer: 2, why: "atol 은 값 크기와 상관없이 더하는 몫, rtol 은 |b| 에 비례하는 몫. FP16 은 유효숫자가 약 3자리라 이 엄격한 기준에서 FAILED 가 흔해, 더 느슨한 기준 + 실제 데이터 정확도로 판단합니다.", anim: "D4-3" },
  { title: "Python 으로 엔진을 빌드하는 build_engine() 에서 FP16 을 켜는 곳은?",
    options: ["builder config — config.set_flag(trt.BuilderFlag.FP16)", "OnnxParser — parser.parse_from_file(path, fp16=True)", "실행할 때 — context.execute_async_v3(fp16=True)", "입력을 np.float16 으로 바꿔 넣으면 자동으로 FP16 엔진이 된다"],
    answer: 0, why: "Logger → Builder → Network → OnnxParser (결과 확인) → Config (WORKSPACE · DETAILED · FP16) → build_serialized_network. FP16 플래그는 '허락' — 레이어마다 재 보고 일부는 FP32 로 남깁니다. 입출력은 FP32 그대로.", anim: "D4-4" },
  { title: "TensorRT 실행에서 execute_async_v3 뒤에 torch.cuda.synchronize() 를 부르는 이유는?",
    options: ["엔진을 다시 빌드하기 위해", "GPU 메모리를 비우기 위해", "execute_async_v3 는 실행을 시작만 하고 바로 돌아오므로 끝날 때까지 기다려야 결과 · 시간이 올바르다", "출력을 FP16 으로 바꾸기 위해"],
    answer: 2, why: "기다리지 않고 읽으면 결과가 비어 있을 수 있고, 시간을 재면 터무니없이 빠르게 나옵니다. 입력 · 출력은 GPU 메모리 주소 (CUDA 텐서의 data_ptr()) 로 set_tensor_address 에 알려 줍니다.", anim: "D4-5" },
  { title: "FP16 엔진의 출력 최대 차이 (max_diff) 가 FP32 엔진보다 훨씬 큽니다. 쓸 수 있는지 판단하는 방법은?",
    options: ["max_diff 가 1e-3 을 넘으면 무조건 버린다", "속도가 2배 이상 빨라졌으면 정확도는 보지 않는다", "np.float16 의 최대값 65504 를 넘는지만 본다", "실제 데이터의 정확도 (분류 정확도 · 탐지 AP) 가 FP32 / ORT 와 거의 같은지 본다"],
    answer: 3, why: "FP16 = 부호 1 · 지수 5 · 가수 10, eps ≈ 0.000977 (유효숫자 약 3자리). '거의 같다' 를 꼭 재 봐야 합니다 — Day 4 는 MNIST 정확도, Day 5 는 Penn-Fudan AP 로.", anim: "D4-6" },
  { title: "엔진 인스펙터에서 레이어 이름이 '/features.0/Conv + /features.0/HardSwish' 처럼 ' + ' 로 이어진 것의 뜻은?",
    options: ["두 엔진을 이어 붙였다", "ONNX 노드 여러 개를 하나로 합친 레이어 — 메모리 왕복이 줄어 빨라지는 이유 중 하나", "FP16 과 FP32 를 함께 쓰는 레이어다", "빌드 중 오류가 난 레이어다"],
    answer: 1, why: "TensorRT 가 빨라진 이유 = 레이어 합치기 + 이 GPU 에서 재 보고 고른 커널 (tactic) + FP16 · Tensor Core. 레이어 수만으로 단정하지 말고 결국 재 봐야 합니다. LayerType 을 보려면 빌드 때 profiling_verbosity = DETAILED.", anim: "D4-7" },

  // ── Day 5 (6) ──
  { title: "Day 5 의 TRTInfer 클래스에 대한 설명으로 옳은 것은?",
    options: ["__call__ 을 부를 때마다 엔진을 다시 deserialize 하고 버퍼를 새로 만든다", "__init__ 에서 엔진 · Context · 입출력 이름 · GPU 버퍼 · 주소를 한 번 준비하고, __call__ 이 있어 yolo16(x) 처럼 함수처럼 부를 수 있다", "__init__ 이 프레임을 받아 탐지 결과 박스를 돌려준다", "TRTInfer 는 ONNX Runtime 세션을 감싼 클래스다"],
    answer: 1, why: "__call__(x) = 복사 → execute_async_v3 → synchronize → NumPy. 함수처럼 부를 수 있어 ORT 세션을 감싼 ort_infer 와 같은 모양으로 바꿔 끼웁니다. build_once 는 엔진이 이미 있으면 다시 빌드하지 않습니다.", anim: "D5-1" },
  { title: "YOLOv8n 엔진의 입출력은 images (1, 3, 640, 640) → output0 (1, 84, 8400) 입니다. 84 와 8400 의 뜻은?",
    options: ["84 = 박스 4 + COCO 80 클래스 점수, 8400 = 80² + 40² + 20² 후보 위치", "84 = 클래스 84개, 8400 = 탐지된 사람 수", "84 = 입력 640 ÷ 8 근처 값, 8400 = 픽셀 수", "84 = 후보 위치 수, 8400 = 클래스 점수 개수"],
    answer: 0, why: "엔진이 바뀌어도 이 모양은 그대로라 VB 의 letterbox · decode_yolo (NMS) 를 그대로 씁니다. 박스는 (b − pad) / r 로 원래 프레임 좌표로 되돌립니다.", anim: "D5-2" },
  { title: "ORT · TRT FP32 · TRT FP16 의 AP 를 비교하는 올바른 방법은?",
    options: ["실행기마다 가장 잘 나오는 전처리를 따로 고른다", "화면에 그릴 때와 같은 conf 0.25 로 탐지해 AP 를 잰다", "FP16 은 AP 대신 속도만 비교한다", "같은 사진 · 같은 detect · 같은 evaluate_ap 로, 낮은 conf (0.01) 로 탐지해 AP 를 잰다"],
    answer: 3, why: "conf 0.25 로 재면 recall 곡선이 짧아져 AP 가 낮게 나옵니다. 기대 결과는 FP32 = ORT, FP16 ≈ ORT (실행 예 ORT CPU AP50 0.968). FP32 엔진과 ORT 가 크게 다르면 대개 BGR/RGB · dtype 실수.", anim: "D5-3" },
  { title: "한 프레임을 전처리 · 추론 · 후처리 · 그리기로 나눠 쟀습니다. 추론을 TensorRT 로 바꾼 뒤 일반적으로 일어나는 일은?",
    options: ["네 조각이 모두 같은 비율로 빨라진다", "파이프라인 FPS = 1000 ÷ 추론 시간이 된다", "추론 비중이 줄어 CPU 쪽 (전처리 · Python NMS · 그리기) 이 새 병목이 된다 — 다음 개선은 GPU 전처리 · GPU NMS", "전처리 · 후처리 시간도 GPU 덕분에 0 이 된다"],
    answer: 2, why: "프레임 FPS = 1000 ÷ (네 조각의 합). 실행 예 (CPU 2코어) ORT CPU: pre 3.5 · infer 75 · post 1.4 · draw 0.8 ms → 약 12.4 FPS. 그래서 추론만의 배수보다 파이프라인 배수가 작습니다.", anim: "D5-4" },
  { title: "VB 의 침입 감지기를 TensorRT 로 옮긴 process_frame 에 대해 옳은 것은?",
    options: ["TensorRT 용으로 발 위치 · 구역 판정 · 그리기를 모두 새로 짰다", "구역 안에 아무도 없으면 streak 를 1 씩 줄인다", "한 프레임만 구역에 들어와도 바로 경보를 울린다", "바꾼 것은 detect 에 넘기는 infer 하나 — 발 위치가 구역 안이면 streak + 1, 아니면 0, streak ≥ 3 이면 경보"],
    answer: 3, why: "debounce 로 한두 프레임 튀는 오탐을 막습니다. 영상을 여는 곳은 run_intrusion 의 source 한 곳 — 다음 과목에서 카메라로 바꿉니다.", anim: "D5-5" },
  { title: "people.mp4 (10 FPS · 795 프레임) 의 프레임별 alarm 을 이벤트로 요약합니다. 옳은 것은?",
    options: ["경보가 꺼진 프레임이 i 면 이벤트 끝은 i − 1, 파이프라인 FPS 가 영상 FPS (10) 보다 크면 실시간으로 따라간다", "경보가 꺼진 프레임 i 를 이벤트 끝으로 기록하고, 추론만의 FPS 가 10 보다 크면 실시간이다", "이벤트 수는 경보가 켜진 프레임 수와 같다", "duration 은 end − start 이므로 한 프레임짜리 이벤트의 길이는 0 초다"],
    answer: 0, why: "duration = end − start + 1/fps (한 프레임 = 0.1 초). 이벤트 수 = 0→1 로 켜진 횟수 (실행 예 ORT 기반: 경보 221 프레임 · 이벤트 5번). 실시간 판단은 읽기 · 저장까지 포함한 파이프라인 FPS 로.", anim: "D5-6" }
];
