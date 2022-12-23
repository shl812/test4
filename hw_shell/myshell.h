#ifndef _MYSH_
#define _MYSH_

#define OUT_REDIR ">"
#define APPEND ">>"
#define IN_REDIR "<"
#define BACK_PRO "&"

void cmd_help();

bool cmd_cd(int token_count, char* tokens[]);

int tokenize(char* buf, char* delims, char* tokens[], int maxTokens);

bool run(char* line) ;

#endif
