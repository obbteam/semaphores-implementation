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

sem_t Mutex;
int cart[ROW];
int idx = 0, singleQueue = 0, groupQueue = 0, notPushedGroup = 0;

void *taddSinglePerson(void *ptr)
{
    while (1)
    {
        sleep(SINGLE_INTERVAL);
        sem_wait(&Mutex);
        ++singleQueue;
        printf("[+] single joins – singles queue: %d\n", singleQueue);
        sem_post(&Mutex);
    }
}

void *taddGroup(void *ptr)
{
    while (1)
    {
        sleep(GROUP_INTERVAL);
        sem_wait(&Mutex);
        ++groupQueue;
        printf("[+] group joins – groups queue: %d\n", groupQueue);
        sem_post(&Mutex);
    }
}

/*@return number of groups pushed into the cart*/
static int pushGroupsIntoCart()
{
    int groupsPushed = 0;

    int front = groupQueue >= 3 ? 3 : groupQueue;
    for (int i = 0; i < front; ++i)
    {
        // we check if there was not pushed group and if so we set the groupSize to it
        int groupSize = notPushedGroup ? notPushedGroup : rand() % 2 + 2;
        notPushedGroup = 0;

        if (groupSize + idx > ROW)
        {
            notPushedGroup = groupSize;
            break;
        }

        --groupQueue;
        printf("[-] group leaves - group size: %d - group queue: %d\n", groupSize, groupQueue);

        for (int k = 0; k < groupSize; ++k)
        {
            cart[idx % ROW]++;
            idx++;
        }
        groupsPushed++;
    }
    return groupsPushed;
}

/*@return number of singles pushed into the cart*/
static int pushSinglesIntoCart()
{
    int singlesPushed = 0;
    while (idx < ROW && singleQueue > 0)
    {
        --singleQueue;
        cart[idx]++;
        printf("[-] single leaves - single size: %d - single queue: %d\n", 1, groupQueue);
        idx++;
        singlesPushed++;
    }
    return singlesPushed;
}

void *tcartProcess(void *ptr)
{

    while (1)
    {
        sleep(CART_INTEVAL);
        sem_wait(&Mutex);
        printf("\n\n===========Cart is departing===========\n\n");
        int groups = pushGroupsIntoCart();
        int singles = pushSinglesIntoCart();

        printf("Groups pushed: %d, singles pushed: %d\n", groups, singles);

        for (int i = 0; i < ROW; ++i)
        {
            printf("[%d]", cart[i]);
        }

        // flush the cart
        for (int i = 0; i < ROW; ++i)
        {
            cart[i] = 0;
        }
        idx = 0;
        printf("\n\n\n===========Cart has left===========\n\n");
        sem_post(&Mutex);
    }
}

int main(int argc, char *argv[])
{
    pthread_t thread[3];

    srand(time(NULL));

    sem_init(&Mutex, 0, 1);

    pthread_create(&thread[0], NULL, (void *)&taddSinglePerson, NULL);
    pthread_create(&thread[1], NULL, (void *)&taddGroup, NULL);
    pthread_create(&thread[2], NULL, (void *)&tcartProcess, NULL);

    for (int i = 0; i < 3; i++)
    {
        pthread_join(thread[i], NULL);
    }

    sem_destroy(&Mutex);

    return 0;
}