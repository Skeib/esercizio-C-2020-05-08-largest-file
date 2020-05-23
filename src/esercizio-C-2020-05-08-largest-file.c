#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <dirent.h>
#include <errno.h>
#include <string.h>

#include <unistd.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>


/*
esercizio:

ottenere l'elenco dei file di una directory: fare riferimento a questo esempio:
https://github.com/marcotessarotto/exOpSys/tree/master/023listFiles

parte 1 - trova il file regolare più grande cercandolo all'interno di una directory

parte 2 - trova il file regolare più grande cercandolo all'interno di una directory e
ricorsivamente in tutte le sue sotto-directory

scrivere la seguente funzione:

char * find_largest_file(char * directory_name, int explore_subdirectories_recursively,
int * largest_file_size);

la funzione restituisce il percorso completo del file regolare più grande cercato nella
directory specificata da directory_name.
se explore_subdirectories_recursively != 0, allora cerca ricorsivamente in tutte le sue sotto-directory.
(a parità di dimensioni, considera il primo trovato).

la dimensione del file regolare più grande viene scritta nella variabile il cui indirizzo è dato da
largest_file_size.
se non ci sono file regolari, la funzione restituisce NULL (e largest_file_size non viene utilizzato).


provare a fare girare il programma a partire da queste directory:

/home/utente

/
 */

#define DEBUG_MSG


// restituisce la dimensione del file, -1 in caso di errore
__off_t get_fd_size(int fd) {

	struct stat sb;
	int res;

	res = fstat(fd, &sb);

	if (res == -1) {
		perror("fstat()");
		return -1;
	}

    // printf("File size: %lld bytes\n", (long long) sb.st_size);

	//printf("sizeof(__off_t) = %lu\n", sizeof(__off_t));

    return sb.st_size;
}



char * find_largest_file_fd(int dir_fd, int explore_subdirectories_recursively,
		int * largest_file_size) {

	DIR * dir_stream_ptr;
	struct dirent *ep;

	dir_stream_ptr = fdopendir(dir_fd);

	if (dir_stream_ptr == NULL) {
		//printf("cannot open directory %s! bye", directory_name);
		*largest_file_size = -1;
		return NULL;
	}

	char * largest_file_name = malloc(256);
	int largest_file_len = -1;

	while ((ep = readdir(dir_stream_ptr)) != NULL) {

#ifdef DEBUG_MSG
		printf("%-10s ", (ep->d_type == DT_REG) ?  "regular" :
		                                    (ep->d_type == DT_DIR) ?  "directory" :
		                                    (ep->d_type == DT_FIFO) ? "FIFO" :
		                                    (ep->d_type == DT_SOCK) ? "socket" :
		                                    (ep->d_type == DT_LNK) ?  "symlink" :
		                                    (ep->d_type == DT_BLK) ?  "block dev" :
		                                    (ep->d_type == DT_CHR) ?  "char dev" : "???");

		printf("%s \n", ep->d_name);
#endif

		if (!strcmp(".", ep->d_name) || !strcmp("..", ep->d_name)) {
			continue;
		}

		// come trovo il file size? posso usare stat (man 2 stat)

		if (explore_subdirectories_recursively && ep->d_type == DT_DIR) { // directory

			int subdir_fd;
			char * sub_largest_file_name;
			int sub_largest_file_len;

			subdir_fd = openat(dir_fd, ep->d_name, O_RDONLY); // file descriptor della sub-directory

			if (subdir_fd == -1) {
				perror("openat");
				continue;
			}

			sub_largest_file_name = find_largest_file_fd(subdir_fd, explore_subdirectories_recursively, &sub_largest_file_len);

			if (sub_largest_file_name != NULL && sub_largest_file_len > largest_file_len) {
				strcpy(largest_file_name, sub_largest_file_name);
				largest_file_len = sub_largest_file_len;
			}

			free(sub_largest_file_name);

		} else if (ep->d_type == DT_REG) { // file regolare

			int file_fd;
			__off_t file_size;

			file_fd = openat(dir_fd, ep->d_name, O_PATH); // O_PATH è ok con fstat()

			file_size = get_fd_size(file_fd);

#ifdef DEBUG_MSG
			printf("%s size: %ld\n", ep->d_name, file_size);
#endif

			if (file_size > largest_file_len) {
				strcpy(largest_file_name, ep->d_name);
				largest_file_len = file_size;
			}

			close(file_fd);

		}
	}

	if (errno) {
		perror("readdir() error");
	}

	closedir(dir_stream_ptr);

	if (largest_file_len == -1) {
		*largest_file_size = -1;
		free(largest_file_name);
		return NULL;
	} else {

		*largest_file_size = largest_file_len;
		return largest_file_name;
	}
}

char * find_largest_file(char * directory_name, int explore_subdirectories_recursively,
		int * largest_file_size) {

	int dir_fd;

	dir_fd = open(directory_name, O_RDONLY);
	if (dir_fd == -1) {
		perror("open");
		printf("cannot open directory: %s\n", directory_name);
		exit(EXIT_FAILURE);
	}

	return find_largest_file_fd(dir_fd, explore_subdirectories_recursively, largest_file_size);
}


int main(int argc, char * argv[]) {

	char * result;
	int largest_file_size;

	char * file_name;

	if (argc == 1) {
		printf("parametro: nome del file");
		exit(EXIT_FAILURE);
	}

	file_name = argv[1];

	result = find_largest_file(file_name, 1, &largest_file_size);

	printf("result = %s\n", result);
	printf("size = %d\n", largest_file_size);

//	printf("pid: %d\n", getpid());


	return 0;
}
