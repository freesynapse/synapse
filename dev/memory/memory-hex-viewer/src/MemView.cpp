
#include <math.h>
#include <time.h>

#include "MemView.h"


// static declarations
void* 			  MemoryViewer::m_memory;
uint32_t 		  MemoryViewer::m_memSize;
uint32_t 		  MemoryViewer::m_memOffsetWords;
uint32_t		  MemoryViewer::m_memNumWords;
uint16_t 		  MemoryViewer::m_selectedNumWords ;
uint16_t 		  MemoryViewer::m_selectedStartWord;
uint16_t 		  MemoryViewer::m_selectedWordsMax;
mem_view_params_t MemoryViewer::m_hexViewParams;
mem_view_params_t MemoryViewer::m_binViewParams;
window_t 		  MemoryViewer::m_hexWindow;
window_t 		  MemoryViewer::m_binWindow;


//---------------------------------------------------------------------------------------
// PUBLIC MEMBER FUNCTIONS
//
void MemoryViewer::initialize(void* _memory, uint32_t _mem_size, bool _zero_mem)
{
	// memory address and size
	//
	m_memory = _memory;
	m_memSize = _mem_size;
	m_memNumWords = _mem_size / WORD_SIZE;
	
	if (_zero_mem)
		memset(m_memory, 0, m_memSize);


	// initialize curses
	//
	initscr();
	noecho();				// no input echoed
	cbreak();				// get all input
	keypad(stdscr, true);	// enable full keyboard functionality
	start_color();			// enable colors
	use_default_colors();	// default foreground and background colors are -1.
	curs_set(0);			// hide the cursor
	// set pairs of colors (C-style)
	init_pair(COLOR_INVERTED, COLOR_WHITE, COLOR_BLACK);
	init_pair(COLOR_ATTENTION, COLOR_RED, -1);


	// viewer parameters
	//

	// coordinates and parameters of hexadecimal view
	m_hexViewParams.x_start_addr = 3;
	m_hexViewParams.x_stop_addr  = m_hexViewParams.x_start_addr + 10;
	m_hexViewParams.x_start_mem  = m_hexViewParams.x_stop_addr + 3;
	m_hexViewParams.x_stop_mem   = COLS / 2 - 2;
	m_hexViewParams.y_start 	 = 4;
	m_hexViewParams.y_stop 		 = LINES - 2 - 1;
	m_hexViewParams.block_width  = 8 + 1; // 4 bytes in hex + 1 space
	m_hexViewParams.words_per_row  = (m_hexViewParams.x_stop_mem - m_hexViewParams.x_start_mem) // width of memory content div by width of one block
									 / m_hexViewParams.block_width;
	m_hexViewParams.words_per_view = (m_hexViewParams.y_stop - m_hexViewParams.y_start) // number of lines times words per line
									 * m_hexViewParams.words_per_row; 
	m_hexViewParams.word_max_view  = m_hexViewParams.words_per_view;
	m_hexViewParams.word_offset = 0;
	
	// coordinates and parameters of binary view
	m_binViewParams.x_start_addr = COLS / 2 + 4;
	m_binViewParams.x_stop_addr  = m_binViewParams.x_start_addr + 10;
	m_binViewParams.x_start_mem  = m_binViewParams.x_stop_addr + 3;
	m_binViewParams.x_stop_mem   = COLS - 2;
	m_binViewParams.y_start 	 = m_hexViewParams.y_start;
	m_binViewParams.y_stop 		 = m_hexViewParams.y_stop;
	m_binViewParams.block_width  = 32 + 8;
	m_binViewParams.words_per_row = (m_binViewParams.x_stop_mem - m_binViewParams.x_start_addr) // width of 32 bit blocks
									/ m_binViewParams.block_width;
	m_binViewParams.word_max_view = (m_binViewParams.y_stop - m_binViewParams.y_start) // find max, this restricts the m_selectedWordsMax
									* m_binViewParams.words_per_row / 2;
	m_binViewParams.word_offset = 0;

	// selection params
	m_selectedWordsMax = m_binViewParams.word_max_view;
	m_selectedNumWords = 1;//MIN(m_selectedWordsMax, m_memSize / WORD_SIZE);
	m_selectedStartWord = 0;


	// initialize windows (bin and hex) and show default mem view
	//
	m_hexWindow.set(0, 0, COLS/2, LINES-2, "HEXADECIMAL VIEW");
	m_binWindow.set(COLS/2+1, 0, COLS/2-2, LINES-2, "BINARY VIEW");
	//initWindow(&m_hexWindow);
	//initWindow(&m_binWindow);
	displayWindow(&m_hexWindow, true);
	displayWindow(&m_binWindow, true);

	// displayed elements
	updateMemoryViews(0);
	displayStaticElements();
	displayDynamicElements();
}


//---------------------------------------------------------------------------------------
void MemoryViewer::shutdown(bool _free_memory)
{
	if (m_memory)
		free(m_memory);
		
	// shut down curses
	endwin();
}


//---------------------------------------------------------------------------------------
void MemoryViewer::displayWindow(window_t* _win, bool _show)
{
	int x = _win->start_x;
	int y = _win->start_y;
	int w = _win->width;
	int h = _win->height;

	if (_show)
	{
		// border
		mvaddch(y, x, _win->border.tl);
		mvaddch(y, x + w, _win->border.tr);
		mvaddch(y + h, x, _win->border.bl);
		mvaddch(y + h, x + w, _win->border.br);
		mvhline(y, x + 1, _win->border.ts, w - 1);
		mvhline(y + h, x + 1, _win->border.bs, w - 1);
		mvvline(y + 1, x, _win->border.ls, h - 1);
		mvvline(y + 1, x + w, _win->border.rs, h - 1);
		// header
		if (_win->header != "")
		{
			mvprintw(y + 1, x + 1, _win->header.c_str());
			mvhline(y + 2, x + 1, _win->border.ts, w - 1);
		}
	}
	else
	{
		// delete border
		for(int j = y; j <= y + h; ++j)
			for(int i = x; i <= x + w; ++i)
				mvaddch(j, i, ' ');
		if (_win->header != "")
		{
			// remove header and separator
			mvhline(y + 1, x + 1, ' ', w - 1);
			mvhline(y + 2, x + 1, ' ', w - 1);
		}
	}
}


//---------------------------------------------------------------------------------------
void MemoryViewer::displayStaticElements()
{
	// commands shown at the last row
	mvprintw(LINES-1, 0, "|  EXIT: X  ||  INC SEL: E  ||  DEC SEL: Q  ||  MOVE SEL: WASD");
	mvprintw(LINES-1, COLS-1, "|");
}


//---------------------------------------------------------------------------------------
void MemoryViewer::displayDynamicElements()
{
	// address of selected memory
	uint32_t* currAddr = (uint32_t*)m_memory + m_selectedStartWord;
	// address difference in number of words (ie 4 byte blocks)
	uint32_t offset_words = ((uint64_t)((unsigned char*)currAddr) - (uint64_t)((unsigned char*)m_memory)) / WORD_SIZE;

	mvprintw(LINES-1, COLS-45, "SEL: 0x%08x [ %d : %d ] [%01d]  ", 
		currAddr,
		offset_words, offset_words + m_selectedNumWords - 1,
		m_selectedNumWords
	);

	//print_debug(10);
}


//---------------------------------------------------------------------------------------
void MemoryViewer::updateMemoryViews(uint32_t _offset_words)
{
	/* 
	 * Starting address is cast as uchar* first to be able to add the _offset as bytes
	 * and not as a word (32 bit, 4 byte). However, the _offset is aligned to a multiple
	 * of words.
	 */
	uint32_t* addr_ptr = (uint32_t*)((unsigned char*)m_memory) + _offset_words;

	// update max word for this page, based on offset
	m_hexViewParams.word_max_view = (m_hexViewParams.y_stop - m_hexViewParams.y_start) * m_hexViewParams.words_per_row + _offset_words;

	#pragma region hex_view

	uint32_t* mem_ptr = addr_ptr;
	uint32_t* mem_end = (uint32_t*)((char*)m_memory + m_memSize);
	size_t y = m_hexViewParams.y_start;
	size_t x = m_hexViewParams.x_start_mem;
	size_t numWords = 0;
	size_t currentWord = _offset_words;
	size_t selectedNumWords = 0;

	while ((mem_ptr != mem_end) && (numWords + _offset_words < m_hexViewParams.word_max_view))
	{
		if (numWords % m_hexViewParams.words_per_row == 0)
		{
			// print mem addresses
			mvprintw(y++, m_hexViewParams.x_start_addr, "0x%08x", addr_ptr+=m_hexViewParams.words_per_row);
		}
		// highlight words
		bool printHighlighted = false;
		if ((numWords + _offset_words >= m_selectedStartWord) && (selectedNumWords < m_selectedNumWords))
		{
			printHighlighted = true;
			selectedNumWords++;
		}
		// print a word
		if (printHighlighted) attron(COLOR_PAIR(COLOR_INVERTED));
		x = m_hexViewParams.x_start_mem + m_hexViewParams.block_width * (numWords % m_hexViewParams.words_per_row);
		mvprintw(y-1, x, "%08x", *mem_ptr);
		if (printHighlighted) attroff(COLOR_PAIR(COLOR_INVERTED));

		// increase counters and step to next word
		numWords++;
		mem_ptr++;
	}
	// blank remaining blocks on row from next block to end of line
	for (int xx = x + m_hexViewParams.block_width; xx < m_hexViewParams.x_stop_mem; xx++)
		mvaddch(y-1, xx, ' ');
	// blank last line to clear previous memory blocks
	for (int yy = y; yy < m_hexViewParams.y_stop+1; yy++)
		for (int xx = m_hexViewParams.x_start_addr; xx < m_hexViewParams.x_stop_mem; xx++)
			mvaddch(yy, xx, ' ');

	#pragma endregion hex_view
	

	#pragma region bin_view

	/*
	 * Takes the currently selected words and displays them word by word
	 * in the binary view.
	 */
	// reset address pointer, add selected offset
	addr_ptr = (uint32_t*)((unsigned char*)m_memory) + m_selectedStartWord;
	
	y = m_binViewParams.y_start;

	for (int i = 0; i < m_selectedNumWords; i++)
	{
		int word_n = i % m_binViewParams.words_per_row;

		if (y < m_binViewParams.y_stop - 1)
		{
			mvprintw(y, m_binViewParams.x_start_addr + word_n * m_binViewParams.block_width, "0x%08x", addr_ptr);
			printWordBinary(y+1, m_binViewParams.x_start_addr + word_n * m_binViewParams.block_width, *(addr_ptr));
		}
		
		if (word_n - 1 == 0)
			y += 2;
		addr_ptr++;
	}

	#pragma endregion bin_view

}


//---------------------------------------------------------------------------------------
void MemoryViewer::nextPage()
{
	// new page
	//m_selectedStartWord += MIN(m_hexViewParams.word_max_view / 2, m_memNumWords);
}


//---------------------------------------------------------------------------------------
void MemoryViewer::prevPage()
{
}


//---------------------------------------------------------------------------------------
void MemoryViewer::print_debug(int _i)
{
	int i = _i;
	static int x = COLS - 40;
	mvprintw(i++, x, "m_memory:            0x%08x  ", m_memory);
	mvprintw(i++, x, "m_memSize:           %d  ", m_memSize);
	mvprintw(i++, x, "m_memOffsetWords:    %d  ", m_memOffsetWords);
	mvprintw(i++, x, "m_memNumWords:       %d  ", m_memNumWords);
	mvprintw(i++, x, "m_selectedNumWords:  %d  ", m_selectedNumWords);
	mvprintw(i++, x, "m_selectedStartWord: %d  ", m_selectedStartWord);
	mvprintw(i++, x, "m_selectedWordsMax:  %d  ", m_selectedWordsMax);
	mvprintw(i++, x, "'w' selection:       %d  ", m_selectedStartWord - m_hexViewParams.words_per_row);
	mvprintw(i++, x, "'s' selection:       %d  ", m_selectedStartWord + m_hexViewParams.words_per_row);
	i++;
	m_hexViewParams.print_debug(i);

}


//---------------------------------------------------------------------------------------
bool MemoryViewer::update(uint32_t _sleep_ms)
{
	// for sleeping 10 ms
	static struct timespec ts;
	ts.tv_sec = 0; ts.tv_nsec = 1e7;

	bool still_running = true;
	wchar_t key = getch();

	switch (key)
	{
		// decrease offset for selection in views one line
		case 'w': case 'W':
		{
			int sel = m_selectedStartWord - m_hexViewParams.words_per_row;

			if (sel >= 0)
			{
				if (sel < m_memOffsetWords)
					// prev page
					m_memOffsetWords -= m_hexViewParams.words_per_view / 2;
				m_selectedStartWord -= m_hexViewParams.words_per_row;
			}
			updateMemoryViews(m_memOffsetWords);
			break;
		}
		// increase offset for selection in views one line
		case 's': case 'S':
		{
			uint16_t sel = m_selectedStartWord + m_selectedNumWords + m_hexViewParams.words_per_row;
			if (sel <= m_memNumWords)
			{
				if ((sel > m_hexViewParams.word_max_view))
					// next page
					m_memOffsetWords += m_hexViewParams.words_per_view / 2;
				m_selectedStartWord += m_hexViewParams.words_per_row;
			}
			updateMemoryViews(m_memOffsetWords);
			break;
		}
		// decrease offset for selection in views with one word
		case 'a': case 'A':
		{
			if (m_selectedStartWord > 0)
			{
				if (m_selectedStartWord <= m_memOffsetWords)
					// prev page
					m_memOffsetWords -= m_hexViewParams.words_per_view / 2;
				//(m_selectedStartWord > m_memOffsetWords))
				m_selectedStartWord--;
			}
			updateMemoryViews(m_memOffsetWords);
			break;
		}
		// increase offset for selection in views with one word
		case 'd': case 'D':
		{
			uint16_t sel = m_selectedStartWord + m_selectedNumWords;
			if (sel < m_memNumWords)
			{
				if (sel >= m_hexViewParams.word_max_view)
					m_memOffsetWords += m_hexViewParams.words_per_view / 2;
				m_selectedStartWord++;
			}
			updateMemoryViews(m_memOffsetWords);
			break;
		}
		// increase word selection in views
		case 'e': case 'E':
		{
			if ((m_selectedNumWords < m_selectedWordsMax) && 
				(m_selectedStartWord + m_selectedNumWords < m_hexViewParams.word_max_view) &&
				(m_selectedStartWord + m_selectedNumWords < m_memNumWords))
				m_selectedNumWords++;
			updateMemoryViews(m_memOffsetWords);
			break;
		}
		// decrease word selection in views
		case 'q': case 'Q':
		{
			if (m_selectedNumWords > 1)
				m_selectedNumWords--;
			displayWindow(&m_binWindow, false);
			displayWindow(&m_binWindow, true);
			updateMemoryViews(m_memOffsetWords);
			break;
		}
		
		// DEBUG : increase view offset
		case 'k': case 'K':
		{
			if (m_memOffsetWords < (m_memSize / WORD_SIZE) - 1)
			{
				m_memOffsetWords++;
				if (m_selectedStartWord < (m_memSize / WORD_SIZE) - 1)
					m_selectedStartWord++;
			}
			updateMemoryViews(m_memOffsetWords);
			break;
		}
		case 'l': case 'L':
		{
			if (m_memOffsetWords > 0)
			{
				m_memOffsetWords--;
				m_selectedStartWord--;
			}
			updateMemoryViews(m_memOffsetWords);
			break;
		}
		
		// exit?
		case 'x': case 'X':
		{
			still_running = false;
			break;
		}

		default:
			break;
	}

	//
	displayDynamicElements();

	// sleep for 1 ms
	nanosleep(&ts, &ts);

	return still_running;

}


//---------------------------------------------------------------------------------------
// PRIVATE MEMBER FUNCTIONS
//
void MemoryViewer::printWordBinary(int _y, int _x, uint32_t _word_val)
{
	static uint8_t width = WORD_SIZE * 8;
	uint8_t x = _x;
	for (int i = width-1; i >= 0; i--)
	{
		mvaddch(_y, x, (_word_val & (1 << i)) ? '1' : '0');
		x++;
		if (!(i%8)) x++;
	}	
}



