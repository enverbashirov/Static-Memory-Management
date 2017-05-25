#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include "salloc.h"

int main(int argc, char *argv[])
{
	int ret;
	int size;
	void *x1, *x2, *x3;	// object pointers

	if (argc != 2) {
		printf("usage: app <size in KB>\n");
		exit(1);
	}

	size = atoi(argv[1]);

	ret =  s_create (size * 1024); 
	if (ret == -1) {
		printf ("could not create segment\n"); 
		exit (1); 
	}

	x1 = s_alloc(600);
	x2 = s_alloc(4500);
	x3 = s_alloc(1300);

	s_free(x1);
	s_free(x2);
	s_free(x3);

	s_print();
	return 0;
}
/*#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include "salloc.h"


#define ARRNUM 3 // number of big arrays
#define ARRLEN 2048 // array size (how many item)

#define THRNUM 100 //number of threads ( for stress testing)
#define THR_ALSIZE 64 //array size  (how many item) allocated inside thread



void* task(void*);

int main() //(int argc, char *argv[])
{
	
	printf("app started --------------\n\r");
	
	int ret;
	int size; //segment size, in KB
	int *x[ARRNUM];	// object pointers
	pthread_t tid[THRNUM]; 
	int i,j;
	
	
	size = 32; //KB
	ret =  s_create(size * 1024);
	if (ret == -1) {
		printf ("could not create segment\n"); 
		exit(1); 
	}

	
	// 1-make and fill ARRNUM number of integer arrays, each by size of (4*ARRLEN) bytes.
	for( i = 0; i < ARRNUM; i++){
		x[i] = (int *) s_alloc(4*ARRLEN); // malloc(4*ARRLEN); //
		if (x[i] == NULL) {
			printf ("can't allocate big array\n"); 
			exit(1);
		}	
		for (j = 0; j < ARRLEN; j++)
			x[i][j] = (i * 1000000 + j);
	}
	s_print();
	
	//2- remove second array
	s_free(x[1]); // free(x[1]); //
	s_print();
	
	
	//3-make multiple alloc/free threads for stress testing
	for(i = 0; i < THRNUM; i++ ){
		pthread_create (&tid[i], NULL, task, NULL);
	}	
	for(i = 0; i < THRNUM; i++ ){
		pthread_join(tid[i], NULL);
	}
	s_print(); // shouldn't be any change 
	
	
	
	//4-read initial ARRNUM integer arrays, and check their values. 
	//  they shouldn't be corrupted.
	for( i = 0; i < ARRNUM; i++){
		if (i==1) continue; // jump over removed array
		for (j = 0; j < ARRLEN; j++){
			//printf(" %d:", (x[i]+j)); //address
			//printf(" %d -", *(x[i]+j)); //value
			if ( x[i][j] != (i * 1000000 + j) ){
				printf("app failed -------------- %d,%d \n\r", i, j); 
				exit(1);
			}
		}
		s_free(x[i]);// free(x[i]);//
	}	
	s_print();
	
	
	printf("app finished successfully --------------\n\r");	
	return 0;
}


void* task(void* arg){
	
	int *arr = (int *) s_alloc(4*THR_ALSIZE); // malloc(4*THR_ALSIZE); //
	if (arr == NULL) {
		printf ("can't allocate small array\n"); 
		pthread_exit(0); 
	}	
	
	int i;
	for( i = 0; i < THR_ALSIZE; i++){
		arr[i] = rand();
		//printf(" %d:", (arr+i) ); //address
		//printf(" %d -", arr[i] ); //value
	}
	
	s_free(arr);// free(arr); //
	
	//printf("  thread finished-\n\r");
	pthread_exit(0);
}*/