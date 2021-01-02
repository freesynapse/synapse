
#pragma once

#include <stdlib.h>
#include <string.h>


/* Zero memory in pointer, sz bytes. */
#define ZERO_MEMORY(ptr, sz) \
					memset(ptr, 0, sz);

/* Allocation of 3D pointer array memory, wrapper for malloc. */
#define ALLOCATE_3D_ARRAY(ptr, type, sz_x, sz_y, sz_z) \
					ptr = (type***)malloc(sizeof(type**) * sz_x); \
					for (int x = 0; x < sz_x; x++) { \
						ptr[x] = (type**)malloc(sizeof(type*) * sz_y); \
						for (int y = 0; y < sz_y; y++) { \
							ptr[x][y] = (type*)malloc(sizeof(type) * sz_z); \
						} \
					}

/* Allocation and zeroing of 3D pointer array memory, wrapper for malloc and memset. */
#define ALLOCATE_ZERO_3D_ARRAY(ptr, type, sz_x, sz_y, sz_z) \
					ptr = (type***)malloc(sizeof(type**) * sz_x); \
					for (int x = 0; x < sz_x; x++) { \
						ptr[x] = (type**)malloc(sizeof(type*) * sz_y); \
						for (int y = 0; y < sz_y; y++) { \
							ptr[x][y] = (type*)malloc(sizeof(type) * sz_z); \
							memset(ptr[x][y], 0, sizeof(type) * sz_z); \
						} \
					}

/* Allocation of 2D pointer array memory, wrapper for malloc. */
#define ALLOCATE_2D_ARRAY(ptr, type, sz_x, sz_y) \
					ptr = (type**)malloc(sizeof(type*) * sz_x); \
					for (int x = 0; x < sz_x; x++) { \
						ptr[x] = (type*)malloc(sizeof(type) * sz_y); \
					}


/* Allocation and zeroing of 2D pointer array memory, wrapper for malloc and memset. */
#define ALLOCATE_ZERO_2D_ARRAY(ptr, type, sz_x, sz_y) \
					ptr = (type**)malloc(sizeof(type*) * sz_x); \
					for (int x = 0; x < sz_x; x++) { \
						ptr[x] = (type*)malloc(sizeof(type) * sz_y); \
						memset(ptr[x], sizeof(type), sz_y); \
					}

/* Deallocation of 3D pointer array memory, wrapper for free. */
#define FREE_3D_ARRAY(ptr, sz_x, sz_y) \
					for (int x = 0; x < sz_x; x++) { \
						for (int y = 0; y < sz_y; y++) { \
							free(ptr[x][y]); \
							ptr[x][y] = nullptr; \
						} \
						free(ptr[x]); \
						ptr[x] = nullptr; \
					} \
					free(ptr); \
					ptr = nullptr;

/* Deallocation of 2D pointer array memory, wrapper for free. */
#define FREE_2D_ARRAY(ptr, sz_x) \
					for (int x = 0; x < sz_x; x++) { \
						free(ptr[x]); \
						ptr[x] = nullptr; \
					} \
					free(ptr); \
					ptr = nullptr;

