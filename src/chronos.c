//------------------------------------------------------------------------------
//
// chronOS - A scheduling simulator
//
//    Copyright (C) 2020 Igor Cordeiro Bordalo Nunes (www.igornunes.com)
//    Copyright (C) 2020 Jorge Miguel Louro Pissarra
//    Copyright (C) 2020 Diogo Castanheira Simões
//    Copyright (C) 2020 Universidade da Beira Interior (www.ubi.pt)
//
//  	This program is free software: you can redistribute it and/or modify
//  	it under the terms of the GNU General Public License as published by
//  	the Free Software Foundation, either version 3 of the License, or
//  	(at your option) any later version.
//
//	This program is distributed in the hope that it will be useful,
//	but WITHOUT ANY WARRANTY; without even the implied warranty of
//	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//	GNU General Public License for more details.
//
//	You should have received a copy of the GNU General Public License
//	along with this program.  If not, see <http://www.gnu.org/licenses/>.
//
//	Contacts:
//			Igor Nunes     <igor.bordalo.nunes@ubi.pt>
//			Jorge Pissarra <jorge.pissarra@ubi.pt>
//          Diogo Simões   <diogo.c.simoes@ubi.pt>
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// DESCRIPTION:
// -----------
// This program is an operating system scheduling simulator. It is built around
// two main components:
//     * A process simulator;
//     * A memory manager.
// It is able to use different algorithms to schedule processes.
// The processes consist of changing an unique integer variable.
// The memory manager uses a first-fit algorithm.
//
// SYNOPSIS:
// --------
// 	chronos [< control.txt]
//
// where:
//     'control.txt' is an optional text file to manipulate the process manager.
//
// FILE FORMAT:
// -----------
// Described at 'plan.h'
//
// LANGUAGE AND CURRENT VERSION:
// ----------------------------
// English, no release version yet
//------------------------------------------------------------------------------


#include "simulator.h"
#include "scheduling.h"
#include "tui.h"
#include "world.h"


int main(int argc, char const *argv[]) {
    // struct world w;
    startworld(&w);

    printf("===== %s %s =====\n\n", w.app.name, w.app.version);
    write("Initializing...\n");

    loadargs(&w, argc, argv);

    // Alocar memória heap
    debug("Allocating %d KB of heap memory for 4 algorithms\n", w.heap.capacity * w.heap.blocksize / 1024);
    heap_first = makeheap(w.heap.capacity);
    heap_next  = makeheap(w.heap.capacity);
    heap_best  = makeheap(w.heap.capacity);
    heap_worst = makeheap(w.heap.capacity);

    // MODO ESPECIAL: apenas solicitações de memória
    if (w.heap.shouldrequest == HEAP_REQUEST_EXCLUSIVE) {
        heaprequest_exclusive();
        heapfree(w.pid);
        destroyheap(heap_first);
        destroyheap(heap_next);
        destroyheap(heap_best);
        destroyheap(heap_worst);
        exit(EXIT_SUCCESS);
    }

    // Alocar células de memória 
    debug("Allocating %d cells of memory\n", w.memory.capacity);
    memory = memcreate(w.memory.capacity);

    // Inicializar tabela PCB
    debug("Allocating %d lines of PCB table\n", w.pcb.size);
    pcb = pcballoc(w.pcb.size);

    // Inicializar queue de plano com o ficheiro plan.txt
    debug("Reading plan queue from %s:\n", w.fileplan);
    plan = plan_read_from_file(w.fileplan);
    debug("Got %d elements in queue.\n", plan_length(plan));
    write("Got %d elements in plan queue.\n", plan_length(plan));

    // Obter instruções de controlo
    if (w.control.controller == CONTROLLER_FILE) {
        debug("Reading control queue from %s:\n", w.control.file);
        control = control_read_from_file(w.control.file);
        if (control != NULL) {
            debug("Got %d elements in queue.\n", control_length(control));
            write("Got %d elements in control queue.\n", control_length(control));
        } else {
            debug("Forcefully setting control to AUTO.\n");
            write("Forcefully setting control to automatic.\n");
            w.control.controller = CONTROLLER_AUTO;
        }
    }

    clock_t clock_start, clock_end;
    float seconds;

    heaprequest_start(w.heap.requestseed);

    /* Ciclo principal do programa */
    while (w.flag.__running) {
        if (!plan_empty(plan)) {
            if (plan_peek(plan).time <= w.cputime) {
                debug("cputime = %ld; plan_peek.time = %ld\n", w.cputime, plan_peek(plan).time);
                write("Creating new process from \"%s\" at CPU time %ld\n", plan_peek(plan).program, w.cputime);
                create_new_process(pcb, plan_pop(plan));
                w.flag.__mustexit = 0;
            }
        }
        
        /* Gestão de processos */
        if (!w.flag.__mustexit) {
            if (w.heap.shouldrequest) {
                switch (heaprequest()) {
                    case 0:
                        break;
                    case 1: {
                        int size = heaprequest_size();
                        int ret = heapalloc(w.pid, size);
                        debug("Random request from chronOS of %d blocks of heap memory (return code = %d)\n", size, ret);
                        write("Random request from chronOS of %d blocks of heap memory (return code = %d)\n", size, ret);
                        break;
                    }
                    default:
                        debug("Deallocating heap mempry from chronOS.\n");
                        write("Deallocating heap mempry from chronOS.\n");
                        heapfree(w.pid);
                        break;
                }
            }

            if (w.pcb.index == SCHEDULER_END)
                w.pcb.index = 0;

            switch (w.control.controller) {
                case CONTROLLER_AUTO:
                    // void
                    break;

                case CONTROLLER_STDIN:
                    if (w.control.fetch)
                        w.control.currentoperation = control_scan();
                    w.control.fetch = execcontrol(w.control.currentoperation, pcb, w.pcb.index);
                    break;

                case CONTROLLER_FILE:
                    if (w.control.fetch)
                        w.control.currentoperation = control_pop(control);
                    w.control.fetch = execcontrol(w.control.currentoperation, pcb, w.pcb.index);
                    break;

                default:
                    fprintf(stderr, "ERROR: Unknown controller. ABORTING!\n");
                    exit(EXIT_FAILURE);
            }

            // Is it still running?
            if (!w.flag.__running)
                break;

            switch (w.pcb.algorithm) {
                case SCHEDULING_FCFS:
                    w.pcb.index = fcfs(pcb, memory, w.pcb.index);
                    break;
                
                case SCHEDULING_SJF:
                    w.pcb.index = sjf(pcb, memory, w.pcb.index);
                    break;
                
                case SCHEDULING_RROBIN:
                    w.pcb.index = rrobin(pcb, memory, w.pcb.index);
                    w.pcb.rr_time++;
                    break;

                case SCHEDULING_PSA:
                    w.pcb.index = psa(pcb, memory, w.pcb.index);
                    break;
                
                default:
                    fprintf(stderr, "ERROR: Unknown scheduling algorithm. ABORTING!\n");
                    exit(EXIT_FAILURE);
            }

            if (w.pcb.index == SCHEDULER_END) {
                debug("Reached SCHEDULER_END.\n");
                write("Reached end of plan queue.\n");
                w.flag.__mustexit = 1;
            }
        }
        
        /* Clock do processador */
        clock_start = clock();
        while (1) {
            clock_end = clock();
            seconds = (float)(clock_end - clock_start) / CLOCKS_PER_SEC;
            if (seconds >= w.timequantum) {
                w.cputime++;
                break;
            }
        }

        // Verificar condições nas quais o simulador deve terminar
        if (plan_empty(plan) && w.flag.__mustexit)
            w.flag.__running = 0;
    }

    // Dealocação de memória heap do chronOS
    debug("Deallocating heap memory requested by chronOS...\n");
    write("Deallocating heap memory requested by chronOS...\n");
    heapfree(w.pid);  // para evitar memory leaks na heap memory por parte do chronOS

    write("Reached end of execution. Printing final reports...\n\n");

    // Relatórios e estatísticas
    pcbreport(pcb);
    memreport(memory);
    heapreport(heap_first, heap_next, heap_best, heap_worst);
    heapdump(heap_first, "first-fit");
    heapdump(heap_next,  "next-fit");
    heapdump(heap_best,  "best-fit");
    heapdump(heap_worst, "worst-fit");

    write("Finalizing...\n");

    // Libertar queue de plano
    debug("Freeing plan queue\n");
    plan_free(plan);

    // Libertar memória
    debug("Freeing memory\n");
    memdestroy(memory);

    // Libertar memória heap
    debug("Freeing heap memory (4 components)\n");
    destroyheap(heap_first);
    destroyheap(heap_next);
    destroyheap(heap_best);
    destroyheap(heap_worst);

    // Libertar tabela PCB
    debug("Freeing PCB table\n");
    pcbfree(pcb);

    return 0;
}
