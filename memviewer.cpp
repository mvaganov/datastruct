#include "memviewer.h"
#include "mem.h"
#include "limits.h"
#include "templatevector.h"

class MemViewer
{
	typedef unsigned char UCHAR_TYPE;

	UCHAR_TYPE * g_ptr;

	int cursorIndex, cursorAnimation;

	int width, height;

	bool viewHexArea, viewCharArea, viewInfoArea;

	TemplateVector<UCHAR_TYPE*> m_history;

public:
	MemViewer()
		:g_ptr(0),cursorIndex(0),cursorAnimation(0),
		 width(0x10),height(10),
		 viewHexArea(true),viewCharArea(true),viewInfoArea(true),
		 confirmMessage(0),confirmInput(0),m_userWantsToQuit(false)
	{g_ptr=((UCHAR_TYPE*)this);}

	#define HEXCOL	2
	#define HEXCOLTOTAL	(HEXCOL+1)

	static int filterSafeColor(unsigned char & c)
	{
		static const int defaultcolor = CLI::COLOR::WHITE;
		int color = defaultcolor;
	#define COLOR_RANGES(rmin, rmax, colorvalue, numtype)\
			if(c >= rmin && c < rmax){			\
				color = colorvalue;				\
				c -= rmin;						\
				if(numtype == 0)				\
					c += (c < 10)?'0':('a'-10);	\
				else							\
					c += 0x40;					\
			}
		COLOR_RANGES(0x00, 0x20, CLI::COLOR::BRIGHT_BLUE,0)
		else COLOR_RANGES(0x80, 0xc0, CLI::COLOR::BRIGHT_CYAN,1)
		else COLOR_RANGES(0xc0, 0x100, CLI::COLOR::BRIGHT_RED,1)
		switch(c){
		case 127:	c = '!';	break;
		case ' ':	if(color != defaultcolor)	c = '-';	break;
		}
	#undef COLOR_RANGES
		return color;
	}

	static int getSafeColor(unsigned char c)
	{
		return filterSafeColor(c);
	}

	/**
	 * @param c will be printed as a plain-text letter with a color that indicates
	 * it's true value
	 */
	static void printCharSafe(unsigned char c, CLI::CommandLineInterface * a_CLI){
		char fcolor = a_CLI->getFcolor();
		char color = filterSafeColor(c);
		if(color != fcolor){	a_CLI->setColor(color);}
		a_CLI->putchar(c);
		if(color != fcolor){	a_CLI->setColor(fcolor);}
	}

	/**
	 * @param whichDigits 0: none, 1: least sig., 2: most sig., 3: both
	 */
	static void printHexByte(char byte, int whichDigits, CLI::CommandLineInterface * a_CLI)
	{
		unsigned char c;
		if(whichDigits & 2)
		{
			c = (byte & 0xf0) >> 4;
			c += (c < 10)?'0':('a'-10);
			a_CLI->putchar(c);
		}
		if(whichDigits & 1)
		{
			c = byte & 0x0f;
			c += (c < 10)?'0':('a'-10);
			a_CLI->putchar(c);
		}
	}

	/**
	 * @param byte what to print
	 * @param startDigit which binary digit to print last (least significant)
	 * @param endDigit which binary digit to start printing after (most significant)
	 * @param digitsPrinted __OUT how many digits have been printed already
	 * @param spaced how often to put a space between digits (0 for none)
	 */
	static void printBinaryByte(char byte, int startDigit, int endDigit,
			int & digitsPrinted, int spaced)
	{
		for(int i = endDigit-1; i >= startDigit; --i)
		{
			CLI::putchar( ( byte & (1 << i) )?'1':'0' );
			if(spaced && ++digitsPrinted >= spaced){CLI::putchar(' ');digitsPrinted=0;}
		}
	}
	/**
	 * @param bytes memory to print as binary
	 * @param numDigits how many binary digits to print
	 * @param bigEndian whether or not this computer is using big endian or not
	 * @param spaced how often to print make a space (0 for none)
	 */
	static void printBinary(void * bytes, int numDigits, bool bigEndian, int spaced)
	{
		char * b = (char*)bytes;
		int bytesToRead = numDigits/CHAR_BIT;
		int index;
		int digitsPrinted = 0;
		if((numDigits % CHAR_BIT) && !bigEndian)
		{
			printBinaryByte(b[bytesToRead], 0, numDigits % CHAR_BIT,  digitsPrinted, spaced);
		}
		for(int i = 0; i < bytesToRead; ++i)
		{
			index = (!bigEndian)?(bytesToRead-1-i):i;
			printBinaryByte(b[index], 0, 8,  digitsPrinted, spaced);
		}
		if((numDigits % CHAR_BIT) && bigEndian)
		{
			printBinaryByte(b[index], 0, numDigits % CHAR_BIT,  digitsPrinted, spaced);
		}
	}
	/**
	 * @param bytes memory to print as binary
	 * @param numDigits how many binary digits to print
	 * @param bigEndian whether or not this computer is using big endian or not
	 * @param spaced how often to print make a space (0 for none)
	 */
	static void printHex(void * bytes, int numDigits, bool bigEndian, int spaced, CLI::CommandLineInterface * a_CLI)
	{
		char * b = (char*)bytes;
		int bytesToRead = numDigits/2;
		int index;
		int digitsPrinted = 0;
		if((numDigits & 1) && !bigEndian)
		{
			printHexByte(b[bytesToRead], 1, a_CLI);
			if(spaced && ++digitsPrinted >= spaced){CLI::putchar(' ');digitsPrinted=0;}
		}
		for(int i = 0; i < bytesToRead; ++i)
		{
			index = (!bigEndian)?(bytesToRead-1-i):i;
			printHexByte(b[index], 2, a_CLI);
			if(spaced && ++digitsPrinted >= spaced){CLI::putchar(' ');digitsPrinted=0;}
			printHexByte(b[index], 1, a_CLI);
			if(spaced && ++digitsPrinted >= spaced){CLI::putchar(' ');digitsPrinted=0;}
		}
		if((numDigits & 1) && bigEndian)
		{
			printHexByte(b[0], 1, a_CLI);
		}
	}
	/**
	 * @param number what to print as binary
	 * @param numDigits how many binary digits to print
	 * @param bigEndian whether or not this computer is using big endian or not
	 */
	static void printBinary(size_t number, int numDigits, bool bigEndian)
	{
		printBinary(&number, numDigits, bigEndian, 8);
	}
	/** @param number what to print as binary */
	//static void printBinary(size_t number){printBinary(number,sizeof(size_t)*CHAR_BIT,0);}
	/**
	 * @param number what to print as hex
	 * @param numDigits how many binary digits to print
	 * @param bigEndian whether or not this computer is using big endian or not
	 */
	static void printHex(size_t number, int numDigits, bool bigEndian, CLI::CommandLineInterface * a_CLI)
	{
		printHex(&number, numDigits, bigEndian, 0, a_CLI);
	}
	/** @param number what to print as hex */
	static void printHex(size_t number, CLI::CommandLineInterface * a_CLI) {
		printHex(number, (sizeof(size_t)*2), false, a_CLI); }

	// TODO minimize calls to isValidPtr
	// TODO give a prompt when moving into memory that isn't known to be valid
	// TODO remember the decision till the user goes back into fully known mem
	bool isValidPtr(void * memory_)
	{
		size_t valueAtPtr = *((size_t*)memory_);
		return MEM::validBytesAt((size_t*)valueAtPtr) > (size_t)(height*width);
	}

	void printmemory(CLI::CommandLineInterface * a_CLI, void * memory_)
	{
	#define UICOLOR	a_CLI->setColor(CLI::COLOR::LIGHT_GRAY)
	#define TEXTCOLOR	a_CLI->setColor(CLI::COLOR::WHITE)
		unsigned char * memory = (unsigned char*)memory_;
		int rows = height, cols = width, index = 0;
		a_CLI->move(0,0);
		for(int r = 0; r < rows; ++r){
			if(viewHexArea)
			{
				for(int c = 0; c < cols; ++c){
					if(c>0){UICOLOR;	a_CLI->putchar(' ');}
					a_CLI->setColor(getSafeColor(memory[index+c]));
					a_CLI->printf("%02x", memory[index+c]);
				}
			}
			if(viewHexArea && viewCharArea)
			{
				UICOLOR;	a_CLI->putchar('>');
			}
			if(viewCharArea)
			{
				for(int c = 0; c < cols; ++c){
					printCharSafe(memory[index+c], a_CLI);
				}
			}
			if(viewHexArea || viewCharArea)
			{
				a_CLI->putchar('\n');
			}
			index += cols;
		}
		if(viewInfoArea)
		{
			UICOLOR;	a_CLI->printf("@ 0x");
			TEXTCOLOR;	printHex((size_t)memory_, a_CLI);
			a_CLI->printf("            ");
			for(int i = sizeof(size_t)-1; i >= 0; --i){
				printCharSafe(memory[i], a_CLI);
				a_CLI->putchar(' ');
			}
			a_CLI->putchar('\n');
			UICOLOR;	a_CLI->printf("(*int64)                     ");
			TEXTCOLOR;	a_CLI->printf("0x");
			for(int i = sizeof(size_t)-1; i >= 0; --i){
				a_CLI->setColor(getSafeColor(memory[i]));
				a_CLI->printf("%02x", memory[i]);
			}
			a_CLI->putchar('\n');
			UICOLOR;	a_CLI->printf("(*int)    ");
			TEXTCOLOR;	a_CLI->printf("%12d               0x%08x\n", *((int*)memory_), *((unsigned int*)memory_));
			UICOLOR;	a_CLI->printf("(*short)  ");
			TEXTCOLOR;	a_CLI->printf("%12d                   0x%04x\n", *((short*)memory_), *((unsigned short*)memory_));
			UICOLOR;	a_CLI->printf("hexadecimal");
			TEXTCOLOR;
			for(int i = 3; i >= 0; --i){
				a_CLI->printf("     0x");	printHex(memory[i], HEXCOL, false, a_CLI);
			}
			a_CLI->putchar('\n');
			UICOLOR;	a_CLI->printf("decimal cha");
			TEXTCOLOR;
			for(int i = 3; i >= 0; --i){
				a_CLI->printf("%6d ", memory[i]);	a_CLI->printf(" ");	printCharSafe(memory[i], a_CLI);
			}
			a_CLI->putchar('\n');
			UICOLOR;	a_CLI->printf("(*binary):  ");
			TEXTCOLOR;	printBinary(*((size_t*)memory_),32,false);
		}
		/*
		// color the cursor
		CLIBuffer::Coord cursor2D(cursorIndex%width, cursorIndex/height), hexA, chaA, infA;
		int numBytes = sizeof(size_t);
		CLIBuffer::CLIOutputType * cell;
		CLIBuffer * b = a_CLI->getOutputBuffer();
		bool isValidP = isValidPtr(memory_);
		for(int i = 0; i < numBytes; ++i)
		{
			int bcolor=(i==cursorAnimation)
					?((isValidP)?CLI::COLOR::LIGHT_GRAY:CLI::COLOR::BLACK)
					:(CLI::COLOR::DARK_GRAY);
			if(viewHexArea)
			{
				// in the main hex area
				for(int col = 0; col < HEXCOLTOTAL; ++col){
					hexA.set(cursor2D.x*HEXCOLTOTAL+col, cursor2D.y);
					cell = b->getAt(hexA);cell->bcolor = bcolor;
				}
			}
			if(viewCharArea)
			{
				// in the char area
				chaA.set(cursor2D.x+HEXCOLTOTAL*((viewHexArea)?width:0), cursor2D.y);
				cell = b->getAt(chaA);cell->bcolor = bcolor;
			}
			if(viewInfoArea)
			{
				// in the info area
				for(int col = 0; col < HEXCOL; ++col)
				{
					infA.set((HEXCOLTOTAL*width)-((i+1)*HEXCOL)+col-1,
							(viewHexArea|viewCharArea)?height:0);
					cell = b->getAt(infA);cell->bcolor = bcolor;
				}
			}
			cursor2D.x++;
			if(cursor2D.x >= width)
			{
				cursor2D.x=0;
				cursor2D.y++;
				if(cursor2D.y >= height)
					break;
			}
		}
		if(++cursorAnimation >= numBytes)
			cursorAnimation = 0;
		*/
		if(confirmMessage){
			a_CLI->move(0,0);
			a_CLI->printf(confirmMessage);
		}
	}
private:
	const char * confirmMessage;
	int confirmInput;
	bool m_userWantsToQuit;

	void initiateConfirmDialog(int code, CLI::CommandLineInterface * a_CLI, const char * prompt)
	{
		a_CLI->move(0,0);
		confirmMessage =
				"Are you sure you want to quit?\n"
				"Press ENTER to confirm quit.\n"
				"or any other key to cancel.\n";
		confirmInput = code;
	}
	void finalizeConfirmDialog(){
		confirmInput = 0;
		confirmMessage = 0;
	}
public:
	bool userWantsToQuit(){return m_userWantsToQuit;}
	void movememoryinput(int input, CLI::CommandLineInterface * a_CLI, unsigned char * & ptr){
		static const char * confirmed = "\0CONFIRMED";
		if(confirmMessage){
			if(input == '\r' || input == '\n'){
				confirmMessage = confirmed;
				input = confirmInput;
			}else if(input != -1){
				confirmMessage = 0;
				confirmInput = 0;
				return;
			}
		}
//		CLI::printf("%d [%c]\n", input, input);
		switch(input){
		case CLI::KEY::UP:
		case 'w':	ptr-=width;	break;
		case CLI::KEY::LEFT:
		case 'a':	ptr-=1;	break;
		case CLI::KEY::DOWN:
		case 's':	ptr+=width;	break;
		case CLI::KEY::RIGHT:
		case 'd':	ptr+=1;	break;
		case CLI::KEY::ESCAPE:
		case 'q':	case 'Q':
		case 'x':	case 'X':
			if(confirmMessage != confirmed){
				initiateConfirmDialog(input, a_CLI,
					"Are you sure you want to quit?\n"
					"Press ENTER to confirm quit.\n"
					"or any other key to cancel.\n");
				break;
			}
			m_userWantsToQuit = true;
			finalizeConfirmDialog();
			break;
		case ' ':
			if(!isValidPtr(ptr) && confirmMessage != confirmed){
				initiateConfirmDialog(input, a_CLI,
					"WARNING! This is not a valid pointer.\n"
					"Press ENTER to confirm traversal.\n"
					"or any other key to cancel.\n");
				break;
			}
			m_history.add((UCHAR_TYPE*)ptr);
			ptr = (unsigned char*)*((size_t*)ptr);
			finalizeConfirmDialog();
			break;
		case CLI::KEY::BACKSPACE:
		case '\b':
			if(m_history.size() > 0)
			{
				ptr = m_history.pop();
			}
			break;
		case '1':	a_CLI->fillScreen(' ');	viewHexArea = !viewHexArea;	break;
		case '2':	a_CLI->fillScreen(' ');	viewCharArea = !viewCharArea;	break;
		case '3':	a_CLI->fillScreen(' ');	viewInfoArea = !viewInfoArea;	break;
		}
	}

	void userInput(const int input){
		movememoryinput(input, CLI::getCLI(), g_ptr);
	}

	void setPtr(void * ptr)
	{
		g_ptr = (UCHAR_TYPE*)ptr;
	}

	void * getPtr()
	{
		return g_ptr;
	}

	void draw(CLI::CommandLineInterface * a_CLI)
	{
		printmemory(a_CLI,g_ptr);
	}
};

MemViewer * g_mv = 0;

void MEMVIEWER::init(){
	g_mv = NEWMEM(MemViewer);
}
void MEMVIEWER::release(){
	DELMEM(g_mv);
}

void MEMVIEWER::printmemory(CLI::CommandLineInterface * a_CLI, void * memory_){
	g_mv->printmemory(a_CLI, memory_);
}

void MEMVIEWER::userInput(const int input){
	g_mv->userInput(input);
}

void MEMVIEWER::setPtr(void * ptr)
{
	g_mv->setPtr(ptr);
}

void * MEMVIEWER::getPtr()
{
	return g_mv->getPtr();
}

void MEMVIEWER::draw()
{
	g_mv->draw(CLI::getCLI());
}

bool MEMVIEWER::userWantsToQuit()
{
	return g_mv->userWantsToQuit();
}

void MEMVIEWER::initBasic(int & argc)
{
	MEM::markAsStack(&argc, 2048);
	CLI::init();
	CLI::setSize(CLI::getWidth(), 20);
	CLI::setDoubleBuffered(true);
	CLI::fillScreen(' ');
	MEMVIEWER::init();
}
void MEMVIEWER::releaseBasic()
{
	CLI::resetColor();
	CLI::fillScreen(' ');
	CLI::move(0,0);
	CLI::refresh();
	MEMVIEWER::release();
	CLI::release();
}
