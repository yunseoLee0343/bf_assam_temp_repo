#ifndef SCHEDULER_H
#define SCHEDULER_H

#define NUM_CHILDREN 10
#define TIME_QUANTUM 1

pid_t child_pids[NUM_CHILDREN];
int remaining_quantum[NUM_CHILDREN];
int current_child = 0;
int msgid;
int msgid;

#endif // SCHEDULER_H