/* SpeyCtlCLI.h
 * Command line handler class.
 *
 * Copyright (C) 2004 David Given
 * You may distribute under the terms of the GNU General Public
 * License version 2 as specified in the file COPYING that comes with the
 * Spey distribution.
 *
 * $Source$
 * $State$
 */

#ifndef SPEYCTLCLI_H
#define SPEYCTLCLI_H

#include <vector>

struct SpeyCtlCLI
{
public:
  /* constructor and destructor */
  SpeyCtlCLI(int argc, char* argv[]);
  ~SpeyCtlCLI(){}

  void usage();

  const string& d() { return _d; }
  const string& f() { return _f; }
  const string& t() { return _t; }
  int v() { return _v; }
  bool x() { return _x; }
  bool i() { return _i; }
  bool h() { return _h; }

  int getparametercount() const;
  const string& getparameter(int i) const;

private:
  string _d;
  string _f;
  string _t;
  int _v;
  bool _x;
  bool _i;
  bool _h;

  /* other stuff to keep track of */
  string _executable;
  vector<string> _parameters;
};

#endif
