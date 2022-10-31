
#pragma once

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif
#ifndef __USE_GNU
#define __USE_GNU
#endif

#include <execinfo.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ucontext.h>
#include <unistd.h>

#include <iostream>
#include <cxxabi.h>
#include <inttypes.h>

#include "Synapse/Core.hpp"


namespace Syn
{
    // This structure mirrors the one found in /usr/include/asm/ucontext.h
    typedef struct sig_ucontext_ {
        unsigned long     uc_flags;
        ucontext_t*       uc_link;
        stack_t           uc_stack;
        sigcontext        uc_mcontext;
        sigset_t          uc_sigmask;
    } sig_ucontext_t;

    // static class
    class Stacktracer
    {
    public:
        static void critical_error_handler(int sig_num, siginfo_t * info, void * ucontext);
        static void register_signal_handler();
      
        static struct sigaction s_sigact;
    };
}
