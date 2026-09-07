/**
 * Python Day 3 복습 퀴즈 — Google Form 자동 생성 스크립트
 * HYUNDAI AI Insight Campus 온디바이스 AI
 *
 * 사용 방법
 * 1. https://script.google.com 접속 → 새 프로젝트
 * 2. 이 파일 전체를 붙여 넣고 저장
 * 3. 상단 함수 선택에서 createDay3Quiz 선택 → ▶ 실행 (최초 1회 권한 승인)
 * 4. 하단 실행 로그에 출력되는 두 URL 확인
 *    - 편집용(editUrl): 강사용, 응답 확인·설정 변경
 *    - 응답용(publishedUrl): 학생에게 공유
 *
 * 생성되는 폼 설정
 * - 퀴즈 모드(자동 채점, 문항당 1점, 총 20점)
 * - 이름 필수 입력, 문항 전부 필수
 * - 제출 직후 점수 공개, 정답·해설 피드백 표시
 * - 문항 순서 고정(교시 순), 보기 순서도 고정
 */

function createDay3Quiz() {
  const form = FormApp.create('Python Day 3 복습 퀴즈');
  form.setIsQuiz(true)
      .setDescription(
        '범위: Day 3 전체 (클래스 기초·심화 · 파일 입출력 · JSON·CSV · 시스템 연동 · SensorLogger)\n' +
        '문항당 1점, 총 20점 · 권장 시간 20분\n' +
        '제출 후 점수와 해설이 바로 표시됩니다. 틀린 문항은 해설의 애니메이션으로 복습하세요.')
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
    item.setFeedbackForCorrect(
      FormApp.createFeedback().setText('정답입니다. ' + q.why + (q.anim ? '  [복습: ' + q.anim + ']' : '')).build());
    item.setFeedbackForIncorrect(
      FormApp.createFeedback().setText('정답: ' + LABEL[q.answer] + ' ' + q.options[q.answer] + ' — ' + q.why + (q.anim ? '  [복습: ' + q.anim + ']' : '')).build());
  });

  Logger.log('편집용  URL: ' + form.getEditUrl());
  Logger.log('응답용  URL: ' + form.getPublishedUrl());
}

var LABEL = ['①', '②', '③', '④'];

var QUESTIONS = [
  { title: 'class Pin: 안에서 __init__ 이 number = n 으로만 대입했다. p = Pin(3); print(p.number) 의 결과는?',
    options: ['3', 'None', 'AttributeError', '0'],
    answer: 2,
    why: 'number = n 은 지역 변수입니다. self.number = n 이어야 인스턴스에 저장됩니다.',
    anim: 'D3-1 · 지역 변수 함정' },

  { title: 's = S() 일 때 print(S.hello(s) == s.hello()) 의 출력은?',
    options: ['True', 'False', '오류', 'None'],
    answer: 0,
    why: '인스턴스.메서드() 는 클래스.메서드(인스턴스) 의 줄임 — 완전히 같은 호출입니다.',
    anim: 'D3-1 ④ · self 의 정체' },

  { title: 'a = R(1); b = R(2); b.t = 9 실행 후 print(a.t, b.t) 의 출력은?',
    options: ['9 9', '1 9', '1 2', '오류'],
    answer: 1,
    why: '인스턴스 속성은 각자 독립 — b 의 변경은 a 에 영향이 없습니다.',
    anim: 'D3-1 ③ · 독립' },

  { title: 'class A: def f(): return 1 로 정의하고 A().f() 를 부르면?',
    options: ['1', 'None', 'AttributeError', 'TypeError'],
    answer: 3,
    why: '인스턴스 호출은 self 를 자동 전달하는데 받을 자리가 없어 TypeError 가 납니다.',
    anim: '1교시 · self 누락' },

  { title: 'class C: K = 10 에서 a = C(); b = C(); C.K = 20 실행 후 print(a.K, b.K) 는?',
    options: ['10 10', '10 20', '20 20', '오류'],
    answer: 2,
    why: '클래스 변수는 설계도에 한 부 — 클래스로 바꾸면 모든 인스턴스에 전파됩니다.',
    anim: 'D3-1 ⑥ · 전파' },

  { title: 'class C: K = 10 에서 a.K = 99 대입 후 print(a.K, b.K, C.K) 는?',
    options: ['99 99 99', '99 10 10', '10 10 10', '오류'],
    answer: 1,
    why: '인스턴스 대입은 a 에만 새 변수를 만들어 클래스 값을 가립니다 — b 와 C 는 그대로.',
    anim: 'D3-1 ⑥ · 가림' },

  { title: '자식 Ch 가 __init__ 에서 self.y = 2 만 정의(super 누락)했다. c = Ch(); print(c.x) 의 결과는? (부모는 self.x = 1)',
    options: ['1', '2', 'AttributeError', 'None'],
    answer: 2,
    why: '자식이 __init__ 을 정의하면 부모 것은 자동으로 불리지 않아 x 가 없습니다 — super().__init__().',
    anim: 'D3-2 ⑤ · super 누락' },

  { title: 'class Ch(P): pass 이고 P 에만 who() 가 있을 때 print(Ch().who()) 는?',
    options: ['P', '오류', 'None', 'Ch'],
    answer: 0,
    why: '자식에 없는 메서드는 부모로 올라가 찾아 실행됩니다 — 자식→부모 탐색.',
    anim: 'D3-2 ② · 탐색' },

  { title: '"w" 로 "A\\n" 저장 후 다시 "w" 로 "B\\n" 저장했다. 파일 내용은?',
    options: ['A 와 B 두 줄', 'B', 'A', '빈 파일'],
    answer: 1,
    why: '두 번째 "w" 가 여는 순간 기존 내용을 지웁니다 — 남는 것은 B 뿐.',
    anim: 'D3-3 ① · w 가 지우는 순간' },

  { title: '"w" 로 "A\\n" 저장 후 "a" 로 "B\\n" 저장했다. 파일 내용은?',
    options: ['B 만', 'A 만', 'A 와 B 두 줄', '오류'],
    answer: 2,
    why: '"a" 는 기존 내용 뒤에 붙입니다 — 로그 이어쓰기의 표준 모드.',
    anim: 'D3-3 ② · a 이어쓰기' },

  { title: 'f.write("x"); f.write("y") 실행 후 파일 내용은?',
    options: ['x 와 y 두 줄', 'xy 한 줄', 'x 만', '오류'],
    answer: 1,
    why: 'write 는 개행을 자동으로 붙이지 않습니다 — 줄을 나누려면 \\n 을 직접.',
    anim: '3교시 · 개행' },

  { title: '"hi\\n" 을 저장한 파일에서 line = f.readline() 후 print(len(line)) 은?',
    options: ['2', '4', '오류', '3'],
    answer: 3,
    why: 'readline 결과는 hi 뒤에 개행까지 포함 — 3글자. 그래서 strip 습관이 필요합니다.',
    anim: 'D3-3 ③ · 줄 끝 \\n' },

  { title: 's = json.dumps({"a": 1}) 일 때 type(s).__name__ 은?',
    options: ['str', 'dict', 'bytes', 'json'],
    answer: 0,
    why: 'dumps 의 s = string — 결과는 JSON 규격의 문자열 한 덩어리입니다.',
    anim: 'D3-4 ① · dumps' },

  { title: 'json.loads(\'{"t": 25.5}\')["t"] + 0.5 의 값은?',
    options: ['"25.50.5"', 'TypeError', '25.5', '26.0'],
    answer: 3,
    why: 'JSON 복원은 타입이 돌아옵니다 — 25.5 는 float 이므로 바로 26.0.',
    anim: 'D3-4 ③ · 타입 복원' },

  { title: '클래스 인스턴스 r 에 대해 json.dumps(r) 의 결과는?',
    options: ['그대로 저장된다', 'None', 'AttributeError', 'TypeError'],
    answer: 3,
    why: 'JSON 이 아는 타입은 dict/list/str/숫자/bool/None 뿐 — 해법은 to_dict().',
    anim: 'D3-4 ⑤ · 인스턴스 불가' },

  { title: 'json.dump({"a": 1}) 처럼 파일 인자를 빼고 부르면?',
    options: ['TypeError (필수 인자 누락)', 'JSON 문자열 반환', '파일 자동 생성', '조용히 무시'],
    answer: 0,
    why: 'dump(obj, f) 는 파일 객체가 필수 — 문자열이 필요하면 dumps 입니다.',
    anim: 'D3-4 ⑥ · dump vs dumps' },

  { title: 'os.makedirs("data", exist_ok=True) 를 두 번 연속 실행하면?',
    options: ['FileExistsError', '아무 오류 없이 정상', '폴더가 삭제된다', '권한 오류'],
    answer: 1,
    why: 'exist_ok=True 는 이미 있어도 조용히 넘어갑니다 — 폴더 보장 패턴.',
    anim: '5교시 · makedirs' },

  { title: 'line = "$T=25." 일 때 line.startswith("$") and line.endswith("*") 의 값은?',
    options: ['False', 'True', '오류', 'None'],
    answer: 0,
    why: '끝이 "*" 가 아니므로 False — 조각 프레임이 검증에서 걸러지는 원리입니다.',
    anim: 'D3-5 ③ · 경계 어긋난 프레임' },

  { title: 'P.make({"x": 7}) (make 는 @classmethod, cls(d["x"]) 반환) 후 p.x 는?',
    options: ['오류', '{"x": 7}', '7', 'None'],
    answer: 2,
    why: 'classmethod 는 설계도(cls)를 받아 cls(...) 로 새 인스턴스를 찍습니다 — from_dict 패턴.',
    anim: '6교시 · classmethod' },

  { title: 'none.json 이 없을 때 try: json.load(open(...)) except FileNotFoundError: data = [] 의 print(data) 는?',
    options: ['FileNotFoundError 로 종료', 'None', '무한 대기', '[]'],
    answer: 3,
    why: '첫 실행에 파일이 없는 것은 정상 시나리오 — 잡아서 빈 리스트로 시작합니다.',
    anim: 'D3-3 ⑤ · load 방어' },

];
