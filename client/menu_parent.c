#include "common.h"
#include "protocol.h"
#include "handlers.h"
#include "menu.h"
#include "input_util.h"
#include <stdio.h>
#include <string.h>

extern char current_user_id[MAX_ID_LEN];

// 부모 사용자 메뉴 처리
void handle_parent_menu(int choice)
{
    char response[BUFFER_SIZE];
    char child_id[MAX_ID_LEN];
    char date[9], end_date[9];
    char edu_office[MAX_EDU_OFFICE_LEN], school_name[MAX_SCHOOL_NAME_LEN];

    switch (choice)
    {
    case 1:
    { // 자녀 급식 조회
        print_meal_query_menu();
        int sub_choice;
        scanf("%d", &sub_choice);
        getchar();

        printf("자녀 아이디: ");
        fgets(child_id, sizeof(child_id), stdin);
        child_id[strcspn(child_id, "\n")] = 0;

        if (sub_choice == 1)
        {
            printf("날짜 (YYYYMMDD): ");
            fgets(date, sizeof(date), stdin);
            date[strcspn(date, "\n")] = 0;

            if (get_child_meal(current_user_id, child_id, date, response))
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

            if (get_child_meals_period(current_user_id, child_id, date, end_date, response))
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
        getchar();

        printf("교육청 이름: ");
        fgets(edu_office, sizeof(edu_office), stdin);
        edu_office[strcspn(edu_office, "\n")] = 0;

        printf("학교 코드: ");
        fgets(school_name, sizeof(school_name), stdin);
        school_name[strcspn(school_name, "\n")] = 0;

        if (sub_choice == 1)
        {
            printf("날짜 (YYYYMMDD): ");
            fgets(date, sizeof(date), stdin);
            date[strcspn(date, "\n")] = 0;

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
    { // 자녀 관리
        print_child_management_menu();
        int sub_choice;
        scanf("%d", &sub_choice);
        getchar();

        switch (sub_choice)
        {
        case 1:
        { // 자녀 추가
            printf("자녀 아이디: ");
            fgets(child_id, sizeof(child_id), stdin);
            child_id[strcspn(child_id, "\n")] = 0;

            if (add_child(current_user_id, child_id, response))
            {
                printf("자녀 추가 성공: %s\n", response);
            }
            else
            {
                printf("자녀 추가 실패: %s\n", response);
            }
            break;
        }
        case 2:
        { // 자녀 삭제
            printf("삭제할 자녀 아이디: ");
            fgets(child_id, sizeof(child_id), stdin);
            child_id[strcspn(child_id, "\n")] = 0;

            if (delete_child(current_user_id, child_id, response))
            {
                printf("자녀 삭제 성공: %s\n", response);
            }
            else
            {
                printf("자녀 삭제 실패: %s\n", response);
            }
            break;
        }
        case 3:
        { // 자녀 조회
            if (get_children(current_user_id, response))
            {
                printf("자녀 목록:\n%s\n", response);
            }
            else
            {
                printf("자녀 목록 조회 실패: %s\n", response);
            }
            break;
        }
        }
        break;
    }
    case 4:
    { // 로그아웃
        handle_logout();
        printf("로그아웃되었습니다.\n");
        break;
    }
    }
}
