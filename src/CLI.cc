/******************************************************************************
**
** src/CLI.cc
**
** Wed Apr 28 16:59:09 2004
** Linux 2.4.25 (#2 Tue Mar 9 11:40:41 GMT 2004) i586
** dg@tiar (David Given)
**
** Definition of command line parser class
**
** Automatically created by genparse v0.5.2
**
** See http://genparse.sourceforge.net/ for details and updates
**
******************************************************************************/

#include <getopt.h>
#include <stdlib.h>
#include "src/CLI.h"

/*----------------------------------------------------------------------------
**
** CLI::CLI()
**
** Constructor method.
**
**--------------------------------------------------------------------------*/

CLI::CLI(int argc, char *argv[]) throw (std::string)
{
  extern char *optarg;
  extern int optind;
  int option_index = 0;
  int c;

  static struct option long_options[] =
  {
    {"database", 1, 0, 'd'},
    {"from", 1, 0, 'f'},
    {"to", 1, 0, 't'},
    {"verbose", 1, 0, 'v'},
    {"help", 0, 0, 'h'},
    {0, 0, 0, 0}
  };

  _executable += argv[0];

  /* default values */
  _d = "/var/lib/misc/spey.db";
  _f = "0.0.0.0:25";
  _t = "localhost:2525";
  _v = 999;
  _h = false;

  while ((c = getopt_long(argc, argv, "d:f:t:v:h", long_options, &option_index)) != EOF)
    {
      switch(c)
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
            {
              std::string s;
              s += "parameter range error: v must be >= 0";
              throw(s);
            }
          break;

        case 'h': 
          _h = true;
          this->usage();
          break;

        default:
          this->usage();

        }
    } /* while */

  _optind = optind;
}

/*----------------------------------------------------------------------------
**
** CLI::usage()
**
** Usage function.
**
**--------------------------------------------------------------------------*/

void CLI::usage()
{
  std::cout << "usage: " << _executable << " [ -dftvh ] " << std::endl;
  std::cout << "  [ -d ] ";
  std::cout << "[ --database ]  ";
  std::cout << "(";
  std::cout << "type=";
  std::cout << "STRING,";
  std::cout << " default=/var/lib/misc/spey.db";
  std::cout << ")\n";
  std::cout << "         Spey configuration file.\n";
  std::cout << "  [ -f ] ";
  std::cout << "[ --from ]  ";
  std::cout << "(";
  std::cout << "type=";
  std::cout << "STRING,";
  std::cout << " default=0.0.0.0:25";
  std::cout << ")\n";
  std::cout << "         Address to listen on.\n";
  std::cout << "  [ -t ] ";
  std::cout << "[ --to ]  ";
  std::cout << "(";
  std::cout << "type=";
  std::cout << "STRING,";
  std::cout << " default=localhost:2525";
  std::cout << ")\n";
  std::cout << "         SMTP server to connect to.\n";
  std::cout << "  [ -v ] ";
  std::cout << "[ --verbose ]  ";
  std::cout << "(";
  std::cout << "type=";
  std::cout << "INTEGER,";
  std::cout << " range=0...,";
  std::cout << " default=999";
  std::cout << ")\n";
  std::cout << "         Set the verbosity level.\n";
  std::cout << "  [ -h ] ";
  std::cout << "[ --help ]  ";
  std::cout << "(";
  std::cout << "type=";
  std::cout << "FLAG";
  std::cout << ")\n";
  std::cout << "         Display help information.\n";
  exit(0);
}

