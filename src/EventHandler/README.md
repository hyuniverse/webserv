# EventHandler

## 개요

EventHandler는 클라이언트 요청, 서버 응답, CGI 처리 등 서버에서 발생하는 다양한 유형의 이벤트를 처리하는 역할을 담당합니다.

## 책임

- 서버 소켓 읽기 이벤트 처리 (새 클라이언트 연결)
- 클라이언트 요청 데이터 처리
- CGI 스크립트 실행 및 결과 관리
- 클라이언트에게 응답 전송
- 오류 처리 및 오류 응답 생성

## 주요 클래스

```cpp
class EventHandler {
public:
    EventHandler();
    ~EventHandler();

    // 서버 이벤트
    int handleServerReadEvent(int fd, ClientManager& clientManager);

    // 클라이언트 이벤트
    EnumSesStatus handleClientReadEvent(ClientSession& clientSession);
    EnumSesStatus handleClientWriteEvent(ClientSession& clientSession);

    // CGI 이벤트
    EnumSesStatus handleCgiReadEvent(ClientSession& clientSession);

    // 오류 처리
    void handleError(EnumStatusCode statusCode, ClientSession& clientSession);

private:
    RequestParser parser_;
    ResponseBuilder responseBuilder_;
    StaticHandler staticHandler_;
    CgiHandler cgiHandler_;

    EnumSesStatus recvRequest(ClientSession& clientSession);
    EnumSesStatus sendResponse(ClientSession& clientSession);
    // ...
};

```

## 요청 처리 흐름

1. `handleClientReadEvent()`가 `recvRequest()`를 통해 클라이언트 데이터 수신
2. RequestParser가 HTTP 요청 파싱
3. 요청에 따라:
    - 정적 콘텐츠: StaticHandler가 요청 처리
    - CGI 스크립트: CgiHandler가 스크립트 실행
    - 리다이렉션: 리다이렉션 응답 구성
4. ResponseBuilder가 HTTP 응답 생성
5. 응답 생성 완료 후 `sendResponse`를 통해 응답 전송 시도
6. 실패 시 `handleClientWriteEvent()`가 `sendResponse()`를 통해 응답 전송

## 오류 처리

오류 상황에 대해 적절한 오류 응답을 생성합니다:

- 400 Bad Request
- 403 Forbidden
- 404 Not Found
- 405 Method Not Allowed
- 408 Request Timeout
- 500 Internal Server Error