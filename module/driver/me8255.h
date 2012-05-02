/**
 * @file me8255.h
 *
 * @brief The 8255 DIO subdevice class header file.
 * @note Copyright (C) 2007 Meilhaus Electronic GmbH (support@meilhaus.de)
 * @author KG (Krzysztof Gantzke) (k.gantzke@meilhaus.de)
 * @author GG (Guenter Gebhardt)
 */

#ifdef __KERNEL__

# ifndef _ME8255_H_
#  define _ME8255_H_

#  include "mesubdevice.h"

/**
 * @brief The 8255 subdevice class.
 */
typedef struct //me8255_subdevice
{
	// Inheritance
	me_subdevice_t base;			/**< The subdevice base class. */

	// Attributes
	me_lock_t* ctrl_reg_lock;		/**< Spin lock to protect ctrl_reg and ctrl_reg_mirror from concurrent access. */

	int* ctrl_reg_mirror;			/**< Pointer to mirror of the control register. */
	uint32_t* port_regs_mirror;		/**< Pointer to mirror of the port register. */

	void* port_base_reg;			/**< Register (base) to read or write a value from or to the port respectively. */
	void* ctrl_reg;					/**< Register to configure the 8255 modes. */
} me8255_subdevice_t;


/**
 * @brief The constructor to generate a 8255 instance.
 *
 * @param device_id The kind of Meilhaus device holding the 8255.
 * @param reg_base The register base address of the device as returned by the PCI BIOS.
 * @param me8255_idx The index of the 8255 chip on the Meilhaus device.
 * @param idx The index of the counter inside a 8255 chip.
 * @param ctr_reg_mirror Pointer to mirror of control register.
 * @param ctrl_reg_lock Pointer to spin lock protecting the 8255 control register and #ctrl_reg_mirror from concurrent access.
 *
 * @return Pointer to new instance on success.\n
 *			NULL on error.
 */
me8255_subdevice_t* me8255_constr(uint16_t device_id, void* reg_base,
										unsigned int me8255_idx, unsigned int idx,
										uint32_t* port_reg_mirror,
										int* ctrl_reg_mirror, me_lock_t* ctrl_reg_lock);

# endif
#endif
