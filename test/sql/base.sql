\set ECHO none
\i sql/usa_ssn.sql
\set ECHO all

CREATE TABLE usa_ssns(id serial primary key, usa_ssn usa_ssn unique);
INSERT INTO usa_ssns(usa_ssn) VALUES('457-55-5462');
-- check for alternative input format
INSERT INTO usa_ssns(usa_ssn) VALUES('078051120');
-- check for invalid SSNs
INSERT INTO usa_ssns(usa_ssn) VALUES('000-00-0000');
INSERT INTO usa_ssns(usa_ssn) VALUES('999-99-9999');


SELECT * FROM usa_ssns ORDER BY usa_ssn;

SELECT id,usa_ssn::text FROM usa_ssns WHERE usa_ssn = '457555462';

-- test format mask
CREATE USER usa_ssn_base_test;
GRANT SELECT ON usa_ssns TO usa_ssn_base_test;
SET ROLE usa_ssn_base_test;

SET usa_ssn.format = 'XXXX-xx-6789';
SELECT id,usa_ssn::text FROM usa_ssns WHERE usa_ssn = '457555462';
SET usa_ssn.format = '123456789';
SELECT id,usa_ssn::text FROM usa_ssns WHERE usa_ssn = '078-05-1120';

RESET usa_ssn.format;
RESET ROLE;
REVOKE SELECT ON usa_ssns FROM usa_ssn_base_test;
DROP USER usa_ssn_base_test;

-- index scan
SET enable_seqscan = false;
SELECT id,usa_ssn::text FROM usa_ssns WHERE usa_ssn = '078-05-1120';
SELECT id,usa_ssn FROM usa_ssns WHERE usa_ssn >= '078-05-1120' LIMIT 5;
SELECT count(id) FROM usa_ssns;
SELECT count(id) FROM usa_ssns WHERE usa_ssn <> ('078-05-1120'::text)::usa_ssn;
RESET enable_seqscan;

-- operators and conversions
SELECT '078-05-1120'::usa_ssn < '078-05-1120'::usa_ssn;
SELECT '078-05-1120'::usa_ssn > '078-05-1120'::usa_ssn;
SELECT '078-05-1120'::usa_ssn < '457-55-5462'::usa_ssn;
SELECT '078-05-1120'::usa_ssn > '457-55-5462'::usa_ssn;
SELECT '078-05-1120'::usa_ssn <= '078-05-1120'::usa_ssn;
SELECT '078-05-1120'::usa_ssn >= '078-05-1120'::usa_ssn;
SELECT '078-05-1120'::usa_ssn <= '457-55-5462'::usa_ssn;
SELECT '078-05-1120'::usa_ssn >= '457-55-5462'::usa_ssn;
SELECT '078-05-1120'::usa_ssn <> '078-05-1120'::usa_ssn;
SELECT '078-05-1120'::usa_ssn <> '457-55-5462'::usa_ssn;
SELECT '078-05-1120'::usa_ssn = '078-05-1120'::usa_ssn;
SELECT '078-05-1120'::usa_ssn = '457-55-5462'::usa_ssn;

-- COPY FROM/TO
TRUNCATE usa_ssns;
COPY usa_ssns(usa_ssn) FROM STDIN;
078-05-1120
457555462
\.
COPY usa_ssns TO STDOUT;

-- clean up --
DROP TABLE usa_ssns;
