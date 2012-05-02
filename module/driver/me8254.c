/**
 * @file me8254.c
 *
 * @brief The 8254 counter subdevice instance.
 * @note Copyright (C) 2007 Meilhaus Electronic GmbH (support@meilhaus.de)
 * @author KG (Krzysztof Gantzke) (k.gantzke@meilhaus.de)
 * @author GG (Guenter Gebhardt)
 */

/*
 * Copyright (C) 2007 Meilhaus Electronic GmbH (support@meilhaus.de)
 *
 * This file is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#ifndef __KERNEL__
# error Kernel only!
#endif

# ifndef MODULE
#  define MODULE
# endif

# include <linux/fs.h>
# include <linux/slab.h>
# include <asm/uaccess.h>

# include <linux/sched.h>

# include "me_debug.h"
# include "me_error.h"
# include "me_defines.h"
# include "me_internal.h"
# include "mehardware_access.h"
# include "me_spin_lock.h"

#include "me8254_reg.h"
#include "me8254.h"

#define ME8254_NUMBER_CHANNELS	1	/**< One channel per counter. */
#define ME8254_CTR_WIDTH		16	/**< One counter has 16 bits. */


int me8254_io_reset_subdevice(struct me_subdevice* subdevice, struct file* filep, int flags);
int me8254_io_single_config(struct me_subdevice* subdevice, struct file* filep, int channel,
									int single_config, int ref, int trig_chain,  int trig_type, int trig_edge, int flags);
int me8254_io_single_read(struct me_subdevice* subdevice, struct file* filep, int channel, int* value, int time_out, int flags);
int me8254_io_single_write(struct me_subdevice* subdevice, struct file* filep, int channel,int value, int time_out, int flags);
int me8254_query_number_channels(struct me_subdevice* subdevice, int* number);
int me8254_query_subdevice_type(struct me_subdevice* subdevice, int* type, int* subtype);
int me8254_query_subdevice_caps(struct me_subdevice* subdevice, int* caps);
int me8254_query_subdevice_caps_args(struct me_subdevice* subdevice, int cap, int* args, int* count);

static int me_ref_config(me8254_subdevice_t* instance, int ref);
static void* me4600_get_val_reg(void* reg_base, unsigned int me8254_idx, unsigned int ctr_idx);
static void* me4600_get_ctrl_reg(void* reg_base, unsigned int me8254_idx, unsigned int ctr_idx);
static void* me8100_get_val_reg(void* reg_base, unsigned int me8254_idx, unsigned int ctr_idx);
static void* me8100_get_ctrl_reg(void* reg_base, unsigned int me8254_idx, unsigned int ctr_idx);

static int me1400_ab_ref_config(me8254_subdevice_t* instance, int ref);
static void* me1400AB_get_val_reg(void* reg_base, unsigned int me8254_idx, unsigned int ctr_idx);
static void* me1400AB_get_ctrl_reg(void* reg_base, unsigned int me8254_idx, unsigned int ctr_idx);
static void* me1400AB_get_clk_src_reg(void* reg_base, unsigned int me8254_idx, unsigned int ctr_idx);

static int me1400_cd_ref_config(me8254_subdevice_t* instance, int ref);
static void* me1400CD_get_val_reg(void* reg_base, unsigned int me8254_idx, unsigned int ctr_idx);
static void* me1400AB_get_ctrl_reg(void* reg_base, unsigned int me8254_idx, unsigned int ctr_idx);
static void* me1400CD_get_clk_src_reg(void* reg_base, unsigned int me8254_idx, unsigned int ctr_idx);

int me8254_io_reset_subdevice(struct me_subdevice* subdevice, struct file* filep, int flags)
{
	me8254_subdevice_t* instance;
	uint8_t clk_src;

	instance = (me8254_subdevice_t *) subdevice;

	PDEBUG("executed.\n");

	if (flags)
	{
		PERROR("Invalid flags specified. Must be ME_IO_RESET_SUBDEVICE_NO_FLAGS.\n");
		return ME_ERRNO_INVALID_FLAGS;
	}

	ME_SUBDEVICE_ENTER;
		ME_SUBDEVICE_LOCK;
			ME_SPIN_LOCK(instance->ctrl_reg_lock);
				if (instance->ctr_idx == 0)
					me_writeb(instance->base.dev, ME8254_CTRL_SC0 | ME8254_CTRL_LM | ME8254_CTRL_M0 | ME8254_CTRL_BIN, instance->ctrl_reg);
				else if (instance->ctr_idx == 1)
					me_writeb(instance->base.dev, ME8254_CTRL_SC1 | ME8254_CTRL_LM | ME8254_CTRL_M0 | ME8254_CTRL_BIN, instance->ctrl_reg);
				else
					me_writeb(instance->base.dev, ME8254_CTRL_SC2 | ME8254_CTRL_LM | ME8254_CTRL_M0 | ME8254_CTRL_BIN, instance->ctrl_reg);
			ME_SPIN_UNLOCK(instance->ctrl_reg_lock);

			me_writeb(instance->base.dev, 0x00, instance->val_reg);
			me_writeb(instance->base.dev, 0x00, instance->val_reg);
			ME_SPIN_LOCK(instance->clk_src_reg_lock);
/** @todo Instead of ID  features flags should be used.
*/
				switch (instance->device_id)
				{
					case PCI_DEVICE_ID_MEILHAUS_ME1400:
					case PCI_DEVICE_ID_MEILHAUS_ME140A:
					case PCI_DEVICE_ID_MEILHAUS_ME140B:
					case PCI_DEVICE_ID_MEILHAUS_ME14E0:
					case PCI_DEVICE_ID_MEILHAUS_ME14EA:
					case PCI_DEVICE_ID_MEILHAUS_ME14EB:
						me_readb(instance->base.dev, &clk_src, instance->clk_src_reg);
						if (instance->me8254_idx == 0)
						{
							if (instance->ctr_idx == 0)
								clk_src &= ~(ME1400AB_8254_A_0_CLK_SRC_10MHZ | ME1400AB_8254_A_0_CLK_SRC_QUARZ);
							else if (instance->ctr_idx == 1)
								clk_src &= ~(ME1400AB_8254_A_1_CLK_SRC_PREV);
							else
								clk_src &= ~(ME1400AB_8254_A_2_CLK_SRC_PREV);
						}
						else
						{
							if (instance->ctr_idx == 0)
								clk_src &= ~(ME1400AB_8254_B_0_CLK_SRC_10MHZ | ME1400AB_8254_B_0_CLK_SRC_QUARZ);
							else if (instance->ctr_idx == 1)
								clk_src &= ~(ME1400AB_8254_B_1_CLK_SRC_PREV);
							else
								clk_src &= ~(ME1400AB_8254_B_2_CLK_SRC_PREV);
						}
						me_writeb(instance->base.dev, clk_src, instance->clk_src_reg);
						break;

					case PCI_DEVICE_ID_MEILHAUS_ME140C:
					case PCI_DEVICE_ID_MEILHAUS_ME140D:
						me_readb(instance->base.dev, &clk_src, instance->clk_src_reg);
						switch (instance->me8254_idx)
						{
							case 0:
							case 2:
							case 4:
							case 6:
							case 8:
								if (instance->ctr_idx == 0)
									clk_src &= ~(ME1400CD_8254_ACE_0_CLK_SRC_MASK);
								else if (instance->ctr_idx == 1)
									clk_src &= ~(ME1400CD_8254_ACE_1_CLK_SRC_MASK);
								else
									clk_src &= ~(ME1400CD_8254_ACE_2_CLK_SRC_MASK);
								break;

							default:
								if (instance->ctr_idx == 0)
									clk_src &= ~(ME1400CD_8254_BD_0_CLK_SRC_MASK);
								else if (instance->ctr_idx == 1)
									clk_src &= ~(ME1400CD_8254_BD_1_CLK_SRC_MASK);
								else
									clk_src &= ~(ME1400CD_8254_BD_2_CLK_SRC_MASK);
								break;
						}
						me_writeb(instance->base.dev, clk_src, instance->clk_src_reg);
						break;

					default:
						// No clock source register available.
						break;
				}
			ME_SPIN_UNLOCK(instance->clk_src_reg_lock);
		ME_SUBDEVICE_UNLOCK;
	ME_SUBDEVICE_EXIT;

	return ME_ERRNO_SUCCESS;
}

static int me1400_ab_ref_config(me8254_subdevice_t* instance, int ref)
{
	uint8_t clk_src;
	int err = ME_ERRNO_SUCCESS;

	ME_SPIN_LOCK(instance->clk_src_reg_lock);
		me_readb(instance->base.dev, &clk_src, instance->clk_src_reg);
		switch (ref)
		{
			case ME_REF_CTR_EXTERNAL:
				if (instance->me8254_idx == 0)
				{
					if (instance->ctr_idx == 0)
						clk_src &= ~(ME1400AB_8254_A_0_CLK_SRC_QUARZ);
					else if (instance->ctr_idx == 1)
						clk_src &= ~(ME1400AB_8254_A_1_CLK_SRC_PREV);
					else
						clk_src &= ~(ME1400AB_8254_A_2_CLK_SRC_PREV);
				}
				else
				{
					if (instance->ctr_idx == 0)
						clk_src &= ~(ME1400AB_8254_B_0_CLK_SRC_QUARZ);
					else if (instance->ctr_idx == 1)
						clk_src &= ~(ME1400AB_8254_B_1_CLK_SRC_PREV);
					else
						clk_src &= ~(ME1400AB_8254_B_2_CLK_SRC_PREV);
				}

				break;

			case ME_REF_CTR_PREVIOUS:
				if (instance->ctr_idx == 0)
				{
					PERROR("Invalid reference. ME_REF_CTR_PREVIOUS not available for first counter.\n");
					err = ME_ERRNO_INVALID_SINGLE_CONFIG;
					break;
				}
				if (instance->me8254_idx == 0)
				{
					if (instance->ctr_idx == 1)
					{
						clk_src |= (ME1400AB_8254_A_1_CLK_SRC_PREV);
					}
					if (instance->ctr_idx == 2)
					{
						clk_src |= (ME1400AB_8254_A_2_CLK_SRC_PREV);
					}
				}
				else
				{
					if (instance->ctr_idx == 1)
					{
						clk_src |= (ME1400AB_8254_B_1_CLK_SRC_PREV);
					}
					if (instance->ctr_idx == 2)
					{
						clk_src |= (ME1400AB_8254_B_2_CLK_SRC_PREV);
					}
				}
				break;

			case ME_REF_CTR_INTERNAL_1MHZ:
				if (instance->ctr_idx)
				{
					PERROR("Invalid reference. ME_REF_CTR_INTERNAL_1MHZ available only for first counter.\n");
					err = ME_ERRNO_INVALID_SINGLE_CONFIG;
					break;
				}
				if (instance->me8254_idx == 0)
				{
					clk_src |= (ME1400AB_8254_A_0_CLK_SRC_QUARZ);
					clk_src &= ~(ME1400AB_8254_A_0_CLK_SRC_1MHZ);
				}
				else
				{
					clk_src |= (ME1400AB_8254_B_0_CLK_SRC_QUARZ);
					clk_src &= ~(ME1400AB_8254_B_0_CLK_SRC_1MHZ);
				}

				break;

			case ME_REF_CTR_INTERNAL_10MHZ:
				if (instance->ctr_idx)
				{
					PERROR("Invalid reference. ME_REF_CTR_INTERNAL_10MHZ available only for first counter.\n");
					err = ME_ERRNO_INVALID_SINGLE_CONFIG;
					break;
				}
				if (instance->me8254_idx == 0)
				{
					clk_src |= (ME1400AB_8254_A_0_CLK_SRC_QUARZ);
					clk_src |= (ME1400AB_8254_A_0_CLK_SRC_10MHZ);
				}
				else
				{
					clk_src |= (ME1400AB_8254_A_0_CLK_SRC_QUARZ);
					clk_src |= (ME1400AB_8254_A_0_CLK_SRC_10MHZ);
				}

				break;

			default:
				PERROR("Invalid reference.\n");
				err = ME_ERRNO_INVALID_REF;
		}
		if (!err)
		{
			me_writeb(instance->base.dev, clk_src, instance->clk_src_reg);
		}
	ME_SPIN_UNLOCK(instance->clk_src_reg_lock);

	return err;
}

static int me1400_cd_ref_config(me8254_subdevice_t* instance, int ref)
{
	uint8_t clk_src;
	int err = ME_ERRNO_SUCCESS;

	ME_SPIN_LOCK(instance->clk_src_reg_lock);
		me_readb(instance->base.dev, &clk_src, instance->clk_src_reg);
		switch (ref)
		{
			case ME_REF_CTR_EXTERNAL:
				switch (instance->me8254_idx)
				{
					case 0:
					case 2:
					case 4:
					case 6:
					case 8:
						if (instance->ctr_idx == 0)
							clk_src &= ~(ME1400CD_8254_ACE_0_CLK_SRC_MASK);
						else if (instance->ctr_idx == 1)
							clk_src &= ~(ME1400CD_8254_ACE_1_CLK_SRC_MASK);
						else
							clk_src &= ~(ME1400CD_8254_ACE_2_CLK_SRC_MASK);
						break;

					default:
						if (instance->ctr_idx == 0)
							clk_src &= ~(ME1400CD_8254_BD_0_CLK_SRC_MASK);
						else if (instance->ctr_idx == 1)
							clk_src &= ~(ME1400CD_8254_BD_1_CLK_SRC_MASK);
						else
							clk_src &= ~(ME1400CD_8254_BD_2_CLK_SRC_MASK);
						break;
				}
				break;

			case ME_REF_CTR_PREVIOUS:
				switch (instance->me8254_idx)
				{
					case 0:
					case 2:
					case 4:
					case 6:
					case 8:
						if (instance->ctr_idx == 0)
						{
							clk_src &= ~(ME1400CD_8254_ACE_0_CLK_SRC_MASK);
							clk_src |= (ME1400CD_8254_ACE_0_CLK_SRC_PREV);
						}
						else if (instance->ctr_idx == 1)
						{
							clk_src &= ~(ME1400CD_8254_ACE_1_CLK_SRC_MASK);
							clk_src |= (ME1400CD_8254_ACE_1_CLK_SRC_PREV);
						}
						else
						{
							clk_src &= ~(ME1400CD_8254_ACE_2_CLK_SRC_MASK);
							clk_src |= (ME1400CD_8254_ACE_2_CLK_SRC_PREV);
						}
						break;

					default:
						if (instance->ctr_idx == 0)
						{
							clk_src &= ~(ME1400CD_8254_BD_0_CLK_SRC_MASK);
							clk_src |= (ME1400CD_8254_BD_0_CLK_SRC_PREV);
						}
						else if (instance->ctr_idx == 1)
						{
							clk_src &= ~(ME1400CD_8254_BD_1_CLK_SRC_MASK);
							clk_src |= (ME1400CD_8254_BD_1_CLK_SRC_PREV);
						}
						else
						{
							clk_src &= ~(ME1400CD_8254_BD_2_CLK_SRC_MASK);
							clk_src |= (ME1400CD_8254_BD_2_CLK_SRC_PREV);
						}
				}
				break;

			case ME_REF_CTR_INTERNAL_1MHZ:
				if (instance->ctr_idx)
				{
					PERROR("Invalid reference. ME_REF_CTR_INTERNAL_1MHZ available only for first counter.\n");
					err = ME_ERRNO_INVALID_SINGLE_CONFIG;
					break;
				}
				switch (instance->me8254_idx)
				{
					case 0:
					case 2:
					case 4:
					case 6:
					case 8:
						clk_src &= ~(ME1400CD_8254_ACE_0_CLK_SRC_MASK);
						clk_src |= (ME1400CD_8254_ACE_0_CLK_SRC_1MHZ);
						break;

					default:
						clk_src &= ~(ME1400CD_8254_BD_0_CLK_SRC_MASK);
						clk_src |= (ME1400CD_8254_BD_0_CLK_SRC_1MHZ);
				}

				break;

			case ME_REF_CTR_INTERNAL_10MHZ:
				if (instance->ctr_idx)
				{
					PERROR("Invalid reference. ME_REF_CTR_INTERNAL_10MHZ available only for first counter.\n");
					err = ME_ERRNO_INVALID_SINGLE_CONFIG;
					break;
				}
				switch (instance->me8254_idx)
				{
					case 0:
					case 2:
					case 4:
					case 6:
					case 8:
						clk_src &= ~(ME1400CD_8254_ACE_0_CLK_SRC_MASK);
						clk_src |= (ME1400CD_8254_ACE_0_CLK_SRC_10MHZ);
						break;

					default:
						clk_src &= ~(ME1400CD_8254_BD_0_CLK_SRC_MASK);
						clk_src |= (ME1400CD_8254_BD_0_CLK_SRC_10MHZ);
				}
				break;

			default:
				PERROR("Invalid reference.\n");
				err = ME_ERRNO_INVALID_REF;
		}
		if (!err)
		{
			me_writeb(instance->base.dev, clk_src, instance->clk_src_reg);
		}
	ME_SPIN_UNLOCK(instance->clk_src_reg_lock);

	return err;
}

static int me_ref_config(me8254_subdevice_t* instance, int ref)
{
	int err;

	switch (ref)
	{

		case ME_REF_CTR_EXTERNAL:
			err = ME_ERRNO_SUCCESS;
			// Nothing to do
			break;

		default:
			PERROR("Invalid reference. Must be ME_REF_CTR_EXTERNAL.\n");
			err = ME_ERRNO_INVALID_REF;
	}

	return err;
}

int me8254_io_single_config(struct me_subdevice* subdevice, struct file* filep, int channel,
									int single_config, int ref, int trig_chain,  int trig_type, int trig_edge, int flags)
{
	me8254_subdevice_t* instance;
	uint8_t ctrl = ME8254_CTRL_LM | ME8254_CTRL_BIN;
	int err;

	instance = (me8254_subdevice_t *) subdevice;

	PDEBUG("executed.\n");

	if (flags)
	{
		PERROR("Invalid flags. Should be ME_IO_SINGLE_CONFIG_NO_FLAGS.\n");
		return ME_ERRNO_INVALID_FLAGS;
	}

	if (trig_edge)
	{
		PERROR("Invalid trigger edge. Must be ME_TRIG_EDGE_NONE.\n");
		return ME_ERRNO_INVALID_TRIG_EDGE;
	}

	switch (trig_type)
	{
		case ME_TRIG_TYPE_NONE:
			if (trig_chain != ME_TRIG_CHAN_NONE)
			{
				PERROR("Invalid trigger chain specified. Must be ME_TRIG_CHAN_NONE.\n");
				return ME_ERRNO_INVALID_TRIG_CHAN;
			}
			break;
		case ME_TRIG_TYPE_SW:
			if (trig_chain != ME_TRIG_CHAN_DEFAULT)
			{
				PERROR("Invalid trigger chain specified. Must be ME_TRIG_CHAN_DEFAULT.\n");
				return ME_ERRNO_INVALID_TRIG_CHAN;
			}
			break;

		default:
			PERROR("Invalid trigger type.\n");
			return ME_ERRNO_INVALID_TRIG_TYPE;
	}

	switch (single_config)
	{
		case ME_SINGLE_CONFIG_CTR_8254_MODE_DISABLE:
			return me8254_io_reset_subdevice(subdevice, filep, flags);
			break;

		case ME_SINGLE_CONFIG_CTR_8254_MODE_INTERRUPT_ON_TERMINAL_COUNT:
		case ME_SINGLE_CONFIG_CTR_8254_MODE_ONE_SHOT:
		case ME_SINGLE_CONFIG_CTR_8254_MODE_RATE_GENERATOR:
		case ME_SINGLE_CONFIG_CTR_8254_MODE_SQUARE_WAVE:
		case ME_SINGLE_CONFIG_CTR_8254_MODE_SOFTWARE_TRIGGER:
		case ME_SINGLE_CONFIG_CTR_8254_MODE_HARDWARE_TRIGGER:
			break;

		default:
			PERROR("Invalid single configuration.\n");
			return ME_ERRNO_INVALID_SINGLE_CONFIG;
	}

	if (channel)
	{
		PERROR("Invalid channel number. Must be 0.\n");
		return ME_ERRNO_INVALID_CHANNEL;
	}

	ME_SUBDEVICE_ENTER;
		ME_SUBDEVICE_LOCK;
			// Configure the counter modes
			switch (instance->ctr_idx)
			{
				case 0:
					ctrl |= ME8254_CTRL_SC0;
					break;

				case 1:
					ctrl |= ME8254_CTRL_SC1;
					break;

				default:
					ctrl |= ME8254_CTRL_SC2;
			}

			switch (single_config)
			{
				case ME_SINGLE_CONFIG_CTR_8254_MODE_INTERRUPT_ON_TERMINAL_COUNT:
					ctrl |= ME8254_CTRL_M0;
					break;

				case ME_SINGLE_CONFIG_CTR_8254_MODE_ONE_SHOT:
					ctrl |= ME8254_CTRL_M1;
					break;

				case ME_SINGLE_CONFIG_CTR_8254_MODE_RATE_GENERATOR:
					ctrl |= ME8254_CTRL_M2;
					break;

				case ME_SINGLE_CONFIG_CTR_8254_MODE_SQUARE_WAVE:
					ctrl |= ME8254_CTRL_M3;
					break;

				case ME_SINGLE_CONFIG_CTR_8254_MODE_SOFTWARE_TRIGGER:
					ctrl |= ME8254_CTRL_M4;
					break;

				case ME_SINGLE_CONFIG_CTR_8254_MODE_HARDWARE_TRIGGER:
					ctrl |= ME8254_CTRL_M5;
					break;
			}
			me_writeb(instance->base.dev, ctrl, instance->ctrl_reg);

/** @todo Instead of ID related calls features flags should be used.
*/
			switch (instance->device_id)
			{
				case PCI_DEVICE_ID_MEILHAUS_ME1400:
				case PCI_DEVICE_ID_MEILHAUS_ME14E0:
				case PCI_DEVICE_ID_MEILHAUS_ME140A:
				case PCI_DEVICE_ID_MEILHAUS_ME14EA:
				case PCI_DEVICE_ID_MEILHAUS_ME140B:
				case PCI_DEVICE_ID_MEILHAUS_ME14EB:
					err = me1400_ab_ref_config(instance, ref);
					break;

				case PCI_DEVICE_ID_MEILHAUS_ME140C:
				case PCI_DEVICE_ID_MEILHAUS_ME140D:
					err = me1400_cd_ref_config(instance, ref);
					break;

				case PCI_DEVICE_ID_MEILHAUS_ME4610:
				case PCI_DEVICE_ID_MEILHAUS_ME4660:
				case PCI_DEVICE_ID_MEILHAUS_ME4660I:
				case PCI_DEVICE_ID_MEILHAUS_ME4660S:
				case PCI_DEVICE_ID_MEILHAUS_ME4660IS:
				case PCI_DEVICE_ID_MEILHAUS_ME4670:
				case PCI_DEVICE_ID_MEILHAUS_ME4670I:
				case PCI_DEVICE_ID_MEILHAUS_ME4670S:
				case PCI_DEVICE_ID_MEILHAUS_ME4670IS:
				case PCI_DEVICE_ID_MEILHAUS_ME4680:
				case PCI_DEVICE_ID_MEILHAUS_ME4680I:
				case PCI_DEVICE_ID_MEILHAUS_ME4680S:
				case PCI_DEVICE_ID_MEILHAUS_ME4680IS:
				case PCI_DEVICE_ID_MEILHAUS_ME8100_A:
				case PCI_DEVICE_ID_MEILHAUS_ME8100_B:
					err = me_ref_config(instance, ref);
					break;

				default:
					PERROR_CRITICAL("Invalid device type. 8254 registred for %x.\n", instance->device_id);
					err =  ME_ERRNO_INVALID_SINGLE_CONFIG;
			}
		ME_SUBDEVICE_UNLOCK;
	ME_SUBDEVICE_EXIT;

	return err;
}

int me8254_io_single_read(struct me_subdevice* subdevice, struct file* filep, int channel, int* value, int time_out, int flags)
{
	me8254_subdevice_t* instance;
	uint8_t ctrl = ME8254_CTRL_TLO;
	uint8_t lo_byte = 0;
	uint8_t hi_byte = 0;
	uint8_t status_byte = 0;
	int err = ME_ERRNO_SUCCESS;

	PDEBUG("executed.\n");

	if (flags)
	{
		PERROR("Invalid flags specified. Must be ME_IO_SINGLE_TYPE_NO_FLAGS.\n");
		return ME_ERRNO_INVALID_FLAGS;
	}

	if (channel)
	{
		PERROR("Invalid channel. Must be 0.\n");
		return ME_ERRNO_INVALID_CHANNEL;
	}

	instance = (me8254_subdevice_t *) subdevice;

	ME_SUBDEVICE_ENTER;
		ME_SUBDEVICE_LOCK;
			ME_SPIN_LOCK(instance->ctrl_reg_lock);
				switch (instance->ctr_idx)
				{
					case 0:
						ctrl |= ME8254_CTRL_SC0;
						break;

					case 1:
						ctrl |= ME8254_CTRL_SC1;
						break;

					default:
						ctrl |= ME8254_CTRL_SC2;
				}
				me_writeb(instance->base.dev, ME8254_STATUS_CMD | (0x2 << instance->ctr_idx), instance->ctrl_reg);
				me_readb(instance->base.dev, &status_byte, instance->val_reg);
				if (status_byte & 0x40)
				{
					PINFO("NULL count detected.\n");
					err = ME_ERRNO_SUBDEVICE_NOT_RUNNING;
					goto EXIT;
				}

				me_writeb(instance->base.dev, ctrl, instance->ctrl_reg);

				me_readb(instance->base.dev, &lo_byte, instance->val_reg);
				me_readb(instance->base.dev, &hi_byte, instance->val_reg);
EXIT:
			ME_SPIN_UNLOCK(instance->ctrl_reg_lock);

			*value = (int)lo_byte | ((int)hi_byte << 8);
		ME_SUBDEVICE_UNLOCK;
	ME_SUBDEVICE_EXIT;

	return err;
}

int me8254_io_single_write(struct me_subdevice* subdevice, struct file* filep, int channel,int value, int time_out, int flags)
{
	me8254_subdevice_t* instance;

	PDEBUG("executed.\n");

	if (flags)
	{
		PERROR("Invalid flags specified. Must be ME_IO_SINGLE_TYPE_NO_FLAGS.\n");
		return ME_ERRNO_INVALID_FLAGS;
	}

	if (channel)
	{
		PERROR("Invalid channel. Must be 0.\n");
		return ME_ERRNO_INVALID_CHANNEL;
	}

	instance = (me8254_subdevice_t *) subdevice;

	ME_SUBDEVICE_ENTER;
		ME_SUBDEVICE_LOCK;
			me_writeb(instance->base.dev, value, instance->val_reg);
			me_writeb(instance->base.dev, (value >> 8), instance->val_reg);
		ME_SUBDEVICE_UNLOCK;
	ME_SUBDEVICE_EXIT;

	return ME_ERRNO_SUCCESS;
}

int me8254_query_number_channels(struct me_subdevice* subdevice, int* number)
{
	PDEBUG("executed.\n");

	*number = ME8254_NUMBER_CHANNELS;
	return ME_ERRNO_SUCCESS;
}

int me8254_query_subdevice_type(struct me_subdevice* subdevice, int* type, int* subtype)
{
	PDEBUG("executed.\n");

	*type = ME_TYPE_CTR;
	*subtype = ME_SUBTYPE_CTR_8254;
	return ME_ERRNO_SUCCESS;
}

int me8254_query_subdevice_caps(struct me_subdevice* subdevice, int* caps)
{
	me8254_subdevice_t* instance;
	PDEBUG("executed.\n");

	instance = (me8254_subdevice_t *) subdevice;

	*caps = instance->caps;
	return ME_ERRNO_SUCCESS;
}

int me8254_query_subdevice_caps_args(struct me_subdevice* subdevice, int cap, int* args, int* count)
{
	int err = ME_ERRNO_SUCCESS;

	PDEBUG("executed.\n");

	if (*count < 1)
	{
		PERROR("Invalid capability argument count. Should be at least 1.\n");
		return ME_ERRNO_INVALID_CAP_ARG_COUNT;
	}

	if (cap == ME_CAP_CTR_WIDTH)
	{
		*count = 1;
		*args = ME8254_CTR_WIDTH;
	}
	else
	{
		*count = 0;
		*args = 0;
		err = ME_ERRNO_INVALID_CAP;
	}

	return err;
}

static void* me1400AB_get_val_reg(void* reg_base, unsigned int me8254_idx, unsigned int ctr_idx)
{
	switch (me8254_idx)
	{

		case 0:
			return (reg_base + ME1400AB_8254_A_0_VAL_REG + ctr_idx);

		default:
			return (reg_base + ME1400AB_8254_B_0_VAL_REG + ctr_idx);
	}

	return NULL;
}

static void* me1400AB_get_ctrl_reg(void* reg_base, unsigned int me8254_idx, unsigned int ctr_idx)
{
	switch (me8254_idx)
	{
		case 0:
			return (reg_base + ME1400AB_8254_A_CTRL_REG);

		default:
			return (reg_base + ME1400AB_8254_B_CTRL_REG);
	}

	return NULL;
}

static void* me1400AB_get_clk_src_reg(void* reg_base, unsigned int me8254_idx, unsigned int ctr_idx)
{
	switch (me8254_idx)
	{
		case 0:
			return (reg_base + ME1400AB_CLK_SRC_REG);

		default:
			return (reg_base + ME1400AB_CLK_SRC_REG);
	}

	return NULL;
}

static void* me1400CD_get_val_reg(void* reg_base, unsigned int me8254_idx, unsigned int ctr_idx)
{
	switch (me8254_idx)
	{
		case 0:
			return (reg_base + ME1400C_8254_A_0_VAL_REG + ctr_idx);

		case 1:
			return (reg_base + ME1400C_8254_B_0_VAL_REG + ctr_idx);

		case 2:
			return (reg_base + ME1400C_8254_C_0_VAL_REG + ctr_idx);

		case 3:
			return (reg_base + ME1400C_8254_D_0_VAL_REG + ctr_idx);

		case 4:
			return (reg_base + ME1400C_8254_E_0_VAL_REG + ctr_idx);

		case 5:
			return (reg_base + ME1400D_8254_A_0_VAL_REG + ctr_idx);

		case 6:
			return (reg_base + ME1400D_8254_B_0_VAL_REG + ctr_idx);

		case 7:
			return (reg_base + ME1400D_8254_C_0_VAL_REG + ctr_idx);

		case 8:
			return (reg_base + ME1400D_8254_D_0_VAL_REG + ctr_idx);

		default:
			return (reg_base + ME1400D_8254_E_0_VAL_REG + ctr_idx);
	}

	return NULL;
}

static void* me1400CD_get_ctrl_reg(void* reg_base, unsigned int me8254_idx, unsigned int ctr_idx)
{
	switch (me8254_idx)
	{
		case 0:
			return (reg_base + ME1400C_8254_A_CTRL_REG);

		case 1:
			return (reg_base + ME1400C_8254_B_CTRL_REG);

		case 2:
			return (reg_base + ME1400C_8254_C_CTRL_REG);

		case 3:
			return (reg_base + ME1400C_8254_D_CTRL_REG);

		case 4:
			return (reg_base + ME1400C_8254_E_CTRL_REG);

		case 5:
			return (reg_base + ME1400D_8254_A_CTRL_REG);

		case 6:
			return (reg_base + ME1400D_8254_B_CTRL_REG);

		case 7:
			return (reg_base + ME1400D_8254_C_CTRL_REG);

		case 8:
			return (reg_base + ME1400D_8254_D_CTRL_REG);

		default:
			return (reg_base + ME1400D_8254_E_CTRL_REG);
	}

	return NULL;
}

static void* me1400CD_get_clk_src_reg(void* reg_base, unsigned int me8254_idx, unsigned int ctr_idx)
{
	switch (me8254_idx)
	{
		case 0:
			return (reg_base + ME1400C_CLK_SRC_0_REG);

		case 1:
			return (reg_base + ME1400C_CLK_SRC_0_REG);

		case 2:
			return (reg_base + ME1400C_CLK_SRC_1_REG);

		case 3:
			return (reg_base + ME1400C_CLK_SRC_1_REG);

		case 4:
			return (reg_base + ME1400C_CLK_SRC_2_REG);

		case 5:
			return (reg_base + ME1400D_CLK_SRC_0_REG);

		case 6:
			return (reg_base + ME1400D_CLK_SRC_0_REG);

		case 7:
			return (reg_base + ME1400D_CLK_SRC_1_REG);

		case 8:
			return (reg_base + ME1400D_CLK_SRC_1_REG);

		default:
			return (reg_base + ME1400D_CLK_SRC_2_REG);
	}

	return NULL;
}

static void* me4600_get_val_reg(void* reg_base, unsigned int me8254_idx, unsigned int ctr_idx)
{
	return (reg_base + ME4600_8254_0_VAL_REG + ctr_idx);
}

static void* me4600_get_ctrl_reg(void* reg_base, unsigned int me8254_idx, unsigned int ctr_idx)
{
	return (reg_base + ME4600_8254_CTRL_REG);
}

static void* me8100_get_val_reg(void* reg_base, unsigned int me8254_idx, unsigned int ctr_idx)
{// 16 bits
	return (reg_base + ME8100_COUNTER_REG_0 + ctr_idx * 2);
}

static void* me8100_get_ctrl_reg(void* reg_base, unsigned int me8254_idx, unsigned int ctr_idx)
{
	return (reg_base + ME8100_COUNTER_CTRL_REG);
}

me8254_subdevice_t* me8254_constr(uint16_t device_id, void* reg_base, unsigned int me8254_idx, unsigned int ctr_idx,
										me_lock_t* ctrl_reg_lock, me_lock_t* clk_src_reg_lock)
{
	me8254_subdevice_t *subdevice;

	PDEBUG("executed.\n");

/** @todo Checkings should be removed. We can safetly assume that data passed from upper level are correct.
*/
	// Check if counter index is out of range
	if (ctr_idx > 2)
	{
		PERROR("Counter index is out of range.\n");
		return NULL;
	}

	// Check device specific values.
	switch (device_id)
	{
		case PCI_DEVICE_ID_MEILHAUS_ME140A:
		case PCI_DEVICE_ID_MEILHAUS_ME14EA:

		case PCI_DEVICE_ID_MEILHAUS_ME4610:
		case PCI_DEVICE_ID_MEILHAUS_ME4660:
		case PCI_DEVICE_ID_MEILHAUS_ME4660I:
		case PCI_DEVICE_ID_MEILHAUS_ME4660S:
		case PCI_DEVICE_ID_MEILHAUS_ME4660IS:
		case PCI_DEVICE_ID_MEILHAUS_ME4670:
		case PCI_DEVICE_ID_MEILHAUS_ME4670I:
		case PCI_DEVICE_ID_MEILHAUS_ME4670S:
		case PCI_DEVICE_ID_MEILHAUS_ME4670IS:
		case PCI_DEVICE_ID_MEILHAUS_ME4680:
		case PCI_DEVICE_ID_MEILHAUS_ME4680I:
		case PCI_DEVICE_ID_MEILHAUS_ME4680S:
		case PCI_DEVICE_ID_MEILHAUS_ME4680IS:

		case PCI_DEVICE_ID_MEILHAUS_ME4560:
		case PCI_DEVICE_ID_MEILHAUS_ME4560I:
		case PCI_DEVICE_ID_MEILHAUS_ME4560S:
		case PCI_DEVICE_ID_MEILHAUS_ME4560IS:
		case PCI_DEVICE_ID_MEILHAUS_ME4570:
		case PCI_DEVICE_ID_MEILHAUS_ME4570I:
		case PCI_DEVICE_ID_MEILHAUS_ME4570S:
		case PCI_DEVICE_ID_MEILHAUS_ME4570IS:

		case PCI_DEVICE_ID_MEILHAUS_ME4760:
		case PCI_DEVICE_ID_MEILHAUS_ME4760I:
		case PCI_DEVICE_ID_MEILHAUS_ME4760S:
		case PCI_DEVICE_ID_MEILHAUS_ME4760IS:
		case PCI_DEVICE_ID_MEILHAUS_ME4770:
		case PCI_DEVICE_ID_MEILHAUS_ME4770I:
		case PCI_DEVICE_ID_MEILHAUS_ME4770S:
		case PCI_DEVICE_ID_MEILHAUS_ME4770IS:
		case PCI_DEVICE_ID_MEILHAUS_ME4780:
		case PCI_DEVICE_ID_MEILHAUS_ME4780I:
		case PCI_DEVICE_ID_MEILHAUS_ME4780S:
		case PCI_DEVICE_ID_MEILHAUS_ME4780IS:

		case PCI_DEVICE_ID_MEILHAUS_ME4860:
		case PCI_DEVICE_ID_MEILHAUS_ME4860I:
		case PCI_DEVICE_ID_MEILHAUS_ME4860S:
		case PCI_DEVICE_ID_MEILHAUS_ME4860IS:
		case PCI_DEVICE_ID_MEILHAUS_ME4870:
		case PCI_DEVICE_ID_MEILHAUS_ME4870I:
		case PCI_DEVICE_ID_MEILHAUS_ME4870S:
		case PCI_DEVICE_ID_MEILHAUS_ME4870IS:
		case PCI_DEVICE_ID_MEILHAUS_ME4880:
		case PCI_DEVICE_ID_MEILHAUS_ME4880I:
		case PCI_DEVICE_ID_MEILHAUS_ME4880S:
		case PCI_DEVICE_ID_MEILHAUS_ME4880IS:

		case PCI_DEVICE_ID_MEILHAUS_ME0752:
		case PCI_DEVICE_ID_MEILHAUS_ME0754:
		case PCI_DEVICE_ID_MEILHAUS_ME0762:
		case PCI_DEVICE_ID_MEILHAUS_ME0764:
		case PCI_DEVICE_ID_MEILHAUS_ME0772:
		case PCI_DEVICE_ID_MEILHAUS_ME0774:
		case PCI_DEVICE_ID_MEILHAUS_ME0782:
		case PCI_DEVICE_ID_MEILHAUS_ME0784:

		case PCI_DEVICE_ID_MEILHAUS_ME8100_A:
		case PCI_DEVICE_ID_MEILHAUS_ME8100_B:
			if (me8254_idx > 0)
			{
				PERROR("8254 index is out of range. Must be 0.\n");
				return NULL;
			}
			break;

		case PCI_DEVICE_ID_MEILHAUS_ME140B:
		case PCI_DEVICE_ID_MEILHAUS_ME14EB:
			if (me8254_idx > 1)
			{
				PERROR("8254 index is out of range. Must be 0 or 1.\n");
				return NULL;
			}
			break;

		case PCI_DEVICE_ID_MEILHAUS_ME140C:
			if (me8254_idx > 4)
			{
				PERROR("8254 index is out of range. Must be between 0 and 3.\n");
				return NULL;
			}

		case PCI_DEVICE_ID_MEILHAUS_ME140D:
			if (me8254_idx > 9)
			{
				PERROR("8254 index is out of range. Must be between 0 and 8.\n");
				return NULL;
			}
			break;

		default:
			PERROR_CRITICAL("8254 registred for %x!\n", device_id);
			return NULL;
	}

	// Allocate memory for subdevice instance
	subdevice = kzalloc(sizeof(me8254_subdevice_t), GFP_KERNEL);
	if (!subdevice)
	{
		PERROR("Cannot get memory for 8254 instance.\n");
		return NULL;
	}

	// Initialize subdevice base class
	if (me_subdevice_init(&subdevice->base))
	{
		PERROR("Cannot initialize subdevice base class instance.\n");
		kfree(subdevice);
		return NULL;
	}

	// Initialize spin locks.
	subdevice->ctrl_reg_lock = ctrl_reg_lock;
	subdevice->clk_src_reg_lock = clk_src_reg_lock;

	// Save type of Meilhaus device
	subdevice->device_id = device_id;

	// Save the subdevice indexes.
	subdevice->me8254_idx = me8254_idx;
	subdevice->ctr_idx = ctr_idx;
	subdevice->base.idx = ctr_idx + (me8254_idx * 3);

/** @todo Setting flags for particular implementations create portibility problem.
	Flags should be passed from upper level.
	Also registers should be some how provided by upper level call.
*/
	// Do device specific initialization
	switch (device_id)
	{
		case PCI_DEVICE_ID_MEILHAUS_ME140A:
		case PCI_DEVICE_ID_MEILHAUS_ME14EA:
		case PCI_DEVICE_ID_MEILHAUS_ME140B:
		case PCI_DEVICE_ID_MEILHAUS_ME14EB:
			// Initialize the counters capabilities
			subdevice->caps = ME_CAPS_CTR_CLK_EXTERNAL;
			if (ctr_idx == 0)
				subdevice->caps |= ME_CAPS_CTR_CLK_INTERNAL_1MHZ | ME_CAPS_CTR_CLK_INTERNAL_10MHZ;
			else
				subdevice->caps |= ME_CAPS_CTR_CLK_PREVIOUS;

			// Get the counters registers
			subdevice->val_reg = me1400AB_get_val_reg(reg_base, me8254_idx, ctr_idx);
			subdevice->ctrl_reg = me1400AB_get_ctrl_reg(reg_base, me8254_idx, ctr_idx);
			subdevice->clk_src_reg = me1400AB_get_clk_src_reg(reg_base, me8254_idx, ctr_idx);
			break;

		case PCI_DEVICE_ID_MEILHAUS_ME140C:
		case PCI_DEVICE_ID_MEILHAUS_ME140D:
			// Initialize the counters capabilities
			subdevice->caps = ME_CAPS_CTR_CLK_EXTERNAL | ME_CAPS_CTR_CLK_PREVIOUS;
			if (ctr_idx == 0)
			{
				subdevice->caps |= ME_CAPS_CTR_CLK_INTERNAL_1MHZ | ME_CAPS_CTR_CLK_INTERNAL_10MHZ;
				if ((me8254_idx == 0) || (me8254_idx == 5))
				{// No cascading for first counter on first chips.
					subdevice->caps &= ~ME_CAPS_CTR_CLK_PREVIOUS;
				}
			}

			// Get the counters registers
			subdevice->val_reg = me1400CD_get_val_reg(reg_base, me8254_idx, ctr_idx);
			subdevice->ctrl_reg = me1400CD_get_ctrl_reg(reg_base, me8254_idx, ctr_idx);
			subdevice->clk_src_reg = me1400CD_get_clk_src_reg(reg_base, me8254_idx, ctr_idx);
			break;

		case PCI_DEVICE_ID_MEILHAUS_ME4610:
		case PCI_DEVICE_ID_MEILHAUS_ME4660:
		case PCI_DEVICE_ID_MEILHAUS_ME4660I:
		case PCI_DEVICE_ID_MEILHAUS_ME4660S:
		case PCI_DEVICE_ID_MEILHAUS_ME4660IS:
		case PCI_DEVICE_ID_MEILHAUS_ME4670:
		case PCI_DEVICE_ID_MEILHAUS_ME4670I:
		case PCI_DEVICE_ID_MEILHAUS_ME4670S:
		case PCI_DEVICE_ID_MEILHAUS_ME4670IS:
		case PCI_DEVICE_ID_MEILHAUS_ME4680:
		case PCI_DEVICE_ID_MEILHAUS_ME4680I:
		case PCI_DEVICE_ID_MEILHAUS_ME4680S:
		case PCI_DEVICE_ID_MEILHAUS_ME4680IS:

		case PCI_DEVICE_ID_MEILHAUS_ME4560:
		case PCI_DEVICE_ID_MEILHAUS_ME4560I:
		case PCI_DEVICE_ID_MEILHAUS_ME4560S:
		case PCI_DEVICE_ID_MEILHAUS_ME4560IS:
		case PCI_DEVICE_ID_MEILHAUS_ME4570:
		case PCI_DEVICE_ID_MEILHAUS_ME4570I:
		case PCI_DEVICE_ID_MEILHAUS_ME4570S:
		case PCI_DEVICE_ID_MEILHAUS_ME4570IS:

		case PCI_DEVICE_ID_MEILHAUS_ME4760:
		case PCI_DEVICE_ID_MEILHAUS_ME4760I:
		case PCI_DEVICE_ID_MEILHAUS_ME4760S:
		case PCI_DEVICE_ID_MEILHAUS_ME4760IS:
		case PCI_DEVICE_ID_MEILHAUS_ME4770:
		case PCI_DEVICE_ID_MEILHAUS_ME4770I:
		case PCI_DEVICE_ID_MEILHAUS_ME4770S:
		case PCI_DEVICE_ID_MEILHAUS_ME4770IS:
		case PCI_DEVICE_ID_MEILHAUS_ME4780:
		case PCI_DEVICE_ID_MEILHAUS_ME4780I:
		case PCI_DEVICE_ID_MEILHAUS_ME4780S:
		case PCI_DEVICE_ID_MEILHAUS_ME4780IS:

		case PCI_DEVICE_ID_MEILHAUS_ME4860:
		case PCI_DEVICE_ID_MEILHAUS_ME4860I:
		case PCI_DEVICE_ID_MEILHAUS_ME4860S:
		case PCI_DEVICE_ID_MEILHAUS_ME4860IS:
		case PCI_DEVICE_ID_MEILHAUS_ME4870:
		case PCI_DEVICE_ID_MEILHAUS_ME4870I:
		case PCI_DEVICE_ID_MEILHAUS_ME4870S:
		case PCI_DEVICE_ID_MEILHAUS_ME4870IS:
		case PCI_DEVICE_ID_MEILHAUS_ME4880:
		case PCI_DEVICE_ID_MEILHAUS_ME4880I:
		case PCI_DEVICE_ID_MEILHAUS_ME4880S:
		case PCI_DEVICE_ID_MEILHAUS_ME4880IS:

		case PCI_DEVICE_ID_MEILHAUS_ME0752:
		case PCI_DEVICE_ID_MEILHAUS_ME0754:
		case PCI_DEVICE_ID_MEILHAUS_ME0762:
		case PCI_DEVICE_ID_MEILHAUS_ME0764:
		case PCI_DEVICE_ID_MEILHAUS_ME0772:
		case PCI_DEVICE_ID_MEILHAUS_ME0774:
		case PCI_DEVICE_ID_MEILHAUS_ME0782:
		case PCI_DEVICE_ID_MEILHAUS_ME0784:

			// Initialize the counters capabilities
			subdevice->caps = ME_CAPS_CTR_CLK_EXTERNAL;

			// Get the counters registers
			subdevice->val_reg = me4600_get_val_reg(reg_base, me8254_idx, ctr_idx);
			subdevice->ctrl_reg = me4600_get_ctrl_reg(reg_base, me8254_idx, ctr_idx);
			subdevice->clk_src_reg = 0; // Not used
			break;

		case PCI_DEVICE_ID_MEILHAUS_ME8100_A:
		case PCI_DEVICE_ID_MEILHAUS_ME8100_B:
			// Initialize the counters capabilities
			subdevice->caps = ME_CAPS_CTR_CLK_EXTERNAL;

			// Get the counters registers
			subdevice->val_reg = me8100_get_val_reg(reg_base, me8254_idx, ctr_idx);
			subdevice->ctrl_reg = me8100_get_ctrl_reg(reg_base, me8254_idx, ctr_idx);
			subdevice->clk_src_reg = 0; // Not used
			break;

		default:
			PERROR("Unknown device type.\n");
			me_subdevice_deinit(&subdevice->base);
			kfree(subdevice);
			return NULL;
	}


	// Overload subdevice base class methods.
	subdevice->base.me_subdevice_io_reset_subdevice = me8254_io_reset_subdevice;
	subdevice->base.me_subdevice_io_single_config = me8254_io_single_config;
	subdevice->base.me_subdevice_io_single_read = me8254_io_single_read;
	subdevice->base.me_subdevice_io_single_write = me8254_io_single_write;
	subdevice->base.me_subdevice_query_number_channels = me8254_query_number_channels;
	subdevice->base.me_subdevice_query_subdevice_type = me8254_query_subdevice_type;
	subdevice->base.me_subdevice_query_subdevice_caps = me8254_query_subdevice_caps;
	subdevice->base.me_subdevice_query_subdevice_caps_args = me8254_query_subdevice_caps_args;

	return subdevice;
}
