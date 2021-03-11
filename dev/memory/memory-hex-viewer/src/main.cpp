
#include "MemView.h"

constexpr uint32_t HEAP_SIZE = 1024 * 1024 * 20;


int main(int argc, char* argv[])
{
	// allocate heap
	void* heap = malloc(HEAP_SIZE);

	// start the viewer
	MemoryViewer::initialize(heap, HEAP_SIZE, false);
	
	// set some memory
	//int i;
	//for (i = 0; i < HEAP_SIZE / WORD_SIZE; i++)
	//	MemoryViewer::setMemory(i, i);
	//MemoryViewer::setMemory(i-1, 4294967295);	
	MemoryViewer::setMemory(0, 4294967295);	
	
	// main loop
	while(MemoryViewer::update() == true);

	// shut down gracefully
	MemoryViewer::shutdown();

	return 0;
}

