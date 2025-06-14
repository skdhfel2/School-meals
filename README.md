# 학교 급식 정보 시스템

이 프로젝트는 NEIS Open API를 활용하여 학교 급식 정보를 조회할 수 있는 클라이언트-서버 시스템입니다.

## 기능

1. 일반 회원가입
   - 아이디, 비밀번호, 교육청, 학교명 입력
   - 비밀번호는 SHA-256으로 해싱하여 저장

2. 부모 회원가입
   - 아이디, 비밀번호만 입력
   - 자녀 연동은 추후 구현 예정

3. 로그인
   - 아이디와 비밀번호로 로그인
   - 사용자 역할(일반/부모)에 따라 다른 메뉴 제공

4. 급식 조회
   - 교육청, 학교명, 날짜를 입력하여 급식 정보 조회
   - NEIS Open API를 통해 실시간 급식 정보 제공

## 빌드 및 실행 방법

### 필수 라이브러리 설치

```bash
# Ubuntu/Debian
sudo apt-get update
sudo apt-get install libsqlite3-dev libjson-c-dev

# CentOS/RHEL
sudo yum install sqlite-devel json-c-devel
```

### 서버 빌드 및 실행

```bash
cd server
make
./server
```

### 클라이언트 빌드 및 실행

```bash
cd client
make
./client
```

## 프로토콜

프로토콜 상세 내용은 `protocol.md` 파일을 참조하세요.

## 주의사항

1. NEIS Open API 키가 필요합니다. `server/neis_api.c` 파일의 `NEIS_API_KEY` 값을 실제 API 키로 교체해야 합니다.
2. 서버는 기본적으로 localhost:8080에서 실행됩니다.
3. 데이터베이스는 SQLite를 사용하며, `school_meals.db` 파일에 저장됩니다.

```
School-meals
├─ client
│  ├─ client.c
│  ├─ client_main.c
│  ├─ input_util.c
│  ├─ Makefile
│  ├─ menu_general.c
│  └─ menu_parent.c
├─ data
│  ├─ children.txt
│  ├─ config.txt
│  ├─ session.txt
│  └─ users.txt
├─ include
│  ├─ common.h
│  ├─ db.h
│  ├─ db_handler.h
│  ├─ protocol.h
│  └─ sqlite3.h
├─ lib
│  ├─ sqlite3.c
│  └─ sqlite3.o
├─ protocol.md
├─ README.md
└─ server
   ├─ db_handler.c
   ├─ db_handler.o
   ├─ Makefile
   ├─ neis_api.c
   ├─ network.c
   ├─ network.o
   ├─ school_meals.db
   ├─ server.c
   ├─ server.exe
   ├─ server.o
   ├─ server_handler.c
   ├─ server_main.c
   └─ sqlite3.dll

```