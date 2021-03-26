#include <stdio.h>
#include <fcntl.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/types.h>
#include "shell_lib.h"

void prompt(char*, char*);
void single_command(InputLine*);
void multiple_commands(InputLine*);
int redirect_input(InputLine*);
void redirect_output(InputLine*);
void cd_command(InputLine*);

char *exec_cmd_url = "";

int main(int argc, char *argv[])
{
    if (argc == 4)
    {
        char buf[4096];
        InputLine *line;

        int STDIN = dup(0), STDOUT = dup(1);
        signal(SIGINT, SIG_IGN);
        signal(SIGQUIT, SIG_IGN);

        exec_cmd_url = strcat(argv[3], "/exec_cmd");

        prompt(argv[1], argv[2]);
        while (fgets(buf, 4096, stdin) != NULL)
        {
            if (!strcmp(buf, "EXIT\n") || !strcmp(buf, "exit\n"))
                exit(0);

            if (!strcmp(buf, "\n"))
            {
                prompt(argv[1], argv[2]);
                continue;
            }

            line = read_line(buf);

            if (line->in_redirect != NULL)
            {
                if (redirect_input(line) == FALSE)
                {
                    prompt(argv[1], argv[2]);
                    continue;
                }
            }

            if (line->out_redirect != NULL)
                redirect_output(line);

            if (line->ncommands == 1){
                if(strcmp(line->commands[0].argv[1], "cd") == 0)
                    cd_command(line);
                else
                    single_command(line);
            }
            else if (line->ncommands > 1)
                multiple_commands(line);

            if (line->in_redirect != NULL)
            {
                dup2(STDIN, 0);
            }

            if (line->out_redirect != NULL)
            {
                dup2(STDOUT, 1);
            }

            free_input_line(line);
            prompt(argv[1], argv[2]);
        }
    }
    else
    {
        printf("Número de parametros incorrectos");
        exit(-1);
    }
}

void single_command(InputLine *cmd_line)
{
    pid_t pid;
    int status;

    if ((pid = fork()) < 0)
    {
        perror("Error en llamada a fork");
    }
    else if (pid == 0)
    {
        execv(exec_cmd_url, cmd_line->commands[0].argv);
        perror("Minishell: orden no encontrada\n");
        exit(127);
    }

    if ((pid = waitpid(pid, &status, 0)) < 0)
    {
        perror("waitpid error");
    }
}

void multiple_commands(InputLine *cmd_line)
{
    int pipes[cmd_line->ncommands - 1][2];

    pipe(pipes[0]);

    int pid = fork();
    if (pid == 0)
    {
        close(pipes[0][0]);
        dup2(pipes[0][1], 1);
        execv(exec_cmd_url, cmd_line->commands[0].argv);
        perror("Minishell: orden no encontrada\n");
        exit(127);
    }

    if (cmd_line->ncommands > 2)
    {
        for (int i = 1; i < (cmd_line->ncommands - 1); i++)
        {
            pipe(pipes[i]);
        }
        for (int i = 1; i < (cmd_line->ncommands - 1); i++)
        {

            pid = fork();
            if (pid == 0)
            {
                close(pipes[i - 1][1]);
                close(pipes[i][0]);
                for (int j = 0; j < (cmd_line->ncommands - 1); j++)
                {
                    if (j != i && j != (i - 1))
                    {
                        close(pipes[j][1]);
                        close(pipes[j][0]);
                    }
                }
                dup2(pipes[i - 1][0], 0);
                dup2(pipes[i][1], 1);
                execv(exec_cmd_url, cmd_line->commands[i].argv);
                perror("Minishell: orden no encontrada\n");
                exit(127);
            }
        }
    }

    pid = fork();
    if (pid == 0)
    {
        close(pipes[cmd_line->ncommands - 2][1]);
        for (int j = 0; j < (cmd_line->ncommands - 2); j++)
        {
            close(pipes[j][1]);
            close(pipes[j][0]);
        }
        dup2(pipes[cmd_line->ncommands - 2][0], 0);
        execv(exec_cmd_url, cmd_line->commands[cmd_line->ncommands - 1].argv);
        perror("Minishell: orden no encontrada\n");
        exit(127);
    }

    for (int j = 0; j < (cmd_line->ncommands - 1); j++)
    {
        close(pipes[j][1]);
        close(pipes[j][0]);
    }
    for (int i = 0; i < cmd_line->ncommands; i++)
    {
        wait(NULL);
    }
}

void cd_command(InputLine *cmd_line)
{
    if (cmd_line->commands[0].argc == 1)
        chdir(getenv("HOME"));
    else if (cmd_line->commands[0].argc == 2)
    {
        if (opendir(cmd_line->commands[0].argv[2]))
        {
            chdir(cmd_line->commands[0].argv[2]);
        }
        else if (ENOENT == errno)
        {
            printf("Minishell: cd: %s: No existe el fichero o el directorio.\n", cmd_line->commands[0].argv[2]);
        }
    }
}

int redirect_input(InputLine *line)
{
    if (access(line->in_redirect, R_OK) != -1)
    {
        int fdi = open(line->in_redirect, O_CREAT | O_RDWR, 0666);
        dup2(fdi, 0);
        return TRUE;
    }
    else
    {
        printf("Error: %s: No existe el fichero o el directorio\n", line->in_redirect);
        return FALSE;
    }
}

void redirect_output(InputLine *line)
{
    if (line->out_redirect_type == SIMPLE)
    {
        int fdo = open(line->out_redirect, O_WRONLY | O_CREAT | O_TRUNC, 0644);
        dup2(fdo, 1);
    }
    else if (line->out_redirect_type == DOUBLE)
    {
        int fdo = open(line->out_redirect, O_WRONLY | O_CREAT | O_APPEND, 0644);
        dup2(fdo, 1);
    }
}

void prompt(char *user, char *nodename)
{
    char pwd[1024];
    getcwd(pwd, 1024);
    int l_home = strlen(getenv("HOME"));
    printf("\n\033[%s;%s;%sm%s %s", user_style, user_background, user_font, user, RESET_COLOR);
    printf("\033[%s;%sm%s%s", nodename_background, s1_font, SEPARATOR, RESET_COLOR);
    printf("\033[%s;%s;%sm%s %s", nodename_style, nodename_background, nodename_font, nodename, RESET_COLOR);
    printf("\033[%s;%sm%s%s", path_background, s2_font, SEPARATOR, RESET_COLOR);
    printf("\033[%s;%s;%sm ~%s/ %s", path_style, path_background, path_font, pwd + l_home, RESET_COLOR);
    printf("\033[%sm%s%s ", s3_font, SEPARATOR, RESET_COLOR);
}