#include "common.h"
#include "protocol.h"
#include "handlers.h"
#include "menu.h"
#include "input_util.h"
#include <stdio.h>
#include <string.h>

extern char current_user_id[MAX_ID_LEN];
extern char current_user_edu_office[MAX_EDU_OFFICE_LEN];
extern char current_user_school[MAX_SCHOOL_NAME_LEN];

// 일반 사용자 메뉴 처리
void handle_general_menu(int choice)
{
    char response[BUFFER_SIZE];
    char id[MAX_ID_LEN], pw[MAX_PW_LEN];
    char edu_office[MAX_EDU_OFFICE_LEN], school_name[MAX_SCHOOL_NAME_LEN];
    char date[9], end_date[9];
    int i;

    switch (choice)
    {
    case 1:
    { // 급식 조회
        print_meal_query_menu();
        int sub_choice;
        scanf("%d", &sub_choice);
        while (getchar() != '\n')
            ; // 버퍼 비우기

        if (sub_choice == 1)
        {
            if (!get_date_input(date, sizeof(date)))
            {
                return;
            }

            if (get_meal_from_neis(current_user_edu_office, current_user_school, date, response))
            {
                printf("급식 정보:\n%s\n", response);
            }
            else
            {
                printf("급식 정보 조회 실패: %s\n", response);
            }
        }
        else if (sub_choice == 2)
        {
            if (!get_period_input(date, end_date, sizeof(date)))
            {
                return;
            }

            if (get_meals_period_from_neis(current_user_edu_office, current_user_school,
                                           date, end_date, response))
            {
                printf("급식 정보:\n%s\n", response);
            }
            else
            {
                printf("급식 정보 조회 실패: %s\n", response);
            }
        }
        break;
    }
    case 2:
    { // 다른 학교 급식 조회
        print_meal_query_menu();
        int sub_choice;
        scanf("%d", &sub_choice);
        while (getchar() != '\n')
            ; // 버퍼 비우기

        printf("교육청 이름: ");
        fgets(edu_office, sizeof(edu_office), stdin);
        edu_office[strcspn(edu_office, "\n")] = 0;

        printf("학교 이름: ");
        fgets(school_name, sizeof(school_name), stdin);
        school_name[strcspn(school_name, "\n")] = 0;

        if (sub_choice == 1)
        {
            printf("날짜 (YYYYMMDD): ");
            for (i = 0; i < 8; i++)
            {
                int c = getchar();
                if (c == '\n' || c == EOF)
                {
                    printf("❌ 날짜 형식이 올바르지 않습니다. YYYYMMDD 형식으로 입력해주세요.\n");
                    return;
                }
                date[i] = c;
            }
            date[8] = '\0';
            while (getchar() != '\n')
                ; // 남은 입력 버퍼 비우기

            if (get_meal_from_neis(edu_office, school_name, date, response))
            {
                printf("급식 정보:\n%s\n", response);
            }
            else
            {
                printf("급식 정보 조회 실패: %s\n", response);
            }
        }
        else if (sub_choice == 2)
        {
            if (!get_period_input(date, end_date, sizeof(date)))
            {
                return;
            }

            if (get_meals_period_from_neis(edu_office, school_name, date, end_date, response))
            {
                printf("급식 정보:\n%s\n", response);
            }
            else
            {
                printf("급식 정보 조회 실패: %s\n", response);
            }
        }
        break;
    }
    case 3:
    { // 사용자 관리
        print_user_management_menu();
        int sub_choice;
        scanf("%d", &sub_choice);
        getchar();

        switch (sub_choice)
        {
        case 1: // 사용자 추가
        {
            printf("\n👤 [사용자 추가]\n");

            get_id_input(id, sizeof(id));
            get_password_input(pw, sizeof(pw));
            get_edu_office_input(edu_office, sizeof(edu_office));
            get_school_input(school_name, sizeof(school_name));

            int status;
            char message[BUFFER_SIZE];

            if (handle_add_user(id, pw, edu_office, school_name, &status, message))
            {
                if (status == RESP_SUCCESS)
                {
                    printf("✅ 사용자 추가 성공: %s\n", message);
                }
                else if (status == RESP_DUPLICATE)
                {
                    printf("⚠️ 사용자 추가 실패: 중복된 아이디입니다.\n");
                }
                else
                {
                    printf("❌ 사용자 추가 실패: %s\n", message);
                }
            }
            else
            {
                printf("❌ 사용자 추가 실패 (통신 오류): %s\n", message);
            }
            break;
        }

        case 2:
        { // 사용자 수정
            printf("\n✏️ [사용자 수정]\n");

            get_id_input(id, sizeof(id));                         //  기존 사용자 ID
            get_password_input(pw, sizeof(pw));                   //  새 비밀번호
            get_edu_office_input(edu_office, sizeof(edu_office)); //  새 교육청
            get_school_input(school_name, sizeof(school_name));   //  새 학교명

            if (handle_update_user(id, pw, edu_office, school_name, response))
            {
                printf("✅ 사용자 수정 성공: %s\n", response);
            }
            else
            {
                printf("❌ 사용자 수정 실패: %s\n", response);
            }
            break;
        }
        case 3:
        { // 사용자 삭제
            printf("\n🗑️ [사용자 삭제]\n");
            printf("→ 삭제할 아이디를 입력하세요.\n");

            get_id_input(id, sizeof(id)); //  유효성 검사 포함

            if (handle_delete_user(id, response))
            {
                printf("✅ 사용자 삭제 성공: %s\n", response);
            }
            else
            {
                printf("❌ 사용자 삭제 실패: %s\n", response);
            }
            break;
        }
        }
        break;
    }
    case 4:
    { // 로그아웃
        handle_logout();
        printf("\n👋 로그아웃 되었습니다.\n");
        printf("➡️ 메인 메뉴로 돌아갑니다.\n");
        break;
    }
    }
}