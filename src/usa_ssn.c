#include "postgres.h"
#include "fmgr.h"
#include "libpq/pqformat.h"
#include "utils/builtins.h"
#include "utils/guc.h"
#include "miscadmin.h"

#ifdef PG_MODULE_MAGIC
PG_MODULE_MAGIC;
#endif

typedef uint32 usa_ssn_t;

#define USASSNGetDatum(x)	 Int32GetDatum(x)
#define DatumGetUSASSN(x)	 DatumGetInt32(x)
#define PG_GETARG_USASSN(n) DatumGetUSASSN(PG_GETARG_DATUM(n))
#define PG_RETURN_USASSN(x) return USASSNGetDatum(x)

//#define DIGITTOINT(n)	(((n) >= '0' && (n) <= '9') ? (int32) ((n) - '0') : 0)

Datum usa_ssn_in(PG_FUNCTION_ARGS);
Datum usa_ssn_out(PG_FUNCTION_ARGS);
Datum usa_ssn_to_text(PG_FUNCTION_ARGS);
Datum usa_ssn_from_text(PG_FUNCTION_ARGS);
Datum usa_ssn_send(PG_FUNCTION_ARGS);
Datum usa_ssn_recv(PG_FUNCTION_ARGS);
Datum usa_ssn_lt(PG_FUNCTION_ARGS);
Datum usa_ssn_le(PG_FUNCTION_ARGS);
Datum usa_ssn_eq(PG_FUNCTION_ARGS);
Datum usa_ssn_ne(PG_FUNCTION_ARGS);
Datum usa_ssn_ge(PG_FUNCTION_ARGS);
Datum usa_ssn_gt(PG_FUNCTION_ARGS);
Datum usa_ssn_cmp(PG_FUNCTION_ARGS);

static usa_ssn_t cstring_to_usa_ssn(char *usa_ssn_string);
static char *usa_ssn_to_cstring(usa_ssn_t usa_ssn);
static bool usa_ssn_is_valid(int32 area, int32 group, int32 serial);



/* generic input/output functions */
PG_FUNCTION_INFO_V1(usa_ssn_in);
Datum
usa_ssn_in(PG_FUNCTION_ARGS)
{
	usa_ssn_t	result;

	char   *usa_ssn_str = PG_GETARG_CSTRING(0);
	result = cstring_to_usa_ssn(usa_ssn_str);

	PG_RETURN_USASSN(result);
}

PG_FUNCTION_INFO_V1(usa_ssn_out);
Datum
usa_ssn_out(PG_FUNCTION_ARGS)
{
	usa_ssn_t	packed_usa_ssn;
	char   *usa_ssn_string;

	packed_usa_ssn = PG_GETARG_USASSN(0);
	usa_ssn_string = usa_ssn_to_cstring(packed_usa_ssn);

	PG_RETURN_CSTRING(usa_ssn_string);
}

/* type to/from text conversion routines */
PG_FUNCTION_INFO_V1(usa_ssn_to_text);
Datum
usa_ssn_to_text(PG_FUNCTION_ARGS)
{
	char	*usa_ssn_string;
	text	*usa_ssn_text;

	usa_ssn_t	packed_usa_ssn =  PG_GETARG_USASSN(0);

	usa_ssn_string = usa_ssn_to_cstring(packed_usa_ssn);
	usa_ssn_text = DatumGetTextP(DirectFunctionCall1(textin, CStringGetDatum(usa_ssn_string)));

	PG_RETURN_TEXT_P(usa_ssn_text);
}

PG_FUNCTION_INFO_V1(usa_ssn_from_text);
Datum
usa_ssn_from_text(PG_FUNCTION_ARGS)
{
	text  *usa_ssn_text = PG_GETARG_TEXT_P(0);
	char  *usa_ssn_str = DatumGetCString(DirectFunctionCall1(textout, PointerGetDatum(usa_ssn_text)));
	usa_ssn_t usa_ssn = cstring_to_usa_ssn(usa_ssn_str);

	PG_RETURN_USASSN(usa_ssn);
}

/* Functions to convert to/from bytea */
PG_FUNCTION_INFO_V1(usa_ssn_send);
Datum
usa_ssn_send(PG_FUNCTION_ARGS)
{
	StringInfoData buffer;
	usa_ssn_t usa_ssn = PG_GETARG_USASSN(0);

	pq_begintypsend(&buffer);
	pq_sendint(&buffer, (int32)usa_ssn, 4);
	PG_RETURN_BYTEA_P(pq_endtypsend(&buffer));
}

PG_FUNCTION_INFO_V1(usa_ssn_recv);
Datum usa_ssn_recv(PG_FUNCTION_ARGS)
{
	usa_ssn_t	usa_ssn;
	StringInfo	buffer = (StringInfo) PG_GETARG_POINTER(0);

	usa_ssn = pq_getmsgint(buffer, 4);
	PG_RETURN_USASSN(usa_ssn);
}

/* functions to support btree opclass */
PG_FUNCTION_INFO_V1(usa_ssn_lt);
Datum
usa_ssn_lt(PG_FUNCTION_ARGS)
{
	usa_ssn_t a = PG_GETARG_USASSN(0);
	usa_ssn_t b = PG_GETARG_USASSN(1);
	PG_RETURN_BOOL(a < b);
}

PG_FUNCTION_INFO_V1(usa_ssn_le);
Datum
usa_ssn_le(PG_FUNCTION_ARGS)
{
	usa_ssn_t a = PG_GETARG_USASSN(0);
	usa_ssn_t b = PG_GETARG_USASSN(1);
	PG_RETURN_BOOL (a <= b);
}

PG_FUNCTION_INFO_V1(usa_ssn_eq);
Datum
usa_ssn_eq(PG_FUNCTION_ARGS)
{
	usa_ssn_t a = PG_GETARG_USASSN(0);
	usa_ssn_t b = PG_GETARG_USASSN(1);
	PG_RETURN_BOOL(a == b);
}

PG_FUNCTION_INFO_V1(usa_ssn_ne);
Datum
usa_ssn_ne(PG_FUNCTION_ARGS)
{
	usa_ssn_t a = PG_GETARG_USASSN(0);
	usa_ssn_t b = PG_GETARG_USASSN(1);
	PG_RETURN_BOOL(a != b);
}

PG_FUNCTION_INFO_V1(usa_ssn_ge);
Datum
usa_ssn_ge(PG_FUNCTION_ARGS)
{
	usa_ssn_t a = PG_GETARG_USASSN(0);
	usa_ssn_t b = PG_GETARG_USASSN(1);
	PG_RETURN_BOOL(a >= b);
}

PG_FUNCTION_INFO_V1(usa_ssn_gt);
Datum
usa_ssn_gt(PG_FUNCTION_ARGS)
{
	usa_ssn_t a = PG_GETARG_USASSN(0);
	usa_ssn_t b = PG_GETARG_USASSN(1);
	PG_RETURN_BOOL(a > b);
}

PG_FUNCTION_INFO_V1(usa_ssn_cmp);
Datum
usa_ssn_cmp(PG_FUNCTION_ARGS)
{
	usa_ssn_t a = PG_GETARG_USASSN(0);
	usa_ssn_t b = PG_GETARG_USASSN(1);

	PG_RETURN_INT32(a - b);
}

static int32
usa_ssn_cmp_internal(usa_ssn_t a, usa_ssn_t b)
{
    return a - b;

    /*if (a < b)
        return -1;
    else if (a > b)
        return 1;

    return 0;*/
}

/*****************************************************************************
 * Aggregate functions
 *****************************************************************************/

PG_FUNCTION_INFO_V1(usa_ssn_smaller);

Datum
usa_ssn_smaller(PG_FUNCTION_ARGS)
{
   usa_ssn_t left  = PG_GETARG_USASSN(0);
   usa_ssn_t right = PG_GETARG_USASSN(1);
   usa_ssn_t result;

   result = usa_ssn_cmp_internal(left, right) < 0 ? left : right;
   PG_RETURN_TEXT_P(result);
}

PG_FUNCTION_INFO_V1(usa_ssn_larger);

Datum
usa_ssn_larger(PG_FUNCTION_ARGS)
{
   usa_ssn_t left  = PG_GETARG_USASSN(0);
   usa_ssn_t right = PG_GETARG_USASSN(1);
   usa_ssn_t result;

   result = usa_ssn_cmp_internal(left, right) > 0 ? left : right;
   PG_RETURN_TEXT_P(result);
}


/*
 * Convert a cstring to an USASSN, validating the input.
 * Input in forms AAA-BB-CCCC or AAABBCCCC is accepted.
 */
static usa_ssn_t
cstring_to_usa_ssn(char *usa_ssn_str)
{
	char   *c;
	usa_ssn_t 	result;
	char 	dashes[] = {3, 6};
	int		dashes_no = 0;
	int		ndigits = 0;
	int32	area,
			group,
			serial;

	area = group = serial = result = 0;

	for (c = usa_ssn_str; *c != 0; c++)
	{
		if (*c >= '0' && *c <= '9')
		{
			if (ndigits < 3) area = area * 10 + *c - '0';
			else if (ndigits < 5) group = group * 10 + *c - '0';
			else if (ndigits < 9) serial = serial * 10 + *c - '0';
			result = result * 10 + *c - '0';
			ndigits++;
		}
		else if (*c == '-')
		{
			int pos = c - usa_ssn_str;
			if (dashes_no < 2 && pos == dashes[dashes_no])
				dashes_no++;
			else
				ereport(ERROR,
						(errmsg("Invalid format of input data \"%s\".", usa_ssn_str),
						 errhint("Valid formats are: AAA-BB-CCCC or AAABBCCCC")));
		}
		else
			ereport(ERROR,
					(errmsg("Unexpected character '%c' in input data \"%s\".", *c, usa_ssn_str),
					 errhint("Valid SSN consists of digits and optional dashes.")));

	}
	if (ndigits != 9)
		ereport(ERROR,
				(errmsg("Invalid number of digits (%d) in input data \"%s\".", ndigits, usa_ssn_str),
				 errhint("Valid SSN consists of 9 digits.")));

	if (!usa_ssn_is_valid(area, group, serial))
		ereport(ERROR,
				(errmsg("SSN number \"%s\" is invalid.", usa_ssn_str)));

	PG_RETURN_USASSN(result);

}
/*static usa_ssn_t
cstring_to_usa_ssn(char *usa_ssn_str)
{
    char   *c;
    usa_ssn_t   result;
    char    dashes[] = {3, 6};
    int     dashes_no = 0;
    int     ndigits = 0;
    int32   area,
            group,
            serial;

    area = group = serial = 0;

    for (c = usa_ssn_str; *c != 0; c++)
    {
        if (isdigit(*c))
        {
            if (ndigits < 3) area = area * 10 + DIGITTOINT(*c);
            else if (ndigits < 5) group = group * 10 + DIGITTOINT(*c);
            else if (ndigits < 9) serial = serial * 10 + DIGITTOINT(*c);
            ndigits++;
        }
        else if (*c == '-')
        {
            int pos = c - usa_ssn_str;
            if (dashes_no < 2 && pos == dashes[dashes_no])
                dashes_no++;
            else
                ereport(ERROR,
                        (errmsg("invalid format of input data %s", usa_ssn_str),
                         errhint("Valid formats are: AAA-BB-CCCC or AAABBCCCC")));
        }
        else
            ereport(ERROR,
                    (errmsg("unexpected character '%c' in input data %s", *c, usa_ssn_str),
                     errhint("Valid SSN consists of digits and optional dashes")));

    }
    if (ndigits != 9)
        ereport(ERROR,
                (errmsg("invalid number of digits (%d) in input data %s", ndigits, usa_ssn_str),
                 errhint("Valid SSN consists of 9 digits")));

    if (!usa_ssn_is_valid(area, group, serial))
        ereport(ERROR,
                (errmsg("SSN number %s is invalid", usa_ssn_str)));

    result = serial + (group << 14) + (area << 21);
    PG_RETURN_USASSN(result);

}*/

/* Convert the internal representation to the AAA-BB-CCCC output string */
static char *
usa_ssn_to_cstring(usa_ssn_t usa_ssn)
{
    int32   remainder = usa_ssn;
    int32   digit_value1;
    int32   digit_value2;
    int32   digit_value3;
    int32   digit_value4;
    int32   digit_value5;
    int32   digit_value6;
    int32   digit_value7;
    int32   digit_value8;
    int32   digit_value9;
    char    *usa_ssn_str;
    const char *ssn_format = 0;
    const char *p;
    int32   str_pos = 0;

    if (usa_ssn > 999999999 || usa_ssn < 0)
        ereport(ERROR,
                (errmsg("Invalid data."),
                 errhint("The SSN data is out of range.")));

    /*
    digit_value1 = remainder * .00000001;
    remainder = remainder - digit_value1 * 100000000;
    digit_value2 = remainder * .0000001;
    remainder = remainder - digit_value2 * 10000000;
    digit_value3 = remainder * .000001;
    remainder = remainder - digit_value3 * 1000000;
    digit_value4 = remainder * .00001;
    remainder = remainder - digit_value4 * 100000;
    digit_value5 = remainder * .0001;
    remainder = remainder - digit_value5 * 10000;
    digit_value6 = remainder * .001;
    remainder = remainder - digit_value6 * 1000;
    digit_value7 = remainder * .01;
    remainder = remainder - digit_value7 * 100;
    digit_value8 = remainder * .1;
    digit_value9 = remainder - digit_value8 * 10;
    */
    digit_value9 = remainder % 10;
    remainder *= .1;
    digit_value8 = remainder % 10;
    remainder *= .1;
    digit_value7 = remainder % 10;
    remainder *= .1;
    digit_value6 = remainder % 10;
    remainder *= .1;
    digit_value5 = remainder % 10;
    remainder *= .1;
    digit_value4 = remainder % 10;
    remainder *= .1;
    digit_value3 = remainder % 10;
    remainder *= .1;
    digit_value2 = remainder % 10;
    remainder *= .1;
    digit_value1 = remainder; // % 10;

    // Force pg_dump & other DBA tasks to dump the full SSN so that backups are complete & valid.
    if (!superuser())
        ssn_format = GetConfigOption("usa_ssn.format", true, false);

    if (!ssn_format) {
        usa_ssn_str = palloc(12);
        usa_ssn_str[0]  = '0' + digit_value1;
        usa_ssn_str[1]  = '0' + digit_value2;
        usa_ssn_str[2]  = '0' + digit_value3;
        usa_ssn_str[3]  = '-';
        usa_ssn_str[4]  = '0' + digit_value4;
        usa_ssn_str[5]  = '0' + digit_value5;
        usa_ssn_str[6]  = '-';
        usa_ssn_str[7]  = '0' + digit_value6;
        usa_ssn_str[8]  = '0' + digit_value7;
        usa_ssn_str[9]  = '0' + digit_value8;
        usa_ssn_str[10] = '0' + digit_value9;
        usa_ssn_str[11] = '\0';
        return usa_ssn_str;
    }

    usa_ssn_str = palloc(strlen(ssn_format) + 1);
    for (p = ssn_format; *p; p++)
    {
        switch (*p) {
            case '-':
                usa_ssn_str[str_pos++] = '-';
            break;
            case '1':
                usa_ssn_str[str_pos++] = '0' + digit_value1;
            break;
            case '2':
                usa_ssn_str[str_pos++] = '0' + digit_value2;
            break;
            case '3':
                usa_ssn_str[str_pos++] = '0' + digit_value3;
            break;
            case '4':
                usa_ssn_str[str_pos++] = '0' + digit_value4;
            break;
            case '5':
                usa_ssn_str[str_pos++] = '0' + digit_value5;
            break;
            case '6':
                usa_ssn_str[str_pos++] = '0' + digit_value6;
            break;
            case '7':
                usa_ssn_str[str_pos++] = '0' + digit_value7;
            break;
            case '8':
                usa_ssn_str[str_pos++] = '0' + digit_value8;
            break;
            case '9':
                usa_ssn_str[str_pos++] = '0' + digit_value9;
            break;
            case 'X':
                usa_ssn_str[str_pos++] = 'X';
            break;
            case 'x':
                usa_ssn_str[str_pos++] = 'x';
            break;
            case '*':
                usa_ssn_str[str_pos++] = '*';
            break;
            case '#':
                usa_ssn_str[str_pos++] = '#';
            break;
            case ' ':
                usa_ssn_str[str_pos++] = ' ';
            break;
            default:
                usa_ssn_str[str_pos++] = '?';
            break;
        }
    }
    usa_ssn_str[str_pos++] = '\0';
    return usa_ssn_str;
}

/* Convert the internal representation to the AAA-BB-CCCC output string
static char *
usa_ssn_to_cstring(usa_ssn_t usa_ssn)
{
    int32   remainder = usa_ssn;
    int32   digit_value;
    //int32   n = usa_ssn;
    char    *usa_ssn_str = palloc(12);

    if (usa_ssn > 999999999 || usa_ssn < 0)
        ereport(ERROR,
                (errmsg("Invalid data."),
                 errhint("The SSN data is out of range.")));

    digit_value = remainder * .00000001;
    usa_ssn_str[0] = '0' + digit_value;
    remainder = remainder - digit_value * 100000000;
    digit_value = remainder * .0000001;
    usa_ssn_str[1] = '0' + digit_value;
    remainder = remainder - digit_value * 10000000;
    digit_value = remainder * .000001;
    usa_ssn_str[2] = '0' + digit_value;
    remainder = remainder - digit_value * 1000000;
    digit_value = remainder * .00001;
    usa_ssn_str[3] = '-';
    usa_ssn_str[4] = '0' + digit_value;
    remainder = remainder - digit_value * 100000;
    digit_value = remainder * .0001;
    usa_ssn_str[5] = '0' + digit_value;
    remainder = remainder - digit_value * 10000;
    digit_value = remainder * .001;
    usa_ssn_str[6] = '-';
    usa_ssn_str[7] = '0' + digit_value;
    remainder = remainder - digit_value * 1000;
    digit_value = remainder * .01;
    usa_ssn_str[8] = '0' + digit_value;
    remainder = remainder - digit_value * 100;
    digit_value = remainder * .1;
    usa_ssn_str[9] = '0' + digit_value;
    remainder = remainder - digit_value * 10;
    usa_ssn_str[10] = '0' + remainder;
    usa_ssn_str[11] = '\0';

    *usa_ssn_str[11] = '\0';
    usa_ssn_str[10] = '0' + n % 10;
    n *= .1;
    usa_ssn_str[9] = '0' + n % 10;
    n *= .1;
    usa_ssn_str[8] = '0' + n % 10;
    n *= .1;
    usa_ssn_str[7] = '0' + n % 10;
    n *= .1;
    usa_ssn_str[6] = '-';
    usa_ssn_str[5] = '0' + n % 10;
    n *= .1;
    usa_ssn_str[4] = '0' + n % 10;
    n *= .1;
    usa_ssn_str[3] = '-';
    usa_ssn_str[2] = '0' + n % 10;
    n *= .1;
    usa_ssn_str[1] = '0' + n % 10;
    n *= .1;
    usa_ssn_str[0] = '0' + n % 10;*

    return usa_ssn_str;
}*/

/* Convert the internal representation to the AAA-BB-CCCC output string */
/*static char *
usa_ssn_to_cstring(usa_ssn_t usa_ssn)
{
	int32	area,
			group,
			serial;
	int32	ndigits;
	char   *usa_ssn_str = palloc(12);

	if (usa_ssn < 0)
		* Error out *;

	serial  = usa_ssn & ((1 << 14) - 1); * first 14 bits /
	group = (usa_ssn >> 14) & 127; * next 7 bits /
	area = (usa_ssn >> 21);  * last 10 bits (highest one is always 0) /

	if (!usa_ssn_is_valid(area, group, serial))
		ereport(ERROR,
				(errmsg("SSN number %s is invalid", usa_ssn_str)));

	if ((ndigits = snprintf(usa_ssn_str, 12, "%03d-%02d-%04d", area, group, serial)) != 11)
		ereport(ERROR,
				(errmsg("invalid size (%d) of in input data %s", ndigits-2, usa_ssn_str),
				 errhint("Valid SSN consists of 9 digits")));

	if (usa_ssn_str[3] != '-' || usa_ssn_str[6] != '-')
		ereport(ERROR,
				(errmsg("invalid format of input data %s", usa_ssn_str),
				 errhint("Valid formats are: AAA-BB-CCCC or AAABBCCCC")));


	return usa_ssn_str;
}*/

/*
 * Check whether 3 components of the SSN are valid. Invalid SSNs are:
 * - containing 0 in one of the components.
 * - those with are code equal to 666 or greater or equal to 900.
 * Note: Area is not validated against area number groups due to
 * SSN randomization process, which will take place after June 25th, 2011.
 */
static bool
usa_ssn_is_valid(int32 area, int32 group, int32 serial)
{
	if (area == 0 || group == 0 || serial == 0)
		return false;

	if (area == 666 || area >= 900)
		return false;

	return true;
}

