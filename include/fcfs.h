//------------------------------------------------------------------------------
//
// chronOS - A scheduling simulator
//
//    Copyright (C) 2020 Igor Cordeiro Bordalo Nunes (www.igornunes.com)
//    Copyright (C) 2020 Jorge Miguel Louro Pissarra
//    Copyright (C) 2020 Diogo Castanheira Simões
//    Copyright (C) 2020 Universidade da Beira Interior (www.ubi.pt)
//
// RUNTIME LIBRARIES PACKAGE
//    fcfs.h
//
// DESCRIPTION:
// -----------
// First Come First Serve algorithm.
//------------------------------------------------------------------------------

#ifndef FCFS_H
#define FCFS_H

#include "types.h"

#define FCFS_END -1

/* Function:  fcfs
 * ---------------
 * (TODO)
 * 
 *  pcbindex: 
 * 
 *  return: 
 */
int fcfs(PCB *pcb, MEMORY *mem, int pcbindex);

#endif