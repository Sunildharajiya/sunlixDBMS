#include <stdio.h>

#include "cli.h"

void cmd_help(void)
{
    printf("\nCommands:\n");

    printf(
        "  create <file> <json>              Create record\n"
    );

    printf(
        "  read <file>                       Read subfile\n"
    );

    printf(
        "  update <file> <key> <field> <value>  Update record\n"
    );

    printf(
        "  delete <file> <key>               Soft-delete record\n"
    );

    printf(
        "  delete-file <file>                Delete subfile\n"
    );

    printf(
        "  help                              Show commands\n"
    );

    printf(
        "  exit                              Exit DBMS\n\n"
    );
}