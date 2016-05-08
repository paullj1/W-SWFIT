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

#include "globals.hpp"
#include "memory.hpp"
#include "cpu.hpp"
//#include "disk.hpp"

using namespace std;

int main(int argc, char *argv[]) {
  // Process Command Line Args
  if ( argc < 3 ) {
    cerr << "Need to specify which type of leak [m]emory, or [c]pu." << endl;
    cerr << "usage: "<< argv[0] <<" -[m|c] <rate>" << endl;
    return 1;
  } 

  Resource *leak = NULL;
  if      ( string(argv[1]).compare("-m") == 0 )
    leak = new Memory();
  else if ( string(argv[1]).compare("-c") == 0 )
    leak = new CPU();
  else {
    cerr << "Unrecognized leak type. Specify [m]emory, or [c]pu." << endl;
    return 1;
  }

  int rate;
  string str_rate = string(argv[2]);
  if ( ! (istringstream(str_rate) >> rate) ) rate = 0;

  if (rate <= 0 || rate > 100) {
    cerr << "Unrecognized rate. Specify rate between 1-100." << endl;
    return 1;
  }

  if (leak)
    leak->start(1);

  while(true) { this_thread::sleep_for(chrono::seconds(1)); }
  return 0;
}
