#ifndef HANDLERS_H
#define HANDLERS_H

#include "common.h"
#include "protocol.h"

// 로그인 관련 핸들러
bool handle_login(const char *id, const char *pw, char *response);
bool handle_register(const char *id, const char *pw, const char *edu_office, const char *school_name, char *response);
void handle_logout(void);

// 급식 조회 핸들러
bool get_meal_from_neis(const char *edu_office, const char *school_name, const char *date, char *response);
bool get_meals_period_from_neis(const char *edu_office, const char *school_name, const char *start_date, const char *end_date, char *response);
bool get_child_meal(const char *parent_id, const char *child_id, const char *date, char *response);
bool get_child_meals_period(const char *parent_id, const char *child_id, const char *start_date, const char *end_date, char *response);

// 자녀 관리 핸들러
bool add_child(const char *parent_id, const char *child_id, char *response);
bool delete_child(const char *parent_id, const char *child_id, char *response);
bool get_children(const char *parent_id, char *response);

// 사용자 관리 핸들러
bool handle_add_user(const char *id, const char *pw,
                     const char *edu_office, const char *school_name,
                     int *status, char *message);
bool handle_update_user(const char *id, const char *pw,
                        const char *edu_office, const char *school_name,
                        int *status, char *message);
bool handle_delete_user(const char *id, char *response);

// 교육청 입력 가이드 출력 함수 선언
void print_edu_office_guide(void);

// 기존 함수들의 별칭
#define register_user handle_register
#define update_user_by_field handle_update_user
#define delete_user handle_delete_user

#endif // HANDLERS_H