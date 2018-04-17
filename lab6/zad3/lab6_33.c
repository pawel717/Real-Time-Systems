
#include <stdio.h>
#include <sys/neutrino.h>
#include <sys/syspage.h>
#include <errno.h>
#include <pthread.h>
#include <stdlib.h>

struct sigevent key_event;
struct sigevent clock_event;
volatile unsigned counter = 0;
int period;
volatile int flag = 1;

const struct sigevent *clock_handler( void *area, int id )
{
	if(++counter == period)
	{
		counter = 0;
		InterruptMask(0, id);
		return &clock_event;
	}

    else
        return NULL;
}

const struct sigevent *key_handler( void *area, int id )
{

	if(flag == 1)
	{
		InterruptMask(1, id);
		return &key_event;
	}

	else
		return NULL;
}

void* clock_thread(void* arg);

int main(int argc, char *argv[])
{
    int i;
    int key_interrupt_id;

    srand(time(NULL));
    period = rand()%1000+100;
    period = 1;
	printf("period=%d, flag=%d\n", period, flag);

    pthread_create(NULL, NULL, clock_thread, NULL);

    // Request I/O privileges
    ThreadCtl( _NTO_TCTL_IO, 0 );

    // Initialize event structure
    key_event.sigev_notify = SIGEV_INTR;

    // Attach ISR vector
    key_interrupt_id = InterruptAttach( 1, &key_handler, NULL, 0, 0 );

    while(1)
    {
        // Wait for ISR to wake us up
        InterruptWait( 0, NULL );

        printf("Hit! %d\n", counter);

       // delay(500);
        InterruptUnmask(1, key_interrupt_id);

    }

    // Disconnect the ISR handler
    InterruptDetach(key_interrupt_id);

	return EXIT_SUCCESS;
}

void* clock_thread(void* arg)
{
	int clock_interrupt_id =0;

	clock_event.sigev_notify = SIGEV_INTR;

	ThreadCtl( _NTO_TCTL_IO, 0 );

	printf("clock_thread\n");

    clock_interrupt_id = InterruptAttach( 0, &clock_handler, NULL, 0, 0 );
	InterruptUnmask(0, clock_interrupt_id);

	while(1)
	{
		InterruptWait(0, NULL);

		period = rand()%5000+1000;

		flag = !flag;

		printf("period=%d, flag=%d\n", period, flag);

		InterruptUnmask(0, clock_interrupt_id);
	}

    // Disconnect the ISR handler
    InterruptDetach(clock_interrupt_id);

	return EXIT_SUCCESS;
}
