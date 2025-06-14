#ifndef NEIS_API_H
#define NEIS_API_H

#include <stdbool.h>

#define MAX_PERIOD_DAYS 7 // ✅ 기간 제한만 유지

// 🔽 단일 날짜 급식 조회
bool get_meal_from_neis(const char *edu_office, const char *school_code, const char *date, char *meal);

// 🔽 날짜 범위 급식 조회
bool get_meals_period_from_neis(const char *edu_office, const char *school_code,
                                const char *start_date, const char *end_date,
                                char *meals_json);

#endif
