#define _GNU_SOURCE
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdbool.h>
#include <dirent.h>
#include <errno.h>
#include <linux/limits.h>


static bool
evita_directorios_especiales(char *nombre_dir)
{
	return (!((strcmp(nombre_dir, ".") == 0)) &&
	        !((strcmp(nombre_dir, "..") == 0)));
}

static DIR *
obtener_prox_dir(DIR *dir_actual, char *nombre_actual)
{
	// Para abrir sub-directorios, usar exclusivamente  el flag O_DIRECTORY
	// como precaución. De esta manera, no es necesario realizar
	// concatenación de cadenas para abrir subdirectorios.

	int next_dir_fd = openat(dirfd(dir_actual), nombre_actual, O_DIRECTORY);
	if (next_dir_fd == -1) {
		perror("Error fdopendir");
		exit(-1);
	}

	DIR *new_dir = fdopendir(next_dir_fd);

	if (new_dir == NULL) {
		perror("Error fdopendir");
		exit(-1);
	}

	return new_dir;
}


static void
find(DIR *directorio_actual,
     char *cadena,
     char *(*funcion_comparadora)(const char *, const char *),
     char *buffer_path,
     int posicion_path)
{
	struct dirent *entry_dir;


	while ((entry_dir = readdir(directorio_actual))) {
		char *nombre = entry_dir->d_name;

		if (funcion_comparadora(nombre, cadena) != NULL) {
			printf("%s%s\n", buffer_path, nombre);
		}


		if (entry_dir->d_type == DT_DIR &&
		    evita_directorios_especiales(nombre)) {
			// En los directorios voy a tener que ir actualizando el path

			int longitud = strlen(nombre);
			posicion_path += longitud;

			buffer_path = strcat(buffer_path, nombre);
			buffer_path = strcat(buffer_path, "/");

			DIR *prox_dir =
			        obtener_prox_dir(directorio_actual, nombre);

			find(prox_dir,
			     cadena,
			     funcion_comparadora,
			     buffer_path,
			     posicion_path);

			posicion_path -= longitud;
			buffer_path[posicion_path + 1] = '\0';

			if (strlen(buffer_path) == 1 && longitud != 1) {
				buffer_path[0] = '\0';
			}
		}
	}

	closedir(directorio_actual);
}

int
main(int argc, char *argv[])
{
	DIR *directorio = opendir(".");

	char *param1 = argv[1];

	char buffer_path[PATH_MAX] = "\0";

	int posicion_path = 0;

	if (strcmp(param1, "-i") == 0) {
		char *param2 = argv[2];
		find(directorio, param2, strcasestr, buffer_path, posicion_path);
	} else if (strcmp(param1, "-i") != 0) {
		find(directorio, param1, strstr, buffer_path, posicion_path);
	} else {
		perror("Error en comando");
		exit(-1);
	}
}