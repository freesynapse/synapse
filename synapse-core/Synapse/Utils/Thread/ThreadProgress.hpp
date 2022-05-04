
#pragma once

#include <atomic>


namespace Syn
{
    typedef struct thread_progress_t
    {
        std::atomic<float> progress;
        std::atomic<bool> done;

        thread_progress_t()
        {
            progress.store(0.0f);
            done.store(false);
        }

    } thread_progress_t;
}