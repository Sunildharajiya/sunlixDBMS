# CLI
The SunlixDBMS CLI is the command interface used to interact with the database engine. It receives commands from the user, processes the input, and passes the required operation to the appropriate DBMS function.

# Available commands
| COMMANDS |   |
| --- | --- |
|`create <file> <json>` | Create data |
|`help`  | Show commands | 
|`exit`  |  Exit DBMS |

example :
```bash
~$/ ./sunlixDBMS

`Starting the main application...
Welcome to SunlixDBMS CLI!
Type 'help' for available commands.`

my_cli> create users.json {"name":"Sunil","age":15}

```

The CLI is responsible for input and command handling. It should not contain the actual database or file-management logic.