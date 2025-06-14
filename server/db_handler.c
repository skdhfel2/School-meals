#include "common.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sqlite3.h>
#include "protocol.h"
#include "db.h"

static sqlite3 *db = NULL; // db 연결 객체 포인터

bool init_db(void) // 데이터베이스 초기화 함수
{
    int rc = sqlite3_open("school_meals.db", &db);          // db 파일 열기
    sqlite3_exec(db, "PRAGMA foreign_keys = ON;", 0, 0, 0); // 왜래키 기능 on, 이걸 해야 오류나면 알수있음
    if (rc != SQLITE_OK)                                    // 연결 실패하면 오류메시지 출력
    {
        fprintf(stderr, "Cannot open database: %s\n", sqlite3_errmsg(db));
        return false; // 실패하면 종료
    }

    const char *create_users_table = // users라는 테이블 생성
        "CREATE TABLE IF NOT EXISTS users ("
        "id TEXT PRIMARY KEY," // 중복 불가
        "pw TEXT NOT NULL,"
        "name TEXT,"
        "role TEXT NOT NULL," // 사용자 역할(학생,부모)
        "edu_office TEXT,"    // 교육청
        "school_name TEXT"
        ");";

    char *err_msg = NULL;
    rc = sqlite3_exec(db, create_users_table, 0, 0, &err_msg); // SQL실행 함수
    if (rc != SQLITE_OK)
    {
        fprintf(stderr, "SQL error: %s\n", err_msg);
        sqlite3_free(err_msg);
        return false;
    }

    const char *create_children_table =
        "CREATE TABLE IF NOT EXISTS children ("
        "child_id TEXT NOT NULL,"
        "parent_id TEXT NOT NULL,"
        "PRIMARY KEY (child_id, parent_id),"           // 부모가 자녀를 중복등록 하는걸 방지
        "FOREIGN KEY (child_id) REFERENCES users(id)," // 자녀는 users테이블의 ID
        "FOREIGN KEY (parent_id) REFERENCES users(id)" // 부모도 users 테이블의 ID
        ");";

    rc = sqlite3_exec(db, create_children_table, 0, 0, &err_msg);
    if (rc != SQLITE_OK)
    {
        fprintf(stderr, "SQL error: %s\n", err_msg);
        sqlite3_free(err_msg);
        return false;
    }

    const char *create_meals_table =
        "CREATE TABLE IF NOT EXISTS meals ("
        "date TEXT NOT NULL,"                         // 급식 날짜
        "edu_office TEXT NOT NULL,"                   // 교육청 이름
        "school_name TEXT NOT NULL,"                  // 학교 이름
        "meal TEXT NOT NULL,"                         // 실제 급식 내용
        "PRIMARY KEY (date, edu_office, school_name)" // 같은 날 급식 정보 중복 저장 방지(캐시역할)
        ");";

    rc = sqlite3_exec(db, create_meals_table, 0, 0, &err_msg);
    if (rc != SQLITE_OK)
    {
        fprintf(stderr, "SQL error: %s\n", err_msg);
        sqlite3_free(err_msg);
        return false;
    }

    return true;
}

void close_db(void)
{
    if (db) // db가 열려있으면
    {
        sqlite3_close(db); // 메모리 정리(db연결 해제)
        db = NULL;         // 포인터 초기화(접근 막음)
    }
}

bool is_user_exists(const char *id) // 사용자 ID 조회
{
    const char *sql = "SELECT 1 FROM users WHERE id = ?;"; // id가 일치하는 사용자가 있으면 1을 반환함
    sqlite3_stmt *stmt;                                    // SQL문을 준비하는 객체

    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL); // 사용자 입력값을 안전하게 SQL에 넣는 과정
    if (rc != SQLITE_OK)
    {
        return false;
    }

    sqlite3_bind_text(stmt, 1, id, -1, SQLITE_STATIC); // SQL 문 안의 첫 번째 ? 자리에 id 값을 넣는 작업
    bool exists = (sqlite3_step(stmt) == SQLITE_ROW);
    sqlite3_finalize(stmt); // 쿼리객체 정리,
                            // sqlite3_step() 호출 후 반드시 호출해야 함
    return exists;
}

bool add_user(const User *user)
{
    const char *sql =
        "INSERT INTO users (id, pw, name, role, edu_office, school_name) "
        "VALUES (?, ?, ?, ?, ?, ?);"; // 사용자 정보를 users 테이블에 넣는 쿼리

    sqlite3_stmt *stmt;
    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK)
    {
        return false;
    }

    sqlite3_bind_text(stmt, 1, user->id, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, user->pw, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 3, user->name, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 4, user->role, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 5, user->edu_office, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 6, user->school_name, -1, SQLITE_STATIC);

    rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);

    return (rc == SQLITE_DONE);
}

bool get_user(const char *id, User *user)
{
    const char *sql = "SELECT id, pw, name, role, edu_office, school_name FROM users WHERE id = ?;"; // users 테이블에서 특정 ID의 지정한 사용자 정보를 가져옴
    sqlite3_stmt *stmt;

    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK)
    {
        return false;
    }

    sqlite3_bind_text(stmt, 1, id, -1, SQLITE_STATIC); // ?에 사용자 ID 값 바인딩

    if (sqlite3_step(stmt) == SQLITE_ROW) // SELECT 결과가 한 줄이라도 있으면 SQLITE_ROW 반환후 조건이 성립하면
    {
        strncpy(user->id, (const char *)sqlite3_column_text(stmt, 0), MAX_ID_LEN - 1); // SELECT한 0번 컬럼 = id 값을 user->id에 복사
        strncpy(user->pw, (const char *)sqlite3_column_text(stmt, 1), MAX_PW_LEN - 1);
        strncpy(user->name, (const char *)sqlite3_column_text(stmt, 2), MAX_NAME_LEN - 1);
        strncpy(user->role, (const char *)sqlite3_column_text(stmt, 3), sizeof(user->role) - 1);
        strncpy(user->edu_office, (const char *)sqlite3_column_text(stmt, 4), MAX_EDU_OFFICE_LEN - 1);
        strncpy(user->school_name, (const char *)sqlite3_column_text(stmt, 5), MAX_SCHOOL_NAME_LEN - 1);

        sqlite3_finalize(stmt);
        return true;
    }

    sqlite3_finalize(stmt);
    return false;
}
bool get_children_db(const char *parent_id, Child *children, int *count)
{
    sqlite3_stmt *stmt;
    const char *sql = "SELECT id FROM children WHERE parent_id = ?;";
    *count = 0;

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK)
    {
        fprintf(stderr, "SQL prepare error: %s\n", sqlite3_errmsg(db));
        return false;
    }

    if (sqlite3_bind_text(stmt, 1, parent_id, -1, SQLITE_STATIC) != SQLITE_OK)
    {
        fprintf(stderr, "SQL bind error: %s\n", sqlite3_errmsg(db));
        sqlite3_finalize(stmt);
        return false;
    }

    while (sqlite3_step(stmt) == SQLITE_ROW)
    {
        const unsigned char *child_id = sqlite3_column_text(stmt, 0);
        if (*count < MAX_CHILDREN)
        {
            strcpy(children[*count].id, (const char *)child_id);
            (*count)++;
        }
    }

    sqlite3_finalize(stmt);
    return true;
}
bool update_user(const User *user)
{ // users 테이블에서 특정 ID의 사용자를 찾아서 정보 수정
    const char *sql =
        "UPDATE users SET pw = ?, name = ?, edu_office = ?, school_name = ? WHERE id = ?;";

    sqlite3_stmt *stmt;
    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK)
    {
        return false;
    }
    sqlite3_bind_text(stmt, 1, user->pw, -1, SQLITE_STATIC);          // 비밀번호
    sqlite3_bind_text(stmt, 2, user->name, -1, SQLITE_STATIC);        // 이름
    sqlite3_bind_text(stmt, 3, user->edu_office, -1, SQLITE_STATIC);  // 교육청
    sqlite3_bind_text(stmt, 4, user->school_name, -1, SQLITE_STATIC); // 학교명
    sqlite3_bind_text(stmt, 5, user->id, -1, SQLITE_STATIC);

    rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);

    return (rc == SQLITE_DONE);
}

bool delete_user(const char *id, char *response) // 사용자 ID를 기준으로 users 테이블에서 해당 사용자를 삭제하는 함수
{
    const char *sql = "DELETE FROM users WHERE id = ?;"; // ID가 일치하는 사용자 한 명을 삭제
    sqlite3_stmt *stmt;

    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK)
    {
        strcpy(response, "데이터베이스 오류");
        return false;
    }

    sqlite3_bind_text(stmt, 1, id, -1, SQLITE_STATIC);

    rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);

    if (rc == SQLITE_DONE)
    {
        strcpy(response, "사용자 삭제 성공");
        return true;
    }
    else
    {
        strcpy(response, "사용자 삭제 실패");
        return false;
    }
}

bool save_meal(const Meal *meal) // 학교 급식 정보(Meal 구조체)를 DB의 meals 테이블에 저장하는 함수, 이미 있으면 덮어쓰기
{
    const char *sql =
        "INSERT OR REPLACE INTO meals (date, edu_office, school_name, meal) "
        "VALUES (?, ?, ?, ?);";

    sqlite3_stmt *stmt;
    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK)
    {
        return false;
    }

    sqlite3_bind_text(stmt, 1, meal->date, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, meal->edu_office, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 3, meal->school_name, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 4, meal->meal, -1, SQLITE_STATIC);

    rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);

    return (rc == SQLITE_DONE);
}

bool get_meal(const char *date, const char *edu_office, const char *school_name, Meal *meal) // 특정 날짜/교육청/학교에 해당하는 급식 정보를 조회해서 Meal 구조체에 저장해주는 함수
{
    const char *sql =
        "SELECT meal FROM meals WHERE date = ? AND edu_office = ? AND school_name = ?;";

    sqlite3_stmt *stmt;
    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK)
    {
        return false;
    }

    sqlite3_bind_text(stmt, 1, date, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, edu_office, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 3, school_name, -1, SQLITE_STATIC);

    if (sqlite3_step(stmt) == SQLITE_ROW)
    {
        strncpy(meal->date, date, sizeof(meal->date) - 1);
        strncpy(meal->meal, (const char *)sqlite3_column_text(stmt, 0), MAX_MEAL_LEN - 1);

        sqlite3_finalize(stmt);
        return true;
    }

    sqlite3_finalize(stmt);
    return false;
}

bool get_children_raw(const char *parent_id, Child *children, int *count)
{
    const char *sql =
        "SELECT u.id, u.name, u.edu_office, u.school_name "
        "FROM children c JOIN users u ON c.child_id = u.id "
        "WHERE c.parent_id = ?;";

    sqlite3_stmt *stmt;
    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK)
    {
        return false;
    }

    sqlite3_bind_text(stmt, 1, parent_id, -1, SQLITE_STATIC);

    *count = 0;
    while (sqlite3_step(stmt) == SQLITE_ROW && *count < MAX_CHILDREN)
    {
        Child *child = &children[*count];
        strncpy(child->id, (const char *)sqlite3_column_text(stmt, 0), MAX_ID_LEN - 1);
        strncpy(child->name, (const char *)sqlite3_column_text(stmt, 1), MAX_NAME_LEN - 1);
        strncpy(child->edu_office, (const char *)sqlite3_column_text(stmt, 2), MAX_EDU_OFFICE_LEN - 1);
        strncpy(child->school_name, (const char *)sqlite3_column_text(stmt, 3), MAX_SCHOOL_NAME_LEN - 1);
        (*count)++;
    }

    sqlite3_finalize(stmt);
    return true;
}

bool db_add_child(const char *child_id, const char *parent_id) // 부모와 자녀 관계를 children 테이블에 등록하는 함수
{
    if (!is_user_exists(child_id)) // 자녀 ID가 users 테이블에 존재하지 않으면 → 자녀 등록 불가능
    {
        return false;
    }

    const char *sql =
        "INSERT INTO children (child_id, parent_id) VALUES (?, ?);"; // children 테이블에 자녀-부모 관계 추가

    sqlite3_stmt *stmt;
    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK)
    {
        return false;
    }
    // 바인딩된 ID를 넣고 SQL 실행
    sqlite3_bind_text(stmt, 1, child_id, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, parent_id, -1, SQLITE_STATIC);
    // 정리 및 반환
    rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);

    return (rc == SQLITE_DONE);
}

bool db_delete_child(const char *child_id, const char *parent_id) // 부모가 등록한 자녀 중에서, 특정 자녀(child_id)를 부모(parent_id)의 자녀 목록에서 삭제하는 기능
{
    const char *sql =
        "DELETE FROM children WHERE child_id = ? AND parent_id = ?;"; // children 테이블에서 → 자녀 ID와 부모 ID가 모두 일치하는 자녀-부모 관계 하나만 삭제합니다.

    sqlite3_stmt *stmt;
    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK)
    {
        return false;
    }
    // ? 자리에 각각 자녀 ID, 부모 ID를 넣음
    sqlite3_bind_text(stmt, 1, child_id, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, parent_id, -1, SQLITE_STATIC);

    rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);

    return (rc == SQLITE_DONE);
}

bool is_child_registered(const char *child_id, const char *parent_id) // 부모 ID(parent_id)와 자녀 ID(child_id)의 조합이 children 테이블에 이미 등록되어 있는지 확인하는 함수
{
    const char *sql =
        "SELECT 1 FROM children WHERE child_id = ? AND parent_id = ?;"; // children 테이블에서 해당 부모-자녀 관계가 존재하는지 확인

    sqlite3_stmt *stmt;
    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK)
    {
        return false;
    }

    sqlite3_bind_text(stmt, 1, child_id, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, parent_id, -1, SQLITE_STATIC);

    bool exists = (sqlite3_step(stmt) == SQLITE_ROW);
    sqlite3_finalize(stmt);
    return exists;
}

bool verify_user(const char *id, const char *pw) // ID와 비밀번호를 검사해서 로그인 인증을 처리하는 함수
{
    const char *sql = "SELECT 1 FROM users WHERE id = ? AND pw = ?;";
    sqlite3_stmt *stmt;

    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK)
    {
        return false;
    }

    sqlite3_bind_text(stmt, 1, id, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, pw, -1, SQLITE_STATIC);

    bool exists = (sqlite3_step(stmt) == SQLITE_ROW);
    sqlite3_finalize(stmt);
    return exists;
}

static int get_children_callback(void *data, int argc, char **argv, char **col_names)
{
    int *count = (int *)data;
    Child *children = (Child *)data - sizeof(int);

    if (*count < MAX_CHILDREN)
    {
        Child *child = &children[*count];
        strncpy(child->id, argv[0], MAX_ID_LEN - 1);
        strncpy(child->name, argv[1], MAX_NAME_LEN - 1);
        strncpy(child->edu_office, argv[2], MAX_EDU_OFFICE_LEN - 1);
        strncpy(child->school_name, argv[3], MAX_SCHOOL_NAME_LEN - 1);
        (*count)++;
    }
    return 0;
}

bool get_children(const char *parent_id, char *response)
{
    Child children[MAX_CHILDREN];
    int count = 0;

    if (!get_children_raw(parent_id, children, &count))
    {
        strcpy(response, RESP_DB_ERROR);
        return false;
    }

    // JSON 형식으로 변환
    char json[BUFFER_SIZE] = "[";
    for (int i = 0; i < count; i++)
    {
        char child_json[256];
        snprintf(child_json, sizeof(child_json),
                 "{\"id\":\"%s\",\"name\":\"%s\",\"edu_office\":\"%s\",\"school_name\":\"%s\"}%s",
                 children[i].id, children[i].name, children[i].edu_office, children[i].school_name,
                 (i < count - 1) ? "," : "");
        strncat(json, child_json, sizeof(json) - strlen(json) - 1);
    }
    strncat(json, "]", sizeof(json) - strlen(json) - 1);

    strcpy(response, json);
    return true;
}