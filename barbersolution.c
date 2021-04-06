/***********************************

  file: barber.c

  Sample solution to the Sleeping Barber Problem,
  5.14, pg. 189, Dinosaur book.

  The solution is similar to the bounded buffer problem with one
  extra semaphore to tell when the consumer (barber) is done with the
  buffer/process/customer.

***********************************/

#include "synch.h"
#include "minithread.h"
#include <stdio.h>
#include "mtio.h"

#define P(s) semaphore_P(s)
#define V(s) semaphore_V(s)

/* Number of chairs: 1 barber chair plus MAX_CHAIRS-1 waiting room chairs */
#define MAX_CHAIRS 2

/* Maximum number of customers that might come into the barber shop */
#define MAX_CUSTOMERS 20

#define TRUE 1
#define FALSE 0
#define NULL 0

/* random number generator function */
long random();

/* shared variable mutex */
semaphore_t mutex;

/* similar in use to the bounded buffer semaphores */
semaphore_t empty;
semaphore_t full;

/* customers V on done to wait until the barber is done cutting their hair */
semaphore_t done;

int customers_waiting;
int customer[MAX_CHAIRS];
int barber_index, customer_index;

void god_func(), barber_func(), customer_func();

main()
{
    /* set the random number generator seed */
    srandom(1693);

    /* start the simulation */
    god_func();
}

void god_func()
{
    int i, num_customers;
    minithread_t mt;

    /* when set to -1, the BARBER is waiting, not the CUSTOMERS */
    customers_waiting = -1;

    /* initial index into the customer number array */
    barber_index = customer_index = 0;

    mutex = semaphore_create();
    empty = semaphore_create();
    full = semaphore_create();
    done = semaphore_create();

    /* initially P-able 1 time */
    semaphore_initialize(mutex, 1);

    /* initially P-able MAX_CHAIRS times*/
    semaphore_initialize(empty, MAX_CHAIRS);

    /* NOT initially P-able */
    semaphore_initialize(full, 0);
    semaphore_initialize(done, 0);

    mt = minithread_fork((proc_t)barber_func, (arg_t)NULL);
    printf("barber = %p\n", mt);

    /* pick a random number of customers wanting to get their hair cut */
    num_customers = random() % MAX_CUSTOMERS;

    printf("And god created %d customers.\n", num_customers);
    printf("There are/is %d chair(s) in the customer waiting room.\n", MAX_CHAIRS - 1);

    for (i = 0; i < num_customers; i++)
    {
        mt = minithread_fork((proc_t)customer_func, (arg_t)i);
        printf("customer %d = %p\n", i, mt);
        printf("Forking customer %d..\n", i);
        fflush(stdout);
        /* wait a random amount [0-5] of "time slices" until the next
       customer starts */
        random_stall(6);
    }

    printf("leaving GOD FUNC\n");
    fflush(stdout);

    _exit(0);
}

void barber_func()
{
    printf("BARBER STARTED\n");
    fflush(stdout);

    while (TRUE)
    {
        P(full);
        P(mutex);
        printf("Barbering customer %d.\n", customer[barber_index % MAX_CHAIRS]);
        fflush(stdout);
        V(mutex);
        barber_index++;
        V(empty);
        /* use a random amount [0-6] of "time slices" to cut the customer's hair */
        random_stall(7);

        /* signal to the customer that he is done and may leave/exit */
        V(done);
    }
}

void customer_func(i) int i;
{
    printf("Customer %d begin. TID = %p\n", i, minithread_self());
    fflush(stdout);

    if (customers_waiting == MAX_CHAIRS - 1)
        printf("** Customer %d leaving.\n", i);
    else
    {
        customers_waiting++;
        printf("Customer %d waiting. Total waiting: %d\n", i, customers_waiting);
        P(empty);
        P(mutex);
        customer[customer_index % MAX_CHAIRS] = i;
        printf("In barber seat...%i \n", i);
        V(mutex);
        V(full);
        customer_index++;
        /* Wait for the barber to get done cutting this customer's hair */
        P(done);
        printf("A satisfied Customer %i leaving...\n", i);

        customers_waiting--;
    }
}

/* This function performs a random number (from 0 to time - 1 yields)
   of minithread_yield()'s to simulate time progressing */

random_stall(time) int time;
{
    int i;
    int length = random() % time;

    for (i = 0; i <= length; i++)
        minithread_yield();
}