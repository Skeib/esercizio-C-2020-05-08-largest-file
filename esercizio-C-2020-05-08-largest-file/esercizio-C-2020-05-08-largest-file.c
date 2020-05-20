#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <dirent.h>
#include <errno.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>


// https://www.gnu.org/software/libc/manual/html_node/Simple-Directory-Lister.html

int MAX;

int check=0;

//struct stat *statbuf;

void check_dir();
void check_sub_dir();

//statbuf = calloc(1, sizeof(stat));
char * find_largest_file(char * directory_name, int explore_subdir, int * largest_file_size){


	char * name;

	name = malloc(256);

	if(explore_subdir == 0){
		check_dir(directory_name, name);
	}

	else{
		check_sub_dir(directory_name, name);
	}
	return name;

}

void check_dir(char * directory_name, char * name){

	DIR * dir_stream_ptr;
	dir_stream_ptr = opendir(directory_name);

	if (dir_stream_ptr == NULL) {
		printf("impossibile accedere alla DIR %s!\n", directory_name);
		return;
		//exit(EXIT_FAILURE);
	}

	struct dirent *ep;

	check =0;

	while ((ep = readdir(dir_stream_ptr)) != NULL) {

			if(ep->d_type == DT_REG){

				static FILE * pf;
				pf = fopen(ep->d_name, "r");

				if(pf != NULL){
					fseek(pf, 0, SEEK_END);

					if(MAX < ftell(pf)){	//valuto lunghezza del file(devo però poterci accedere in lettura)
						MAX = ftell(pf);
						memcpy(name, ep->d_name, 256);
					}
					check++; //valuto se ho potuto accedere ad almeno un file
					fclose(pf);
				}else{
					printf("impossibile accedere al FILE %s!\n", ep->d_name);
				}
			}

	}

	closedir(dir_stream_ptr);
}

void check_sub_dir(char * directory_name, char * name){

	DIR * dir_stream_ptr;
	dir_stream_ptr = opendir(directory_name);

	if (dir_stream_ptr == NULL) {
		printf("impossibile accedere alla DIR %s!\n", directory_name);

		exit(EXIT_FAILURE);
	}

	struct dirent *ep;

	while ((ep = readdir(dir_stream_ptr)) != NULL) {

			if(ep->d_type == DT_DIR){
				check_dir(ep->d_name, name);
			}

	}
	closedir(dir_stream_ptr);
}

int main(int argc, char *argv[]) {

	errno = 0;

// man 3 opendir
//    DIR *opendir(const char *name);
//    DIR *fdopendir(int fd);

	char * dirName;

	dirName = malloc(256);

	DIR * dir_stream_ptr;
	dir_stream_ptr = opendir(getcwd(NULL,0));

	if (dir_stream_ptr == NULL) {
		printf("impossibile accedere alla CWD %s!\n", getcwd(NULL,0));
		exit(EXIT_FAILURE);
	}

	struct dirent *ep;

	while ((ep = readdir(dir_stream_ptr)) != NULL) {

		if(ep->d_type == DT_REG){
			check++;
			}
		}
	if(check != 0){

		dirName	= find_largest_file(getcwd(NULL,0), 1, &MAX);

		//getcwd(NULL,0) con lo 0 adatta le dimensioni in base alla dimensione della stringa

		printf("Nella cartella: %s\n", getcwd(NULL,0));

		if(check != 0){

		printf("Il file più grande è %s e pesa : %d bytes\n", dirName, MAX);
		}
		else{
			printf("\nC'è stato un errore nell'accesso ai file, non è possibile valutare la dimensione\n");
		}

		return EXIT_SUCCESS;
	}
	else{
		printf("\nNella cartella %s non ci sono file di cui poter valutare le dimensioni\n", getcwd(NULL,0));
		return EXIT_FAILURE;
	}
}

