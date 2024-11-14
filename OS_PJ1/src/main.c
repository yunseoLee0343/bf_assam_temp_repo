#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/wait.h>
#include <sys/ipc.h>
#include <sys/msg.h>

#include "timer.h"
#include "child_process.h"
#include "message_queue.h"
#include "scheduler.h"

int main()
{
    // Signal handler for SIGALRM
    signal(SIGALRM, alarm_handler);
    setup_timer(); // Set up timer to trigger SIGALRM every TIME_QUANTUM seconds

    // Create message queue
    key_t key = ftok("scheduler", 65);
    msgid = msgget(key, 0666 | IPC_CREAT);

    // Create children
    create_children(NUM_CHILDREN, msgid);

    // Wait for all child processes to terminate
    for (int i = 0; i < NUM_CHILDREN; i++)
    {
        wait(NULL);
    }

    // Remove message queue after work is done
    msgctl(msgid, IPC_RMID, NULL);

    return 0;
}
