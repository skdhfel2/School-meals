#include "common.h"
#include "protocol.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>   // CURL 라이브러리
#include <json-c/json.h> // JSON 파싱을 위한 json-c 라이브러리
#include <time.h>

const char* NEIS_API_KEY = "53ea0d0873e048e188a0b13834667795";
#define NEIS_API_URL "https://open.neis.go.kr/hub/mealServiceDietInfo" // NEIS API URL

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

    curl_easy_cleanup(curl);

    if (res != CURLE_OK)
    {
        free(chunk.memory);
        return false;
    }

    *response = chunk.memory;
    return true;
}

bool get_meal_from_neis(const char *edu_office, const char *school_code, const char *date, char *meal)
{
    char url[1024];
    snprintf(url, sizeof(url),
             "%s?KEY=%s&Type=json&ATPT_OFCDC_SC_CODE=%s&SD_SCHUL_CODE=%s&MLSV_YMD=%s",
             NEIS_API_URL, NEIS_API_KEY, edu_office, school_code, date);

    printf("🔍 요청 URL: %s\n", url);

    char *response = NULL;
    if (!perform_curl_request(url, &response))
    {
        printf("❌ CURL 요청 실패\n");
        return false;
    }

    struct json_object *json = json_tokener_parse(response);
    free(response);

    if (!json)
    {
        printf("❌ JSON 파싱 실패\n");
        return false;
    }

    struct json_object *meal_service_array;
    if (!json_object_object_get_ex(json, "mealServiceDietInfo", &meal_service_array))
    {
        printf("❌ mealServiceDietInfo 없음\n");
        json_object_put(json);
        return false;
    }

    // 배열인지 확인
    if (!json_object_is_type(meal_service_array, json_type_array))
    {
        printf("❌ mealServiceDietInfo가 배열이 아님\n");
        json_object_put(json);
        return false;
    }

    // 두 번째 요소 (index = 1)에 row가 있음
    struct json_object *second_obj = json_object_array_get_idx(meal_service_array, 1);

    struct json_object *row;
    if (!json_object_object_get_ex(second_obj, "row", &row))
    {
        printf("❌ row 없음\n");
        json_object_put(json);
        return false;
    }

    if (!json_object_is_type(row, json_type_array))
    {
        printf("❌ row가 배열이 아님\n");
        json_object_put(json);
        return false;
    }

    struct json_object *first_meal = json_object_array_get_idx(row, 0);
    struct json_object *menu;
    if (first_meal && json_object_object_get_ex(first_meal, "DDISH_NM", &menu))
    {
        const char *menu_str = json_object_get_string(menu);
        char *temp = strdup(menu_str);
        char *result = malloc(MAX_MEAL_LEN);
        result[0] = '\0';
        
        // 날짜 포맷팅 (YYYYMMDD -> YYYY년 MM월 DD일)
        char formatted_date[20];
        snprintf(formatted_date, sizeof(formatted_date), "%c%c%c%c년 %c%c월 %c%c일",
                 date[0], date[1], date[2], date[3],
                 date[4], date[5], date[6], date[7]);
        
        // 제목과 날짜 추가
        strcat(result, "\n🍱 오늘의 급식 메뉴\n\n");
        strcat(result, "📅 ");
        strcat(result, formatted_date);
        strcat(result, "\n\n");
        
        // <br/> 태그로 분리
        char *item = strtok(temp, "<br/>");
        while (item) {
            // 알레르기 정보 제거 (괄호 안의 내용)
            char *paren = strchr(item, '(');
            if (paren) {
                *paren = '\0';
            }
            
            // 앞뒤 공백 제거
            while (*item == ' ') item++;
            char *end = item + strlen(item) - 1;
            while (end > item && *end == ' ') {
                *end = '\0';
                end--;
            }
            
            // 빈 문자열이 아닌 경우에만 추가
            if (strlen(item) > 0) {
                strcat(result, "• ");
                strcat(result, item);
                strcat(result, "\n");
            }
            
            item = strtok(NULL, "<br/>");
        }
        
        strncpy(meal, result, MAX_MEAL_LEN - 1);
        free(temp);
        free(result);
        
        json_object_put(json);
        printf("🍱 급식 메뉴:\n%s\n", meal);
        return true;
    }
    else
    {
        printf("❌ DDISH_NM 없음\n");
    }

    json_object_put(json);
    return false;
}
bool get_meals_period_from_neis(const char *edu_office, const char *school_code,
                                const char *start_date, const char *end_date,
                                char *meals_json)
{
    if (days_between(start_date, end_date) > 7)
    {
        printf("❌ 기간이 7일 초과\n");
        strcpy(meals_json, "기간은 최대 7일까지 가능합니다.");
        return false;
    }

    char url[1024];
    snprintf(url, sizeof(url),
             "%s?KEY=%s&Type=json&ATPT_OFCDC_SC_CODE=%s&SD_SCHUL_CODE=%s&MLSV_FROM_YMD=%s&MLSV_TO_YMD=%s",
             NEIS_API_URL, NEIS_API_KEY, edu_office, school_code, start_date, end_date);

    printf("🔍 요청 URL: %s\n", url);

    char *response = NULL;
    if (!perform_curl_request(url, &response))
    {
        printf("❌ CURL 요청 실패 (기간)\n");
        return false;
    }

    struct json_object *json = json_tokener_parse(response);
    free(response);

    if (!json)
    {
        printf("❌ JSON 파싱 실패 (기간)\n");
        return false;
    }

    struct json_object *meal_service_array;
    if (!json_object_object_get_ex(json, "mealServiceDietInfo", &meal_service_array))
    {
        printf("❌ mealServiceDietInfo 없음 (기간)\n");
        json_object_put(json);
        return false;
    }

    // 배열인지 확인
    if (!json_object_is_type(meal_service_array, json_type_array))
    {
        printf("❌ mealServiceDietInfo가 배열이 아님 (기간)\n");
        json_object_put(json);
        return false;
    }

    // 두 번째 요소 (index = 1)에 row가 있음
    struct json_object *second_obj = json_object_array_get_idx(meal_service_array, 1);
    if (!second_obj)
    {
        printf("❌ mealServiceDietInfo[1] 없음 (기간)\n");
        json_object_put(json);
        return false;
    }

    struct json_object *row;
    if (!json_object_object_get_ex(second_obj, "row", &row))
    {
        printf("❌ row 없음 (기간)\n");
        json_object_put(json);
        return false;
    }

    if (!json_object_is_type(row, json_type_array))
    {
        printf("❌ row가 배열이 아님 (기간)\n");
        json_object_put(json);
        return false;
    }

    char buffer[MAX_MEAL_LEN] = "";
    strcat(buffer, "\n🍱 기간별 급식 메뉴\n\n");
    
    for (int i = 0; i < json_object_array_length(row); i++)
    {
        struct json_object *item = json_object_array_get_idx(row, i);
        struct json_object *date_obj, *menu_obj;

        const char *menu = "", *date = "";

        if (json_object_object_get_ex(item, "MLSV_YMD", &date_obj))
            date = json_object_get_string(date_obj);
        if (json_object_object_get_ex(item, "DDISH_NM", &menu_obj))
            menu = json_object_get_string(menu_obj);

        // <br/> 태그로 분리
        char *temp = strdup(menu);
        char *item_str = strtok(temp, "<br/>");
        char entry[512] = "";
        
        // 날짜 포맷팅 (YYYYMMDD -> YYYY년 MM월 DD일)
        char formatted_date[20];
        snprintf(formatted_date, sizeof(formatted_date), "%c%c%c%c년 %c%c월 %c%c일",
                 date[0], date[1], date[2], date[3],
                 date[4], date[5], date[6], date[7]);
        
        snprintf(entry, sizeof(entry), "📅 %s\n", formatted_date);
        
        while (item_str) {
            // 알레르기 정보 제거 (괄호 안의 내용)
            char *paren = strchr(item_str, '(');
            if (paren) {
                *paren = '\0';
            }
            
            // 앞뒤 공백 제거
            while (*item_str == ' ') item_str++;
            char *end = item_str + strlen(item_str) - 1;
            while (end > item_str && *end == ' ') {
                *end = '\0';
                end--;
            }
            
            // 빈 문자열이 아닌 경우에만 추가
            if (strlen(item_str) > 0) {
                strcat(entry, "• ");
                strcat(entry, item_str);
                strcat(entry, "\n");
            }
            
            item_str = strtok(NULL, "<br/>");
        }
        
        strncat(buffer, entry, sizeof(buffer) - strlen(buffer) - 1);
        strcat(buffer, "\n");  // 날짜 사이에 빈 줄 추가
        free(temp);
    }

    strncpy(meals_json, buffer, MAX_MEAL_LEN - 1);
    json_object_put(json);
    return true;
}
