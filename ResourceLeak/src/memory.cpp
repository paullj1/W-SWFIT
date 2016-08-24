/*****************************************************************************/
/*                                                                           */
/* Memory.cpp                                                                */
/* Project: W-SWFIT: Resource Leak                                           */
/* Authors: Paul Jordan                                                      */
/* Date Created:  8 May 2016                                                 */
/* Description: Implementation file for the Memory leak class.               */
/*                                                                           */
/*****************************************************************************/

#include <math.h>
#include "memory.hpp"

Memory::Memory() {
  storage = vector<void *>();
}

Memory::~Memory() {
  storage.clear();
}

bool Memory::start(int rate) {
  _running = true;
  _rate = rate;
  _leak = thread(&Memory::leak, this);
  return true;
}

void Memory::leak() {
  while(_running) {
    void *buf = malloc(pow(10,6)); // Allocate 1MB at rate 
    storage.push_back(buf);
    this_thread::sleep_for(chrono::milliseconds(_rate));
  }
}

bool Memory::stop() {
  _running = false;
  return true;
}

