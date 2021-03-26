#ifndef _shell_lib_
#define _shell_lib_

#define SEPARATOR "\uE0B0"
#define RESET_COLOR "\x1b[0m"

//----------------------TEXT STYLE---------------------

#define NONE "0"
#define BOLD "1"
#define DIM "2"
#define ITALICS "3"
#define UNDERLINE "4"
#define INVERT "7"
#define HIDDEN "8"

//----------------------TEXT COLOR----------------------

#define BLACK_F "30"
#define RED_F "31"
#define GREEN_F "32"
#define YELLOW_F "33"
#define BLUE_F "34"
#define MAGENTA_F "35"
#define CYAN_F "36"
#define WHITE_F "37"

//-------------------BACKGORUND COLOR-------------------

#define BLACK_B "40"
#define RED_B "41"
#define GREEN_B "42"
#define YELLOW_B "43"
#define BLUE_B "44"
#define MAGENTA_B "45"
#define CYAN_B "46"
#define WHITE_B "47"
#define DEFAULT_B "49"

//-------------------PROMPT PREFERENCES --------------------
#define user_style BOLD
#define user_font WHITE_F
#define user_background BLUE_B
#define s1_font BLUE_F

#define nodename_style BOLD
#define nodename_font WHITE_F
#define nodename_background CYAN_B
#define s2_font CYAN_F

#define path_style BOLD
#define path_font BLACK_F
#define path_background WHITE_B
#define s3_font WHITE_F

#define SIMPLE 1
#define DOUBLE 2
#define TRUE 1
#define FALSE 0

typedef struct{
    int argc;
    char **argv;
} Command;

typedef struct{
    int ncommands;
    Command *commands;
    char *in_redirect;
    int in_redirect_type;
    char *out_redirect;
    int out_redirect_type;
} InputLine;

char *ltrim(char*, const char* );
char *rtrim(char*, const char*);
char *trim(char*, const char*);
int get_ndelim(char[], char );
int split_cmd(char **, char *, char *);
void verify_in_redirect(InputLine *, char *);
void verify_out_redirect(InputLine *, char *);
void split_commands(InputLine *, char *, int);
void free_input_line(InputLine *);
void cd_command(InputLine *);
InputLine *read_line(char *);

#endif