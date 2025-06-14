#ifndef MENU_H
#define MENU_H

#include "common.h"
#include "protocol.h"

// 메뉴 출력 함수
void print_login_menu(void);
void print_general_menu(void);
void print_parent_menu(void);

// 메뉴 입력 처리 함수
void handle_login_menu_input(int choice);
void handle_general_menu_input(int choice);
void handle_parent_menu_input(int choice);

// 메뉴 핸들러 함수
void handle_general_menu(int choice);
void handle_parent_menu(int choice);

// 서브메뉴 출력 함수
void print_meal_query_menu(void);
void print_user_management_menu(void);
void print_child_management_menu(void);

#endif // MENU_H 