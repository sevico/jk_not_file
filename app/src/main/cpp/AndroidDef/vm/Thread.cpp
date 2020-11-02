//
// Created by F8LEFT on 2017/3/31.
//

#include "Thread.h"

void dvmLockMutex(pthread_mutex_t *pMutex) {
    int cc __attribute__ ((__unused__)) = pthread_mutex_lock(pMutex);
    assert(cc == 0);
}

int dvmTryLockMutex(pthread_mutex_t *pMutex) {
    int cc = pthread_mutex_trylock(pMutex);
    assert(cc == 0 || cc == EBUSY);
    return cc;
}

void dvmUnlockMutex(pthread_mutex_t *pMutex) {
    int cc __attribute__ ((__unused__)) = pthread_mutex_unlock(pMutex);
    assert(cc == 0);
}

void dvmDestroyMutex(pthread_mutex_t *pMutex) {
    int cc __attribute__ ((__unused__)) = pthread_mutex_destroy(pMutex);
    assert(cc == 0);
}

void dvmBroadcastCond(pthread_cond_t *pCond) {
    int cc __attribute__ ((__unused__)) = pthread_cond_broadcast(pCond);
    assert(cc == 0);
}

void dvmSignalCond(pthread_cond_t *pCond) {
    int cc __attribute__ ((__unused__)) = pthread_cond_signal(pCond);
    assert(cc == 0);
}

void dvmWaitCond(pthread_cond_t *pCond, pthread_mutex_t *pMutex) {
    int cc __attribute__ ((__unused__)) = pthread_cond_wait(pCond, pMutex);
    assert(cc == 0);
}

void dvmInitMutex(pthread_mutex_t *pMutex) {
#ifdef CHECK_MUTEX
    pthread_mutexattr_t attr;
    int cc;

    pthread_mutexattr_init(&attr);
    cc = pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_ERRORCHECK_NP);
    assert(cc == 0);
    pthread_mutex_init(pMutex, &attr);
    pthread_mutexattr_destroy(&attr);
#else
    pthread_mutex_init(pMutex, NULL);       // default=PTHREAD_MUTEX_FAST_NP
#endif
}
