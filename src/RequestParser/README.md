# RequestParser

## 개요

`RequestParser`는 HTTP/1.1 요청 메시지를 분석하여 서버가 이를 정확하게 해석하고 처리할 수 있도록 돕는 핵심 컴포넌트입니다. 이 모듈은 비동기 이벤트 루프 기반 환경에서 동작하며, 클라이언트로부터 수신한 요청 데이터를 순차적으로 파싱해 `RequestMessage` 객체를 구성합니다.

---

## 주요 역할

- 요청 라인(메서드, URI, 버전), 헤더, 본문 파싱
- HTTP/1.1 프로토콜에 기반한 요청 형식 및 의미 검증
- URI 및 본문 길이에 대한 서버 구성 기반 제한 적용
- `Transfer-Encoding: chunked` 요청 처리
- 오류가 있는 요청에 대해 적절한 HTTP 상태 코드 반환

---

## Request Message 파싱

### 파싱 구조

`RequestParser`는 상태 기반 파서(Finite State Machine)로 작동하며, 다음의 단계로 요청을 해석합니다:

1. **요청 수신 및 라인 분리**: 입력 데이터를 줄 단위로 처리
2. **상태에 따른 파싱 분기**: 요청 라인 → 헤더 → 본문 순으로 파싱 흐름 제어
3. **본문 처리**: `Content-Length` 또는 `Transfer-Encoding` 헤더에 따라 본문 수신 및 저장

### 오류 대응

HTTP 요청이 사양에 어긋나거나 서버 설정을 초과할 경우, 다음과 같은 오류를 반환합니다:

- `400 Bad Request`: 구문 오류, 필수 필드 누락, 잘못된 헤더 형식 등
- `413 Payload Too Large`: 요청 본문이 서버 설정을 초과함
- `414 URI Too Long`: 요청 URI가 서버가 허용하는 최대 길이 초과
- `501 Not Implemented`: 구현되지 않은 HTTP 메서드 사용

### 성능 최적화

- `std::string::substr` 사용을 줄이고, 인덱스 기반 탐색으로 복사 비용 최소화
- 문자열 탐색 시 불필요한 임시 객체 생성을 피하는 방식 적용
- FSM 기반 파싱을 통해 구조적이고 명확한 상태 전이 관리

---

## [Chunked Transfer Coding](https://datatracker.ietf.org/doc/html/rfc9112#name-chunked-transfer-coding) 지원

`Transfer-Encoding: chunked` 방식은 본문 크기를 사전에 알 수 없을 때 사용되며, 서버는 이를 다음과 같은 절차로 해석합니다:

- 각 청크는 크기(16진수) + CRLF + 데이터 + CRLF 구조로 구성
- 크기 0의 종료 청크를 확인한 후 파싱 종료
- 파싱된 모든 청크를 합쳐 본문 데이터 완성

---

# 참고문서

- https://datatracker.ietf.org/doc/html/rfc9110#name-delete
- https://datatracker.ietf.org/doc/html/rfc9112

---

# 담당

- [taerakim](https://github.com/taerakim)