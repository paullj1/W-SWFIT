/*****************************************************************************/
/*                                                                           */
/* resourceleak.cpp                                                          */
/* Project: W-SWFIT: Resource Leak                                           */
/* Authors: Paul Jordan                                                      */
/* Date Created:  8 May 2016                                                 */
/*                                                                           */
/* Description: Small app designed to fill up memory, disk, or CPU at a      */
/* configurable rate  in order to force a system to fail.  This application  */
/* simulates a poorly written third-party application which might cause      */
/* failure in an underlying system.                                          */
/*                                                                           */
/* Copyright (c) 2016                                                        */
/*                                                                           */
/*****************************************************************************/

#include <stdlib.h>
#include <chrono>
#include <thread>

#include "memory.hpp"
#include "cpu.hpp"
//#include "disk.hpp"

using namespace std;

int main(int argc, char *argv[]) {
  //CPU *leak = new CPU();
  Memory *leak = new Memory();
  leak->start(1);

  while(true) { this_thread::sleep_for(chrono::seconds(1)); }
  return 0;
}
