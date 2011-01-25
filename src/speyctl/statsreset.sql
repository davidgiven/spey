-- statsreset.sql
-- Statistics counter reset script.
--
-- Copyright (C) 2011 David Given
-- You may distribute under the terms of the GNU General Public
-- License version 2 as specified in the file COPYING that comes with the
-- Spey distribution.
--
-- $Source$
-- $State$

BEGIN;
UPDATE statistics SET value=0;
COMMIT;
