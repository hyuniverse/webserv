# WebServ - C++98 HTTP 서버 구현

![](https://img.shields.io/badge/C++-98-blue.svg)

[](https://img.shields.io/badge/platform-macOS-lightgrey)

## 개요

WebServ는 C++98로 구현된 경량 HTTP 서버로 HTTP/1.1 프로토콜을 지원합니다. 이 프로젝트는 현대적인 웹 서버 구현을 통해 핵심 네트워킹 개념, 이벤트 기반 프로그래밍 및 논블로킹 I/O를 보여줍니다.

## 특징

- HTTP/1.1 프로토콜 지원
- 이벤트 기반 아키텍처를 통한 논블로킹 I/O
- 설정 가능한 가상 호스트 및 서버 설정
- 정적 파일 제공
- CGI 스크립트 처리 지원
- 사용자 정의 오류 페이지
- 요청 타임아웃 처리
- 다중 클라이언트 연결 지원

## 아키텍처

서버는 Reactor 패턴을 사용한 이벤트 기반 아키텍처로 구축되었습니다. macOS에서는 이벤트 멀티플렉싱을 위해 kqueue를 활용합니다.

```mermaid
flowchart TB
    subgraph Main["초기화 및 설정"]
        GlobalConfig["GlobalConfig (싱글톤)"]
        ConfigParser["ConfigParser"]
    end
    
    subgraph ServerCore["서버 코어"]
        ServerManager["ServerManager"]
        Demultiplexer["Demultiplexer"]
        TimeoutHandler["TimeoutHandler"]
        EventHandler["EventHandler"]
        ClientManager["ClientManager"]
    end
    
    subgraph RequestProcessing["요청 처리"]
        RequestParser["RequestParser"]
        StaticHandler["StaticHandler"]
        CgiHandler["CgiHandler"]
        ResponseBuilder["ResponseBuilder"]
    end
    
    subgraph DataStructures["데이터 구조"]
        ClientSession["ClientSession"]
        RequestMessage["RequestMessage"]
        ServerConfig["ServerConfig"]
        RequestConfig["RequestConfig"]
    end
    
    %% 주요 초기화 흐름
    ConfigParser -->|설정 파일 파싱| GlobalConfig
    GlobalConfig --> ServerConfig
    GlobalConfig --> RequestConfig
    
    %% 서버 실행 흐름
    Main --> ServerCore
    ServerManager -->|이벤트 감지| Demultiplexer
    ServerManager -->|타임아웃 관리| TimeoutHandler
    ServerManager -->|이벤트 처리| EventHandler
    ServerManager -->|클라이언트 관리| ClientManager
    
    %% 이벤트 처리 흐름
    EventHandler -->|요청 파싱| RequestParser
    EventHandler -->|정적 파일 처리| StaticHandler
    EventHandler -->|CGI 처리| CgiHandler
    
    %% Handler와 ResponseBuilder 관계
    StaticHandler -->|응답 생성 요청| ResponseBuilder
    CgiHandler -->|응답 생성 요청| ResponseBuilder
    EventHandler -.->|에러 응답 생성| ResponseBuilder
    
    %% 데이터 구조 관계
    ClientManager -->|관리| ClientSession
    ClientSession -->|포함| RequestMessage
    RequestParser -->|생성 및 채움| RequestMessage
    
    %% 설정 참조 관계
    StaticHandler -.->|참조| RequestConfig
    CgiHandler -.->|참조| RequestConfig
    ResponseBuilder -.->|참조| RequestConfig
    
    %% 비동기 이벤트 통지 관계
    Demultiplexer -.->|이벤트 통지| EventHandler
    TimeoutHandler -.->|타임아웃 통지| EventHandler
    TimeoutHandler -.->|연결 관리| ClientManager
    
    %% 스타일 정의
    classDef coreComponents fill:#f96,stroke:#333,stroke-width:2px;
    classDef dataStructure fill:#bbf,stroke:#333,stroke-width:1px;
    classDef mainComponents fill:#9c6,stroke:#333,stroke-width:2px;
    classDef requestProc fill:#c9a,stroke:#333,stroke-width:2px;
    
    class ServerManager,Demultiplexer,TimeoutHandler,EventHandler,ClientManager coreComponents;
    class ClientSession,RequestMessage,ServerConfig,RequestConfig dataStructure;
    class GlobalConfig,ConfigParser mainComponents;
    class RequestParser,StaticHandler,CgiHandler,ResponseBuilder requestProc;
```

## 디렉토리 구조

```
src/
├── GlobalConfig/        # 서버 설정 저장
├── ConfigParser/        # 설정 파일 파싱
├── ServerManager/       # 서버 라이프사이클 및 메인 루프
├── Demultiplexer/       # I/O 이벤트 멀티플렉싱 처리(kqueue)
├── EventHandler/        # 이벤트 처리
├── ClientManager/       # 클라이언트 세션 관리
├── ClientSession/       # 클라이언트 연결 상태
├── RequestMessage/      # HTTP 요청 메시지
├── RequestParser/       # HTTP 요청 파싱
├── RequestHandler/      # HTTP 요청 처리 (정적 콘텐츠 및 CGI)
├── ResponseBuilder/     # HTTP 응답 구성 및 생성
├── TimeoutHandler/      # 연결 타임아웃 관리
├── include/             # 공통 헤더 파일
└── utils/               # 유틸리티 함수
```

## Event Loop

이벤트 루프는 이벤트를 기다리고, 이벤트 유형에 따라 처리하며, 클라이언트 연결을 관리하는 순차적 패턴을 따릅니다.

```mermaid
flowchart TD
    Start([ServerManager::run 시작]) --> Init[이벤트 핸들러, 클라이언트 매니저,\n디멀티플렉서, 타임아웃 핸들러 초기화]
    Init --> Loop[서버 실행 루프]
    
    Loop --> Timeout[가장 빠른 타임아웃 값 획득]
    Timeout --> Wait[이벤트 대기 - waitForEvent]
    Wait --> ProcessLoop[이벤트 처리 루프]
    
    ProcessLoop --> EventType{이벤트 타입?}
    
    EventType -->|READ_EVENT| SocketType{소켓 타입?}
    EventType -->|WRITE_EVENT| ClientWrite[클라이언트 쓰기 이벤트 처리\nprocessClientWriteEvent]
    
    SocketType -->|리스닝 소켓| ServerRead[새 클라이언트 연결 수락\nprocessServerReadEvent]
    SocketType -->|클라이언트 소켓| ClientRead[클라이언트 데이터 수신 및 처리\nprocessClientReadEvent]
    SocketType -->|CGI 파이프| CgiRead[CGI 데이터 수신 및 처리\nprocessCgiReadEvent]
    
    ServerRead --> ClientSuccess{연결 성공?}
    ClientSuccess -->|Yes| AddClient[클라이언트 등록\naddClientInfo]
    ClientSuccess -->|No| NextEvent
    
    ClientRead --> ClientReadProcess[상태에 따른 처리:
    - 연결 종료: 클라이언트 제거
    - CGI 읽기: CGI 파이프 이벤트 설정
    - 읽기 계속: 계속 감시
    - 쓰기 완료: 활성 시간 갱신
    - 쓰기 계속: 쓰기 이벤트 등록]
    
    CgiRead --> CgiReadProcess[상태에 따른 처리:
    - 연결 종료: 클라이언트 제거
    - 쓰기 완료: 활성 시간 갱신
    - 쓰기 계속: 쓰기 이벤트 등록
    - CGI 읽기 계속: 계속 감시]
    
    ClientWrite --> ClientWriteProcess[상태에 따른 처리:
    - 연결 종료: 클라이언트 제거
    - 쓰기 완료: 쓰기 이벤트 제거
    - 쓰기 계속: 계속 감시]
    
    AddClient --> NextEvent[다음 이벤트 처리]
    ClientReadProcess --> NextEvent
    CgiReadProcess --> NextEvent
    ClientWriteProcess --> NextEvent
    
    NextEvent --> ProcessLoop
    
    ProcessLoop -->|모든 이벤트 처리 완료| CheckTimeouts[타임아웃된 클라이언트 확인 및 처리\ncheckTimeouts]
    CheckTimeouts --> Loop
    
    classDef start fill:#5d8aa8,stroke:#333,stroke-width:2px,color:white;
    classDef process fill:#bbf,stroke:#333,stroke-width:1px;
    classDef decision fill:#c9a,stroke:#333,stroke-width:2px;
    classDef event fill:#f96,stroke:#333,stroke-width:2px;
    classDef processGroup fill:#d4f0c8,stroke:#333,stroke-width:1px;
    
    class Start start;
    class Init,Timeout,Wait,ServerRead,ClientRead,CgiRead,ClientWrite,AddClient,CheckTimeouts process;
    class ClientReadProcess,CgiReadProcess,ClientWriteProcess processGroup;
    class EventType,SocketType,ClientSuccess decision;
    class ProcessLoop,NextEvent,Loop event;
```

## HTTP 요청 처리 흐름

```mermaid
flowchart TD
    Start([클라이언트 요청]) --> DM[Demultiplexer 이벤트 감지]
    DM --> |READ_EVENT| SM[ServerManager 이벤트 타입 확인]
    
    SM --> |서버 소켓| EH1[EventHandler handleServerReadEvent]
    SM --> |클라이언트 소켓| EH2[EventHandler handleClientReadEvent]
    
    EH1 --> |신규 연결| CM1[ClientManager addClient]
    CM1 --> |클라이언트 등록| TH1[TimeoutHandler addConnection]
    TH1 --> End1([연결 수립 완료])
    
    EH2 --> RP[RequestParser parse]
    RP --> |파싱| RM[RequestMessage 생성 및 채우기]
    RM --> |요청 분석| GC[GlobalConfig findRequestConfig]
    GC --> |설정 찾기| RC[RequestConfig 적용]
    
    RC --> |요청 처리 결정| RH{요청 타입?}
    RH --> |정적 파일| SH[StaticHandler handleRequest]
    RH --> |CGI 요청| CH[CgiHandler handleRequest]
    RH --> |리다이렉션| RD[리다이렉션 처리]
    
    SH --> RB1[ResponseBuilder build]
    CH --> RB2[ResponseBuilder AddHeaderForCgi]
    RD --> RB3[ResponseBuilder build]
    
    RB1 --> CS1[ClientSession setWriteBuffer]
    RB2 --> CS1
    RB3 --> CS1
    
    CS1 --> EH3[EventHandler sendResponse]
    EH3 --> |WRITE_EVENT| DM2[Demultiplexer addWriteEvent]
    DM2 --> End2([응답 전송 완료])
    
    classDef startNode fill:#5d8aa8,stroke:#333,stroke-width:2px,color:white
    classDef event fill:#f96,stroke:#333,stroke-width:2px
    classDef process fill:#bbf,stroke:#333,stroke-width:1px
    classDef decision fill:#c9a,stroke:#333,stroke-width:2px
    classDef endNode fill:#6b8e23,stroke:#333,stroke-width:2px,color:white
    
    class Start,End1,End2 startNode
    class DM,DM2,EH1,EH2,EH3 event
    class RP,RM,GC,RC,SH,CH,RD,RB1,RB2,RB3,CS1,CM1,TH1 process
    class RH,SM decision
```

1. **이벤트 감지**: Demultiplexer가 클라이언트 연결에서 읽기 이벤트 감지
2. **요청 파싱**: EventHandler가 RequestParser를 통해 클라이언트 요청 처리
3. **설정 선택**: GlobalConfig가 요청에 적합한 설정 찾기
4. **요청 처리**: StaticHandler 또는 CgiHandler가 요청 처리
5. **응답 구성**: ResponseBuilder가 HTTP 응답 구성
6. **응답 전송**: 서버가 클라이언트에게 응답 전송

## 빌드 및 실행

### 요구 사항

- C++98 지원 C++ 컴파일러
- macOS (kqueue 지원용)

### 컴파일

```bash
# 레포지토리 클론
git clone https://github.com/yourusername/webserv.git
cd webserv

# 프로젝트 빌드
make

# 설정 파일로 실행
./webserv configs/default.conf

```

## 설정

서버는 Nginx와 유사한 설정 파일을 통해 구성됩니다. 예시:

```
server {
    listen 127.0.0.1:8080;
    server_name localhost;

    root /var/www/html;
    index index.html;

    error_page 404 /error/404.html;

    location / {
        methods GET POST;
        autoindex on;
    }

    location /cgi-bin {
        cgi_extension .py;
        methods GET POST;
    }
}

```

## 컴포넌트 문서

각 컴포넌트에 대한 자세한 문서는 해당 README 파일을 참조하세요:

- [ClientManager](src/ClientManager/README.md)
- [ConfigParser](src/ConfigParser/README.md)
- [Demultiplexer](src/Demultiplexer/README.md)
- [EventHandler](src/EventHandler/README.md)
- [GlobalConfig](src/GlobalConfig/README.md)
- [RequestHandler](src/RequestHandler/README.md)
- [RequestParser](src/RequestParser/README.md)
- [ServerManager](src/ServerManager/README.md)
- [TimeoutHandler](src/TimeoutHandler/README.md)

## 개발 팀원

- sehyupar
- seonseo
- damin
- taerakim