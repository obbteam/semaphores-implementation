#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <time.h>
#include <unistd.h>

#define ROW 6             /* Size of the cart */
#define CART_INTEVAL 5    /* Timing of pushing the cart */
#define SINGLE_INTERVAL 2 /* Timing of adding a single person into a queue */
#define GROUP_INTERVAL 1  /* Timing of adding a group into a queue */
#define MAX_SINGLE 4
#define MAX_GROUP 5

sem_t MutexSingle, MutexGroup, MutexCart;
int cart[ROW];
int Idx = 0, SingleQueue = 0, GroupQueue = 0;

void *taddSinglePerson(void *ptr)
{
    while (1)
    {
        sleep(SINGLE_INTERVAL);
        sem_wait(&MutexSingle);
        if (SingleQueue >= MAX_SINGLE)
        {
            sem_post(&MutexSingle);
            continue;
        }
        ++SingleQueue;
        printf("[+] single joins – queue: %d\n", SingleQueue);
        sem_post(&MutexSingle);
    }
}

void *taddGroup(void *ptr)
{
    while (1)
    {
        sleep(GROUP_INTERVAL);
        sem_wait(&MutexGroup);
        if (GroupQueue >= MAX_GROUP)
        {
            sem_post(&MutexGroup);
            continue;
        }
        ++GroupQueue;
        printf("[+] group joins – queue: %d\n", GroupQueue);
        sem_post(&MutexGroup);
    }
}

void *tsingleRowPush(void *ptr)
{
    while (1)
    {
        sem_wait(&MutexCart);
        sem_wait(&MutexSingle);

        if (SingleQueue < 1)
        {
            sem_post(&MutexSingle);
            sem_post(&MutexCart);
            continue;
        }

        while (Idx < ROW && SingleQueue > 0)
        {
            --SingleQueue;
            cart[Idx++]++;
        }
        sem_post(&MutexSingle);
        sem_post(&MutexCart);
    }
}

void *tgroupRowPush(void *ptr)
{
    while (1)
    {
        sem_wait(&MutexCart);

        if (Idx != 0)
        { /* row not empty, try later */
            sem_post(&MutexCart);
            usleep(10000); /* 10 ms back-off is enough */
            continue;
        }

        sem_wait(&MutexGroup);

        for (int i = 0; i < 3; ++i)
        {

            int groupSize = rand() % 2 + 2; /* 2 or 3 seats */

            if (GroupQueue == 0 || Idx + groupSize > ROW)
                break;

            --GroupQueue;

            for (int k = 0; k < groupSize; ++k)
            {
                cart[Idx % ROW]++; /* keep inside 0–5 */
                Idx++;
            }
        }
        sem_post(&MutexGroup);
        sem_post(&MutexCart);
    }
}

void *tcartProcess(void *ptr)
{

    while (1)
    {
        sleep(CART_INTEVAL);
        sem_wait(&MutexCart);
        printf("===========Cart is leaving===========\n");
        for (int i = 0; i < ROW; ++i)
        {
            printf("[%d]", cart[i]);
        }
        printf("\n");
        printf(" singleQ=%d  groupQ=%d\n", SingleQueue, GroupQueue);
        for (int i = 0; i < ROW; ++i)
        {
            cart[i] = 0;
        }
        Idx = 0;
        sem_post(&MutexGroup);
        sem_post(&MutexCart);
    }
}

int main(int argc, char *argv[])
{
    pthread_t thread[5];

    srand(time(NULL));

    sem_init(&MutexSingle, 0, 1);
    sem_init(&MutexGroup, 0, 1);
    sem_init(&MutexCart, 0, 1);

    pthread_create(&thread[0], NULL, (void *)&taddSinglePerson, NULL);
    pthread_create(&thread[1], NULL, (void *)&taddGroup, NULL);
    pthread_create(&thread[2], NULL, (void *)&tgroupRowPush, NULL);
    pthread_create(&thread[3], NULL, (void *)&tsingleRowPush, NULL);
    pthread_create(&thread[4], NULL, (void *)&tcartProcess, NULL);

    for (int i = 0; i <= 4; i++)
    {
        pthread_join(thread[i], NULL);
    }

    sem_destroy(&MutexSingle);
    sem_destroy(&MutexGroup);
    sem_destroy(&MutexCart);

    return 0;
}