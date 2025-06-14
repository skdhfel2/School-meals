#ifndef PROTOCOL_H
#define PROTOCOL_H

#include "common.h"

// 명령어 구분자
#define CMD_DELIMITER "//"

// 응답 코드
#define RESP_SUCCESS 1
#define RESP_ERROR 0

// ✅ 문자열 응답 코드 (클라이언트 비교용)
#define RESP_SUCCESS_STR "SUCCESS"
#define RESP_ERROR_STR "ERROR"

// 응답 메시지
#define RESP_LOGIN_OK "로그인 성공"
#define RESP_REGISTER_OK "회원가입 성공"
#define RESP_UPDATE_OK "사용자 정보 수정 성공"
#define RESP_DELETE_OK "사용자 삭제 성공"
#define RESP_GET_CHILDREN_OK "자녀 목록 조회 성공"
#define RESP_ADD_CHILD_OK "자녀 추가 성공"
#define RESP_DEL_CHILD_OK "자녀 삭제 성공"
#define RESP_GET_CHILD_MEAL_OK "자녀 급식 정보 조회 성공"
#define RESP_GET_CHILD_MULTI_MEAL_OK "자녀 기간 급식 정보 조회 성공"
#define RESP_GET_MEAL_OK "급식 정보 조회 성공"
#define RESP_GET_MULTI_MEAL_OK "기간 급식 정보 조회 성공"

// 에러 메시지

#define RESP_DB_ERROR "데이터베이스 오류"
#define RESP_INVALID_REQUEST "잘못된 요청"
#define RESP_UNKNOWN_COMMAND "알 수 없는 명령어"
#define ERR_INVALID_LOGIN "로그인 실패"
#define ERR_ID_DUPLICATE "중복된 아이디"
#define ERR_CHILD_USER_NOT_FOUND "자녀 사용자를 찾을 수 없습니다"
#define ERR_INVALID_CHILD_ID "잘못된 자녀 ID입니다"
#define ERR_CHILD_MEAL_NOT_FOUND "자녀 급식 정보를 찾을 수 없습니다"
#define ERR_MEAL_NOT_FOUND "급식 정보를 찾을 수 없습니다"
#define ERR_INVALID_DATE_FORMAT "잘못된 날짜 형식입니다"

// 클라이언트 명령어
#define CMD_LOGIN "LOGIN"
#define CMD_REGISTER_GENERAL "REGISTER_GENERAL"
#define CMD_REGISTER_PARENT "REGISTER_PARENT"
#define CMD_UPDATE "UPDATE"
#define CMD_DELETE "DELETE"
#define CMD_LOGOUT "LOGOUT"
#define CMD_QUIT "QUIT"
#define CMD_GET_CHILDREN "GET_CHILDREN"
#define CMD_ADD_CHILD "ADD_CHILD"
#define CMD_DEL_CHILD "DEL_CHILD"
#define CMD_CHILD_MEAL "CHILD_MEAL"
#define CMD_CHILD_MULTI_MEAL "CHILD_MULTI_MEAL"
#define CMD_GET_MEAL "GET_MEAL"
#define CMD_GET_MULTI_MEAL "GET_MULTI_MEAL"

// 사용자 역할
#define ROLE_GENERAL "GENERAL"
#define ROLE_CHILD "CHILD"
#define ROLE_PARENT "PARENT"

// 응답 메시지 포맷
#define RESP_LOGIN_FORMAT "%s//LOGIN//%s//%s//%s"
#define RESP_MEAL_FORMAT "%s//MEAL//%s"
#define RESP_MULTI_MEAL_FORMAT "%s//MULTI_MEAL//%s"

// 기간 제한
#define MAX_PERIOD_DAYS 7

// 오류 메시지
#define ERR_TOO_LONG_PERIOD "조회 기간은 최대 7일까지입니다"

// 다른 학교 급식 정보 요청
#define RESP_OTHER_MEAL_FORMAT "%s//OTHER_MEAL//%s"
#define RESP_MULTI_OTHER_MEAL_FORMAT "%s//MULTI_OTHER_MEAL//%s"

// API 함수 선언
bool login(const char *id, const char *pw, char *response);
bool register_general(const char *id, const char *pw, const char *edu_office, const char *school_name, char *response);
bool register_parent(const char *id, const char *pw, char *response);
bool update_user_by_field(const char *id, const char *pw, const char *edu_office, const char *school_name, char *response);
bool delete_user(const char *id, char *response);
bool get_children(const char *parent_id, char *response);
bool add_child(const char *parent_id, const char *child_id, char *response);
bool delete_child(const char *parent_id, const char *child_id, char *response);
bool get_child_meal(const char *parent_id, const char *child_id, const char *date, char *response);
bool get_child_meals_period(const char *parent_id, const char *child_id, const char *start_date, const char *end_date, char *response);
bool get_meal_from_neis(const char *edu_office, const char *school_name, const char *date, char *response);
bool get_meals_period_from_neis(const char *edu_office, const char *school_name, const char *start_date, const char *end_date, char *response);

#endif // PROTOCOL_H