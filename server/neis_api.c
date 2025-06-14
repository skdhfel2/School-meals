#include "common.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>   // CURL 라이브러리
#include <json-c/json.h> // JSON 파싱을 위한 json-c 라이브러리
#include <time.h>

#define NEIS_API_KEY "53ea0d0873e048e188a0b13834667795" // NEIS API 키
#define NEIS_API_URL "https://open.neis.go.kr/hub/mealServiceDietInfo"

struct MemoryStruct
{
    char *memory;
    size_t size;
};

static size_t WriteMemoryCallback(void *contents, size_t size, size_t nmemb, void *userp)
{
    size_t realsize = size * nmemb;
    struct MemoryStruct *mem = (struct MemoryStruct *)userp;

    char *ptr = realloc(mem->memory, mem->size + realsize + 1);
    if (!ptr)
    {
        printf("not enough memory (realloc returned NULL)\n");
        return 0;
    }

    mem->memory = ptr;
    memcpy(&(mem->memory[mem->size]), contents, realsize);
    mem->size += realsize;
    mem->memory[mem->size] = 0;

    return realsize;
}

/// 날짜 문자열을 time_t로 변환 (Windows 호환)
static time_t parse_date(const char *date_str)
{
    struct tm tm = {0};
    sscanf(date_str, "%4d%2d%2d", &tm.tm_year, &tm.tm_mon, &tm.tm_mday);
    tm.tm_year -= 1900;
    tm.tm_mon -= 1;
    return mktime(&tm);
}

static int days_between(const char *start_date, const char *end_date)
{
    time_t start = parse_date(start_date);
    time_t end = parse_date(end_date);
    return (int)(difftime(end, start) / (60 * 60 * 24)) + 1;
}

static void encode_params(CURL *curl, const char *edu_office, const char *school_name, char *edu_encoded, char *school_encoded, size_t size)
{
    char *tmp1 = curl_easy_escape(curl, edu_office, 0);
    char *tmp2 = curl_easy_escape(curl, school_name, 0);
    strncpy(edu_encoded, tmp1, size - 1);
    strncpy(school_encoded, tmp2, size - 1);
    curl_free(tmp1);
    curl_free(tmp2);
}

static bool perform_curl_request(const char *url, char **response)
{
    CURL *curl = curl_easy_init();
    if (!curl)
        return false;

    struct MemoryStruct chunk;
    chunk.memory = malloc(1);
    chunk.size = 0;

    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&chunk);

    CURLcode res = curl_easy_perform(curl);

    printf("기간 요청 URL: %s\n", url);
    printf("응답 내용:\n%s\n", chunk.memory);
    curl_easy_cleanup(curl);

    if (res != CURLE_OK)
    {
        free(chunk.memory);
        return false;
    }

    *response = chunk.memory;
    return true;
}

static bool check_api_error(struct json_object *json) {
    struct json_object *error;
    if (json_object_object_get_ex(json, "RESULT", &error)) {
        struct json_object *code, *message;
        if (json_object_object_get_ex(error, "CODE", &code) &&
            json_object_object_get_ex(error, "MESSAGE", &message)) {
            printf("API 에러 발생: %s - %s\n", 
                   json_object_get_string(code),
                   json_object_get_string(message));
            return true;
        }
    }
    return false;
}

// 급식 정보를 저장할 구조체
typedef struct {
    char date[9];        // YYYYMMDD 형식
    char meal_type[20];  // 조식, 중식, 석식
    char menu[1024];     // 메뉴 정보
} MealInfo;

// 메뉴 문자열 정리 함수
static void clean_menu_string(char *menu) {
    char *p = menu;
    char *q = menu;
    
    while (*p) {
        if (*p == '<') {
            // HTML 태그 건너뛰기
            while (*p && *p != '>') p++;
            if (*p == '>') p++;
            continue;
        }
        if (*p == '&') {
            // HTML 엔티티 건너뛰기
            while (*p && *p != ';') p++;
            if (*p == ';') p++;
            continue;
        }
        *q++ = *p++;
    }
    *q = '\0';
}

bool get_meal_from_neis(const char *edu_office, const char *school_name, const char *date, char *meal)
{
    CURL *curl = curl_easy_init();
    if (!curl)
        return false;

    char encoded_edu[128], encoded_school[128];
    encode_params(curl, edu_office, school_name, encoded_edu, encoded_school, sizeof(encoded_edu));
    curl_easy_cleanup(curl);

    char url[1024];
    snprintf(url, sizeof(url), "%s?KEY=%s&Type=json&ATPT_OFCDC_SC_CODE=%s&SD_SCHUL_CODE=%s&MLSV_YMD=%s",
             NEIS_API_URL, NEIS_API_KEY, encoded_edu, encoded_school, date);

    char *response = NULL;
    if (!perform_curl_request(url, &response))
        return false;

    struct json_object *json = json_tokener_parse(response);
    free(response);

    if (!json)
        return false;

    if (check_api_error(json)) {
        json_object_put(json);
        return false;
    }

    struct json_object *meal_service;
    if (json_object_object_get_ex(json, "mealServiceDietInfo", &meal_service)) {
        struct json_object *row;
        if (json_object_object_get_ex(meal_service, "row", &row) && json_object_is_type(row, json_type_array)) {
            int array_len = json_object_array_length(row);
            char formatted_meals[1024] = {0};
            
            for (int i = 0; i < array_len; i++) {
                struct json_object *meal_data = json_object_array_get_idx(row, i);
                struct json_object *menu, *meal_type, *meal_date;
                
                if (meal_data) {
                    char meal_info[256] = {0};
                    
                    // 식사 구분 (조식, 중식, 석식)
                    if (json_object_object_get_ex(meal_data, "MMEAL_SC_NM", &meal_type)) {
                        strcat(meal_info, json_object_get_string(meal_type));
                        strcat(meal_info, "\n");
                    }
                    
                    // 메뉴 정보
                    if (json_object_object_get_ex(meal_data, "DDISH_NM", &menu)) {
                        char clean_menu[1024];
                        strncpy(clean_menu, json_object_get_string(menu), sizeof(clean_menu) - 1);
                        clean_menu_string(clean_menu);
                        strcat(meal_info, clean_menu);
                    }
                    
                    strcat(formatted_meals, meal_info);
                    if (i < array_len - 1) {
                        strcat(formatted_meals, "\n\n");
                    }
                }
            }
            
            strncpy(meal, formatted_meals, MAX_MEAL_LEN - 1);
            json_object_put(json);
            return true;
        }
    }

    json_object_put(json);
    return false;
}

bool get_meals_period_from_neis(const char *edu_office, const char *school_name,
                                const char *start_date, const char *end_date,
                                char *meals_json)
{
    int days = days_between(start_date, end_date);
    if (days > MAX_PERIOD_DAYS)
        return false;

    CURL *curl = curl_easy_init();
    if (!curl)
        return false;

    char encoded_edu[128], encoded_school[128];
    encode_params(curl, edu_office, school_name, encoded_edu, encoded_school, sizeof(encoded_edu));
    curl_easy_cleanup(curl);

    char url[1024];
    snprintf(url, sizeof(url), "%s?KEY=%s&Type=json&ATPT_OFCDC_SC_CODE=%s&SD_SCHUL_CODE=%s&MLSV_FROM_YMD=%s&MLSV_TO_YMD=%s",
             NEIS_API_URL, NEIS_API_KEY, encoded_edu, encoded_school, start_date, end_date);

    char *response = NULL;
    if (!perform_curl_request(url, &response))
        return false;

    struct json_object *json = json_tokener_parse(response);
    free(response);

    if (!json)
        return false;

    if (check_api_error(json)) {
        json_object_put(json);
        return false;
    }

    struct json_object *meal_service;
    if (json_object_object_get_ex(json, "mealServiceDietInfo", &meal_service)) {
        struct json_object *row;
        if (json_object_object_get_ex(meal_service, "row", &row) && json_object_is_type(row, json_type_array)) {
            int array_len = json_object_array_length(row);
            char formatted_meals[4096] = {0};
            char current_date[9] = {0};
            
            for (int i = 0; i < array_len; i++) {
                struct json_object *meal_data = json_object_array_get_idx(row, i);
                struct json_object *menu, *meal_type, *meal_date;
                
                if (meal_data) {
                    // 날짜 정보
                    if (json_object_object_get_ex(meal_data, "MLSV_YMD", &meal_date)) {
                        const char *date_str = json_object_get_string(meal_date);
                        if (strcmp(current_date, date_str) != 0) {
                            if (i > 0) {
                                strcat(formatted_meals, "\n\n");
                            }
                            strcpy(current_date, date_str);
                            strcat(formatted_meals, "=== ");
                            strcat(formatted_meals, date_str);
                            strcat(formatted_meals, " ===\n");
                        }
                    }
                    
                    // 식사 구분
                    if (json_object_object_get_ex(meal_data, "MMEAL_SC_NM", &meal_type)) {
                        strcat(formatted_meals, json_object_get_string(meal_type));
                        strcat(formatted_meals, "\n");
                    }
                    
                    // 메뉴 정보
                    if (json_object_object_get_ex(meal_data, "DDISH_NM", &menu)) {
                        char clean_menu[1024];
                        strncpy(clean_menu, json_object_get_string(menu), sizeof(clean_menu) - 1);
                        clean_menu_string(clean_menu);
                        strcat(formatted_meals, clean_menu);
                        strcat(formatted_meals, "\n");
                    }
                }
            }
            
            strncpy(meals_json, formatted_meals, MAX_MEAL_LEN - 1);
            json_object_put(json);
            return true;
        }
    }

    json_object_put(json);
    return false;
}
