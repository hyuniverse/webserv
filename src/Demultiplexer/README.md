# Demultiplexer

## 개요

Demultiplexer는 kqueue(macOS)를 사용하여 여러 파일 디스크립터의 I/O 이벤트를 모니터링하는 역할을 담당합니다. Reactor 패턴의 이벤트 디멀티플렉싱 부분을 구현합니다.

(Epoll(linux) 확장 예정)

## 책임

- 읽기/쓰기 이벤트에 대한 파일 디스크립터 모니터링
- 모니터링 세트에 파일 디스크립터 추가 및 제거
- ServerManager에 준비된 이벤트 보고
- 논블로킹 I/O 작업 지원

## 주요 클래스

```cpp
// CRTP(Curiously Recurring Template Pattern)를 사용한 템플릿 기본 클래스
template <typename Derived>
class DemultiplexerBase {
public:
    int waitForEvent(timespec* timeout);
    void removeFd(int fd);
    void addReadEvent(int fd);
    void removeReadEvent(int fd);
    void addWriteEvent(int fd);
    void removeWriteEvent(int fd);
    int getSocketFd(int idx);
    EnumEvent getEventType(int idx);
    // ...
};

// kqueue를 위한 구체적인 구현
class KqueueDemultiplexer : public DemultiplexerBase<KqueueDemultiplexer> {
public:
    KqueueDemultiplexer(std::set<int>& listenFds);
    ~KqueueDemultiplexer();

    // 구현 메서드
    int waitForEventImpl(timespec* timeout);
    void addReadEventImpl(int fd);
    void removeReadEventImpl(int fd);
    void addWriteEventImpl(int fd);
    void removeWriteEventImpl(int fd);
    // ...

private:
    int kq_;
    int numEvents_;
    std::vector<struct kevent> eventList_;
    std::vector<struct kevent> changedEvents_;
};

// 쉬운 사용을 위한 타입 별칭
typedef KqueueDemultiplexer Demultiplexer;

```

## 이벤트 처리

1. `waitForEvent()`는 이벤트가 발생하거나 타임아웃에 도달할 때까지 블록
2. 감지된 각 이벤트에 대해:
    - `getEventType()`은 읽기 또는 쓰기 이벤트인지 결정
    - `getSocketFd()`는 이벤트를 트리거한 파일 디스크립터를 반환
3. ServerManager는 이 정보를 사용하여 적절한 핸들러 호출

## kqueue 구현

KqueueDemultiplexer는 파일 디스크립터를 모니터링하기 위해 macOS의 kqueue 시스템 호출을 사용합니다. 다음을 유지합니다:

- kqueue 디스크립터(`kq_`)
- 모니터링된 이벤트 목록(`eventList_`)
- 변경될 이벤트 목록(`changedEvents_`)