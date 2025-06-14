#include "menu.h"
#include "handlers.h"
#include <stdio.h>
#include <string.h>

// 메뉴 출력 함수
void print_login_menu(void) {
    printf("\n=== 로그인 ===\n");
    printf("1. 로그인\n");
    printf("2. 회원가입\n");
    printf("3. 종료\n");
    printf("선택: ");
}

void print_general_menu(void) {
    printf("\n=== 일반 사용자 메뉴 ===\n");
    printf("1. 급식 조회\n");
    printf("2. 다른 학교 급식 조회\n");
    printf("3. 사용자 관리\n");
    printf("4. 로그아웃\n");
    printf("5. 종료\n");
    printf("선택: ");
}

void print_parent_menu(void) {
    printf("\n=== 부모 사용자 메뉴 ===\n");
    printf("1. 자녀 급식 조회\n");
    printf("2. 다른 학교 급식 조회\n");
    printf("3. 자녀 관리\n");
    printf("4. 로그아웃\n");
    printf("5. 종료\n");
    printf("선택: ");
}

// 서브메뉴 출력 함수
void print_meal_query_menu(void) {
    printf("\n=== 급식 조회 ===\n");
    printf("1. 단일 급식 조회\n");
    printf("2. 기간별 급식 조회\n");
    printf("선택: ");
}

void print_user_management_menu(void) {
    printf("\n=== 사용자 관리 ===\n");
    printf("1. 사용자 추가\n");
    printf("2. 사용자 수정\n");
    printf("3. 사용자 삭제\n");
    printf("선택: ");
}

void print_child_management_menu(void) {
    printf("\n=== 자녀 관리 ===\n");
    printf("1. 자녀 추가\n");
    printf("2. 자녀 삭제\n");
    printf("3. 자녀 조회\n");
    printf("선택: ");
}

// 메뉴 입력 처리 함수
void handle_login_menu_input(int choice) {
    char response[BUFFER_SIZE];
    char id[MAX_ID_LEN], pw[MAX_PW_LEN];
    char edu_office[MAX_EDU_OFFICE_LEN], school_name[MAX_SCHOOL_NAME_LEN];

    switch (choice) {
        case 1: // 로그인
            printf("아이디: ");
            fgets(id, sizeof(id), stdin);
            id[strcspn(id, "\n")] = 0;

            printf("비밀번호: ");
            fgets(pw, sizeof(pw), stdin);
            pw[strcspn(pw, "\n")] = 0;

            if (handle_login(id, pw, response)) {
                printf("로그인 성공: %s\n", response);
            } else {
                printf("로그인 실패: %s\n", response);
            }
            break;

        case 2: // 회원가입
            printf("아이디: ");
            fgets(id, sizeof(id), stdin);
            id[strcspn(id, "\n")] = 0;

            printf("비밀번호: ");
            fgets(pw, sizeof(pw), stdin);
            pw[strcspn(pw, "\n")] = 0;

            // TODO: 추후 이름 필드 구현 시 활성화 예정
            /*
            printf("이름: ");
            fgets(name, sizeof(name), stdin);
            name[strcspn(name, "\n")] = 0;
            */

            printf("교육청 이름: ");
            fgets(edu_office, sizeof(edu_office), stdin);
            edu_office[strcspn(edu_office, "\n")] = 0;

            printf("학교 코드: ");
            fgets(school_name, sizeof(school_name), stdin);
            school_name[strcspn(school_name, "\n")] = 0;

            if (handle_register(id, pw, edu_office, school_name, response)) {
                printf("회원가입 성공: %s\n", response);
            } else {
                printf("회원가입 실패: %s\n", response);
            }
            break;
    }
}

void handle_general_menu_input(int choice) {
    handle_general_menu(choice);
}

void handle_parent_menu_input(int choice) {
    // TODO: 추후 부모 사용자 기능 구현 시 활성화 예정
    printf("부모 사용자 기능은 현재 비활성화되어 있습니다.\n");
    /*
    handle_parent_menu(choice);
    */
} 