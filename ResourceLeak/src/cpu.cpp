/*****************************************************************************/
/*                                                                           */
/* cpu.cpp                                                                */
/* Project: W-SWFIT: Resource Leak                                           */
/* Authors: Paul Jordan                                                      */
/* Date Created:  8 May 2016                                                 */
/* Description: Implementation file for the CPU leak class.               */
/*                                                                           */
/*****************************************************************************/

#include "cpu.hpp"

bool CPU::start(int rate) {
  _running = true;
  _rate = rate;
  __rate = rate;  // mutable (degrading) rate
  _leak = thread(&CPU::leak, this);
  return true;
}

void CPU::leak() {
  while(_running) {

    if (__rate > 1) { __rate *= .99; }
    else { __rate = 0; }

    this_thread::sleep_for(chrono::milliseconds((int)__rate));
  }
}

bool CPU::stop() {
  _running = false;
  return true;
}

