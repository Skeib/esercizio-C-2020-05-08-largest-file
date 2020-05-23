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

#define MSG_REG


// restituisce la dimensione del file, -1 in caso di errore
__off_t get_file_size(char * file_name) {

	struct stat sb;
	int res;

	res = stat(file_name, &sb);

	if (res == -1) {
		perror("stat()");
		return -1;
	}

	return sb.st_size;
}


char * find_largest_file(char * directory_name, int explore_subdirectories_recursively,
		int * largest_file_size){

	char * file_name;
	file_name = malloc(256);

	DIR * dir_stream_ptr;
	struct dirent *ep;

	dir_stream_ptr = opendir(directory_name);

	if (dir_stream_ptr == NULL) {
		*largest_file_size = -1;
		return NULL;
	}
	int max=-1;

	while ((ep = readdir(dir_stream_ptr)) != NULL) {


#ifdef MSG_REG
		printf("%-10s ", (ep->d_type == DT_REG) ?  "regular" :
		                                    (ep->d_type == DT_DIR) ?  "directory" :
		                                    (ep->d_type == DT_FIFO) ? "FIFO" :
		                                    (ep->d_type == DT_SOCK) ? "socket" :
		                                    (ep->d_type == DT_LNK) ?  "symlink" :
		                                    (ep->d_type == DT_BLK) ?  "block dev" :
		                                    (ep->d_type == DT_CHR) ?  "char dev" : "???");

		printf("%s", ep->d_name);
#endif


		if (strcmp(".", ep->d_name)==0 || strcmp("..", ep->d_name)==0) {
					printf("\n");
					continue;//passo alla iterazione successiva
		}


		//confronto file nella singola directory
		if(ep->d_type == DT_REG){
			__off_t size;
			size = get_file_size(ep->d_name);

#ifdef MSG_REG
			printf("  size: %ld\n", size);
#endif

			if(max < size){
					max = size;
					strcpy(file_name, ep->d_name);
			}
		}

		else if(explore_subdirectories_recursively !=0 && ep->d_type == DT_DIR){//subdirectory

#ifdef MSG_REG
			printf("\n");
#endif

			int sub_max=-1;
			char * sub_file_name;
			sub_file_name = malloc(256);

			//valuto il massimo sulla singola directory e poi libero la memoria allocata

			sub_file_name = find_largest_file(ep->d_name, explore_subdirectories_recursively, &sub_max);

			if(max < sub_max){
					max = sub_max;
					strcpy(file_name, sub_file_name);
			}
			free(sub_file_name);
		}
	}

	if (errno) {
			perror("readdir() error");
		}

	closedir(dir_stream_ptr);

	if(max == -1){
		*largest_file_size = -1;
		return NULL;
	}else{

	*largest_file_size = max;
	return file_name;
	}

}


int main(int argc, char * argv[]) {

	char * result;
	int largest_file_size;

	char * file_name;

	if (argc == 1) {
		file_name = getcwd(NULL,0);
		printf("parametro: nome del file non inserito, procedo dalla cartella corrente : %s\n", file_name);
	}else{
		file_name = argv[1];
	}

	result = find_largest_file(file_name, 1, &largest_file_size);

	printf("result = %s\n", result);
	printf("size = %d\n", largest_file_size);

//	printf("pid: %d\n", getpid());


	return 0;
}
