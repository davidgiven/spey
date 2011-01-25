-- init.sql
-- Database initialisation script.
--
-- Copyright (C) 2011 David Given
-- You may distribute under the terms of the GNU General Public
-- License version 2 as specified in the file COPYING that comes with the
-- Spey distribution.
--
-- $Source$
-- $State$

BEGIN;
CREATE TABLE triples (
	id INTEGER PRIMARY KEY,
	sender INTEGER NOT NULL,
	fromaddress VARCHAR NOT NULL,
	toaddress VARCHAR NOT NULL,
	timesseen INTEGER NOT NULL,
	firstseen INTEGER NOT NULL,
	lastseen INTEGER NOT NULL);

CREATE TABLE trustedhosts (
    left INTEGER NOT NULL,
    right INTEGER NOT NULL);
CREATE INDEX trustedhosts_idx ON trustedhosts
	(left);
INSERT INTO trustedhosts VALUES
    (2130706433, 32);

CREATE TABLE validrecipients (
    left VARCHAR,
    right VARCHAR);

CREATE TABLE settings (
    key VARCHAR PRIMARY KEY,
    value VARCHAR);
INSERT INTO settings VALUES
    ('identity',        'yourdomain.invalid');
INSERT INTO settings VALUES
    ('intolerant',      '1');
INSERT INTO settings VALUES
    ('quarantine-time', '60');
INSERT INTO settings VALUES
    ('socket-timeout',  '30');
INSERT INTO settings VALUES
    ('invalid-expiry',  '86400');
INSERT INTO settings VALUES
    ('valid-expiry',    '1209600');
INSERT INTO settings VALUES
    ('runtime-user-id', 'spey:spey');
INSERT INTO settings VALUES
    ('greet-pause',     '2');
INSERT INTO settings VALUES
    ('external-auth-mode', 'none');

CREATE TABLE statistics (
    key VARCHAR PRIMARY KEY,
    value INTEGER);
INSERT INTO statistics VALUES
    ('malformed-domain',    0);
INSERT INTO statistics VALUES
    ('malformed-address',   0);
INSERT INTO statistics VALUES
    ('illegal-relay',       0);
INSERT INTO statistics VALUES
    ('timeout',             0);
INSERT INTO statistics VALUES
    ('greylisted',          0);
INSERT INTO statistics VALUES
    ('accepted',            0);
INSERT INTO statistics VALUES
    ('whitelisted',         0);
INSERT INTO statistics VALUES
    ('spoke-too-soon',      0);
INSERT INTO statistics VALUES
    ('blackholed',          0);

CREATE TABLE whitelist (
    sender VARCHAR,
    recipient VARCHAR);

CREATE TABLE blacklist (
    sender VARCHAR,
    recipient VARCHAR);

CREATE TABLE users (
    username VARCHAR,
    password VARCHAR);
COMMIT;
