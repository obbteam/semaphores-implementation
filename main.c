#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <time.h>
#include <unistd.h>

#define ROW 6                 /* Size of the cart */
#define CART_INTEVAL 5        /* Timing of pushing the cart */
#define SINGLE_INTERVAL 2     /* Timing of adding a single person into a queue */
#define GROUP_INTERVAL 1      /* Timing of adding a group into a queue */
#define FRONT_GROUPS 3        /* Number of groups to pick from the front of the queue */
#define MAX_CARTS_W_SINGLES 2 /*Number of carts allowed to leave without singles*/

sem_t Mutex;
sem_t SingleQueue, GroupQueue;
int cart[ROW];
int idx = 0, notPushedGroup = 0, cartsWithoutSingles = 0;

void *taddSinglePerson(void *ptr)
{
    while (1)
    {
        sleep(SINGLE_INTERVAL);
        sem_wait(&Mutex);
        sem_post(&SingleQueue);
        int count;
        sem_getvalue(&SingleQueue, &count);
        printf("[+] single joins – singles queue: %d\n", count);
        sem_post(&Mutex);
    }
    pthread_exit(0);
}

void *taddGroup(void *ptr)
{
    while (1)
    {
        sleep(GROUP_INTERVAL);
        sem_wait(&Mutex);
        sem_post(&GroupQueue);
        int count;
        sem_getvalue(&GroupQueue, &count);
        printf("[+] group joins – groups queue: %d\n", count);
        sem_post(&Mutex);
    }
    pthread_exit(0);
}

/*@return number of groups pushed into the cart*/
static int pushGroupsIntoCart()
{
    int groupsPushed = 0;

    for (int i = 0; i < FRONT_GROUPS; ++i)
    {
        if (cartsWithoutSingles >= MAX_CARTS_W_SINGLES)
        {
            printf("======================%d carts without singles================\n\n", MAX_CARTS_W_SINGLES);
            break;
        }
        if (sem_trywait(&GroupQueue) != 0)
            break;

        // we check if there was not pushed group and if so we set the groupSize to it
        int groupSize = notPushedGroup ? notPushedGroup : rand() % 4 + 2;
        notPushedGroup = 0;

        if (groupSize + idx > ROW)
        {
            sem_post(&GroupQueue);
            printf("[/] group holds - group size: %d\n", groupSize);
            notPushedGroup = groupSize;
            break;
        }

        int count;
        sem_getvalue(&GroupQueue, &count);
        printf("[-] group leaves - group size: %d - group queue: %d\n", groupSize, count);

        for (int k = 0; k < groupSize; ++k)
        {
            cart[idx]++;
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
    while (idx < ROW)
    {
        if (sem_trywait(&SingleQueue) != 0)
            break;
        cart[idx++]++;
        int count = 0;
        sem_getvalue(&SingleQueue, &count);
        printf("[-] single leaves - single size: %d - single queue: %d\n", 1, count);
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
        printf("\n\n=============================================================\n");
        printf("======================Cart is departing======================\n\n");
        int groups = pushGroupsIntoCart();
        int singles = pushSinglesIntoCart();

        if (singles == 0)
            cartsWithoutSingles++;
        else
            cartsWithoutSingles = 0;

        if (groups == 0 && singles == 0)
        {
            printf("==========No groups or singles. Cart is not leaving==========\n");
            printf("=============================================================\n\n");
            sem_post(&Mutex);
            continue;
        }

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
        printf("\n\n======================Cart has left==========================\n");
        printf("=============================================================\n\n");
        sem_post(&Mutex);
    }
    pthread_exit(0);
}

int main(int argc, char *argv[])
{
    pthread_t thread[3];

    srand(time(NULL));

    sem_init(&Mutex, 0, 1);
    sem_init(&SingleQueue, 0, 0);
    sem_init(&GroupQueue, 0, 0);

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