#include <stdio.h>

#include "cli.h"

void cmd_help(void)
{
    printf("\nCommands:\n");

    printf(
        "  create <file> <json>  Create data\n"
    );

    printf(
        "  read <file>           Read data\n"
    );

    printf(
        "  update <file> <key> <field> <value>  "
        "Update data\n"
    );

    printf(
        "  delete <file> <key>   Delete record\n"
    );

    printf(
        "  delete-file <file>    Delete file\n"
    );

    printf(
        "  help                  Show commands\n"
    );

    printf(
        "  exit                  Exit DBMS\n\n"
    );
}