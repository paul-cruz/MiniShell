#!/bin/bash

user=$(whoami)
nodename=$(uname -n)
current_dir=$(pwd)
echo "Compilando exec_cmd.c ..."
gcc exec_cmd.c -o exec_cmd
echo "exec_cmd.c Compilado con exito"

echo "Compilando shell.c ..."
gcc -Wall -Wextra shell.c shell_lib.c -o shell
echo "shell.c Compilado con exito"

echo "Iniciando shell ..."
sleep 1s
clear

./shell $user $nodename $current_dir