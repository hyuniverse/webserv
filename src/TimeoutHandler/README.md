# TimeoutHandler

## 개요

TimeoutHandler는 유휴 또는 정체된 연결로 인한 리소스 소모를 방지하기 위해 클라이언트 연결 타임아웃을 관리하는 역할을 담당합니다. 클라이언트 활동 타임스탬프를 추적하고 타임아웃 임계값을 초과하는 연결에 대한 정리를 트리거합니다.

## 책임

- 클라이언트 연결 타임스탬프 추적
- 연결 상태에 따른 적절한 타임아웃 임계값 설정
- 타임아웃된 연결 감지
- 만료된 연결의 정리 시작
- 이벤트 루프에 타임아웃 정보 제공

## 주요 클래스

```cpp
class TimeoutHandler {
public:
    // 내부 데이터 구조를 위한 타입 정의
    typedef std::map<int, time_t>::iterator         TypeConnectionIter;
    typedef std::multimap<time_t, int>::iterator    TypeExpireQueueIter;
    typedef std::map<int, TypeExpireQueueIter>      TypeExpireIterMap;
    typedef TypeExpireIterMap::iterator             TypeExpireIterMapIter;

    TimeoutHandler();
    ~TimeoutHandler();

    // 이벤트 루프를 위한 가장 빠른 타임아웃 가져오기
    timespec* getEarliestTimeout();

    // 클라이언트 연결 관리
    void addConnection(int fd);
    void updateActivity(int fd, EnumSesStatus status);
    void removeConnection(int fd);

    // 만료된 연결 확인 및 처리
    void checkTimeouts(EventHandler& eventHandler, Demultiplexer& reactor, ClientManager& clientManager);

private:
    timespec timeout_;                   // 이벤트 루프를 위한 타임아웃 값
    std::map<int, time_t> connections_;  // 클라이언트 FD를 마지막 활동 시간에 매핑
    std::multimap<time_t, int> expireQueue_;  // 클라이언트 FD로 정렬된 만료 시간
    TypeExpireIterMap expireMap_;        // 클라이언트 FD를 expireQueue의 항목에 매핑

    // 헬퍼 메서드
    void removeConnection(int fd, TypeExpireQueueIter it);
    time_t getTime() const;
};

```

## 타임아웃 임계값

TimeoutHandler는 두 가지 다른 타임아웃 임계값을 사용합니다:

1. **요청 타임아웃(REQ_LIMIT)**: 15초
    - 요청이 활발히 처리 중일 때 적용(READ_CONTINUE, WAIT_FOR_CGI)
    - 요청이 너무 오래 걸릴 경우 적절한 오류 응답 생성 및 WRITE_EVENT 등록
2. **유휴 타임아웃(IDLE_LIMIT)**: 30초
    - 요청 간 연결에 적용(READ_COMPLETE, WRITE_COMPLETE)
    - 버려진 연결에서 리소스 해제

## 연결 라이프사이클

1. 클라이언트가 연결되면 `addConnection()`이 현재 시간 기록 및 추적 정보 추가
2. 클라이언트가 상호 작용하면 `updateActivity()`가 타임스탬프를 업데이트하고 적절한 타임아웃 임계값 설정
3. `checkTimeouts()`가 주기적으로 만료된 연결 확인
4. 만료된 연결의 경우:
    - 요청 중인 경우: 408 Request Timeout 전송
    - CGI 처리가 지연되는 경우: 503 Internal Server Error 전송
    - 유휴 상태인 경우: 연결 종료 및 리소스 정리
5. 클라이언트가 연결을 해제하면 `removeConnection()`이 추적 정보 정리

## 타임아웃 데이터 구조

TimeoutHandler는 효율적인 타임아웃 관리를 위해 여러 데이터 구조를 사용합니다:

- `connections_`: 클라이언트 FD를 마지막 활동 타임스탬프에 매핑
- `expireQueue_`: 빠른 만료 연결 감지를 위해 만료 시간별로 연결 정렬
- `expireMap_`: 클라이언트 FD를 expireQueue의 항목에 매핑하여 효율적인 업데이트 지원

## 이벤트 루프와의 통합

1. ServerManager는 `getEarliestTimeout()`을 통해 가장 빠른 타임아웃 요청
2. 이 타임아웃은 이벤트 멀티플렉서의 대기 함수에서 사용
3. 이벤트 처리 후 ServerManager는 `checkTimeouts()` 호출
4. 만료된 연결이 적절하게 처리됨

[메인 README로 돌아가기](../../README.md)
