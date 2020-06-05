/*TODO*/
#define _GNU_SOURCE 
#include <pthread.h>

pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER ;

typedef struct {
	//TODO
//	sem_t sem ;
	pthread_cond_t enqueue_cv ;
	pthread_cond_t dequeue_cv ;
//	int front ;
//	int rear ;
//	int capacity ;
//	int num ;

} mysem_t ;


void
mysem_init (mysem_t * sem, int init)
{
	//TODO
	
	pthread_cond_init(&(sem->enqueue_cv), &lock ) ;
	pthread_cond_init(&(sem->dequeue_cv), &lock) ;

}

void
mysem_post (mysem_t * sem)
{
	//TODO
	pthread_mutex_lock(&lock) ;
	pthread_cond_signal(&(sem->enqueue_cv)) ;
	pthread_cond_signal(&(sem->dequeue_cv)) ;
	pthread_mutex_unlock(&lock) ;
}

void
mysem_wait (mysem_t * sem)
{
	//TODO
	pthread_mutex_lock(&lock) ;
	pthread_cond_wait(&(sem->enqueue_cv), &(sem->lock)) ;
	pthread_cond_wait(&(sem->dequeue_cv), &(sem->lock)) ;
	pthread_mutex_unlock(&lock) ;
}
