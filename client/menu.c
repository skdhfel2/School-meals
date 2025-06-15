#include "menu.h"
#include "handlers.h"
#include <stdio.h>
#include <string.h>

// 메뉴 출력 함수
void print_login_menu(void)
{
    printf("\n=== 로그인 ===\n");
    printf("1. 로그인\n");
    printf("2. 회원가입\n");
    printf("3. 종료\n");
    printf("선택: ");
}

void print_general_menu(void)
{
    printf("\n=== 일반 사용자 메뉴 ===\n");
    printf("1. 급식 조회\n");
    printf("2. 다른 학교 급식 조회\n");
    printf("3. 사용자 관리\n");
    printf("4. 로그아웃\n");
    printf("5. 종료\n");
    printf("선택: ");
}

void print_parent_menu(void)
{
    printf("\n=== 부모 사용자 메뉴 ===\n");
    printf("1. 자녀 급식 조회\n");
    printf("2. 다른 학교 급식 조회\n");
    printf("3. 자녀 관리\n");
    printf("4. 로그아웃\n");
    printf("5. 종료\n");
    printf("선택: ");
}

// 서브메뉴 출력 함수
void print_meal_query_menu(void)
{
    printf("\n=== 급식 조회 ===\n");
    printf("1. 단일 급식 조회\n");
    printf("2. 기간별 급식 조회\n");
    printf("선택: ");
}

void print_user_management_menu(void)
{
    printf("\n=== 사용자 관리 ===\n");
    printf("1. 사용자 추가\n");
    printf("2. 사용자 수정\n");
    printf("3. 사용자 삭제\n");
    printf("선택: ");
}

void print_child_management_menu(void)
{
    printf("\n=== 자녀 관리 ===\n");
    printf("1. 자녀 추가\n");
    printf("2. 자녀 삭제\n");
    printf("3. 자녀 조회\n");
    printf("선택: ");
}

// 메뉴 입력 처리 함수
void handle_login_menu_input(int choice)
{
    char response[BUFFER_SIZE];
    char id[MAX_ID_LEN], pw[MAX_PW_LEN];
    char edu_office[MAX_EDU_OFFICE_LEN], school_name[MAX_SCHOOL_NAME_LEN];

    switch (choice)
    {
    case 1: // 로그인
        printf("\n🔐 [로그인]\n");
        printf("→ 등록된 아이디와 비밀번호를 입력해주세요.\n");

        printf("\n🆔 아이디: ");
        fgets(id, sizeof(id), stdin);
        id[strcspn(id, "\n")] = 0;

        printf("🔑 비밀번호: ");
        fgets(pw, sizeof(pw), stdin);
        pw[strcspn(pw, "\n")] = 0;

        if (handle_login(id, pw, response))
        {
            printf("✅ 로그인 성공! 환영합니다, %s님\n", id);
        }
        else
        {
            printf("❌ 로그인 실패: %s\n", response);
            printf("👉 아이디 또는 비밀번호를 다시 확인해주세요.\n");
        }
        break;

    case 2: // 회원가입
        printf("\n📝 [회원가입]\n");
        printf("→ 신규 아이디, 비밀번호, 교육청, 학교명을 입력해주세요.\n");

        // ID 유효성 검사
        while (1)
        {
            printf("\n🆔 [아이디 입력]\n");
            printf("→ 4~16자의 영문자와 숫자로만 구성\n");
            printf("예시: user123, hello2024\n");
            printf("입력: ");
            fgets(id, sizeof(id), stdin);
            id[strcspn(id, "\n")] = 0;

            if (is_valid_id(id))
                break;
            printf("❌ 유효하지 않은 형식입니다. 다시 입력해주세요.\n");
        }

        // PW 유효성 검사
        while (1)
        {
            printf("\n🔑 [비밀번호 입력]\n");
            printf("→ 공백 없이 4자 이상 입력 (영문/숫자 조합 권장)\n");
            printf("입력: ");
            fgets(pw, sizeof(pw), stdin);
            pw[strcspn(pw, "\n")] = 0;

            if (is_valid_password(pw))
                break;
            printf("❌ 비밀번호 형식이 올바르지 않습니다. 다시 시도해주세요.\n");
        }

        // 교육청 이름
        printf("\n🏫 [교육청 이름 입력]\n");
        printf("예시: 서울특별시교육청, 경기도교육청\n");
        printf("입력: ");
        fgets(edu_office, sizeof(edu_office), stdin);
        edu_office[strcspn(edu_office, "\n")] = 0;

        // 학교명
        printf("\n🏫 [학교 이름 입력]\n");
        printf("예시: 서울고등학교, 부천중학교\n");
        printf("입력: ");
        fgets(school_name, sizeof(school_name), stdin);
        school_name[strcspn(school_name, "\n")] = 0;

        if (handle_register(id, pw, edu_office, school_name, response))
        {
            printf("🎉 회원가입 성공! 이제 로그인하여 서비스를 이용하실 수 있습니다.\n");
        }
        else
        {
            printf("❌ 회원가입 실패: %s\n", response);
            printf("👉 입력한 정보를 다시 확인해주세요.\n");
        }
        break;
    }
}

void handle_general_menu_input(int choice)
{
    handle_general_menu(choice);
}

void handle_parent_menu_input(int choice)
{
    // TODO: 추후 부모 사용자 기능 구현 시 활성화 예정
    printf("부모 사용자 기능은 현재 비활성화되어 있습니다.\n");
    /*
    handle_parent_menu(choice);
    */
}