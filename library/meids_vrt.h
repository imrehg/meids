#ifndef __KERNEL__
# ifndef _MEIDS_VRT_H_
#  define _MEIDS_VRT_H_

#  include "meids_config_structs.h"

/// Calls via vrtual table
//Lock
int  ME_virtual_LockDriver(void* context, int lock, int iFlags);
int  ME_virtual_LockDevice(const me_config_t* cfg, int device, int lock, int iFlags);
int  ME_virtual_LockSubdevice(const me_config_t* cfg, int device, int subdevice, int lock, int iFlags);

//Query
int  ME_virtual_QueryDriverVersion(const me_config_t* cfg, int device, int* version, int iFlags);
int  ME_virtual_QueryDriverName(const me_config_t* cfg, int device, char* name, int count, int iFlags);

int  ME_virtual_QuerySubdriverVersion(const me_config_t* cfg, int device, int* version, int iFlags);
int  ME_virtual_QuerySubdriverName(const me_config_t* cfg, int device, char* name, int count, int iFlags);

int  ME_virtual_QueryDeviceName(const me_config_t* cfg, int device, char* name, int count, int iFlags);
int  ME_virtual_QueryDeviceDescription(const me_config_t* cfg, int device, char* description, int count, int iFlags);
int  ME_virtual_QueryDeviceInfo(const me_config_t* cfg, int device, unsigned int* vendor_id, unsigned int* device_id,
						unsigned int* serial_no, unsigned int* bus_type, unsigned int* bus_no,
						unsigned int* dev_no, unsigned int* func_no, int* plugged, int iFlags);

int  ME_virtual_QuerySubdevicesNumber(const me_config_t* cfg, int device, int* no_subdevice, int iFlags);
int  ME_virtual_QuerySubdevicesNumberByType(const me_config_t* cfg, int device, int type, int subtype, int* no_subdevice, int iFlags);
int  ME_virtual_QuerySubdeviceType(const me_config_t* cfg, int device, int subdevice, int* type, int* subtype, int iFlags);
int  ME_virtual_QuerySubdeviceByType(const me_config_t* cfg, int device, int subdevice, int type, int subtype, int* result, int iFlags);
int  ME_virtual_QuerySubdeviceCaps(const me_config_t* cfg, int device, int subdevice, int* caps, int iFlags);
int  ME_virtual_QuerySubdeviceCapsArgs(const me_config_t* cfg, int device, int subdevice, int cap, int* args, int count, int iFlags);

int  ME_virtual_QueryChannelsNumber(const me_config_t* cfg, int device, int subdevice, unsigned int* number, int iFlags);

int  ME_virtual_QueryRangesNumber(const me_config_t* cfg, int device, int subdevice, int unit, int* no_ranges, int iFlags);
int  ME_virtual_QueryRangeInfo(const me_config_t* cfg, int device, int subdevice, int range, int* unit, double* min, double* max, unsigned int* max_data, int iFlags);
int  ME_virtual_QueryRangeByMinMax(const me_config_t* cfg, int device, int subdevice, int unit, double* min, double* max, int* max_data, int* range, int iFlags);
int  ME_virtual_QuerySubdeviceTimer(const me_config_t* cfg, int device, int subdevice, int timer, int* base, int* min_ticks_low, int* min_ticks_high, int* max_ticks_low, int* max_ticks_high, int iFlags);

//Input/Output
int  ME_virtual_IrqStart(const me_config_t* cfg, int device, int subdevice, int channel, int source, int edge, int arg, int iFlags);
int  ME_virtual_IrqWait(const me_config_t* cfg, int device, int subdevice, int channel, int* count, int* value, int timeout, int iFlags);
int  ME_virtual_IrqStop(const me_config_t* cfg, int device, int subdevice, int channel, int iFlags);
int  ME_virtual_IrqTest(const me_config_t* cfg, int device, int subdevice, int channel, int iFlags);

int  ME_virtual_IrqSetCallback(const me_config_t* cfg, int device, int subdevice, meIOIrqCB_t irq_fn, void* irq_context, int iFlags);

int  ME_virtual_ResetDevice(const me_config_t* cfg, int device, int iFlags);
int  ME_virtual_ResetSubdevice(const me_config_t* cfg, int device, int subdevice, int iFlags);

int  ME_virtual_SingleConfig(const me_config_t* cfg, int device, int subdevice, int channel,
                		int config, int reference, int synchro,
                     	int trigger, int edge,	int iFlags);
int  ME_virtual_Single(const me_config_t* cfg, int device, int subdevice, int channel, int direction, int* value, int timeout, int iFlags);
int  ME_virtual_SingleList(const me_config_t* cfg, meIOSingle_t* list, int count, int iFlags);

/// Universal call - old triggers' structure.
int  ME_virtual_StreamConfig(const me_config_t* cfg, int device,int subdevice, meIOStreamConfig_t* list, int count, meIOStreamTrigger_t* trigger, int threshold, int iFlags);
/// Local call - new triggers' structure.
int  ME_virtual_StreamConfigure(const me_config_t* cfg, int device,int subdevice, meIOStreamSimpleConfig_t* list, int count, meIOStreamSimpleTriggers_t* trigger, int threshold, int iFlags);
int  ME_virtual_StreamNewValues(const me_config_t* cfg, int device, int subdevice, int timeout, int* count, int iFlags);
int  ME_virtual_StreamRead(const me_config_t* cfg,  int device, int subdevice, int mode, int* values, int* count, int timeout, int iFlags);
int  ME_virtual_StreamWrite(const me_config_t* cfg, int device, int subdevice, int mode, int* values, int* count, int timeout, int iFlags);
int  ME_virtual_StreamStart(const me_config_t* cfg, int device, int subdevice, int mode, int timeout, int iFlags);
int  ME_virtual_StreamStartList(const me_config_t* cfg, meIOStreamStart_t* list, int count, int iFlags);
int  ME_virtual_StreamStatus(const me_config_t* cfg, int device, int subdevice, int wait, int* status, int* count, int iFlags);
int  ME_virtual_StreamStop(const me_config_t* cfg, int device, int subdevice, int mode, int timeout, int iFlags);
int  ME_virtual_StreamStopList(const me_config_t* cfg, meIOStreamStop_t* list, int count, int iFlags);

int  ME_virtual_StreamSetCallbacks(const me_config_t* cfg,
							int device, int subdevice,
							meIOStreamCB_t start, void* start_context,
							meIOStreamCB_t new_values, void* new_value_context,
							meIOStreamCB_t end, void* end_context,
							int iFlags);

int  ME_virtual_StreamTimeToTicks(const me_config_t* cfg, int device, int subdevice, int timer, double* stream_time, int* ticks_low, int* ticks_high, int iFlags);
int  ME_virtual_StreamFrequencyToTicks(const me_config_t* cfg, int device, int subdevice, int timer, double* frequency, int* ticks_low, int* ticks_high, int iFlags);

int ME_virtual_ParametersSet(const me_config_t* cfg, me_extra_param_set_t* paramset, int flags);

int ME_virtual_SetOffset(const me_config_t* cfg, int device, int subdevice, int channel, int range, double* offset, int iFlags);

# endif	//_MEIDS_VRT_H_
#else
# error KERNEL???
#endif	//__KERNEL__
