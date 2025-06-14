#include "common.h"
#include "protocol.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>   // CURL 라이브러리
#include <json-c/json.h> // JSON 파싱을 위한 json-c 라이브러리
#include <time.h>

#define NEIS_API_KEY "53ea0d0873e048e188a0b13834667795"                // NEIS API 키
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

    struct json_object *meal_service;
    if (json_object_object_get_ex(json, "mealServiceDietInfo", &meal_service))
    {
        struct json_object *row;
        if (json_object_object_get_ex(meal_service, "row", &row) && json_object_is_type(row, json_type_array))
        {
            struct json_object *first_meal = json_object_array_get_idx(row, 0);
            struct json_object *menu;
            if (first_meal && json_object_object_get_ex(first_meal, "DDISH_NM", &menu))
            {
                strncpy(meal, json_object_get_string(menu), MAX_MEAL_LEN - 1);
                json_object_put(json);
                return true;
            }
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

    struct json_object *meal_service;
    if (json_object_object_get_ex(json, "mealServiceDietInfo", &meal_service))
    {
        strncpy(meals_json, json_object_to_json_string(meal_service), MAX_MEAL_LEN - 1);
        json_object_put(json);
        return true;
    }

    json_object_put(json);
    return false;
}
