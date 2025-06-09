#include <stdio.h>
#include <pthread.h>
#include <semaphore.h>
#include <time.h>
#include <unistd.h>

#define ROW 6             /* Size of the cart */
#define CART_PUSH 5       /* Timing of pushing the cart */
#define SINGLE_ROW_PUSH 2 /* Timing of adding a single person into a queue */
#define GROUP_ROW_PUSH 1  /* Timing of adding a group into a queue */
#define MAX_SINGLE 100
#define MAX_GROUP 50

sem_t SinglePeople, GroupPeople, Spaces, Mutex;
int Cart[ROW];
int In = 0, Out = 0, SingleQueue = 0, GroupQueue = 0;

void *taddSinglePerson(void *ptr)
{
    while (SingleQueue <= MAX_SINGLE)
    {

        sem_wait(&SinglePeople);
        SingleQueue++;
        printf("Adding a person to the single queue.\n Single Queue: %d\n", SingleQueue);
        sem_post(&SinglePeople);
        usleep(1000000 * SINGLE_ROW_PUSH); /* 1 s */
    }
    pthread_exit(0);
}

void *taddGroup(void *ptr)
{
    while (GroupQueue <= MAX_GROUP)
    {
        sem_wait(&GroupPeople);
        GroupQueue++;
        printf("Adding a group to the group queue.\n Group Queue: %d\n", GroupQueue);
        sem_post(&GroupPeople);
        usleep(1000000 * GROUP_ROW_PUSH); /* 2 s */
    }
    pthread_exit(0);
}

void *tsingleRowPush(void *ptr)
{
    pthread_exit(0);
}

void *tgroupRowPush(void *ptr)
{
    pthread_exit(0);
}

void *tcartProcess(void *ptr)
{
    pthread_exit(0);
}

int main(int argc, char *argv[])
{
    pthread_t thread[5];

    sem_init(&Mutex, 0, 1);
    sem_init(&SinglePeople, 0, 1);
    sem_init(&GroupPeople, 0, 1);
    sem_init(&Spaces, 0, ROW);

    pthread_create(&thread[0], NULL, (void *)&taddSinglePerson, NULL);
    pthread_create(&thread[1], NULL, (void *)&taddGroup, NULL);
    pthread_create(&thread[2], NULL, (void *)&tgroupRowPush, NULL);
    pthread_create(&thread[3], NULL, (void *)&tsingleRowPush, NULL);
    pthread_create(&thread[4], NULL, (void *)&tcartProcess, NULL);

    for (int i = 0; i <= 4; i++)
    {
        pthread_join(thread[i], NULL);
    }

    sem_destroy(&SinglePeople);
    sem_destroy(&GroupPeople);
    sem_destroy(&Spaces);
    sem_destroy(&Mutex);

    return 0;
}