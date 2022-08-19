#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <unistd.h>


#ifndef NARGS
#define NARGS 4
#endif

void ejecucion_comando(char **argumentos, char *comando, int contador);
void leer_argumentos(int contador, size_t n, char **argumentos, char *comando);
void ultima_ejecucion(char **argumentos, char *comando, int contador);

void
ultima_ejecucion(char **argumentos, char *comando, int contador)
{
	int j = fork();

	if (j < 0) {
		perror("Error en fork!\n");
		exit(-1);
	}

	if (j == 0) {
		argumentos[contador + 1] = NULL;

		if (execvp(comando, argumentos) == -1) {
			exit(-1);
		}  // la funcion execvp solo devuelve algo si tiene error

	} else {
		wait(NULL);
		exit(0);
	}
}

void
ejecucion_comando(char **argumentos, char *comando, int contador)
{
	int j = fork();

	if (j < 0) {
		perror("Error en fork!\n");
		exit(-1);
	}

	if (j == 0) {
		argumentos[NARGS + 1] = NULL;
		if (execvp(comando, argumentos) == -1) {
			exit(-1);
		}  // la funcion execvp solo devuelve algo si tiene error

	} else {
		contador = 0;
		wait(NULL);
	}
}


void
leer_argumentos(int contador, size_t n, char **argumentos, char *comando)
{
	while (contador < NARGS) {
		char *linea = NULL;
		ssize_t lectura = getline(
		        &linea,
		        &n,
		        stdin);  // getline devuelve el numero de caracteres leidos incluyendo el \n


		if (lectura == -1) {
			ultima_ejecucion(argumentos, comando, contador);
		}


		if (linea[lectura - 1] == '\n') {
			linea[lectura - 1] = '\0';  // elimino el \n
		}

		if (linea[0] == '/') {
			linea++;
		}

		contador++;
		argumentos[contador] = linea;
	}
	ejecucion_comando(argumentos, comando, contador);

	leer_argumentos(0, n, argumentos, comando);
}


int
main(int argc, char *argv[])
{
	size_t n = 0;
	char *argumentos[NARGS + 2];  // ya que tendra el comando , la cantidad de nargsy null al final
	char *comando = argv[1];
	argumentos[0] = comando;
	int contador = 0;

	leer_argumentos(contador, n, argumentos, comando);
}
