/*****************************************************************************/
/*                                                                           */
/* Resource.hpp                                                              */
/* Project: W-SWFIT: Resource Leak                                           */
/* Authors: Paul Jordan                                                      */
/* Date Created:  8 May 2016                                                 */
/* Description: Abstract resource header file.  Each resource implements     */
/* abstract class.                                                           */
/*                                                                           */
/*****************************************************************************/

#ifndef RESOURCE_H
#define RESOURCE_H

#include "globals.hpp"

class Resource {
  public:
    virtual bool start(int rate) = 0;
    virtual bool stop() = 0;
    bool running() const { return _running; }
    int rate() const { return _rate; }

  private:
    bool _running = false;
    int _rate = 0;

};

#endif
