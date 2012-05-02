/**
 * @file me4600_ai_CosateQ.c
 *
 * @brief The ME-4600 analog input subdevice instance. Special version for CosateQ.
 * @note Copyright (C) 2007 Meilhaus Electronic GmbH (support@meilhaus.de)
 * @author KG (Krzysztof Gantzke) (k.gantzke@meilhaus.de)
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


# include <linux/delay.h>
# include <linux/sched.h>
# include <linux/workqueue.h>
# include <asm/uaccess.h>
# include <asm/msr.h>

# include "me_debug.h"
# include "me_error.h"
# include "me_defines.h"
# include "mehardware_access.h"
# include "me_spin_lock.h"
# include "meeeprom.h"

# include "me4600_reg.h"
# include "me4600_ai_reg.h"
# include "me4600_ai_CosateQ.h"
# include "medevice.h"

# define me4600_AI_ERROR_TIMEOUT	((HZ<<7)+2)



/// Declarations

static void me4600_ai_destructor(me_subdevice_t* subdevice);
static int me4600_ai_io_reset_subdevice(me_subdevice_t* subdevice, struct file* filep, int flags);

static int me4600_ai_io_single_config_check(me4600_ai_CosateQ_subdevice_t* instance, int channel,
										int single_config, int ref, int trig_chain, int trig_type, int trig_edge, int flags);
static int me4600_ai_io_single_config(me_subdevice_t* subdevice, struct file* filep, int channel,
										int single_config, int ref, int trig_chain, int trig_type, int trig_edge, int flags);

static int me4600_ai_io_single_read_check(me4600_ai_CosateQ_subdevice_t* instance, int channel,  int* value, int time_out, int flags);
static int me4600_ai_io_single_read(me_subdevice_t* subdevice, struct file* filep, int channel, int* value, int time_out, int flags);

static int me4600_ai_io_single_start(me_subdevice_t* subdevice, struct file* filep, int start_mode, int time_out, int flags);

static int me4600_ai_query_range_by_min_max(me_subdevice_t* subdevice, int unit, int* min, int* max, int* maxdata, int* range);
static int me4600_ai_query_number_ranges(me_subdevice_t* subdevice, int unit, int* count);
static int me4600_ai_query_range_info(me_subdevice_t* subdevice, int range, int* unit, int* min, int* max, int* maxdata);
static int me4600_ai_query_number_channels(me_subdevice_t* subdevice, int *number);
static int me4600_ai_query_subdevice_type(me_subdevice_t* subdevice, int* type, int* subtype);
static int me4600_ai_query_subdevice_caps(me_subdevice_t* subdevice, int* caps);
static int me4600_ai_postinit(me_subdevice_t* subdevice, void* args);

static int me4600_ai_irq_handle(me_subdevice_t* subdevice, uint32_t irq_status);

static void ai_mux_toggler(me4600_ai_CosateQ_subdevice_t* instance);

static void ai_read_calibration(me4600_ai_CosateQ_subdevice_t* instance);
static void ai_default_calibration_entry(me_calibration_entry_t* entry);
static void ai_default_calibration(me4600_ai_CosateQ_subdevice_t* instance);
static void ai_calculate_calibration(me4600_ai_CosateQ_subdevice_t* instance, me4600_ai_eeprom_t raw_cal);
static void ai_calculate_calibration_entry(int64_t nominal_A, int64_t actual_A, int64_t nominal_B, int64_t actual_B, me_calibration_entry_t* entry);

static uint16_t inline ai_calculate_end_value(const me_calibration_entry_t calibration, int16_t value);
static uint16_t inline ai_calculate_calibrated_value(me4600_ai_CosateQ_subdevice_t* instance, int entry, int value);


static int ai_reset(me4600_ai_CosateQ_subdevice_t* instance);
static int ai_set_single_config(me4600_ai_CosateQ_subdevice_t* instance, int flags);

static inline void read_from_fifo(me4600_ai_CosateQ_subdevice_t* instance);
static int polling_read(me4600_ai_CosateQ_subdevice_t* instance);

static void me4600_ai_destructor(me_subdevice_t* subdevice)
{
	me4600_ai_CosateQ_subdevice_t* instance;

	instance = (me4600_ai_CosateQ_subdevice_t *) subdevice;

	PDEBUG("executed. idx=0\n");

	if (!instance)
	{
		return;
	}

	if (instance->base.dev && (((me_general_dev_t *)instance->base.dev)->dev))
	{
		// Reset subdevice to asure clean exit.
		me4600_ai_io_reset_subdevice(subdevice, NULL, ME_IO_RESET_SUBDEVICE_NO_FLAGS);
	}

	me_subdevice_deinit(&instance->base);
}

static int me4600_ai_io_reset_subdevice(me_subdevice_t* subdevice, struct file* filep, int flags)
{
	me4600_ai_CosateQ_subdevice_t* instance;
	int i;

	PDEBUG("executed. idx=0\n");

	if (flags)
	{
		PERROR("Invalid flags specified. Must be ME_IO_RESET_SUBDEVICE_NO_FLAGS.\n");
		return ME_ERRNO_INVALID_FLAGS;
	}

	instance = (me4600_ai_CosateQ_subdevice_t *)subdevice;

	ME_SUBDEVICE_ENTER;
		ME_LOCK_PROTECTOR;
			instance->status = ai_status_none;

			ai_reset(instance);

			me_writel(instance->base.dev, ME4600_AI_MIN_CHAN_TICKS-1, instance->chan_timer_reg);
			me_writel(instance->base.dev, ME4600_AI_MIN_ACQ_TICKS-1, instance->chan_pre_timer_reg);
			me_writel(instance->base.dev, 0xFFFFFFFF, instance->scan_timer_low_reg);
			me_writel(instance->base.dev, 0xFFFFFFFF, instance->scan_timer_high_reg);
			me_writel(instance->base.dev, 0, instance->scan_pre_timer_low_reg);
			me_writel(instance->base.dev, 0, instance->scan_pre_timer_high_reg);
			me_writel(instance->base.dev, 0, instance->sample_counter_reg);

			// Initialize the single config entries to reset values.
			for (i = 0; i < instance->channels; i++)
			{
				instance->single_entry[i] = 0x0;
				instance->single_value[i] = 0xFFFFFFFF;
			}
			instance->single_ctrl = 0x0;
			instance->single_status = 0x0;

			instance->raw_values = 0;
		ME_UNLOCK_PROTECTOR;
	ME_SUBDEVICE_EXIT;

	return ME_ERRNO_SUCCESS;
}

static int me4600_ai_io_single_config_check(me4600_ai_CosateQ_subdevice_t* instance, int channel, int single_config, int ref, int trig_chain, int trig_type, int trig_edge, int flags)
{

	PDEBUG("executed. idx=0\n");

	if (flags & ~(ME_IO_SINGLE_CONFIG_NO_FLAGS | ME_IO_SINGLE_CONFIG_CONTINUE | ME_IO_SINGLE_CONFIG_DONT_SET_BACKGROUND | ME_IO_SINGLE_CONFIG_POLLING_MODE))
	{
		PERROR("Invalid flags. Must be ME_IO_SINGLE_CONFIG_NO_FLAGS or ME_IO_SINGLE_CONFIG_CONTINUE.\n");
		return ME_ERRNO_INVALID_FLAGS;
	}

	if ((flags & (ME_IO_SINGLE_CONFIG_DONT_SET_BACKGROUND | ME_IO_SINGLE_CONFIG_POLLING_MODE)) == (ME_IO_SINGLE_CONFIG_DONT_SET_BACKGROUND | ME_IO_SINGLE_CONFIG_POLLING_MODE))
	{
		PERROR("Invalid flags. ME_IO_SINGLE_CONFIG_DONT_SET_BACKGROUND and ME_IO_SINGLE_CONFIG_POLLING_MODE can not be set together.\n");
		return ME_ERRNO_INVALID_FLAGS;
	}

	switch (trig_type)
	{
		case ME_TRIG_TYPE_NONE:
		case ME_TRIG_TYPE_SW:
			if (trig_edge != ME_TRIG_EDGE_NONE)
			{
				PERROR("Invalid trigger edge. Software trigger has not edge. Must be ME_TRIG_EDGE_NONE.\n");
				return ME_ERRNO_INVALID_TRIG_EDGE;
			}
			break;

		case ME_TRIG_TYPE_EXT_ANALOG:
			if (!(instance->features & ME4600_ANALOG_TRIGGER))
			{
				PERROR("Invalid trigger type specified. ANALOG TRIGGER not supported.\n");
				return ME_ERRNO_INVALID_TRIG_TYPE;
			}

		case ME_TRIG_TYPE_EXT_DIGITAL:
			if ((trig_edge != ME_TRIG_EDGE_ANY) && (trig_edge != ME_TRIG_EDGE_RISING) && (trig_edge != ME_TRIG_EDGE_FALLING))
			{
				PERROR("Invalid trigger edge specified. Must be ME_TRIG_EDGE_RISING, ME_TRIG_EDGE_FALLING or ME_TRIG_EDGE_ANY.\n");
				return ME_ERRNO_INVALID_TRIG_EDGE;
			}
			break;

		default:
			PERROR("Invalid trigger type specified.\n");
			return ME_ERRNO_INVALID_TRIG_TYPE;
	}

	if ((trig_type == ME_TRIG_TYPE_NONE) && (trig_chain != ME_TRIG_CHAN_NONE))
	{
		PERROR("Invalid trigger chain specified. Must be ME_TRIG_CHAN_NONE.\n");
		return ME_ERRNO_INVALID_TRIG_CHAN;
	}

	if ((trig_type != ME_TRIG_TYPE_NONE) && (trig_chain != ME_TRIG_CHAN_DEFAULT))
	{
		PERROR("Invalid trigger chain specified. Must be ME_TRIG_CHAN_DEFAULT.\n");
		return ME_ERRNO_INVALID_TRIG_CHAN;
	}

	if ((single_config < 0) || (single_config >= instance->ranges_len))
	{
		PERROR("Invalid range specified. Must be between 0 and %d.\n", instance->ranges_len - 1);
		return ME_ERRNO_INVALID_SINGLE_CONFIG;
	}

	if ((ref != ME_REF_AI_GROUND) && (ref != ME_REF_AI_DIFFERENTIAL))
	{
		PERROR("Invalid reference specified. Must be ME_REF_AI_GROUND or ME_REF_AI_DIFFERENTIAL.\n");
		return ME_ERRNO_INVALID_REF;
	}

	if ((single_config % 2) && (ref != ME_REF_AI_GROUND))
	{
		PERROR("Invalid reference specified. Only ME_REF_AI_GROUND supported by range %d.\n", single_config);
		return ME_ERRNO_INVALID_REF;
	}

	if (ref == ME_REF_AI_DIFFERENTIAL)
	{
		if (!(instance->features & ME4600_DIFFERENTIAL))
		{
			PERROR("Invalid reference specified. ME_REF_AI_DIFFERENTIAL not supported.\n");
			return ME_ERRNO_INVALID_REF;
		}
		if (channel >= (instance->channels / 2))
		{
			PERROR("Invalid channel specified. Only %d channels available in differential mode.\n", instance->channels / 2);
			return ME_ERRNO_INVALID_CHANNEL;
		}
	}

	if ((channel < 0) || (channel >= instance->channels))
	{
		PERROR("Invalid channel specified. Must be between 0 and %d.\n", instance->channels - 1);
		return ME_ERRNO_INVALID_CHANNEL;
	}

	return ME_ERRNO_SUCCESS;
}

static int me4600_ai_io_single_config(me_subdevice_t* subdevice, struct file* filep, int channel,
										int single_config, int ref, int trig_chain, int trig_type, int trig_edge, int flags)
{
	me4600_ai_CosateQ_subdevice_t* instance;
	int err = ME_ERRNO_SUCCESS;
	int i;

	instance = (me4600_ai_CosateQ_subdevice_t *) subdevice;

	PDEBUG("executed. idx=0\n");

	err = me4600_ai_io_single_config_check(instance, channel, single_config, ref, trig_chain, trig_type, trig_edge, flags);
	if (err)
		return err;

	ME_SUBDEVICE_ENTER;
		ME_LOCK_PROTECTOR;
			switch (instance->status)
			{
				case ai_status_none:
				case ai_status_configured:
				case ai_status_configured_polling:
				case ai_status_end:
					// OK - subdevice in idle
					break;

				case ai_status_run:
				case ai_status_run_polling:
					//Subdevice running!
					PERROR("Subdevice is busy.\n");
					err = ME_ERRNO_SUBDEVICE_BUSY;
					goto ERROR;
					break;

				case ai_status_error:
					err = ME_ERRNO_COMMUNICATION;
					goto ERROR;
					break;

				default:
					PERROR_CRITICAL("WRONG STATUS!\n");
					err = ME_ERRNO_INTERNAL;
					goto ERROR;
			}

			//Prepare data entry.
			if (ref == ME_REF_NONE)
			{// Remove channel from list.
				instance->single_entry[channel] = channel;
				instance->single_status &= ~(ME_SINGLE_CHANNEL_CONFIGURED << channel);
				// Copy this settings to other outputs.
				if(flags == ME_IO_SINGLE_CONFIG_CONTINUE)
				{
					for(i=channel+1; i<instance->channels; i++)
					{
						instance->single_entry[i] = i;
						instance->single_status &= ~(ME_SINGLE_CHANNEL_CONFIGURED << i);
					}
				}
				goto EXIT;
			}

			//Set channel number. Reset other bits.
			instance->single_entry[channel] = channel;

			if (ref == ME_REF_AI_DIFFERENTIAL)
			{// ME_REF_AI_DIFFERENTIAL
				instance->single_entry[channel] |= ME4600_AI_LIST_INPUT_DIFFERENTIAL;
			}
			/*
			// ME4600_AI_LIST_INPUT_SINGLE_ENDED = 0x0000
			// 'entry |= ME4600_AI_LIST_INPUT_SINGLE_ENDED' <== Do nothing. Removed.
			else
			{// ME_REF_AI_GROUND
				instance->single_entry[channel]= ME4600_AI_LIST_INPUT_SINGLE_ENDED;
			}
			*/
			switch (single_config)
			{
				case 0:	//-10V..10V
						/*
						// ME4600_AI_LIST_RANGE_BIPOLAR_10 = 0x0000
						// 'entry |= ME4600_AI_LIST_RANGE_BIPOLAR_10' <== Do nothing. Removed.
						instance->single_entry[channel] |= ME4600_AI_LIST_RANGE_BIPOLAR_10;
						*/
					break;

				case 1:	//0V..10V
					instance->single_entry[channel] |= ME4600_AI_LIST_RANGE_UNIPOLAR_10;
					break;

				case 2:	//-2.5V..2.5V
					instance->single_entry[channel] |= ME4600_AI_LIST_RANGE_BIPOLAR_2_5;
					break;

				case 3:	//0V..2.5V
					instance->single_entry[channel] |= ME4600_AI_LIST_RANGE_UNIPOLAR_2_5;
					break;
			}

			// Prepare control register. Enable FIFOs. Reset other bits.
			instance->single_ctrl = ME4600_AI_CTRL_BIT_CHANNEL_FIFO | ME4600_AI_CTRL_BIT_DATA_FIFO;

			switch (trig_type)
			{
				case ME_TRIG_TYPE_NONE:
				case ME_TRIG_TYPE_SW:
					// Use 'software triggered' AI hardware mode.
					instance->single_ctrl |= ME4600_AI_CTRL_BIT_MODE_0;
					break;

				case ME_TRIG_TYPE_EXT_ANALOG:
					// Use 'ext triggered' AI hardware mode.
					instance->single_ctrl |= ME4600_AI_CTRL_BIT_MODE_1;
					instance->single_ctrl |= ME4600_AI_CTRL_BIT_EX_TRIG_ANALOG;

				case ME_TRIG_TYPE_EXT_DIGITAL:
					// Use 'ext triggered' AI hardware mode.
					instance->single_ctrl |= ME4600_AI_CTRL_BIT_MODE_1;
					instance->single_ctrl |= ME4600_AI_CTRL_BIT_EX_TRIG;
					break;
			}

			switch (trig_edge)
			{
				case ME_TRIG_EDGE_RISING:
					// Nothing to set.
					break;

				case ME_TRIG_EDGE_ANY:
						instance->single_ctrl |= ME4600_AI_CTRL_BIT_EX_TRIG_BOTH;

				case ME_TRIG_EDGE_FALLING:
						instance->single_ctrl |= ME4600_AI_CTRL_BIT_EX_TRIG_FALLING;
					break;
			}

			// Enable this channel
			instance->single_status |= (ME_SINGLE_CHANNEL_CONFIGURED << channel);

			// Copy this settings to other outputs.
			if(flags & ME_IO_SINGLE_CONFIG_CONTINUE)
			{
				for(i=channel+1; i<instance->channels; i++)
				{
					instance->single_entry[i] = (instance->single_entry[channel] & ~0x1F) | i;
					instance->single_status |= (ME_SINGLE_CHANNEL_CONFIGURED << i);
				}
			}

EXIT:
			instance->status = ai_status_none;
			if (!(flags & ME_IO_SINGLE_CONFIG_DONT_SET_BACKGROUND))
			{
				err = ai_set_single_config(instance, flags & ME_IO_SINGLE_CONFIG_POLLING_MODE);
				if (!err)
				{
					if (instance->single_status)
					{
						instance->status = (flags & ME_IO_SINGLE_CONFIG_POLLING_MODE) ? ai_status_configured_polling : ai_status_configured;
					}
					else
					{
						instance->status = ai_status_none;
					}
				}
			}
ERROR:
		ME_UNLOCK_PROTECTOR;
	ME_SUBDEVICE_EXIT;

	return err;
}

static int me4600_ai_io_single_read_check(me4600_ai_CosateQ_subdevice_t* instance, int channel,  int* value, int time_out, int flags)
{
	if ((flags & ME_IO_SINGLE_TYPE_NONBLOCKING) != ME_IO_SINGLE_TYPE_NONBLOCKING)
	{
		PERROR("Invalid flag specified. Must be ME_IO_SINGLE_TYPE_NONBLOCKING.\n");
		return ME_ERRNO_INVALID_FLAGS;
	}

	if ((channel < 0) || (channel >= instance->channels))
	{
		PERROR("Invalid channel specified. Must be between 0 and %d.\n", instance->channels - 1);
		return ME_ERRNO_INVALID_CHANNEL;
	}

	if(((instance->single_status >> channel) & ME_SINGLE_CHANNEL_CONFIG_MASK) != ME_SINGLE_CHANNEL_CONFIGURED)
	{
		PERROR("Channel is not configured!\n");
		return ME_ERRNO_PREVIOUS_CONFIG;
	}

	return ME_ERRNO_SUCCESS;
}

static int me4600_ai_io_single_read(me_subdevice_t* subdevice, struct file* filep, int channel,  int* value, int time_out, int flags)
{
	me4600_ai_CosateQ_subdevice_t* instance;
	int err = ME_ERRNO_SUCCESS;

	PDEBUG("executed. idx=0\n");

	instance = (me4600_ai_CosateQ_subdevice_t *) subdevice;

	err = me4600_ai_io_single_read_check(instance, channel,  value, time_out, flags);
	if (err)
	{
		return err;
	}

	ME_SUBDEVICE_ENTER;
		ME_LOCK_PROTECTOR;
		*value = 0xFFFFFFFF;
		switch (instance->status)
		{
			case ai_status_end:
				// OK - subdevice is ready
					*value = instance->single_value[channel];
				break;

			case ai_status_configured:
			case ai_status_configured_polling:
				//Subdevice is not ready!
				PERROR("Subdevice wasn't started.\n");
				err = ME_ERRNO_PREVIOUS_CONFIG;
				break;

			case ai_status_none:
				//Subdevice is not ready!
				PERROR("Subdevice wasn't prepared.\n");
				err = ME_ERRNO_PREVIOUS_CONFIG;
				break;

			case ai_status_run:
				//Subdevice is running!
				PERROR("Subdevice is busy.\n");
				err = ME_ERRNO_SUBDEVICE_BUSY;
				break;

			case ai_status_run_polling:
				err = polling_read(instance);
				if (!err)
				{
					// OK - subdevice is ready
					*value = instance->single_value[channel];
				}
				break;

			case ai_status_error:
				err = ME_ERRNO_COMMUNICATION;
				break;

			default:
				PERROR_CRITICAL("WRONG STATUS!\n");
				err = ME_ERRNO_INTERNAL;
		}
		ME_UNLOCK_PROTECTOR
	ME_SUBDEVICE_EXIT;

	return err;
}

static int me4600_ai_io_single_start(me_subdevice_t* subdevice, struct file* filep, int start_mode, int time_out, int flags)
{
	me4600_ai_CosateQ_subdevice_t* instance;

	uint32_t tmp;
	int err = ME_ERRNO_SUCCESS;

	PDEBUG("executed. idx=0\n");

	instance = (me4600_ai_CosateQ_subdevice_t *) subdevice;

	if (flags)
	{
		PERROR("Invalid flag specified. Must be ME_IO_STREAM_START_NO_FLAGS.\n");
		return ME_ERRNO_INVALID_FLAGS;
	}

	if (start_mode != ME_START_MODE_NONBLOCKING)
	{
		PERROR("Invalid start mode specified. Must be ME_START_MODE_NONBLOCKING.\n");
		return ME_ERRNO_INVALID_START_MODE;
	}

	ME_SUBDEVICE_ENTER;
		ME_LOCK_PROTECTOR;
			switch (instance->status)
			{
				case ai_status_end:
				case ai_status_configured:
				case ai_status_end_polling:
				case ai_status_configured_polling:
					// OK - subdevice in idle
					if((instance->single_ctrl & (ME4600_AI_CTRL_BIT_MODE_0 | ME4600_AI_CTRL_BIT_MODE_1 | ME4600_AI_CTRL_BIT_MODE_2)) == ME4600_AI_CTRL_BIT_MODE_0)
					{//Mode 1 = software trigger
						PDEBUG("Software trigger. Lets go! Start work.\n");
						//Lets go! Start work
						me_readl(instance->base.dev, &tmp, instance->start_reg);
					}
					else
					{//Mode 4 = hardware trigger
						PDEBUG("Hardware trigger. Lets go! Start work.\n");
						me_readl(instance->base.dev, &tmp, instance->ctrl_reg);
						//Enable internal state engine.
						tmp &= ~ME4600_AI_CTRL_BIT_MODE_MASK;
						tmp |= ME4600_AI_CTRL_BIT_MODE_1;
						me_writel(instance->base.dev, tmp, instance->ctrl_reg);
					}
					instance->status = ((instance->status == ai_status_end) || (instance->status == ai_status_configured)) ? instance->status = ai_status_run : ai_status_run_polling;
					break;

				case ai_status_none:
					//Subdevice is not ready!
					PERROR("Subdevice is not ready.\n");
					err = ME_ERRNO_PREVIOUS_CONFIG;
					break;

				case ai_status_run:
					//Subdevice is running!
					PERROR("Subdevice is busy.\n");
					err = ME_ERRNO_SUBDEVICE_BUSY;
					break;

				default:
					PERROR_CRITICAL("WRONG STATUS!\n");
					err = ME_ERRNO_INTERNAL;
			}
		ME_UNLOCK_PROTECTOR;
	ME_SUBDEVICE_EXIT;

	return err;
}

/** @brief Reset internal state engine -> stop all actions. Clear interrupts. Disable (clear) both FIFOs.
 *
 * @param instance The subdevice instance (pointer).
 */
static int ai_reset(me4600_ai_CosateQ_subdevice_t* instance)
{
	uint32_t tmp = 0;

	PDEBUG("executed.\n");

	// Clear all features. Dissable interrupts.
	tmp = (   ME4600_AI_CTRL_BIT_IMMEDIATE_STOP
			| ME4600_AI_CTRL_BIT_LE_IRQ_RESET
			| ME4600_AI_CTRL_BIT_HF_IRQ_RESET
			| ME4600_AI_CTRL_BIT_SC_IRQ_RESET);
	me_writel(instance->base.dev, tmp, instance->ctrl_reg);
	do
	{
		me_readl(instance->base.dev, &tmp, instance->ctrl_reg);
	}
	while(tmp & ME4600_AI_STATUS_BIT_FSM);
	tmp = (   ME4600_AI_CTRL_BIT_LE_IRQ_RESET
			| ME4600_AI_CTRL_BIT_HF_IRQ_RESET
			| ME4600_AI_CTRL_BIT_SC_IRQ_RESET);
	me_writel(instance->base.dev, tmp, instance->ctrl_reg);

	return ME_ERRNO_SUCCESS;
}

static int ai_set_single_config(me4600_ai_CosateQ_subdevice_t* instance, int flags)
{
	int err = ME_ERRNO_SUCCESS;
	uint32_t tmp = 0;
	int i;

	PDEBUG("executed.\n");

	ai_reset(instance);

	me_readl(instance->base.dev, &tmp, instance->ctrl_reg);
	//Enable channel FIFO
	tmp |= ME4600_AI_CTRL_BIT_CHANNEL_FIFO;
	me_writel(instance->base.dev, tmp, instance->ctrl_reg);
	// Write channels' list.
	for (i=0; i<32; ++i)
	{
		if (instance->single_status & (0x1 << i))
		{
			me_writel(instance->base.dev, instance->single_entry[i], instance->channel_list_reg);
		}
	}
	if (instance->single_status != 0x00000000)
	{ //Add last entry
		me_writel(instance->base.dev, ME4600_AI_LIST_LAST_ENTRY, instance->channel_list_reg);
	}

	//Set CTRL
	tmp |= instance->single_ctrl;
	//Set software trigger as default mode.
	tmp &= ~ME4600_AI_CTRL_BIT_MODE_MASK;
	tmp |= ME4600_AI_CTRL_BIT_MODE_0;
	if (!flags)
	{// Enable LE interrupt
		tmp &= ~ME4600_AI_CTRL_BIT_LE_IRQ_RESET;
		tmp |= ME4600_AI_CTRL_BIT_LE_IRQ;
	}
	tmp |= ME4600_AI_CTRL_BIT_STOP;
	me_writel(instance->base.dev, tmp, instance->ctrl_reg);

	return err;
}

static int me4600_ai_query_range_by_min_max(me_subdevice_t* subdevice, int unit, int* min, int* max, int* maxdata, int* range)
{
	me4600_ai_CosateQ_subdevice_t* instance;
	int i;
	int r = -1;
	int diff = 21E6;

	PDEBUG("executed. idx=0\n");

	instance = (me4600_ai_CosateQ_subdevice_t *) subdevice;

	if (*max < *min)
	{
		PERROR("Invalid minimum and maximum values specified. MIN: %d > MAX: %d\n", *min, *max);
		return ME_ERRNO_INVALID_MIN_MAX;
	}

	if (unit == ME_UNIT_VOLT)
	{
		for (i = 0; i < instance->ranges_len; i++)
		{
			if ((instance->ranges[i].min <= *min) && ((instance->ranges[i].max + (instance->ranges[i].max >> ME4600_RANGE_INACCURACY)) >= *max))
			{
				if ((instance->ranges[i].max - instance->ranges[i].min) - (*max - *min) < diff)
				{
					r = i;
					diff = (instance->ranges[i].max - instance->ranges[i].min) - (*max - *min);
				}
			}
		}

		if (r < 0)
		{
			PERROR("No matching range found.\n");
			return ME_ERRNO_NO_RANGE;
		}
		else
		{
			*min = instance->ranges[r].min;
			*max = instance->ranges[r].max;
			*maxdata = ME4600_AI_MAX_DATA;
			*range = r;
		}
	}
	else
	{
		PERROR("Invalid physical unit specified. Should be ME_UNIT_VOLT.\n");
		return ME_ERRNO_INVALID_UNIT;
	}

	return ME_ERRNO_SUCCESS;
}

static int me4600_ai_query_number_ranges(me_subdevice_t* subdevice, int unit, int* count)
{
	me4600_ai_CosateQ_subdevice_t* instance;

	PDEBUG("executed. idx=0\n");

	instance = (me4600_ai_CosateQ_subdevice_t *) subdevice;

	if ((unit == ME_UNIT_VOLT) || (unit == ME_UNIT_ANY))
	{
		*count = instance->ranges_len;
	}
	else
	{
		*count = 0;
	}

	return ME_ERRNO_SUCCESS;
}

static int me4600_ai_query_range_info(me_subdevice_t* subdevice, int range, int* unit, int* min, int* max, int* maxdata)
{
	me4600_ai_CosateQ_subdevice_t* instance;

	PDEBUG("executed. idx=0\n");

	instance = (me4600_ai_CosateQ_subdevice_t *) subdevice;

	if ((range < 0) || (range >= instance->ranges_len))
	{
		PERROR("Invalid range specified. Must be between 0 and %d.\n", instance->ranges_len - 1);
		return ME_ERRNO_INVALID_RANGE;
	}

	*unit = ME_UNIT_VOLT;
	*min = instance->ranges[range].min;
	*max = instance->ranges[range].max;
	*maxdata = ME4600_AI_MAX_DATA;

	return ME_ERRNO_SUCCESS;
}

static int me4600_ai_query_number_channels(me_subdevice_t* subdevice, int* number)
{
	me4600_ai_CosateQ_subdevice_t* instance;

	PDEBUG("executed. idx=0\n");

	instance = (me4600_ai_CosateQ_subdevice_t *) subdevice;
	*number = instance->channels;

	return ME_ERRNO_SUCCESS;
}

static int me4600_ai_query_subdevice_type(me_subdevice_t* subdevice, int* type, int* subtype)
{
	PDEBUG("executed. idx=0\n");

	*type = ME_TYPE_AI;
	*subtype = ME_SUBTYPE_SINGLE;

	return ME_ERRNO_SUCCESS;
}

static int me4600_ai_query_subdevice_caps(me_subdevice_t* subdevice, int* caps)
{
	me4600_ai_CosateQ_subdevice_t* instance;

	instance = (me4600_ai_CosateQ_subdevice_t *) subdevice;

	PDEBUG("executed. idx=0\n");

	*caps = 0;
	if (instance->features & ME4600_DIFFERENTIAL)
	{
		*caps = *caps | ME_CAPS_AI_DIFFERENTIAL;
	}

	PINFO("CAPS: %x\n", *caps);

	return ME_ERRNO_SUCCESS;
}

static int me4600_ai_irq_handle(me_subdevice_t* subdevice, uint32_t irq_status)
{/// @note This is time critical function!
	uint32_t ctrl_status;
	me4600_ai_CosateQ_subdevice_t* instance = (me4600_ai_CosateQ_subdevice_t *)subdevice;
	int err = ME_ERRNO_SUCCESS;

#ifdef MEDEBUG_SPEED_TEST
	uint64_t execuction_time;

	rdtscll(instance->int_end);
#endif

	PDEBUG("executed. idx=0\n");

	if (!(irq_status & ME4600_IRQ_STATUS_BIT_LE))
	{
		//This is security check case. This should never ever happend because of 'LE' is only one that is in use.
		if (irq_status & (ME4600_IRQ_STATUS_BIT_AI_HF | ME4600_IRQ_STATUS_BIT_SC | ME4600_IRQ_STATUS_BIT_AI_OF))
		{
			me_readl(instance->base.dev, &ctrl_status, instance->ctrl_reg);
			me_writel(instance->base.dev, ctrl_status | ME4600_AI_CTRL_BIT_HF_IRQ_RESET | ME4600_AI_CTRL_BIT_SC_IRQ_RESET, instance->ctrl_reg);
			PERROR_CRITICAL("%ld Shared interrupt. %s(): irq_status_reg=0x%04X\n", jiffies, __FUNCTION__, irq_status);
		}
		else
		{
			PINFO("%ld Shared interrupt. %s(): irq_status_reg=0x%04X\n", jiffies, __FUNCTION__, irq_status);
			err = ME_ERRNO_INTERNAL;
		}
		goto ERROR;
	}

	//Get the status register.
	me_readl(instance->base.dev, &ctrl_status, instance->status_reg);

#ifdef MEDEBUG_INFO
	if (irq_status & ME4600_IRQ_STATUS_BIT_AI_HF)
			PINFO("HF interrupt active\n");
	if (irq_status & ME4600_IRQ_STATUS_BIT_SC)
			PINFO("SC interrupt active\n");
	if (irq_status & ME4600_IRQ_STATUS_BIT_LE)
			PINFO("LE interrupt active\n");
	if (irq_status & ME4600_IRQ_STATUS_BIT_AI_OF)
			PINFO("OF interrupt active\n");
#endif

#ifdef MEDEBUG_INFO
	PINFO("%s IN:\n", __FUNCTION__);
# ifdef MEDEBUG_EXTENDED_INFO
	PINFO("  mode: %d.\n", ctrl_status & (ME4600_AI_CTRL_BIT_MODE_0 | ME4600_AI_CTRL_BIT_MODE_1 | ME4600_AI_CTRL_BIT_MODE_2) );
	PINFO("  CTRL_BIT_SAMPLE_HOLD=%s.\n", (ctrl_status & ME4600_AI_CTRL_BIT_SAMPLE_HOLD)?"on":"off");
	PINFO("  CTRL_BIT_IMMEDIATE_STOP=%s.\n", (ctrl_status & ME4600_AI_CTRL_BIT_IMMEDIATE_STOP)?"on":"off");
	PINFO("  CTRL_BIT_STOP=%s.\n", (ctrl_status & ME4600_AI_CTRL_BIT_STOP)?"on":"off");
	PINFO("  CTRL_BIT_CHANNEL_FIFO=%s.\n", (ctrl_status & ME4600_AI_CTRL_BIT_CHANNEL_FIFO)?"enable":"disable");
	PINFO("  CTRL_BIT_DATA_FIFO=%s.\n", (ctrl_status & ME4600_AI_CTRL_BIT_DATA_FIFO)?"enable":"disable");
	PINFO("  CTRL_BIT_FULLSCALE=%s.\n", (ctrl_status & ME4600_AI_CTRL_BIT_FULLSCALE)?"on":"off");
	PINFO("  CTRL_BIT_OFFSET=%s.\n", (ctrl_status & ME4600_AI_CTRL_BIT_OFFSET)?"on":"off");
# endif
	PINFO("  STATUS_BIT_FSM: %s.\n", (ctrl_status & ME4600_AI_STATUS_BIT_FSM)?"on":"off");

	PINFO("  STATUS_BIT_EF_CHANNEL: %s.\n", (ctrl_status & ME4600_AI_STATUS_BIT_EF_CHANNEL)?"not empty":"empty");
	PINFO("  STATUS_BIT_HF_CHANNEL: %s.\n", (ctrl_status & ME4600_AI_STATUS_BIT_HF_CHANNEL)?" <= HF":" > HF");
	PINFO("  STATUS_BIT_FF_CHANNEL: %s.\n", (ctrl_status & ME4600_AI_STATUS_BIT_FF_CHANNEL)?"not full":"full");

	PINFO("  STATUS_BIT_EF_DATA: %s.\n", (ctrl_status & ME4600_AI_STATUS_BIT_EF_DATA)?"not empty":"empty");
	PINFO("  STATUS_BIT_HF_DATA:%s.\n", (ctrl_status & ME4600_AI_STATUS_BIT_HF_DATA)?" <= HF":" > HF");
	PINFO("  STATUS_BIT_FF_DATA: %s.\n", (ctrl_status & ME4600_AI_STATUS_BIT_FF_DATA)?"not full":"full");

	PINFO("  CTRL_BIT_HF_IRQ: %s.\n", (ctrl_status & ME4600_AI_CTRL_BIT_HF_IRQ)?"enable":"disable");
	PINFO("  CTRL_BIT_HF_IRQ_RESET: %s.\n", (ctrl_status & ME4600_AI_CTRL_BIT_HF_IRQ_RESET)?"reset":"work");
	PINFO("  CTRL_BIT_SC_IRQ: %s.\n", (ctrl_status & ME4600_AI_CTRL_BIT_SC_IRQ)?"enable":"disable");
	PINFO("  CTRL_BIT_SC_RELOAD: %s.\n", (ctrl_status & ME4600_AI_CTRL_BIT_SC_RELOAD)?"on":"off");
	PINFO("  CTRL_BIT_SC_IRQ_RESET: %s.\n", (ctrl_status & ME4600_AI_CTRL_BIT_SC_IRQ_RESET)?"reset":"work");
#endif

	read_from_fifo(instance);

	me_readl(instance->base.dev, &ctrl_status, instance->ctrl_reg);
	ctrl_status |= ME4600_AI_CTRL_BIT_LE_IRQ_RESET;
	ctrl_status &= ~ME4600_AI_CTRL_BIT_DATA_FIFO;
	me_writel(instance->base.dev, ctrl_status, instance->ctrl_reg);
	ctrl_status &= ~(ME4600_AI_CTRL_BIT_LE_IRQ_RESET);
	ctrl_status |= ME4600_AI_CTRL_BIT_DATA_FIFO;
	me_writel(instance->base.dev, ctrl_status, instance->ctrl_reg);

	instance->status = ai_status_end;
#ifdef MEDEBUG_INFO
	me_readl(instance->base.dev, &ctrl_status, instance->ctrl_reg);
	PINFO("%s OUT:\n", __FUNCTION__);
# ifdef MEDEBUG_EXTENDED_INFO
	PINFO("  mode: %d.\n", ctrl_status & (ME4600_AI_CTRL_BIT_MODE_0 | ME4600_AI_CTRL_BIT_MODE_1 | ME4600_AI_CTRL_BIT_MODE_2) );
	PINFO("  CTRL_BIT_SAMPLE_HOLD=%s.\n", (ctrl_status & ME4600_AI_CTRL_BIT_SAMPLE_HOLD)?"on":"off");
	PINFO("  CTRL_BIT_IMMEDIATE_STOP=%s.\n", (ctrl_status & ME4600_AI_CTRL_BIT_IMMEDIATE_STOP)?"on":"off");
	PINFO("  CTRL_BIT_STOP=%s.\n", (ctrl_status & ME4600_AI_CTRL_BIT_STOP)?"on":"off");
	PINFO("  CTRL_BIT_CHANNEL_FIFO=%s.\n", (ctrl_status & ME4600_AI_CTRL_BIT_CHANNEL_FIFO)?"enable":"disable");
	PINFO("  CTRL_BIT_DATA_FIFO=%s.\n", (ctrl_status & ME4600_AI_CTRL_BIT_DATA_FIFO)?"enable":"disable");
	PINFO("  CTRL_BIT_FULLSCALE=%s.\n", (ctrl_status & ME4600_AI_CTRL_BIT_FULLSCALE)?"on":"off");
	PINFO("  CTRL_BIT_OFFSET=%s.\n", (ctrl_status & ME4600_AI_CTRL_BIT_OFFSET)?"on":"off");
# endif
	PINFO("  STATUS_BIT_FSM: %s.\n", (ctrl_status & ME4600_AI_STATUS_BIT_FSM)?"on":"off");

	PINFO("  STATUS_BIT_EF_CHANNEL: %s.\n", (ctrl_status & ME4600_AI_STATUS_BIT_EF_CHANNEL)?"not empty":"empty");
	PINFO("  STATUS_BIT_HF_CHANNEL: %s.\n", (ctrl_status & ME4600_AI_STATUS_BIT_HF_CHANNEL)?" <= HF":" > HF");
	PINFO("  STATUS_BIT_FF_CHANNEL: %s.\n", (ctrl_status & ME4600_AI_STATUS_BIT_FF_CHANNEL)?"not full":"full");

	PINFO("  STATUS_BIT_EF_DATA: %s.\n", (ctrl_status & ME4600_AI_STATUS_BIT_EF_DATA)?"not empty":"empty");
	PINFO("  STATUS_BIT_HF_DATA: %s.\n", (ctrl_status & ME4600_AI_STATUS_BIT_HF_DATA)?" <= HF":" > HF");
	PINFO("  STATUS_BIT_FF_DATA: %s.\n", (ctrl_status & ME4600_AI_STATUS_BIT_FF_DATA)?"not full":"full");

	PINFO("  CTRL_BIT_HF_IRQ_RESET: %s.\n", (ctrl_status & ME4600_AI_CTRL_BIT_HF_IRQ_RESET)?"reset":"work");
	PINFO("  CTRL_BIT_SC_IRQ: %s.\n", (ctrl_status & ME4600_AI_CTRL_BIT_SC_IRQ)?"enable":"disable");
	PINFO("  CTRL_BIT_SC_RELOAD: %s.\n", (ctrl_status & ME4600_AI_CTRL_BIT_SC_RELOAD)?"on":"off");
	PINFO("  CTRL_BIT_SC_IRQ_RESET: %s.\n", (ctrl_status & ME4600_AI_CTRL_BIT_SC_IRQ_RESET)?"reset":"work");
#endif

ERROR:
#ifdef MEDEBUG_SPEED_TEST
	rdtscll(execuction_time);
	PSPEED("me4600 AI:  %lld %lld %lld\n", instance->int_end, instance->int_end - instance->int_start, execuction_time - instance->int_end);
	instance->int_start = instance->int_end;
#endif
	return err;
}

static inline void read_from_fifo(me4600_ai_CosateQ_subdevice_t* instance)
{
	uint32_t tmp;
	int channel;

	for (channel=0; channel<32; ++channel)
	{
		if (instance->single_status & (0x1 << channel))
		{
			me_readl(instance->base.dev, &tmp, instance->data_reg);
			tmp = ai_calculate_calibrated_value(instance, instance->single_entry[channel], tmp);
			instance->single_value[channel] = tmp & ME4600_AI_MAX_DATA;

		}
		else
		{
			instance->single_value[channel] = 0xFFFFFFFF;
		}
	}
}

static int polling_read(me4600_ai_CosateQ_subdevice_t* instance)
{
	uint32_t ctrl_status;
	int err = ME_ERRNO_SUCCESS;

	me_readl(instance->base.dev, &ctrl_status, instance->ctrl_reg);

	if (!(ctrl_status & ME4600_AI_STATUS_BIT_FSM))
	{
		read_from_fifo(instance);

		me_readl(instance->base.dev, &ctrl_status, instance->ctrl_reg);
		ctrl_status &= ~ME4600_AI_CTRL_BIT_DATA_FIFO;
		me_writel(instance->base.dev, ctrl_status, instance->ctrl_reg);
		ctrl_status |= ME4600_AI_CTRL_BIT_DATA_FIFO;
		me_writel(instance->base.dev, ctrl_status, instance->ctrl_reg);

		instance->status = ai_status_end_polling;
	}
	else
	{
		//Subdevice is running!
		PERROR("Subdevice is busy.\n");
		err = ME_ERRNO_SUBDEVICE_BUSY;
	}

	return err;
}
static int me4600_ai_postinit(me_subdevice_t* subdevice, void* args)
{
	PDEBUG("executed. idx=0\n");

	// We have to switch the mux in order to get it work correctly.
	ai_mux_toggler((me4600_ai_CosateQ_subdevice_t *)subdevice);

	ai_read_calibration((me4600_ai_CosateQ_subdevice_t *)subdevice);

	// Reset subdevice.
	return me4600_ai_io_reset_subdevice(subdevice, NULL, ME_IO_RESET_SUBDEVICE_NO_FLAGS);
}

me4600_ai_CosateQ_subdevice_t* me4600_ai_constr(void* reg_base,
											void* DMA_base,
											void* PLX_base,
											unsigned int idx,
											unsigned int channels, unsigned int ranges,
											int features,
											/* me4600_interrupt_status_t* int_status, */
											struct workqueue_struct* me4600_wq)
{
	me4600_ai_CosateQ_subdevice_t* subdevice;

	PDEBUG("executed. idx=0\n");

	// Allocate memory for subdevice instance.
	subdevice = kzalloc(sizeof(me4600_ai_CosateQ_subdevice_t), GFP_KERNEL);
	if (!subdevice)
	{
		PERROR("Cannot get memory for subdevice instance.\n");
		return NULL;
	}

	// Initialize subdevice base class.
	if (me_subdevice_init(&subdevice->base))
	{
		PERROR("Cannot initialize subdevice base class instance.\n");
		kfree(subdevice);
		return NULL;
	}

	// Store analog input index.
	subdevice->base.idx = idx;

	subdevice->status = ai_status_none;

	// Save the number of channels.
	subdevice->channels = channels;

	// Save sub-device features: "sample and hold", "isolated" and "analog trigger".
	subdevice->features = features;

	subdevice->raw_values = 0;

	// Initialize registers addresses.
	subdevice->ctrl_reg = reg_base + ME4600_AI_CTRL_REG;
	subdevice->status_reg = reg_base + ME4600_AI_STATUS_REG;
	subdevice->channel_list_reg = reg_base + ME4600_AI_CHANNEL_LIST_REG;
	subdevice->data_reg = reg_base + ME4600_AI_DATA_REG;
	subdevice->chan_timer_reg = reg_base + ME4600_AI_CHAN_TIMER_REG;
	subdevice->chan_pre_timer_reg = reg_base + ME4600_AI_CHAN_PRE_TIMER_REG;
	subdevice->scan_timer_low_reg = reg_base + ME4600_AI_SCAN_TIMER_LOW_REG;
	subdevice->scan_timer_high_reg = reg_base + ME4600_AI_SCAN_TIMER_HIGH_REG;
	subdevice->scan_pre_timer_low_reg = reg_base + ME4600_AI_SCAN_PRE_TIMER_LOW_REG;
	subdevice->scan_pre_timer_high_reg = reg_base + ME4600_AI_SCAN_PRE_TIMER_HIGH_REG;
	subdevice->start_reg = reg_base + ME4600_AI_START_REG;
	subdevice->sample_counter_reg = reg_base + ME4600_AI_SAMPLE_COUNTER_REG;

	subdevice->DMA_base = DMA_base;
	subdevice->PLX_base = PLX_base;

	subdevice->irq_status_reg = reg_base + ME4600_IRQ_STATUS_REG;

	// Initialize ranges.
	subdevice->ranges_len = ranges;
	subdevice->ranges[0].min = -10E6;
	subdevice->ranges[0].max = 9999695;

	subdevice->ranges[1].min = 0;
	subdevice->ranges[1].max = 9999847;

	subdevice->ranges[2].min = -25E5;
	subdevice->ranges[2].max = 2499924;

	subdevice->ranges[3].min = 0;
	subdevice->ranges[3].max = 2499962;

	// Override base class methods.
	subdevice->base.me_subdevice_destructor = me4600_ai_destructor;
	subdevice->base.me_subdevice_io_reset_subdevice = me4600_ai_io_reset_subdevice;
	subdevice->base.me_subdevice_io_single_config = me4600_ai_io_single_config;
	subdevice->base.me_subdevice_io_single_read = me4600_ai_io_single_read;
	subdevice->base.me_subdevice_io_stream_start = me4600_ai_io_single_start;
	subdevice->base.me_subdevice_query_number_channels = me4600_ai_query_number_channels;
	subdevice->base.me_subdevice_query_subdevice_type = me4600_ai_query_subdevice_type;
	subdevice->base.me_subdevice_query_subdevice_caps = me4600_ai_query_subdevice_caps;
	subdevice->base.me_subdevice_query_range_by_min_max = me4600_ai_query_range_by_min_max;
	subdevice->base.me_subdevice_query_number_ranges = me4600_ai_query_number_ranges;
	subdevice->base.me_subdevice_query_range_info = me4600_ai_query_range_info;

	subdevice->base.me_subdevice_irq_handle = me4600_ai_irq_handle;

	subdevice->base.me_subdevice_postinit = me4600_ai_postinit;

	PLOG("me4600_AI: special version for CosateQ.\n");
	return subdevice;
}

static void ai_mux_toggler(me4600_ai_CosateQ_subdevice_t* instance)
{
	uint32_t ctrl;
	uint32_t tmp;

	ME_LOCK_PROTECTOR;
		PDEBUG("executed. idx=0\n");
		me_writel(instance->base.dev, 0, instance->scan_pre_timer_low_reg);
		me_writel(instance->base.dev, 0, instance->scan_pre_timer_high_reg);
		me_writel(instance->base.dev, 0, instance->scan_timer_low_reg);
		me_writel(instance->base.dev, 0, instance->scan_timer_high_reg);
		me_writel(instance->base.dev, ME4600_AI_MIN_CHAN_TICKS-1, instance->chan_timer_reg);
		me_writel(instance->base.dev, ME4600_AI_MIN_ACQ_TICKS-1, instance->chan_pre_timer_reg);
		// Clear all features. Dissable interrupts.
		ctrl = ( 0x00
			| ME4600_AI_CTRL_BIT_IMMEDIATE_STOP
			| ME4600_AI_CTRL_BIT_LE_IRQ_RESET
			| ME4600_AI_CTRL_BIT_HF_IRQ_RESET
			| ME4600_AI_CTRL_BIT_SC_IRQ_RESET);
		// Turn on internal reference.
		ctrl |= ME4600_AI_CTRL_BIT_FULLSCALE;
		me_writel(instance->base.dev, ctrl, instance->ctrl_reg);
		// Clear data and channel fifo.
		ctrl &= ~(ME4600_AI_CTRL_BIT_CHANNEL_FIFO | ME4600_AI_CTRL_BIT_DATA_FIFO);
		me_writel(instance->base.dev, ctrl, instance->ctrl_reg);
		ctrl |= ME4600_AI_CTRL_BIT_CHANNEL_FIFO | ME4600_AI_CTRL_BIT_DATA_FIFO;
		me_writel(instance->base.dev, ctrl, instance->ctrl_reg);
		// Write channel entry.
		me_writel(instance->base.dev, ME4600_AI_LIST_INPUT_DIFFERENTIAL | ME4600_AI_LIST_RANGE_UNIPOLAR_2_5 | 31, instance->channel_list_reg);
		ctrl &= ~ME4600_AI_CTRL_BIT_IMMEDIATE_STOP;
		me_writel(instance->base.dev, ctrl, instance->ctrl_reg);
		// Start conversion.
		me_readl(instance->base.dev, &tmp, instance->start_reg);
		udelay(10);

		// Clear data and channel fifo.
		ctrl &= ~(ME4600_AI_CTRL_BIT_CHANNEL_FIFO | ME4600_AI_CTRL_BIT_DATA_FIFO);
		me_writel(instance->base.dev, ctrl, instance->ctrl_reg);
		ctrl |= ME4600_AI_CTRL_BIT_CHANNEL_FIFO | ME4600_AI_CTRL_BIT_DATA_FIFO;
		me_writel(instance->base.dev, ctrl, instance->ctrl_reg);
		// Write channel entry.
		// ME4600_AI_LIST_INPUT_SINGLE_ENDED | ME4600_AI_LIST_RANGE_BIPOLAR_10 <= 0x0000
		me_writel(instance->base.dev, ME4600_AI_LIST_INPUT_SINGLE_ENDED | ME4600_AI_LIST_RANGE_BIPOLAR_10, instance->channel_list_reg);
		// Start conversion.
		me_readl(instance->base.dev, &tmp, instance->start_reg);
		udelay(10);

		// Clear control register.
		// Turn off internal reference (reset ME4600_AI_CTRL_BIT_FULLSCALE !).
		ctrl = ( 0x00
			| ME4600_AI_CTRL_BIT_IMMEDIATE_STOP
			| ME4600_AI_CTRL_BIT_LE_IRQ_RESET
			| ME4600_AI_CTRL_BIT_HF_IRQ_RESET
			| ME4600_AI_CTRL_BIT_SC_IRQ_RESET);
		me_writel(instance->base.dev, tmp, instance->ctrl_reg);
	ME_UNLOCK_PROTECTOR;
}

static void ai_read_calibration(me4600_ai_CosateQ_subdevice_t* instance)
{
	int address_lenght = 0;
	me4600_ai_eeprom_t raw_cal = {0,0,0,0,0,0,0,0,0,0,0,0,0,0};

	if (!me_plx_eeprom_check(instance->base.dev, instance->PLX_base))
	{
		me_plx_eeprom_address_lenght_check(instance->base.dev, instance->PLX_base, &address_lenght);
	}
	if ((address_lenght < 4) || (address_lenght > 32))
	{// No correct EEPROM
		return;
	}

	me_plx_eeprom_read(instance->base.dev, instance->PLX_base, address_lenght, ME4600_AI_CALIB_START, (uint16_t *) &raw_cal, sizeof(me4600_ai_eeprom_t)/sizeof(uint16_t));

	PINFO("year: %d\n", raw_cal.year);
	PINFO("month: %d\n", raw_cal.month);
	PINFO("unipolar_10A: 0x%04x\n", raw_cal.unipolar_10A);
	PINFO("unipolar_10B: 0x%04x\n", raw_cal.unipolar_10B);
	PINFO("bipolar_10A: 0x%04x\n", raw_cal.bipolar_10A);
	PINFO("bipolar_10B: 0x%04x\n", raw_cal.bipolar_10B);
	PINFO("differential_10A: 0x%04x\n", raw_cal.differential_10A);
	PINFO("differential_10B: 0x%04x\n", raw_cal.differential_10B);
	PINFO("unipolar_2_5A: 0x%04x\n", raw_cal.unipolar_2_5A);
	PINFO("unipolar_2_5B: 0x%04x\n", raw_cal.unipolar_2_5B);
	PINFO("bipolar_2_5A: 0x%04x\n", raw_cal.bipolar_2_5A);
	PINFO("bipolar_2_5B: 0x%04x\n", raw_cal.bipolar_2_5B);
	PINFO("differential_2_5A: 0x%04x\n", raw_cal.differential_2_5A);
	PINFO("differential_2_5B: 0x%04x\n", raw_cal.differential_2_5B);

	ai_calculate_calibration(instance, raw_cal);
	PINFO("CAL: unipolar_10.constant: %ld\n", instance->calibration.unipolar_10.constant);
	PINFO("CAL: unipolar_10.multiplier: %ld\n", instance->calibration.unipolar_10.multiplier);
	PINFO("CAL: unipolar_10.divisor: %ld\n", instance->calibration.unipolar_10.divisor);
	PINFO("CAL: bipolar_10.constant: %ld\n", instance->calibration.bipolar_10.constant);
	PINFO("CAL: bipolar_10.multiplier: %ld\n", instance->calibration.bipolar_10.multiplier);
	PINFO("CAL: bipolar_10.divisor: %ld\n", instance->calibration.bipolar_10.divisor);
	PINFO("CAL: differential_10.constant: %ld\n", instance->calibration.differential_10.constant);
	PINFO("CAL: differential_10.multiplier: %ld\n", instance->calibration.differential_10.multiplier);
	PINFO("CAL: differential_10.divisor: %ld\n", instance->calibration.differential_10.divisor);
	PINFO("CAL: unipolar_2_5.constant: %ld\n", instance->calibration.unipolar_2_5.constant);
	PINFO("CAL: unipolar_2_5.multiplier: %ld\n", instance->calibration.unipolar_2_5.multiplier);
	PINFO("CAL: unipolar_2_5.divisor: %ld\n", instance->calibration.unipolar_2_5.divisor);
	PINFO("CAL: bipolar_2_5.constant: %ld\n", instance->calibration.bipolar_2_5.constant);
	PINFO("CAL: bipolar_2_5.multiplier: %ld\n", instance->calibration.bipolar_2_5.multiplier);
	PINFO("CAL: bipolar_2_5.divisor: %ld\n", instance->calibration.bipolar_2_5.divisor);
	PINFO("CAL: differential_2_5.constant: %ld\n", instance->calibration.differential_2_5.constant);
	PINFO("CAL: differential_2_5.multiplier: %ld\n", instance->calibration.differential_2_5.multiplier);
	PINFO("CAL: differential_2_5.divisor: %ld\n", instance->calibration.differential_2_5.divisor);
}

static void ai_default_calibration_entry(me_calibration_entry_t* entry)
{
	entry->constant = 0;
	entry->multiplier = 1;
	entry->divisor = 1;
}

static void ai_default_calibration(me4600_ai_CosateQ_subdevice_t* instance)
{
	ai_default_calibration_entry (&instance->calibration.unipolar_10);
	ai_default_calibration_entry (&instance->calibration.bipolar_10);
	ai_default_calibration_entry (&instance->calibration.differential_10);

	ai_default_calibration_entry (&instance->calibration.unipolar_2_5);
	ai_default_calibration_entry (&instance->calibration.bipolar_2_5);
	ai_default_calibration_entry (&instance->calibration.differential_2_5);
}

static void ai_calculate_calibration_entry(int64_t nominal_A, int64_t actual_A, int64_t nominal_B, int64_t actual_B, me_calibration_entry_t* entry)
{
/**
	Because of both actual and desired characteristics of the transformer are linear, there is a
	linear relationship between the two. It is enough for the DIGITAL debit and
	actual values at two different (the sake of accuracy as far as possible disparate)
	points to know about freely between debit and actual values back and forth to counts.

	There is a relationship SOLL = IST * F + K between DIGITAL Transformer values.

	Suppose that the nominal value SOLL_a the actual value IST_a and setpoint
	SOLL_b corresponds to the actual value IST_b

	SOLL_a --> IST_a
	SOLL_b --> IST_b

	Then we have;

	SOLL_a = IST_a * F + K
	SOLL_b = IST_b * F + K

	Resulting:

	F = (SOLL_b - SOLL_a ) / (IST_b - IST_a)
	and
	K = (SOLL_a * IST_b - IST_a * SOLL_b) / (IST_b - IST_a)
	and so the relationship between projected and actual value is as follows:
	SOLL = ( IST *(SOLL_b - SOLL_a) + (SOLL_a * IST_b - IST_a * SOLL_b) ) / (IST_b - IST_a)

	Putting:
	X = SOLL_b - SOLL_a
	Y = SOLL_a * IST_b - IST_a * SOLL_b
	Z = IST_b - IST_a

	then we have:
	SOLL = (IST * X + Y) / Z
*/

	if(actual_A != actual_B)
	{
		entry->constant = -((nominal_A * actual_B) + (actual_A * nominal_B));
		entry->multiplier = nominal_B + nominal_A;
		entry->divisor = actual_B - actual_A;
	}
	else
	{
		ai_default_calibration_entry(entry);
	}
}

static void ai_calculate_calibration(me4600_ai_CosateQ_subdevice_t* instance, me4600_ai_eeprom_t raw_cal)
{
	int calibration_exist = 0;

	if ((raw_cal.year > 1990) && (raw_cal.year < 2100))
	{
		if ((raw_cal.month > 0) && (raw_cal.month < 13))
		{
			calibration_exist = 1;
		}
	}
	else if ((raw_cal.month > 1990) && (raw_cal.month < 2100))
	{	/// @note: Year and month may have been swapped due to an error in early versions of the calibration software.

		if ((raw_cal.year > 0) && (raw_cal.year < 13))
		{
			calibration_exist = 1;
		}
	}

	if (calibration_exist)
	{
# define AI_CAL_VALIDATE(val) ((val >= -1000) && (val <= 1000))

		if (!AI_CAL_VALIDATE(raw_cal.unipolar_10A + ME4600_AI_CALIB_UNIPOLAR_10_A)
			||
			!AI_CAL_VALIDATE(raw_cal.unipolar_10B - ME4600_AI_CALIB_UNIPOLAR_10_B))
		{
			calibration_exist = 0;
			PERROR("Invalid calibration for range 1!\n");
		}

		if (!AI_CAL_VALIDATE(raw_cal.bipolar_10A + ME4600_AI_CALIB_BIPOLAR_10_A)
			||
			!AI_CAL_VALIDATE(raw_cal.bipolar_10B - ME4600_AI_CALIB_BIPOLAR_10_B))
		{
			calibration_exist = 0;
			PERROR("Invalid calibration for range 0!\n");
		}

		if (!AI_CAL_VALIDATE(raw_cal.differential_10A + ME4600_AI_CALIB_BIPOLAR_10_A)
			||
			!AI_CAL_VALIDATE(raw_cal.differential_10B - ME4600_AI_CALIB_BIPOLAR_10_B))
		{
			calibration_exist = 0;
			PERROR("Invalid calibration for range 0 in differential mode!\n");
		}

		if (!AI_CAL_VALIDATE(raw_cal.unipolar_2_5A + ME4600_AI_CALIB_UNIPOLAR_2_5_A)
			||
			!AI_CAL_VALIDATE(raw_cal.unipolar_2_5B - ME4600_AI_CALIB_UNIPOLAR_2_5_B))
		{
			calibration_exist = 0;
			PERROR("Invalid calibration for range 3!\n");
		}

		if (!AI_CAL_VALIDATE(raw_cal.bipolar_2_5A + ME4600_AI_CALIB_BIPOLAR_2_5_A)
			||
			!AI_CAL_VALIDATE(raw_cal.bipolar_2_5B - ME4600_AI_CALIB_BIPOLAR_2_5_B))
		{
			calibration_exist = 0;
			PERROR("Invalid calibration for range 2!\n");
		}

		if (!AI_CAL_VALIDATE(raw_cal.differential_2_5A + ME4600_AI_CALIB_BIPOLAR_2_5_A)
			||
			!AI_CAL_VALIDATE(raw_cal.differential_2_5B - ME4600_AI_CALIB_BIPOLAR_2_5_B))
		{
			calibration_exist = 0;
			PERROR("Invalid calibration for range 2 in differential mode!\n");
		}
	}
	else
	{
		PDEBUG("EEPROM doesn't contain valid calibration!\n");
	}

	if (calibration_exist)
	{
		ai_calculate_calibration_entry(	ME4600_AI_CALIB_UNIPOLAR_10_A, raw_cal.unipolar_10A,
										ME4600_AI_CALIB_UNIPOLAR_10_B, raw_cal.unipolar_10B,
										&instance->calibration.unipolar_10);
		ai_calculate_calibration_entry(	ME4600_AI_CALIB_BIPOLAR_10_A, raw_cal.bipolar_10A,
										ME4600_AI_CALIB_BIPOLAR_10_B, raw_cal.bipolar_10B,
										&instance->calibration.bipolar_10);
		ai_calculate_calibration_entry(	ME4600_AI_CALIB_BIPOLAR_10_A, raw_cal.differential_10A,
										ME4600_AI_CALIB_BIPOLAR_10_B, raw_cal.differential_10B,
										&instance->calibration.differential_10);
		ai_calculate_calibration_entry(	ME4600_AI_CALIB_UNIPOLAR_2_5_A, raw_cal.unipolar_2_5A,
										ME4600_AI_CALIB_UNIPOLAR_2_5_B, raw_cal.unipolar_2_5B,
										&instance->calibration.unipolar_2_5);
		ai_calculate_calibration_entry(	ME4600_AI_CALIB_BIPOLAR_2_5_A, raw_cal.bipolar_2_5A,
										ME4600_AI_CALIB_BIPOLAR_2_5_B, raw_cal.bipolar_2_5B,
										&instance->calibration.bipolar_2_5);
		ai_calculate_calibration_entry(	ME4600_AI_CALIB_BIPOLAR_2_5_A, raw_cal.differential_2_5A,
										ME4600_AI_CALIB_BIPOLAR_2_5_B, raw_cal.differential_2_5B,
										&instance->calibration.differential_2_5);
	}
	else
	{
		ai_default_calibration(instance);
	}
}

static uint16_t inline ai_calculate_end_value(const me_calibration_entry_t calibration, int16_t value)
{
	long int cal_val;

	if (((value & 0xFFFF) == 0x8000) || ((value & 0xFFFF) == 0x7FFF))
	{// We shouldn't calibrate border values.
		return (uint16_t)(value ^ 0x8000);
	}

	cal_val = value;
	cal_val *= calibration.multiplier;
	cal_val += calibration.constant;
	cal_val /= calibration.divisor;

	if(cal_val < -0x8000)
	{
		cal_val = -0x8000;
	}
	else if(cal_val > 0x7FFF)
	{
		cal_val = 0x7FFF;
	}

	cal_val ^= 0x8000;

	return (uint16_t)cal_val;

}

static uint16_t inline ai_calculate_calibrated_value(me4600_ai_CosateQ_subdevice_t* instance, int entry, int value)
{
	uint16_t tmp;

	if(instance->raw_values)
	{
		PINFO("Returning raw values.\n");
		return value ^ 0x8000;
	}

	switch (entry & ME4600_AI_LIST_CONFIG_MASK)
	{
		case ME4600_AI_LIST_RANGE_BIPOLAR_10:
			tmp = ai_calculate_end_value(instance->calibration.bipolar_10, value);
			break;

		case ME4600_AI_LIST_RANGE_BIPOLAR_2_5:
			tmp = ai_calculate_end_value(instance->calibration.bipolar_2_5, value);
			break;

		case (ME4600_AI_LIST_INPUT_DIFFERENTIAL | ME4600_AI_LIST_RANGE_BIPOLAR_10):
			tmp = ai_calculate_end_value(instance->calibration.unipolar_10, value);
			break;

		case ME4600_AI_LIST_RANGE_UNIPOLAR_10:
			tmp = ai_calculate_end_value(instance->calibration.unipolar_2_5, value);
			break;

		case ME4600_AI_LIST_RANGE_UNIPOLAR_2_5:
			tmp = ai_calculate_end_value(instance->calibration.differential_10, value);
			break;

		case (ME4600_AI_LIST_INPUT_DIFFERENTIAL | ME4600_AI_LIST_RANGE_BIPOLAR_2_5):
			tmp = ai_calculate_end_value(instance->calibration.differential_2_5, value);
			break;

		default:
			PERROR("Unrecognized mode:0x%x\n", entry & ME4600_AI_LIST_CONFIG_MASK);
			tmp = value ^ 0x8000;
	}
	return tmp;
}
