#include "types.h"
#include "time.h"
#include "rtc.h"
#define __CONFIG_H__
#include "stdio.h"


static int month[12] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
static const char format[] = "%.3s %.3s%3d %.2d:%.2d:%.2d %d\n";
static char tbuf[	         3+1+ 3+1+20+1+20+1+20+1+20+1+20+1 + 1];
static const char * const dayname[] = {
    "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat" };
static const char * const monthname[] = {
    "Jan", "Feb", "Mar", "Apr", "May", "Jun",
    "Jul", "Aug", "Sep", "Oct", "Nov", "Dec" };

static int
yeartoday(int year)
{
    return ((year % 4) ? 365 : 366);
}

time_t
mktime(struct tm *tm)
{
    time_t n;
    int i, days = 0;

    if ((tm->tm_sec > 61) || (tm->tm_min > 59) || 
			(tm->tm_hour > 23) || (tm->tm_mday > 31) || 
			(tm->tm_mon > 12) || (tm->tm_year < 70)) {
		return (time_t)-1;
    }

    n = tm->tm_sec + 60 * tm->tm_min + 3600 * tm->tm_hour;
    n += (tm->tm_mday - 1) * 3600 * 24;

    month[1] = (yeartoday(tm->tm_year) == 366) ? 29 : 28;
    for (i = tm->tm_mon - 1; i >= 0; i--)
		days += month[i];

    for (i = 70; i < tm->tm_year; i++)
		days += yeartoday(i);
    n += days * 3600 * 24;

    return (n);
}

struct tm *
localtime_r(const time_t *t, struct tm *tm)
{	
    time_t n;
	unsigned int i, j;

    n = *t % (3600 * 24);		/* hrs+mins+secs */
    tm->tm_sec = n % 60;
    n /= 60;
    tm->tm_min = n % 60;
    tm->tm_hour = n / 60;
    
    n = *t / (3600 * 24);		/* days since 1/1/1970*/
    tm->tm_wday = (n + 4) % 7;	/* 1/1/70 was Thursday */
    
    for (j = 1970, i = yeartoday(j); n >= i; j++, i = yeartoday(j))
		n -= i;

    tm->tm_yday = n;
    tm->tm_year = j - 1900;

    month[1] = (i == 366) ? 29 : 28;
    for (i = 0; n >= month[i]; i++)
		n -= month[i];
    tm->tm_mon = i;
    tm->tm_mday = n + 1;
    tm->tm_isdst = 0;

    return (tm);
}

/*
 * Same as gmtime until timezones are defined. RK
 */
struct tm *
localtime(const time_t *t)
{
    static struct tm tm;
	
	localtime_r(t, &tm);
	return (&tm);
}

struct tm *
gmtime_r(const time_t *t, struct tm *tm)
{
    time_t n;
	unsigned int i, j;

    n = *t % (3600 * 24);		/* hrs+mins+secs */
    tm->tm_sec = n % 60;
    n /= 60;
    tm->tm_min = n % 60;
    tm->tm_hour = n / 60;
    
    n = *t / (3600 * 24);		/* days since 1/1/1970*/
    tm->tm_wday = (n + 4) % 7;	/* 1/1/70 was Thursday */
    
    for (j = 1970, i = yeartoday(j); n >= i; j++, i = yeartoday(j))
		n -= i;

    tm->tm_yday = n;
    tm->tm_year = j - 1900;

    month[1] = (i == 366) ? 29 : 28;
    for (i = 0; n >= month[i]; i++)
		n -= month[i];
    tm->tm_mon = i;
    tm->tm_mday = n + 1;
    tm->tm_isdst = 0;

    return (tm);
}

struct tm *
gmtime(const time_t *t)
{	
    static struct tm tm;

	gmtime_r(t, &tm);
	return (&tm);
}

char *
asctime_r(const struct tm *tp, char *buf)
{
	if (tp == 0) {
		return (0);
	}
	sprintf (buf, format,
				(tp->tm_wday < 0 || tp->tm_wday >= 7 ?
				"???" : dayname[tp->tm_wday]),
				(tp->tm_mon < 0 || tp->tm_mon >= 12 ?
				"???" : monthname[tp->tm_mon]),
				tp->tm_mday, tp->tm_hour, tp->tm_min,
				tp->tm_sec, 1900 + tp->tm_year);
	return (buf);
}

/*
 * Returns a string of the form "Day Mon dd hh:mm:ss yyyy\n"
 * which is the representation of TP in that form.
 */
char *
asctime(const struct tm *tp)
{
	return (asctime_r(tp, tbuf));
}


/*
 * Return a string as returned by asctime which
 * is the representation of *T in that form.
 */
char *
ctime(const time_t *t)
{
  /*
   * The C Standard says ctime (t) is equivalent to asctime (localtime (t)).
   * In particular, ctime and asctime must yield the same pointer.
   */
  return (asctime(localtime (t)));
}

time_t 
time(time_t *it)
{
	struct tm 	tm;
	time_t		t;
	
	GetTime(&tm);
	t = mktime(&tm);
  	if (it) {
		*it = t;
	}
	return (t);
}	
