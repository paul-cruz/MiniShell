#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

int main(int argc, char* argv[]){
    if (argc >= 2){
        char **ops = argv + 1;
        execvp(argv[1],ops);
        perror("Error al ejecutar comando");
        exit(127);
    }else{
        perror("Al menos un argumento esperado");
        return -1;
    }
    exit(0);
}