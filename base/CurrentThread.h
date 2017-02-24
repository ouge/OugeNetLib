#ifndef CURRENTTHREAD_H
#define CURRENTTHREAD_H


#include <pthread.h>


namespace ouge
{

class CurrentThread
{
public:
    static pid_t tid();
};

}




#endif /* CURRENTTHREAD_H */
