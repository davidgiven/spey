/* SpeyCtlCLI.cc
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

#include "common.h"
#include "SpeyCtlCLI.h"
#include <getopt.h>

SpeyCtlCLI::SpeyCtlCLI(int argc, char* argv[]):
	_d("/var/lib/spey/spey.db"),
	_v(3),
	_h(false)
{
	int option_index = 0;
	int c;

	static struct option long_options[] =
	{
		{ "database", 1, 0, 'd' },
		{ "verbose", 1, 0, 'v' },
		{ "help", 0, 0, 'h' },
		{ 0, 0, 0, 0 }
	};

	_executable = argv[0];

	/* default values */

	while ((c = getopt_long(argc, argv, "d:v:h", long_options,
			&option_index)) != EOF)
	{
		switch (c)
		{
			case 'd':
				_d = optarg;
				break;

			case 'v':
				_v = atoi(optarg);
				if (_v < 0)
					throw InvocationException("-v parameter must be >= 0");
				break;

			case 'h':
				_h = true;
				usage();
				break;

			default:
				usage();
		}
	}

	for (;;)
	{
		const char* p = argv[optind++];
		if (!p)
			break;

		_parameters.push_back(p);
	}
}

int SpeyCtlCLI::getparametercount() const
{
	return _parameters.size();
}

const string& SpeyCtlCLI::getparameter(int i) const
{
	static string oob = "";
	if (i >= _parameters.size())
		return oob;
	return _parameters[i];
}

void SpeyCtlCLI::usage()
{
	cout << "Usage: speyctl [OPTIONS...] COMMAND\n"
	     << "SMTP greylisting proxy version " << MajorVersion << " build " << BuildCount << ".\n"
		 << "(C) 2004-2011 David Given. This software is licensed under the\n"
		 << "terms of the GPLv2 open source license. See the COPYING file in\n"
		 << "the distribution for the full text.\n"
		 << "\n"
		 << "Options:\n"
		 << " -d, --database=FILE      Location of the Spey configuration file\n"
		 << "                          Default: /var/lib/spey/spey.db\n"
		 << " -v, --verbose=LEVEL      The verbosity level\n"
		 << "                          Default: 3\n"
		 << " -h, --help               Display help information (this message)\n"
		 << "\n"
		 << "Commands:\n"
	     << "    help                     Displays this message.\n"
	     << "    init                     Create a new configuration file.\n"
	     << "    set <key> [<value>]      Set/display a single configuration value.\n"
	     << "    list                     List all configuration values to stdout.\n"
	     << "    trust <cmd> [<spec>]     Manipulate the trusted host table.\n"
	     << "    recipient <cmd> [<spec>] Manipulate the allowed recipients table.\n"
	     << "    whitelist <cmd> [<spec>] Manipulate the whitelist table.\n"
	     << "    blacklist <cmd> [<spec>] Manipulate the blacklist table.\n"
	     << "    user <cmd> [<spec>]      Manipulate the users table.\n"
	     << "    stats                    List statistics.\n"
	     << "    statsreset               Reset statistic counters to 0.\n"
	     << "    showdb                   Show address database. (Warning --- can be large!)\n"
	     << "    purge                    Purge stale addresses.\n"
	     << "\n"
	     << "See http://spey.sourceforge.net for more information.\n";
	exit(0);
}

