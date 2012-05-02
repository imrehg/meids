#ifndef __KERNEL__
# ifndef _MEIDS_HEADER_H_
#  define _MEIDS_HEADER_H_

#  include "meids_config_structs.h"

int  ME_Open(char* address, int iFlags);
int  ME_Close(int iFlags);

int  ME_QueryDevicesNumber(int* no_devices, int iFlags);
int  ME_QueryLibraryVersion(int* version, int iFlags);

int  ME_ConfigRead(me_config_t* cfg, const char* address, int flags);

//Lock
int  ME_LockAll(int lock, int iFlags);
int  ME_LockDevice(int device, int lock, int iFlags);
int  ME_LockSubdevice(int device, int subdevice, int lock, int iFlags);

//Query
int  ME_QueryDriverVersion(int device, int* version, int iFlags);
int  ME_QueryDriverName(int device, char* name, int count, int iFlags);

int  ME_QuerySubdriverVersion(int device, int* version, int iFlags);
int  ME_QuerySubdriverName(int device, char* name, int count, int iFlags);

int  ME_QueryDeviceName(int device, char* name, int count, int iFlags);
int  ME_QueryDeviceDescription(int device, char* description, int count, int iFlags);
int  ME_QueryDeviceInfo(int device, unsigned int* vendor_id, unsigned int* device_id,
						unsigned int* serial_no, unsigned int* bus_type, unsigned int* bus_no,
						unsigned int* dev_no, unsigned int* func_no, int* plugged, int iFlags);

int  ME_QuerySubdevicesNumber(int device, int* no_subdevice, int iFlags);
int  ME_QuerySubdevicesNumberByType(int device, int type, int subtype, int* no_subdevice, int iFlags);
int  ME_QuerySubdeviceType(int device, int subdevice, int* type, int* subtype, int iFlags);
int  ME_QuerySubdeviceByType(int device, int subdevice, int type, int subtype, int* result, int iFlags);
int  ME_QuerySubdeviceCaps(int device, int subdevice, int* caps, int iFlags);
int  ME_QuerySubdeviceCapsArgs(int device, int subdevice, int cap, int* args, int count, int iFlags);

int  ME_QueryChannelsNumber(int device, int subdevice, unsigned int* number, int iFlags);

int  ME_QueryRangesNumber(int device, int subdevice, int unit, int* no_ranges, int iFlags);
int  ME_QueryRangeInfo(int device, int subdevice, int range, int* unit, double* min, double* max, unsigned int* max_data, int iFlags);
int  ME_QueryRangeByMinMax(int device, int subdevice, int unit, double* min, double* max, int* max_data, int* range, int iFlags);
int  ME_QuerySubdeviceTimer(int device, int subdevice, int timer, int* base, int* min_ticks_low, int* min_ticks_high, int* max_ticks_low, int* max_ticks_high, int iFlags);

//Input/Output
int  ME_IrqStart(int device, int subdevice, int channel, int source, int edge, int arg, int iFlags);
int  ME_IrqWait(int device, int subdevice, int channel, int* count, int* value, int timeout, int iFlags);
int  ME_IrqStop(int device, int subdevice, int channel, int iFlags);
int  ME_IrqTest(int device, int subdevice, int channel, int iFlags);

int  ME_IrqSetCallback(int device, int subdevice, meIOIrqCB_t pIrqCB, void* pContext, int iFlags);

int  ME_ResetDevice(int device, int iFlags);
int  ME_ResetSubdevice(int device, int subdevice, int iFlags);

int  ME_SingleConfig(int device, int subdevice, int channel,
                		int config, int reference, int synchro,
                     	int trigger, int edge,	int iFlags);
int  ME_Single(int device, int subdevice, int channel, int direction, int* value, int timeout, int iFlags);
int  ME_SingleList(meIOSingle_t* list, int count, int iFlags);

/// Universal call - old triggers' structure.
int  ME_StreamConfig(int device,int subdevice, meIOStreamConfig_t* list, int count, meIOStreamTrigger_t* trigger, int threshold, int iFlags);
/// Local call - new triggers' structure.
int  ME_StreamConfigure(int device,int subdevice, meIOStreamSimpleConfig_t* list, int count, meIOStreamSimpleTriggers_t* trigger, int threshold, int iFlags);
int  ME_StreamNewValues(int device, int subdevice, int timeout, int* count, int iFlags);
int  ME_StreamRead( int device, int subdevice, int mode, int* values, int* count, int timeout, int iFlags);
int  ME_StreamWrite(int device, int subdevice, int mode, int* values, int* count, int timeout, int iFlags);
int  ME_StreamStart(int device, int subdevice, int mode, int timeout, int iFlags);
int  ME_StreamStartList(meIOStreamStart_t* list, int count, int iFlags);
int  ME_StreamStatus(int device, int subdevice, int wait, int* status, int* count, int iFlags);
int  ME_StreamStop(int device, int subdevice, int mode, int timeout, int iFlags);
int  ME_StreamStopList(meIOStreamStop_t* list, int count, int iFlags);

int  ME_StreamSetCallbacks(int device, int subdevice,
							meIOStreamCB_t start, void* start_context,
							meIOStreamCB_t new_values, void* new_value_context,
							meIOStreamCB_t end, void* end_context,
							int iFlags);

int  ME_StreamTimeToTicks(int device, int subdevice, int timer, double* stream_time, int* ticks_low, int* ticks_high, int iFlags);
int  ME_StreamFrequencyToTicks(int device, int subdevice, int timer, double* frequency, int* ticks_low, int* ticks_high, int iFlags);

int  ME_SetOffset(int device, int subdevice, int channel, int range, double* offset, int iFlags);

void ME_ConfigPrint(void);

void ME_SetErrno(char* text, int err);
int  ME_GetErrno(void);

int  ME_ParametersSet(me_extra_param_set_t* paramset, int flags);

# endif	//_MEIDS_HEADER_H_
#else
# error KERNEL???
#endif	//__KERNEL__
