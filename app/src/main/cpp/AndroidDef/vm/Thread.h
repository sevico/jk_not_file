/*
 * Copyright (C) 2008 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/*
 * VM thread support.
 */
#ifndef DALVIK_THREAD_H_
#define DALVIK_THREAD_H_


#include <errno.h>
#include <pthread.h>
#include <assert.h>
#include "Common.h"

#if defined(CHECK_MUTEX) && !defined(__USE_UNIX98)
/* glibc lacks this unless you #define __USE_UNIX98 */
int pthread_mutexattr_settype(pthread_mutexattr_t *attr, int type);
enum { PTHREAD_MUTEX_ERRORCHECK = PTHREAD_MUTEX_ERRORCHECK_NP };
#endif

/*
 * Current status; these map to JDWP constants, so don't rearrange them.
 * (If you do alter this, update the strings in dvmDumpThread and the
 * conversion table in VMThread.java.)
 *
 * Note that "suspended" is orthogonal to these values (so says JDWP).
 */
enum ThreadStatus {
    THREAD_UNDEFINED    = -1,       /* makes enum compatible with int32_t */

    /* these match up with JDWP values */
    THREAD_ZOMBIE       = 0,        /* TERMINATED */
    THREAD_RUNNING      = 1,        /* RUNNABLE or running now */
    THREAD_TIMED_WAIT   = 2,        /* TIMED_WAITING in Object.wait() */
    THREAD_MONITOR      = 3,        /* BLOCKED on a monitor */
    THREAD_WAIT         = 4,        /* WAITING in Object.wait() */
    /* non-JDWP states */
    THREAD_INITIALIZING = 5,        /* allocated, not yet running */
    THREAD_STARTING     = 6,        /* started, not yet on thread list */
    THREAD_NATIVE       = 7,        /* off in a JNI native method */
    THREAD_VMWAIT       = 8,        /* waiting on a VM resource */
    THREAD_SUSPENDED    = 9,        /* suspended, usually by GC or debugger */
};

/* thread priorities, from java.lang.Thread */
enum {
    THREAD_MIN_PRIORITY     = 1,
    THREAD_NORM_PRIORITY    = 5,
    THREAD_MAX_PRIORITY     = 10,
};


/* initialization */
bool dvmThreadStartup(void);
void dvmThreadShutdown(void);
void dvmSlayDaemons(void);


#define kJniLocalRefMin         64
#define kJniLocalRefMax         512     /* arbitrary; should be plenty */
#define kInternalRefDefault     32      /* equally arbitrary */
#define kInternalRefMax         4096    /* mainly a sanity check */

#define kMinStackSize       (512 + STACK_OVERFLOW_RESERVE)
#define kDefaultStackSize   (16*1024)   /* four 4K pages */
#define kMaxStackSize       (256*1024 + STACK_OVERFLOW_RESERVE)

/*
 * Interpreter control struction.  Packed into a long long to enable
 * atomic updates.
 */
union InterpBreak {
    volatile int64_t   all;
    struct {
        uint16_t   subMode;
        uint8_t    breakFlags;
        int8_t     unused;   /* for future expansion */
#ifndef DVM_NO_ASM_INTERP
        void* curHandlerTable;
#else
        int32_t    unused1;
#endif
    } ctl;
};


/*
 * Thread suspend/resume, used by the GC and debugger.
 */
enum SuspendCause {
    SUSPEND_NOT = 0,
    SUSPEND_FOR_GC,
    SUSPEND_FOR_DEBUG,
    SUSPEND_FOR_DEBUG_EVENT,
    SUSPEND_FOR_STACK_DUMP,
    SUSPEND_FOR_DEX_OPT,
    SUSPEND_FOR_VERIFY,
    SUSPEND_FOR_HPROF,
    SUSPEND_FOR_SAMPLING,
#if defined(WITH_JIT)
    SUSPEND_FOR_TBL_RESIZE,  // jit-table resize
    SUSPEND_FOR_IC_PATCH,    // polymorphic callsite inline-cache patch
    SUSPEND_FOR_CC_RESET,    // code-cache reset
    SUSPEND_FOR_REFRESH,     // Reload data cached in interpState
#endif
};



/*
 * Initialize a mutex.
 */
void dvmInitMutex(pthread_mutex_t* pMutex);

/*
 * Grab a plain mutex.
 */
void dvmLockMutex(pthread_mutex_t* pMutex);

/*
 * Try grabbing a plain mutex.  Returns 0 if successful.
 */
int dvmTryLockMutex(pthread_mutex_t* pMutex);

/*
 * Unlock pthread mutex.
 */
void dvmUnlockMutex(pthread_mutex_t* pMutex);

/*
 * Destroy a mutex.
 */
void dvmDestroyMutex(pthread_mutex_t* pMutex);

void dvmBroadcastCond(pthread_cond_t* pCond);

void dvmSignalCond(pthread_cond_t* pCond);

void dvmWaitCond(pthread_cond_t* pCond, pthread_mutex_t* pMutex);

#endif  // DALVIK_THREAD_H_
