#ifndef DB_HANDLER_H
#define DB_HANDLER_H

#include "common.h" // MAX 상수들 포함되어 있다고 가정
#include <stdbool.h>
#include "protocol.h" // Meal, User, Child 구조체 정의 포함되어 있다고 가정

// DB 초기화 및 종료
bool init_db(void);
void close_db(void);

// 사용자 관련
bool is_user_exists(const char *id);
bool add_user(const User *user);
bool get_user(const char *id, User *user);
bool update_user(const User *user); // 사용자 정보 수정
bool delete_user_by_id(const char *id);

// 급식 관련
bool save_meal(const Meal *meal);
bool get_meal(const char *date, const char *edu_office, const char *school_name, Meal *meal);

// 자녀 관련
bool get_children_db(const char *parent_id, Child *children, int *count);
bool db_add_child(const char *child_id, const char *parent_id);
bool db_delete_child(const char *child_id, const char *parent_id);
bool is_child_registered(const char *child_id, const char *parent_id);

#endif
