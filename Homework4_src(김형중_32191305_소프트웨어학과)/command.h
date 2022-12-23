#ifndef _COMMAND_
#define _COMMAND_

int set_environment_path(char*** arr);
char* get_cmd_path(char **env_path, char* command[]);
int execute(char* command, char** env);

#endif
