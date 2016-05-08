/*****************************************************************************/
/*                                                                           */
/* Memory.hpp                                                                */
/* Project: W-SWFIT: Resource Leak                                           */
/* Authors: Paul Jordan                                                      */
/* Date Created:  8 May 2016                                                 */
/* Description: Header file for the Memory leak class.                       */
/*                                                                           */
/* Copyright (c) 2016                                                        */
/*                                                                           */
/*****************************************************************************/

#ifndef MEMORY_H
#define MEMORY_H

#include "globals.hpp"
#include "resource.hpp"

using namespace std;

class Memory : public Resource {
  public:
    Memory();
    ~Memory();

    bool start(int rate); // # of milliseconds to sleep 
                          // before allocating more memory
                          // (smaller number = faster leak)
    bool stop();

    bool running() const { return _running; }
    int rate() const { return _rate; }

  private:
    void leak();

    vector<void *> storage;
    bool _running = false;
    int _rate = 0;
    thread _leak;

};

#endif
