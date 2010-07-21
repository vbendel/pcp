/*
 * Copyright (c) 2004 Silicon Graphics, Inc.  All Rights Reserved.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 * for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
 */

#ifndef __PMDASOLARIS_COMMON_H
#define __PMDASOLARIS_COMMON_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "pmapi.h"
#include "impl.h"
#include "pmda.h"
#include "domain.h"

#include <kstat.h>
#include <sys/sysinfo.h>

/*
 * kstat method controls
 *
 * md_method choices (see below) ... must be contiguous integers so
 * we can index directly into method[]
 */
#define M_SYSINFO	0
#define M_DISK		1
#define M_NETIF		2
#define M_ZPOOL		3
#define M_ZFS		4
#define M_ZPOOL_PERDISK	5
#define M_NETLINK	6
#define M_KVM		7

typedef struct {
    void	(*m_init)(int);
    void	(*m_prefetch)(void);
    int		(*m_fetch)(pmdaMetric *, int, pmAtomValue *);
    int		fetched;
} method_t;

extern method_t		methodtab[];
extern int		methodtab_sz;

extern void init_data(int);

extern void sysinfo_init(int);
extern void sysinfo_prefetch(void);
extern int sysinfo_fetch(pmdaMetric *, int, pmAtomValue *);

extern void disk_init(int);
extern void disk_prefetch(void);
extern int disk_fetch(pmdaMetric *, int, pmAtomValue *);

void zpool_init(int);
void zpool_refresh(void);
int zpool_fetch(pmdaMetric *, int, pmAtomValue *);

void zfs_init(int);
void zfs_refresh(void);
int zfs_fetch(pmdaMetric *, int, pmAtomValue *);

void zpool_perdisk_init(int);
void zpool_perdisk_refresh(void);
int zpool_perdisk_fetch(pmdaMetric *, int, pmAtomValue *);

void netlink_init(int);
void netlink_refresh(void);
int netlink_fetch(pmdaMetric *, int, pmAtomValue *);

void kvm_init(int);
void kvm_refresh(void);
int kvm_fetch(pmdaMetric *, int, pmAtomValue *);

/*
 * metric descriptions
 */
typedef struct {
    pmDesc	md_desc;	// PMDA's idea of the semantics
    int		md_method;	// specific kstat method
    ptrdiff_t	md_offset;	// offset into kstat stats structure
} metricdesc_t;

extern metricdesc_t	metricdesc[];
extern pmdaMetric	*metrictab;
extern int		metrictab_sz;

#define DISK_INDOM	0
#define CPU_INDOM	1
#define NETIF_INDOM	2
#define ZPOOL_INDOM	3
#define ZFS_INDOM	4
#define ZPOOL_PERDISK_INDOM	5
#define NETLINK_INDOM	6
#define ZFS_SNAP_INDOM	7
#define LOADAVG_INDOM	8

extern pmdaIndom	indomtab[];
extern int		indomtab_sz;

/*
 * kstat() control
 */
extern kstat_ctl_t		*kc;

/* Snarfed from usr/src/uts/common/fs/fsflush.c in OpenSolaris source tree */
typedef struct {
        ulong_t fsf_scan;       /* number of pages scanned */
        ulong_t fsf_examined;   /* number of page_t's actually examined, can */
                                /* be less than fsf_scan due to large pages */
        ulong_t fsf_locked;     /* pages we actually page_lock()ed */
        ulong_t fsf_modified;   /* number of modified pages found */
        ulong_t fsf_coalesce;   /* number of page coalesces done */
        ulong_t fsf_time;       /* nanoseconds of run time */
        ulong_t fsf_releases;   /* number of page_release() done */
} fsf_stat_t;

#endif
