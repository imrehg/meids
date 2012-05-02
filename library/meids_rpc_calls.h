#ifndef __KERNEL__
# ifndef _MEIDS_RPC_CALLS_H_
#  define _MEIDS_RPC_CALLS_H_

#  include "meids_config_structs.h"

/// Library for "EXTERNAL" calls
//Access
int  Open_RPC(me_rpc_context_t* context, const char* address, int iFlags);
int  Close_RPC(me_rpc_context_t* context, int iFlags);

//Lock
int  LockDriver_RPC(void* context, int lock, int iFlags);
int  LockDevice_RPC(void* context, int device, int lock, int iFlags);
int  LockSubdevice_RPC(void* context, int device, int subdevice, int lock, int iFlags);

//Query
int  QueryDriverVersion_RPC(void* context, int* version, int iFlags);
int  QueryDriverName_RPC(void* context, char* name, int count, int iFlags);

int  QuerySubdriverVersion_RPC(void* context, int device, int* version, int iFlags);
int  QuerySubdriverName_RPC(void* context, int device, char *name, int count, int iFlags);

int  QueryDeviceName_RPC(void* context, int device, char *name, int count, int iFlags);
int  QueryDeviceDescription_RPC(void* context, int device, char *description, int count, int iFlags);
int  QueryDevicesNumber_RPC(void* context, int* no_devices, int iFlags);
int  QueryDeviceInfo_RPC(void* context, int device, unsigned int* vendor_id, unsigned int* device_id,
						unsigned int* serial_no, unsigned int* bus_type, unsigned int* bus_no,
						unsigned int* dev_no, unsigned int* func_no, int* plugged, int iFlags);

int  QuerySubdevicesNumber_RPC(void* context, int device, int* no_subdevice, int iFlags);
int  QuerySubdevicesNumberByType_RPC(void* context, int device, int type, int subtype, int* no_subdevices, int iFlags);
int  QuerySubdeviceType_RPC(void* context, int device, int subdevice, int* type, int* subtype, int iFlags);
int  QuerySubdeviceByType_RPC(void* context, int device, int subdevice, int type, int subtype, int* result, int iFlags);
int  QuerySubdeviceCaps_RPC(void* context, int device, int subdevice, int* caps, int iFlags);
int  QuerySubdeviceCapsArgs_RPC(void* context, int device, int subdevice, int cap, int* args, int count, int iFlags);
int  QuerySubdeviceTimer_RPC(void* context, int device, int subdevice, int timer,
															int* base, int* min_ticks_low, int* min_ticks_high, int* max_ticks_low, int* max_ticks_high, int iFlags);

int  QueryChannelsNumber_RPC(void* context, int device, int subdevice, unsigned int* number, int iFlags);

int  QueryRangesNumber_RPC(void* context, int device, int subdevice, int unit, int* no_ranges, int iFlags);
int  QueryRangeInfo_RPC(void* context, int device, int subdevice, int range, int* unit, double *min, double *max, unsigned int* max_data, int iFlags);
int  QueryRangeByMinMax_RPC(void* context, int device, int subdevice, int unit, double *min, double *max, int* max_data, int* range, int iFlags);

//Input/Output
int  IrqStart_RPC(void* context, int device, int subdevice, int channel, int source, int edge, int arg, int iFlags);
int  IrqWait_RPC(void* context, int device, int subdevice, int channel, int* count, int* value, int timeout, int iFlags);
int  IrqStop_RPC(void* context, int device, int subdevice, int channel, int iFlags);
int  IrqTest_RPC(void* context, int device, int subdevice, int channel, int iFlags);

int  IrqSetCallback_RPC(void* context, int device, int subdevice, meIOIrqCB_t irq_fn, void* irq_context, int iFlags);

int  ResetDevice_RPC(void* context, int device, int iFlags);
int  ResetSubdevice_RPC(void* context, int device, int subdevice, int iFlags);

int  SingleConfig_RPC(void* context, int device, int subdevice, int channel,
                		int config, int reference, int synchro,
                     	int trigger, int edge,	int iFlags);
int  Single_RPC(void* context, int device, int subdevice, int channel, int direction, int* value, int timeout, int iFlags);
int  SingleList_RPC(void* context, meIOSingle_t* list, int count, int iFlags);

int  StreamConfigure_RPC(void* context, int device,int subdevice, meIOStreamSimpleConfig_t* list, int count, meIOStreamSimpleTriggers_t* trigger, int threshold, int iFlags);
int  StreamConfig_RPC(void* context, int device,int subdevice, meIOStreamConfig_t* list, int count, meIOStreamTrigger_t* trigger, int threshold, int iFlags);
int  StreamStart_RPC(void* context, int device, int subdevice, int mode, int timeout, int iFlags);
int  StreamStartList_RPC(void* context, meIOStreamStart_t* list, int count, int iFlags);
int  StreamStop_RPC(void* context, int device, int subdevice, int mode, int, int iFlags);
int  StreamStopList_RPC(void* context, meIOStreamStop_t* list, int count, int iFlags);
int  StreamNewValues_RPC(void* context, int device, int subdevice, int timeout, int* count, int iFlags);
int  StreamRead_RPC(void* context,  int device, int subdevice, int mode, int* values, int* count, int timeout, int iFlags);
int  StreamWrite_RPC(void* context, int device, int subdevice, int mode, int* values, int* count, int timeout, int iFlags);
int  StreamStatus_RPC(void* context, int device, int subdevice, int wait, int* status, int* count, int iFlags);

int  StreamSetCallbacks_RPC(void* context,
							int device, int subdevice,
							meIOStreamCB_t start, void* start_context,
							meIOStreamCB_t new_values, void* new_value_context,
							meIOStreamCB_t end, void* end_context,
							int iFlags);

int  StreamTimeToTicks_RPC(void* context, int device, int subdevice, int timer, double* stream_time, int* ticks_low, int* ticks_high, int iFlags);
int  StreamFrequencyToTicks_RPC(void* context, int device, int subdevice, int timer, double* frequency, int* ticks_low, int* ticks_high, int iFlags);

int  ParametersSet_RPC(void* context, int device, me_extra_param_set_t* paramset, int flags);

int SetOffset_RPC(void* context, int device, int subdevice, int channel, int range, double* offset, int iFlags);

# endif	//_MEIDS_RPC_CALLS_H_
#endif	//__KERNEL__
