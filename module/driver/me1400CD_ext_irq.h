/**
 * @file me1400CD_ext_irq.h
 *
 * @brief The ME-1400 (ver. C and D) external interrupt header file.
 * @note Copyright (C) 2007 Meilhaus Electronic GmbH (support@meilhaus.de)
 * @author KG (Krzysztof Gantzke) (k.gantzke@meilhaus.de)
 * @author GG (Guenter Gebhardt)
 */

#ifdef __KERNEL__

# ifndef _ME1400CD_EXT_IRQ_H_
#  define _ME1400CD_EXT_IRQ_H_

#  include <linux/sched.h>

#  include "mesubdevice.h"
#  include "me_interrupt_types.h"

/**
 * @brief The ME-1400CD external interrupt subdevice class.
 */
typedef struct //me1400CD_ext_irq_subdevice
{
	// Inheritance
	me_subdevice_t base;			/**< The subdevice base class. */

	// Attributes
	me_lock_t* ctrl_reg_lock;		/**< Lock protecting the irq/clock control register (C and D). */

	wait_queue_head_t wait_queue;	/**< Queue to put on threads waiting for an interrupt. */

	volatile enum ME_IRQ_STATUS status;
	int value;
	volatile int count;
	volatile int reset_count;

	void* ctrl_reg;					/**< The control register. */
} me1400CD_ext_irq_subdevice_t;


/**
 * @brief The constructor to generate a ME-1400CD external interrupt instance.
 *
 * @param reg_base The register base address of the device as returned by the PCI BIOS.
 * @param idx Subdevice number.
 * @param ctrl_reg_lock Pointer to spin lock protecting the irq control register from concurrent access.
 *
 * @return Pointer to new instance on success.\n
 *			NULL on error.
 */
me1400CD_ext_irq_subdevice_t* me1400CD_ext_irq_constr(void* reg_base, int idx, me_lock_t* ctrl_reg_lock);

# endif
#endif
