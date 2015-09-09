//win_crap.c

#include "win_crap.h"

#ifdef _WIN32

#include <time.h>
#include "memory.h"
#include "windows.h"

void bcopy(void* pFrom, void* pTo, U32 cb) {
	memcpy(pTo, pFrom, cb);
}



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

struct tm * gmtime_r(const time_t *t, struct tm *tm){
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


int SetTime( struct tm *t ){
	SYSTEMTIME	sysTime;

	sysTime.wDay			= t->tm_mday;
	sysTime.wHour			= t->tm_hour;
	sysTime.wMilliseconds	= 0;
	sysTime.wMinute			= t->tm_min;
	sysTime.wMonth			= t->tm_mon;
	sysTime.wSecond			= t->tm_sec;
	sysTime.wYear			= t->tm_year;

	return SetSystemTime( &sysTime );
}
#endif