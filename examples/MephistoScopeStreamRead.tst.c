/// MephistoScopeStreamRead.tst.c : linux copy of Windows console file Con_meIOMEPhistoScopeStreamRead.cpp
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/select.h>
#include <sys/time.h>
#include <sys/types.h>

#include <medriver.h>


typedef unsigned short bool ;

#define false 0
#define true 1
#define strnicmp strncmp
#define getch getchar
#define _getch  getch
#define _kbhit kbhit

#define MODE_ANALOG_OSCILLOSCOPE								1
#define MODE_ANALOG_DATA_LOGGER									2
#define MODE_DIGITAL_OSCILLOSCOPE								3
#define MODE_DIGITAL_DATA_LOGGER								4

#define ANALOG_TRIGGER_TYPE_MANUAL								1
#define ANALOG_TRIGGER_TYPE_THRESHOLD_ABOVE						2
#define ANALOG_TRIGGER_TYPE_THRESHOLD_BELOW						3
#define ANALOG_TRIGGER_TYPE_WINDOW_ENTER						4
#define ANALOG_TRIGGER_TYPE_WINDOW_LEAVE						5
#define ANALOG_TRIGGER_TYPE_EDGE_RISING							6
#define ANALOG_TRIGGER_TYPE_EDGE_FALLING						7
#define ANALOG_TRIGGER_TYPE_SLOPE_POSITIVE						8
#define ANALOG_TRIGGER_TYPE_SLOPE_NEGATIVE						9
#define ANALOG_TRIGGER_TYPE_EXTERNAL_DIGITAL_EDGE_RISING		10
#define ANALOG_TRIGGER_TYPE_EXTERNAL_DIGITAL_EDGE_FALLING		11

#define DIGITAL_TRIGGER_TYPE_MANUAL								101
#define DIGITAL_TRIGGER_TYPE_EXTERNAL_DIGITAL_EDGE_RISING		102
#define DIGITAL_TRIGGER_TYPE_EXTERNAL_DIGITAL_EDGE_FALLING		103
#define DIGITAL_TRIGGER_TYPE_BIT_PATTERN						104

int iOscilloscopeBufferSize = 0;

int* piOscilloscopeBuffer = NULL;

int kbhit(void) ;


int main(int argc, char* argv[])
{
	// Call generic console function in parent directory to
	// open system and list the available devices and subdevices.

	int i_number_of_devices = 0;

        int LibVer;
        int DrvVer;

        printf("Hello, AI stream test!\n");

        if (meOpen(0))
                exit (EXIT_FAILURE);

        if (!meQueryVersionLibrary(&LibVer))
                printf("Library version: 0x%08x.\n", LibVer);

        if (!meQueryVersionMainDriver(&DrvVer))
                printf("Main driver version: 0x%08x.\n", DrvVer);

        if (!meQueryNumberDevices(&i_number_of_devices))
                printf("%d device%s in system.\n", i_number_of_devices, (i_number_of_devices>1)?"s":"");

	if(i_number_of_devices <= 0)
	{
		return(-1);
	}

	// ---------------------------------------------------------------------

	int i_mephisto_scope_device	= -1;
	int i_stream_read_subdevice = -1;

	// Search for a MEphisto Scope device in the system

	int i_me_error;

	// Iterate through the devices in the system

	int index_device = 0 ;
	for( index_device = 0; index_device < i_number_of_devices; index_device++)
	{
		char sz_device_name[ME_DEVICE_NAME_MAX_COUNT];

		// Query the device name

		i_me_error = meQueryNameDevice(index_device, &sz_device_name[0], ME_DEVICE_NAME_MAX_COUNT);

		// If the device name begins with "MEphisto Scope" (for example "MEphisto Scope 1") then we have found a suitable device

		if( strnicmp("MephistoScope", &sz_device_name[0], strlen("MephistoScope") ) == 0 )
		{
			// We've found a MEPhisto Scope device, make sure it is 'plugged in', that's to say actually present in the system.

			int i_vendor_id;
			int i_device_id;
			int i_serial_no;
			int i_bus_type;
			int i_bus_no;
			int i_dev_no;
			int i_func_no;
			int i_plugged;

			i_me_error = meQueryInfoDevice(	index_device,				// Device index
											&i_vendor_id,				// Vendor ID returned here - not required
											&i_device_id,				// Device ID returned here - not required
											&i_serial_no,				// Serial number returned here - not required
											&i_bus_type,				// Bus type returned here - not required
											&i_bus_no,					// Bus number returned here - not required
											&i_dev_no,					// Bus device number returned here - not required
											&i_func_no,					// Bus function number returned here - not required
											&i_plugged				);	// Plugged status returned hee - not required

			if(i_plugged != ME_PLUGGED_IN)
			{
				// The device isn't present at the moment, continue the search.

				continue;
			}

			// Search for the AI-Streaming sub-device on this device.
			// If we know exactly which device we're dealing with then we would have this information anyway, but this
			// is the proper, flexible way to do things.

			i_me_error = meQuerySubdeviceByType(index_device,					// Device
												0,								// Begin search at this sub-device
												ME_TYPE_AI,						// Type of sub-device to search for
												ME_SUBTYPE_STREAMING,			// Sub-type of sub-device to search for
												&i_stream_read_subdevice	);	// sub-device index returned here

			if(i_me_error == ME_ERRNO_SUCCESS)
			{
				// In fact, since a MEphisto Scope always has an AI-Streaming sub.device, we don't really need to check
				// for success, but once again this is the flexible way to do things

				i_mephisto_scope_device = index_device;

				break;
			}
		}
	}

	if(i_mephisto_scope_device == -1)
	{
		printf("****    No MEphisto Scope device with AI-Streaming sub-device found in system    ****\n\n");

		printf("Press any key to terminate\n");

		_getch();

		meClose(0);

		return -1;
	}

	printf("Device: %d is a MEphisto Scope device and Subdevice: %d is its AI-Streaming sub-device\n\n", i_mephisto_scope_device, i_stream_read_subdevice);

	bool b_terminate = false;

	char buffer[128];

	while(true)
	{
		int i_mode = 0;

		// Let the user choose the mode

		while(true)
		{
			printf(	"Operation mode:\n\n"
					"1) Analog Oscilloscope\n"
					"2) Analog Data Logger\n"
					"3) Digital Oscilloscope (Logic Analyzer)\n"
					"4) Digital Data Logger\n\n"	);

			printf("\nYour choice (1 - 4) (RETURN to terminate): ");

			gets(&buffer[0]);

			printf("\n\n");

			if(buffer[0] == 0)
			{
				b_terminate = true;
				printf("Terminate\n\n");

				break;
			}

			if( (sscanf(&buffer[0], "%d", &i_mode) != 1 ) )
			{
				continue;
			}

			if( (i_mode < 1)||(i_mode > 4) )
			{
				continue;
			}

			break;
		}

		if(b_terminate)
		{
			break;
		}

		int i_number_of_channels_to_sample = 0;

		int i_sample_channels[2];

		int i_channel_range[2];

		double d_channel_physical_min[2];

		double d_channel_physical_max[2];

		double d_channel_offset[2];

		int i_trigger_type;

		int i_trigger_channel;

		int i_trigger_args[2] = {0,0};

		int i_trigger_timeout_seconds;

		int i_memory_depth;

		int i_trigger_point_percent;

		int i_bit_pattern;

		if( (i_mode == MODE_ANALOG_OSCILLOSCOPE)||(i_mode == MODE_ANALOG_DATA_LOGGER) )
		{
			// Let the user choose which channels to sample

			// Query the available ranges so that we can present the user with a list

			// First query the number of the available ranges

			int i_number_of_ranges;

			i_me_error =  meQueryNumberRanges(	i_mephisto_scope_device,		// Device index
												i_stream_read_subdevice,		// Subdevice index,
												ME_UNIT_ANY,					// Unit
												&i_number_of_ranges		);		// Number of ranges returned here


			if(i_me_error != ME_ERRNO_SUCCESS)
			{
				printf("****    meQueryNumberRanges - Error: %d    ****\n\n", i_me_error);

				printf("Press any key to terminate\n");

				_getch();

				meClose(0);

				return -1;
			}

			// Now make a list of the ranges

			double* p_d_range_physical_minimum = (double*) malloc(sizeof(double)*i_number_of_ranges);

			double* p_d_range_physical_maximum = (double*) malloc(sizeof(double)*i_number_of_ranges);

			int index_range = 0 ;
			for( index_range = 0; index_range < i_number_of_ranges; index_range++)
			{
				int i_unit;

				int i_digital_max;

				i_me_error = meQueryRangeInfo(	i_mephisto_scope_device,						// Device index
												i_stream_read_subdevice,						// Subdevice index,
												index_range,									// Range index
												&i_unit,										// Unit returned here - not require
												&p_d_range_physical_minimum[index_range],		// Physical minimum returned here
												&p_d_range_physical_maximum[index_range],		// Physical maximum returned here
												&i_digital_max								);	// Digital maximum value returned here

				if(i_me_error != ME_ERRNO_SUCCESS)
				{
					printf("****    meQueryRangeInfo - Error: %d    ****\n\n", i_me_error);

					printf("Press any key to terminate\n");

					_getch();

					meClose(0);

					return -1;
				}
			}

			// Ask the user for each channel in turn whether he wishes to sample it, and if so which range he wants to use

			int index_channel = 0;
			for( index_channel = 0; index_channel < 2; index_channel++)
			{
				while(true)
				{
					printf("Sample Channel %d (Y/N) (RETURN to terminate): ", index_channel + 1);

					gets(&buffer[0]);

					printf("\n\n");

					if(buffer[0] == 0)
					{
						b_terminate = true;
						printf("Terminate\n\n");

						break;
					}

					if( (buffer[0] == 'y')||(buffer[0] == 'Y') )
					{
						// User wants to sample this channel, let him choose the range.

						int index_range;

						while(true)
						{
							printf("Choose Range:\n\n");

							for(index_range = 0; index_range < i_number_of_ranges; index_range++)
							{
								printf("%2d.   %8.3f - %8.3f Volt\n",
								index_range + 1,
								p_d_range_physical_minimum[index_range],
								p_d_range_physical_maximum[index_range]	 );
							}

							printf("\nYour choice (1 - %2d) (RETURN to terminate): ", i_number_of_ranges);

							gets(&buffer[0]);

							printf("\n\n");

							if(buffer[0] == 0)
							{
								b_terminate = true;
								printf("Terminate\n\n");

								break;
							}

							if( (sscanf(&buffer[0], "%d", &index_range) != 1 ) )
							{
								continue;
							}

							if( (index_range < 1)||(index_range > i_number_of_ranges) )
							{
								continue;
							}

							--index_range; // Range index starts at 0

							break;
						}

						if(b_terminate)
						{
							break;
						}

						double d_offset;

						while(true)
						{
							printf("Offset in Volts (RETURN to terminate): ");

							gets(&buffer[0]);

							printf("\n\n");

							if(buffer[0] == 0)
							{
								b_terminate = true;
								printf("Terminate\n\n");

								break;
							}

							double d_required_offset;

							if( (sscanf(&buffer[0], "%lf", &d_required_offset) != 1 ) )
							{
								continue;
							}

							d_offset = d_required_offset;

							meIOSetChannelOffset(	i_mephisto_scope_device,			// Device index
													i_stream_read_subdevice,			// Sub-device index
													index_channel,						// Channel index
													index_range,						// Range index
													&d_offset,							// Required offset, actual offset returned here
													ME_IO_SET_CHANNEL_OFFSET_NO_FLAGS);	// Flags

							printf("Offset of %f Volts required - %f Volts will be used\n\n", d_required_offset, d_offset);

							printf("Press any key to continue\n\n");

							getch();

							break;
						}

						if(b_terminate)
						{
							break;
						}

						i_sample_channels[i_number_of_channels_to_sample] = index_channel;
						i_channel_range[i_number_of_channels_to_sample] = index_range;

						d_channel_physical_min[i_number_of_channels_to_sample] = p_d_range_physical_minimum[index_range];

						d_channel_physical_max[i_number_of_channels_to_sample] = p_d_range_physical_maximum[index_range];

						d_channel_offset[i_number_of_channels_to_sample] = d_offset;

						++i_number_of_channels_to_sample;

						break;
					}
					else if( (buffer[0] == 'n')||(buffer[0] == 'N') )
					{
						break;
					}
				}

				if(b_terminate)
				{
					break;
				}
			}

			free(p_d_range_physical_minimum);

			free(p_d_range_physical_maximum);

			if(b_terminate)
			{
				break;
			}

			if(i_number_of_channels_to_sample <= 0)
			{
				printf("Please choose at least one channel to sample\n\n");

				printf("Press any key to continue\n\n");

				getch();

				continue;
			}

			// Let the user choose the trigger type

			while(true)
			{
				printf(	"Trigger type:\n\n"
						" 1) Manual (Software)\n"
						" 2) Threshold, above\n"
						" 3) Threshold, below\n"
						" 4) Window, on entering\n"
						" 5) Window, on leaving\n"
						" 6) Edge, rising\n"
						" 7) Edge, falling\n"
						" 8) Slope, positive\n"
						" 9) Slope, negative\n"
						"10) External digital, falling edge\n"
						"11) External digital, rising edge\n\n"	);

				printf("\nYour choice (1 - 11) (RETURN to terminate): ");

				gets(&buffer[0]);

				printf("\n\n");

				if(buffer[0] == 0)
				{
					b_terminate = true;
					printf("Terminate\n\n");

					break;
				}

				if( (sscanf(&buffer[0], "%d", &i_trigger_type) != 1 ) )
				{
					continue;
				}

				if( (i_trigger_type < 1)||(i_trigger_type > 11) )
				{
					continue;
				}

				break;
			}

			if(b_terminate)
			{
				break;
			}

			if( (i_trigger_type != ANALOG_TRIGGER_TYPE_MANUAL)&&(i_trigger_type != ANALOG_TRIGGER_TYPE_EXTERNAL_DIGITAL_EDGE_RISING)&&(i_trigger_type != ANALOG_TRIGGER_TYPE_EXTERNAL_DIGITAL_EDGE_FALLING) )
			{
				while(true)
				{
					printf("Trigger channel, 1 or 2 (RETURN to terminate): ");

					gets(&buffer[0]);

					printf("\n\n");

					if(buffer[0] == 0)
					{
						b_terminate = true;
						printf("Terminate\n\n");

						break;
					}

					if( (sscanf(&buffer[0], "%d", &i_trigger_channel) != 1 ) )
					{
						continue;
					}

					if( (i_trigger_channel != 1)&&(i_trigger_channel != 2) )
					{
						continue;
					}

					--i_trigger_channel;

					break;
				}
			}

			if(b_terminate)
			{
				break;
			}

			// Additional parameters for the various trigger types

			if( (i_trigger_type == ANALOG_TRIGGER_TYPE_THRESHOLD_ABOVE)||
						(i_trigger_type == ANALOG_TRIGGER_TYPE_THRESHOLD_BELOW)||
								(i_trigger_type == ANALOG_TRIGGER_TYPE_EDGE_RISING)||
									(i_trigger_type == ANALOG_TRIGGER_TYPE_EDGE_FALLING) )
			{
				while(true)
				{
					printf("Threshold in Volts (RETURN to terminate): ");

					gets(&buffer[0]);

					printf("\n\n");

					if(buffer[0] == 0)
					{
						b_terminate = true;
						printf("Terminate\n\n");

						break;
					}

					double d_volts;

					if( (sscanf(&buffer[0], "%lf", &d_volts) != 1 ) )
					{
						continue;
					}

					// ME-iDS requires this parameter in microvolts

					i_trigger_args[0] = (int)(d_volts * 1000000.0);

					break;
				}

				if(b_terminate)
				{
					break;
				}
			}
			else if( (i_trigger_type == ANALOG_TRIGGER_TYPE_WINDOW_ENTER)||
								(i_trigger_type == ANALOG_TRIGGER_TYPE_WINDOW_LEAVE) )
			{
				while(true)
				{
					printf("Fenster upper limit in Volts (RETURN to terminate): ");

					gets(&buffer[0]);

					printf("\n\n");

					if(buffer[0] == 0)
					{
						b_terminate = true;
						printf("Terminate\n\n");

						break;
					}

					double d_volts;

					if( (sscanf(&buffer[0], "%lf", &d_volts) != 1 ) )
					{
						continue;
					}

					// ME-iDS requires this parameter in microvolts

					i_trigger_args[0] = (int)(d_volts * 1000000.0);

					break;
				}

				if(b_terminate)
				{
					break;
				}

				while(true)
				{
					printf("Fenster lower limit in Volts (RETURN to terminate): ");

					gets(&buffer[0]);

					printf("\n\n");

					if(buffer[0] == 0)
					{
						b_terminate = true;
						printf("Terminate\n\n");

						break;
					}

					double d_volts;

					if( (sscanf(&buffer[0], "%lf", &d_volts) != 1 ) )
					{
						continue;
					}

					// ME-iDS requires this parameter in microvolts

					i_trigger_args[1] = (int)(d_volts * 1000000.0);

					break;
				}

				if(b_terminate)
				{
					break;
				}
			}
			else if( (i_trigger_type == ANALOG_TRIGGER_TYPE_SLOPE_POSITIVE)||
						(i_trigger_type == ANALOG_TRIGGER_TYPE_SLOPE_NEGATIVE) )
			{
				while(true)
				{
					printf("Slope in Volts per sample(RETURN to terminate): ");

					gets(&buffer[0]);

					printf("\n\n");

					if(buffer[0] == 0)
					{
						b_terminate = true;
						printf("Terminate\n\n");

						break;
					}

					double d_volts;

					if( (sscanf(&buffer[0], "%lf", &d_volts) != 1 ) )
					{
						continue;
					}

					// ME-iDS requires this parameter in microvolts per sample

					i_trigger_args[0] = (int)(d_volts * 1000000.0);

					break;
				}

				if(b_terminate)
				{
					break;
				}
			}

		}
		else if( (i_mode == MODE_DIGITAL_OSCILLOSCOPE)||(i_mode == MODE_DIGITAL_DATA_LOGGER) )
		{
			// Let the user choose the trigger type

			i_number_of_channels_to_sample = 1;

			while(true)
			{
				printf(	"Trigger type:\n\n"
						"1) Manual (Software)\n"
						"2) External digital, falling edge\n"
						"3) External digital, rising edge\n"
						"4) Bit Pattern\n\n"					);

				printf("\nYour choice (1 - 4) (RETURN to terminate): ");

				gets(&buffer[0]);

				printf("\n\n");

				if(buffer[0] == 0)
				{
					b_terminate = true;
					printf("Terminate\n\n");

					break;
				}

				if( (sscanf(&buffer[0], "%d", &i_trigger_type) != 1 ) )
				{
					continue;
				}

				if( (i_trigger_type < 1)||(i_trigger_type > 4) )
				{
					continue;
				}

				i_trigger_type+= 100;

				break;
			}

			if(b_terminate)
			{
				break;
			}

			// Additional parameter for bit pattern trigger

			if(i_trigger_type == DIGITAL_TRIGGER_TYPE_BIT_PATTERN)
			{
				while(true)
				{
					printf("Bit pattern (RETURN to terminate): 0X");

					gets(&buffer[0]);

					printf("\n\n");

					if(buffer[0] == 0)
					{
						b_terminate = true;
						printf("Terminate\n\n");

						break;
					}

					if( (sscanf(&buffer[0], "%X", &i_bit_pattern) != 1 ) )
					{
						continue;
					}

					break;
				}
			}
		}

		if( (i_mode == MODE_ANALOG_OSCILLOSCOPE)||(i_mode == MODE_DIGITAL_OSCILLOSCOPE) )
		{
			while(true)
			{
				printf("Memory depth (Minimum 100) (RETURN to terminate): ");

				gets(&buffer[0]);

				printf("\n\n");

				if(buffer[0] == 0)
				{
					b_terminate = true;
					printf("Terminate\n\n");

					break;
				}

				if( (sscanf(&buffer[0], "%d", &i_memory_depth) != 1 ) )
				{
					continue;
				}

				if(i_memory_depth < 100)
				{
					continue;
				}

				break;
			}

			if(b_terminate)
			{
				break;
			}

			while(true)
			{
				printf("Trigger point, 0 - 100 %% (RETURN to terminate): ");

				gets(&buffer[0]);

				printf("\n\n");

				if(buffer[0] == 0)
				{
					b_terminate = true;
					printf("Terminate\n\n");

					break;
				}

				if( (sscanf(&buffer[0], "%d", &i_trigger_point_percent) != 1 ) )
				{
					continue;
				}

				if( (i_trigger_point_percent < 0)||(i_trigger_point_percent > 100) )
				{
					continue;
				}

				break;
			}

			if(b_terminate)
			{
				break;
			}
		}

		// Time-out

		if( (i_trigger_type != ANALOG_TRIGGER_TYPE_MANUAL)&&(i_trigger_type != DIGITAL_TRIGGER_TYPE_MANUAL) )
		{
			while(true)
			{
				printf("Trigger time-out in seconds ( > 0 ) (RETURN to terminate): ");

				gets(&buffer[0]);

				printf("\n\n");

				if(buffer[0] == 0)
				{
					b_terminate = true;
					printf("Terminate\n\n");

					break;
				}

				if( (sscanf(&buffer[0], "%d", &i_trigger_timeout_seconds) != 1 ) )
				{
					continue;
				}

				if(i_trigger_timeout_seconds < 1)
				{
					continue;
				}

				break;
			}

			if(b_terminate)
			{
				break;
			}
		}

		double d_required_sampling_time = 0.1;
		double d_achieved_sampling_time = d_required_sampling_time;

		int i_conv_ticks_low = 0;
		int i_conv_ticks_high = 0;

		int i_flags;

		if( (i_mode == MODE_ANALOG_OSCILLOSCOPE)||(i_mode == MODE_DIGITAL_OSCILLOSCOPE) )
		{
			i_flags = ME_IO_TIME_TO_TICKS_MEPHISTO_SCOPE_OSCILLOSCOPE;
		}
		else
		{
			i_flags = ME_IO_TIME_TO_TICKS_NO_FLAGS;
		}

		i_me_error = meIOStreamTimeToTicks(	i_mephisto_scope_device,			// Device index
											i_stream_read_subdevice,			// Subdevice index,
											ME_TIMER_CONV_START,				// Timer used
											&d_achieved_sampling_time,			// Required sampling time un seconds, Achieved sampling time returned here
											&i_conv_ticks_low,					// Lower 32 bits of the corresponding tick value returned here
											&i_conv_ticks_high,					// Upper 32 bits of the corresponding tick value returned here
											i_flags						);		// No flags

		if(i_me_error != ME_ERRNO_SUCCESS)
		{
			printf("****    meIOStreamTimeToTicks - Error: %d    ****\n\n", i_me_error);

			printf("Press any key to terminate\n");

			_getch();

			meClose(0);

			return -1;
		}

		printf("meIOStreamTimeToTicks - Required Sampling Time: %f Achieved Sampling Time: %f Ticks low: %d Ticks high: %d\n\n", d_required_sampling_time, d_achieved_sampling_time, i_conv_ticks_low, i_conv_ticks_high);

		meIOStreamConfig_t arr_stream_config[2];

		if( (i_mode == MODE_ANALOG_OSCILLOSCOPE)||(i_mode == MODE_ANALOG_DATA_LOGGER) )
		{
			int index_channel = 0 ;
			for( index_channel = 0; index_channel < i_number_of_channels_to_sample; index_channel++)
			{
				arr_stream_config[index_channel].iChannel = i_sample_channels[index_channel];

				arr_stream_config[index_channel].iStreamConfig = i_channel_range[index_channel];

				arr_stream_config[index_channel].iRef = ME_REF_AI_GROUND;

				arr_stream_config[index_channel].iFlags = ME_IO_STREAM_CONFIG_TYPE_NO_FLAGS;
			}
		}
		else // if( (i_mode == MODE_DIGITAL_OSCILLOSCOPE)||(i_mode == MODE_DIGITAL_DATA_LOGGER) )
		{
			i_number_of_channels_to_sample = 1;

			arr_stream_config[0].iChannel = 0;

			arr_stream_config[0].iStreamConfig = 0;

			arr_stream_config[0].iRef = ME_REF_AI_GROUND;

			arr_stream_config[0].iFlags = ME_IO_STREAM_CONFIG_TYPE_NO_FLAGS;
		}

		meIOStreamTrigger_t stream_trigger;

		memset( &stream_trigger, 0, sizeof(meIOStreamTrigger_t) );

		switch(i_trigger_type)
		{
		case ANALOG_TRIGGER_TYPE_MANUAL:
		case DIGITAL_TRIGGER_TYPE_MANUAL:

			stream_trigger.iAcqStartTrigType = ME_TRIG_TYPE_SW;

			stream_trigger.iAcqStartTrigChan = ME_TRIG_CHAN_DEFAULT;

			break;

		case ANALOG_TRIGGER_TYPE_THRESHOLD_ABOVE:

			stream_trigger.iAcqStartTrigType = ME_TRIG_TYPE_THRESHOLD;

			stream_trigger.iAcqStartTrigEdge = ME_TRIG_EDGE_ABOVE;

			stream_trigger.iAcqStartTrigChan = i_trigger_channel;

			stream_trigger.iAcqStartArgs[0] = i_trigger_args[0];

			break;

		case ANALOG_TRIGGER_TYPE_THRESHOLD_BELOW:

			stream_trigger.iAcqStartTrigType = ME_TRIG_TYPE_THRESHOLD;

			stream_trigger.iAcqStartTrigEdge = ME_TRIG_EDGE_BELOW;

			stream_trigger.iAcqStartTrigChan = i_trigger_channel;

			stream_trigger.iAcqStartArgs[0] = i_trigger_args[0];

			break;

		case ANALOG_TRIGGER_TYPE_WINDOW_ENTER:

			stream_trigger.iAcqStartTrigType = ME_TRIG_TYPE_WINDOW;

			stream_trigger.iAcqStartTrigEdge = ME_TRIG_EDGE_ENTRY;

			stream_trigger.iAcqStartTrigChan = i_trigger_channel;

			stream_trigger.iAcqStartArgs[0] = i_trigger_args[0];

			stream_trigger.iAcqStartArgs[1] = i_trigger_args[1];

			break;

		case ANALOG_TRIGGER_TYPE_WINDOW_LEAVE:

			stream_trigger.iAcqStartTrigType = ME_TRIG_TYPE_WINDOW;

			stream_trigger.iAcqStartTrigEdge = ME_TRIG_EDGE_EXIT;

			stream_trigger.iAcqStartTrigChan = i_trigger_channel;

			stream_trigger.iAcqStartArgs[0] = i_trigger_args[0];

			stream_trigger.iAcqStartArgs[1] = i_trigger_args[1];

			break;

		case ANALOG_TRIGGER_TYPE_EDGE_RISING:

			stream_trigger.iAcqStartTrigType = ME_TRIG_TYPE_EDGE;

			stream_trigger.iAcqStartTrigEdge = ME_TRIG_EDGE_RISING;

			stream_trigger.iAcqStartTrigChan = i_trigger_channel;

			stream_trigger.iAcqStartArgs[0] = i_trigger_args[0];

			break;

		case ANALOG_TRIGGER_TYPE_EDGE_FALLING:

			stream_trigger.iAcqStartTrigType = ME_TRIG_TYPE_EDGE;

			stream_trigger.iAcqStartTrigEdge = ME_TRIG_EDGE_FALLING;

			stream_trigger.iAcqStartTrigChan = i_trigger_channel;

			stream_trigger.iAcqStartArgs[0] = i_trigger_args[0];

			break;

		case ANALOG_TRIGGER_TYPE_SLOPE_POSITIVE:

			stream_trigger.iAcqStartTrigType = ME_TRIG_TYPE_SLOPE;

			stream_trigger.iAcqStartTrigEdge = ME_TRIG_EDGE_RISING;

			stream_trigger.iAcqStartTrigChan = i_trigger_channel;

			stream_trigger.iAcqStartArgs[0] = i_trigger_args[0];

			break;

		case ANALOG_TRIGGER_TYPE_SLOPE_NEGATIVE:

			stream_trigger.iAcqStartTrigType = ME_TRIG_TYPE_SLOPE;

			stream_trigger.iAcqStartTrigEdge = ME_TRIG_EDGE_FALLING;

			stream_trigger.iAcqStartTrigChan = i_trigger_channel;

			stream_trigger.iAcqStartArgs[0] = i_trigger_args[0];

			break;

		case ANALOG_TRIGGER_TYPE_EXTERNAL_DIGITAL_EDGE_RISING:
		case DIGITAL_TRIGGER_TYPE_EXTERNAL_DIGITAL_EDGE_RISING:

			stream_trigger.iAcqStartTrigType = ME_TRIG_TYPE_EXT_DIGITAL;

			stream_trigger.iAcqStartTrigEdge = ME_TRIG_EDGE_RISING;

			break;

		case ANALOG_TRIGGER_TYPE_EXTERNAL_DIGITAL_EDGE_FALLING:
		case DIGITAL_TRIGGER_TYPE_EXTERNAL_DIGITAL_EDGE_FALLING:

			stream_trigger.iAcqStartTrigType = ME_TRIG_TYPE_EXT_DIGITAL;

			stream_trigger.iAcqStartTrigEdge = ME_TRIG_EDGE_FALLING;

			break;

		case DIGITAL_TRIGGER_TYPE_BIT_PATTERN:

			stream_trigger.iAcqStartTrigType = ME_TRIG_TYPE_PATTERN;

			stream_trigger.iAcqStartTrigChan = i_bit_pattern;

			break;
		}

		stream_trigger.iAcqStartTicksLow = 0;
		stream_trigger.iAcqStartTicksHigh = 0;
		// int iAcqStartArgs[10];
		stream_trigger.iScanStartTrigType = ME_TRIG_TYPE_FOLLOW;
		stream_trigger.iScanStartTicksLow = 0;
		stream_trigger.iScanStartTicksHigh = 0;
		//int iScanStartArgs[10];
		stream_trigger.iConvStartTrigType = ME_TRIG_TYPE_TIMER;
		stream_trigger.iConvStartTicksLow = i_conv_ticks_low;
		stream_trigger.iConvStartTicksHigh = i_conv_ticks_high;
		//int iConvStartArgs[10];

		if( (i_mode == MODE_ANALOG_OSCILLOSCOPE)||(i_mode == MODE_DIGITAL_OSCILLOSCOPE) )
		{
			stream_trigger.iScanStopTrigType = ME_TRIG_TYPE_COUNT;
			stream_trigger.iScanStopCount = i_memory_depth * i_number_of_channels_to_sample;
			stream_trigger.iScanStopArgs[0] = i_trigger_point_percent;

			stream_trigger.iAcqStopTrigType = ME_TRIG_TYPE_FOLLOW;
			stream_trigger.iAcqStopCount = 0;
		}

		// int iScanStopTrigType;
		// int iScanStopCount;
		// int iScanStopArgs[10];
		// int iAcqStopTrigType;
		// int iAcqStopCount;
		// int iAcqStopArgs[10];
		stream_trigger.iFlags = ME_IO_STREAM_TRIGGER_TYPE_NO_FLAGS;

		if( (i_mode == MODE_DIGITAL_OSCILLOSCOPE)||(i_mode == MODE_DIGITAL_DATA_LOGGER) )
		{
			i_flags = ME_IO_STREAM_CONFIG_BIT_PATTERN;
		}
		else
		{
			i_flags = ME_IO_STREAM_CONFIG_NO_FLAGS;
		}

		i_me_error = meIOStreamConfig(	i_mephisto_scope_device,			// Device index
										i_stream_read_subdevice,			// Subdevice index,
 										&arr_stream_config[0],				// Pointer to an array of meIOStreamConfig_t structures
										i_number_of_channels_to_sample,		// Number of elements in the stream config array above
										&stream_trigger,					// Pointer to an meIOStreamTrigger_t structure
										0,									// FIFO IRQ threshold - Not used in the MEphisto Scope
										i_flags							);	// No flags

		if(i_me_error != ME_ERRNO_SUCCESS)
		{
			printf("****    meIOStreamConfig - Error: %d    ****\n\n", i_me_error);

			printf("Press any key to terminate\n");

			_getch();

			meClose(0);

			return -1;
		}

		printf("\n\n");

		switch(i_mode)
		{
		case MODE_ANALOG_OSCILLOSCOPE:

			printf("Calling the Analog Oscilloscope with the following parameters:\n\n");

			break;

		case MODE_ANALOG_DATA_LOGGER:

			printf("Calling the Analog Data Logger with the following parameters:\n\n");

			break;

		case MODE_DIGITAL_OSCILLOSCOPE:

			printf("Calling the Digital Oscilloscope (Logic Analyzer) with the following parameters:\n\n");

			break;

		case MODE_DIGITAL_DATA_LOGGER:

			printf("Calling the Digital Data Logger with the following parameters:\n\n");

			break;
		}

		if( (i_mode == MODE_ANALOG_OSCILLOSCOPE)||(i_mode == MODE_ANALOG_DATA_LOGGER) )
		{
			printf("    Sampled channels:\n\n");

			int index_channel = 0;
			for( index_channel = 0; index_channel < i_number_of_channels_to_sample; index_channel++)
			{
				printf(	"        %d. Channel: %d    Range: %5.2f - %5.2f Volts\n",
						index_channel + 1,
						i_sample_channels[index_channel] + 1,
						d_channel_physical_min[index_channel] + d_channel_offset[index_channel],
						d_channel_physical_max[index_channel] + d_channel_offset[index_channel]				);
			}

			printf("\n");
		}

		printf("    Sampling Time: %f Seconds\n\n", d_achieved_sampling_time);

		if( (i_mode == MODE_ANALOG_OSCILLOSCOPE)||(i_mode == MODE_DIGITAL_OSCILLOSCOPE) )
		{
			printf("    Memory depth:  %d Samples\n", stream_trigger.iScanStopCount);
			printf("    Trigger point: %d %%\n\n", stream_trigger.iScanStopArgs[0]);
		}

		printf("    Trigger type\n\n");

		switch(i_trigger_type)
		{
		case ANALOG_TRIGGER_TYPE_MANUAL:
		case DIGITAL_TRIGGER_TYPE_MANUAL:

			printf("        Manual\n");

			break;

		case ANALOG_TRIGGER_TYPE_THRESHOLD_ABOVE:

			printf("        Above Threshold at: %7.3f Volt\n", (double)stream_trigger.iAcqStartArgs[0] / 10000000.0);

			break;

		case ANALOG_TRIGGER_TYPE_THRESHOLD_BELOW:

			printf("        Below Threshold at: %7.3f Volt\n", (double)stream_trigger.iAcqStartArgs[0] / 1000000.0);

			break;

		case ANALOG_TRIGGER_TYPE_WINDOW_ENTER:

			printf("        Enter Window from: %7.3f Volt to %7.3f Volt\n", (double)stream_trigger.iAcqStartArgs[1] / 1000000.0, (double)stream_trigger.iAcqStartArgs[0] / 1000000.0);

			break;

		case ANALOG_TRIGGER_TYPE_WINDOW_LEAVE:

			printf("        Leave Window from: %7.3f Volt to %7.3f Volt\n", (double)stream_trigger.iAcqStartArgs[1] / 1000000.0, (double)stream_trigger.iAcqStartArgs[0] / 1000000.0);

			break;

		case ANALOG_TRIGGER_TYPE_EDGE_RISING:

			printf("        Cross the Threshold at %7.3f Volt from above\n", (double)stream_trigger.iAcqStartArgs[0] / 1000000.0);

			break;

		case ANALOG_TRIGGER_TYPE_EDGE_FALLING:

			printf("        Cross the Threshold at %7.3f Volt from below\n", (double)stream_trigger.iAcqStartArgs[0] / 1000000.0);

			break;

		case ANALOG_TRIGGER_TYPE_SLOPE_POSITIVE:

			printf("        Positive slope greater than %7.3f Volts / Sample\n", (double)stream_trigger.iAcqStartArgs[0] / 1000000.0);

			break;

		case ANALOG_TRIGGER_TYPE_SLOPE_NEGATIVE:

			printf("        Negative slope greater than %7.3f Volts / Sample\n", (double)stream_trigger.iAcqStartArgs[0] / 1000000.0);

			break;

		case ANALOG_TRIGGER_TYPE_EXTERNAL_DIGITAL_EDGE_RISING:
		case DIGITAL_TRIGGER_TYPE_EXTERNAL_DIGITAL_EDGE_RISING:
			printf("        Digital signal, rising edge\n");

			break;

		case ANALOG_TRIGGER_TYPE_EXTERNAL_DIGITAL_EDGE_FALLING:
		case DIGITAL_TRIGGER_TYPE_EXTERNAL_DIGITAL_EDGE_FALLING:

			printf("        Digital signal, falling edge\n");

			break;

		case DIGITAL_TRIGGER_TYPE_BIT_PATTERN:

			printf("        Digital pattern: 0x%08X\n", stream_trigger.iAcqStartTrigChan);

			break;
		}

		if( (i_trigger_type == ANALOG_TRIGGER_TYPE_THRESHOLD_ABOVE)||
				(i_trigger_type == ANALOG_TRIGGER_TYPE_THRESHOLD_BELOW)||
					(i_trigger_type == ANALOG_TRIGGER_TYPE_WINDOW_ENTER)||
						(i_trigger_type == ANALOG_TRIGGER_TYPE_WINDOW_LEAVE)||
							(i_trigger_type == ANALOG_TRIGGER_TYPE_EDGE_RISING)||
								(i_trigger_type == ANALOG_TRIGGER_TYPE_EDGE_FALLING)||
									(i_trigger_type == ANALOG_TRIGGER_TYPE_SLOPE_POSITIVE)||
										(i_trigger_type == ANALOG_TRIGGER_TYPE_SLOPE_NEGATIVE) )
		{
			printf("        Trigger channel: %d\n\n", i_trigger_channel);
		}
		else
		{
			printf("\n");
		}

		printf("Press any key to start the acquisition\n\n");

		if( (i_mode == MODE_ANALOG_DATA_LOGGER)||(i_mode == MODE_DIGITAL_DATA_LOGGER) )
		{
			printf("Press the 'q' at any time to end the acquisition\n\n");
		}
		else // if( (i_mode == MODE_ANALOG_OSCILLOSCOPE)||(i_mode == MODE_DIGITAL_OSCILLOSCOPE) )
		{
			printf("Then please wait until all the data is available\n\n");
		}

		_getch();

		meIOStreamStart_t arr_stream_start[1];

		arr_stream_start[0].iDevice = i_mephisto_scope_device;
		arr_stream_start[0].iSubdevice = i_stream_read_subdevice;
		arr_stream_start[0].iStartMode = ME_START_MODE_BLOCKING;

		if( (i_trigger_type == ANALOG_TRIGGER_TYPE_MANUAL)||(i_trigger_type == DIGITAL_TRIGGER_TYPE_MANUAL) )
		{
			arr_stream_start[0].iTimeOut = 0;
		}
		else
		{
			arr_stream_start[0].iTimeOut = i_trigger_timeout_seconds * 1000;	// ME-iDS time-out is in milliseconds
		}

		arr_stream_start[0].iFlags = ME_IO_STREAM_START_TYPE_NO_FLAGS;
		arr_stream_start[0].iErrno = 0;

		i_me_error = meIOStreamStart(	&arr_stream_start[0],			// Pointer to an array of meIOStreamStart_t structures
										1,								// Number of elements in the stream start array above
										ME_IO_STREAM_START_NO_FLAGS	);	// No flags

		if(i_me_error != ME_ERRNO_SUCCESS)
		{
			printf("****    meIOStreamStart - Error: %d    ****\n\n", i_me_error);

			printf("Press any key to terminate\n");

			_getch();

			meClose(0);

			return -1;
		}

		printf("Acquisition started\n\n");

		if( (i_mode == MODE_ANALOG_DATA_LOGGER)||(i_mode == MODE_DIGITAL_DATA_LOGGER) )
		{
			while(true)
			{
				if( _kbhit() != 0 )
				{
		printf("Char acquisition started\n");
					char c = _getch();
		printf("Char:%c\n", c);
					if(c == 'q')
					{
						break;
					}
				}

				int i_buffer[4096];

				int i_count = 10;

				i_me_error = meIOStreamRead(i_mephisto_scope_device,		// Device index
											i_stream_read_subdevice,		// Subdevice index,
											ME_READ_MODE_BLOCKING,			// Read mode
											&i_buffer[0],					// Acquired data values returned in this buffer
											&i_count,						// Size of buffer in data values - Number of data values returned here
											ME_IO_STREAM_READ_NO_FLAGS);	// No flags

				if(i_me_error != ME_ERRNO_SUCCESS)
				{
					printf("****    meIOStreamRead - Error: %d    ****\n\n", i_me_error);

					printf("Press any key to end the acquisition\n");

					_getch();

					break;
				}

				int index_value = 0;
				for( index_value = 0; index_value < i_count; )
				{
					int index_channel = 0;
					for( index_channel = 0; index_channel < i_number_of_channels_to_sample; index_channel++)
					{
						if(i_mode == MODE_ANALOG_DATA_LOGGER)
						{
							double d_volt;

							i_me_error =  meUtilityDigitalToPhysical(	d_channel_physical_min[index_channel],		// Min physical value
																		d_channel_physical_max[index_channel],		// Max physical value
																		0xFFFF,										// Maximum digital value
																		i_buffer[index_value],						// Digital value to convert
																		ME_MODULE_TYPE_MULTISIG_NONE,				// Module type - not used
																		0.0,										// Reference value - not used
																		&d_volt									);	// Converted physical value returned here

							printf( "%5d  -  %5.2lf V    ", i_buffer[index_value], d_volt + d_channel_offset[index_channel] );
						}
						else
						{
							printf( "0X%04X", (unsigned short)i_buffer[index_value] );
						}

						index_value++;
					}

					printf("\n");
				}
			}
		}
		else // if( (i_mode == MODE_ANALOG_OSCILLOSCOPE)||(i_mode == MODE_DIGITAL_OSCILLOSCOPE) )
		{
			// In this mode all the data has to be read in one action

			if(stream_trigger.iScanStopCount > iOscilloscopeBufferSize)
			{
				if(piOscilloscopeBuffer != NULL)
				{
					free( piOscilloscopeBuffer );
				}

				iOscilloscopeBufferSize = stream_trigger.iScanStopCount;

				piOscilloscopeBuffer = (int*) malloc( sizeof(int) * iOscilloscopeBufferSize ) ;
			}

			int i_count = stream_trigger.iScanStopCount;

			i_me_error = meIOStreamRead(i_mephisto_scope_device,		// Device index
										i_stream_read_subdevice,		// Subdevice index,
										ME_READ_MODE_BLOCKING,			// Read mode
										&piOscilloscopeBuffer[0],		// Acquired data values returned in this buffer
										&i_count,						// Size of buffer in data values - Number of data values returned here
										ME_IO_STREAM_READ_NO_FLAGS);	// No flags

			if(i_me_error != ME_ERRNO_SUCCESS)
			{
				printf("****    meIOStreamRead - Error: %d    ****\n\n", i_me_error);

				printf("Press any key to end the acquisition\n");

				_getch();

				break;
			}

			int index_value = 0;
			for( index_value = 0; index_value < i_count; )
			{
				printf("%5d.    ", index_value + 1);

				int index_channel = 0;
				for(index_channel = 0; index_channel < i_number_of_channels_to_sample; index_channel++)
				{
					if(i_mode == MODE_ANALOG_OSCILLOSCOPE)
					{
						double d_volt;

						i_me_error =  meUtilityDigitalToPhysical(	d_channel_physical_min[index_channel],		// Min physical value
																	d_channel_physical_max[index_channel],		// Max physical value
																	0xFFFF,										// Maximum digital value
																	piOscilloscopeBuffer[index_value],			// Digital value to convert
																	ME_MODULE_TYPE_MULTISIG_NONE,				// Module type - not used
																	0.0,										// Reference value - not used
																	&d_volt									);	// Converted physical value returned here

						printf("%5d  -  %5.2lf V    ", piOscilloscopeBuffer[index_value], d_volt + d_channel_offset[index_channel]);
					}
					else
					{
						printf("0X%04X", (unsigned short)piOscilloscopeBuffer[index_value] );
					}

					index_value++;
				}

				printf("\n");
			}
		}

		meIOStreamStop_t arr_stream_stop[1];

		arr_stream_stop[0].iDevice = i_mephisto_scope_device;
		arr_stream_stop[0].iSubdevice = i_stream_read_subdevice;
		arr_stream_stop[0].iStopMode = ME_STOP_MODE_IMMEDIATE;
		arr_stream_stop[0].iFlags = ME_IO_STREAM_STOP_TYPE_NO_FLAGS;
		arr_stream_stop[0].iErrno = 0;

		i_me_error = meIOStreamStop(&arr_stream_stop[0],			// Pointer to an array of meIOStreamStop_t structures
									1,								// Number of elements in the stream stop array above
									ME_IO_STREAM_STOP_NO_FLAGS);	// No flags

		if(i_me_error != ME_ERRNO_SUCCESS)
		{
			printf("****    meIOStreamStop - Error: %d    ****\n\n", i_me_error);

			printf("Press any key to terminate\n");

			_getch();

			meClose(0);

			return(-1);
		}

		printf("Acquisition ended\n\n");

		printf("Press any key to continue\n");

		getch();
	}

	// ---------------------------------------------------------------------


	meClose(0);

	printf("Program completed successfully - Press any key to terminate\n\n");

	_getch();

	return 0;
}

int kbhit(void)
{
	fd_set rfds;
	struct timeval tv;
	int retval;

	FD_ZERO(&rfds);
	FD_SET(0, &rfds);
	tv.tv_sec = 1;
	tv.tv_usec = 0;

	retval = select(1, &rfds, NULL, NULL, &tv);
    if (FD_ISSET(0, &rfds))
	{
		printf("Pressing key detected.\n\n");
		return 1;
	}

    return 0;
}

