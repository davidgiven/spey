/* SpeyCLI.cc
 * Command line handle class
 *
 * Copyright (C) 2008 David Given
 * You may distribute under the terms of the GNU General Public
 * License version 2 as specified in the file COPYING that comes with the
 * Spey distribution.
 *
 * $Source$
 * $State$
 */

#include "spey.h"
#include <getopt.h>

SpeyCLI::SpeyCLI(int argc, char* argv[]):
	_d("/var/lib/spey/spey.db"),
	_f("0.0.0.0:25"),
	_t("localhost:2525"),
	_v(3),
	_x(false),
	_i(false),
	_h(false)
{
	int option_index = 0;
	int c;

	static struct option long_options[] =
	{
		{ "database", 1, 0, 'd' },
		{ "from", 1, 0, 'f' },
		{ "to", 1, 0, 't' },
		{ "verbose", 1, 0, 'v' },
		{ "foreground", 0, 0, 'x' },
		{ "inetd", 0, 0, 'i' },
		{ "help", 0, 0, 'h' },
		{ 0, 0, 0, 0 }
	};

	_executable = argv[0];

	/* default values */

	while ((c = getopt_long(argc, argv, "d:f:t:v:xih", long_options,
			&option_index)) != EOF)
	{
		switch (c)
		{
			case 'd':
				_d = optarg;
				break;

			case 'f':
				_f = optarg;
				break;

			case 't':
				_t = optarg;
				break;

			case 'v':
				_v = atoi(optarg);
				if (_v < 0)
					throw InvocationException("-v parameter must be >= 0");
				break;

			case 'x':
				_x = true;
				break;

			case 'i':
				_i = true;
				break;

			case 'h':
				_h = true;
				usage();
				break;

			default:
				usage();
		}
	}

	_optind = optind;
}

void SpeyCLI::usage()
{
	cout << "Usage: spey [OPTION]...\n"
	     << "SMTP greylisting proxy version " << MajorVersion << " build " << BuildCount << ".\n"
	     << "(C) 2004-2011 David Given. This software is licensed under the\n"
	     << "terms of the GPLv2 open source license. See the COPYING file in\n"
	     << "the distribution for the full text.\n"
	     << "\n"
	     << "Options:\n"
	     << " -d, --database=FILE      Location of the Spey configuration file\n"
	     << "                          Default: /var/lib/spey/spey.db\n"
	     << " -f, --from=ADDRESS       The address to listen on\n"
	     << "                          Default: 0.0.0.0:25\n"
	     << " -t, --to=ADDRESS         The SMTP server to connect to\n"
	     << "                          Default: localhost:2525\n"
	     << " -v, --verbose=LEVEL      The verbosity level\n"
	     << "                          Default: 3\n"
	     << " -x, --foreground         Run in the foreground instead of as a daemon\n"
	     << " -t, --inetd              Run in inetd mode instead of as a daemon\n"
	     << " -h, --help               Display help information (this message)\n"
	     << "\n"
	     << "See http://spey.sourceforge.net for more information.\n";
	exit(0);
}

