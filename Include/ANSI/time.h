#ifndef TIME_H
#define TIME_H

#ifdef  __cplusplus
extern "C" {
#endif

typedef	long	time_t;

struct tm {
	int		tm_sec;		/* seconds after the minute [0-60]. (1 leap second) */
	int		tm_min;		/* minutes after the hour [0-59] */
	int		tm_hour;	/* hours since midnight [0-23] */
	int		tm_mday;	/* day of the month [1-31] */
	int		tm_mon;		/* months since January [0-11] */
	int		tm_year;	/* years since 1900 */
	int		tm_wday;	/* days since Sunday [0-6] */
	int		tm_yday;	/* days since January 1 [0-365] */
	int		tm_isdst;	/* Daylight Savings Time flag. Unused */
	long	tm_gmtoff;	/* offset from UCT in seconds. Unused */
	char	*tm_zone;	/* timezone abbreviation. Unused */
};

extern time_t mktime(struct tm *);
extern time_t time(time_t *);
extern char *ctime(const time_t *);
extern char *asctime(const struct tm *);
extern struct tm *gmtime(const time_t *);
extern struct tm *localtime(const time_t *);

extern char *asctime_r(const struct tm *, char *buf);
extern struct tm *gmtime_r(const time_t *, struct tm *res);
extern struct tm *localtime_r(const time_t *, struct tm *res);

#ifdef  __cplusplus
}
#endif

#endif	/* TIME_H */
