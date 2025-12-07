// ELEC2645 Unit 2 Project Template
// Command Line Application Menu Handling Code

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
#include "funcs.h"

/* Prototypes mirroring the C++ version */
static void main_menu(void);            /* runs in the main loop */
static void print_main_menu(void);      /* output the main menu description */
static int  get_user_input(void);       /* get a valid integer menu choice */
static void select_menu_item(int input);/* run code based on user's choice */
static void go_back_to_main(void);      /* wait for 'b'/'B' to continue */
static int  is_integer(const char *s);  /* validate integer string */

int main(void) 
{
    /* this will run forever until we call exit(0) in select_menu_item() */
    for(;;) {
        main_menu();
    }
    /* not reached */
    return 0;
}

static void main_menu(void)
{
    print_main_menu();
    {
        int input = get_user_input();
        select_menu_item(input);
    }
}

static int get_user_input(void)
{
    enum { MENU_ITEMS = 5 };   /* 1..4 = items, 5 = Exit */
    char buf[128];
    int valid_input = 0;
    int value = 0;

    do {
        printf("\nSelect item: ");
        if (!fgets(buf, sizeof(buf), stdin)) {
            /* EOF or error; bail out gracefully */
            puts("\nInput error. Exiting.");
            exit(1);
        }

        // strip trailing newline
        buf[strcspn(buf, "\r\n")] = '\0';

        if (!is_integer(buf)) {
            printf("Enter an integer!\n");
            valid_input = 0;
        } else {
            value = (int)strtol(buf, NULL, 10);
            if (value >= 1 && value <= MENU_ITEMS) {
                valid_input = 1;
            } else {
                printf("Invalid menu item!\n");
                valid_input = 0;
            }
        }
    } while (!valid_input);

    return value;
}

static void select_menu_item(int input)
{
    switch (input) {
        case 1:
            buck_converter();
            //After the scanf(), \n still remains and will be put into fgets, make two go_back_to_main. 
            //Use a while loop to read that and empty the buffer.
            int flush_buck;
            while ((flush_buck = getchar()) != '\n' && flush_buck != EOF) {}
            go_back_to_main();
            break;
        case 2:
            boost_converter();
            int flush_boost;
            while ((flush_boost = getchar()) != '\n' && flush_boost != EOF) {}
            go_back_to_main();
            break;
        case 3:
            buck_boost_converter();
            int flush_buck_boost;
            while ((flush_buck_boost = getchar()) != '\n' && flush_buck_boost != EOF) {}
            go_back_to_main();
            break;
        case 4:
            cuk_converter();
            int flush_cuk;
            while ((flush_cuk = getchar()) != '\n' && flush_cuk != EOF) {}
            go_back_to_main();
            break;
        default:
            printf("Bye!\n");
            exit(0);
    }
}

static void print_main_menu(void)
{
    printf("\n----------- Main menu -----------\n");
    printf("\n"
           "\t1. Buck Converter\n"
           "\t2. Boost Converter\n"
           "\t3. Buck Boost Converter\n"
           "\t4. Cuk Converter\n"
           "\t5. Exit\n\n");
    printf("---------------------------------\n");
}

static void go_back_to_main(void)
{
    char buf[64];
    do {
        printf("\nEnter 'b' or 'B' to go back to main menu: ");
        if (!fgets(buf, sizeof(buf), stdin)) {
            puts("\nInput error. Exiting.");
            exit(1);
        }
        buf[strcspn(buf, "\r\n")] = '\0'; /* strip newline */
    } while (!(buf[0] == 'b' || buf[0] == 'B') || buf[1] != '\0');
}

/* Return 1 if s is an optional [+/-] followed by one-or-more digits, else 0. */
static int is_integer(const char *s)
{
    if (!s || !*s) return 0;

    /* optional sign */
    if (*s == '+' || *s == '-') s++;

    /* must have at least one digit */
    if (!isdigit((unsigned char)*s)) return 0;

    while (*s) {
        if (!isdigit((unsigned char)*s)) return 0;
        s++;
    }
    return 1;
}