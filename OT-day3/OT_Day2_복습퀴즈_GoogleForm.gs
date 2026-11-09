/**
 * ONNX·TensorRT Day 2 복습 퀴즈 — Google Form 자동 생성 스크립트
 * HYUNDAI AI Insight Campus 온디바이스 AI · ONNX·TensorRT Day 3 (11/6) 09:00~09:15
 *
 * 사용 방법
 * 1. https://script.google.com 접속 → 새 프로젝트
 * 2. 이 파일 전체를 붙여 넣고 저장
 * 3. 상단 함수 선택에서 createOTDay2Quiz 선택 → ▶ 실행 (최초 1회 권한 승인)
 * 4. 하단 실행 로그에 출력되는 두 URL 확인
 *    - 편집용(editUrl): 강사용, 응답 확인·설정 변경
 *    - 응답용(publishedUrl): 학생에게 공유
 *
 * 생성되는 폼 설정
 * - 퀴즈 모드(자동 채점, 문항당 1점, 총 20점)
 * - 이름 필수 입력, 문항 전부 필수
 * - 제출 직후 점수 공개, 정답·해설·복습할 애니메이션 링크 표시
 * - 문항 순서 고정(Day 2 교시 순), 보기 순서도 고정
 * - 회사 Workspace 계정에서 401 invalid_client 오류가 나면 개인 Google 계정으로 실행
 */

function createOTDay2Quiz() {
  const form = FormApp.create('ONNX·TensorRT Day 2 복습 퀴즈');
  form.setIsQuiz(true)
      .setDescription(
        '범위: ONNX·TensorRT Day 2 전체 (ONNX 파일의 속 구조 · 이름으로 잇기 · describe_onnx · opset · checker · shape_inference · dynamic_axes · 조용히 틀리는 export · 미지원 연산 · onnxslim · BrightNet)\n' +
        '문항당 1점, 총 20점 · 권장 시간 15분\n' +
        '제출 후 점수와 해설이 바로 표시됩니다. 틀린 문항은 해설의 애니메이션 링크로 복습하세요.\n' +
        '애니메이션: https://imguru-mooc.github.io/AI_CAMPUS/OT-day2/animation.html')
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
var ANIM_URL = 'https://imguru-mooc.github.io/AI_CAMPUS/OT-day2/animation.html';

var QUESTIONS = [
  { title: "export 전에 model.eval() 을 빠뜨리고 ONNX Runtime 결과를 PyTorch 결과와 비교하면? (PyTorch 2.15, BatchNorm 이 있는 모델)",
    options: ["export 가 학습 모드 그래프를 녹화해 ONNX 파일이 달라진다", "export 가 오류로 멈춘다", "ONNX 파일은 같지만(export 는 기본값으로 실행 모드 녹화), 비교 기준인 PyTorch 결과가 BN 배치 통계로 계산되어 비교가 실패한다", "아무 차이 없다 — 비교도 통과한다"],
    answer: 2,
    why: "torch.onnx.export 는 기본값으로 실행 모드 그래프를 녹화합니다. 달라지는 것은 ONNX 파일이 아니라 비교 기준인 PyTorch 쪽 — 그래서 4단계의 0번은 언제나 eval().",
    anim: "D2-1" },
  { title: "g = onnx.load(\"mnist_cnn.onnx\").graph 안의 네 가지 목록이 아닌 것은?",
    options: ["g.input", "g.node", "g.initializer", "g.opset_import"],
    answer: 3,
    why: "그래프(GraphProto) 안의 네 목록 = input · node · initializer · output. opset 정보는 한 단계 바깥인 모델(ModelProto) 에 있습니다 — m.opset_import[0].version.",
    anim: "D2-2" },
  { title: "mnist_cnn.onnx 에서 m.ir_version 은 9, m.opset_import[0].version 은 20 입니다. 두 숫자의 뜻은?",
    options: ["ir_version = 파일을 저장하는 형식의 버전, opset = 그 파일이 쓰는 '연산 사전' 의 판 번호", "둘 다 PyTorch 버전", "ir_version = 연산 사전의 판, opset = 파일 형식의 버전", "ir_version = 노드 수, opset = 가중치 텐서 수"],
    answer: 0,
    why: "파일 형식(IR) 과 연산 사전(opset) 은 따로 버전이 붙습니다. opset_version 을 지정하지 않은 mnist_cnn.onnx 의 opset 은 20, producer 는 pytorch 2.15.0.",
    anim: "D2-2" },
  { title: "Netron 에서 보던 노드 사이의 화살표는 ONNX 파일 안에서 어떻게 표현될까요?",
    options: ["'화살표 목록' 이 따로 저장되어 있다", "앞 노드의 output 이름과 뒤 노드의 input 이름이 같으면 이어진다", "initializer 의 순서로 정해진다", "파일에는 없고 Netron 이 shape 를 보고 추측한다"],
    answer: 1,
    why: "노드는 input 이름 목록과 output 이름 목록만 가집니다 — 이름이 연결선입니다. 예: Relu 의 input '/conv/Conv_output_0' = Conv 의 output. 노드는 계산 순서(topological order) 로 저장됩니다.",
    anim: "D2-2" },
  { title: "mnist_cnn.onnx (input 1 · output 1 · node 7 · initializer 6) 에서 conv.weight 같은 가중치가 들어 있는 곳은?",
    options: ["g.input", "g.value_info", "g.initializer", "g.output"],
    answer: 2,
    why: "가중치는 파일 안에 이미 들어 있으므로 input 이 아니라 initializer 입니다. 이름은 PyTorch 변수 이름 그대로 (conv.weight · fc1.bias …) — Netron 의 B · C 가 바로 이것.",
    anim: "D2-2" },
  { title: "initializer 하나 t 를 NumPy 배열로 바꾸는 함수는?",
    options: ["onnx.helper.get_attribute_value(t)", "t.numpy()", "np.array(t)", "onnx.numpy_helper.to_array(t)"],
    answer: 3,
    why: "가중치 = numpy_helper.to_array, 속성 = helper.get_attribute_value. W = {t.name: to_array(t) …} 로 만든 W[\"conv.weight\"] 는 PyTorch cnn.conv.weight 와 같습니다.",
    anim: "D2-3" },
  { title: "W[\"fc1.weight\"] (shape (100, 4320)) 의 원소 개수를 구하는 코드와 값은?",
    options: ["W[\"fc1.weight\"].size → 432,000", "W[\"fc1.weight\"].shape → 432,000", "len(W[\"fc1.weight\"]) → 432,000", "W[\"fc1.weight\"].ndim → 432,000"],
    answer: 0,
    why: ".shape 는 (100, 4320) 이라는 모양, len 은 첫 축 100, 개수는 .size (모든 축을 곱한 값) = 432,000. fc1.weight 하나가 전체 433,890 의 99.6% — describe_onnx 의 params 열에서 Gemm (fc1) 이 432,100.",
    anim: "D2-3" },
  { title: "SafetyNet 의 PyTorch 파라미터는 1,491 개인데 describe_onnx 의 params 합계는 1,443 개입니다. 차이 48 의 이유는?",
    options: ["ONNX 가 bias 를 저장하지 않아서", "BatchNorm 의 γ · β 가 앞 Conv 의 weight · bias 에 합쳐져서 (Conv-BN fusion)", "Dropout 의 가중치가 빠져서", "float16 으로 저장되어 개수가 줄어서"],
    answer: 1,
    why: "실행 모드 BN 은 '곱하고 더하기' 라 앞 Conv 에 미리 녹여 넣을 수 있습니다. Conv 224 + Conv 1,168 + Gemm 51 = 1,443.",
    anim: "D2-3" },
  { title: "nn.Sequential(nn.Linear(4, 4), nn.GELU()) 를 opset 13 과 opset 20 으로 내보낸 결과는?",
    options: ["opset 13 은 Gelu 한 노드, opset 20 은 여러 노드로 풀어 쓴다", "둘 다 Gelu 한 노드", "opset 13 은 Div · Erf · Add · Mul 등으로 풀어 쓰고, opset 20 은 Gelu 한 노드 — 두 결과 최대 차이 0.0", "opset 13 에서는 export 가 오류로 멈춘다"],
    answer: 2,
    why: "opset = 연산 사전의 판. 20 판에는 'Gelu' 라는 단어가 있어 한 노드로, 그 전 판에는 단어가 없어 풀어 씁니다. 같은 계산입니다. 실행기가 모르는 높은 opset 은 '모르는 단어' 오류 — 배포할 실행기의 지원 판부터 확인.",
    anim: "D2-4" },
  { title: "onnx.checker.check_model · onnx.shape_inference.infer_shapes 에 대해 옳은 것은?",
    options: ["checker 를 통과하면 결과가 PyTorch 와 같다는 것도 확인된 것이다", "infer_shapes(m) 는 m 을 직접 바꾸므로 m.graph.value_info 를 보면 된다", "infer_shapes 는 shape 를 채운 새 모델을 돌려준다 — mi = infer_shapes(m) 의 value_info 가 0 → 6 (중간 화살표)", "infer_shapes 는 input · output 을 포함한 화살표 7개 모두를 value_info 에 채운다"],
    answer: 2,
    why: "checker = 형식 검사, shape_inference = 중간 shape 채우기. 둘 다 '결과가 맞는지' 는 말해 주지 않아 allclose 로 따로 확인합니다. input · output 은 이미 shape 가 있어 중간 6개만 채워집니다.",
    anim: "D2-4" },
  { title: "입력 input · 출력 logits 의 배치 축을 열어 주는 올바른 export 옵션은?",
    options: ["dynamic_axes=[0]", "dynamic_axes={\"batch\": 0}", "dynamic_axes={0: \"input\"}", "dynamic_axes={\"input\": {0: \"batch\"}, \"logits\": {0: \"batch\"}}"],
    answer: 3,
    why: "{이름: {축 번호: 축 이름}}. 키는 input_names · output_names 와 같은 이름이어야 하고, 입력 · 출력 둘 다 적는 습관을. 더미는 여전히 배치 1 이어도 됩니다.",
    anim: "D2-5" },
  { title: "dynamic_axes 로 내보낸 mnist_cnn_dyn.onnx 에서 sess.get_inputs()[0].shape 의 결과는?",
    options: ["[1, 1, 28, 28]", "[None, 1, 28, 28]", "['batch', 1, 28, 28]", "[-1, 1, 28, 28]"],
    answer: 2,
    why: "첫 축이 숫자 1 대신 이름 'batch' (dim_param) 가 됩니다. 출력은 ['batch', 10] — 배치 1 · 4 · 8 → (1, 10) · (4, 10) · (8, 10). 노드 구성은 고정 버전과 같습니다.",
    anim: "D2-5" },
  { title: "dynamic_axes 로 높이 · 너비 (2 · 3번 축) 까지 열고 SimpleConvNet 에 32×32 입력을 넣으면?",
    options: ["잘 실행된다 — dynamic 이면 어떤 크기든 된다", "export 단계에서 오류가 난다", "자동으로 28×28 로 줄여서 실행한다", "export 는 성공하지만 실행 중 Gemm 에서 오류 — Flatten 뒤 5880 개를 4320 개용 fc1 에 넣을 수 없다"],
    answer: 3,
    why: "32 − 5 + 1 = 28 → 28 / 2 = 14 → 30 × 14 × 14 = 5880. dynamic 은 '바뀔 수 있다' 는 약속일 뿐, 처리할 수 있는지는 모델 구조가 정합니다.",
    anim: "D2-5" },
  { title: "Gate (평균이 양수면 x * 2, 아니면 x − 1) 를 더미 torch.ones(1, 3) 으로 export 했습니다. gate.onnx 에 [-3, -2, -1] 을 넣은 결과는? (PyTorch 는 [-4, -3, -2])",
    options: ["[-6, -4, -2] — 그래프에는 녹화된 x * 2 길 (Constant · Mul) 만 있어 오류 없이 틀린다", "[-4, -3, -2] — PyTorch 와 같다", "실행 오류가 난다", "[-3, -2, -1] — 입력이 그대로 나온다"],
    answer: 0,
    why: "export 는 '실행된 길만' 녹화합니다. 파이썬 if · for 는 그래프에 남지 않습니다. 고치는 법: torch.where(조건, x * 2, x - 1) — 두 길을 모두 계산하고 조건으로 고릅니다.",
    anim: "D2-6" },
  { title: "export 할 때 '그때의 값' 이 상수로 굳어 버리는 코드는?",
    options: ["float(np.mean(x.detach().numpy()))", "x.flatten(1)", "x - x.mean(dim=1, keepdim=True)", "torch.where(m > 0, x * 2, x - 1)"],
    answer: 0,
    why: ".numpy() · .item() · float() · int() · .tolist() 는 지금 값을 파이썬 숫자로 꺼냅니다 → 그래프에서는 Constant. torch 연산은 매번 계산하는 노드(ReduceMean 등) 로 녹화됩니다.",
    anim: "D2-6" },
  { title: "조용히 틀리는 export 를 잡아내는 verify(model, path, cases) 사용법으로 옳은 것은?",
    options: ["더미 입력 하나로만 비교하면 충분하다", "더미와 다른 입력 (다른 값 · 음수 · 다른 배치 크기) 여러 개로 PyTorch 결과와 allclose 비교한다", "TracerWarning 이 없으면 비교하지 않아도 된다", "checker 를 통과하면 verify 는 필요 없다"],
    answer: 1,
    why: "더미 하나로만 돌리면 녹화된 길과 같은 입력이라 늘 ✅ 입니다. TracerWarning 은 힌트일 뿐 — 결과 비교만이 최종 확인입니다.",
    anim: "D2-6" },
  { title: "Exporting the operator 'aten::median' to ONNX opset version 20 is not supported — 이 경우 고치는 방법은?",
    options: ["opset_version 을 낮춰 다시 export 한다", "오류를 무시하고 만들어진 부분만 쓴다", "Netron 에서 Median 노드를 직접 그려 넣는다", "같은 계산을 지원되는 연산 조합으로 바꿔 쓴다 (예: torch.sort → TopK 로 가운데 값) 뒤 원래 모델과 비교"],
    answer: 3,
    why: "최신 판(20) 에도 없는 연산이면 지원되는 연산으로 다시 씁니다. 모델 구조를 바꿨으면 verify 의 첫 인자로 원래 모델 (torch.median) 을 넣어 의도와 같은지 확인합니다.",
    anim: "D2-7" },
  { title: "x.view(x.size(0), -1) 를 쓴 viewnet.onnx (11개 노드) 를 onnxslim 으로 단순화하면?",
    options: ["Conv · Relu · Reshape · Gemm 4개 — 배치 크기를 읽어 shape 를 조립하던 Shape · Gather · Unsqueeze · Concat 이 상수로 접힘, 결과 최대 차이 0.0", "노드는 그대로 11개이고 가중치만 압축된다", "Reshape 까지 사라져 배치 4 는 실행되지 않는다", "가중치를 FP16 으로 바꿔 결과가 조금 달라진다"],
    answer: 0,
    why: "onnxslim = 같은 계산을 더 짧게 적기 (상수 접기 등). 결과는 같아야 하므로 단순화 후에도 반드시 비교합니다. 노드가 적으면 읽기 쉽고 Day 4 TensorRT 변환도 덜 까다롭습니다.",
    anim: "D2-7" },
  { title: "BrightNet 의 고장 ① if x.mean() < 0.3 ② NumPy 평균 빼기 ③ x.view(1, -1) 을 고친 코드로 옳은 짝은?",
    options: ["① 그대로 ② float(x.mean()) ③ x.view(-1)", "① torch.where(mean < 0.3, x * 2, x) ② x - x.mean(dim=(1, 2, 3), keepdim=True) ③ x.flatten(1)", "① for 문으로 사진마다 ② np.mean 그대로 ③ x.reshape(1, -1)", "① opset 20 으로 올리기 ② onnxslim ③ dynamic_axes 만 추가"],
    answer: 1,
    why: "6교시의 세 가지 고치는 법 그대로 — if → torch.where, 값을 꺼내는 NumPy → torch 연산 (사진마다 평균), 숫자로 고정한 배치 → flatten(1). 고친 모델 vs 의도 (한 장씩) 최대 차이 1.49e-08.",
    anim: "D2-8" },
  { title: "brightnet_final.onnx 가 checker · onnxslim · shape_inference 를 모두 통과했습니다. 배포 전에 남은 일은?",
    options: ["바로 배포한다 — 세 도구가 결과까지 확인했다", "opset 을 20 으로 올려 다시 export 한다", "intended (한 장씩 넣은 PyTorch) 와 배치 1 · 8 로 직접 비교한다", "Netron 으로 노드 수만 확인한다"],
    answer: 2,
    why: "세 도구는 형식 검사 · 단순화 · shape 채우기만 합니다. 기준은 '원래 코드' 가 아니라 의도 — 실행 예 배치 1 · 8 모두 최대 차이 9.7e-08. 배포 담당자에게 opset 17 · 입력 [batch, 3, 64, 64] · 전처리 · 출력 클래스 순서를 함께 전달합니다.",
    anim: "D2-8" }
];
