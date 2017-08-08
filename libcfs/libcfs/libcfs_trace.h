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
 * version 2 along with this program; If not, see
 * http://www.gnu.org/licenses/gpl-2.0.html
 *
 * GPL HEADER END
 */
/*
 * Copyright (c) 2016, James Simmons <jsimmons@infradead.org>
 */
/*
 * This file is part of Lustre, http://www.lustre.org/
 * Lustre is a trademark of Seagate, Inc.
 *
 * libcfs tracepoint handling
 */

#ifndef DEBUG_SUBSYSTEM
#define DEBUG_SUBSYSTEM S_LIBCFS
#endif

#if !defined(__LIBCFS_TRACE_H) || defined(TRACE_HEADER_MULTI_READ)
#define __LIBCFS_TRACE_H

#include <linux/limits.h>
#include <linux/tracepoint.h>
#include <libcfs/linux/linux-misc.h> /* for kbasename */
#include <libcfs/linux/linux-time.h> /* for timespec64 */
#include <libcfs/libcfs_crypto.h>
#include <libcfs/libcfs_debug.h>

#undef TRACE_SYSTEM
#define TRACE_SYSTEM libcfs

/* create empty functions when tracing is disabled */
#if !defined(CONFIG_LUSTRE_TRACEPOINT)
#undef TRACE_EVENT
#define TRACE_EVENT(name, proto, ...) \
static inline void trace_ ## name(proto) {}
#undef TRACE_EVENT_CONDITION
#define TRACE_EVENT_CONDITION(name, proto, ...) \
static inline void trace_ ## name(proto) {}

#endif /* !CONFIG_LUSTRE_TRACEPOINT */

extern struct ratelimit_state libcfs_trace_rs;
struct cfs_cpt_table;

TRACE_EVENT(libcfs_config_crypto,
	TP_PROTO(const char *msg_file, int msg_line, const char *msg_fn,
		 const char *algo, int speed),
	TP_ARGS(msg_file, msg_line, msg_fn, algo, speed),
	TP_STRUCT__entry(
		__string(msg_file, msg_file ? msg_file : "<nofile>")
		__field(int, msg_line)
		__string(msg_func, msg_fn ? msg_fn : "<nofunc>")
		__string(algo, algo)
		__field(int, speed)
	),
	TP_fast_assign(
		__assign_str(msg_file, kbasename(msg_file));
		__entry->msg_line = msg_line;
		__assign_str(msg_func, msg_fn);
		__assign_str(algo, algo);
		__entry->speed = speed;
	),
	TP_printk("(%s:%d:%s) Crypto hash algorithm %s speed = %d MB/s",
		  __get_str(msg_file), __entry->msg_line, __get_str(msg_func),
		  __get_str(algo), __entry->speed)
);

#define trace_config(hash_name, hash_speed)				\
do {									\
	trace_libcfs_config_crypto(__FILE__, __LINE__, __func__,	\
				   hash_name, hash_speed);		\
									\
	CDEBUG(D_CONFIG, "Crypto hash algorithm %s speed = %d MB/s\n",	\
	       cfs_crypto_hash_name(hash_alg),				\
	       cfs_crypto_hash_speeds[hash_alg]);			\
} while (0)

TRACE_EVENT(libcfs_console_cpu_cpt_setup,
	TP_PROTO(const char *msg_file, int msg_line, const char *msg_fn,
		 int num_nodes, int num_cpus, int num_cpts),
	TP_ARGS(msg_file, msg_line, msg_fn, num_nodes, num_cpus, num_cpts),
	TP_STRUCT__entry(
		__string(msg_file, msg_file ? msg_file : "<nofile>")
		__field(int, msg_line)
		__string(msg_func, msg_fn ? msg_fn : "<nofunc>")
		__field(int, num_nodes)
		__field(int, num_cpus)
		__field(int, num_cpts)
	),
	TP_fast_assign(
		__assign_str(msg_file, kbasename(msg_file));
		__entry->msg_line = msg_line;
		__assign_str(msg_func, msg_fn);
		__entry->num_nodes = num_nodes;
		__entry->num_cpus = num_cpus;
		__entry->num_cpts = num_cpts;
	),
	TP_printk("(%s:%d:%s) HW NUMA nodes: %d, HW CPU cores: %d, npartitions: %d",
		  __get_str(msg_file), __entry->msg_line, __get_str(msg_func),
		  __entry->num_nodes, __entry->num_cpus, __entry->num_cpts)
);

#define trace_console(num_nodes, num_cpus, num_cpts)			\
do {									\
	trace_libcfs_console_cpu_cpt_setup(__FILE__, __LINE__, __func__,\
					   num_nodes, num_cpus,		\
					   num_cpts);			\
									\
	LCONSOLE(0, "HW NUMA nodes: %d, HW CPU cores: %d, npartitions: %d\n", \
		 num_nodes, num_cpus, num_cpts);			\
} while (0)

TRACE_EVENT_CONDITION(libcfs_warning_bind_failure,
	TP_PROTO(struct ratelimit_state *rs, const char *msg_file,
		 int msg_line, const char *msg_fn, const char *thread_name,
		 int cpt, int bound),
	TP_ARGS(rs, msg_file, msg_line, msg_fn, thread_name, cpt, bound),
	TP_CONDITION(__ratelimit(rs) && bound),
	TP_STRUCT__entry(
		__string(msg_file, msg_file ? msg_file : "<nofile>")
		__field(int, msg_line)
		__string(msg_func, msg_fn ? msg_fn : "<nofunc>")
		__string(thread_name, thread_name)
		__field(int, cpt)
	),
	TP_fast_assign(
		__assign_str(msg_file, kbasename(msg_file));
		__entry->msg_line = msg_line;
		__assign_str(msg_func, msg_fn);
		__assign_str(thread_name, thread_name);
		__entry->cpt = cpt;
	),
	TP_printk("(%s:%d:%s) Unable to bind %s on CPU partition %d",
		  __get_str(msg_file), __entry->msg_line, __get_str(msg_func),
		  __get_str(thread_name), __entry->cpt)
);

#define trace_cwarn_bind_failure(thread_name, cpt, bound)		\
do {									\
	trace_libcfs_warning_bind_failure(&libcfs_trace_rs, __FILE__,	\
					  __LINE__, __func__,		\
					  thread_name, cpt, bound);	\
									\
	CWARN("Unable to bind %s on CPU partition %d\n", thread_name,	\
	      cpt);							\
} while (0)

TRACE_EVENT_CONDITION(libcfs_warning_invalid_cpt_number,
	TP_PROTO(struct ratelimit_state *rs, const char *msg_file,
		 int msg_line, const char *msg_fn, int num_cpt, int cpt_est),
	TP_ARGS(rs, msg_file, msg_line, msg_fn, num_cpt, cpt_est),
	TP_CONDITION(__ratelimit(rs)),
	TP_STRUCT__entry(
		__string(msg_file, msg_file ? msg_file : "<nofile>")
		__field(int, msg_line)
		__string(msg_func, msg_fn ? msg_fn : "<nofunc>")
		__field(int, ncpt)
		__field(int, cpt_est)
	),
	TP_fast_assign(
		__assign_str(msg_file, kbasename(msg_file));
		__entry->msg_line = msg_line;
		__assign_str(msg_func, msg_fn);
		__entry->ncpt = num_cpt;
		__entry->cpt_est = cpt_est;
	),
	TP_printk("(%s:%d:%s) CPU partition number %d is larger than suggested value (%d), your system may have performance issue or run out of memory while under pressure",
		  __get_str(msg_file), __entry->msg_line, __get_str(msg_func),
		  __entry->ncpt, __entry->cpt_est)
);

#define trace_cwarn_invalid_cpt_number(num_cpt, cpt_est)		\
do {									\
	trace_libcfs_warning_invalid_cpt_number(&libcfs_trace_rs,	\
						__FILE__, __LINE__,	\
						__func__, num_cpt,	\
						cpt_est);		\
									\
	CWARN("CPU partition number %d is larger than suggested value (%d), your system may have performance issue or run out of memory while under pressure\n",							\
	      ncpt, rc);						\
} while (0)

TRACE_EVENT_CONDITION(libcfs_warning_invalid_hash_algo,
	TP_PROTO(struct ratelimit_state *rs, const char *msg_file,
		 int msg_line, const char *msg_fn,
		 enum cfs_crypto_hash_alg hash_algo),
	TP_ARGS(rs, msg_file, msg_line, msg_fn, hash_algo),
	TP_CONDITION(__ratelimit(rs)),
	TP_STRUCT__entry(
		__string(msg_file, msg_file ? msg_file : "<nofile>")
		__field(int, msg_line)
		__string(msg_func, msg_fn ? msg_fn : "<nofunc>")
		__field(int, algo)
	),
	TP_fast_assign(
		__assign_str(msg_file, kbasename(msg_file));
		__entry->msg_line = msg_line;
		__assign_str(msg_func, msg_fn);
		__entry->algo = hash_algo;
	),
	TP_printk("(%s:%d:%s) Unsupported hash algorithm id = %d, max id is %d",
		  __get_str(msg_file), __entry->msg_line, __get_str(msg_func),
		  __entry->algo, CFS_HASH_ALG_MAX)
);

#define trace_cwarn_invalid_hash_algo(algo)				\
do {									\
	trace_libcfs_warning_invalid_hash_algo(&libcfs_trace_rs,	\
					       __FILE__, __LINE__,	\
					       __func__, algo);		\
									\
	CWARN("Unsupported hash algorithm id = %d, max id is %d\n",	\
	      hash_alg, CFS_HASH_ALG_MAX);				\
} while (0)

TRACE_EVENT_CONDITION(libcfs_warning_invalid_mask,
	TP_PROTO(struct ratelimit_state *rs, const char *msg_file,
		 int msg_line, const char *msg_fn, int len,
		 const char *str),
	TP_ARGS(rs, msg_file, msg_line, msg_fn, len, str),
	TP_CONDITION(__ratelimit(rs)),
	TP_STRUCT__entry(
		__string(msg_file, msg_file ? msg_file : "<nofile>")
		__field(int, msg_line)
		__string(msg_func, msg_fn ? msg_fn : "<nofunc>")
		__field(int, len)
		__string(mask, str)
	),
	TP_fast_assign(
		__assign_str(msg_file, kbasename(msg_file));
		__entry->msg_line = msg_line;
		__assign_str(msg_func, msg_fn);
		__entry->len = len;
		__assign_str(mask, str);
	),
	TP_printk("(%s:%d:%s) unknown mask '%.*s'.\n mask usage: [+|-]<all|type> ...",
		  __get_str(msg_file), __entry->msg_line, __get_str(msg_func),
		  __entry->len, __get_str(mask))
);

#define trace_cwarn_invalid_mask(len, mask)				\
do {									\
	trace_libcfs_warning_invalid_mask(&libcfs_trace_rs, __FILE__,	\
					  __LINE__, __func__, len,	\
					  mask);			\
									\
	CWARN("unknown mask '%.*s'.\n mask usage: [+|-]<all|type> ...\n",\
	      len, mask);						\
} while (0)

TRACE_EVENT_CONDITION(libcfs_warning_lock_no_key,
	TP_PROTO(struct ratelimit_state *rs, struct lock_class_key *key,
		 const char *msg_file, int msg_line, const char *msg_fn),
	TP_ARGS(rs, key, msg_file, msg_line, msg_fn),
	TP_CONDITION(__ratelimit(rs) && !key),
	TP_STRUCT__entry(
		__string(msg_file, msg_file ? msg_file : "<nofile>")
		__field(int, msg_line)
		__string(msg_func, msg_fn ? msg_fn : "<nofunc>")
	),
	TP_fast_assign(
		__assign_str(msg_file, kbasename(msg_file));
		__entry->msg_line = msg_line;
		__assign_str(msg_func, msg_fn);
	),
	TP_printk("(%s:%d:%s) Cannot setup class key for percpt lock, you may see recursive locking warnings which are actually fake.",
		  __get_str(msg_file), __entry->msg_line, __get_str(msg_func))
);

#define trace_cwarn_lock_no_key(key)					\
do {									\
	trace_libcfs_warning_lock_no_key(&libcfs_trace_rs, key,		\
					 __FILE__, __LINE__, __func__);	\
									\
	CWARN("can't show stack: kernel doesn't export show_task\n");	\
} while (0)

TRACE_EVENT_CONDITION(libcfs_warning_no_show_task,
	TP_PROTO(struct ratelimit_state *rs, const char *msg_file,
		 int msg_line, const char *msg_fn),
	TP_ARGS(rs, msg_file, msg_line, msg_fn),
	TP_CONDITION(__ratelimit(rs)),
	TP_STRUCT__entry(
		__string(msg_file, msg_file ? msg_file : "<nofile>")
		__field(int, msg_line)
		__string(msg_func, msg_fn ? msg_fn : "<nofunc>")
	),
	TP_fast_assign(
		__assign_str(msg_file, kbasename(msg_file));
		__entry->msg_line = msg_line;
		__assign_str(msg_func, msg_fn);
	),
	TP_printk("(%s:%d:%s) can't show stack: kernel doesn't export show_task",
		  __get_str(msg_file), __entry->msg_line, __get_str(msg_func))
);

#define trace_cwarn_no_show_task(void)					\
do {									\
	trace_libcfs_warning_no_show_task(&libcfs_trace_rs, __FILE__,	\
					  __LINE__, __func__);		\
									\
	CWARN("can't show stack: kernel doesn't export show_task\n");	\
} while (0)

TRACE_EVENT_CONDITION(libcfs_warning_unsupported_mask,
	TP_PROTO(struct ratelimit_state *rs, const char *msg_file,
		 int msg_line, const char *msg_fn),
	TP_ARGS(rs, msg_file, msg_line, msg_fn),
	TP_CONDITION(__ratelimit(rs)),
	TP_STRUCT__entry(
		__string(msg_file, msg_file ? msg_file : "<nofile>")
		__field(int, msg_line)
		__string(msg_func, msg_fn ? msg_fn : "<nofunc>")
	),
	TP_fast_assign(
		__assign_str(msg_file, kbasename(msg_file));
		__entry->msg_line = msg_line;
		__assign_str(msg_func, msg_fn);
	),
	TP_printk("(%s:%d:%s) You are trying to use a numerical value for the mask - this will be deprecated in a future release.",
		  __get_str(msg_file), __entry->msg_line, __get_str(msg_func))
);

#define trace_cwarn_unsupported_mask(void)				\
do {									\
	trace_libcfs_warning_unsupported_mask(&libcfs_trace_rs,		\
					      __FILE__,	__LINE__,	\
					      __func__);		\
									\
	CWARN("You are trying to use a numerical value for the mask - this will be deprecated in a future release.\n"); \
} while (0)

TRACE_EVENT_CONDITION(libcfs_warning_watchdog_lcw_dump,
	TP_PROTO(struct ratelimit_state *rs, const char *msg_file,
		 int msg_line, const char *msg_fn, int pid),
	TP_ARGS(rs, msg_file, msg_line, msg_fn, pid),
	TP_CONDITION(__ratelimit(rs)),
	TP_STRUCT__entry(
		__string(msg_file, msg_file ? msg_file : "<nofile>")
		__field(int, msg_line)
		__string(msg_func, msg_fn ? msg_fn : "<nofunc>")
		__field(int, pid)
	),
	TP_fast_assign(
		__assign_str(msg_file, kbasename(msg_file));
		__entry->msg_line = msg_line;
		__assign_str(msg_func, msg_fn);
		__entry->pid = pid;
	),
	TP_printk("(%s:%d:%s) Process %d was not found in the task list; watchdog callback may be incomplete",
		  __get_str(msg_file), __entry->msg_line, __get_str(msg_func),
		  __entry->pid)
);

#define trace_cwarn_watchdog_lcw_dump(pid)				\
do {									\
	trace_libcfs_warning_watchdog_lcw_dump(&libcfs_trace_rs,	\
					       __FILE__, __LINE__,	\
					       __func__, pid);		\
									\
	LCONSOLE_WARN("Process %d was not found in the task list; watchdog callback may be incomplete\n", \
		      pid);						\
} while (0)

TRACE_EVENT_CONDITION(libcfs_warning_watchdog_hang,
	TP_PROTO(struct ratelimit_state *rs, const char *msg_file,
		 int msg_line, const char *msg_fn, unsigned int pid,
		 struct timespec64 ts),
	TP_ARGS(rs, msg_file, msg_line, msg_fn, pid, ts),
	TP_CONDITION(__ratelimit(rs)),
	TP_STRUCT__entry(
		__string(msg_file, msg_file ? msg_file : "<nofile>")
		__field(int, msg_line)
		__string(msg_func, msg_fn ? msg_fn : "<nofunc>")
		__field(unsigned int, pid)
		__field(unsigned long, sec)
		__field(unsigned long, msec)
	),
	TP_fast_assign(
		__assign_str(msg_file, kbasename(msg_file));
		__entry->msg_line = msg_line;
		__assign_str(msg_func, msg_fn);
		__entry->pid = pid;
		__entry->sec = ts.tv_sec;
		__entry->msec = ts.tv_nsec / (NSEC_PER_SEC * 100);
	),
	TP_printk("(%s:%d:%s) Service thread pid %u was inactive for %lu.%.02lu secs. The thread might be hung, or it might only be slow and will resume later. Dumping the stack trace for debugging purposes",
		  __get_str(msg_file), __entry->msg_line, __get_str(msg_func),
		  __entry->pid, __entry->sec, __entry->msec)
);

#define trace_cwarn_watchdog_hang(pid, ts)				\
do {									\
	trace_libcfs_warning_watchdog_hang(&libcfs_trace_rs, __FILE__,	\
					   __LINE__, __func__, pid, ts);\
									\
	LCONSOLE_WARN("Service thread pid %u was inactive for %lu.%.02lu secs. The thread might be hung, or it might only be slow and will resume later. Dumping the stack trace for debugging purposes\n",	       \
		      pid, ts.tv_sec,					\
		      ts.tv_nsec / (NSEC_PER_SEC * 100));		\
} while (0)

TRACE_EVENT_CONDITION(libcfs_warning_watchdog_overflow,
	TP_PROTO(struct ratelimit_state *rs, const char *msg_file,
		 int msg_line, const char *msg_fn, unsigned int pid,
		 struct timespec64 ts, int limit),
	TP_ARGS(rs, msg_file, msg_line, msg_fn, pid, ts, limit),
	TP_CONDITION(__ratelimit(rs)),
	TP_STRUCT__entry(
		__string(msg_file, msg_file ? msg_file : "<nofile>")
		__field(int, msg_line)
		__string(msg_func, msg_fn ? msg_fn : "<nofunc>")
		__field(unsigned int, pid)
		__field(unsigned long, sec)
		__field(unsigned long, msec)
		__field(int, limit)
	),
	TP_fast_assign(
		__assign_str(msg_file, kbasename(msg_file));
		__entry->msg_line = msg_line;
		__assign_str(msg_func, msg_fn);
		__entry->pid = pid;
		__entry->sec = ts.tv_sec;
		__entry->msec = ts.tv_nsec / (NSEC_PER_SEC * 100);
		__entry->limit = limit;
	),
	TP_printk("(%s:%d:%s) Service thread pid %u was inactive for %lu.%.02lu secs. Watchdog stack traces are limited to 3 per %d seconds, skipping this one.",
		  __get_str(msg_file), __entry->msg_line, __get_str(msg_func),
		  __entry->pid, __entry->sec, __entry->msec, __entry->limit)
);

#define trace_cwarn_watchdog_overflow(pid, ts, limit)			\
do {									\
	trace_libcfs_warning_watchdog_overflow(&libcfs_trace_rs,	\
					       __FILE__, __LINE__,	\
					       __func__, pid, ts,	\
					       limit);			\
									\
	LCONSOLE_WARN("Service thread pid %u was inactive for %lu.%.02lu secs. Watchdog stack traces are limited to 3 per %d seconds, skipping this one.\n", \
		      pid, ts.tv_sec,					\
		      ts.tv_nsec / (NSEC_PER_SEC * 100), limit);	\
} while (0)

TRACE_EVENT_CONDITION(libcfs_error_cpu_allocate_map,
	TP_PROTO(struct ratelimit_state *rs, const char *msg_file,
		 int msg_line, const char *msg_fn, int ncpts),
	TP_ARGS(rs, msg_file, msg_line, msg_fn, ncpts),
	TP_CONDITION(__ratelimit(rs)),
	TP_STRUCT__entry(
		__string(msg_file, msg_file ? msg_file : "<nofile>")
		__field(int, msg_line)
		__string(msg_func, msg_fn ? msg_fn : "<nofunc>")
		__field(int, ncpts)
	),
	TP_fast_assign(
		__assign_str(msg_file, kbasename(msg_file));
		__entry->msg_line = msg_line;
		__assign_str(msg_func, msg_fn);
		__entry->ncpts = ncpts;
	),
	TP_printk("(%s:%d:%s) Failed to allocate CPU map(%d)",
		  __get_str(msg_file), __entry->msg_line, __get_str(msg_func),
		  __entry->ncpts)
);

#define trace_cerror_cpu_allocate_map(ncpts)				\
do {									\
	trace_libcfs_error_cpu_allocate_map(&libcfs_trace_rs, __FILE__,	\
					    __LINE__, __func__, ncpts);	\
									\
	CERROR("Failed to allocate CPU map(%d)\n", ncpts);		\
} while (0)

TRACE_EVENT_CONDITION(libcfs_error_cpu_allocate_scratch,
	TP_PROTO(struct ratelimit_state *rs, const char *msg_file,
		 int msg_line, const char *msg_fn),
	TP_ARGS(rs, msg_file, msg_line, msg_fn),
	TP_CONDITION(__ratelimit(rs)),
	TP_STRUCT__entry(
		__string(msg_file, msg_file ? msg_file : "<nofile>")
		__field(int, msg_line)
		__string(msg_func, msg_fn ? msg_fn : "<nofunc>")
	),
	TP_fast_assign(
		__assign_str(msg_file, kbasename(msg_file));
		__entry->msg_line = msg_line;
		__assign_str(msg_func, msg_fn);
	),
	TP_printk("(%s:%d:%s) Failed to allocate scratch cpumask",
		  __get_str(msg_file), __entry->msg_line, __get_str(msg_func))
);

#define trace_cerror_cpu_allocate_scratch(void)				\
do {									\
	trace_libcfs_error_cpu_allocate_scratch(&libcfs_trace_rs,	\
						__FILE__, __LINE__,	\
						__func__);		\
									\
	CERROR("Failed to allocate scratch cpumask\n");			\
} while (0)

TRACE_EVENT_CONDITION(libcfs_error_cpu_allocate_table,
	TP_PROTO(struct ratelimit_state *rs, const char *msg_file,
		 int msg_line, const char *msg_fn),
	TP_ARGS(rs, msg_file, msg_line, msg_fn),
	TP_CONDITION(__ratelimit(rs)),
	TP_STRUCT__entry(
		__string(msg_file, msg_file ? msg_file : "<nofile>")
		__field(int, msg_line)
		__string(msg_func, msg_fn ? msg_fn : "<nofunc>")
	),
	TP_fast_assign(
		__assign_str(msg_file, kbasename(msg_file));
		__entry->msg_line = msg_line;
		__assign_str(msg_func, msg_fn);
	),
	TP_printk("(%s:%d:%s) Failed to allocate CPU partition table",
		  __get_str(msg_file), __entry->msg_line, __get_str(msg_func))
);

#define trace_cerror_cpu_allocate_table(void)				\
do {									\
	trace_libcfs_error_cpu_allocate_table(&libcfs_trace_rs,	\
					      __FILE__, __LINE__,	\
					      __func__);		\
									\
	CERROR("Failed to allocate CPU partition table\n");		\
} while (0)

TRACE_EVENT_CONDITION(libcfs_error_cpu_bad_ump_setup,
	TP_PROTO(struct ratelimit_state *rs, const char *msg_file,
		 int msg_line, const char *msg_fn, unsigned int ncpts),
	TP_ARGS(rs, msg_file, msg_line, msg_fn, ncpts),
	TP_CONDITION(__ratelimit(rs)),
	TP_STRUCT__entry(
		__string(msg_file, msg_file ? msg_file : "<nofile>")
		__field(int, msg_line)
		__string(msg_func, msg_fn ? msg_fn : "<nofunc>")
		__field(unsigned int, ncpts)
	),
	TP_fast_assign(
		__assign_str(msg_file, kbasename(msg_file));
		__entry->msg_line = msg_line;
		__assign_str(msg_func, msg_fn);
		__entry->ncpts = ncpts;
	),
	TP_printk("(%s:%d:%s) Can't support CPU partition number %d",
		  __get_str(msg_file), __entry->msg_line, __get_str(msg_func),
		  __entry->ncpts)
);

#define trace_cerror_cpu_bad_ump_setup(ncpts)				\
do {									\
	trace_libcfs_error_cpu_bad_ump_setup(&libcfs_trace_rs, __FILE__,\
					     __LINE__, __func__, ncpts);\
									\
	CERROR("Can't support CPU partition number %d\n", ncpts);	\
} while (0)

TRACE_EVENT_CONDITION(libcfs_error_cpu_cpt_out_of_bound,
	TP_PROTO(struct ratelimit_state *rs, const char *msg_file,
		 int msg_line, const char *msg_fn, int cpt,
		 int num_cpts),
	TP_ARGS(rs, msg_file, msg_line, msg_fn, cpt, num_cpts),
	TP_CONDITION(__ratelimit(rs)),
	TP_STRUCT__entry(
		__string(msg_file, msg_file ? msg_file : "<nofile>")
		__field(int, msg_line)
		__string(msg_func, msg_fn ? msg_fn : "<nofunc>")
		__field(int, cpt)
		__field(int, ncpts)
	),
	TP_fast_assign(
		__assign_str(msg_file, kbasename(msg_file));
		__entry->msg_line = msg_line;
		__assign_str(msg_func, msg_fn);
		__entry->cpt = cpt;
		__entry->ncpts = num_cpts;
	),
	TP_printk("(%s:%d:%s) Invalid partition id %d, total partitions %d",
		  __get_str(msg_file), __entry->msg_line, __get_str(msg_func),
		  __entry->cpt, __entry->ncpts)
);

#define trace_cerror_cpus_cpt_out_of_bound(cpt, ncpts)			\
do {									\
	trace_libcfs_error_cpu_cpt_out_of_bound(&libcfs_trace_rs,	\
						__FILE__, __LINE__,	\
						__func__, cpt, ncpts);	\
									\
	CERROR("Invalid partition id %d, total partitions %d\n", cpt,	\
	       ncpts);							\
} while (0)

TRACE_EVENT_CONDITION(libcfs_error_cpu_invalid_pattern,
	TP_PROTO(struct ratelimit_state *rs, const char *msg_file,
		 int msg_line, const char *msg_fn, const char *pattern),
	TP_ARGS(rs, msg_file, msg_line, msg_fn, pattern),
	TP_CONDITION(__ratelimit(rs)),
	TP_STRUCT__entry(
		__string(msg_file, msg_file ? msg_file : "<nofile>")
		__field(int, msg_line)
		__string(msg_func, msg_fn ? msg_fn : "<nofunc>")
		__string(pattern, pattern)
	),
	TP_fast_assign(
		__assign_str(msg_file, kbasename(msg_file));
		__entry->msg_line = msg_line;
		__assign_str(msg_func, msg_fn);
		__assign_str(pattern, pattern);
	),
	TP_printk("(%s:%d:%s) Invalid CPU pattern '%s'",
		  __get_str(msg_file), __entry->msg_line, __get_str(msg_func),
		  __get_str(pattern))
);

#define trace_cerror_cpus_invalid_pattern(pattern)			\
do {									\
	trace_libcfs_error_cpu_invalid_pattern(&libcfs_trace_rs,	\
					       __FILE__, __LINE__,	\
					       __func__, pattern);	\
									\
	CERROR("Invalid CPU pattern '%s'\n", pattern);			\
} while (0)

TRACE_EVENT_CONDITION(libcfs_error_cpu_invalid_range,
	TP_PROTO(struct ratelimit_state *rs, const char *msg_file,
		 int msg_line, const char *msg_fn, const char *str),
	TP_ARGS(rs, msg_file, msg_line, msg_fn, str),
	TP_CONDITION(__ratelimit(rs)),
	TP_STRUCT__entry(
		__string(msg_file, msg_file ? msg_file : "<nofile>")
		__field(int, msg_line)
		__string(msg_func, msg_fn ? msg_fn : "<nofunc>")
		__string(str, str)
	),
	TP_fast_assign(
		__assign_str(msg_file, kbasename(msg_file));
		__entry->msg_line = msg_line;
		__assign_str(msg_func, msg_fn);
		__assign_str(str, str);
	),
	TP_printk("(%s:%d:%s) Can't parse number range in %s",
		  __get_str(msg_file), __entry->msg_line, __get_str(msg_func),
		  __get_str(str))
);

#define trace_cerror_cpus_invalid_range(str)				\
do {									\
	trace_libcfs_error_cpu_invalid_range(&libcfs_trace_rs, __FILE__,\
					     __LINE__, __func__, str);	\
									\
	CERROR("Can't parse number range in '%s'\n", str);		\
} while (0)

TRACE_EVENT_CONDITION(libcfs_error_cpu_empty_partition,
	TP_PROTO(struct ratelimit_state *rs, const char *msg_file,
		 int msg_line, const char *msg_fn, int cpt),
	TP_ARGS(rs, msg_file, msg_line, msg_fn, cpt),
	TP_CONDITION(__ratelimit(rs)),
	TP_STRUCT__entry(
		__string(msg_file, msg_file ? msg_file : "<nofile>")
		__field(int, msg_line)
		__string(msg_func, msg_fn ? msg_fn : "<nofunc>")
		__field(int, cpt)
	),
	TP_fast_assign(
		__assign_str(msg_file, kbasename(msg_file));
		__entry->msg_line = msg_line;
		__assign_str(msg_func, msg_fn);
		__entry->cpt = cpt;
	),
	TP_printk("(%s:%d:%s) No online CPU is found on partition %d",
		  __get_str(msg_file), __entry->msg_line, __get_str(msg_func),
		  __entry->cpt)
);

#define trace_cerror_cpus_empty_partition(cpt)				\
do {									\
	trace_libcfs_error_cpu_empty_partition(&libcfs_trace_rs,	\
					       __FILE__, __LINE__,	\
					       __func__, cpt);		\
									\
	CERROR("No online CPU is found on partition %d\n", cpt);	\
} while (0)

TRACE_EVENT_CONDITION(libcfs_error_cpu_fail_create_cptab,
	TP_PROTO(struct ratelimit_state *rs, const char *msg_file,
		 int msg_line, const char *msg_fn, const char *pattern),
	TP_ARGS(rs, msg_file, msg_line, msg_fn, pattern),
	TP_CONDITION(__ratelimit(rs)),
	TP_STRUCT__entry(
		__string(msg_file, msg_file ? msg_file : "<nofile>")
		__field(int, msg_line)
		__string(msg_func, msg_fn ? msg_fn : "<nofunc>")
		__string(pattern, pattern)
	),
	TP_fast_assign(
		__assign_str(msg_file, kbasename(msg_file));
		__entry->msg_line = msg_line;
		__assign_str(msg_func, msg_fn);
		__assign_str(pattern, pattern);
	),
	TP_printk("(%s:%d:%s) Failed to create cptab from pattern %s",
		  __get_str(msg_file), __entry->msg_line, __get_str(msg_func),
		  __get_str(pattern))
);

#define trace_cerror_cpus_fail_create_cptab(pattern)			\
do {									\
	trace_libcfs_error_cpu_fail_create_cptab(&libcfs_trace_rs,	\
						 __FILE__, __LINE__,	\
						 __func__, pattern);	\
									\
	CERROR("Failed to create cptab from pattern %s\n", pattern);	\
} while (0)

TRACE_EVENT_CONDITION(libcfs_error_cpu_fail_dup_pattern,
	TP_PROTO(struct ratelimit_state *rs, const char *msg_file,
		 int msg_line, const char *msg_fn, const char *pattern),
	TP_ARGS(rs, msg_file, msg_line, msg_fn, pattern),
	TP_CONDITION(__ratelimit(rs)),
	TP_STRUCT__entry(
		__string(msg_file, msg_file ? msg_file : "<nofile>")
		__field(int, msg_line)
		__string(msg_func, msg_fn ? msg_fn : "<nofunc>")
		__string(pattern, pattern)
	),
	TP_fast_assign(
		__assign_str(msg_file, kbasename(msg_file));
		__entry->msg_line = msg_line;
		__assign_str(msg_func, msg_fn);
		__assign_str(pattern, pattern);
	),
	TP_printk("(%s:%d:%s) Failed to duplicate pattern %s",
		  __get_str(msg_file), __entry->msg_line, __get_str(msg_func),
		  __get_str(pattern))
);

#define trace_cerror_cpus_fail_dup_pattern(pattern)			\
do {									\
	trace_libcfs_error_cpu_fail_dup_pattern(&libcfs_trace_rs,	\
						__FILE__, __LINE__,	\
						__func__, pattern);	\
									\
	CERROR("Failed to duplicate pattern '%s'\n", pattern);		\
} while (0)

TRACE_EVENT_CONDITION(libcfs_error_cpu_fail_create_ptable,
	TP_PROTO(struct ratelimit_state *rs, const char *msg_file,
		 int msg_line, const char *msg_fn, int npartition),
	TP_ARGS(rs, msg_file, msg_line, msg_fn, npartition),
	TP_CONDITION(__ratelimit(rs)),
	TP_STRUCT__entry(
		__string(msg_file, msg_file ? msg_file : "<nofile>")
		__field(int, msg_line)
		__string(msg_func, msg_fn ? msg_fn : "<nofunc>")
		__field(int, npartition)
	),
	TP_fast_assign(
		__assign_str(msg_file, kbasename(msg_file));
		__entry->msg_line = msg_line;
		__assign_str(msg_func, msg_fn);
		__entry->npartition = npartition;
	),
	TP_printk("(%s:%d:%s) Failed to create ptable with npartitions %d",
		  __get_str(msg_file), __entry->msg_line, __get_str(msg_func),
		  __entry->npartition)
);

#define trace_cerror_cpus_fail_create_ptable(num_partitions)		\
do {									\
	trace_libcfs_error_cpu_fail_create_ptable(&libcfs_trace_rs,	\
						  __FILE__, __LINE__,	\
						  __func__,		\
						  num_partitions);	\
									\
	CERROR("Failed to create ptable with npartitions %d\n",		\
	       num_partitions);						\
} while (0)

TRACE_EVENT_CONDITION(libcfs_error_cpu_fail_state,
	TP_PROTO(struct ratelimit_state *rs, const char *msg_file,
		 int msg_line, const char *msg_fn, int ncpt,
		 int num_nodes, int num_cores, int rc),
	TP_ARGS(rs, msg_file, msg_line, msg_fn, ncpt, num_nodes,
		num_cores, rc),
	TP_CONDITION(__ratelimit(rs)),
	TP_STRUCT__entry(
		__string(msg_file, msg_file ? msg_file : "<nofile>")
		__field(int, msg_line)
		__string(msg_func, msg_fn ? msg_fn : "<nofunc>")
		__field(int, ncpt)
		__field(int, num_nodes)
		__field(int, num_cores)
		__field(int, rc)
	),
	TP_fast_assign(
		__assign_str(msg_file, kbasename(msg_file));
		__entry->msg_line = msg_line;
		__assign_str(msg_func, msg_fn);
		__entry->ncpt = ncpt;
		__entry->num_nodes = num_nodes;
		__entry->num_cores = num_cores;
		__entry->rc = rc;
	),
	TP_printk("(%s:%d:%s) Failed (rc=%d) to setup CPU-partition-table with %d CPU-partitions, online HW nodes: %d, HW cpus: %d.",
		  __get_str(msg_file), __entry->msg_line, __get_str(msg_func),
		  __entry->rc, __entry->ncpt, __entry->num_nodes,
		  __entry->num_cores)
);

#define trace_cerror_cpus_fail_state(ncpt, num_nodes, num_cores, rc)	\
do {									\
	trace_libcfs_error_cpu_fail_state(&libcfs_trace_rs, __FILE__,	\
					  __LINE__, __func__, ncpt,	\
					  num_nodes, num_cores, rc);	\
									\
	CERROR("Failed (rc=%d) to setup CPU-partition-table with %d CPU-partitions, online HW nodes: %d, HW cpus: %d.\n", \
	       rc, ncpt, num_nodes, num_cores);				\
} while (0)

TRACE_EVENT_CONDITION(libcfs_error_cpu_missing_bracket,
	TP_PROTO(struct ratelimit_state *rs, const char *msg_file,
		 int msg_line, const char *msg_fn, const char *pattern,
		 int cpt),
	TP_ARGS(rs, msg_file, msg_line, msg_fn, pattern, cpt),
	TP_CONDITION(__ratelimit(rs)),
	TP_STRUCT__entry(
		__string(msg_file, msg_file ? msg_file : "<nofile>")
		__field(int, msg_line)
		__string(msg_func, msg_fn ? msg_fn : "<nofunc>")
		__string(pattern, pattern)
		__field(int, cpt)
	),
	TP_fast_assign(
		__assign_str(msg_file, kbasename(msg_file));
		__entry->msg_line = msg_line;
		__assign_str(msg_func, msg_fn);
		__assign_str(pattern, pattern);
		__entry->cpt = cpt;
	),
	TP_printk("(%s:%d:%s) Missing right bracket for cpt %d, %s",
		  __get_str(msg_file), __entry->msg_line, __get_str(msg_func),
		  __entry->cpt, __get_str(pattern))
);

#define trace_cerror_cpus_missing_bracket(cpt, pattern)			\
do {									\
	trace_libcfs_error_cpu_missing_bracket(&libcfs_trace_rs,	\
					       __FILE__, __LINE__,	\
					       __func__, pattern, cpt);	\
									\
	CERROR("Missing right bracket for cpt %d, %s\n", cpt, pattern);	\
} while (0)

TRACE_EVENT_CONDITION(libcfs_error_cpu_missing,
	TP_PROTO(struct ratelimit_state *rs, const char *msg_file,
		 int msg_line, const char *msg_fn, int cpt),
	TP_ARGS(rs, msg_file, msg_line, msg_fn, cpt),
	TP_CONDITION(__ratelimit(rs)),
	TP_STRUCT__entry(
		__string(msg_file, msg_file ? msg_file : "<nofile>")
		__field(int, msg_line)
		__string(msg_func, msg_fn ? msg_fn : "<nofunc>")
		__field(int, cpt)
	),
	TP_fast_assign(
		__assign_str(msg_file, kbasename(msg_file));
		__entry->msg_line = msg_line;
		__assign_str(msg_func, msg_fn);
		__entry->cpt = cpt;
	),
	TP_printk("(%s:%d:%s) No online CPU found in CPU partition %d, did someone do CPU hotplug on system? You might need to reload Lustre modules to keep system working well.",
		  __get_str(msg_file), __entry->msg_line, __get_str(msg_func),
		  __entry->cpt)
);

#define trace_cerror_cpus_missing(cpt)					\
do {									\
	trace_libcfs_error_cpu_missing(&libcfs_trace_rs, __FILE__,	\
				       __LINE__, __func__, cpt);	\
									\
	CERROR("No online CPU found in CPU partition %d, did someone do CPU hotplug on system? You might need to reload Lustre modules to keep system working well.\n", \
	       cpt);							\
} while (0)

TRACE_EVENT_CONDITION(libcfs_error_cpu_partition_set,
	TP_PROTO(struct ratelimit_state *rs, const char *msg_file,
		 int msg_line, const char *msg_fn, int cpt),
	TP_ARGS(rs, msg_file, msg_line, msg_fn, cpt),
	TP_CONDITION(__ratelimit(rs)),
	TP_STRUCT__entry(
		__string(msg_file, msg_file ? msg_file : "<nofile>")
		__field(int, msg_line)
		__string(msg_func, msg_fn ? msg_fn : "<nofunc>")
		__field(int, cpt)
	),
	TP_fast_assign(
		__assign_str(msg_file, kbasename(msg_file));
		__entry->msg_line = msg_line;
		__assign_str(msg_func, msg_fn);
		__entry->cpt = cpt;
	),
	TP_printk("(%s:%d:%s) Partition %d has already been set.",
		  __get_str(msg_file), __entry->msg_line, __get_str(msg_func),
		  __entry->cpt)
);

#define trace_cerror_cpus_partition_set(cpt)				\
do {									\
	trace_libcfs_error_cpu_partition_set(&libcfs_trace_rs, __FILE__,\
					     __LINE__, __func__, cpt);	\
									\
	CERROR("Partition %d has already been set.\n", cpt);		\
} while (0)



TRACE_EVENT_CONDITION(libcfs_error_cpu_wrong_cpt_count,
	TP_PROTO(struct ratelimit_state *rs, const char *msg_file,
		 int msg_line, const char *msg_fn, int ncpt, int count),
	TP_ARGS(rs, msg_file, msg_line, msg_fn, ncpt, count),
	TP_CONDITION(__ratelimit(rs)),
	TP_STRUCT__entry(
		__string(msg_file, msg_file ? msg_file : "<nofile>")
		__field(int, msg_line)
		__string(msg_func, msg_fn ? msg_fn : "<nofunc>")
		__field(int, ncpt)
		__field(int, count)
	),
	TP_fast_assign(
		__assign_str(msg_file, kbasename(msg_file));
		__entry->msg_line = msg_line;
		__assign_str(msg_func, msg_fn);
		__entry->ncpt = ncpt;
		__entry->count = count;
	),
	TP_printk("(%s:%d:%s) Expect %d partitions but found %d",
		  __get_str(msg_file), __entry->msg_line, __get_str(msg_func),
		  __entry->ncpt, __entry->count)
);

#define trace_cerror_cpus_wrong_cpt_count(ncpt, count)			\
do {									\
	trace_libcfs_error_cpu_wrong_cpt_count(&libcfs_trace_rs,	\
					       __FILE__, __LINE__,	\
					       __func__, ncpt, count);	\
									\
	CERROR("Expect %d partitions but found %d\n", ncpt, count);	\
} while (0)

TRACE_EVENT_CONDITION(libcfs_error_startup_crypto,
	TP_PROTO(struct ratelimit_state *rs, const char *msg_file,
		 int msg_line, const char *msg_fn, int rc),
	TP_ARGS(rs, msg_file, msg_line, msg_fn, rc),
	TP_CONDITION(__ratelimit(rs)),
	TP_STRUCT__entry(
		__string(msg_file, msg_file ? msg_file : "<nofile>")
		__field(int, msg_line)
		__string(msg_func, msg_fn ? msg_fn : "<nofunc>")
		__field(int, rc)
	),
	TP_fast_assign(
		__assign_str(msg_file, kbasename(msg_file));
		__entry->msg_line = msg_line;
		__assign_str(msg_func, msg_fn);
		__entry->rc = rc;
	),
	TP_printk("(%s:%d:%s) cfs_crypto_regster: error %d",
		  __get_str(msg_file), __entry->msg_line, __get_str(msg_func),
		  __entry->rc)
);

#define trace_cerror_startup_crypto(rc)					\
do {									\
	trace_libcfs_error_startup_crypto(&libcfs_trace_rs, __FILE__,	\
					  __LINE__, __func__, rc);	\
									\
	CERROR("cfs_crypto_regster: error %d\n", rc);			\
} while (0)

TRACE_EVENT_CONDITION(libcfs_error_startup_procfs,
	TP_PROTO(struct ratelimit_state *rs, const char *msg_file,
		 int msg_line, const char *msg_fn, int rc),
	TP_ARGS(rs, msg_file, msg_line, msg_fn, rc),
	TP_CONDITION(__ratelimit(rs)),
	TP_STRUCT__entry(
		__string(msg_file, msg_file ? msg_file : "<nofile>")
		__field(int, msg_line)
		__string(msg_func, msg_fn ? msg_fn : "<nofunc>")
		__field(int, rc)
	),
	TP_fast_assign(
		__assign_str(msg_file, kbasename(msg_file));
		__entry->msg_line = msg_line;
		__assign_str(msg_func, msg_fn);
		__entry->rc = rc;
	),
	TP_printk("(%s:%d:%s) insert_proc: error %d",
		  __get_str(msg_file), __entry->msg_line, __get_str(msg_func),
		  __entry->rc)
);

#define trace_cerror_startup_procfs(rc)					\
do {									\
	trace_libcfs_error_startup_procfs(&libcfs_trace_rs, __FILE__,	\
					  __LINE__, __func__, rc);	\
									\
	CERROR("insert_proc: error %d\n", rc);				\
} while (0)

TRACE_EVENT_CONDITION(libcfs_error_startup_misc_dev,
	TP_PROTO(struct ratelimit_state *rs, const char *msg_file,
		 int msg_line, const char *msg_fn, int rc),
	TP_ARGS(rs, msg_file, msg_line, msg_fn, rc),
	TP_CONDITION(__ratelimit(rs)),
	TP_STRUCT__entry(
		__string(msg_file, msg_file ? msg_file : "<nofile>")
		__field(int, msg_line)
		__string(msg_func, msg_fn ? msg_fn : "<nofunc>")
		__field(int, rc)
	),
	TP_fast_assign(
		__assign_str(msg_file, kbasename(msg_file));
		__entry->msg_line = msg_line;
		__assign_str(msg_func, msg_fn);
		__entry->rc = rc;
	),
	TP_printk("(%s:%d:%s) misc_register: error %d",
		  __get_str(msg_file), __entry->msg_line, __get_str(msg_func),
		  __entry->rc)
);

#define trace_cerror_startup_misc_dev(rc)				\
do {									\
	trace_libcfs_error_startup_misc_dev(&libcfs_trace_rs, __FILE__,	\
					    __LINE__, __func__, rc);	\
									\
	CERROR("misc_register: error %d\n", rc);			\
} while (0)

TRACE_EVENT_CONDITION(libcfs_error_startup_wi,
	TP_PROTO(struct ratelimit_state *rs, const char *msg_file,
		 int msg_line, const char *msg_fn, int rc),
	TP_ARGS(rs, msg_file, msg_line, msg_fn, rc),
	TP_CONDITION(__ratelimit(rs)),
	TP_STRUCT__entry(
		__string(msg_file, msg_file ? msg_file : "<nofile>")
		__field(int, msg_line)
		__string(msg_func, msg_fn ? msg_fn : "<nofunc>")
		__field(int, rc)
	),
	TP_fast_assign(
		__assign_str(msg_file, kbasename(msg_file));
		__entry->msg_line = msg_line;
		__assign_str(msg_func, msg_fn);
		__entry->rc = rc;
	),
	TP_printk("(%s:%d:%s) initialize workitem: error %d",
		  __get_str(msg_file), __entry->msg_line, __get_str(msg_func),
		  __entry->rc)
);

#define trace_cerror_startup_wi(rc)					\
do {									\
	trace_libcfs_error_startup_wi(&libcfs_trace_rs, __FILE__,	\
				      __LINE__, __func__, rc);		\
									\
	CERROR("initialize workitem: error %d\n", rc);			\
} while (0)

TRACE_EVENT_CONDITION(libcfs_error_startup_wi_sched,
	TP_PROTO(struct ratelimit_state *rs, const char *msg_file,
		 int msg_line, const char *msg_fn, int rc),
	TP_ARGS(rs, msg_file, msg_line, msg_fn, rc),
	TP_CONDITION(__ratelimit(rs)),
	TP_STRUCT__entry(
		__string(msg_file, msg_file ? msg_file : "<nofile>")
		__field(int, msg_line)
		__string(msg_func, msg_fn ? msg_fn : "<nofunc>")
		__field(int, rc)
	),
	TP_fast_assign(
		__assign_str(msg_file, kbasename(msg_file));
		__entry->msg_line = msg_line;
		__assign_str(msg_func, msg_fn);
		__entry->rc = rc;
	),
	TP_printk("(%s:%d:%s) Startup workitem scheduler: error: %d",
		  __get_str(msg_file), __entry->msg_line, __get_str(msg_func),
		  __entry->rc)
);

#define trace_cerror_startup_wi_sched(rc)				\
do {									\
	trace_libcfs_error_startup_wi_sched(&libcfs_trace_rs, __FILE__,	\
					    __LINE__, __func__, rc);	\
									\
	CERROR("Startup workitem scheduler: error: %d\n", rc);		\
} while (0)

TRACE_EVENT_CONDITION(libcfs_error_watchdog_thread_shutdown,
	TP_PROTO(struct ratelimit_state *rs, const char *msg_file,
		 int msg_line, const char *msg_fn, int rc),
	TP_ARGS(rs, msg_file, msg_line, msg_fn, rc),
	TP_CONDITION(__ratelimit(rs) && rc),
	TP_STRUCT__entry(
		__string(msg_file, msg_file ? msg_file : "<nofile>")
		__field(int, msg_line)
		__string(msg_func, msg_fn ? msg_fn : "<nofunc>")
	),
	TP_fast_assign(
		__assign_str(msg_file, kbasename(msg_file));
		__entry->msg_line = msg_line;
		__assign_str(msg_func, msg_fn);
	),
	TP_printk("(%s:%d:%s) pending timers list was not empty at time of watchdog dispatch shutdown",
		  __get_str(msg_file), __entry->msg_line, __get_str(msg_func))
);

#define trace_cerror_watchdog_thread_shutdown(rc)			\
do {									\
	trace_libcfs_error_watchdog_thread_shutdown(&libcfs_trace_rs,	\
						    __FILE__, __LINE__,	\
						    __func__, rc);	\
									\
	if (rc)								\
		CERROR("pending timers list was not empty at time of watchdog dispatch shutdown\n"); \
} while (0)

TRACE_EVENT_CONDITION(libcfs_error_watchdog_thread_startup,
	TP_PROTO(struct ratelimit_state *rs, const char *msg_file,
		 int msg_line, const char *msg_fn, long rc),
	TP_ARGS(rs, msg_file, msg_line, msg_fn, rc),
	TP_CONDITION(__ratelimit(rs)),
	TP_STRUCT__entry(
		__string(msg_file, msg_file ? msg_file : "<nofile>")
		__field(int, msg_line)
		__string(msg_func, msg_fn ? msg_fn : "<nofunc>")
		__field(long, rc)
	),
	TP_fast_assign(
		__assign_str(msg_file, kbasename(msg_file));
		__entry->msg_line = msg_line;
		__assign_str(msg_func, msg_fn);
		__entry->rc = rc;
	),
	TP_printk("(%s:%d:%s) error spawning watchdog dispatch thread: %ld",
		  __get_str(msg_file), __entry->msg_line, __get_str(msg_func),
		  __entry->rc)
);

#define trace_cerror_watchdog_thread_startup(rc)			\
do {									\
	trace_libcfs_error_watchdog_thread_startup(&libcfs_trace_rs,	\
						   __FILE__, __LINE__,	\
						   __func__, rc);	\
									\
	CERROR("error spawning watchdog dispatch thread: %ld\n", rc);	\
} while (0)

TRACE_EVENT_CONDITION(libcfs_error_wi_thread_start,
	TP_PROTO(struct ratelimit_state *rs, const char *msg_file,
		 int msg_line, const char *msg_fn, const char *name,
		 int rc),
	TP_ARGS(rs, msg_file, msg_line, msg_fn, name, rc),
	TP_CONDITION(__ratelimit(rs)),
	TP_STRUCT__entry(
		__string(msg_file, msg_file ? msg_file : "<nofile>")
		__field(int, msg_line)
		__string(msg_func, msg_fn ? msg_fn : "<nofunc>")
		__string(thread_name, name)
		__field(int, rc)
	),
	TP_fast_assign(
		__assign_str(msg_file, kbasename(msg_file));
		__entry->msg_line = msg_line;
		__assign_str(msg_func, msg_fn);
		__assign_str(thread_name, name);
		__entry->rc = rc;
	),
	TP_printk("(%s:%d:%s) Failed to create thread for WI scheduler %s: %d",
		  __get_str(msg_file), __entry->msg_line, __get_str(msg_func),
		  __get_str(thread_name), __entry->rc)
);

#define trace_cerror_wi_thread_start(name, rc)				\
do {									\
	trace_libcfs_error_wi_thread_start(&libcfs_trace_rs, __FILE__,	\
					   __LINE__, __func__, name,	\
					   rc);				\
									\
	CERROR("Failed to create thread for WI scheduler %s: %d\n",	\
	       name, rc);						\
} while (0)

TRACE_EVENT_CONDITION(libcfs_error_timeout_awake,
	TP_PROTO(struct ratelimit_state *rs, const char *msg_file,
		 int msg_line, const char *msg_fn, u32 id),
	TP_ARGS(rs, msg_file, msg_line, msg_fn, id),
	TP_CONDITION(__ratelimit(rs)),
	TP_STRUCT__entry(
		__string(msg_file, msg_file ? msg_file : "<nofile>")
		__field(int, msg_line)
		__string(msg_func, msg_fn ? msg_fn : "<nofunc>")
		__field(u32, id)
	),
	TP_fast_assign(
		__assign_str(msg_file, kbasename(msg_file));
		__entry->msg_line = msg_line;
		__assign_str(msg_func, msg_fn);
		__entry->id = id;
	),
	TP_printk("(%s:%d:%s) cfs_fail_timeout id %x awake",
		  __get_str(msg_file), __entry->msg_line, __get_str(msg_func),
		  __entry->id)
);

#define trace_cerror_timeout_awake(id)					\
do {									\
	trace_libcfs_error_timeout_awake(&libcfs_trace_rs, __FILE__,	\
					 __LINE__, __func__, id);	\
									\
	CERROR("cfs_fail_timeout id %x awake\n", id);			\
} while (0)

TRACE_EVENT_CONDITION(libcfs_error_timeout_sleep,
	TP_PROTO(struct ratelimit_state *rs, const char *msg_file,
		 int msg_line, const char *msg_fn, u32 id, int ms),
	TP_ARGS(rs, msg_file, msg_line, msg_fn, id, ms),
	TP_CONDITION(__ratelimit(rs)),
	TP_STRUCT__entry(
		__string(msg_file, msg_file ? msg_file : "<nofile>")
		__field(int, msg_line)
		__string(msg_func, msg_fn ? msg_fn : "<nofunc>")
		__field(u32, id)
		__field(int, ms)
	),
	TP_fast_assign(
		__assign_str(msg_file, kbasename(msg_file));
		__entry->msg_line = msg_line;
		__assign_str(msg_func, msg_fn);
		__entry->id = id;
		__entry->ms = ms;
	),
	TP_printk("(%s:%d:%s) cfs_fail_timeout id %x sleeping for %d ms",
		  __get_str(msg_file), __entry->msg_line, __get_str(msg_func),
		  __entry->id, __entry->ms)
);

#define trace_cerror_timeout_sleep(id, ms)				\
do {									\
	trace_libcfs_error_timeout_sleep(&libcfs_trace_rs, __FILE__,	\
					 __LINE__, __func__, id, ms);	\
									\
	CERROR("cfs_fail_timeout id %x sleeping for %d ms\n", id, ms);	\
} while (0)

TRACE_EVENT_CONDITION(libcfs_error_fail_race_awake,
	TP_PROTO(struct ratelimit_state *rs, const char *msg_file,
		 int msg_line, const char *msg_fn, int id, int rc),
	TP_ARGS(rs, msg_file, msg_line, msg_fn, id, rc),
	TP_CONDITION(__ratelimit(rs)),
	TP_STRUCT__entry(
		__string(msg_file, msg_file ? msg_file : "<nofile>")
		__field(int, msg_line)
		__string(msg_func, msg_fn ? msg_fn : "<nofunc>")
		__field(int, id)
		__field(int, rc)
	),
	TP_fast_assign(
		__assign_str(msg_file, kbasename(msg_file));
		__entry->msg_line = msg_line;
		__assign_str(msg_func, msg_fn);
		__entry->id = id;
		__entry->rc = rc;
	),
	TP_printk("(%s:%d:%s) cfs_fail_race id %x awake, rc=%d",
		  __get_str(msg_file), __entry->msg_line, __get_str(msg_func),
		  __entry->id, __entry->rc)
);

#define trace_cerror_fail_race_awake(id, rc)				\
do {									\
	trace_libcfs_error_fail_race_awake(&libcfs_trace_rs, __FILE__,	\
					   __LINE__, __func__, id, rc);	\
									\
	CERROR("cfs_fail_race id %x awake, rc=%d\n", id, rc);		\
} while (0)

TRACE_EVENT_CONDITION(libcfs_error_fail_race_awaking,
	TP_PROTO(struct ratelimit_state *rs, const char *msg_file,
		 int msg_line, const char *msg_fn, int id),
	TP_ARGS(rs, msg_file, msg_line, msg_fn, id),
	TP_CONDITION(__ratelimit(rs)),
	TP_STRUCT__entry(
		__string(msg_file, msg_file ? msg_file : "<nofile>")
		__field(int, msg_line)
		__string(msg_func, msg_fn ? msg_fn : "<nofunc>")
		__field(int, id)
	),
	TP_fast_assign(
		__assign_str(msg_file, kbasename(msg_file));
		__entry->msg_line = msg_line;
		__assign_str(msg_func, msg_fn);
		__entry->id = id;
	),
	TP_printk("(%s:%d:%s) cfs_fail_race id %x awaking",
		  __get_str(msg_file), __entry->msg_line, __get_str(msg_func),
		  __entry->id)
);

#define trace_cerror_fail_race_awaking(id)				\
do {									\
	trace_libcfs_error_fail_race_awaking(&libcfs_trace_rs, __FILE__,\
					     __LINE__, __func__, id);	\
									\
	CERROR("cfs_fail_race id %x awaking\n", id);			\
} while (0)

TRACE_EVENT_CONDITION(libcfs_error_fail_race_sleeping,
	TP_PROTO(struct ratelimit_state *rs, const char *msg_file,
		 int msg_line, const char *msg_fn, int id),
	TP_ARGS(rs, msg_file, msg_line, msg_fn, id),
	TP_CONDITION(__ratelimit(rs)),
	TP_STRUCT__entry(
		__string(msg_file, msg_file ? msg_file : "<nofile>")
		__field(int, msg_line)
		__string(msg_func, msg_fn ? msg_fn : "<nofunc>")
		__field(int, id)
	),
	TP_fast_assign(
		__assign_str(msg_file, kbasename(msg_file));
		__entry->msg_line = msg_line;
		__assign_str(msg_func, msg_fn);
		__entry->id = id;
	),
	TP_printk("(%s:%d:%s) cfs_race id %x sleeping",
		  __get_str(msg_file), __entry->msg_line, __get_str(msg_func),
		  __entry->id)
);

#define trace_cerror_fail_race_sleeping(id)				\
do {									\
	trace_libcfs_error_fail_race_sleeping(&libcfs_trace_rs,		\
					      __FILE__,	__LINE__,	\
					      __func__, id);		\
									\
	CERROR("cfs_race id %x sleeping\n", id);			\
} while (0)

TRACE_EVENT_CONDITION(libcfs_error_ioctl_buffer_to_big,
	TP_PROTO(struct ratelimit_state *rs, const char *msg_file,
		 int msg_line, const char *msg_fn, int ioc_len,
		 int max_len),
	TP_ARGS(rs, msg_file, msg_line, msg_fn, ioc_len, max_len),
	TP_CONDITION(__ratelimit(rs)),
	TP_STRUCT__entry(
		__string(msg_file, msg_file ? msg_file : "<nofile>")
		__field(int, msg_line)
		__string(msg_func, msg_fn ? msg_fn : "<nofunc>")
		__field(int, len)
		__field(int, max_len)
	),
	TP_fast_assign(
		__assign_str(msg_file, kbasename(msg_file));
		__entry->msg_line = msg_line;
		__assign_str(msg_func, msg_fn);
		__entry->len = ioc_len;
		__entry->max_len = max_len;
	),
	TP_printk("(%s:%d:%s) libcfs ioctl: user buffer is too large %d/%d",
		  __get_str(msg_file), __entry->msg_line, __get_str(msg_func),
		  __entry->len, __entry->max_len)
);

#define trace_cerror_ioctl_buffer_to_big(len, max)			\
do {									\
	trace_libcfs_error_ioctl_buffer_to_big(&libcfs_trace_rs,	\
					       __FILE__, __LINE__,	\
					       __func__, len, max);	\
									\
	CERROR("libcfs ioctl: user buffer is too large %d/%d\n", len,	\
	       max);							\
} while (0)

TRACE_EVENT_CONDITION(libcfs_error_ioctl_buffer_to_small,
	TP_PROTO(struct ratelimit_state *rs, const char *msg_file,
		 int msg_line, const char *msg_fn),
	TP_ARGS(rs, msg_file, msg_line, msg_fn),
	TP_CONDITION(__ratelimit(rs)),
	TP_STRUCT__entry(
		__string(msg_file, msg_file ? msg_file : "<nofile>")
		__field(int, msg_line)
		__string(msg_func, msg_fn ? msg_fn : "<nofunc>")
	),
	TP_fast_assign(
		__assign_str(msg_file, kbasename(msg_file));
		__entry->msg_line = msg_line;
		__assign_str(msg_func, msg_fn);
	),
	TP_printk("(%s:%d:%s) libcfs ioctl: user buffer is too small for ioctl",
		  __get_str(msg_file), __entry->msg_line, __get_str(msg_func))
);

#define trace_cerror_ioctl_buffer_to_small(void)			\
do {									\
	trace_libcfs_error_ioctl_buffer_to_small(&libcfs_trace_rs,	\
						 __FILE__, __LINE__,	\
						 __func__);		\
									\
	CERROR("libcfs ioctl: user buffer is too small for ioctl\n");	\
} while (0)

TRACE_EVENT_CONDITION(libcfs_error_ioctl_invalid_header,
	TP_PROTO(struct ratelimit_state *rs, const char *msg_file,
		 int msg_line, const char *msg_fn, int rc),
	TP_ARGS(rs, msg_file, msg_line, msg_fn, rc),
	TP_CONDITION(__ratelimit(rs)),
	TP_STRUCT__entry(
		__string(msg_file, msg_file ? msg_file : "<nofile>")
		__field(int, msg_line)
		__string(msg_func, msg_fn ? msg_fn : "<nofunc>")
		__field(int, rc)
	),
	TP_fast_assign(
		__assign_str(msg_file, kbasename(msg_file));
		__entry->msg_line = msg_line;
		__assign_str(msg_func, msg_fn);
		__entry->rc = rc;
	),
	TP_printk("(%s:%d:%s) libcfs ioctl: data header error %d",
		  __get_str(msg_file), __entry->msg_line, __get_str(msg_func),
		  __entry->rc)
);

#define trace_cerror_ioctl_invalid_header(rc)				\
do {									\
	trace_libcfs_error_ioctl_invalid_header(&libcfs_trace_rs,	\
						__FILE__, __LINE__,	\
						__func__, rc);		\
									\
	CERROR("libcfs ioctl: data header error %d\n", rc);		\
} while (0)

TRACE_EVENT_CONDITION(libcfs_error_ioctl_invalid_parameter,
	TP_PROTO(struct ratelimit_state *rs, const char *msg_file,
		 int msg_line, const char *msg_fn),
	TP_ARGS(rs, msg_file, msg_line, msg_fn),
	TP_CONDITION(__ratelimit(rs)),
	TP_STRUCT__entry(
		__string(msg_file, msg_file ? msg_file : "<nofile>")
		__field(int, msg_line)
		__string(msg_func, msg_fn ? msg_fn : "<nofunc>")
	),
	TP_fast_assign(
		__assign_str(msg_file, kbasename(msg_file));
		__entry->msg_line = msg_line;
		__assign_str(msg_func, msg_fn);
	),
	TP_printk("(%s:%d:%s) libcfs ioctl: parameter not correctly formatted",
		  __get_str(msg_file), __entry->msg_line, __get_str(msg_func))
);

#define trace_cerror_ioctl_invalid_parameter(void)			\
do {									\
	trace_libcfs_error_ioctl_invalid_parameter(&libcfs_trace_rs,	\
						   __FILE__, __LINE__,	\
						   __func__);		\
									\
	CERROR("libcfs ioctl: parameter not correctly formatted\n");	\
} while (0)

TRACE_EVENT_CONDITION(libcfs_error_ioctl_invalid_version,
	TP_PROTO(struct ratelimit_state *rs, const char *msg_file,
		 int msg_line, const char *msg_fn, int expected,
		 int version),
	TP_ARGS(rs, msg_file, msg_line, msg_fn, expected, version),
	TP_CONDITION(__ratelimit(rs)),
	TP_STRUCT__entry(
		__string(msg_file, msg_file ? msg_file : "<nofile>")
		__field(int, msg_line)
		__string(msg_func, msg_fn ? msg_fn : "<nofunc>")
		__field(int, expected)
		__field(int, version)
	),
	TP_fast_assign(
		__assign_str(msg_file, kbasename(msg_file));
		__entry->msg_line = msg_line;
		__assign_str(msg_func, msg_fn);
		__entry->expected = expected;
		__entry->version = version;
	),
	TP_printk("(%s:%d:%s) libcfs ioctl: version mismatch expected %#x, got %#x",
		  __get_str(msg_file), __entry->msg_line, __get_str(msg_func),
		  __entry->expected, __entry->version)
);

#define trace_cerror_ioctl_invalid_version(expected, version)		\
do {									\
	trace_libcfs_error_ioctl_invalid_version(&libcfs_trace_rs,	\
						 __FILE__, __LINE__,	\
						 __func__, expected,	\
						 version);		\
									\
	CERROR("libcfs ioctl: version mismatch expected %#x, got %#x\n",\
	       expected, version);					\
} while (0)

TRACE_EVENT_CONDITION(libcfs_error_list_excess,
	TP_PROTO(struct ratelimit_state *rs, const char *msg_file,
		 int msg_line, const char *msg_fn, int max, int count),
	TP_ARGS(rs, msg_file, msg_line, msg_fn, max, count),
	TP_CONDITION(__ratelimit(rs)),
	TP_STRUCT__entry(
		__string(msg_file, msg_file ? msg_file : "<nofile>")
		__field(int, msg_line)
		__string(msg_func, msg_fn ? msg_fn : "<nofunc>")
		__field(int, max)
		__field(int, count)
	),
	TP_fast_assign(
		__assign_str(msg_file, kbasename(msg_file));
		__entry->msg_line = msg_line;
		__assign_str(msg_func, msg_fn);
		__entry->max = max;
		__entry->count = count;
	),
	TP_printk("(%s:%d:%s) Number of values %d exceeds max allowed %d",
		  __get_str(msg_file), __entry->msg_line, __get_str(msg_func),
		  __entry->max, __entry->count)
);

#define trace_cerror_list_excess(max, count)				\
do {									\
	trace_libcfs_error_list_excess(&libcfs_trace_rs, __FILE__,	\
				       __LINE__, __func__, max, count);	\
									\
	CERROR("Number of values %d exceeds max allowed %d\n", max,	\
	       count);							\
} while (0)

TRACE_EVENT_CONDITION(libcfs_error_log_upcall,
	TP_PROTO(struct ratelimit_state *rs, const char *msg_file,
		 int msg_line, const char *msg_fn, int rc,
		 const char *argv0, const char *argv1),
	TP_ARGS(rs, msg_file, msg_line, msg_fn, rc, argv0, argv1),
	TP_CONDITION(__ratelimit(rs)),
	TP_STRUCT__entry(
		__string(msg_file, msg_file ? msg_file : "<nofile>")
		__field(int, msg_line)
		__string(msg_func, msg_fn ? msg_fn : "<nofunc>")
		__field(int, rc)
		__string(argv0, argv0)
		__string(argv1, argv1)
	),
	TP_fast_assign(
		__assign_str(msg_file, kbasename(msg_file));
		__entry->msg_line = msg_line;
		__assign_str(msg_func, msg_fn);
		__entry->rc = rc;
		__assign_str(argv0, argv0);
		__assign_str(argv1, argv1);
	),
	TP_printk("(%s:%d:%s) Error %d invoking LNET debug log upcall %s %s; check /sys/kernel/debug/lnet/debug_log_upcall",
		  __get_str(msg_file), __entry->msg_line, __get_str(msg_func),
		  __entry->rc, __get_str(argv0), __get_str(argv1))
);

#define trace_cerror_log_upcall(rc, argv0, argv1)			\
do {									\
	trace_libcfs_error_log_upcall(&libcfs_trace_rs, __FILE__,	\
				      __LINE__, __func__, rc, argv0,	\
				      argv1);				\
									\
	CERROR("Error %d invoking LNET debug log upcall %s %s; check /sys/kernel/debug/lnet/debug_log_upcall\n",\
	       rc, argv0, argv1);					\
} while (0)

TRACE_EVENT(libcfs_other_setup,
	TP_PROTO(const char *msg_file, int msg_line, const char *msg_fn),
	TP_ARGS(msg_file, msg_line, msg_fn),
	TP_STRUCT__entry(
		__string(msg_file, msg_file ? msg_file : "<nofile>")
		__field(int, msg_line)
		__string(msg_func, msg_fn ? msg_fn : "<nofunc>")
	),
	TP_fast_assign(
		__assign_str(msg_file, kbasename(msg_file));
		__entry->msg_line = msg_line;
		__assign_str(msg_func, msg_fn);
	),
	TP_printk("(%s:%d:%s) libcfs setup OK",
		  __get_str(msg_file), __entry->msg_line, __get_str(msg_func))
);

#define trace_other(void)						\
do {									\
	trace_libcfs_other_setup(__FILE__, __LINE__, __func__);		\
									\
	CDEBUG(D_OTHER, "libcfs setup OK\n");				\
} while (0)

TRACE_EVENT(libcfs_ha_debug_upcall,
	TP_PROTO(const char *msg_file, int msg_line, const char *msg_fn,
		 const char *argv1, const char *argv2),
	TP_ARGS(msg_file, msg_line, msg_fn, argv1, argv2),
	TP_STRUCT__entry(
		__string(msg_file, msg_file ? msg_file : "<nofile>")
		__field(int, msg_line)
		__string(msg_func, msg_fn ? msg_fn : "<nofunc>")
		__string(argv1, argv1)
		__string(argv2, argv1)
	),
	TP_fast_assign(
		__assign_str(msg_file, kbasename(msg_file));
		__entry->msg_line = msg_line;
		__assign_str(msg_func, msg_fn);
		__assign_str(argv1, argv1);
		__assign_str(argv2, argv2);
	),
	TP_printk("(%s:%d:%s) Invoked LNET debug log upcall %s %s",
		  __get_str(msg_file), __entry->msg_line, __get_str(msg_func),
		  __get_str(argv1), __get_str(argv2))
);

#define trace_ha(argv1, argv2)						\
do {									\
	trace_libcfs_ha_debug_upcall(__FILE__, __LINE__, __func__,	\
				     argv1, argv2);			\
									\
	CDEBUG(D_HA, "Invoked LNET debug log upcall %s %s\n", argv[0],	\
	       argv[1]);						\
} while (0)

TRACE_EVENT(libcfs_info_cpu_all_offline,
	TP_PROTO(const char *msg_file, int msg_line, const char *msg_fn,
		 int cpt),
	TP_ARGS(msg_file, msg_line, msg_fn, cpt),
	TP_STRUCT__entry(
		__string(msg_file, msg_file ? msg_file : "<nofile>")
		__field(int, msg_line)
		__string(msg_func, msg_fn ? msg_fn : "<nofunc>")
		__field(int, cpt)
	),
	TP_fast_assign(
		__assign_str(msg_file, kbasename(msg_file));
		__entry->msg_line = msg_line;
		__assign_str(msg_func, msg_fn);
		__entry->cpt = cpt;
	),
	TP_printk("(%s:%d:%s) No online CPU is found in the CPU mask for CPU partition %d",
		  __get_str(msg_file), __entry->msg_line, __get_str(msg_func),
		  __entry->cpt)
);

#define trace_info_cpu_all_offline(cpt)					\
do {									\
	trace_libcfs_info_cpu_all_offline(__FILE__, __LINE__, __func__,	\
					  cpt);				\
									\
	CDEBUG(D_INFO, "No online CPU is found in the CPU mask for CPU partition %d\n",\
	       cpt);							\
} while (0)

TRACE_EVENT(libcfs_info_cpu_already_in_cpumask,
	TP_PROTO(const char *msg_file, int msg_line, const char *msg_fn,
		 int cpu),
	TP_ARGS(msg_file, msg_line, msg_fn, cpu),
	TP_STRUCT__entry(
		__string(msg_file, msg_file ? msg_file : "<nofile>")
		__field(int, msg_line)
		__string(msg_func, msg_fn ? msg_fn : "<nofunc>")
		__field(int, cpu)
	),
	TP_fast_assign(
		__assign_str(msg_file, kbasename(msg_file));
		__entry->msg_line = msg_line;
		__assign_str(msg_func, msg_fn);
		__entry->cpu = cpu;
	),
	TP_printk("(%s:%d:%s) CPU %d is already in cpumask",
		  __get_str(msg_file), __entry->msg_line, __get_str(msg_func),
		  __entry->cpu)
);

#define trace_info_cpu_already_in_cpumask(cpu)				\
do {									\
	trace_libcfs_info_cpu_already_in_cpumask(__FILE__, __LINE__,	\
						  __func__, cpu);	\
									\
	CDEBUG(D_INFO, "CPU %d is already in cpumask\n", cpu);		\
} while (0)

TRACE_EVENT(libcfs_info_cpu_changed,
	TP_PROTO(const char *msg_file, int msg_line, const char *msg_fn,
		 int cpu, unsigned long action),
	TP_ARGS(msg_file, msg_line, msg_fn, cpu, action),
	TP_STRUCT__entry(
		__string(msg_file, msg_file ? msg_file : "<nofile>")
		__field(int, msg_line)
		__string(msg_func, msg_fn ? msg_fn : "<nofunc>")
		__field(int, cpu)
		__field(unsigned long, action)
	),
	TP_fast_assign(
		__assign_str(msg_file, kbasename(msg_file));
		__entry->msg_line = msg_line;
		__assign_str(msg_func, msg_fn);
		__entry->cpu = cpu;
		__entry->action = action;
	),
	TP_printk("(%s:%d:%s) CPU changed [cpu %u action %lx]",
		  __get_str(msg_file), __entry->msg_line, __get_str(msg_func),
		  __entry->cpu, __entry->action)
);

#define trace_info_cpu_changed(cpu, action)				\
do {									\
	trace_libcfs_info_cpu_changed(__FILE__, __LINE__, __func__, cpu,\
				      action);				\
									\
	CDEBUG(D_INFO, "CPU changed [cpu %u action %lx]\n", cpu,	\
	       action);							\
} while (0)

TRACE_EVENT(libcfs_info_cpu_extra_mapping,
	TP_PROTO(const char *msg_file, int msg_line, const char *msg_fn,
		 int cpu_id, int cpt),
	TP_ARGS(msg_file, msg_line, msg_fn, cpu_id, cpt),
	TP_STRUCT__entry(
		__string(msg_file, msg_file ? msg_file : "<nofile>")
		__field(int, msg_line)
		__string(msg_func, msg_fn ? msg_fn : "<nofunc>")
		__field(int, cpu_id)
		__field(int, cpt)
	),
	TP_fast_assign(
		__assign_str(msg_file, kbasename(msg_file));
		__entry->msg_line = msg_line;
		__assign_str(msg_func, msg_fn);
		__entry->cpu_id = cpu_id;
		__entry->cpt = cpt;
	),
	TP_printk("(%s:%d:%s) CPU %d is already in partition %d cpumask",
		  __get_str(msg_file), __entry->msg_line, __get_str(msg_func),
		  __entry->cpu_id, __entry->cpt)
);

#define trace_info_cpu_extra_mapping(cpu_id, cpt)			\
do {									\
	trace_libcfs_info_cpu_extra_mapping(__FILE__, __LINE__,		\
					    __func__, cpu_id, cpt);	\
									\
	CDEBUG(D_INFO, "CPU %d is already in partition %d cpumask\n",	\
	       cpu_id, cpt);						\
} while (0)

TRACE_EVENT(libcfs_info_cpu_invalid_id,
	TP_PROTO(const char *msg_file, int msg_line, const char *msg_fn,
		 int cpu_id),
	TP_ARGS(msg_file, msg_line, msg_fn, cpu_id),
	TP_STRUCT__entry(
		__string(msg_file, msg_file ? msg_file : "<nofile>")
		__field(int, msg_line)
		__string(msg_func, msg_fn ? msg_fn : "<nofunc>")
		__field(int, cpu_id)
	),
	TP_fast_assign(
		__assign_str(msg_file, kbasename(msg_file));
		__entry->msg_line = msg_line;
		__assign_str(msg_func, msg_fn);
		__entry->cpu_id = cpu_id;
	),
	TP_printk("(%s:%d:%s) Invalid CPU id %d",
		  __get_str(msg_file), __entry->msg_line, __get_str(msg_func),
		  __entry->cpu_id)
);

#define trace_info_cpu_invalid_id(cpu_id)				\
do {									\
	trace_libcfs_info_cpu_invalid_id(__FILE__, __LINE__, __func__,	\
					 cpu_id);			\
									\
	CDEBUG(D_INFO, "Invalid CPU id %d\n", cpu_id);			\
} while (0)

TRACE_EVENT(libcfs_info_cpu_invalid_numa,
	TP_PROTO(const char *msg_file, int msg_line, const char *msg_fn,
		 int numa_id, int cpu_id),
	TP_ARGS(msg_file, msg_line, msg_fn, numa_id, cpu_id),
	TP_STRUCT__entry(
		__string(msg_file, msg_file ? msg_file : "<nofile>")
		__field(int, msg_line)
		__string(msg_func, msg_fn ? msg_fn : "<nofunc>")
		__field(int, numa_id)
		__field(int, cpu_id)
	),
	TP_fast_assign(
		__assign_str(msg_file, kbasename(msg_file));
		__entry->msg_line = msg_line;
		__assign_str(msg_func, msg_fn);
		__entry->numa_id = numa_id;
		__entry->cpu_id = cpu_id;
	),
	TP_printk("(%s:%d:%s) Invalid NUMA id %d for CPU partition %d",
		  __get_str(msg_file), __entry->msg_line, __get_str(msg_func),
		  __entry->numa_id, __entry->cpu_id)
);

#define trace_info_cpu_invalid_numa(numa_id, cpu_id)			\
do {									\
	trace_libcfs_info_cpu_invalid_numa(__FILE__, __LINE__, __func__,\
					   numa_id, cpu_id);		\
									\
	CDEBUG(D_INFO, "Invalid NUMA id %d for CPU partition %d\n",	\
	       numa_id, cpu_id);					\
} while (0)

TRACE_EVENT(libcfs_info_cpu_not_mapped,
	TP_PROTO(const char *msg_file, int msg_line, const char *msg_fn,
		 int cpu_id, int cpt),
	TP_ARGS(msg_file, msg_line, msg_fn, cpu_id, cpt),
	TP_STRUCT__entry(
		__string(msg_file, msg_file ? msg_file : "<nofile>")
		__field(int, msg_line)
		__string(msg_func, msg_fn ? msg_fn : "<nofunc>")
		__field(int, cpu_id)
		__field(int, cpt)
	),
	TP_fast_assign(
		__assign_str(msg_file, kbasename(msg_file));
		__entry->msg_line = msg_line;
		__assign_str(msg_func, msg_fn);
		__entry->cpu_id = cpu_id;
		__entry->cpt = cpt;
	),
	TP_printk("(%s:%d:%s) CPU %d is not in cpu-partition %d",
		  __get_str(msg_file), __entry->msg_line, __get_str(msg_func),
		  __entry->cpu_id, __entry->cpt)
);

#define trace_info_cpu_not_mapped(cpu_id, cpt)				\
do {									\
	trace_libcfs_info_cpu_not_mapped(__FILE__, __LINE__, __func__,	\
					 cpu_id, cpt);			\
									\
	CDEBUG(D_INFO, "CPU %d is not in cpu-partition %d\n", cpu_id,	\
	       cpt);							\
} while (0)

TRACE_EVENT(libcfs_info_cpu_offline,
	TP_PROTO(const char *msg_file, int msg_line, const char *msg_fn,
		 int cpu_id),
	TP_ARGS(msg_file, msg_line, msg_fn, cpu_id),
	TP_STRUCT__entry(
		__string(msg_file, msg_file ? msg_file : "<nofile>")
		__field(int, msg_line)
		__string(msg_func, msg_fn ? msg_fn : "<nofunc>")
		__field(int, cpu_id)
	),
	TP_fast_assign(
		__assign_str(msg_file, kbasename(msg_file));
		__entry->msg_line = msg_line;
		__assign_str(msg_func, msg_fn);
		__entry->cpu_id = cpu_id;
	),
	TP_printk("(%s:%d:%s) CPU %d is invalid or it's offline",
		  __get_str(msg_file), __entry->msg_line, __get_str(msg_func),
		  __entry->cpu_id)
);

#define trace_info_cpu_offline(cpu_id)					\
do {									\
	trace_libcfs_info_cpu_offline(__FILE__, __LINE__, __func__,	\
				      cpu_id);				\
									\
	CDEBUG(D_INFO, "CPU %d is invalid or it's offline\n", cpu_id);	\
} while (0)

TRACE_EVENT(libcfs_info_cpu_unavailable,
	TP_PROTO(const char *msg_file, int msg_line, const char *msg_fn,
		 int cpt, struct cfs_cpt_table *cptab),
	TP_ARGS(msg_file, msg_line, msg_fn, cpt, cptab),
	TP_STRUCT__entry(
		__string(msg_file, msg_file ? msg_file : "<nofile>")
		__field(int, msg_line)
		__string(msg_func, msg_fn ? msg_fn : "<nofunc>")
		__field(int, cpt)
		__field(struct cfs_cpt_table *, cptab)
	),
	TP_fast_assign(
		__assign_str(msg_file, kbasename(msg_file));
		__entry->msg_line = msg_line;
		__assign_str(msg_func, msg_fn);
		__entry->cpt = cpt;
		__entry->cptab = cptab;
	),
	TP_printk("(%s:%d:%s) Try to unset cpu %d which is not in CPT-table %p",
		  __get_str(msg_file), __entry->msg_line, __get_str(msg_func),
		  __entry->cpt, __entry->cptab)
);

#define trace_info_cpu_unavailable(cpt, cptab)				\
do {									\
	trace_libcfs_info_cpu_unavailable(__FILE__, __LINE__, __func__,	\
					  cpt, cptab);			\
									\
	CDEBUG(D_INFO, "Try to unset cpu %d which is not in CPT-table %p\n",\
	       cpt, cptab);						\
} while (0)

TRACE_EVENT(libcfs_info_crypto_crc32_pclmul,
	TP_PROTO(const char *msg_file, int msg_line, const char *msg_fn),
	TP_ARGS(msg_file, msg_line, msg_fn),
	TP_STRUCT__entry(
		__string(msg_file, msg_file ? msg_file : "<nofile>")
		__field(int, msg_line)
		__string(msg_func, msg_fn ? msg_fn : "<nofunc>")
	),
	TP_fast_assign(
		__assign_str(msg_file, kbasename(msg_file));
		__entry->msg_line = msg_line;
		__assign_str(msg_func, msg_fn);
	),
	TP_printk("(%s:%d:%s) PCLMULQDQ-NI instructions are not detected.",
		  __get_str(msg_file), __entry->msg_line, __get_str(msg_func))
);

#define trace_info_crypto_crc32_pclmul_missing(void)			\
do {									\
	trace_libcfs_info_crypto_crc32_pclmul(__FILE__,	__LINE__,	\
					      __func__);		\
									\
	CDEBUG(D_INFO, "PCLMULQDQ-NI instructions are not detected.\n");\
} while (0)

TRACE_EVENT(libcfs_info_crypto_crc32c_pclmul,
	TP_PROTO(const char *msg_file, int msg_line, const char *msg_fn),
	TP_ARGS(msg_file, msg_line, msg_fn),
	TP_STRUCT__entry(
		__string(msg_file, msg_file ? msg_file : "<nofile>")
		__field(int, msg_line)
		__string(msg_func, msg_fn ? msg_fn : "<nofunc>")
	),
	TP_fast_assign(
		__assign_str(msg_file, kbasename(msg_file));
		__entry->msg_line = msg_line;
		__assign_str(msg_func, msg_fn);
	),
	TP_printk("(%s:%d:%s) CRC32 instruction are not detected.",
		  __get_str(msg_file), __entry->msg_line, __get_str(msg_func))
);

#define trace_info_crypto_crc32c_pclmul_missing(void)			\
do {									\
	trace_libcfs_info_crypto_crc32c_pclmul(__FILE__, __LINE__,	\
					       __func__);		\
									\
	CDEBUG(D_INFO, "CRC32 instruction are not detected.\n");	\
} while (0)

TRACE_EVENT(libcfs_info_crypto_ahash_alloc_fail,
	TP_PROTO(const char *msg_file, int msg_line, const char *msg_fn,
		 const char *name),
	TP_ARGS(msg_file, msg_line, msg_fn, name),
	TP_STRUCT__entry(
		__string(msg_file, msg_file ? msg_file : "<nofile>")
		__field(int, msg_line)
		__string(msg_func, msg_fn ? msg_fn : "<nofunc>")
		__string(cht_name, name)
	),
	TP_fast_assign(
		__assign_str(msg_file, kbasename(msg_file));
		__entry->msg_line = msg_line;
		__assign_str(msg_func, msg_fn);
		__assign_str(cht_name, name);
	),
	TP_printk("(%s:%d:%s) Failed to alloc crypto hash %s",
		  __get_str(msg_file), __entry->msg_line, __get_str(msg_func),
		  __get_str(cht_name))
);

#define trace_info_crypto_ahash_alloc_fail(cht_name)			\
do {									\
	trace_libcfs_info_crypto_ahash_alloc_fail(__FILE__, __LINE__,	\
						  __func__, cht_name);	\
									\
	CDEBUG(D_INFO, "Failed to alloc crypto hash %s\n", cht_name);	\
} while (0)

TRACE_EVENT(libcfs_info_crypto_request_alloc_fail,
	TP_PROTO(const char *msg_file, int msg_line, const char *msg_fn,
		 const char *name),
	TP_ARGS(msg_file, msg_line, msg_fn, name),
	TP_STRUCT__entry(
		__string(msg_file, msg_file ? msg_file : "<nofile>")
		__field(int, msg_line)
		__string(msg_func, msg_fn ? msg_fn : "<nofunc>")
		__string(cht_name, name)
	),
	TP_fast_assign(
		__assign_str(msg_file, kbasename(msg_file));
		__entry->msg_line = msg_line;
		__assign_str(msg_func, msg_fn);
		__assign_str(cht_name, name);
	),
	TP_printk("(%s:%d:%s) Failed to alloc ahash_request for %s",
		  __get_str(msg_file), __entry->msg_line, __get_str(msg_func),
		  __get_str(cht_name))
);

#define trace_info_crypto_request_alloc_fail(cht_name)			\
do {									\
	trace_libcfs_info_crypto_request_alloc_fail(__FILE__, __LINE__,	\
						    __func__, cht_name);\
									\
	CDEBUG(D_INFO, "Failed to alloc ahash_request for %s\n",	\
	       cht_name);						\
} while (0)

TRACE_EVENT(libcfs_info_crypto_setup,
	TP_PROTO(const char *msg_file, int msg_line, const char *msg_fn,
		 const char *algo, const char *driver, int speed),
	TP_ARGS(msg_file, msg_line, msg_fn, algo, driver, speed),
	TP_STRUCT__entry(
		__string(msg_file, msg_file ? msg_file : "<nofile>")
		__field(int, msg_line)
		__string(msg_func, msg_fn ? msg_fn : "<nofunc>")
		__string(algo, algo)
		__string(driver, driver)
		__field(int, speed)
	),
	TP_fast_assign(
		__assign_str(msg_file, kbasename(msg_file));
		__entry->msg_line = msg_line;
		__assign_str(msg_func, msg_fn);
		__assign_str(algo, algo);
		__assign_str(driver, driver);
		__entry->speed = speed;
	),
	TP_printk("(%s:%d:%s) Using crypto hash: %s (%s) speed %d MB/s",
		  __get_str(msg_file), __entry->msg_line, __get_str(msg_func),
		  __get_str(algo), __get_str(driver), __entry->speed)
);

#define trace_info_crypto_setup(algo, driver, speed)			\
do {									\
	trace_libcfs_info_crypto_setup(__FILE__, __LINE__, __func__,	\
				       algo, driver, speed);		\
									\
	CDEBUG(D_INFO, "Using crypto hash: %s (%s) speed %d MB/s\n",	\
	       algo, driver, speed);					\
} while (0)

TRACE_EVENT(libcfs_info_crypto_test_fail,
	TP_PROTO(const char *msg_file, int msg_line, const char *msg_fn,
		 const char *algo, int err),
	TP_ARGS(msg_file, msg_line, msg_fn, algo, err),
	TP_STRUCT__entry(
		__string(msg_file, msg_file ? msg_file : "<nofile>")
		__field(int, msg_line)
		__string(msg_func, msg_fn ? msg_fn : "<nofunc>")
		__string(algo, algo)
		__field(int, err)
	),
	TP_fast_assign(
		__assign_str(msg_file, kbasename(msg_file));
		__entry->msg_line = msg_line;
		__assign_str(msg_func, msg_fn);
		__assign_str(algo, algo);
		__entry->err = err;
	),
	TP_printk("(%s:%d:%s) Crypto hash algorithm %s test error: rc = %d",
		  __get_str(msg_file), __entry->msg_line, __get_str(msg_func),
		  __get_str(algo), __entry->err)
);

#define trace_info_crypto_test_fail(algo, err)				\
do {									\
	trace_libcfs_info_crypto_test_fail(__FILE__, __LINE__, __func__,\
					   algo, err);			\
									\
	CDEBUG(D_INFO, "Crypto hash algorithm %s test error: rc = %d\n",\
	       algo, err);						\
} while (0)

TRACE_EVENT(libcfs_info_fail_loc,
	TP_PROTO(const char *msg_file, int msg_line, const char *msg_fn,
		 u32 id, u32 value),
	TP_ARGS(msg_file, msg_line, msg_fn, id, value),
	TP_STRUCT__entry(
		__string(msg_file, msg_file ? msg_file : "<nofile>")
		__field(int, msg_line)
		__string(msg_func, msg_fn ? msg_fn : "<nofunc>")
		__field(u32, cfs_fail_loc)
		__field(u32, value)
	),
	TP_fast_assign(
		__assign_str(msg_file, kbasename(msg_file));
		__entry->msg_line = msg_line;
		__assign_str(msg_func, msg_fn);
		__entry->cfs_fail_loc = id;
		__entry->value = value;
	),
	TP_printk("(%s:%d:%s) *** cfs_fail_loc=%x, val=%u***",
		  __get_str(msg_file), __entry->msg_line, __get_str(msg_func),
		  __entry->cfs_fail_loc, __entry->value)
);

#define trace_info_fail_loc(id, value)					\
do {									\
	trace_libcfs_info_fail_loc(__FILE__, __LINE__, __func__, id,	\
				   value);				\
									\
	CDEBUG(D_INFO, "*** cfs_fail_loc=%x, val=%u***\n", id, value);	\
} while (0)

TRACE_EVENT(libcfs_info_hash_flushing,
	TP_PROTO(const char *msg_file, int msg_line, const char *msg_fn,
		 const char *name, unsigned int iter),
	TP_ARGS(msg_file, msg_line, msg_fn, name, iter),
	TP_STRUCT__entry(
		__string(msg_file, msg_file ? msg_file : "<nofile>")
		__field(int, msg_line)
		__string(msg_func, msg_fn ? msg_fn : "<nofunc>")
		__string(hs_name, name)
		__field(unsigned int, iter)
	),
	TP_fast_assign(
		__assign_str(msg_file, kbasename(msg_file));
		__entry->msg_line = msg_line;
		__assign_str(msg_func, msg_fn);
		__assign_str(hs_name, name);
		__entry->iter = iter;
	),
	TP_printk("(%s:%d:%s) Try to empty hash: %s, loop: %u",
		  __get_str(msg_file), __entry->msg_line, __get_str(msg_func),
		  __get_str(hs_name), __entry->iter)
);

#define trace_info_hash_flushing(hs_name, i)				\
do {									\
	trace_libcfs_info_hash_flushing(__FILE__, __LINE__, __func__,	\
					hs_name, i);			\
									\
	CDEBUG(D_INFO, "Try to empty hash: %s, loop: %u\n", hs_name, i);\
} while (0)

TRACE_EVENT(libcfs_info_hash_quit_rehashing,
	TP_PROTO(const char *msg_file, int msg_line, const char *msg_fn,
		 int rc),
	TP_ARGS(msg_file, msg_line, msg_fn, rc),
	TP_STRUCT__entry(
		__string(msg_file, msg_file ? msg_file : "<nofile>")
		__field(int, msg_line)
		__string(msg_func, msg_fn ? msg_fn : "<nofunc>")
		__field(int, rc)
	),
	TP_fast_assign(
		__assign_str(msg_file, kbasename(msg_file));
		__entry->msg_line = msg_line;
		__assign_str(msg_func, msg_fn);
		__entry->rc = rc;
	),
	TP_printk("(%s:%d:%s) early quit of rehashing: %d",
		  __get_str(msg_file), __entry->msg_line, __get_str(msg_func),
		  __entry->rc)
);

#define trace_info_hash_quit_rehashing(rc)				\
do {									\
	trace_libcfs_info_hash_quit_rehashing(__FILE__,	__LINE__,	\
					      __func__, rc);		\
									\
	CDEBUG(D_INFO, "early quit of rehashing: %d\n", rc);		\
} while (0)

TRACE_EVENT(libcfs_info_ptask_setup,
	TP_PROTO(const char *msg_file, int msg_line, const char *msg_fn,
		 const char *name, unsigned int weight, const char *plist,
		 const char *cblist),
	TP_ARGS(msg_file, msg_line, msg_fn, name, weight, plist, cblist),
	TP_STRUCT__entry(
		__string(msg_file, msg_file ? msg_file : "<nofile>")
		__field(int, msg_line)
		__string(msg_func, msg_fn ? msg_fn : "<nofunc>")
		__string(name, name)
		__field(unsigned int, weight)
		__string(plist, plist)
		__string(cblist, cblist)
	),
	TP_fast_assign(
		__assign_str(msg_file, kbasename(msg_file));
		__entry->msg_line = msg_line;
		__assign_str(msg_func, msg_fn);
		__assign_str(name, name);
		__entry->weight = weight;
		__assign_str(plist, plist);
		__assign_str(cblist, cblist);
	),
	TP_printk("(%s:%d:%s) %s weight=%u plist='%s' cblist='%s'",
		  __get_str(msg_file), __entry->msg_line, __get_str(msg_func),
		  __get_str(name), __entry->weight, __get_str(plist),
		  __get_str(cblist))
);

#define trace_info_ptask_setup(name, weight, plist, cblist)		\
do {									\
	trace_libcfs_info_ptask_setup(__FILE__,	__LINE__, __func__,	\
				      name, weight, plist, cblist);	\
									\
	CDEBUG(D_INFO, "%s weight=%u plist='%s' cblist='%s'\n", name,	\
	       weight, plist, cblist);					\
} while (0)

TRACE_EVENT(libcfs_info_watchdog_alloc_failed,
	TP_PROTO(const char *msg_file, int msg_line, const char *msg_fn),
	TP_ARGS(msg_file, msg_line, msg_fn),
	TP_STRUCT__entry(
		__string(msg_file, msg_file ? msg_file : "<nofile>")
		__field(int, msg_line)
		__string(msg_func, msg_fn ? msg_fn : "<nofunc>")
	),
	TP_fast_assign(
		__assign_str(msg_file, kbasename(msg_file));
		__entry->msg_line = msg_line;
		__assign_str(msg_func, msg_fn);
	),
	TP_printk("(%s:%d:%s) Could not allocate new lc_watchdog",
		  __get_str(msg_file), __entry->msg_line, __get_str(msg_func))
);

#define trace_info_watchdog_alloc_failed(void)				\
do {									\
	trace_libcfs_info_watchdog_alloc_failed(__FILE__, __LINE__,	\
						__func__);		\
									\
	CDEBUG(D_INFO, "Could not allocate new lc_watchdog\n");		\
} while (0)

TRACE_EVENT(libcfs_info_watchdog_dispatch_starting,
	TP_PROTO(const char *msg_file, int msg_line, const char *msg_fn),
	TP_ARGS(msg_file, msg_line, msg_fn),
	TP_STRUCT__entry(
		__string(msg_file, msg_file ? msg_file : "<nofile>")
		__field(int, msg_line)
		__string(msg_func, msg_fn ? msg_fn : "<nofunc>")
	),
	TP_fast_assign(
		__assign_str(msg_file, kbasename(msg_file));
		__entry->msg_line = msg_line;
		__assign_str(msg_func, msg_fn);
	),
	TP_printk("(%s:%d:%s) starting dispatch thread",
		  __get_str(msg_file), __entry->msg_line, __get_str(msg_func))
);

#define trace_info_watchdog_dispatch_starting(void)			\
do {									\
	trace_libcfs_info_watchdog_dispatch_starting(__FILE__, __LINE__,\
						     __func__);		\
									\
	CDEBUG(D_INFO, "starting dispatch thread\n");			\
} while (0)

TRACE_EVENT(libcfs_info_watchdog_dispatch_complete,
	TP_PROTO(const char *msg_file, int msg_line, const char *msg_fn),
	TP_ARGS(msg_file, msg_line, msg_fn),
	TP_STRUCT__entry(
		__string(msg_file, msg_file ? msg_file : "<nofile>")
		__field(int, msg_line)
		__string(msg_func, msg_fn ? msg_fn : "<nofunc>")
	),
	TP_fast_assign(
		__assign_str(msg_file, kbasename(msg_file));
		__entry->msg_line = msg_line;
		__assign_str(msg_func, msg_fn);
	),
	TP_printk("(%s:%d:%s) starting dispatch thread",
		  __get_str(msg_file), __entry->msg_line, __get_str(msg_func))
);

#define trace_info_watchdog_dispatch_complete(void)			\
do {									\
	trace_libcfs_info_watchdog_dispatch_complete(__FILE__, __LINE__,\
						     __func__);		\
									\
	CDEBUG(D_INFO, "starting dispatch thread\n");			\
} while (0)

TRACE_EVENT(libcfs_info_watchdog_dispatch_main,
	TP_PROTO(const char *msg_file, int msg_line, const char *msg_fn),
	TP_ARGS(msg_file, msg_line, msg_fn),
	TP_STRUCT__entry(
		__string(msg_file, msg_file ? msg_file : "<nofile>")
		__field(int, msg_line)
		__string(msg_func, msg_fn ? msg_fn : "<nofunc>")
	),
	TP_fast_assign(
		__assign_str(msg_file, kbasename(msg_file));
		__entry->msg_line = msg_line;
		__assign_str(msg_func, msg_fn);
	),
	TP_printk("(%s:%d:%s) Watchdog got woken up...",
		  __get_str(msg_file), __entry->msg_line, __get_str(msg_func))
);

#define trace_info_watchdog_dispatch_main(void)				\
do {									\
	trace_libcfs_info_watchdog_dispatch_main(__FILE__, __LINE__,	\
						 __func__);		\
									\
	CDEBUG(D_INFO, "Watchdog got woken up...\n");			\
} while (0)

TRACE_EVENT(libcfs_info_watchdog_dispatch_stopping,
	TP_PROTO(const char *msg_file, int msg_line, const char *msg_fn),
	TP_ARGS(msg_file, msg_line, msg_fn),
	TP_STRUCT__entry(
		__string(msg_file, msg_file ? msg_file : "<nofile>")
		__field(int, msg_line)
		__string(msg_func, msg_fn ? msg_fn : "<nofunc>")
	),
	TP_fast_assign(
		__assign_str(msg_file, kbasename(msg_file));
		__entry->msg_line = msg_line;
		__assign_str(msg_func, msg_fn);
	),
	TP_printk("(%s:%d:%s) trying to stop watchdog dispatcher.",
		  __get_str(msg_file), __entry->msg_line, __get_str(msg_func))
);

#define trace_info_watchdog_dispatch_stopping(void)			\
do {									\
	trace_libcfs_info_watchdog_dispatch_stopping(__FILE__, __LINE__,\
						     __func__);		\
									\
	CDEBUG(D_INFO, "trying to stop watchdog dispatcher.");		\
} while (0)

TRACE_EVENT(libcfs_info_watchdog_dispatch_stopped,
	TP_PROTO(const char *msg_file, int msg_line, const char *msg_fn),
	TP_ARGS(msg_file, msg_line, msg_fn),
	TP_STRUCT__entry(
		__string(msg_file, msg_file ? msg_file : "<nofile>")
		__field(int, msg_line)
		__string(msg_func, msg_fn ? msg_fn : "<nofunc>")
	),
	TP_fast_assign(
		__assign_str(msg_file, kbasename(msg_file));
		__entry->msg_line = msg_line;
		__assign_str(msg_func, msg_fn);
	),
	TP_printk("(%s:%d:%s) watchdog dispatcher has shut down.",
		  __get_str(msg_file), __entry->msg_line, __get_str(msg_func))
);

#define trace_info_watchdog_dispatch_stopped(void)			\
do {									\
	trace_libcfs_info_watchdog_dispatch_stopped(__FILE__, __LINE__, \
						    __func__);		\
									\
	CDEBUG(D_INFO, "watchdog dispatcher has shut down.\n");		\
} while (0)

TRACE_EVENT(libcfs_info_watchdog_flagged_stop,
	TP_PROTO(const char *msg_file, int msg_line, const char *msg_fn),
	TP_ARGS(msg_file, msg_line, msg_fn),
	TP_STRUCT__entry(
		__string(msg_file, msg_file ? msg_file : "<nofile>")
		__field(int, msg_line)
		__string(msg_func, msg_fn ? msg_fn : "<nofunc>")
	),
	TP_fast_assign(
		__assign_str(msg_file, kbasename(msg_file));
		__entry->msg_line = msg_line;
		__assign_str(msg_func, msg_fn);
	),
	TP_printk("(%s:%d:%s) LCW_FLAG_STOP set, shutting down...",
		  __get_str(msg_file), __entry->msg_line, __get_str(msg_func))
);

#define trace_info_watchdog_flagged_stop(void)				\
do {									\
	trace_libcfs_info_watchdog_flagged_stop(__FILE__, __LINE__,	\
						__func__);		\
									\
	CDEBUG(D_INFO, "LCW_FLAG_STOP set, shutting down...\n");	\
} while (0)

TRACE_EVENT(libcfs_info_watchdog_pid,
	TP_PROTO(const char *msg_file, int msg_line, const char *msg_fn,
		 int pid),
	TP_ARGS(msg_file, msg_line, msg_fn, pid),
	TP_STRUCT__entry(
		__string(msg_file, msg_file ? msg_file : "<nofile>")
		__field(int, msg_line)
		__string(msg_func, msg_fn ? msg_fn : "<nofunc>")
		__field(int, pid)
	),
	TP_fast_assign(
		__assign_str(msg_file, kbasename(msg_file));
		__entry->msg_line = msg_line;
		__assign_str(msg_func, msg_fn);
		__entry->pid = pid;
	),
	TP_printk("(%s:%d:%s) found lcw for pid %d",
		  __get_str(msg_file), __entry->msg_line, __get_str(msg_func),
		  __entry->pid)
);

#define trace_info_watchdog_pid(pid)					\
do {									\
	trace_libcfs_info_watchdog_pid(__FILE__, __LINE__, __func__,	\
				       pid);				\
									\
	CDEBUG(D_INFO, "found lcw for pid %d\n", pid);			\
} while (0)

TRACE_EVENT(libcfs_info_workitem_destroy,
	TP_PROTO(const char *msg_file, int msg_line, const char *msg_fn,
		 const char *ws_name),
	TP_ARGS(msg_file, msg_line, msg_fn, ws_name),
	TP_STRUCT__entry(
		__string(msg_file, msg_file ? msg_file : "<nofile>")
		__field(int, msg_line)
		__string(msg_func, msg_fn ? msg_fn : "<nofunc>")
		__string(ws_name, ws_name)
	),
	TP_fast_assign(
		__assign_str(msg_file, kbasename(msg_file));
		__entry->msg_line = msg_line;
		__assign_str(msg_func, msg_fn);
		__assign_str(ws_name, ws_name);
	),
	TP_printk("(%s:%d:%s) %s is in progress of stopping",
		  __get_str(msg_file), __entry->msg_line, __get_str(msg_func),
		  __get_str(ws_name))
);

#define trace_info_workitem_destroy(ws_name)				\
do {									\
	trace_libcfs_info_workitem_destroy(__FILE__, __LINE__, __func__,\
					   ws_name);			\
									\
	CDEBUG(D_INFO, "%s is in progress of stopping\n", ws_name);	\
} while (0)

TRACE_EVENT(libcfs_ioctl_invalid,
	TP_PROTO(const char *msg_file, int msg_line, const char *msg_fn,
		 int ioctl_cmd),
	TP_ARGS(msg_file, msg_line, msg_fn, ioctl_cmd),
	TP_STRUCT__entry(
		__string(msg_file, msg_file ? msg_file : "<nofile>")
		__field(int, msg_line)
		__string(msg_func, msg_fn ? msg_fn : "<nofunc>")
		__field(int, cmd)
	),
	TP_fast_assign(
		__assign_str(msg_file, kbasename(msg_file));
		__entry->msg_line = msg_line;
		__assign_str(msg_func, msg_fn);
		__entry->cmd = ioctl_cmd;
	),
	TP_printk("(%s:%d:%s) invalid ioctl ( type %d, nr %d, size %d )",
		  __get_str(msg_file), __entry->msg_line, __get_str(msg_func),
		  _IOC_TYPE(__entry->cmd), _IOC_NR(__entry->cmd),
		  _IOC_SIZE(__entry->cmd))
);

#define trace_ioctl_failed(cmd)						\
do {									\
	trace_libcfs_ioctl_invalid(__FILE__, __LINE__, __func__, cmd);	\
									\
	CDEBUG(D_IOCTL, "invalid ioctl ( type %d, nr %d, size %d )\n",	\
	       _IOC_TYPE(cmd), _IOC_NR(cmd), _IOC_SIZE(cmd));		\
} while (0)

TRACE_EVENT(libcfs_ioctl,
	TP_PROTO(const char *msg_file, int msg_line, const char *msg_fn,
		 unsigned long ioctl_cmd),
	TP_ARGS(msg_file, msg_line, msg_fn, ioctl_cmd),
	TP_STRUCT__entry(
		__string(msg_file, msg_file ? msg_file : "<nofile>")
		__field(int, msg_line)
		__string(msg_func, msg_fn ? msg_fn : "<nofunc>")
		__field(unsigned long, cmd)
	),
	TP_fast_assign(
		__assign_str(msg_file, kbasename(msg_file));
		__entry->msg_line = msg_line;
		__assign_str(msg_func, msg_fn);
		__entry->cmd = ioctl_cmd;
	),
	TP_printk("(%s:%d:%s) libcfs ioctl cmd %lu",
		  __get_str(msg_file), __entry->msg_line, __get_str(msg_func),
		  __entry->cmd)
);

#define trace_ioctl(cmd)						\
do {									\
	trace_libcfs_ioctl(__FILE__, __LINE__, __func__, cmd);		\
									\
	CDEBUG(D_IOCTL, "libcfs ioctl cmd %lu\n", cmd);			\
} while (0)

TRACE_EVENT(libcfs_warning_hash_depth,
	TP_PROTO(const char *msg_file, int msg_line, const char *msg_fn,
		 const char *hs_name, int bits, int dep_max, int bkt,
		 int dep_off),
	TP_ARGS(msg_file, msg_line, msg_fn, hs_name, bits, dep_max, bkt,
		dep_off),
	TP_STRUCT__entry(
		__string(msg_file, msg_file ? msg_file : "<nofile>")
		__field(int, msg_line)
		__string(msg_func, msg_fn ? msg_fn : "<nofunc>")
		__string(name, hs_name)
		__field(int, bits)
		__field(int, dep_max)
		__field(int, bkt)
		__field(int, dep_off)
	),
	TP_fast_assign(
		__assign_str(msg_file, kbasename(msg_file));
		__entry->msg_line = msg_line;
		__assign_str(msg_func, msg_fn);
		__assign_str(name, hs_name);
		__entry->bits = bits;
		__entry->dep_max = dep_max;
		__entry->bkt = bkt;
		__entry->dep_off = dep_off;
	),
	TP_printk("(%s:%d:%s) #### HASH %s (bits: %d): max depth %d at bucket %d/%d",
		  __get_str(msg_file), __entry->msg_line, __get_str(msg_func),
		  __get_str(name), __entry->bits, __entry->dep_max,
		  __entry->bkt, __entry->dep_off)
);

#define trace_warn_hash_depth(name, bits, max, bkt, off)		\
do {									\
	trace_libcfs_warning_hash_depth(__FILE__, __LINE__, __func__,	\
					name, bits, max, bkt, dep_off);	\
									\
	LCONSOLE_WARN("#### HASH %s (bits: %d): max depth %d at bucket %d/%d\n",\
		      hs->hs_name, bits, dep, bkt, off);		\
} while (0)

TRACE_EVENT(libcfs_warning_watchdog_expired,
	TP_PROTO(const char *msg_file, int msg_line, const char *msg_fn,
		 int pid, const char *message, struct timespec64 ts),
	TP_ARGS(msg_file, msg_line, msg_fn, pid, message, ts),
	TP_STRUCT__entry(
		__string(msg_file, msg_file ? msg_file : "<nofile>")
		__field(int, msg_line)
		__string(msg_func, msg_fn ? msg_fn : "<nofunc>")
		__field(int, pid)
		__string(msg, message)
		__field(unsigned long, sec)
		__field(unsigned long, msec)
	),
	TP_fast_assign(
		__assign_str(msg_file, kbasename(msg_file));
		__entry->msg_line = msg_line;
		__assign_str(msg_func, msg_fn);
		__entry->pid = pid;
		__assign_str(msg, message);
		__entry->sec = ts.tv_sec;
		__entry->msec = ts.tv_nsec / (NSEC_PER_SEC * 100);
	),
	TP_printk("(%s:%d:%s) Service thread pid %u %s after %lu.%.02lu secs. This indicates the system was overloaded (too many service threads, or there were not enough hardware resources).",
		  __get_str(msg_file), __entry->msg_line, __get_str(msg_func),
		  __entry->pid, __get_str(msg), __entry->sec, __entry->msec)
);

#define trace_warn_watchdog_expired(pid, msg, ts)			\
do {									\
	trace_libcfs_warning_watchdog_expired(__FILE__, __LINE__,	\
					      __func__, pid, msg, ts);	\
									\
	LCONSOLE_WARN("Service thread pid %u %s after %lu.%.02lu secs. This indicates the system was overloaded (too many service threads, or there were not enough hardware resources).\n",			       \
		      pid, msg, ts.tv_sec,				\
		      ts.tv_nsec / (NSEC_PER_SEC * 100));		\
} while (0)

#endif /* __LIBCFS_TRACE_H */

#undef TRACE_INCLUDE_PATH
#define TRACE_INCLUDE_PATH .

#undef TRACE_INCLUDE_FILE
#define TRACE_INCLUDE_FILE libcfs_trace

#include <trace/define_trace.h>
