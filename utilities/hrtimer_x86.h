/*
 * FILE: hrtimer_x86.h
 * DESCRIPTION: Hearder file for hrtimer_x86.c - a high-resolution timer on x86.
 */

#ifndef _HRTIMER_X86_H_
#define _HRTIMER_X86_H_

typedef long long hrtime_t;

/* get the elapsed time (in seconds) since startup */
double gethrtime_x86(void);

/* get the number of CPU cycles since startup */
hrtime_t gethrcycle_x86(void);

/* get the number of CPU cycles per microsecond - from Linux /proc filesystem */
double getMHZ_x86(void);

#endif	// _HRTIMER_X86_H_
