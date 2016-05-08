/*****************************************************************************/
/*                                                                           */
/* cpu.hpp                                                                */
/* Project: W-SWFIT: Resource Leak                                           */
/* Authors: Paul Jordan                                                      */
/* Date Created:  8 May 2016                                                 */
/* Description: Header file for the CPU leak class.                       */
/*                                                                           */
/* Copyright (c) 2016                                                        */
/*                                                                           */
/*****************************************************************************/

#ifndef CPU_H
#define CPU_H

#include "mingw.thread.h"

#include <stdlib.h>
#include <chrono>
#include <thread>

#include "resource.hpp"

using namespace std;

class CPU : public Resource {
  public:
    CPU() {}
    ~CPU() {}

    bool start(int rate); // smaller number = faster leak 
    bool stop();

    bool running() const { return _running; }
    int rate() const { return _rate; }

  private:
    void leak();

    bool _running = false;
    int _rate = 0;
    double __rate = 0;
    thread _leak;

};

#endif
