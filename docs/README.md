# G1 제어 및 UniStore 앱 분석 문서

이 문서는 Unitree G1의 제어 경로와 `g1_mimic_demo`의 실제 구현을 함께 설명한다.
공식 운용 제어기의 상태와 앱 내부 상태를 구분하고, 각 전환이 어떤 API와 DDS 토픽에 연결되는지 추적한다.

## 문서 구성

1. [공식 제어 모드, FSM, DDS 인터페이스](g1-control-concepts.ko.md)
   - `FsmId`, `FsmMode`, `InternalFsmMode`, `mode_machine`의 구분
   - Damping, 기존 Debug Mode, UserCtrl의 관계
   - `ReleaseMode()`와 `SwitchToUserCtrl()`의 차이
   - `rt/lowcmd`, `rt/user_lowcmd`, `rt/arm_sdk`, Arm Action의 역할과 확인 범위
2. [g1_mimic_demo 실행 구조와 상태 전이](g1-mimic-runtime.ko.md)
   - HTTP 서버, 자식 프로세스, C++ 제어기, 정책 모델의 연결
   - 앱 상태 전이도와 공식 제어기와의 상호작용
   - 초기 진입, 재실행, Damping에서의 실행 거부, 정상 종료 및 오류 경로
   - 현재 소스에서 확인되는 미구현 기능과 제한
3. [빌드, 산출물, 패키징 및 배포](g1-build-and-deploy.ko.md)
   - `deploy_template/`, `out/`, `app/g1/app_out/`의 관계
   - ARM64 Docker 빌드와 수동 동기화
   - 포함된 바이너리와 현재 소스의 차이
   - 메타데이터, 패키징, 실행 환경 및 시뮬레이션의 전제
4. [공식 Mimic 모션 API와 550 접두사 검증](g1-mimic-motion-api.ko.md)
   - `GetMimicMotion()`, `motion_api_id`, `SetFsmId(550507)`의 공식 정의
   - `550501`의 조건부 의미와 앱 ID·공식 모드 ID `501`의 구분
   - 공식 모션 호출과 UserCtrl 기반 외부 정책 실행의 별도 상태 전이도
   - 자동 `550xxx → 1000 → 550xxx` 전환 주장에 대한 확인 범위
5. [모션 선택·제어권 인계·앱 FSM의 연결과 리버싱 검토](g1-fsm-dispatch-and-handoff.ko.md)
   - 같은 sport 서비스에서 서로 다른 요청인 7100·7101·7110·7111의 비교
   - 앱 상태와 공식 FsmId를 조합한 상태 전이도
   - `unitree_sdk2_reversed`의 범용 RPC 서버와 G1 운동 제어기 구현의 구분
   - 개발자 포럼, 실제 모션 클라이언트와 실기체 연구의 교차 검토
   - 원본 SDK 헤더를 컴파일한 [격리 RPC 확인 자료](evidence/README.md)

## 분석 기준

| 항목 | 기준 |
|---|---|
| 최초 확인일 | 2026-11-17, Asia/Seoul |
| Mimic 모션 API 추가 검토일 | 2026-11-18, Asia/Seoul |
| 리버싱 및 제어권 인계 심화 검토일 | 2026-11-18, Asia/Seoul |
| 앱 저장소 기준 커밋 | `1a16513b8a57d76708ffd2854fe2a73c090cced1` |
| 로컬 공식 C++ SDK 기준 커밋 | `63096d0ac0c5d2dec9d6e0c22cd5233410ca2f36` |
| 추가 검토 시 공식 C++ SDK 원격 main | `aaf69cb56c9d5ed63d8b284008ec3a8ab999bec1` |
| 로컬 unitree_sdk2_reversed / lgrobot_sdk | `392e753457adb24d08c2c1efd09cc2112d29a953` |
| 로컬 공식 Python SDK 기준 커밋 | `814556d15970dd2ecf1c9984e845ca02ab07e206` |
| 앱 분석 대상 | `g1_mimic_demo/deploy_template/` 및 `g1_mimic_demo/app/g1/` |
| 실행 확인 범위 | 소스·설정·ELF 검사와 격리된 Python 함수 확인. 로봇 제어 및 ARM64 전체 빌드는 수행하지 않음 |
| 추가 실행 확인 | 실제 G1 inline 메서드·원본 JSON 구현 + 전송 테스트 대역으로 6개 RPC 기록. 로봇 연결 없음 |

커밋은 이 문서 작성 전 소스의 기준이다. Dockerfile은 별도로 SDK의 `main`을 다운로드하므로,
새로 생성한 빌더 이미지가 위의 로컬 SDK 커밋을 사용한다고 가정하면 안 된다.

추가 검토에서는 원격 main의 G1 Loco 클라이언트·API 헤더와 Loco·UserCtrl 예제를
가져와 로컬 SDK와 대조했다. 해당 네 파일의 내용은 로컬 SDK와 동일했다.
`GetMimicMotion()`은 로컬 SDK에도 이미 있었으며, 최초 문서의 모드 표는 전체 인터페이스를
포괄하지 못했다. 이번 보강에서는 공식 모션 호출 경로를 추가하되, 비공개 서버의 내부 구현까지
UserCtrl과 동일하다고 해석하지 않는다. SDK 체크아웃과 실행 코드는 변경하지 않았다.

## 근거를 읽는 방법

- **문서상 계약**: Unitree 공식 문서가 명시한 인터페이스의 의미와 조건이다.
- **소스 확인**: 위 커밋의 코드에서 확인한 분기, 호출, 토픽, 기본값이다.
- **관찰 결과**: 실행 파일의 아키텍처·동적 의존성, 설정 파일의 수치처럼 직접 검사한 결과이다.
- **미확정**: 로봇 측 비공개 구현이나 특정 펌웨어 실측이 필요하여 확정하지 않은 항목이다.

상태 전이도는 각각의 설명 범위에 한정된다. 공식 펌웨어 전체의 전이표를 추측해 그리지 않는다.
API 호출이 성공했다고 최종 상태 전환까지 성공했다고 간주하지 않는다.
개념 예시와 현재 앱의 실제 실행 순서도 구분한다.

## 용어의 적용 범위

| 이 문서의 표현 | 대상 |
|---|---|
| 공식 FSM / 공식 `FsmId` | 공식 운용 모드의 선택·조회. `SetFsmId()`는 모션별 복합 호출값도 수용 |
| 공식 `FsmMode` | 공식 운동 제어기가 보고하는 모드의 정적·동적 상태 |
| 앱 FSM / 앱 상태 ID | `CtrlFSM`, `FSMStringMap`, YAML이 관리하는 프로그램 내부 상태 |
| 복귀 선택값 | `SwitchToInternalCtrl()`의 `InternalFsmMode` 인자 |
| 기체 구성 ID | `LowCmd` / `LowState`의 `mode_machine` |
| 팔 동작 ID | Arm Action 서비스의 동작 번호. 공식 `FsmId`와 별개 |
| Mimic `motion_api_id` | `GetMimicMotion()`의 `motion` 목록에서 모션을 식별하는 키 |
| Mimic 복합 호출 ID | `550` 접두사와 모션 API ID를 이어 만든 `SetFsmId()` 인자. 예: `550507` |

공유 AI 대화와 이전 설명은 조사 항목을 정리하는 참고로만 사용했다.
인터페이스 이름, 지원 조건 및 코드 동작의 근거는 공식 문서와 실제 소스이다.

## 주요 공식 자료

- [G1 High-level motion service interface](https://support.unitree.com/home/en/G1_developer/sport_services_interface)
- [G1 고수준 운동 서비스 인터페이스 중문 원문](https://support.unitree.com/home/zh/G1_developer/sport_services_interface)
- [G1 Motion Switcher Service Interface](https://support.unitree.com/home/en/G1_developer/motion_witcher_service_interface)
- [G1 Remote Control](https://support.unitree.com/home/en/G1_developer/remote_control)
- [G1 Arm Action Service Interface](https://support.unitree.com/home/en/G1_developer/arm_action_interface)
- [UniStore 사용자 프로그램 전환 설명](https://unistore.unitree.com/manuals?slug=switch-sdk2)
- [공식 G1 모델 및 mode_machine 대응표](https://github.com/unitreerobotics/unitree_ros/blob/master/robots/g1_description/README.md)
- [공식 C++ SDK](https://github.com/unitreerobotics/unitree_sdk2)
- [공식 앱 템플릿](https://github.com/unitreerobotics/unitree-app-templates)

공식 웹 문서는 펌웨어와 함께 갱신될 수 있다. API의 이름이 같아도 새로운 기체·펌웨어에서의
전체 지원 상태 목록과 전환 결과를 과거 SDK 주석만으로 확정하지 않는다.
