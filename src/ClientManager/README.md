# ClientManager

## 개요

ClientManager는 서버 내에서 클라이언트 세션과 그 라이프사이클을 관리하는 역할을 담당합니다.

## 책임

- 연결된 클라이언트를 위한 ClientSession 객체 생성 및 저장
- 파일 디스크립터로 클라이언트 세션 접근 및 조회 관리
- 클라이언트 연결 종료 및 정리 처리
- CGI 파이프 파일 디스크립터와 클라이언트 파일 디스크립터 간 매핑

## 주요 클래스

```cpp
class ClientManager {
public:
    typedef std::map<int, ClientSession*> TypeClientMap;

    ClientManager();
    ~ClientManager();

    // 클라이언트 세션 관리
    void addClient(int listenFd, int clientFd, std::string clientAddr);
    TypeClientMap::iterator removeClient(int fd);
    ClientSession* accessClientSession(int fd);
    TypeClientMap& accessClientSessionMap();

    // CGI 파이프 관리
    void addPipeMap(int outPipe, int clientFd);
    void removePipeFromMap(int pipeFd);
    int accessClientFd(int pipeFd);

    // 유효성 검사
    bool isClientSocket(int fd);

private:
    TypeClientMap clientList_;
    std::map<int, int> pipeToClientFdMap_;
};

```

## 클라이언트 세션 라이프사이클

1. 새 클라이언트가 연결되면 `addClient()`가 새 ClientSession 생성
2. 요청 처리 중에는 `accessClientSession()`을 통해 세션에 접근
3. 클라이언트 연결 종료 또는 타임아웃 시 `removeClient()`가 리소스 정리

## CGI 파이프 매핑

CGI 스크립트 실행을 위해:

1. CGI 프로세스와 통신을 위한 파이프 생성
2. `addPipeMap()`은 파이프 FD를 클라이언트 FD와 연결
3. 파이프에 데이터가 있을 때 `accessClientFd()`는 해당 클라이언트 찾기
4. CGI 완료 후 `removePipeFromMap()`은 매핑 제거

[메인 README로 돌아가기](../../README.md)
