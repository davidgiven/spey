-- $Id$
-- $Source$
-- $State$

-- Miscellaneous utilities.

STRIP = "strip -o %out[1]% %in[1]%"

strip = simple {
	class = "strip",
	
	command = {
		"%STRIP%"
	},
	outputs = {"%U%-%I%"},
}
