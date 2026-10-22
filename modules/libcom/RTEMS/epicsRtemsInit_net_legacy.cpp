/*************************************************************************\
* Copyright (c) 2026 Jeremy Lorelli
* SPDX-License-Identifier: EPICS
* EPICS BASE is distributed subject to a Software License Agreement found
* in file LICENSE that is included with this distribution.
\*************************************************************************/

#include <cstring>
#include <cstdlib>
#include <filesystem>
#include <iostream>

#include <epicsRtemsInit.h>
#include <epicsThread.h>

#include <rtems.h>
#include <rtems/libio.h>
#include <rtems/rtems_bsdnet.h>
#include <rtems/rtems_bsdnet_internal.h>
#include <bsp.h>

#ifndef RTEMS_LEGACY_STACK
#error This file cannot be used with the libBSD networking stack.
#endif

static struct rtems_bsdnet_ifconfig if0_config = {
    RTEMS_BSP_NETWORK_DRIVER_NAME,
    RTEMS_BSP_NETWORK_DRIVER_ATTACH,
};

#undef MY_DO_BOOTP
#ifndef MY_DO_BOOTP
#define MY_DO_BOOTP rtems_bsdnet_do_bootp
#endif

/**
 * \brief Init static IP configuration for an interface.
 * \param inst The interface instance.
 * \param conf The interface config.
 */
static void initStaticConfig(int inst, struct rtems_bsdnet_ifconfig* conf) {
    char buf[64];

    snprintf(buf, sizeof(buf), "RTEMS_NET_IF%d_NETMASK", inst);
    conf->ip_netmask = getenv(buf);

    snprintf(buf, sizeof(buf), "RTEMS_NET_IF%d_IP", inst);
    conf->ip_address = getenv(buf);

    snprintf(buf, sizeof(buf), "RTEMS_NET_IF%d_MTU", inst);
    int mtu = 0;
    if (auto* envp = getenv(buf); (mtu = atoi(envp)) > 0) {
        conf->mtu = mtu;
    }

    snprintf(buf, sizeof(buf), "RTEMS_NET_IF%d_NAME", inst);
    if (auto* envp = getenv(buf)) {
        conf->name = envp;
    }
}

static int rtemsNetLegacyInitialize() {
    rtems_bsdnet_config.ifconfig = &if0_config;

    if (rtems_bsdnet_config.network_task_priority == 0) {
        if (unsigned int prio; epicsThreadHighestPriorityLevelBelow(epicsThreadPriorityScanLow, &prio)
                                            == epicsThreadBooleanStatusSuccess)
        {
            rtems_bsdnet_config.network_task_priority = prio;
        }
        else
        {
            std::cout << "error: rtemsNetLegacyInitialize: Unable to determine network task priority" <<
                std::endl;
            return -1;
        }
    }

    /* TODO */
    rtems_bsdnet_config.mbuf_bytecount = 2048 * 1024;
    rtems_bsdnet_config.mbuf_cluster_bytecount = 5120 * 1024;

    rtems_bsdnet_config.gateway = getenv("RTEMS_NET_GATEWAY");

    /* Default bootp selection */
    rtems_bsdnet_config.bootp = MY_DO_BOOTP;

    if (auto* envp = getenv("RTEMS_NET_DO_BOOTP"); envp && strcmp(envp, "0")) {
        rtems_bsdnet_config.bootp = rtems_bsdnet_do_bootp;
    } else {
        /* if we're not using bootp, we must be static */
        initStaticConfig(0, &if0_config);
    }

    if (auto* envp = getenv("RTEMS_NET_NTP_IP")) {
        rtems_bsdnet_config.ntp_server[0] = envp;
    }

    if (rtems_bsdnet_initialize_network() != 0) {
        std::cout << "error: rtems_bsdnet_initialize_network failed" << std::endl;
        return -1;
    }

    /* Propagate BOOTP supplied parameters to environment */
    if (rtems_bsdnet_config.bootp) {
        if (rtems_bsdnet_bootp_cmdline[0]) {
            setenv("RTEMS_BOOT_CMD_LINE", rtems_bsdnet_bootp_cmdline, 1);
        } else {
            std::cout << "warn: rtemsnetLegacyInitialize: No cmdline supplied by BOOTP" <<
                std::endl;
        }

        if (rtems_bsdnet_bootp_boot_file_name[0]) {
            setenv("RTEMS_NFS_MOUNT_PATH", rtems_bsdnet_bootp_boot_file_name, 1);
        } else {
            std::cout << "warn: rtemsNetLegacyInitialize: No filename supplied by BOOTP" <<
                std::endl;
        }
    }

    return 0;
}

void epicRtemsInit_net() {
    epicsRtemsInitRegisterHandler(
        "system", "netLegacy", rtemsInit_Order_net, true, rtemsNetLegacyInitialize);
}
