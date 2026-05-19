#include "plankamath.h"

#include <stdio.h>
#include <string.h>

static void print_pair(PM_PAIR pair)
{
    char buffer[64];
    pm_format(pair.value, buffer, sizeof(buffer));
    printf("%s, %d\n", buffer, pair.error);
}

int main(int argc, char **argv)
{
    char buffer[128];
    char number[64];

    if (argc < 2) {
        printf("PlankaMath C runner\n");
        printf("commands: compile demo tests guarded\n");
        return 0;
    }

    if (strcmp(argv[1], "compile") == 0) {
        if (pm_compile_project(buffer, sizeof(buffer)) == PM_OK) {
            printf("%s\n", buffer);
            return 0;
        }
        printf("%s\n", buffer);
        return 1;
    }

    if (strcmp(argv[1], "demo") == 0) {
        pm_format(pm_calculator_demo(), number, sizeof(number));
        printf("%s\n", number);
        return 0;
    }

    if (strcmp(argv[1], "tests") == 0) {
        printf("%d\n", pm_all_tests());
        return pm_all_tests() ? 0 : 1;
    }

    if (strcmp(argv[1], "guarded") == 0) {
        print_pair(pm_guarded_division_demo(84.0, 0.0));
        return 0;
    }

    printf("Unknown command: %s\n", argv[1]);
    return 1;
}
