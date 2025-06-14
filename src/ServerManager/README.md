# ServerManager

## 개요

ServerManager는 서버 소켓 초기화, 메인 이벤트 루프 관리, 웹 서버의 다양한 컴포넌트 간 상호작용 조정을 담당하는 핵심 컴포넌트입니다.

## 책임

- 설정에 기반한 리스닝 소켓 설정
- 메인 이벤트 루프 관리 (Reactor 패턴)
- Demultiplexer의 이벤트 처리
- EventHandler, ClientManager, TimeoutHandler 간 조정
- 클라이언트 연결 및 연결 해제 처리

## 주요 클래스

```cpp
class ServerManager {
public:
    ~ServerManager();
    void setupListeningSockets();
    void run();

private:
    std::set<int> listenFds_;
    // 소켓 설정 메서드
    int createListeningSocket(const ServerConfig &server) const;
    // 이벤트 처리 메서드
    void processServerReadEvent(int fd, ClientManager& clientManager,
                               EventHandler& eventHandler, TimeoutHandler& timeoutHandler,
                               Demultiplexer& reactor);
    void processClientReadEvent(int clientFd, ClientManager& clientManager,
                               EventHandler& eventHandler, TimeoutHandler& timeoutHandler,
                               Demultiplexer& reactor);
    // ...
};

```

## 수신 소켓 초기화

setupListeningSockets 메서드는 서버 시작 시 클라이언트 연결을 수신할 소켓들을 초기화합니다.

- **핵심 기능**:
  - 설정 파일에 정의된 가상 호스트별로 수신 소켓 생성 및 구성
  - 중복 (호스트, 포트) 조합에 대해 소켓 재사용
  - 생성된 소켓을 논블로킹 모드로 설정
  - 소켓과 서버 설정 간 매핑 구성

- **동작 흐름**:
  1. GlobalConfig에서 서버 설정 목록 로드
  2. 각 서버 설정마다 소켓 생성 또는 재사용
  3. 소켓을 listenFds_ 집합에 추가
  4. 각 소켓과 관련 서버 설정을 매핑

- **설계 특징**:
  - **자원 최적화**: 동일 주소/포트에 대한 소켓 재사용
  - **비동기 처리**: 논블로킹 소켓으로 이벤트 기반 아키텍처 지원
  - **가상 호스트**: 하나의 소켓이 여러 서버 설정 처리 가능

## 이벤트 루프 개요

`ServerManager::run()` 내부에서 이벤트 루프가 실행되며, 전체 흐름은 다음과 같습니다:

1. **핵심 컴포넌트 초기화**
    - `Demultiplexer`: `kqueue` 래퍼
    - `EventHandler`: 이벤트 처리기
    - `ClientManager`: 클라이언트 세션 추적
    - `TimeoutHandler`: 연결 시간 초과 제어
2. **이벤트 루프 시작**
    - `kqueue`로부터 이벤트 감지
    - 감지된 이벤트 수만큼 반복문으로 순회
    - 각 이벤트의 종류 및 소켓 종류에 따라 적절한 처리 함수로 위임


## 이벤트 분기 처리

| 이벤트 유형 | 발생 위치 | 처리 함수 |
| --- | --- | --- |
| READ | 리스닝 소켓 | `processServerReadEvent()` |
| READ | 클라이언트 소켓 | `processClientReadEvent()` |
| READ | CGI 파이프 | `processCgiReadEvent()` |
| WRITE | 클라이언트 소켓 | `processClientWriteEvent()` |

각 함수는 관련된 매니저들을 받아 현재 상황에 맞는 요청/응답 처리를 수행하고, 클라이언트 상태를 갱신합니다.

---

## 관련 컴포넌트

- `Demultiplexer`: `kqueue`를 감싸 이벤트 감지를 담당
- `ClientManager`: 클라이언트 소켓 및 상태 추적
- `TimeoutHandler`: 비활성 클라이언트 타임아웃 제어
- `EventHandler`: 요청 파싱, 응답 생성 등 핵심 처리 로직 포함

---

## 흐름 요약

```
main.cpp
 └── ServerManager::setupListeningSockets()
 └── ServerManager::run()
      ├── kqueue로 이벤트 대기
      ├── 이벤트 반복 순회
      └── 이벤트 타입 및 소켓 종류에 따라 분기 처리
```

---

## 참고 사항

- 루프는 `isServerRunning()`이 `true`일 때 계속 실행됩니다.
- `TimeoutHandler`는 가장 빠른 타임아웃 시점을 `timespec*` 형태로 제공하여, `kqueue`의 타임아웃 기능을 활용합니다.
- 서버가 종료되거나 예외가 발생할 경우, `GlobalConfig::destroyInstance()`를 통해 리소스를 정리합니다.

[메인 README로 돌아가기](../../README.md)