/*
 * catsem.c
 *
 * 30-1-2003 : GWA : Stub functions created for CS161 Asst1.
 *
 * NB: Please use SEMAPHORES to solve the cat syncronization problem in 
 * this file.
 */

#include <types.h>
#include <lib.h>
#include <test.h>
#include <thread.h>
#include <synch.h>

#define NFOODBOWLS 2
#define NCATS 6
#define NMICE 2
#define SLEEPTIME 1

static struct semaphore* animal_done;
static struct semaphore* other_done;
static struct semaphore* mutex;
static struct semaphore* cats_queue;
static struct semaphore* mice_queue;
static volatile int all_dishes_available;
static volatile int cat_wait_count;
static volatile int no_other_eat;
static volatile int mice_wait_count;

/*
 * 
 * Function Definitions
 * 
 */


/*
 * catsem()
 *
 * Arguments:
 *      void * unusedpointer: currently unused.
 *      unsigned long catnumber: holds the cat identifier from 0 to NCATS - 1.
 *
 * Returns:
 *      nothing.
 *
 * Notes:
 *      Write and comment this function using semaphores.
 *
 */

static
void
catsem( void * unusedpointer,
	unsigned long catnumber)
{
	(void) unusedpointer;
	int first_cat_eat = 0;
	int another_cat_eat = 0;	

	P(mutex);
	if (all_dishes_available == 1) {
		all_dishes_available = 0;
		V(cats_queue);
	}
	cat_wait_count++;
	V(mutex);

	P(cats_queue);
	kprintf("Cat (%d) in the kitchen.\n", catnumber);

	if (no_other_eat == 1) {
		no_other_eat = 0;
		first_cat_eat = 1;
	}
	else {
		first_cat_eat = 0;
	}
	
	if (first_cat_eat == 1) {
		P(mutex);
		if (cat_wait_count > 1) {
			another_cat_eat = 1;
			V(cats_queue);
		}
		V(mutex);
	}	

	kprintf("Cat (%d) eating.\n", catnumber);
	clocksleep(SLEEPTIME);
	kprintf("Cat (%d) done eating.\n", catnumber);

	P(mutex);
	cat_wait_count--;
	V(mutex);

	if (first_cat_eat == 1) {
		if (another_cat_eat == 1) {
			P(other_done);
		}
		
		kprintf("First cat (%d) is leaving the kitchen.\n", catnumber);
		no_other_eat = 1;

		P(mutex);
		if (mice_wait_count > 0) {
			V(mice_queue);
		}
		else if (cat_wait_count > 0){
			V(cats_queue);
		}
		else {
			all_dishes_available = 1;
		}
		V(mutex);
	}
	else {
		kprintf("Other cat (%d) is leaving the kitchen.\n", catnumber);
		V(other_done);	
	}
	
	V(animal_done);
}
        

/*
 * mousesem()
 *
 * Arguments:
 *      void * unusedpointer: currently unused.
 *      unsigned long mousenumber: holds the mouse identifier from 0 to 
 *              NMICE - 1.
 *
 * Returns:
 *      nothing.
 *
 * Notes:
 *      Write and comment this function using semaphores.
 *
 */

static
void
mousesem(void * unusedpointer,
	unsigned long mousenumber)
{
	(void) unusedpointer;
 	int first_mouse_eat = 0;
	int another_mouse_eat = 0;	

	P(mutex);
	if (all_dishes_available == 1) {
		all_dishes_available = 0;
		V(mice_queue);
	}
	mice_wait_count++;
	V(mutex);

	P(mice_queue);
	kprintf("Mouse (%d) in the kitchen.\n", mousenumber);

	if (no_other_eat == 1) {
		no_other_eat = 0;
		first_mouse_eat = 1;
	}
	else {
		first_mouse_eat = 0;
	}
	
	if (first_mouse_eat == 1) {
		P(mutex);
		if (mice_wait_count > 1) {
			another_mouse_eat = 1;
			V(mice_queue);
		}
		V(mutex);
	}	
	
	kprintf("Mouse (%d) eating.\n", mousenumber);
	clocksleep(SLEEPTIME);
	kprintf("Mouse (%d) done eating.\n", mousenumber);

	P(mutex);
	mice_wait_count--;
	V(mutex);

	if (first_mouse_eat == 1) {
		if (another_mouse_eat == 1) {
			P(other_done);
		}
		
		kprintf("First mouse (%d) is leaving the kitchen.\n", mousenumber);
		no_other_eat = 1;

		P(mutex);
		if (cat_wait_count > 0) {
			V(cats_queue);
		}
		else if (mice_wait_count > 0){
			V(mice_queue);
		}
		else {
			all_dishes_available = 1;
		}
		V(mutex);
	}
	else {
		kprintf("Other mouse (%d) is leaving the kitchen.\n", mousenumber);
		V(other_done);	
	}
	
	V(animal_done);
}


/*
 * catmousesem()
 *
 * Arguments:
 *      int nargs: unused.
 *      char ** args: unused.
 *
 * Returns:
 *      0 on success.
 *
 * Notes:
 *      Driver code to start up catsem() and mousesem() threads.  Change this 
 *      code as necessary for your solution.
 */

int
catmousesem(int nargs,
            char ** args)
{
        int index, error;
   
        /*
         * Avoid unused variable warnings.
         */

        (void) nargs;
        (void) args;
	
	/*
 	 * Initialize all shared resources
 	 */
	
	all_dishes_available = 1;
	int totalAnimals = NCATS + NMICE;
	animal_done = sem_create("animal_done", 0);
	other_done = sem_create("other_done", 0);
	mutex = sem_create("mutex", 1);
	
	cats_queue = sem_create("cats_queue", 0); 
	cat_wait_count = 0;
	no_other_eat = 1; 

	mice_queue = sem_create("mice_queue", 0);
	mice_wait_count = 0;

        /*
         * Start NCATS catsem() threads.
         */

        for (index = 0; index < NCATS; index++) {
           
                error = thread_fork("catsem Thread", 
                                    NULL, 
                                    index, 
                                    catsem, 
                                    NULL
                                    );
                
                /*
                 * panic() on error.
                 */

                if (error) {
                 
                        panic("catsem: thread_fork failed: %s\n", 
                              strerror(error)
                              );
                }
        }
        
        /*
         * Start NMICE mousesem() threads.
         */

        for (index = 0; index < NMICE; index++) {
   
                error = thread_fork("mousesem Thread", 
                                    NULL, 
                                    index, 
                                    mousesem, 
                                    NULL
                                    );
                
                /*
                 * panic() on error.
                 */

                if (error) {
         
                        panic("mousesem: thread_fork failed: %s\n", 
                              strerror(error)
                              );
                }
        }
	
	for (index = 0; index < totalAnimals; index++) {
		P(animal_done);
	}

	kprintf("\nALL ANIMALS DONE\n");
        return 0;
}


/*
 * End of catsem.c
 */
