/* CLI.h
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

#ifndef SPEYCLI_H
#define SPEYCLI_H

struct SpeyCLI
{
public:
  /* constructor and destructor */
  SpeyCLI(int argc, char* argv[]);
  ~SpeyCLI(){}

  void usage();

  const string& d() { return _d; }
  const string& f() { return _f; }
  const string& t() { return _t; }
  int v() { return _v; }
  bool x() { return _x; }
  bool i() { return _i; }
  bool h() { return _h; }

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
  int _optind;
};

#endif
