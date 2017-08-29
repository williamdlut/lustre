/*
 * GPL HEADER START
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 only,
 * as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License version 2 for more details (a copy is included
 * in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU General Public License
 * version 2 along with this program; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 021110-1307, USA
 *
 * GPL HEADER END
 */
/*
 * Copyright (c) 2010, Oracle and/or its affiliates. All rights reserved.
 * Copyright (c) 2012, 2013, Intel Corporation.
 */
/*
 * This file is part of Lustre, http://www.lustre.org/
 * Lustre is a trademark of Sun Microsystems, Inc.
 *
 * Please see comments in libcfs/include/libcfs/libcfs_cpu.h for introduction
 *
 * Author: liang@whamcloud.com
 */

#define DEBUG_SUBSYSTEM S_LIBCFS

#include <libcfs/libcfs.h>

/** Global CPU partition table */
struct cfs_cpt_table *cfs_cpt_table __read_mostly = NULL;
EXPORT_SYMBOL(cfs_cpt_table);

#ifndef HAVE_LIBCFS_CPT

#define CFS_CPU_VERSION_MAGIC           0xbabecafe

#define CFS_CPT_DISTANCE		1	/* Arbitrary positive value */

struct cfs_cpt_table *cfs_cpt_table_alloc(int ncpt)
{
	struct cfs_cpt_table *cptab;

	if (ncpt != 1) {
		trace_cerror_cpu_bad_ump_setup(ncpt);
		return NULL;
	}

	LIBCFS_ALLOC(cptab, sizeof(*cptab));
	if (cptab != NULL) {
		cptab->ctb_version = CFS_CPU_VERSION_MAGIC;
		cpu_set(0, cptab->ctb_cpumask);
		node_set(0, cptab->ctb_nodemask);
		cptab->ctb_nparts  = ncpt;
	}

	return cptab;
}
EXPORT_SYMBOL(cfs_cpt_table_alloc);

void cfs_cpt_table_free(struct cfs_cpt_table *cptab)
{
	LASSERT(cptab->ctb_version == CFS_CPU_VERSION_MAGIC);

	LIBCFS_FREE(cptab, sizeof(*cptab));
}
EXPORT_SYMBOL(cfs_cpt_table_free);

int cfs_cpt_table_print(struct cfs_cpt_table *cptab, char *buf, int len)
{
	int rc = 0;

	rc = snprintf(buf, len, "%d\t: %d\n", 0, 0);
	len -= rc;
	if (len <= 0)
		return -EFBIG;

	return rc;
}
EXPORT_SYMBOL(cfs_cpt_table_print);

int cfs_cpt_distance_print(struct cfs_cpt_table *cptab, char *buf, int len)
{
	int	rc = 0;

	rc = snprintf(buf, len, "%d\t: %d:%d\n", 0, CFS_CPT_DISTANCE);
	len -= rc;
	if (len <= 0)
		return -EFBIG;

	return rc;
}
EXPORT_SYMBOL(cfs_cpt_distance_print);

int cfs_cpt_number(struct cfs_cpt_table *cptab)
{
	return 1;
}
EXPORT_SYMBOL(cfs_cpt_number);

int cfs_cpt_weight(struct cfs_cpt_table *cptab, int cpt)
{
	return 1;
}
EXPORT_SYMBOL(cfs_cpt_weight);

int cfs_cpt_online(struct cfs_cpt_table *cptab, int cpt)
{
	return 1;
}
EXPORT_SYMBOL(cfs_cpt_online);

cpumask_t *cfs_cpt_cpumask(struct cfs_cpt_table *cptab, int cpt)
{
	return &cptab->ctb_mask;
}
EXPORT_SYMBOL(cfs_cpt_cpumask);

nodemask_t *cfs_cpt_nodemask(struct cfs_cpt_table *cptab, int cpt)
{
	return &cptab->ctb_nodemask;
}
EXPORT_SYMBOL(cfs_cpt_nodemask);

unsigned cfs_cpt_distance(struct cfs_cpt_table *cptab, int cpt1, int cpt2)
{
	return CFS_CPT_DISTANCE;
}
EXPORT_SYMBOL(cfs_cpt_distance);

int cfs_cpt_set_cpu(struct cfs_cpt_table *cptab, int cpt, int cpu)
{
	return 1;
}
EXPORT_SYMBOL(cfs_cpt_set_cpu);

void cfs_cpt_unset_cpu(struct cfs_cpt_table *cptab, int cpt, int cpu)
{
}
EXPORT_SYMBOL(cfs_cpt_unset_cpu);

int cfs_cpt_set_cpumask(struct cfs_cpt_table *cptab, int cpt,
			const cpumask_t *mask)
{
	return 1;
}
EXPORT_SYMBOL(cfs_cpt_set_cpumask);

void cfs_cpt_unset_cpumask(struct cfs_cpt_table *cptab, int cpt,
			   const cpumask_t *mask)
{
}
EXPORT_SYMBOL(cfs_cpt_unset_cpumask);

int cfs_cpt_set_node(struct cfs_cpt_table *cptab, int cpt, int node)
{
	return 1;
}
EXPORT_SYMBOL(cfs_cpt_set_node);

void cfs_cpt_unset_node(struct cfs_cpt_table *cptab, int cpt, int node)
{
}
EXPORT_SYMBOL(cfs_cpt_unset_node);

int cfs_cpt_set_nodemask(struct cfs_cpt_table *cptab, int cpt,
			 const nodemask_t *mask)
{
	return 1;
}
EXPORT_SYMBOL(cfs_cpt_set_nodemask);

void cfs_cpt_unset_nodemask(struct cfs_cpt_table *cptab, int cpt,
			    const nodemask_t *mask)
{
}
EXPORT_SYMBOL(cfs_cpt_unset_nodemask);

int cfs_cpt_spread_node(struct cfs_cpt_table *cptab, int cpt)
{
	return 0;
}
EXPORT_SYMBOL(cfs_cpt_spread_node);

int cfs_cpt_current(struct cfs_cpt_table *cptab, int remap)
{
	return 0;
}
EXPORT_SYMBOL(cfs_cpt_current);

int cfs_cpt_of_cpu(struct cfs_cpt_table *cptab, int cpu)
{
	return 0;
}
EXPORT_SYMBOL(cfs_cpt_of_cpu);

int cfs_cpt_of_node(struct cfs_cpt_table *cptab, int node)
{
	return 0;
}
EXPORT_SYMBOL(cfs_cpt_of_node);

int cfs_cpt_bind(struct cfs_cpt_table *cptab, int cpt)
{
	return 0;
}
EXPORT_SYMBOL(cfs_cpt_bind);

void cfs_cpu_fini(void)
{
	if (cfs_cpt_table != NULL) {
		cfs_cpt_table_free(cfs_cpt_table);
		cfs_cpt_table = NULL;
	}
}

int cfs_cpu_init(void)
{
	cfs_cpt_table = cfs_cpt_table_alloc(1);

	return cfs_cpt_table != NULL ? 0 : -1;
}

#endif /* HAVE_LIBCFS_CPT */
