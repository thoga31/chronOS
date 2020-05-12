#include "processor.h"
#include "pcbmgr.h"

// file to do the 7 types of intructions

void changeValue (process * p, int n) {
    p->context = n;
}

void addValue(process * p, int n) {
    p->context += n;
}

void subtractValue(process * p, int n) {
    p->context -= n;
}

void blockProcess(process * p) {
    p->state = 'B';
    // find out what to do with a blocked process
    // maybe send it to the end of memory
}

void terminateProcess(process * p) {
    p->state = 'T';
}

void createNewProcess(process * p) {
    process *new_p    = malloc(sizeof(process));
    new_p->name       = p->name;
    new_p->start      = -1; //
    new_p->id         = p->id+1; //find better scheme to number child processes
    new_p->context    = 0;
    new_p->counter    = p->counter;
    new_p->pid        = p->id;
    new_p->priority   = p->priority;
    new_p->time_limit = p->time_limit;
    new_p->state      = STATUS_NEW;
    new_p->insNum     = p->insNum - p-> counter;

    //add new_p to the pcb
}

void cleanProgram(process * p, char * filename) {
    // Remove remaning instructions from memory
    // ChangeFileName(pcb, p, filename)
    // Load new file to memory
    return; 
}
