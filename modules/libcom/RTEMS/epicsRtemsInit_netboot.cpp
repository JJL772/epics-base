/*************************************************************************\
* Copyright (c) 2026 Jeremy Lorelli
* SPDX-License-Identifier: EPICS
* EPICS BASE is distributed subject to a Software License Agreement found
* in file LICENSE that is included with this distribution.
\*************************************************************************/


#include <cstring>
#include <cstdlib>
#include <iostream>

#include <epicsRtemsInit.h>
#include <bsp.h>
#include <rtems/bspcmdline.h>

/* uC5282 needs to call into the bootloader for this */
#ifdef LIBBSP_M68K_UC5282_BSP_H
#define getbenv getenv
#else
#define getbenv getenv
#endif

static struct {
    const char* booloader;
    const char* epics;
} envMapping[] = {
    {"BP_FILE", "RTEMS_NET_BOOT_FILE"},
    {"BP_PARM", "RTEMS_BOOT_CMD_LINE"},
    {"BP_NTP1", "RTEMS_NET_NTP_IP"},
    {"BP_GTWY", ""},
    {"BP_MYIP", "RTEMS_NET_IFACE_0"},
};

/**
 * \brief Initialize the environment with variables provided by the bootloader.
 * The 'netboot' bootloader written by Till Straumann uses these BP_XXX
 * variables to pass the boot configuration to the loaded system.
 */
static int rtemsInitBootloaderEnv() {
    bool anyProvided = false;

    printf("cmdline: %s\n", rtems_bsp_cmdline_get());

    for (const auto [k, v] : envMapping) {
        if (auto* val = getenv(k)) {
            setenv(v, val, 1);
            anyProvided = true;
        }
    }

    /* Already configured by the bootloader, no need for bootp anymore. */
    if (anyProvided) {
        setenv("RTEMS_NET_DO_BOOTP", "0", 1);
    }

    return 0;
}

void epicsRtemsInit_bootloaderenv() {
    epicsRtemsInitRegisterHandler(
        "system", "bootloader.env", rtemsInit_Order_early, true, rtemsInitBootloaderEnv);
}

