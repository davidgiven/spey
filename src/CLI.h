/******************************************************************************
**
** src/CLI.h
**
** Wed Apr 28 16:59:09 2004
** Linux 2.4.25 (#2 Tue Mar 9 11:40:41 GMT 2004) i586
** dg@tiar (David Given)
**
** Header file for command line parser class
**
** Automatically created by genparse v0.5.2
**
** See http://genparse.sourceforge.net/ for details and updates
**
******************************************************************************/

#ifndef CLI_H
#define CLI_H

#include <iostream>
#include <string>

/*----------------------------------------------------------------------------
**
** class CLI
**
** command line parser class
**
**--------------------------------------------------------------------------*/

class CLI
{
private:
  /* parameters */
  std::string _d;
  std::string _f;
  std::string _t;
  int _v;
  bool _h;

  /* other stuff to keep track of */
  std::string _executable;
  int _optind;

public:
  /* constructor and destructor */
  CLI(int, char **) throw(std::string);
  ~CLI(){}

  /* usage function */
  void usage();

  /* return next (non-option) parameter */
  int next_param() { return _optind; }

  /* callback functions */

  std::string d() { return _d; }
  std::string f() { return _f; }
  std::string t() { return _t; }
  int v() { return _v; }
  bool h() { return _h; }
};

#endif
