
#include <stdio.h>
#include <unistd.h>		// sysconf()
#include <string>
#include <fstream>

typedef struct proc_mem_ proc_mem_t;
struct proc_mem_
{
	// all sizes in kB
	size_t vm_size;
	size_t rss_size;
	size_t shared_size;

};

void processMemory(proc_mem_t* _proc_ptr_ret)
{
	_proc_ptr_ret->vm_size     = 0;
	_proc_ptr_ret->rss_size    = 0;
	_proc_ptr_ret->shared_size = 0;
	

	unsigned long size;
	long rss, shared;
	{
		std::ifstream ifs("/proc/self/statm", std::ios_base::in);
		ifs >> size >> rss >> shared;
	}

	long pageSize_kB = sysconf(_SC_PAGE_SIZE) / 1024;
	_proc_ptr_ret->vm_size     = size * pageSize_kB;
	_proc_ptr_ret->rss_size    = rss * pageSize_kB;
	_proc_ptr_ret->shared_size = shared * pageSize_kB;

	printf("\traw size   = %ld\n", size);
	printf("\traw rss    = %ld\n", rss);
	printf("\traw shared = %ld\n", shared);
	
}




int main(int argc, char* argv[])
{
	printf("sysconf(_SC_PAGE_SIZE) = %ld\n", sysconf(_SC_PAGE_SIZE));

	proc_mem_t usage2;
	processMemory(&usage2);
	
	printf("vm_size: %zu (%.2f MB)\nvm_rss:  %zu (%.2f MB)\n", usage2.vm_size, usage2.vm_size / 1024.0f, usage2.rss_size, usage2.rss_size / 1024.0f);


	return 0;
}






