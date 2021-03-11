#pragma once

#include <stdlib.h>
#include <ncurses.h>
#include <string.h>
#include <string>


// curses helper structs

//
typedef struct window_border_t_
{
	chtype ls = '|', rs = '|', ts = '-', bs = '-';
	chtype tl = '+', tr = '+', bl = '+', br = '+';

} window_border_t;

//
typedef struct window_t_
{
	int start_x = 0, start_y = 0;
	int width = 0, height = 0;
	window_border_t border;
	std::string header = "";

	window_t_(int _x, int _y, int _w, int _h, const std::string& _header="") : 
		start_x(_x), start_y(_y), width(_w), height(_h), header(_header)
	{}

	window_t_() {}

	void set(int _x, int _y, int _w, int _h, const std::string& _header="")
	{ start_x = _x; start_y = _y; width = _w; height = _h; header = _header; }

} window_t;

//
typedef struct mem_view_params_t_
{
	uint16_t x_start_addr, 	x_stop_addr;	// start and stop coord for address
	uint16_t x_start_mem, 	x_stop_mem;		// start and stop coord for memory content
	uint16_t y_start, 		y_stop;			// start and stop for lines in window

	uint8_t block_width;		// width of a word of memory content (+ spacing)
	
	uint16_t words_per_row;		// number of words per row
	uint16_t words_per_view;	// number of words per page
	uint32_t word_max_view;		// current pages max word
	uint32_t word_offset;		// the current word (permitting multiple pages)

	void print_debug(int _i=10)
	{
		mvprintw(_i++, COLS-40, "hex_view_params_t.x_start_addr:   %d  ", x_start_addr);
		mvprintw(_i++, COLS-40, "hex_view_params_t.x_stop_addr:    %d  ", x_stop_addr);
		mvprintw(_i++, COLS-40, "hex_view_params_t.x_start_mem:    %d  ", x_start_mem);
		mvprintw(_i++, COLS-40, "hex_view_params_t.x_stop_mem:     %d  ", x_stop_mem);
		mvprintw(_i++, COLS-40, "hex_view_params_t.y_start:        %d  ", y_start);
		mvprintw(_i++, COLS-40, "hex_view_params_t.y_stop:         %d  ", y_stop);

		mvprintw(_i++, COLS-40, "hex_view_params_t.block_width:    %d  ", block_width);
		mvprintw(_i++, COLS-40, "hex_view_params_t.words_per_row:  %d  ", words_per_row);
		mvprintw(_i++, COLS-40, "hex_view_params_t.words_per_view: %d  ", words_per_view);
		mvprintw(_i++, COLS-40, "hex_view_params_t.word_max_view:  %d  ", word_max_view);
		mvprintw(_i++, COLS-40, "hex_view_params_t.word_offset:    %d  ", word_offset);
	}

} mem_view_params_t;


// editor constants
//
constexpr uint8_t WORD_SIZE = sizeof(uint32_t); 

#define COLOR_INVERTED			1
#define COLOR_ATTENTION			2

#define MIN(a,b) ((a)<(b))?(a):(b)


// memory viewer class (static)
class MemoryViewer
{
public:
	static void initialize(void* _memory, uint32_t _mem_size, bool _zero_mem=true);
	static void shutdown(bool _free_memory=true);

	static void displayWindow(window_t* _win, bool _show);
	static void displayStaticElements();
	static void displayDynamicElements();
	static void updateMemoryViews(uint32_t _offset_words);
	
	static bool update(uint32_t _sleep_ms=1e7);

	static inline void setMemory(size_t _word_offset, uint32_t _word_val)
	{
		if (_word_offset * WORD_SIZE >= m_memSize)
			return;
		uint32_t* ptr = (uint32_t*)m_memory + _word_offset;
		*ptr = _word_val;
		updateMemoryViews(m_memOffsetWords);
	}
	static void freeMemory() { if (m_memory) free(m_memory); }


private:
	// member functions
	static void printWordBinary(int _y, int _x, uint32_t _word_val);
	// DEBUG
	static void print_debug(int _i=10);
	static void nextPage();
	static void prevPage();

	// member variables
	static void* m_memory;						// Pointer to inspected memory.
	static uint32_t m_memSize;					// Size of allocated memory in bytes.
	static uint32_t m_memOffsetWords;			// Offset into memory (in words).
	static uint32_t m_memNumWords;				// Number of allocated words.

	static uint16_t m_selectedNumWords ;		// How many words of memory that are selected for selectedion.
	static uint16_t m_selectedStartWord;		// How many words into the current hex view that selectedion starts.
	static uint16_t m_selectedWordsMax;			// Max number to be selected.

	static mem_view_params_t m_hexViewParams;	// Display coordinates for hexadecimal view.
	static mem_view_params_t m_binViewParams;	// Display coordinates for binary view.
	
	static window_t m_hexWindow;				// Hexadecimal view window
	static window_t m_binWindow;				// Binary view window


};




