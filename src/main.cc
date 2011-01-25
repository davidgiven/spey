/* version.cc
 * Contains the version number constants.
 *
 * Copyright (C) 2007 David Given
 * You may distribute under the terms of the GNU General Public
 * License version 2 as specified in the file COPYING that comes with the
 * Spey distribution.
 *
 * $Source$
 * $State$
 */

#include "common.h"
#include <stdexcept>

extern int main_spey(int argc, char* argv[]);
extern int main_speyctl(int argc, char* argv[]);

static const char* programname;

int main(int argc, char* argv[])
{
	try
	{
		programname = strrchr(argv[0], '/');
		if (!programname)
			programname = argv[0];
		else
			programname++;

		if (strcmp(programname, "speyctl") == 0)
			return main_speyctl(argc, argv);
		else if (strcmp(programname, "spey") == 0)
			return main_spey(argc, argv);
		else
			throw InvocationException("the spey binary must be called as 'spey' or 'speyctl'");
	}
	catch (const InvocationException& e)
	{
		fprintf(stderr, "%s: %s\n",
				programname ? programname : "Error",
						string(e).c_str());
		exit(-1);
	}
	catch (const Exception& e)
	{
		SystemLog() << "generic exception caught: "
			    << e;
		exit(-1);
	}
	catch (const std::logic_error& e)
	{
		SystemLog() << "internal logic error: "
				<< e.what();
		exit(-1);
	}
	catch (...)
	{
		SystemLog() << "illegal exception caught! terminating!";
		throw;
		exit(-1);
	}

	return 0;
}
