/*
 * Copyright (c) 1986 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 *
 *	@(#)rkauto.c	1.2 (2.11BSD GTE) 12/30/92
 */

#include "param.h"
#include "../machine/autoconfig.h"
#include "../machine/machparam.h"

#include "rkreg.h"

rkprobe(addr,vector)
	struct rkdevice *addr;
	int vector;
{
	extern int errno;

	stuff(RKCS_IDE | RKCS_DRESET | RKCS_GO, (&(addr->rkcs)));
	DELAY(10L);
	stuff(0, (&(addr->rkcs)));

	/* in case no drive 0 was attached, the DRESET above will leave
	 * the controller in an error state. The grab below checks for this
	 * and the stuff below executes a RESET (control reset) to clear
	 * the error condition. Without this the first access to any drive
	 * will fail when the system is up.
	 * 2008-12-25 wfjm
	*/
	errno = 0;
	if ((grab(&(addr->rker)) & RKER_DRE) && !errno) {
		stuff(RKCS_RESET | RKCS_GO, (&(addr->rkcs)));
		DELAY(10L);
	}

	return(ACP_IFINTR);
}
