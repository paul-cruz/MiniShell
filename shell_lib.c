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

char *ltrim(char *str, const char *seps)
{
    size_t totrim;
    if (seps == NULL)
    {
        seps = "\t\n\v\f\r ";
    }
    totrim = strspn(str, seps);
    if (totrim > 0)
    {
        size_t len = strlen(str);
        if (totrim == len)
        {
            str[0] = '\0';
        }
        else
        {
            memmove(str, str + totrim, len + 1 - totrim);
        }
    }
    return str;
}

char *rtrim(char *str, const char *seps)
{
    int i;
    if (seps == NULL)
    {
        seps = "\t\n\v\f\r ";
    }
    i = strlen(str) - 1;
    while (i >= 0 && strchr(seps, str[i]) != NULL)
    {
        str[i] = '\0';
        i--;
    }
    return str;
}

char *trim(char *str, const char *seps)
{
    return ltrim(rtrim(str, seps), seps);
}

InputLine *read_line(char *str)
{
    InputLine *new_line = (InputLine *)malloc(sizeof(InputLine));
    if (new_line == NULL)
    {
        printf("Error in read_line");
        exit(-1);
    }
    else
    {
        new_line->in_redirect = NULL;
        new_line->out_redirect = NULL;
        new_line->in_redirect_type = -1;
        new_line->out_redirect_type = -1;
        new_line->ncommands = get_ndelim(str, '|');
        new_line->commands = calloc((new_line->ncommands), sizeof(Command));
        split_commands(new_line, str, new_line->ncommands);
    }
    return new_line;
}

void split_commands(InputLine *line, char *str, int n)
{
    int i;
    char **cmds = calloc(n + 1, sizeof(char *));
    split_cmd(cmds, str, "|");
    for (i = 0; i < n; i++)
    {
        cmds[i] = trim(cmds[i], NULL);
        if (i == n - 1)
            verify_out_redirect(line, cmds[i]);
        if (i == 0)
            verify_in_redirect(line, cmds[i]);
        line->commands[i].argc = get_ndelim(cmds[i], ' ');
        line->commands[i].argv = malloc((line->commands[i].argc + 2) * sizeof(char *));
        split_cmd(line->commands[i].argv, cmds[i], " ");
    }
}

int split_cmd(char **arr, char *str, char *delim)
{
    int i = 0, n = 0;
    char *ptr = strtok(str, delim);
    if (strcmp(delim, "|") && strcmp(delim, "<<") && strcmp(delim, "<") && strcmp(delim, ">") && strcmp(delim, ">>"))
        arr[i++] = "exec_cmd";
    while (ptr != NULL)
    {
        arr[i++] = ptr;
        ptr = strtok(NULL, delim);
        n++;
    }
    if (strcmp(delim, "<<") && strcmp(delim, "<") && strcmp(delim, ">") && strcmp(delim, ">>"))
        arr[i] = NULL;
    return n;
}

int get_ndelim(char str[], char delim)
{
    int init_size = strlen(str), n = 1;
    for (int i = 0; i < init_size; i++)
    {
        if (str[i] == delim)
            n++;
    }
    return n;
}

void verify_in_redirect(InputLine *line, char *cmd)
{
    char *command[2];
    if (strstr(cmd, "<<") != NULL)
        line->in_redirect_type = DOUBLE;
    else if (strstr(cmd, "<") != NULL)
        line->in_redirect_type = SIMPLE;
    else
        return;
    if (split_cmd(command, cmd, "<") > 1)
    {
        cmd = trim(command[0], NULL);
        line->in_redirect = trim(command[1], NULL);
    }
}

void verify_out_redirect(InputLine *line, char *cmd)
{
    char *command[2];
    if (strstr(cmd, ">>") != NULL)
        line->out_redirect_type = DOUBLE;
    else if (strstr(cmd, ">") != NULL)
        line->out_redirect_type = SIMPLE;
    else
        return;
    if (split_cmd(command, cmd, ">") > 1)
    {
        cmd = trim(command[0], NULL);
        line->out_redirect = trim(command[1], NULL);
    }
}

void free_input_line(InputLine *line)
{
    int i;
    if (line->commands != NULL)
    {
        for (i = 0; i < line->ncommands; i++)
        {
            free(line->commands[i].argv);
        }
        free(line->commands);
    }
    free(line);
}

