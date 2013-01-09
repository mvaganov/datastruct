#pragma once

#include "license.txt"
#include "cli.h"

/**
 * Command Line Interface (Mem)ory (Viewer)
 * for debugging memory in the commandline console
 *
 * Work In Progress
 *
 * @author Michael Vaganov
 */
namespace MEMVIEWER
{
// TODO identify where memory blocks and pages begin and end
// TODO identify which part of memory you are in (stack, which heap page)
// TODO make this work in windows
// TODO finish commenting
	/** @param ptr where in memory to view */
	void setPtr(void * ptr);
	/** @return which pointer is being used */
	void * getPtr();
	/** prints to the current CLI */
	void draw();
	/** @param keys to move traversal */
	void userInput(const int input);
	void printmemory(CLI::CommandLineInterface * a_CLI, void * ptr);
	void init();
	void release();

	void initBasic(int & argc);
	void releaseBasic();
	bool userWantsToQuit();
}
