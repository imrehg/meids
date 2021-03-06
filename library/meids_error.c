/**
 * @file meids_error.c
 *
 * @brief Errors' descriptions.
 * @note Copyright (C) 2007 Meilhaus Electronic GmbH (support@meilhaus.de)
 * @author KG (Krzysztof Gantzke) (k.gantzke@meilhaus.de)
 * @author GG (Guenter Gebhardt)
 */

char* meErrorMsgTable[] =
{
	"Success.",
	"Invalid device number specified.",
	"Invalid subdevice number specified.",
	"Invalid channel number specified.",
	"Invalid single configuration specified.",
	"Invalid reference specified.",
	"Invalid trigger channel specified.",
	"Invalid trigger type specified.",
	"Invalid trigger edge specified.",
	"Invalid timeout specified.",
	"Invalid flags specified.",
	"Can't open driver system.",
	"Can't close driver system.",
	"Driver system was not opened by user process.",
	"Invalid single direction specified.",
	"Device was not configured for this function.",
	"Function not supported by device.",
	"No such subdevice type available on this device.",
	"User buffer size is to small to hold name.",
	"The Resource or parts of it is locked by another process.",
	"No more subdevice of this type available.",
	"Operation timed out.",
	"Operation aborted by signal.",
	"Invalid irq source specified.",
	"There is a background thread running on this subdevice.",
	"Cannot start background thread.",
	"Cannot cancel background thread.",
	"No callback function for notification on irq or new values specified.",
	"The Resource or parts of it is currently used by another process.",
	"Invalid physical unit specified.",
	"Invalid minimum and maximum values specified specified.",
	"No matching range found.",
	"Invalid range specified.",
	"Subdevice is busy.",
	"Invalid lock specified.",
	"Invalid switch specified.",
	"Error message string is to small.",
	"Invalid stream configuration specified.",
	"Invalid stream configuration list count specified.",
	"Invalid acquisition start trigger type specified.",
	"Invalid acquisition start trigger edge specified.",
	"Invalid acquisition start trigger channel specified.",
	"Invalid acquisition start time out specified.",
	"Invalid acquisition start argument specified.",
	"Invalid scan start trigger type specified.",
	"Invalid scan start argument specified.",
	"Invalid conversion start trigger type specified.",
	"Invalid conversion start argument specified.",
	"Invalid scan stop trigger type specified.",
	"Invalid scan stop argument specified.",
	"Invalid acquisition stop trigger type specified.",
	"Invalid acquisition stop argument specified.",
	"Subdevice is not running.",
	"Invalid read mode specified.",
	"Invalid value count specified.",
	"Invalid write mode specified.",
	"Invalid timer specified.",
	"Device was unplugged.",
	"Subdevice is reserved for internal usage.",
	"Invalid duty cycle specified.",
	"Invalid wait argument specified.",
	"Cannot connect to remote host.",
	"Communication error.",
	"Invalid single list specified.",
	"Invalid module type specified.",
	"Invalid start mode specified.",
	"Invalid stop mode specified.",
	"Invalid fifo irq threshold specified.",
	"Invalid pointer passed.",
	"Unable to create event.",
	"Insufficient resources.",
	"Operation cancelled.",
	"Software buffer overflow.",
	"Software buffer underflow.",
	"Invalid irq edge specified.",
	"Invalid irq arg specified.",
	"Invalid capability specified.",
	"Invalid capability argument count specified.",
	"Internal error occured.",
	"Value out of range.",
	"Hardware buffer overflow.",
	"Hardware buffer underflow.",
	"Loading config to device failed.",
	"Invalid error number specified."
};
