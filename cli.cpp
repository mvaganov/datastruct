#include "license.txt"
#include "cli.h"
#include "clibuffer.h"
#include <stdlib.h>	// malloc for custom sprintf implementation
#include <stdio.h>	// putchar, printf, reading from STDIN
#include "mem.h"

const char * CLI::version = "0.0.3";

/** the current CLI. if not initialized, nothing works. */
static CLI::CommandLineInterface * g_CLI = 0;

// <prototypes>

/** data struct for key escape sequences */
struct EscapeTranslate{const int bytes; const char * sequence; const int code;};
/** which escape sequences are valid on this platform */
extern const EscapeTranslate * g_ESC_SEQ_LIST;
/** how many valid escape sequences there are */
extern const int g_ESC_SEQ_LIST_SIZE;
/** platform-specific cursor move function */
void platform_move(int row, int col);
// TODO work with windows
void platform_setColor(int foreground, int background);
// TODO work with windows
bool platform_kbhit();
// TODO work with windows
int platform_getchar();
/** helper function for translating input bytes if an escape sequence is found */
int returnKey(const EscapeTranslate * list, const int listCount, CLI::CommandLineInterface * a_CLI);

// </prototypes>

#define _K(name)	CLI::KEY::name
const EscapeTranslate listWin[] = {
		{2, "\0\073",_K(F1)},
		{2, "\0\074",_K(F2)},
		{2, "\0\075",_K(F3)},
		{2, "\0\076",_K(F4)},
		{2, "\0\077",_K(F5)},
		{2, "\0G",_K(HOME)},
		{2, "\0H",_K(UP)},
		{2, "\0I",_K(PAGE_UP)},
		{2, "\0K",_K(LEFT)},
		{2, "\0M",_K(RIGHT)},
		{2, "\0O",_K(END)},
		{2, "\0P",_K(DOWN)},
		{2, "\340Q",_K(PAGE_DOWN)},
		{2, "\0\100",_K(F6)},
		{2, "\0\101",_K(F7)},
		{2, "\0\102",_K(F8)},
		{2, "\0\103",_K(F9)},
		{2, "\0\104",_K(F10)},
		{2, "\340H",_K(UP)},
		{2, "\340\205",_K(F11)},
		{2, "\340\206",_K(F12)},
		{2, "\340G",_K(HOME)},
		{2, "\340H",_K(UP)},
		{2, "\340I",_K(PAGE_UP)},
		{2, "\340K",_K(LEFT)},
		{2, "\340M",_K(RIGHT)},
		{2, "\340O",_K(END)},
		{2, "\340P",_K(DOWN)},
		{2, "\340Q",_K(PAGE_DOWN)},
		{2, "\340R",_K(INSERT)},
		{2, "\340S",_K(DELETEKEY)},
};
const EscapeTranslate listTTY[] = {
		{3, "\033OH",_K(HOME)},
		{3, "\033OF",_K(END)},
		{3, "\033OQ",_K(F2)},
		{3, "\033OR",_K(F3)},
		{3, "\033OS",_K(F4)},
		{5, "\033[15~",_K(F5)},
		{5, "\033[17~",_K(F6)},
		{5, "\033[18~",_K(F7)},
		{5, "\033[19~",_K(F8)},
		{3, "\033[A",_K(UP)},
		{3, "\033[B",_K(DOWN)},
		{3, "\033[C",_K(RIGHT)},
		{3, "\033[D",_K(LEFT)},
		{4, "\033[2~",_K(INSERT)},
		{5, "\033[20~",_K(F9)},
		{5, "\033[24~",_K(F12)},
		{4, "\033[3~",_K(DELETEKEY)},
		{4, "\033[5~",_K(PAGE_UP)},
		{4, "\033[6~",_K(PAGE_DOWN)},
};
#undef _K

void CLI::CommandLineInterface::move(int row, int col)
{
	if(m_softwareCLIdata){
		m_softwareCLIdata->move(row,col);
	}else{
		::platform_move(row, col);
	}
}

#define USER_DIDNOT_DEFINE_SIZE	-1
CLI::CommandLineInterface::~CommandLineInterface()
{
	m_userSize.set(USER_DIDNOT_DEFINE_SIZE,USER_DIDNOT_DEFINE_SIZE);
	if(m_softwareCLIdata){
		DELMEM(m_softwareCLIdata);
		m_softwareCLIdata = 0;
	}
}

CLI::CommandLineInterface::CommandLineInterface()
	:m_userSize(USER_DIDNOT_DEFINE_SIZE,USER_DIDNOT_DEFINE_SIZE),
	 m_softwareCLIdata(0),m_inputBufferSize(0),
	 m_fcolor(CLI::COLOR::DEFAULT), m_bcolor(CLI::COLOR::DEFAULT)
{}

/** @return if using double buffering */
int CLI::CommandLineInterface::getBufferCount(){
	if(!m_softwareCLIdata)return 0;
	return m_softwareCLIdata->getNumberOfBuffers();
}

void CLI::CommandLineInterface::resetColor()
{
	this->setColor(CLI::COLOR::DEFAULT, CLI::COLOR::DEFAULT);
}

/**
 * @param a_numberOfBuffers 0 for no buffering (use platform buffer),
 * 1 to manage a buffer internally (required for non-standard i/o)
 * 2 to keep track of what is printed, reducing redraws where possible
 */
void CLI::CommandLineInterface::setBufferCount(int a_numberOfBuffers)
{
	if(a_numberOfBuffers > 0){
		if(!m_softwareCLIdata){
			m_softwareCLIdata = NEWMEM(CLIBuffer());
		}
		m_softwareCLIdata->reinitializeDoubleBuffer(a_numberOfBuffers,
				CLIBuffer::Coord(CLI::getWidth(),CLI::getHeight()));
	}else{
		if(m_softwareCLIdata){
			DELMEM(m_softwareCLIdata);
			m_softwareCLIdata = 0;
		}
	}
}

/** force the width and height. -1 for either value to use the default */
void CLI::CommandLineInterface::setSize(int width, int height)
{
	m_userSize.set(width,height);
	if(m_softwareCLIdata)
		m_softwareCLIdata->internalDoubleBufferConsistencyEnforcement(
				CLIBuffer::Coord(CLI::getWidth(),CLI::getHeight()));
}

/** @param a_numbytes how much to absorb of the (beginning end) input buffer */
void CLI::CommandLineInterface::inputBufferConsume(unsigned int a_numBytes)
{
	if(a_numBytes == m_inputBufferSize)
		m_inputBufferSize = 0;
	else
	{
		int max = m_inputBufferSize-a_numBytes;
		for(int i = 0; i < max; ++i)
		{
			m_inputBuffer[i] = m_inputBuffer[i+a_numBytes];
		}
		m_inputBufferSize -= a_numBytes;
	}
}

/** place a single character at the current cursor, advancing it */
void CLI::CommandLineInterface::putchar(const int a_letter)
{
	if(m_softwareCLIdata && m_softwareCLIdata->getNumberOfBuffers() > 0){
		m_softwareCLIdata->putchar(a_letter, CLI::getFcolor(), CLI::getBcolor());
	}else{
#ifdef CLI_DEBUG
		// some how, characters are being put into an unbuffered CLI
		::printf("CommandLineInterface::putchar printing without buffer\n");
		int i=0;i=1/i;
#endif
		::putchar(a_letter);
	}
}

/** fills the entire screen with the given character */
void CLI::CommandLineInterface::fillScreen(char a_letter)
{
	this->move(0,0);
	int numSpaces = this->getWidth()*this->getHeight();
	for(int i = 0; i < numSpaces; ++i){
		this->putchar(a_letter);
	}
}

void CLI::CommandLineInterface::puts( const char * str, const int size ){
	for(int i = 0; i < size && str[i]; ++i)
		this->putchar(str[i]);
}

#include <stdarg.h>	// for va_list and va_start
int CLI::CommandLineInterface::printf( const char * fmt, ... )
{
	int size = 40, n;
	char * buffer = NULL;
	do
	{
		size *= 2;
		va_list valist;
		va_start( valist, fmt );
		buffer = (char*)realloc( buffer, size );

#if __STDC_WANT_SECURE_LIB__
		n = vsnprintf_s( buffer, size, _TRUNCATE, fmt, valist );
#else
		n = vsnprintf( buffer, size, fmt, valist );
#endif
		va_end( valist );
	}
	while( n >= size );
	this->puts(buffer, size);
	free(buffer);
	return n;
}

/** @return true if there is a key press waiting to be read */
bool CLI::CommandLineInterface::kbhit(){return (m_inputBufferSize > 0);}

int CLI::CommandLineInterface::getchar()
{
	return returnKey(g_ESC_SEQ_LIST, g_ESC_SEQ_LIST_SIZE, this);
}

/** determine how many columns the screen is */
int CLI::CommandLineInterface::getWidth(){return m_userSize.x;}

/** determine how many rows the screen is */
int CLI::CommandLineInterface::getHeight(){return m_userSize.y;}

/** @return foreground color */
int CLI::CommandLineInterface::getFcolor(){return m_fcolor;}

/** @return background color */
int CLI::CommandLineInterface::getBcolor(){return m_bcolor;}

/** use CLI::COLOR values for foreground and background */
void CLI::CommandLineInterface::setColor(int foreground, int background)
{
	m_fcolor = foreground;
	m_bcolor = background;
	if(!m_softwareCLIdata)
		platform_setColor(foreground, background);
}

void CLI::CommandLineInterface::refresh_stdout()
{
	if(getBufferCount() > 0){
		m_softwareCLIdata->draw();
	}else{
		// some how, characters are being put into an unbuffered CLI
		int i=0;i=1/i;
		printf("CommandLineInterface::refresh_stdout without a buffer\n");
	}
}

////////////////////////////////////////////////////////////////////////////////

/**
 *
 * @return the current CLI environment object. Useful if there
 * is a multi-tabbed command-line interface, or a game with multiple CLI
 * accessible.
 */
CLI::CommandLineInterface * CLI::getCLI()
{
	return g_CLI;
}

/**
 * Sets the current CLI environment object to another one. Useful if there
 * is a multi-tabbed command-line interface, or a game with multiple CLI
 * accessible.
 *
 * Be sure to delete the current CLI
 */
void CLI::setCLI(CLI::CommandLineInterface * a_CLI)
{
	g_CLI = a_CLI;
}

void CLI::move(int row, int col)
{
	if(g_CLI){
		g_CLI->move(row,col);
	}else{
		::platform_move(row, col);
	}
}

/** @return if using double buffering */
bool CLI::isDoubleBuffered(){return g_CLI->getBufferCount()==2;}

void CLI::resetColor(){
	if(g_CLI)
		g_CLI->resetColor();
	else
		platform_setColor(CLI::COLOR::WHITE, CLI::COLOR::BLACK);
}

/** @param a_2Bufr whether or not to enable double buffering */
void CLI::setDoubleBuffered(bool a_2Bufr){g_CLI->setBufferCount(a_2Bufr?2:0);}

////////////////////////////////////////////////////////////////////////////////

/** force the width and height. -1 for either value to use the default */
void CLI::setSize(int width, int height){g_CLI->setSize(width,height);}

/**
 * @param value __OUT will be the next special value from the input buffer
 * @param bytesRead __OUT how many bytes make up the next special value
 * @param list listWin or listTTY
 * @param listCount sizeof(list___)/sizeof(*list___)
 */
void translateSpecialCharacters(int & value, int & bytesRead,
		const EscapeTranslate * list, const int listCount, CLI::CommandLineInterface * a_this)
{
	int len;
	bool found = false;
	char * bufr = a_this->getInputBuffer();
	int bufrSize = a_this->getInputBufferSize();
	for(int li = 0; !found && li < listCount; ++li) {
		len = list[li].bytes;
		found = true;
		for(int i = 0; found && i < len; ++i) {
			if(i >= bufrSize)break;
			found = (list[li].sequence[i] == bufr[i]);
		}
		if(found) {
			value = list[li].code;
			bytesRead = len;
		}
	}
//#define TESTING_KEYCODES
#ifdef TESTING_KEYCODES
	// really ugly test code to discover new special key sequences
	if(!value && (bufrSize) > 1){
		CLI::printf("{");
		int v;
		bool isPrintable;
		for(int i = 0; i < bufrSize; ++i)
		{
			v = bufr[i];
#define Or_(x, a,b,c,d)	x==a||x==b||x==c||x==d
			isPrintable = !(Or_(v,'\0','\a','\b','\t')
						 || Or_(v,'\r','\n',-1,'\033'));
#undef Or_
			CLI::printf("(%2X,\'%c\')", v, isPrintable?v:' ');
		}
		CLI::printf("} byte size: %d", bufrSize);
		value = 0;
		bytesRead = bufrSize;
	}
#endif
}

/**
 * @param list what the special key sequences are
 * @param listCount how many special key sequences there are
 * @param a_CLI the command line interface to get the data from
 */
int returnKey(const EscapeTranslate * list, const int listCount, CLI::CommandLineInterface * a_CLI)
{
	int bufrSize = a_CLI->getInputBufferSize();
	char * bufr = a_CLI->getInputBuffer();
	// if a multi-byte escape sequence was found in the input buffer
	if(bufrSize > 1){
		int value = 0, bytesRead = 0;
		translateSpecialCharacters(value, bytesRead, list, listCount, a_CLI);
		if(bytesRead){
			a_CLI->inputBufferConsume(bytesRead);
			return value;
		}
	}
	// if there is input in the buffer at all
	if(bufrSize > 0)
	{
		int result = bufr[0];
		a_CLI->inputBufferConsume(1);
		return result;
	}
	// buffer exhausted
	return EOF;
}

/** place a single character at the current cursor, advancing it */
void CLI::putchar(int a_letter)
{
	if(g_CLI && g_CLI->getBufferCount() > 0){
		g_CLI->putchar(a_letter);
	}else{
		::putchar(a_letter);
	}
}

/** fills the entire screen with the given character */
void CLI::fillScreen(char a_letter){
#ifdef CLI_DEBUG
	if(g_CLI){
		g_CLI->fillScreen(a_letter);
	}else{
		// this should not be happening...
		::printf("fillscreen without initialized CLI\n");
		int i=0;i=1/i;
	}
#else
	g_CLI->fillScreen(a_letter);
#endif
}

#include <stdarg.h>	// for va_list and va_start
int CLI::printf( const char * fmt, ... )
{
	int size = 40, n;
	char * buffer = NULL;
	do{
		size *= 2;
		va_list valist;
		va_start( valist, fmt );
		buffer = (char*)realloc( buffer, size );
#ifndef __STDC_WANT_SECURE_LIB__
		n = vsnprintf( buffer, size, fmt, valist );
#else
		n = vsnprintf_s( buffer, size, size, fmt, valist );
#endif
		va_end( valist );
	}
	while( n >= size );
#ifdef CLI_DEBUG
	if(g_CLI){
		g_CLI->puts(buffer, size);
	}else{
		::printf("CLI::printf called on uninitialized CLI\n");
		free(buffer);
		int i=0;i=1/i;
	}
#else
	g_CLI->puts(buffer, size);
#endif
	free(buffer);
	return n;
}

/** @return key press from input buffer, waits if none. {@link kbhit()} */
int CLI::getcharBlocking()
{
	refresh();
	while(!CLI::kbhit())
		CLI::sleep(1);
	return CLI::getchar();
}

/** @return key press from input buffer, waits if none. {@link kbhit()} */
int CLI::getcharBlocking(int a_ms)
{
	refresh();
	long long soon = CLI::upTimeMS()+a_ms;
	while(!CLI::kbhit() && CLI::upTimeMS() < soon)
		CLI::sleep(1);
	return CLI::getchar();
}

#ifdef _WIN32
#include <windows.h>	// move cursor, change color, sleep
#include <conio.h>		// non-blocking input
#include <time.h>		// clock()
#undef DELETE		// something in windows defines DELETE... so #undef ...
#include "cli.h"	// ... then re-#define

HANDLE __WINDOWS_CLI = 0;
/** keep track of the old terminal settings */
WORD oldAttributes;

const EscapeTranslate * g_ESC_SEQ_LIST = listWin;
const int g_ESC_SEQ_LIST_SIZE = sizeof(listWin)/sizeof(*listWin);

/** call at the beginning of the command-line application */
void CLI::init()
{
	__WINDOWS_CLI = GetStdHandle(STD_OUTPUT_HANDLE);
	if(!g_CLI)
	{
		CONSOLE_SCREEN_BUFFER_INFO lpConsoleScreenBufferInfo;
		GetConsoleScreenBufferInfo(__WINDOWS_CLI, &lpConsoleScreenBufferInfo);
		oldAttributes = lpConsoleScreenBufferInfo.wAttributes;
		// make ::getchar() read right at the key press, without echoing
		g_CLI = NEWMEM(CLI::CommandLineInterface);
		g_CLI->setSize(CLI::getWidth(), CLI::getHeight());
	}
}

/** call at the end of the command-line application */
void CLI::release()
{
	CLI::resetColor();
	if(g_CLI)
	{
		resetColor();
		::putchar('\n');
		DELMEM(g_CLI);
		g_CLI = 0;
	}
	__WINDOWS_CLI = 0;
}

/** determine how many columns the screen is */
int CLI::getWidth()
{
	int v = USER_DIDNOT_DEFINE_SIZE;
	if(g_CLI)v=g_CLI->getWidth();
	if(v == USER_DIDNOT_DEFINE_SIZE)
	{
		CONSOLE_SCREEN_BUFFER_INFO lpConsoleScreenBufferInfo;
		GetConsoleScreenBufferInfo(__WINDOWS_CLI, &lpConsoleScreenBufferInfo);
		return lpConsoleScreenBufferInfo.dwSize.X;
	}
	return v;
}

/** determine how many rows the screen is */
int CLI::getHeight()
{
	int v = USER_DIDNOT_DEFINE_SIZE;
	if(g_CLI)v=g_CLI->getHeight();
	if(v == USER_DIDNOT_DEFINE_SIZE)
	{
		CONSOLE_SCREEN_BUFFER_INFO lpConsoleScreenBufferInfo;
		GetConsoleScreenBufferInfo(__WINDOWS_CLI, &lpConsoleScreenBufferInfo);
		return lpConsoleScreenBufferInfo.dwSize.Y;
	}
	return v;
}

/** @return foreground color */
int CLI::getFcolor()
{
	if(g_CLI)return g_CLI->getFcolor();
	CONSOLE_SCREEN_BUFFER_INFO lpConsoleScreenBufferInfo;
	GetConsoleScreenBufferInfo(__WINDOWS_CLI, &lpConsoleScreenBufferInfo);
	return lpConsoleScreenBufferInfo.wAttributes & 0xf;
}

/** @return background color */
int CLI::getBcolor()
{
	if(g_CLI)return g_CLI->getBcolor();
	CONSOLE_SCREEN_BUFFER_INFO lpConsoleScreenBufferInfo;
	GetConsoleScreenBufferInfo(__WINDOWS_CLI, &lpConsoleScreenBufferInfo);
	return (lpConsoleScreenBufferInfo.wAttributes & 0xf0) >> 4;
}

/** refresh the screen */
void CLI::refresh()
{
	if(isDoubleBuffered())
	{
		g_CLI->refresh_stdout();
	}
}

/** move the cursor to the given location in the console */
void platform_move(int row, int col)
{
	COORD p = {(short)col, (short)row};
	::SetConsoleCursorPosition(__WINDOWS_CLI, p);
}

bool platform_kbhit()
{
	return ::_kbhit() != 0;
}

int platform_getchar()
{
	return ::_getch();
}

void platform_setColor(int foreground, int background)
{
	if(foreground < 0){	foreground = oldAttributes & 0xf;			}
	if(background < 0){	background = (oldAttributes & 0xf0) >> 4;	}
	::SetConsoleTextAttribute(__WINDOWS_CLI,
			(foreground & 0xf) | ( (background & 0xf) << 4) );
}

/** non-blocking key press, returns 0 if there is no key */
int CLI::getchar()
{
	if(g_CLI){
		int bufrSize = g_CLI->getInputBufferSize();
		char * bufr = g_CLI->getInputBuffer();
		while(platform_kbhit()){
			bufr[bufrSize++] = platform_getchar();
		}
		g_CLI->setInputBufferSize(bufrSize);
		return g_CLI->getchar();
	}else{
		if(platform_kbhit()){
			return platform_getchar();
		}
		return -1;
	}
	//while(::_kbhit()){
	//	m_inputBuffer[m_inputBufferSize++] = ::_getch();
	//}
	//return returnKey(listWin, sizeof(listWin)/sizeof(*listWin));
}

/** use CLI::COLOR values for foreground and background */
void CLI::setColor(int foreground, int background)
{
	if(g_CLI){
		g_CLI->setColor(foreground, background);
	}else{
		platform_setColor(foreground, background);
	}
}


/** pause the CPU for the given number of milliseconds */
void CLI::sleep(int ms)
{
	::Sleep(ms);
}

/** how many milliseconds since initialization */
long long CLI::upTimeMS()
{
	return ::clock();
}

/** @return true if there is a key press waiting */
bool CLI::kbhit()
{
	if(g_CLI)
		return g_CLI->kbhit();
	return (platform_kbhit()) != 0;
}

/** @return true if initialized */
bool CLI::isInitialized()
{
	return g_CLI != 0;
}

#else

#include <unistd.h>		// sleep
#include <sys/select.h>	// select, fd_set (for raw, low-level access to input)
#include <sys/time.h>	// for wall-clock timer (as opposed to clock cycle timer)
#include <sys/ioctl.h>	// ioctl
#include <termios.h>	// terminal i/o settings

// using ANSI console features to control color and cursor position by default.
// http://en.wikipedia.org/wiki/ANSI_escape_code#graphics

// experimented with ncurses. the TTY ANSI console is plenty powerful already...
// http://tldp.org/HOWTO/NCURSES-Programming-HOWTO/index.html

/** keep track of the old terminal settings */
termios oldTerminalIOSettings;
/** Linux keeps track of time this way. clock() returns CPU cycles, not time. */
timeval g_startTime;
/** how long to wait while checking kbhit */
timeval g_tv = {0,0};
/** input check during kbhit */
fd_set g_fds;

bool platform_kbhit()
{
	// check the hardware input stream if there is data waiting
	FD_SET(STDIN_FILENO, &g_fds);
	int result = ::select(STDIN_FILENO+1, &g_fds, NULL, NULL, &g_tv);
	// specifically, check for data to be read
	return result && (FD_ISSET(0, &g_fds));
}

int platform_getchar()
{
	int buffer;
	::read(STDIN_FILENO, (char*)&buffer, sizeof(buffer));
	return buffer;
}

/** @return true if there is a key press waiting to be read */
bool CLI::kbhit()
{
	if(g_CLI && g_CLI->kbhit())	// if there is data in the software input buffer
		return true;
	return platform_kbhit();
}

const EscapeTranslate * g_ESC_SEQ_LIST = listTTY;
const int g_ESC_SEQ_LIST_SIZE = sizeof(listTTY)/sizeof(*listTTY);

int CLI::getchar()
{
	if(g_CLI){
		if(platform_kbhit()){
			int bufrSize = g_CLI->getInputBufferSize();
			char * bufr = g_CLI->getInputBuffer();
			// add something to the input buffer if there is anything
			int bytesInHardware = ::read(STDIN_FILENO, (bufr+bufrSize),
					sizeof(bufr)-bufrSize);
			bufrSize += bytesInHardware;
			g_CLI->setInputBufferSize(bufrSize);
		}
		return g_CLI->getchar();
	}else{
		if(platform_kbhit()){
			return platform_getchar();
		}
		return -1;
	}
}

/** call at the beginning of the command-line application */
void CLI::init()
{
	if(!g_CLI)
	{
		// make ::getchar() read right at the key press, without echoing
		tcgetattr( STDIN_FILENO, &oldTerminalIOSettings );
		termios currentTerminalIOSettings = oldTerminalIOSettings;
		currentTerminalIOSettings.c_lflag &= ~( ICANON | ECHO );	// don't wait for enter, don't print
		tcsetattr( STDIN_FILENO, TCSANOW, &currentTerminalIOSettings );
		FD_ZERO(&g_fds);	// initialize the struct that checks for input
		gettimeofday(&g_startTime, NULL);	// start the timer
		g_CLI = NEWMEM(CLI::CommandLineInterface);
		g_CLI->setSize(CLI::getWidth(), CLI::getHeight());
	}
}

/** @return true if initialized */
bool CLI::isInitialized()
{
	return g_CLI != 0;
}

/** call at the end of the command-line application */
void CLI::release()
{
	CLI::resetColor();
	platform_setColor(g_CLI->getFcolor(), g_CLI->getBcolor());
	if(g_CLI)
	{
		tcsetattr( STDIN_FILENO, TCSANOW, &oldTerminalIOSettings );
		resetColor();
		::putchar('\n');
		DELMEM(g_CLI);
		g_CLI = 0;
	}
}

/** determine how many columns the screen is */
int CLI::getWidth()
{
	int v = g_CLI->getWidth();
	if(v == USER_DIDNOT_DEFINE_SIZE)
	{
		winsize ws;
		ioctl(STDIN_FILENO, TIOCGWINSZ, &ws);
		return ws.ws_col;
	}
	return v;
}

/** determine how many rows the screen is */
int CLI::getHeight()
{
	int v = g_CLI->getHeight();
	if(v == USER_DIDNOT_DEFINE_SIZE)
	{
		winsize ws;
		ioctl(STDIN_FILENO, TIOCGWINSZ, &ws);
		return ws.ws_row;
	}
	return v;
}

/** @return foreground color */
int CLI::getFcolor(){return g_CLI->getFcolor();}

/** @return background color */
int CLI::getBcolor(){return g_CLI->getBcolor();}

/** refresh the screen */
void CLI::refresh()
{
	if(g_CLI && g_CLI->getBufferCount() > 0){
		g_CLI->refresh_stdout();
	}
	fflush(stdout); // force stdout refresh
}

/** move the cursor to the given location in the console */
void platform_move(int row, int col)
{
	if(row < 0 || col < 0 || row >= CLI::getHeight() || col >= CLI::getWidth())return;
	::printf("\033[%d;%df", row+1, col+1);	// move cursor, without ncurses
}

void platform_setColor(int foreground, int background)
{
	// colorRGB and colorGRAY usable for TTY (unix/linux) expanded console color
	if(foreground >= 0)
		::printf("\033[38;5;%dm", foreground);
	else
		::printf("\033[39m");// default foreground color
	if(background >= 0){
		::printf("\033[48;5;%dm", background);
	}else{
		::printf("\033[49m");// default background color
	}
}

// R, G, and B values must be between 0 and 5 (inclusive)
#define colorRGB(R,G,B) (16+(B+(G*6)+(R*36)))
// GRAY must be between 0 and 23 (inclusive)
#define colorGRAY(GRAY)	(232+GRAY)

/** use CLI::COLOR values for foreground and background */
void CLI::setColor(int foreground, int background)
{
	if(g_CLI){
		g_CLI->setColor(foreground, background);
	}else{
		platform_setColor(foreground, background);
	}
}

/** pause the CPU for the given number of milliseconds */
void CLI::sleep(int a_ms)
{
	static timeval endTime, startTime;
	static __time_t seconds, useconds, ms;
	gettimeofday(&startTime, NULL);
	ms = 0;
	while(ms < a_ms)
	{
		usleep(128);	// sleeps "microseconds worth" of CPU-instructions
		gettimeofday(&endTime, NULL);					// but we must calculate
		seconds  = endTime.tv_sec  - startTime.tv_sec;	// how much time was
		useconds = endTime.tv_usec - startTime.tv_usec;	// really spent
		ms = seconds*1000 + useconds/1000;
	}
}

long long CLI::upTimeMS()
{
	static timeval now;
	static __time_t seconds, useconds, ms;
	gettimeofday(&now, NULL);
	seconds  = now.tv_sec  - g_startTime.tv_sec;
	useconds = now.tv_usec - g_startTime.tv_usec;
	ms = seconds*1000 + useconds/1000;
	return ms;
}
#endif


void CLIBuffer::reinitializeDoubleBuffer(int a_numberOfBuffers, Coord a_size)
{
	int w = a_size.x;
	int h = a_size.y;
	bool brandNewAllocation = w != m_size.x || h != m_size.y || !m_mainBuffer;
	size_t count = w*h;
	if(brandNewAllocation)
	{
		CLIOutputType * buffer = NEWMEM_ARR(CLIOutputType, count);
		for(unsigned int i = 0; i < count; ++i){
			buffer[i].set(WHITESPACE,CLI::COLOR::DEFAULT,CLI::COLOR::DEFAULT);
		}
		// if the old buffer is being resized
		if(m_mainBuffer)
		{
			// copy all of the old data into the new buffer
			Coord c;
			for(int row = 0; row < h; ++row)
			{
				for(int col = 0; col < w; ++col)
				{
					c.set(col, row);
					buffer[col+row*w] = *getAt(c);
				}
			}
			// and kill the old data
			DELMEM_ARR(m_mainBuffer);
		}
		m_mainBuffer = buffer;
	}
	if((brandNewAllocation && m_historyBuffer)
	|| (a_numberOfBuffers >= 2 && !m_historyBuffer))
	{
		CLIOutputType * buffer2 = NEWMEM_ARR(CLIOutputType, count);
		for(unsigned int i = 0; i < count; ++i){
			buffer2[i].set(WHITESPACE+1,CLI::COLOR::DEFAULT,CLI::COLOR::DEFAULT);
		}
		if(m_historyBuffer)
			DELMEM_ARR(m_historyBuffer);
		m_historyBuffer = buffer2;
	}
	if(m_cursor.x >= w)	m_cursor.x = w-1;
	if(m_cursor.y >= h)	m_cursor.y = h-1;
	m_size.set(w, h);
}

void CLIBuffer::release()
{
	if(m_mainBuffer)
	{
		DELMEM_ARR(m_mainBuffer);
		DELMEM_ARR(m_historyBuffer);
	}
	m_mainBuffer = 0;
	m_historyBuffer = 0;
	m_size.set(0,0);
}

void CLIBuffer::draw()
{
	platform_move(0,0);
	CLI::resetColor();
	/** cursor checking to see if a character should print here */
	Coord c;
	/** where the cursor will be next */
	Coord n;
	CLIOutputType * cell;
	int fcolor = CLI::getFcolor(), bcolor = CLI::getBcolor();
//#define TEST_DIRTY_CELL
#ifdef TEST_DIRTY_CELL
	static int test = 0;
#endif
	bool moved;
	for(c.y = 0; c.y < m_size.y; ++c.y)
	{
		for(c.x = 0; c.x < m_size.x; ++c.x)
		{
			if(changedSinceLastDraw(c))
			{
				int index = correctIndex(c);
				cell = &m_mainBuffer[index];
				if((moved = c!=n || c.y==0))
				{
					platform_move(c.y,c.x);
				}
				if(moved
				|| cell->fcolor != fcolor
				|| cell->bcolor != bcolor)
				{
					fcolor = cell->fcolor;
					bcolor = cell->bcolor;
					platform_setColor(fcolor, bcolor);
				}
#ifdef TEST_DIRTY_CELL
				platform_setColor(fcolor, test);
#endif
				::putchar(cell->letter);
				if(m_historyBuffer){
					m_historyBuffer[index] = m_mainBuffer[index];
				}
				c = n;
				n.x++;
				if(n.x >= m_size.x){
					n.x = 0;
					n.y++;
				}
			}
		}
	}
#ifdef TEST_DIRTY_CELL
	test++;
	if(test>255)test=-1;
#endif
	platform_move(m_cursor.y, m_cursor.x);
	CLI::resetColor();
}

void CLIBuffer::printTotallyEmptyChar()
{
	this->putcharDirect(WHITESPACE, CLI::COLOR::DEFAULT, CLI::COLOR::DEFAULT);
}
