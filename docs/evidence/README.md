# G1 Loco RPC 직렬화 확인 자료

[분석 본문](../g1-fsm-dispatch-and-handoff.ko.md)

이 검사는 원본 SDK의 G1 inline 메서드와 JSON 직렬화 구현을 실행하되,
네트워크 전송 계층 `unitree::robot::Client`만 요청을 기록하는 테스트 대역으로 바꾼다.
로봇의 FsmId나 모터 반응을 모의하지 않는다. 테스트 대역의 반환값 0도 실제 RPC 성공이 아니다.
실제 Client 생성자의 `Noop()` 등 연결 초기화 통신도 이 테스트 대역에서는 수행하지 않는다.
검사 대상은 아래 여섯 메서드의 요청 생성이며, 실제 프로그램 전체의 네트워크 패킷 수가 아니다.

- [실행 소스](g1-loco-rpc-probe.cpp)
- [전송 계층 테스트 대역](g1-loco-rpc-probe-client.hpp)
- [기록된 요청](g1-loco-rpc-trace.json)

## 재현

저장소 루트에서 실행한다. `SDK_ROOT`는 분석한 원본 C++ SDK 체크아웃이다.
예시 명령은 x86_64 archive와 Linux C++17 컴파일러를 사용한다.
임시 디렉터리는 확인용 헤더와 실행 파일을 담는다. SDK와 앱의 실행 소스를 수정하지 않는다.

```bash
SDK_ROOT=/hdd/Project/_unitree/unitree_sdk2
PROBE_DIR=$(mktemp -d /tmp/unitree-loco-probe.XXXXXX)
mkdir -p "$PROBE_DIR/stub/unitree/robot/client"
cp docs/evidence/g1-loco-rpc-probe-client.hpp \
  "$PROBE_DIR/stub/unitree/robot/client/client.hpp"
g++ -std=c++17 \
  -I"$PROBE_DIR/stub" -I"$SDK_ROOT/include" \
  docs/evidence/g1-loco-rpc-probe.cpp \
  "$SDK_ROOT/lib/x86_64/libunitree_sdk2.a" -pthread \
  -o "$PROBE_DIR/probe"
"$PROBE_DIR/probe"
```

출력은 순서대로 다음 API 번호와 JSON parameter를 포함한다.

```text
sport  7100  {"fsm_id": 550, "api_id": 1}
sport  7101  {"data":550507}
sport  7110  {"data":false}
sport  7111  {"data":0}
sport  7111  {"data":1}
sport  7111  {"data":2}
```

실제 구분자는 tab이다. JSON 객체의 공백은 의미 비교 대상이 아니다.
6개의 호출에서 6개의 요청을 기록하는지 확인한다.
서버가 요청을 받은 후 수행하는 상태 전이는 이 검사 범위에 포함되지 않는다.
