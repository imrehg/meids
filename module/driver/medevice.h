/**
 * @file medevice.h
 *
 * @brief Headers for Meilhaus device base class.
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

#ifdef __KERNEL__

# if !defined(ME_PCI) && !defined(ME_USB) && !defined(ME_COMEDI) && !defined(ME_MEPHISTO)
#  error NO VALID DRIVER TYPE declared!
# endif

# ifndef _ME_DEVICE_H_
#  define _ME_DEVICE_H_

# ifndef KBUILD_MODNAME
#  define KBUILD_MODNAME KBUILD_STR(meilhaus)
# endif


# include <linux/module.h>
# include <linux/version.h>
# include <linux/interrupt.h>
# include <linux/pci.h>

# if defined(ME_USB) || defined(ME_MEPHISTO)
#  include <linux/usb.h>
# endif

# include <linux/fs.h>

# include "me_types.h"

# include "meslist.h"
# include "medlock.h"


# if defined(ME_PCI)
/**
	* @brief Struct pci_local_dev holds the PCI device information.
	*/
struct pci_local_dev
{
	struct pci_dev*	dev;					/// Kernel PCI device structure.

	uint16_t 		vendor;					/// Meilhaus PCI vendor id.
	uint16_t		device;					/// Meilhaus device id.
	uint8_t			hw_revision;			/// Hardware revision of the device.
	uint32_t		serial_no;				/// Serial number of the device.
	unsigned int	irq_no;					/// Used interrupt number.
};
typedef struct pci_local_dev me_general_dev_t;
# elif defined(ME_USB)
#  include "NET2282_access.h"

 /**
	* @brief Struct NET2282_usb_device holds the USB device information.
	*/
typedef struct NET2282_usb_device me_general_dev_t;

typedef struct //me_complete_context
{
	wait_queue_head_t irq_internal_queue;		/// Queue to put on threads waiting for an interrupt.
	volatile uint32_t complete_status;
}me_complete_context_t;
# elif defined(ME_MEPHISTO)
#  include "mephisto_access.h"

 /**
	* @brief Struct mephisto_usb_device holds the USB device information.
	*/
typedef mephisto_usb_device_t me_general_dev_t;
# elif defined(ME_COMEDI)
/**
	* @brief Struct comedi_local_dev holds the PCI/Comedi device information.
	*/
struct comedi_local_dev
{
	struct pci_dev*	dev;					/// Kernel PCI device structure.

	uint16_t 		vendor;					/// Meilhaus PCI vendor id.
	uint16_t		device;					/// Meilhaus device id.
	uint8_t			hw_revision;			/// Hardware revision of the device.
	uint32_t		serial_no;				/// Serial number of the device.
	unsigned int	irq_no;					/// Used interrupt number.
};
typedef struct comedi_local_dev me_general_dev_t;

# endif

/**
  * @brief Holds the device information.
  */
typedef struct //me_device_info
{
	me_general_dev_t			local_dev;

	void*			        	PCI_Base[6];

	uint32_t			        bus_no;				/// PCI bus number.
	uint32_t			        dev_no;				/// PCI device number.
	uint32_t 			        func_no;			/// PCI function number.

	int 						plugged;
}me_device_info_t;

typedef void me_interrupt_status_t;
/**
  * @brief The Meilhaus irq context base class structure.
  */
typedef struct //me_irq_context
{
	int no_subdev;
	me_subdevice_t** subdevice;
	me_interrupt_status_t* int_status;			/// For each device one set of interrupt status registers.

	irqreturn_t (*me_device_irq_handle)(int irq, void* context
#  if (LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 19)) && !defined(ME_USB)
												, struct pt_regs* regs
# endif
										);

# if defined(ME_USB)
	struct workqueue_struct* irq_queue;		/// Queue to put on threads waiting for an interrupt.
#  if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,20)
	struct work_struct irq_task;
#  else
	struct delayed_work irq_task;
#  endif

	interrupt_usb_struct_t statin_val;
	struct urb *irq_urb;

	me_complete_context_t complete_context;
# endif

	int unhandled_irq;

}me_irq_context_t;

typedef struct //me_device_ID
{
	char* device_name;							/// The name of the Meilhaus device.
	char* device_description;					/// The description of the Meilhaus device.
	int device_version;							/// The version of driver.
	int device_release;							/// The number of board release.

	char* driver_name;							/// The name of the device driver module supporting the device family.
	char* custom_subname;						/// The custom identifier
}me_device_ID_t;

/**
  * @brief The Meilhaus device base class structure.
  */
typedef struct me_device
{
	/// Attributes
	struct list_head	list;					/// Enables the device to be added to a dynamic list.

	me_device_info_t	bus;					/// Device specific set of information.
	me_irq_context_t	irq_context;			/// Interrupt related specific set of information.

	me_dlock_t			dlock;					/// The device locking structure.
	me_slist_t			slist;					/// The container holding all subdevices belonging to this device.

	me_device_ID_t		info;					/// Set of device general information.

	/// Methods.

	int (*me_device_reinit)(struct me_device* me_device, me_general_dev_t* hw_device);
	void (*me_device_disconnect)(struct me_device* device);
	void (*me_device_destructor)(struct me_device* device);

	int (*me_device_io_irq_start)(
			struct me_device* device,
		   	struct file* filep,
			int subdevice,
		   	int channel,
		   	int irq_source,
		   	int irq_edge,
		   	int irq_arg,
		   	int flags);

	int (*me_device_io_irq_wait)(
			struct me_device* device,
		   	struct file* filep,
			int subdevice,
		   	int channel,
		   	int *irq_count,
		   	int* value,
		   	int time_out,
		   	int flags);

	int (*me_device_io_irq_stop)(
			struct me_device* device,
		   	struct file* filep,
			int subdevice,
			int channel,
		   	int flags);

	int (*me_device_io_irq_test)(
			struct me_device* device,
		   	struct file* filep,
			int subdevice,
			int channel,
		   	int flags);

	int (*me_device_io_reset_device)(
			struct me_device* device,
			struct file* filep,
			int flags);

	int (*me_device_io_reset_subdevice)(
			struct me_device* device,
		   	struct file* filep,
		   	int subdevice,
		   	int flags);

	int (*me_device_io_single_config)(
			struct me_device* device,
			struct file* filep,
			int subdevice,
			int channel,
			int single_config,
			int ref,
			int trig_chain,
			int trig_type,
			int trig_edge,
			int flags);

	int (*me_device_io_single_read)(
			struct me_device* device,
			struct file* filep,
			int subdevice,
			int channel,
			int* value,
			int time_out,
			int flags);

	int (*me_device_io_single_write)(
			struct me_device* device,
			struct file* filep,
			int subdevice,
			int channel,
			int value,
			int time_out,
			int flags);

	int (*me_device_io_stream_config)(
			struct me_device* device,
			struct file* filep,
			int subdevice,
			meIOStreamSimpleConfig_t* config_list,
			int count,
			meIOStreamSimpleTriggers_t* trigger,
			int fifo_irq_threshold,
			int flags);

	int (*me_device_io_stream_new_values)(
			struct me_device* device,
			struct file* filep,
			int subdevice,
			int time_out,
			int *count,
			int flags);

	int (*me_device_io_stream_read)(
			struct me_device* device,
			struct file* filep,
			int subdevice,
			int read_mode,
			int* values,
			int *count,
			int flags);

	int (*me_device_io_stream_timeout_read)(
			struct me_device* device,
			struct file* filep,
			int subdevice,
			int read_mode,
			int* values,
			int *count,
			int timeout,
			int flags);

	int (*me_device_io_stream_start)(
			struct me_device* device,
			struct file* filep,
			int subdevice,
			int start_mode,
			int time_out,
			int flags);

	int (*me_device_io_stream_status)(
			struct me_device* device,
			struct file* filep,
			int subdevice,
			int wait,
			int *status,
			int *count,
			int flags);

	int (*me_device_io_stream_stop)(
			struct me_device* device,
		   	struct file* filep,
			int subdevice,
		   	int stop_mode,
		   	int time_out,
		   	int flags);

	int (*me_device_io_stream_write)(
			struct me_device* device,
			struct file* filep,
			int subdevice,
			int write_mode,
			int* values,
			int *count,
			int flags);

	int (*me_device_io_stream_timeout_write)(
			struct me_device* device,
			struct file* filep,
			int subdevice,
			int write_mode,
			int* values,
			int *count,
			int timeout,
			int flags);

	int (*me_device_set_offset)(
			struct me_device* device,
			struct file* filep,
			int subdevice,
			int channel,
			int range,
			int* offset,
			int flags);

	int (*me_device_lock_device)(
			struct me_device* device,
			struct file* filep,
		   	int lock,
			int flags);

	int (*me_device_lock_subdevice)(
			struct me_device* device,
			struct file* filep,
		   	int subdevice,
		   	int lock,
		   	int flags);

	int (*me_device_query_description_device)(
			struct me_device* device,
			char **description);

	int (*me_device_query_info_device)(
			struct me_device* device,
			int *vendor_id,
			int *device_id,
			int *serial_no,
			int *bus_type,
			int *bus_no,
			int *dev_no,
			int *func_no,
			int *plugged);

	int (*me_device_query_name_device)(
			struct me_device* device,
			char **name);

	int (*me_device_query_name_device_driver)(
			struct me_device* device,
			char **name);

	int (*me_device_query_number_subdevices)(
			struct me_device* device,
			int *number);

	int (*me_device_query_number_subdevices_by_type)(
			struct me_device* device,
			int type,
			int subtype,
			int *number);

	int (*me_device_query_number_channels)(
			struct me_device* device,
			int subdevice, int *number);

	int (*me_device_query_number_ranges)(
			struct me_device* device,
		   	int subdevice,
		   	int unit,
		   	int *count);

	int (*me_device_query_range_by_min_max)(
			struct me_device* device,
			int subdevice,
			int unit,
			int *min,
			int *max,
			int *maxdata,
			int *range);

	int (*me_device_query_range_info)(
			struct me_device* device,
			int subdevice,
		   	int range,
		   	int *unit,
		   	int *min,
		   	int *max,
		   	int *maxdata);

	int (*me_device_query_subdevice_by_type)(
			struct me_device* device,
		   	int start_subdevice,
			int type,
			int subtype,
			int *subdevice);

	int (*me_device_query_subdevice_type)(
			struct me_device* device,
		   	int subdevice,
		   	int *type,
		   	int *subtype);

	int (*me_device_query_subdevice_caps)(
			struct me_device* device,
			int subdevice, int* caps);

	int (*me_device_query_subdevice_caps_args)(
			struct me_device* device,
		   	int subdevice,
		   	int cap,
		   	int* args,
		   	int* count);

	int (*me_device_query_timer)(
			struct me_device* device,
			int subdevice,
			int timer,
			int *base_frequency,
			uint64_t *min_ticks,
			uint64_t *max_ticks);

	int (*me_device_query_version_device_driver)(
			struct me_device* device,
			int *version);

	int (*me_device_query_device_release)(
			struct me_device* device,
			int *version);

	int (*me_device_query_version_firmware)(
			struct me_device* device,
			int subdevice,
			int *version);

	int (*me_device_config_load)(
			struct me_device* device,
			struct file* filep,
			void* config,
			unsigned int size);

	int (*me_device_postinit)(
			struct me_device* device,
			struct file* filep);
} me_device_t;


/**
	* @brief Defines a pointer type to a constructor function.
	*/
typedef struct me_device *(*me_constr_t)(me_general_dev_t *, me_device_t*);

/**
  * @brief Initializes device base class structure.
  *
  * @param me_device The device class to initialize.
  * @param hw_device The PCI device context as handed over by kernel.
  *
  * @return 0 on success.
  */
int me_device_init(me_device_t* me_device, me_general_dev_t* hw_device);

/**
  * @brief Set routines to standart ones.
  *
  * @param me_device The device class to initialize.
  */
void me_install_default_routines(me_device_t* me_device);

/**
  * @brief Initializes interrupts.
  *
  * @param me_device The device class to initialize.
  *
  * @return 0 on success.
  */
int me_device_init_irq(me_device_t* me_device);
/**
  * @brief Re-initializes device base class structure.
  *
  * @param me_device The device class to initialize.
  * @param hw_device The PCI device context as handed over by kernel.
  *
  * @return 0 on success.
  */
int me_device_reinit(me_device_t* me_device, me_general_dev_t* hw_device);

/**
  * @brief Deinitializes a device base class structure.
  *
  * Frees any previously requested resources related with this structure.
  * It also frees any subdevice instance hold by the subdevice list.
  *
  * @param me_device The device class to deinitialize.
  */
void me_device_deinit(me_device_t *me_device);

/**
  * @brief Partly deinitializes a device base class structure.
  *
  * Frees previously requested resources related with connection but keep rest of the context.
  * It also frees interrupt resources.
  *
  * @param me_device The device class to disconnect.
  */
void me_device_disconnect(me_device_t *me_device);

#  if defined(ME_USB)
void NET2282_IRQ_handle(
#   if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,20)
							void *subdevice
#   else
							struct work_struct* work
#   endif
						);
void NET2282_IRQ_complete(struct urb* urb, struct pt_regs* regs);
#  endif

# endif
#endif
