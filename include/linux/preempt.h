#ifndef __LINUX_PREEMPT_H
#define __LINUX_PREEMPT_H

/*
 * include/linux/preempt.h - macros for accessing and manipulating
 * preempt_count (used for kernel preemption, interrupt count, etc.)
 */

#include <linux/thread_info.h>
#include <linux/linkage.h>
#include <linux/list.h>

#if defined(CONFIG_DEBUG_PREEMPT) || defined(CONFIG_PREEMPT_TRACER)
  extern void add_preempt_count(int val);
  extern void sub_preempt_count(int val);
#else
# define add_preempt_count(val)	do { preempt_count() += (val); } while (0)
# define sub_preempt_count(val)	do { preempt_count() -= (val); } while (0)
#endif

#define raw_add_preempt_count(val)	do { preempt_count() += (val); } while (0)
#define raw_sub_preempt_count(val)	do { preempt_count() -= (val); } while (0)

#define inc_preempt_count() add_preempt_count(1)
#define raw_inc_preempt_count() raw_add_preempt_count(1)
#define dec_preempt_count() sub_preempt_count(1)
#define raw_dec_preempt_count() raw_sub_preempt_count(1)

#define preempt_count()	(current_thread_info()->preempt_count)

#ifdef CONFIG_PREEMPT_LAZY
#define add_preempt_lazy_count(val)	do { preempt_lazy_count() += (val); } while (0)
#define sub_preempt_lazy_count(val)	do { preempt_lazy_count() -= (val); } while (0)
#define inc_preempt_lazy_count()	add_preempt_lazy_count(1)
#define dec_preempt_lazy_count()	sub_preempt_lazy_count(1)
#define preempt_lazy_count()		(current_thread_info()->preempt_lazy_count)
#else
#define add_preempt_lazy_count(val)	do { } while (0)
#define sub_preempt_lazy_count(val)	do { } while (0)
#define inc_preempt_lazy_count()	do { } while (0)
#define dec_preempt_lazy_count()	do { } while (0)
#define preempt_lazy_count()		(0)
#endif

#ifdef CONFIG_PREEMPT

asmlinkage void preempt_schedule(void);

# ifdef CONFIG_PREEMPT_LAZY
#define preempt_check_resched() \
do { \
	if (unlikely(test_thread_flag(TIF_NEED_RESCHED) || \
		     test_thread_flag(TIF_NEED_RESCHED_LAZY)))  \
		preempt_schedule(); \
} while (0)
# else
#define preempt_check_resched() \
do { \
	if (unlikely(test_thread_flag(TIF_NEED_RESCHED)))	\
		preempt_schedule(); \
} while (0)
# endif

#ifdef CONFIG_CONTEXT_TRACKING

void preempt_schedule_context(void);

#define preempt_check_resched_context() \
do { \
	if (unlikely(test_thread_flag(TIF_NEED_RESCHED))) \
		preempt_schedule_context(); \
} while (0)
#else

#define preempt_check_resched_context() preempt_check_resched()

#endif /* CONFIG_CONTEXT_TRACKING */

#else /* !CONFIG_PREEMPT */

#define preempt_check_resched()		do { } while (0)
#define preempt_check_resched_context()	do { } while (0)

#endif /* CONFIG_PREEMPT */


#ifdef CONFIG_PREEMPT_COUNT

#define preempt_disable() \
do { \
	inc_preempt_count(); \
	barrier(); \
} while (0)

#define preempt_lazy_disable() \
do { \
	inc_preempt_lazy_count(); \
	barrier(); \
} while (0)

#define raw_preempt_disable() \
do { \
	raw_inc_preempt_count(); \
	barrier(); \
} while (0)

#define sched_preempt_enable_no_resched() \
do { \
	barrier(); \
	dec_preempt_count(); \
} while (0)

#ifndef CONFIG_PREEMPT_RT_BASE
# define preempt_enable_no_resched()	sched_preempt_enable_no_resched()
# define preempt_check_resched_rt()	barrier()
#else
# define preempt_enable_no_resched()	preempt_enable()
# define preempt_check_resched_rt()	preempt_check_resched()
#endif

#define raw_preempt_enable_no_resched() \
do { \
	barrier(); \
	raw_dec_preempt_count(); \
} while (0)

#define preempt_enable() \
do { \
	sched_preempt_enable_no_resched(); \
	barrier(); \
	preempt_check_resched(); \
} while (0)

#define preempt_lazy_enable() \
do { \
	dec_preempt_lazy_count(); \
	barrier(); \
	preempt_check_resched(); \
} while (0)

/* For debugging and tracer internals only! */
#define add_preempt_count_notrace(val)			\
	do { preempt_count() += (val); } while (0)
#define sub_preempt_count_notrace(val)			\
	do { preempt_count() -= (val); } while (0)
#define inc_preempt_count_notrace() add_preempt_count_notrace(1)
#define dec_preempt_count_notrace() sub_preempt_count_notrace(1)

#define preempt_disable_notrace() \
do { \
	inc_preempt_count_notrace(); \
	barrier(); \
} while (0)

#define preempt_enable_no_resched_notrace() \
do { \
	barrier(); \
	dec_preempt_count_notrace(); \
} while (0)

/* preempt_check_resched is OK to trace */
#define preempt_enable_notrace() \
do { \
	preempt_enable_no_resched_notrace(); \
	barrier(); \
	preempt_check_resched_context(); \
} while (0)

#else /* !CONFIG_PREEMPT_COUNT */

/*
 * Even if we don't have any preemption, we need preempt disable/enable
 * to be barriers, so that we don't have things like get_user/put_user
 * that can cause faults and scheduling migrate into our preempt-protected
 * region.
 */
#define preempt_disable()		barrier()
#define raw_preempt_disable()		barrier()
#define sched_preempt_enable_no_resched()	barrier()
#define preempt_enable_no_resched()	barrier()
#define raw_preempt_enable_no_resched()	barrier()
#define preempt_enable()		barrier()

#define preempt_disable_notrace()		barrier()
#define preempt_enable_no_resched_notrace()	barrier()
#define preempt_enable_notrace()		barrier()
#define preempt_check_resched_rt()		barrier()

#endif /* CONFIG_PREEMPT_COUNT */

#ifdef CONFIG_PREEMPT_RT_FULL
# define preempt_disable_rt()		preempt_disable()
# define preempt_enable_rt()		preempt_enable()
# define preempt_disable_nort()		barrier()
# define preempt_enable_nort()		barrier()
# ifdef CONFIG_SMP
   extern void migrate_disable(void);
   extern void migrate_enable(void);
# else /* CONFIG_SMP */
#  define migrate_disable()		barrier()
#  define migrate_enable()		barrier()
# endif /* CONFIG_SMP */
#else
# define preempt_disable_rt()		barrier()
# define preempt_enable_rt()		barrier()
# define preempt_disable_nort()		preempt_disable()
# define preempt_enable_nort()		preempt_enable()
# define migrate_disable()		preempt_disable()
# define migrate_enable()		preempt_enable()
#endif

#ifdef CONFIG_PREEMPT_NOTIFIERS

struct preempt_notifier;

/**
 * preempt_ops - notifiers called when a task is preempted and rescheduled
 * @sched_in: we're about to be rescheduled:
 *    notifier: struct preempt_notifier for the task being scheduled
 *    cpu:  cpu we're scheduled on
 * @sched_out: we've just been preempted
 *    notifier: struct preempt_notifier for the task being preempted
 *    next: the task that's kicking us out
 *
 * Please note that sched_in and out are called under different
 * contexts.  sched_out is called with rq lock held and irq disabled
 * while sched_in is called without rq lock and irq enabled.  This
 * difference is intentional and depended upon by its users.
 */
struct preempt_ops {
	void (*sched_in)(struct preempt_notifier *notifier, int cpu);
	void (*sched_out)(struct preempt_notifier *notifier,
			  struct task_struct *next);
};

/**
 * preempt_notifier - key for installing preemption notifiers
 * @link: internal use
 * @ops: defines the notifier functions to be called
 *
 * Usually used in conjunction with container_of().
 */
struct preempt_notifier {
	struct hlist_node link;
	struct preempt_ops *ops;
};

void preempt_notifier_register(struct preempt_notifier *notifier);
void preempt_notifier_unregister(struct preempt_notifier *notifier);

static inline void preempt_notifier_init(struct preempt_notifier *notifier,
				     struct preempt_ops *ops)
{
	INIT_HLIST_NODE(&notifier->link);
	notifier->ops = ops;
}

#endif

#endif /* __LINUX_PREEMPT_H */