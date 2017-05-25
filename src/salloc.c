/* @author Enver Bashirov 21203023
CS342 Project 4
Static Memory Allocation Library*/

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>

#define MINSEG 32*1024		// 32KB	Min segment size
#define MAXSEG 32*1024*1024	// 32MB	Max segment Size
#define MINALL 65 			// 64B 	Min allocation size
#define MAXALL 64*1024		// 64KB Max allocation size
#define ADRLEN 8			// 8B 	Adress Length

#define allign8(x) (((((x)-1)>>3)<<3)+8);

#define BLOCK_SIZE (sizeof(struct s_block))

typedef struct s_block *t_block;

struct s_block {
	size_t size;
	struct s_block *next;
	int f;
};

/* Global Variables */
void *segmentptr; // points to the start of memory segment to manage
int   segmentsize;  // size of segment to manage
pthread_mutex_t mutex; // protection of global variables
//--------------------------------

//Prints information about a single block
void printBlock ( t_block b) {
    if ( b->f == 1)
        printf( "Block addr: %lx, size: %zu\t is free\n", ( unsigned long) b, b->size);
    else
        printf( "Block addr: %lx, size: %zu\t is allocated\n", ( unsigned long) b, b->size);
}//--------------------------------------

//Puts a block with the provided information
t_block insertBlock ( void * ptr, void * next, size_t s, int f){
	t_block b = ( t_block) ptr;
	b->size = s;
	b->f = f;
	if ( next == NULL) {
    	b->next = ( t_block) ((void *) b + (BLOCK_SIZE + s));
    	b->next->f = 1;
    	b->next->size = (size_t) ( (segmentptr + segmentsize) - ( ( void *) b->next + BLOCK_SIZE));
	} else { 
	    b->next = ( t_block) next;
	}
	return b;
}//----------------------------------------

//PROVIDED
int s_create (int size) {
	//Condition for MAX and MIN segment size
	if ( size < MINSEG || size > MAXSEG) {
		printf( "Invalid segment size! TERMINATING CREATE\n");
		return 0;
	}//-------------------------------------

	int i; 
	void *endptr; 
	char *cptr;
	void *vp; 
	
	segmentptr = sbrk(0);  // end of data segment                          
	segmentsize = size; 
	
	vp = sbrk(size); // extend data segment by indicated amount       
	if (vp == ((void *)-1)) {
		printf ("segment creation failed\n"); 
		return (-1); 
	}
	
	endptr = sbrk(0); 
	
	printf("segmentstart=%lx, segmentend=%lx, segmentsize=%lu bytes\n",
	       (unsigned long)segmentptr,
	       (unsigned long)endptr, (unsigned long)(endptr - segmentptr));

	//test the segment 
	printf("---starting testing segment\n");
	cptr = (char *)segmentptr;
	for (i = 0; i < size; ++i)
		cptr[i] = 0;
	printf("---segment test ended - success\n");

	pthread_mutex_lock( &mutex);
	insertBlock ( segmentptr, NULL, 0, 1);
	pthread_mutex_unlock( &mutex);

	return (0); 
}

//Finds the next sufficient block with provided size
t_block findBlock( size_t size) {
	t_block b = segmentptr;

	while ( b->next != NULL && !(b->f == 1 && b->size >= size) ){
		b = b->next;
	}
	return (b);
}//-------------------------------------------------

//Splits a given block into two with provided new size
void splitBlock( t_block b, size_t newsize) {
	t_block next =  ( t_block) (( void *) b + (BLOCK_SIZE + newsize));
	next->size = b->size - (BLOCK_SIZE + newsize);
	next->f = 1;
	next->next = ( t_block) b->next;
	b->size = newsize;
	b->next = ( t_block) next;
}//---------------------------------------------------

/*Memory managing function which is similar to malloc
With the given objectsize, "allocates" an unused memory form the segment*/
void *s_alloc(int objectsize) {
	//Check if objectsize obeys the allocation rules
	if ( objectsize < MINALL || objectsize > MAXALL) {
		printf( "s_alloc | Invalid object size! TERMINATING ALLOC\n");
		return NULL;
	}//---------------------------------------------

	t_block b = ( t_block) segmentptr;
	size_t s = ( size_t) allign8( objectsize);

	pthread_mutex_lock( &mutex);
	if ( b->f == 1)
		b = insertBlock ( segmentptr, NULL, s, 0);
	else {
		b = findBlock ( s);
		//If segment is full than exit
		if ( (int) (( ( void *) b + BLOCK_SIZE + s) - segmentptr) > segmentsize){
			printf( "s_alloc | Segment is full! %lx\n", (( ( void *) b + BLOCK_SIZE + s) - segmentptr));
			pthread_mutex_unlock( &mutex);
			return NULL;
		}
		if ( b->size > s && MINALL < (b->size - ((BLOCK_SIZE) + s)))
			splitBlock( b, s);
			
		b = insertBlock ( ( void *) b, ( void *) b->next, s, 0);
	}
	pthread_mutex_unlock( &mutex);
	
	return ( ( void *) (( unsigned long) b + ( unsigned long) BLOCK_SIZE));
}//-------------------------------------------------------------------------

//Merges the free segments if they are repeating
void mergeFree(){
	t_block b = ( t_block) segmentptr;
	t_block temp = b;
	while ( b->next) {
		if ( b->f == 1 && b->next->f == 1) {
			temp = b->next;
			b->size += (temp->size + BLOCK_SIZE);
			if ( temp->next)
				b->next = b->next->next;
			else 
				b->next = NULL;
		} else {
		    if ( b->next == NULL)
		        return;
			b = b->next;
		}
	}
}//--------------------------------------------

/*Memory managing function which is similar to free
if the given object is not free than "deallocates" the block*/
void s_free(void *objectptr) {
	t_block b = ( t_block) ( objectptr - BLOCK_SIZE);
	
	if ( b->size != 0 && b->f == 0) {
		pthread_mutex_lock( &mutex);
		b->f = 1;
		mergeFree();
		pthread_mutex_unlock( &mutex);
	}
	else {
		if ( b->f == 1)
			printf( "s_free | This block is already free!\n" );
		else	
			printf( "s_free | This block is already NULL!\n" );
		return;
	}
	return;
}//-----------------------------------------------------------

//Prints the holelist
void s_print(void) {
    pthread_mutex_lock( &mutex);
	t_block b = ( t_block) segmentptr;

	while ( b) {
		printBlock( b); b = b->next;
	}
    pthread_mutex_unlock( &mutex);
	return;
}//------------------

/* GARBAGE - Where all the non used functions are gathered
// void removeBlock( void * ptr) {
// 	t_block b = ( t_block) ( ptr);
// 	b->f = NULL;
// 	b->size = NULL;
// 	b->next = NULL;
// 	b = NULL;   
// }

// t_block insertChunk ( void * ptr, size_t s) {
// 	t_block b = findBlock ( s);
// 	if ( b->size > s && MINALL < (b->size - s)) {
// 		splitBlock( b, s);
// 		return insertBlock ( ptr, (void *) b->next, s, 0);
// 	}
// 	return insertBlock ( ptr, NULL, s, 0);
// }

// void * calcAddr( void * ptr, size_t s) {
// 	return ( void *) ( ptr + ( BLOCK_SIZE + s));
// }
*/