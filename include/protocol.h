#ifndef PROTOCOL_H
#define PROTOCOL_H

#include "common.h" // User, Child, Meal, Response 구조체 포함
#include <stdbool.h>

// 명령어 구분자
#define CMD_DELIMITER "//"

// 클라이언트 명령어
#define CMD_REGISTER_GENERAL "REGISTER_GENERAL"
#define CMD_REGISTER_PARENT "REGISTER_PARENT"
#define CMD_LOGIN "LOGIN"
#define CMD_UPDATE "UPDATE"
#define CMD_DELETE "DELETE"
#define CMD_GET_CHILDREN "GET_CHILDREN"
#define CMD_ADD_CHILD "ADD_CHILD"
#define CMD_DEL_CHILD "DEL_CHILD"
#define CMD_CHILD_MEAL "CHILD_MEAL"
#define CMD_CHILD_MEALS_PERIOD "CHILD_MEALS_PERIOD"
#define CMD_MEAL "MEAL"
#define CMD_MULTI_MEAL "MULTI_MEAL"
#define CMD_OTHER_MEAL "OTHER_MEAL"
#define CMD_MULTI_OTHER_MEAL "MULTI_OTHER_MEAL"
#define CMD_ADD_USER "ADD_USER"
#define CMD_EDIT_USER "EDIT_USER"
#define CMD_DELETE_USER "DELETE_USER"
#define CMD_LOGOUT "LOGOUT"
#define CMD_QUIT "QUIT"
#define CMD_MEAL_QUERY "MEAL_QUERY"
#define CMD_MEAL_PERIOD_QUERY "MEAL_PERIOD_QUERY"
#define CMD_DELETE_CHILD "DELETE_CHILD"
#define CMD_UPDATE_USER "UPDATE_USER"

// 응답 코드
#define RESP_SUCCESS "SUCCESS"
#define RESP_ERROR "ERROR"

// 응답 메시지
#define RESP_REGISTER_OK "REGISTER_OK"
#define RESP_LOGIN_OK "LOGIN_OK"
#define RESP_UPDATE_OK "UPDATE_OK"
#define RESP_DELETE_OK "DELETE_OK"
#define RESP_GET_CHILDREN_OK "GET_CHILDREN_OK"
#define RESP_ADD_CHILD_OK "ADD_CHILD_OK"
#define RESP_DEL_CHILD_OK "DEL_CHILD_OK"
#define RESP_MEAL_OK "MEAL_OK"
#define RESP_UNKNOWN_COMMAND "UNKNOWN_COMMAND"
#define RESP_INVALID_REQUEST "INVALID_REQUEST"
#define RESP_DB_ERROR "DB_ERROR"

// 에러 코드
#define ERR_ID_DUPLICATE "ID_DUPLICATE"
#define ERR_INVALID_LOGIN "INVALID_LOGIN"
#define ERR_MEAL_NOT_FOUND "MEAL_NOT_FOUND"
#define ERR_TOO_LONG_PERIOD "TOO_LONG_PERIOD"
#define ERR_EDIT_FAIL "EDIT_FAIL"
#define ERR_DELETE_FAIL "DELETE_FAIL"
#define ERR_NOT_LOGGED_IN "NOT_LOGGED_IN"

// 사용자 역할
#define ROLE_GENERAL "GENERAL"
#define ROLE_PARENT "PARENT"
#define ROLE_ADMIN "admin"

// 응답 메시지 포맷
#define RESP_LOGIN_FORMAT "%s//LOGIN//%s//%s//%s"
#define RESP_MEAL_FORMAT "%s//MEAL//%s"
#define RESP_MULTI_MEAL_FORMAT "%s//MULTI_MEAL//%s"
#define RESP_OTHER_MEAL_FORMAT "%s//OTHER_MEAL//%s"
#define RESP_MULTI_OTHER_MEAL_FORMAT "%s//MULTI_OTHER_MEAL//%s"

// 기간 제한
#define MAX_PERIOD_DAYS 7

// 함수 선언
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