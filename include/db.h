#ifndef DB_H
#define DB_H

#include "common.h"

// DB 초기화
bool init_db(void);
void close_db(void);

// 사용자 관련 함수
bool is_user_exists(const char *id);
bool add_user(const User *user);
bool get_user(const char *id, User *user);
bool verify_user(const char *id, const char *pw);

// 급식 정보 관련 함수
bool save_meal(const Meal *meal);
bool get_meal(const char *date, const char *edu_office, const char *school_name, Meal *meal);

#endif // DB_H
