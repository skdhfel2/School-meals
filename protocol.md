# 프로토콜 명세서

## 명령어 구분자
- 모든 명령어와 파라미터는 `//`로 구분됩니다.

## 클라이언트 -> 서버 명령어

### 1. 일반 회원가입
```
REGISTER_GENERAL//ID//PW//교육청//학교명
```
- 응답:
  - 성공: `SUCCESS//REGISTER_GENERAL`
  - 실패: `ERROR//ID_DUPLICATE`

### 2. 부모 회원가입
```
REGISTER_PARENT//ID//PW
```
- 응답:
  - 성공: `SUCCESS//REGISTER_PARENT`
  - 실패: `ERROR//ID_DUPLICATE`

### 3. 로그인
```
LOGIN//ID//PW
```
- 응답:
  - 성공: `SUCCESS//LOGIN//교육청//학교명//ROLE`
  - 실패: `ERROR//INVALID_LOGIN`

### 4. 내 학교 급식 조회 (단일)
```
MEAL//교육청//학교명//YYYYMMDD
```
- 응답:
  - 성공: `SUCCESS//MEAL//{JSON}`
  - 실패: `ERROR//MEAL_NOT_FOUND`

### 5. 내 학교 급식 조회 (기간)
```
MULTI_MEAL//교육청//학교명//YYYYMMDD-YYYYMMDD
```
- 응답:
  - 성공: `SUCCESS//MULTI_MEAL//{JSON}`
  - 실패: `ERROR//TOO_LONG_PERIOD` 또는 `ERROR//MEAL_NOT_FOUND`

### 6. 다른 학교 급식 조회 (단일)
```
OTHER_MEAL//교육청//학교명//YYYYMMDD
```
- 응답:
  - 성공: `SUCCESS//OTHER_MEAL//{JSON}`
  - 실패: `ERROR//MEAL_NOT_FOUND`

### 7. 다른 학교 급식 조회 (기간)
```
MULTI_OTHER_MEAL//교육청//학교명//YYYYMMDD-YYYYMMDD
```
- 응답:
  - 성공: `SUCCESS//MULTI_OTHER_MEAL//{JSON}`
  - 실패: `ERROR//TOO_LONG_PERIOD` 또는 `ERROR//MEAL_NOT_FOUND`

### 8. 종료
```
QUIT
```
- 응답:
  - 성공: `SUCCESS//QUIT`

## 데이터 형식

### 사용자 정보
- ID: 최대 32자
- 비밀번호: 최대 64자 
- 교육청: 최대 32자
- 학교명: 최대 64자
- 역할: "general" 또는 "parent"

### 급식 정보
- 날짜: YYYYMMDD 형식
- 기간: YYYYMMDD-YYYYMMDD 형식 (최대 7일)
- 급식 메뉴: JSON 형식 (NEIS API 응답)

## 에러 코드
- `ID_DUPLICATE`: 이미 존재하는 ID
- `INVALID_LOGIN`: 잘못된 로그인 정보
- `MEAL_NOT_FOUND`: 급식 정보 없음
- `TOO_LONG_PERIOD`: 조회 기간이 7일을 초과
- `DB_ERROR`: 데이터베이스 오류
