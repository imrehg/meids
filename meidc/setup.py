#!/usr/bin/env python

# Copyright (2006) Meilhaus Electronic GmbH.
# All rights reserved.

import os
from distutils.core import setup
if os.name == 'nt':
	import py2exe


setup(name = "ME-iDC",
		version = "1.4",
		description = "Meilhaus intelligent Device Configuration Utility",
		author = "Guenter Gebhardt & Krzysztof Gantzke",
		author_email = "support@meilhaus.com",
		url = "http://www.meilhaus.com",
		windows = [ "meIDC.py" ],
		py_modules = [
			"editDemux32Dialog",
			"editMux32mDialog",
			"helpDialog",
			"meIDC",
			"myMainWindow",
			"newDemux32Dialog",
			"newMux32mDialog",
			"newRemoteDeviceDialog"],
		data_files=[("bmp",
					["bmp/subdevice_locked_da.xpm",
					"bmp/subdevice_ad.xpm",
					"bmp/devices.xpm",
					"bmp/subdevice_irq.xpm",
					"bmp/remoteDeviceInfo.xpm",
					"bmp/info.xpm",
					"bmp/range.xpm",
					"bmp/multisig.xpm",
					"bmp/save.xpm",
					"bmp/subdevice_locked_ctr.xpm",
					"bmp/device.xpm",
					"bmp/subdevice_locked_di.xpm",
					"bmp/subdevice_dio.xpm",
					"bmp/ranges.xpm",
					"bmp/down.xpm",
					"bmp/data.xpm",
					"bmp/calender.xpm",
					"bmp/help.xpm",
					"bmp/subdevice_locked_do.xpm",
					"bmp/usbDevice_unplugged.xpm",
					"bmp/subdevice_info.xpm",
					"bmp/me1001_delete.xpm",
					"bmp/remoteDevice_unplugged.xpm",
					"bmp/subdevice_locked_ad.xpm",
					"bmp/subdevice_locked_irq.xpm",
					"bmp/device_info.xpm",
					"bmp/me1001_add.xpm",
					"bmp/multisigs.xpm",
					"bmp/subdevice_ctr.xpm",
					"bmp/quit.xpm",
					"bmp/up.xpm",
					"bmp/configuration.xpm",
					"bmp/device_unplugged.xpm",
					 "bmp/usbDevice.xpm",
					 "bmp/remoteDevice.xpm",
					 "bmp/subdevice_da.xpm",
					 "bmp/subdevice_locked_dio.xpm",
					 "bmp/subdevice_di.xpm",
					 "bmp/edit.xpm",
					 "bmp/MEIcon.xpm",
					 "bmp/subdevice_do.xpm",
					 "bmp/subdevices.xpm",
					 "bmp/delete.xpm",
					"bmp/MEIcon.ico"]
				),
				("help", ["help/help.html"])]
)
