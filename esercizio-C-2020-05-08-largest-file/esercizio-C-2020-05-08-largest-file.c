#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>

#include <errno.h>
#include <semaphore.h>
#include <pthread.h>
#include <dirent.h>

// https://www.gnu.org/software/libc/manual/html_node/Simple-Directory-Lister.html

__off_t MAX;

int check=0;

sem_t * semaforo;
sem_t * semaforo_dir;


//struct stat *statbuf;

void check_dir();
void check_sub_dir();

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

//statbuf = calloc(1, sizeof(stat));
char * find_largest_file(char * directory_name, int explore_subdir, __off_t * largest_file_size){

	char * name;

	name = malloc(256);

	if(explore_subdir == 0){

		check =0;
		check_dir(directory_name, name);
	}

	else{
		check =0;

		if (sem_wait(semaforo_dir) == -1) {
				perror("sem_wait");
				exit(EXIT_FAILURE);
		}

		check_dir(directory_name, name);

		if (sem_post(semaforo_dir) == -1) {
				perror("sem_post");
				exit(EXIT_FAILURE);
		}

		check_sub_dir(directory_name, name);
	}
	return name;

}

void check_dir(char * directory_name, char * name){

	DIR * dir_stream_ptr;
	dir_stream_ptr = opendir(directory_name);

	if (dir_stream_ptr == NULL) {
		printf("[check_dir] impossibile accedere alla DIR %s!\n", directory_name);
		//printf("%s", errno);
		return;
		//exit(EXIT_FAILURE);
	}
	//printf("accedo alla DIR %s!\n", directory_name);

	struct dirent *ep;

	while ((ep = readdir(dir_stream_ptr)) != NULL) {

			if(ep->d_type == DT_REG){
				char pathname[256];

				//strcpy(pathname, "/");
				strcpy(pathname, directory_name);
				strcat(pathname, "/");
				strcat(pathname, ep->d_name);

				if(MAX < get_file_size(pathname)){//valuto lunghezza del file(devo però poterci accedere in lettura)

						if (sem_wait(semaforo) == -1) {
								perror("sem_wait");
								exit(EXIT_FAILURE);
						}

						MAX = get_file_size(pathname);
						memcpy(name, pathname, 256);
						check++; //valuto se ho potuto accedere ad almeno un file

						if (sem_post(semaforo) == -1) {
								perror("sem_post");
								exit(EXIT_FAILURE);
						}

					}

				}else{
					//printf("impossibile accedere al FILE %s!\n", ep->d_name);
				}
		}

	closedir(dir_stream_ptr);
	}




void check_sub_dir(char * directory_name, char * name){

	DIR * dir_stream_ptr;
	dir_stream_ptr = opendir(directory_name);

	if (dir_stream_ptr == NULL) {
		printf("[check_sub_dir] impossibile accedere alla DIR %s!\n", directory_name);
		return;
		//exit(EXIT_FAILURE);
	}

	struct dirent *ep;
	//char dot[256] = "..";

	while ((ep = readdir(dir_stream_ptr)) != NULL) {

			if(ep->d_type == DT_DIR && *ep->d_name != '.' && ep->d_name[1] != '.'){

				if (sem_wait(semaforo_dir) == -1) {
						perror("sem_wait");
						exit(EXIT_FAILURE);
				}

				strcat(directory_name, "/");
				strcat(directory_name, ep->d_name);

				printf("\n%s\n\n", directory_name);

				check_dir(ep->d_name, name);

				if (sem_post(semaforo_dir) == -1) {
						perror("sem_post");
						exit(EXIT_FAILURE);
				}

				check_sub_dir(ep->d_name, name);

			}

	}
	closedir(dir_stream_ptr);
}

int main(int argc, char *argv[]) {

	errno = 0;

// man 3 opendir
//    DIR *opendir(const char *name);
//    DIR *fdopendir(int fd);
	semaforo_dir = malloc(sizeof(sem_t));
	sem_init(semaforo_dir, 1, 1);

	semaforo = malloc(sizeof(sem_t));

    sem_init(semaforo, 1, 1);

	char * dirName;

	dirName = malloc(256);

	DIR * dir_stream_ptr;
	dir_stream_ptr = opendir(getcwd(NULL,0));

	if (dir_stream_ptr == NULL) {
		printf("impossibile accedere alla CWD %s!\n", getcwd(NULL,0));
		exit(EXIT_FAILURE);
	}

	struct dirent *ep;
	check = 0;

	while ((ep = readdir(dir_stream_ptr)) != NULL) {

		if(ep->d_type == DT_REG){
			check++;
			}
		}
	if(check != 0){

		dirName	= find_largest_file(getcwd(NULL,0), 1, &MAX);

		//getcwd(NULL,0) con lo 0 adatta le dimensioni in base alla dimensione della stringa

		printf("\nNella cartella: %s\n", getcwd(NULL,0));

		if(check>0){

		printf("Il file più grande è %s e pesa : %ld bytes\n", dirName, MAX);
		}
		else{
			printf("\nC'è stato un errore nell'accesso ai file, non è possibile valutare la dimensione\n");
			printf("sarebbe %s e pesa : %ld bytes\n", dirName, MAX);
		}

		return EXIT_SUCCESS;
	}
	else{
		printf("\nNella cartella %s non ci sono file di cui poter valutare le dimensioni\n", getcwd(NULL,0));
		return EXIT_FAILURE;
	}
}

