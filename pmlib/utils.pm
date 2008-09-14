-- $Id$
-- $HeadURL: https://primemover.svn.sf.net/svnroot/primemover/pm/lib/c.pm $
-- $LastChangedDate: 2007-04-30 22:41:42 +0000 (Mon, 30 Apr 2007) $

-- Miscellaneous utilities.

STRIP = "strip -o %out[1]% %in[1]%"

strip = simple {
	class = "strip",
	
	command = {
		"%STRIP%"
	},
	outputs = {"%U%-%I%"},
}
