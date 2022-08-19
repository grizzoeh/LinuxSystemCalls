#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
/*
Como las tuberías —pipes— son unidireccionales,
se necesitarán dos para poder transmitir el valor en una dirección
y en otra (ping-pong)
*/


static void
escritura(int fd_escritura, int mensaje)
{
	if (write(fd_escritura, &mensaje, sizeof(mensaje)) < 0) {
		perror("error en write");
		exit(-1);
	}
}


int
main(void)
{
	int fds1[2];
	int fds2[2];


	int pipe_pah = pipe(fds1);
	if (pipe_pah < 0) {
		perror("Error en pipe padre a hijo");
		exit(-1);
	}

	int pipe_hap = pipe(fds2);
	if (pipe_hap < 0) {
		perror("Error en pipe hijo a padre");
		exit(-1);
	}


	printf("Hola, soy PID <%d>:\n", getpid());

	printf("- primer pipe me devuelve: [%i, %i]\n", fds1[0], fds1[1]);

	printf("- segundo pipe me devuelve: [%i, %i]\n", fds2[0], fds2[1]);

	printf("\n");


	int i = fork();

	if (i > 0) {
		// Padre

		srandom(time(NULL));

		int mensaje_aleatorio = random() % 10;

		// El padre no va a leer en la ida y no escribira en la vuelta
		close(fds1[0]);
		close(fds2[1]);

		escritura(fds1[1], mensaje_aleatorio);


		printf("Donde fork me devuelve <%i>:\n", i);

		printf("  - getpid me devuelve: <%d>\n", getpid());

		printf("  - getppid me devuelve: <%d>\n", getppid());

		printf("  - random me devuelve: <%i>\n", mensaje_aleatorio);

		printf("  - envío valor <%i> a través de fd=%i\n",
		       mensaje_aleatorio,
		       fds1[1]);

		printf("\n");


		int mensaje_recibido_por_padre;

		if (read(fds2[0],
		         &mensaje_recibido_por_padre,
		         sizeof(mensaje_recibido_por_padre)) < 0) {
			// hace el read y chequea si falla al mismo tiempo
			perror("Error en read del padre");
			exit(-1);
		}


		printf("Hola, de nuevo PID <%d>:\n", getpid());
		printf("  - recibí valor <%i> vía fd=%i\n",
		       mensaje_recibido_por_padre,
		       fds2[0]);

		// cierro file descriptors que ya no uso para evitar leaks
		close(fds1[1]);
		close(fds2[0]);

		wait(NULL);  // espero a que termine el hijo
		exit(0);


	}

	else if (i == 0) {
		// El hijo no va a escribir en la ida y no leera en la vuelta
		close(fds1[1]);
		close(fds2[0]);

		int mensaje_recibido_por_hijo;


		if (read(fds1[0],
		         &mensaje_recibido_por_hijo,
		         sizeof(mensaje_recibido_por_hijo)) < 0) {
			// hace el read y chequea si falla al mismo tiempo
			perror("Error en read del hijo");
			exit(-1);
		}


		printf("Donde fork me devuelve <%i>:\n", i);

		printf("  - getpid me devuelve: <%d>\n", getpid());

		printf("  - getppid me devuelve: <%d>\n", getppid());

		printf("  - recibo valor <%i> vía fd=%i\n",
		       mensaje_recibido_por_hijo,
		       fds1[0]);

		printf("  - reenvío valor en fd=%i y termino\n", fds2[1]);

		printf("\n");

		escritura(fds2[1], mensaje_recibido_por_hijo);


		// cierro file descriptors que ya no uso para evitar leaks
		close(fds1[0]);
		close(fds2[1]);
		exit(0);


	} else {
		perror("Error en fork");
		exit(-1);
	}
}
