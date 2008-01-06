/* CLI.gp
 * genparse data file describing the command line parameters Spey accepts.
 *
 * Copyright (C) 2004 David Given
 * You may distribute under the terms of the GNU General Public
 * License version 2 as specified in the file COPYING that comes with the
 * Spey distribution.
 *
 * $Source$
 * $State$
 */

d / database		string {"/var/lib/spey/spey.db"}
	"Spey configuration file."

f / from                string {"0.0.0.0:25"}
	"Address to listen on."

t / to			string {"localhost:2525"}
	"SMTP server to connect to."

v / verbose		int 3 [0...]
	"Set the verbosity level."

x / foreground		flag
	"Run in the foreground instead of as a daemon."

i / inetd		flag
	"Run in inetd mode instead of as a daemon."
