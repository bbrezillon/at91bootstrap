/* ----------------------------------------------------------------------------
 *         ATMEL Microcontroller Software Support
 * ----------------------------------------------------------------------------
 * Copyright (c) 2016, Atmel Corporation
 *
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * - Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the disclaimer below.
 *
 * Atmel's name may not be used to endorse or promote products derived from
 * this software without specific prior written permission.
 *
 * DISCLAIMER: THIS SOFTWARE IS PROVIDED BY ATMEL "AS IS" AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT ARE
 * DISCLAIMED. IN NO EVENT SHALL ATMEL BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA,
 * OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
#include "backup.h"
#include "debug.h"
#include "hardware.h"
#include "arch/at91_sfrbu.h"

static struct at91_pm_bu {
	int suspended;
	unsigned long tmp;
	unsigned long resume;
	int *canary;
} *pm_bu;

static int resuming = -1;

static int backup_mode(void)
{
	int ret;

	resuming = 0;

	ret = readl(AT91C_BASE_SFRBU + SFRBU_DDRBUMCR);
	if (ret == 0)
		return 0;

	do {
		ret = readl(AT91C_BASE_SECUMOD + 0x14);
	} while (ret == 0);

	pm_bu = (struct at91_pm_bu *)AT91C_BASE_SECURAM;
	if (!pm_bu->suspended)
		return 0;

	resuming = 1;
	return 1;
}

int backup_resume(void)
{
	if (resuming == -1)
		backup_mode();

	return resuming;
}

unsigned long backup_mode_resume(void)
{
	if (!backup_resume())
		return 0;

	writel(0, AT91C_BASE_SFRBU + SFRBU_DDRBUMCR);

	if (*pm_bu->canary != 0xa5a5a5a5)
		return 0;

	return pm_bu->resume;
}
