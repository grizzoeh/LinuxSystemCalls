#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>


static void
pasaje_numeros(int fd_escritura, int i)
{
	if (write(fd_escritura, &i, sizeof(i)) < 0) {
		perror("error en write");
		exit(-1);
	}
}


static void
pasaje_inicial_padre(int fd_escritura, int n)
{
	int i = 2;
	while (i <= n) {
		pasaje_numeros(fd_escritura, i);

		i++;
	}
	// agrego condicion de corte en -1
	pasaje_numeros(fd_escritura, -1);
}


static void
recursiva(int fds_rec[2], int n)
{
	close(fds_rec[1]);

	int mensaje;


	if (read(fds_rec[0], &mensaje, sizeof(mensaje)) < 0) {
		perror("Error en read");
		exit(-1);
	}

	if (mensaje == -1) {
		close(fds_rec[0]);
		exit(0);  // condicion de corte
	}


	printf("primo %i\n", mensaje);


	// creo otro fds y pipe
	int fds2[2];
	int pipe2 = pipe(fds2);
	if (pipe2 < 0) {
		perror("Error en pipe");
		exit(-1);
	}


	int j = fork();

	if (j < 0) {
		printf("Error en fork!\n");
		exit(-1);
	}

	if (j == 0) {
		// Hijo
		close(fds_rec[0]);
		close(fds2[1]);
		recursiva(fds2, n);

	} else {
		// Padre
		close(fds2[0]);

		int mensaje_copy;

		while (read(fds_rec[0], &mensaje_copy, sizeof(int)) > 0) {
			if (mensaje_copy % mensaje != 0) {
				// chequeo que no sean multiplos
				pasaje_numeros(fds2[1], mensaje_copy);
			}
		}

		close(fds_rec[0]);
		close(fds2[1]);
		wait(NULL);
		exit(0);
	}
}


int
main(int argc, char *argv[])
{
	int n = atoi(argv[1]);

	int fds1[2];

	int pipe1 = pipe(fds1);
	if (pipe1 < 0) {
		perror("Error en pipe");
		exit(-1);
	}


	int i = fork();

	if (i < 0) {
		printf("Error en fork!\n");
		exit(-1);
	}

	if (i == 0) {
		// Hijo
		recursiva(fds1, n);


	} else {
		// Padre
		close(fds1[0]);
		pasaje_inicial_padre(fds1[1], n);
		close(fds1[1]);
		wait(NULL);  // espera a que termine la recursion
		exit(0);
	}
}
