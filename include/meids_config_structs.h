#ifndef _MEIDS_CONFIG_STRUCTS_H_
# define _MEIDS_CONFIG_STRUCTS_H_

# include <rpc/rpc.h>

# include <meids_pthread.h>
# include <meids_tcp.h>
# include <me_structs.h>

typedef struct meids_calls
{
//Lock
	int  (*LockDriver)(void*, int, int);
	int  (*LockDevice)(void*, int, int, int);
	int  (*LockSubdevice)(void*, int, int, int, int);

//Query
	int  (*QueryDriverVersion)(void*, int*, int);
	int  (*QueryDriverName)(void*, char*, int, int);

	int  (*QuerySubdriverVersion)(void*, int, int*, int);
	int  (*QuerySubdriverName)(void*, int, char*, int, int);

	int  (*QueryDeviceName)(void*, int, char*, int, int);
	int  (*QueryDeviceDescription)(void*, int, char*, int, int);
	int  (*QueryDeviceInfo)(void*, int, unsigned int*, unsigned int*, unsigned int*, unsigned int*, unsigned int*, unsigned int*, unsigned int*, int*, int);

	int  (*QuerySubdevicesNumber)(void*, int, int*, int);
	int  (*QuerySubdevicesNumberByType)(void*, int, int, int, int*, int);
	int  (*QuerySubdeviceType)(void*, int, int, int*, int*, int);
	int  (*QuerySubdeviceByType)(void*, int, int, int, int, int*, int);
	int  (*QuerySubdeviceCaps)(void*, int, int, int*, int);
	int  (*QuerySubdeviceCapsArgs)(void*, int, int, int, int*, int, int);

	int  (*QueryChannelsNumber)(void*, int, int, unsigned int*, int);

	int  (*QueryRangesNumber)(void*, int, int, int, int*, int);
	int  (*QueryRangeInfo)(void*, int, int, int, int*, double* min, double*, unsigned int*, int);
	int  (*QueryRangeByMinMax)(void*, int, int, int, double* min, double*, int*, int*, int);
	int  (*QuerySubdeviceTimer)(void*, int, int, int, int*, int*, int*, int*, int*, int);

//Input/Output
	int  (*IrqStart)(void*, int, int, int, int, int, int, int);
	int  (*IrqWait)(void*, int, int, int, int*, int*, int, int);
	int  (*IrqStop)(void*, int, int, int, int);
	int  (*IrqTest)(void*, int, int, int, int);

	int  (*IrqSetCallback)(void*, int, int, meIOIrqCB_t, void*, int);

	int  (*ResetDevice)(void*, int, int);
	int  (*ResetSubdevice)(void*, int, int, int);

	int  (*SingleConfig)(void*, int, int, int, int, int, int, int, int,	int);
	int  (*Single)(void*, int, int, int, int, int*, int, int);
	int  (*SingleList)(void*, meIOSingle_t*, int, int);

	int  (*StreamConfig)(void*, int, int, meIOStreamConfig_t*, int, meIOStreamTrigger_t*, int, int);
	int  (*StreamConfigure)(void*, int,int, meIOStreamSimpleConfig_t*, int, meIOStreamSimpleTriggers_t*, int, int);
	int  (*StreamNewValues)(void*, int, int, int, int*, int);
	int  (*StreamRead)(void*, int, int, int, int*, int*, int, int);
	int  (*StreamWrite)(void*, int, int, int, int*, int*, int, int);
	int  (*StreamStart)(void*, int, int, int, int, int);
	int  (*StreamStartList)(void*, meIOStreamStart_t*, int, int);
	int  (*StreamStatus)(void*, int, int, int, int*, int*, int);
	int  (*StreamStop)(void*, int, int, int, int, int);
	int  (*StreamStopList)(void*, meIOStreamStop_t*, int, int);

	int  (*StreamSetCallbacks)(void*, int, int, meIOStreamCB_t, void*, meIOStreamCB_t, void*, meIOStreamCB_t, void*, int);

	int  (*StreamTimeToTicks)(void*, int, int, int, double*, int*, int*, int);
	int  (*StreamFrequencyToTicks)(void*, int, int, int, double*, int*, int*, int);

	int  (*SetOffset)(void*, int, int, int, int, double*, int);

	int  (*ParametersSet)(void*, int, me_extra_param_set_t*, int);
} meids_calls_t;

///Thread context structures
typedef struct threadsList
{
	struct threadsList*	next;

	pthread_t 			threadID;

	int					device;
	int					subdevice;
	void*				context;

	volatile int		cancel;
}
threadsList_t;

typedef struct threadContext
{
	threadsList_t* instance;
	union
	{
		meIOStreamCB_t	streamCB;
		meIOIrqCB_t		irqCB;
		void*			fnCB;
	};

	void*	contextCB;
	int		flags;
}threadContext_t;

/// Context stuctures. There are all internal information.
typedef struct ME_Context_list
{
	struct ME_Context_list* next;
	void* context;
}me_context_list_t;

typedef enum me_context_type
{
	me_context_type_invalid = 0,	//Begin of enumeration
	me_context_type_local,		//PCI and USB
	me_context_type_remote,		//RPC
	me_context_type_max			//End of enumeration
}me_context_type_t;

typedef struct ME_Dummy_Context
{
	/// @note Table of calls MUST BE a first element in context struct!
	meids_calls_t* context_calls;
	me_context_type_t context_type;
}me_dummy_context_t;

typedef struct ME_Local_Context
{
	/// @note Table of calls MUST BE a first element in context struct!
	meids_calls_t* context_calls;
	me_context_type_t context_type;
	//Driver's file descriptor
	int fd;

	pthread_mutex_t callbackContextMutex;
	threadsList_t* activeThreads;
}me_local_context_t;

typedef struct ME_RPC_SubdevContext
{
	// RPC client
	CLIENT* fd;
	// Protect RPC context
	pthread_mutex_t rpc_mutex;

}me_rpc_subdevcontext_t;

typedef struct ME_RPC_DevContext
{
	// RPC client
	CLIENT* fd;
	// Protect RPC context
	pthread_mutex_t rpc_mutex;

	int count;
	me_rpc_subdevcontext_t* subdevice_context;
}me_rpc_devcontext_t;

typedef struct ME_RPC_Context
{
	/// @note Table of calls MUST BE a first element in context struct!
	meids_calls_t* context_calls;
	me_context_type_t context_type;
	// RPC client
	CLIENT* fd;
	// Protect RPC context
	pthread_mutex_t rpc_mutex;

	pthread_mutex_t callbackContextMutex;
	threadsList_t* activeThreads;
	char* access_point_addr;

#if defined RPC_USE_SUBCONTEXT
	int count;
	me_rpc_devcontext_t* device_context;
#endif
	pid_t pid;
}me_rpc_context_t;

typedef enum me_cfg_extention_type
{
	me_cfg_extention_type_invalid =		0x00000000,
	me_cfg_extention_type_min = 		0x001C0000,	//Begin of enumeration
	me_cfg_extention_type_none,
	me_cfg_extention_type_mux32,
	me_cfg_extention_type_demux32,
	me_cfg_extention_type_max						//End of enumeration
}me_cfg_extention_type_t;

typedef enum me_access_type
{
	me_access_type_invalid =	0x00000000,
	me_access_type_min = 		0x001D0000,	//Begin of enumeration
	me_access_type_PCI,						//Local PCI and ePCI boards
	me_access_type_TCPIP,					//Synapse-LAN
	me_access_type_USB,						//Synapse-USB & Mephisto-Family
	me_access_type_USB_MephistoScope,		//Mephisto-Scope
	me_access_type_max						//End of enumeration
}me_access_type_t;

typedef enum me_units_type
{
	me_units_type_invalid =		0x00000000,
	me_units_type_min = 		0x00170000,	//Begin of enumeration
	me_units_type_volts,
	me_units_type_ampers,
	me_units_type_any,
	me_units_type_hertz,
	me_units_type_max						//End of enumeration
}
me_units_type_t;

typedef enum me_plugged_type
{

	me_plugged_type_invalid =		0x00000000,
	me_plugged_type_min = 			0x001B0000,	//Begin of basic section
	me_plugged_type_IN,
	me_plugged_type_OUT,
	me_plugged_type_BLOCKED,					//This entry is just a dummy.
	me_plugged_type_USED,						//This is for marking that entry was already used an shouldn't be choose again.
	me_plugged_type_CONTROL_min = 	0x001B8000,	//Begin of control fields 14 bits
	me_plugged_type_max= 			0x001BC000,
}
me_plugged_type_t;

// Transport layer. Hardware specification.

typedef struct me_cfg_usb_hw_info
{
	unsigned int root_hub_no;
} me_cfg_usb_hw_info_t;


// Extensions layer.
typedef struct me_cfg_mux32_device
{
	int type;
	int timed;
	unsigned int ai_channel;
	unsigned int dio_device;
	unsigned int dio_subdevice;
	unsigned int timer_device;
	unsigned int timer_subdevice;
	unsigned int mux32s_count;
} me_cfg_mux32_device_t;

typedef struct me_cfg_demux32_device
{
	int type;
	int timed;
	unsigned int ao_channel;
	unsigned int dio_device;
	unsigned int dio_subdevice;
	unsigned int timer_device;
	unsigned int timer_subdevice;
} me_cfg_demux32_device_t;

typedef struct me_cfg_extention
{
	me_cfg_extention_type_t type;
	union
	{
		me_cfg_mux32_device_t mux32;
		me_cfg_demux32_device_t demux32;
	};
}me_cfg_extention_t;


// Subdevice layer
typedef struct me_cfg_range_info
{
	me_units_type_t unit;
	double min;
	double max;
	unsigned int max_data;
}
me_cfg_range_info_t;

typedef struct me_cfg_subdevice_info
{
	int type;
	int sub_type;
	unsigned int channels;

	me_cfg_range_info_t** range_list;
	unsigned int range_list_count;
}
me_cfg_subdevice_info_t;

typedef struct me_cfg_subdevice_entry
{
	int locked;
	me_cfg_subdevice_info_t info;

	me_cfg_extention_t extention;
}
me_cfg_subdevice_entry_t;

// Device layer

typedef struct me_cfg_pci_hw_info
{
	unsigned int bus_no;
	unsigned int device_no;
	unsigned int function_no;
} me_cfg_pci_hw_info_t;


typedef struct me_cfg_device_info
{
	// Common information
	unsigned int vendor_id;
	unsigned int device_id;
	unsigned int serial_no;

	char* device_name;
	char* device_description;

	int device_no;							// 'REAL' device number

	union
	{										// Hardware specific information (egz.: address).
		me_cfg_pci_hw_info_t pci;
		me_cfg_tcpip_hw_info_t tcpip;
		me_cfg_usb_hw_info_t usb;
	};
}
me_cfg_device_info_t;

typedef struct me_cfg_device_entry
{
	// Bus specific context.
	void* context;

	// Info section
	me_access_type_t access_type;
	int logical_device_no;					// 'LOGICAL' device number
	me_cfg_device_info_t info;
	me_plugged_type_t plugged;				//Control field for forceing/blocking entries.

	//Subdevice section
	me_cfg_subdevice_entry_t** subdevice_list;
	unsigned int subdevice_list_count;
}
me_cfg_device_entry_t;

typedef struct me_config
{// Static config structure
	me_cfg_device_entry_t** device_list;
	unsigned int device_list_count;
}
me_config_t;

# define CHECK_POINTER(pointer) \
if (!pointer) \
{ \
	LIBPERROR("Invalid pointer.\n"); \
	return ME_ERRNO_INVALID_POINTER;	\
}

#endif	//_MEIDS_CONFIG_STRUCTS_H_

