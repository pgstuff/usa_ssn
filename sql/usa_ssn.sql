/*
 * Author: The maintainer's name
 * Created at: Wed Oct 14 23:12:59 -0400 2015
 *
 */

SET client_min_messages = warning;

-- SQL definitions for USAEIN type
CREATE TYPE usa_ssn;

-- basic i/o functions
CREATE OR REPLACE FUNCTION usa_ssn_in(cstring) RETURNS usa_ssn AS '$libdir/usa_ssn'
LANGUAGE C IMMUTABLE STRICT;
CREATE OR REPLACE FUNCTION usa_ssn_out(usa_ssn) RETURNS cstring AS '$libdir/usa_ssn'
LANGUAGE C IMMUTABLE STRICT;
CREATE OR REPLACE FUNCTION usa_ssn_send(usa_ssn) RETURNS bytea AS '$libdir/usa_ssn'
LANGUAGE C IMMUTABLE STRICT;
CREATE OR REPLACE FUNCTION usa_ssn_recv(internal) RETURNS usa_ssn AS '$libdir/usa_ssn'
LANGUAGE C IMMUTABLE STRICT;

CREATE TYPE usa_ssn (
	input = usa_ssn_in,
	output = usa_ssn_out,
	send = usa_ssn_send,
	receive = usa_ssn_recv,
	internallength = 4,
	passedbyvalue
);

-- functions to support btree opclass
CREATE OR REPLACE FUNCTION usa_ssn_lt(usa_ssn, usa_ssn) RETURNS bool AS '$libdir/usa_ssn'
LANGUAGE C IMMUTABLE STRICT;
CREATE OR REPLACE FUNCTION usa_ssn_le(usa_ssn, usa_ssn) RETURNS bool AS '$libdir/usa_ssn'
LANGUAGE C IMMUTABLE STRICT;
CREATE OR REPLACE FUNCTION usa_ssn_eq(usa_ssn, usa_ssn) RETURNS bool AS '$libdir/usa_ssn'
LANGUAGE C IMMUTABLE STRICT;
CREATE OR REPLACE FUNCTION usa_ssn_ne(usa_ssn, usa_ssn) RETURNS bool AS '$libdir/usa_ssn'
LANGUAGE C IMMUTABLE STRICT;
CREATE OR REPLACE FUNCTION usa_ssn_ge(usa_ssn, usa_ssn) RETURNS bool AS '$libdir/usa_ssn'
LANGUAGE C IMMUTABLE STRICT;
CREATE OR REPLACE FUNCTION usa_ssn_gt(usa_ssn, usa_ssn) RETURNS bool AS '$libdir/usa_ssn'
LANGUAGE C IMMUTABLE STRICT;
CREATE OR REPLACE FUNCTION usa_ssn_cmp(usa_ssn, usa_ssn) RETURNS int4 AS '$libdir/usa_ssn'
LANGUAGE C IMMUTABLE STRICT;

-- to/from text conversion
CREATE OR REPLACE FUNCTION usa_ssn_to_text(usa_ssn) RETURNS text AS '$libdir/usa_ssn'
LANGUAGE C IMMUTABLE STRICT;
CREATE OR REPLACE FUNCTION usa_ssn_from_text(text) RETURNS usa_ssn AS '$libdir/usa_ssn'
LANGUAGE C IMMUTABLE STRICT;

-- operators
CREATE OPERATOR < (
	leftarg = usa_ssn, rightarg = usa_ssn, procedure = usa_ssn_lt,
	commutator = >, negator = >=,
	restrict = scalarltsel, join = scalarltjoinsel
);
CREATE OPERATOR <= (
	leftarg = usa_ssn, rightarg = usa_ssn, procedure = usa_ssn_le,
	commutator = >=, negator = >,
	restrict = scalarltsel, join = scalarltjoinsel
);
CREATE OPERATOR = (
	leftarg = usa_ssn, rightarg = usa_ssn, procedure = usa_ssn_eq,
	commutator = =, negator = <>,
	restrict = eqsel, join = eqjoinsel,
	merges
);
CREATE OPERATOR <> (
	leftarg = usa_ssn, rightarg = usa_ssn, procedure = usa_ssn_ne,
	commutator = <>, negator = =,
	restrict = neqsel, join = neqjoinsel
);
CREATE OPERATOR > (
	leftarg = usa_ssn, rightarg = usa_ssn, procedure = usa_ssn_gt,
	commutator = <, negator = <=,
	restrict = scalargtsel, join = scalargtjoinsel
);
CREATE OPERATOR >= (
	leftarg = usa_ssn, rightarg = usa_ssn, procedure = usa_ssn_ge,
	commutator = <=, negator = <,
	restrict = scalargtsel, join = scalargtjoinsel
);

-- aggregates
CREATE OR REPLACE FUNCTION usa_ssn_smaller(usa_ssn, usa_ssn)
RETURNS usa_ssn
AS '$libdir/usa_ssn'
    LANGUAGE C IMMUTABLE STRICT;

CREATE OR REPLACE FUNCTION usa_ssn_larger(usa_ssn, usa_ssn)
RETURNS usa_ssn
AS '$libdir/usa_ssn'
    LANGUAGE C IMMUTABLE STRICT;

CREATE AGGREGATE min(usa_ssn)  (
    SFUNC = usa_ssn_smaller,
    STYPE = usa_ssn,
    SORTOP = <
);

CREATE AGGREGATE max(usa_ssn)  (
    SFUNC = usa_ssn_larger,
    STYPE = usa_ssn,
    SORTOP = >
);

-- btree operator class
CREATE OPERATOR CLASS usa_ssn_ops DEFAULT FOR TYPE usa_ssn USING btree AS
	OPERATOR 1 <,
	OPERATOR 2 <=,
	OPERATOR 3 =,
	OPERATOR 4 >=,
	OPERATOR 5 >,
	FUNCTION 1 usa_ssn_cmp(usa_ssn, usa_ssn);
-- cast from/to text
CREATE CAST (usa_ssn AS text) WITH FUNCTION usa_ssn_to_text(usa_ssn) AS ASSIGNMENT;
CREATE CAST (text AS usa_ssn) WITH FUNCTION usa_ssn_from_text(text) AS ASSIGNMENT;

/* Does this survive a pg_dump?
CREATE CAST (int       AS usa_ssn)   WITHOUT FUNCTION AS ASSIGNMENT;
CREATE CAST (usa_ssn   AS int)       WITHOUT FUNCTION;
*/

DO $$BEGIN
    EXECUTE 'ALTER DATABASE '|| quote_ident(current_database()) || '
  SET usa_ssn.format=''123-45-6789'';';
END$$;
