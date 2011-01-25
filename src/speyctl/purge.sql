-- purge.sql
-- Database purge script.
--
-- Copyright (C) 2011 David Given
-- You may distribute under the terms of the GNU General Public
-- License version 2 as specified in the file COPYING that comes with the
-- Spey distribution.
--
-- $Source$
-- $State$

BEGIN;
DELETE FROM triples WHERE 
	timesseen=1 AND 
	lastseen<(STRFTIME('%%s', 'now') - 
		(SELECT value FROM settings WHERE
			key = 'invalid-expiry')
		);

DELETE FROM triples WHERE
	timesseen>1 AND 
	lastseen<(STRFTIME('%%s', 'now') -
		(SELECT value FROM settings WHERE
			key = 'valid-expiry')
		);
COMMIT;
VACUUM;
