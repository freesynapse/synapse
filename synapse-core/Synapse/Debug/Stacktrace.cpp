
#include "Stacktrace.h"

#include "Synapse/Event/EventHandler.hpp"
#include "Synapse/Utils/Thread/ThreadPool.hpp"
#include "Synapse/Debug/Profiler.hpp"
#include "Synapse/Debug/Log.hpp"


namespace Syn
{
    // static decls
    struct sigaction Stacktracer::s_sigact;

    //-----------------------------------------------------------------------------------
    void Stacktracer::register_signal_handler()
    {
        s_sigact.sa_sigaction = critical_error_handler;
        s_sigact.sa_flags = SA_RESTART | SA_SIGINFO;

        if (sigaction(SIGSEGV, &s_sigact, (struct sigaction *)NULL) != 0)
        {
            fprintf(stderr, "error setting signal handler for %d (%s)\n",
            SIGSEGV, strsignal(SIGSEGV));
            exit(EXIT_FAILURE);
        }
        
    }
    //-----------------------------------------------------------------------------------
    void Stacktracer::critical_error_handler(int sig_num, siginfo_t * info, void * ucontext)
    {
        sig_ucontext_t * uc = (sig_ucontext_t *)ucontext;

        void* caller_address;
        #if defined(__i386__) // gcc specific
            caller_address = (void *) uc->uc_mcontext.eip; // EIP: x86 specific
        #elif defined(__x86_64__) // gcc specific
            caller_address = (void *) uc->uc_mcontext.rip; // RIP: x86_64 specific
        #else
            #error Unsupported architecture. // TODO: Add support for other arch.
        #endif


        FILE* fptr = fopen("./crash_report.txt", "w");
        if (fptr == NULL)
            printf("\x1b[1;37;41mcrash report couldn't be created.\n");
        else
            fprintf(fptr, "SIGSEGV stacktrace\n");

        fprintf(stderr, "\x1b[1;37;41m--------------------- STACKTRACE ---------------------\x1b[0m\n");
        fprintf(stderr, "signal %d (%s) address is 0x%" PRIXPTR " from 0x%" PRIXPTR ".\n",
                sig_num, strsignal(sig_num), (uintptr_t)info->si_addr, (uintptr_t)caller_address);
        
        if (fptr)
            fprintf(fptr, "signal %d (%s) address is 0x%" PRIXPTR " from 0x%" PRIXPTR ".\n",
                    sig_num, strsignal(sig_num), (uintptr_t)info->si_addr, (uintptr_t)caller_address);

        void* array[50];
        int size = backtrace(array, 50);

        array[1] = caller_address;

        char** messages = backtrace_symbols(array, size);    

        // skip first stack frame (points here)
        for (int i = 1; i < size && messages != NULL; ++i)
        {
            char* mangled_name = 0;
            char* offset_begin = 0;
            char* offset_end = 0;

            // find parantheses and +address offset surrounding mangled name
            for (char* p = messages[i]; *p; ++p)
            {
                if (*p == '(') 
                    mangled_name = p; 
                else if (*p == '+') 
                    offset_begin = p;
                else if (*p == ')')
                {
                    offset_end = p;
                    break;
                }
            }

            // if the line could be processed, attempt to demangle the symbol
            if (mangled_name && offset_begin && offset_end && 
                mangled_name < offset_begin)
            {
                *mangled_name++ = '\0';
                *offset_begin++ = '\0';
                *offset_end++ = '\0';

                int status;
                char* real_name = abi::__cxa_demangle(mangled_name, 0, 0, &status);

                // if demangling is successful, output the demangled function name
                if (status == 0)
                {
                    fprintf(stderr, "[stacktrace]: (%2d) \x1b[4m%s: %s\x1b[0m+%s%s\n", 
                            i, messages[i], real_name, offset_begin, offset_end);

                    if (fptr)
                        fprintf(fptr, "[stacktrace]: (%2d) %s: %s+%s%s\n", 
                                i, messages[i], real_name, offset_begin, offset_end);
                }

                // otherwise, output the mangled function name
                else
                {
                    fprintf(stderr, "[stacktrace]: (%2d) \x1b[4m%s: %s\x1b[0m+%s%s\n", 
                            i, messages[i], mangled_name, offset_begin, offset_end);
                    if (fptr)
                        fprintf(fptr, "[stacktrace]: (%2d) %s: %s+%s%s\n", 
                                i, messages[i], mangled_name, offset_begin, offset_end);
                }

                free(real_name);
            }
            // otherwise, print the whole line
            else
            {
                fprintf(stderr, "[stacktrace]: (%d) %s\n", i, messages[i]);
                if (fptr)
                    fprintf(fptr, "[stacktrace]: (%d) %s\n", i, messages[i]);
            }
        }

        fprintf(stderr, "\x1b[1;37;41m------------------------------------------------------\x1b[0m\n");

        fclose(fptr);
        free(messages);

        Syn::EventHandler::shutdown();
        //Syn::FileIOHandler::shutdown();
        Syn::ThreadPool::get().shutdown();

        #ifdef ENABLE_ADDONS_PY_EMBEDDING
            Syn::PyWrapper::shutdown();
        #endif
        
        #ifdef DEBUG_PROFILING
            Syn::Profiler::get().endSession();
        #endif

        SYN_CORE_TRACE("closing log.");
        Syn::Log::close();

        exit(-1);
    }

}