
#include <unistd.h> // zawiera pause i sleep
#include <stdlib.h>
#include <string.h>
#include <time.h> // srand
#include <pthread.h>
#include <stdio.h>
#include <process.h>
#include <math.h>

#define NUMBER_OF_FRAMES 4
#define HYPER_PERIOD 10

typedef struct task_t{
	char name[50];
	struct task_t * next;
} task_t;

void add_task(char * name, task_t ** task)
{
	if(*task == NULL)
	{
		*task = malloc(sizeof(task_t));
		strncpy((*task)->name, name, 50);
		(*task)->next = NULL;
		return;
	}

	task_t *current = *task;
	while(current->next != NULL)
		current = current->next;

	current->next = malloc(sizeof(task_t));
	strncpy(current->next->name, name, 50);
	current->next->next = NULL;
}

void init_task(task_t * task)
{
	strcpy(task->name, "");
	task->next = NULL;
}

task_t frames[NUMBER_OF_FRAMES];
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
int current_index = 0;

void interruption_handler(union sigval sigvalue);

int main()
{
	srand(time(NULL));
	char name[] = "lab5_3_proces";

	timer_t timer;
	struct sigevent event;
	struct itimerspec itime;

	int i;
	for(i=0; i<NUMBER_OF_FRAMES; i++)
	{
		task_t * task = NULL;
		add_task(name, &task);
		frames[i] = *task;
	}

	SIGEV_THREAD_INIT(&event, interruption_handler, i, NULL);

	timer_create(CLOCK_REALTIME, &event, &timer);

	double intpart;
	double fractpart;
	fractpart = modf (HYPER_PERIOD / (double)NUMBER_OF_FRAMES , &intpart);
	printf("intpart=%d, fraqpart=%d\n", (int)intpart, (int) (fractpart*1000000000));
	itime.it_value.tv_sec = (int) intpart;
	itime.it_value.tv_nsec = (int) (fractpart*1000000000);
	itime.it_interval.tv_sec = (int) intpart;
	itime.it_interval.tv_nsec = (int) (fractpart*1000000000);

	timer_settime(timer, 0, &itime, NULL);

	while(1)
	{
	}

	return EXIT_SUCCESS;
}

void interruption_handler(union sigval sigvalue)
{
	int status;
	task_t * task = &frames[current_index];
	printf("frame nr %d\n", current_index);
	while(task != NULL)
	{
		int pid = spawnl(P_NOWAIT, task->name, frames->name, NULL);
		waitpid(pid, &status, WEXITED);
		task = task->next;
	}

	current_index = (current_index+1) % NUMBER_OF_FRAMES;
}

