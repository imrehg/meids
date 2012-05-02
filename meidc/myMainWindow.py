# Copyright (2006) Meilhaus Electronic GmbH.
# All rights reserved.

import wx
from wx.gizmos import *
from xml.dom import *
from xml.dom.minidom import *
import os
import sys
import time
import string
import re

import meDriver
import newDemux32Dialog
import newMux32mDialog
import editDemux32Dialog
import editMux32mDialog
import helpDialog
import newRemoteDeviceDialog


#--------------------------------------------------------------------------------------------


class MyMainWindow(wx.Frame):
	def __init__(self,
			parent,
			id,
			title,
			pos = wx.DefaultPosition,
			size = wx.Size(800, 600),
			style = wx.MINIMIZE_BOX | wx.SYSTEM_MENU | wx.CAPTION | wx.CLOSE_BOX | wx.CLIP_CHILDREN,
			name = "myMainWindow"):

		wx.Frame.__init__(self, parent, id, title, pos, size, style, name)

		# Determine the working directory
		if sys.path[0]:
			self.rootPath = sys.path[0] + os.sep
		else:
			self.rootPath = ''

		# Platform specific stuff
		if os.name == 'posix':
			self.rootPath = re.sub("meIDC", "", self.rootPath) # cx_Freeze generated executable path fix
			self.configBasePath = '/etc'
			self.configDir = 'medriver'
			self.configPath = self.configBasePath + os.sep + self.configDir
			self.configFile = 'meconfig.xml'
			self.dtdFile = "medrvconfig.dtd"
			self.SetIcon(wx.Icon(self.rootPath + 'bmp' + os.sep + 'MEIcon.xpm', wx.BITMAP_TYPE_XPM))
		elif os.name == 'nt':
			self.rootPath = re.sub("meIDC.exe", "", self.rootPath) # cx_Freeze generated executable path fix
			import _winreg
			REGISTRY_KEY = _winreg.HKEY_LOCAL_MACHINE
			REGISTRY_SUB_KEY = 'Software\Meilhaus Electronic\ME-iDC\\1.3'
			REGISTRY_VALUE_CONFIG_BASE_PATH = 'configBasePath'

			try:
				key = _winreg.OpenKey(REGISTRY_KEY, REGISTRY_SUB_KEY)
				self.configBasePath = _winreg.QueryValueEx(key, REGISTRY_VALUE_CONFIG_BASE_PATH)[0]
			except:
				self.configBasePath = 'C:\\'

			self.configDir = 'medriver'
			self.configPath = self.configBasePath + os.sep + self.configDir
			self.configFile = 'meconfig.xml'
			self.dtdFile = "medrvconfig.dtd"

			self.SetIcon(wx.Icon(self.rootPath + 'bmp' + os.sep + 'MEIcon.ico', wx.BITMAP_TYPE_ICO))
		else:
			dlg = wx.MessageDialog(self, str(value), "Unknown operating system.", wx.OK | wx.ICON_ERROR)
			dlg.ShowModal()
			dlg.Destroy()
			sys.exit(1)

		# Get custom bitmaps
		self.xpmCalender = wx.Bitmap(self.rootPath + 'bmp' + os.sep + 'calender.xpm', wx.BITMAP_TYPE_XPM)
		self.xpmCongiguration = wx.Bitmap(self.rootPath + 'bmp' + os.sep + 'configuration.xpm', wx.BITMAP_TYPE_XPM)
		self.xpmData = wx.Bitmap(self.rootPath + 'bmp' + os.sep + 'data.xpm', wx.BITMAP_TYPE_XPM)
		self.xpmDelete = wx.Bitmap(self.rootPath + 'bmp' + os.sep + 'delete.xpm', wx.BITMAP_TYPE_XPM)
		self.xpmDeviceInfo = wx.Bitmap(self.rootPath + 'bmp' + os.sep + 'device_info.xpm', wx.BITMAP_TYPE_XPM)
		self.xpmDevices = wx.Bitmap(self.rootPath + 'bmp' + os.sep + 'devices.xpm', wx.BITMAP_TYPE_XPM)
		self.xpmDeviceUnplugged = wx.Bitmap(self.rootPath + 'bmp' + os.sep + 'device_unplugged.xpm', wx.BITMAP_TYPE_XPM)
		self.xpmUsbDeviceUnplugged = wx.Bitmap(self.rootPath + 'bmp' + os.sep + 'usbDevice_unplugged.xpm', wx.BITMAP_TYPE_XPM)
		self.xpmPNPdeviceUnplugged = wx.Bitmap(self.rootPath + 'bmp' + os.sep + 'PNPdevice_unplugged.xpm', wx.BITMAP_TYPE_XPM)
		self.xpmDevice = wx.Bitmap(self.rootPath + 'bmp' + os.sep + 'device.xpm', wx.BITMAP_TYPE_XPM)
		self.xpmUsbDevice = wx.Bitmap(self.rootPath + 'bmp' + os.sep + 'usbDevice.xpm', wx.BITMAP_TYPE_XPM)
		self.xpmPNPdevice = wx.Bitmap(self.rootPath + 'bmp' + os.sep + 'PNPdevice.xpm', wx.BITMAP_TYPE_XPM)
		self.xpmRemoteDevice = wx.Bitmap(self.rootPath + 'bmp' + os.sep + 'remoteDevice.xpm', wx.BITMAP_TYPE_XPM)
		self.xpmRemoteDeviceUnplugged = wx.Bitmap(self.rootPath + 'bmp' + os.sep + 'remoteDevice_unplugged.xpm', wx.BITMAP_TYPE_XPM)
		self.xpmRemoteDeviceInfo = wx.Bitmap(self.rootPath + 'bmp' + os.sep + 'remoteDeviceInfo.xpm', wx.BITMAP_TYPE_XPM)
		self.xpmDown = wx.Bitmap(self.rootPath + 'bmp' + os.sep + 'down.xpm', wx.BITMAP_TYPE_XPM)
		self.xpmEdit = wx.Bitmap(self.rootPath + 'bmp' + os.sep + 'edit.xpm', wx.BITMAP_TYPE_XPM)
		self.xpmHelp = wx.Bitmap(self.rootPath + 'bmp' + os.sep + 'help.xpm', wx.BITMAP_TYPE_XPM)
		self.xpmInfo = wx.Bitmap(self.rootPath + 'bmp' + os.sep + 'info.xpm', wx.BITMAP_TYPE_XPM)
		self.xpmME1001Add = wx.Bitmap(self.rootPath + 'bmp' + os.sep + 'me1001_add.xpm', wx.BITMAP_TYPE_XPM)
		self.xpmME1001Delete = wx.Bitmap(self.rootPath + 'bmp' + os.sep + 'me1001_delete.xpm', wx.BITMAP_TYPE_XPM)
		self.xpmMultisigs = wx.Bitmap(self.rootPath + 'bmp' + os.sep + 'multisigs.xpm', wx.BITMAP_TYPE_XPM)
		self.xpmMultisig = wx.Bitmap(self.rootPath + 'bmp' + os.sep + 'multisig.xpm', wx.BITMAP_TYPE_XPM)
		self.xpmQuit = wx.Bitmap(self.rootPath + 'bmp' + os.sep + 'quit.xpm', wx.BITMAP_TYPE_XPM)
		self.xpmRanges = wx.Bitmap(self.rootPath + 'bmp' + os.sep + 'ranges.xpm', wx.BITMAP_TYPE_XPM)
		self.xpmRange = wx.Bitmap(self.rootPath + 'bmp' + os.sep + 'range.xpm', wx.BITMAP_TYPE_XPM)
		self.xpmSave = wx.Bitmap(self.rootPath + 'bmp' + os.sep + 'save.xpm', wx.BITMAP_TYPE_XPM)
		self.xpmSubdeviceAD = wx.Bitmap(self.rootPath + 'bmp' + os.sep + 'subdevice_ad.xpm', wx.BITMAP_TYPE_XPM)
		self.xpmSubdeviceCTR = wx.Bitmap(self.rootPath + 'bmp' + os.sep + 'subdevice_ctr.xpm', wx.BITMAP_TYPE_XPM)
		self.xpmSubdeviceDA = wx.Bitmap(self.rootPath + 'bmp' + os.sep + 'subdevice_da.xpm', wx.BITMAP_TYPE_XPM)
		self.xpmSubdeviceDIO = wx.Bitmap(self.rootPath + 'bmp' + os.sep + 'subdevice_dio.xpm', wx.BITMAP_TYPE_XPM)
		self.xpmSubdeviceDI = wx.Bitmap(self.rootPath + 'bmp' + os.sep + 'subdevice_di.xpm', wx.BITMAP_TYPE_XPM)
		self.xpmSubdeviceDO = wx.Bitmap(self.rootPath + 'bmp' + os.sep + 'subdevice_do.xpm', wx.BITMAP_TYPE_XPM)
		self.xpmSubdeviceInfo = wx.Bitmap(self.rootPath + 'bmp' + os.sep + 'subdevice_info.xpm', wx.BITMAP_TYPE_XPM)
		self.xpmSubdeviceIRQ = wx.Bitmap(self.rootPath + 'bmp' + os.sep + 'subdevice_irq.xpm', wx.BITMAP_TYPE_XPM)
		self.xpmSubdeviceLockedAD = wx.Bitmap(self.rootPath + 'bmp' + os.sep + 'subdevice_locked_ad.xpm', wx.BITMAP_TYPE_XPM)
		self.xpmSubdeviceLockedCTR = wx.Bitmap(self.rootPath + 'bmp' + os.sep + 'subdevice_locked_ctr.xpm', wx.BITMAP_TYPE_XPM)
		self.xpmSubdeviceLockedDA = wx.Bitmap(self.rootPath + 'bmp' + os.sep + 'subdevice_locked_da.xpm', wx.BITMAP_TYPE_XPM)
		self.xpmSubdeviceLockedDIO = wx.Bitmap(self.rootPath + 'bmp' + os.sep + 'subdevice_locked_dio.xpm', wx.BITMAP_TYPE_XPM)
		self.xpmSubdeviceLockedDI = wx.Bitmap(self.rootPath + 'bmp' + os.sep + 'subdevice_locked_di.xpm', wx.BITMAP_TYPE_XPM)
		self.xpmSubdeviceLockedDO = wx.Bitmap(self.rootPath + 'bmp' + os.sep + 'subdevice_locked_do.xpm', wx.BITMAP_TYPE_XPM)
		self.xpmSubdeviceLockedIRQ = wx.Bitmap(self.rootPath + 'bmp' + os.sep + 'subdevice_locked_irq.xpm', wx.BITMAP_TYPE_XPM)
		self.xpmSubdevices = wx.Bitmap(self.rootPath + 'bmp' + os.sep + 'subdevices.xpm', wx.BITMAP_TYPE_XPM)
		self.xpmUp = wx.Bitmap(self.rootPath + 'bmp' + os.sep + 'up.xpm', wx.BITMAP_TYPE_XPM)

		#
		# Menu section
		#
		fileMenu = wx.Menu()
		self.MENU_ID_SAVE = wx.NewId()
		fileMenuSaveItem = wx.MenuItem(fileMenu, self.MENU_ID_SAVE, "&Save\tCTRL+S", "Save the driver configuration")
		fileMenuSaveItem.SetBitmap(self.xpmSave)
		fileMenuSaveItem = fileMenu.AppendItem(fileMenuSaveItem)

		fileMenu.AppendSeparator()

		self.MENU_ID_QUIT = wx.NewId()
		fileMenuQuitItem = wx.MenuItem(fileMenu, self.MENU_ID_QUIT, "&Quit\tCTRL+Q", "Quit the application")
		fileMenuQuitItem.SetBitmap(self.xpmQuit)
		fileMenuQuitItem = fileMenu.AppendItem(fileMenuQuitItem)

		infoMenu = wx.Menu()
		self.MENU_ID_INFO = wx.NewId()
		infoMenuInfoItem = wx.MenuItem(infoMenu, self.MENU_ID_INFO, "&Info\tCTRL+I", "Displays the program info")
		infoMenuInfoItem.SetBitmap(self.xpmInfo)
		infoMenuInfoItem = infoMenu.AppendItem(infoMenuInfoItem)
		self.MENU_ID_HELP = wx.NewId()
		infoMenuHelpItem = wx.MenuItem(infoMenu, self.MENU_ID_HELP, "&Help\tCTRL+H", "Displays the program help")
		infoMenuHelpItem.SetBitmap(self.xpmHelp)
		infoMenuHelpItem = infoMenu.AppendItem(infoMenuHelpItem)

		menuBar = wx.MenuBar()
		menuBar.Append(fileMenu, "&File")
		menuBar.Append(infoMenu, "&?")

		self.SetMenuBar(menuBar)

		self.CreateStatusBar()
		self.SetStatusText("READY")

		# Menu events
		wx.EVT_MENU(self, self.MENU_ID_SAVE, self.SaveCB)
		wx.EVT_MENU(self, self.MENU_ID_QUIT, self.QuitCB)
		wx.EVT_MENU(self, self.MENU_ID_INFO, self.InfoCB)
		wx.EVT_MENU(self, self.MENU_ID_HELP, self.HelpCB)
		wx.EVT_CLOSE(self, self.QuitCB)

		#
		# Tool bar section
		#
		toolBar = self.CreateToolBar()
		toolBar.SetToolBitmapSize(wx.Size(32, 32))
		self.TOOL_ID_SAVE = wx.NewId()
		toolBar.AddSimpleTool(self.TOOL_ID_SAVE, self.xpmSave, "Save configuration", "Save the current configuration to file")
		toolBar.Realize() # Needed for Windows to display it
		wx.EVT_MENU(self, self.TOOL_ID_SAVE, self.SaveCB)

		#
		# The List/Tree control
		#
		treeCtrlID = wx.NewId()
		self.treeCtrl = TreeListCtrl(self, treeCtrlID, wx.DefaultPosition, wx.DefaultSize, style = wx.TR_DEFAULT_STYLE | wx.TR_FULL_ROW_HIGHLIGHT)
		self.treeCtrl.AddColumn("Selection")
		self.treeCtrl.AddColumn("Value")
		self.treeCtrl.SetMainColumn(0)
		self.treeCtrl.SetColumnWidth(0, 400)
		self.treeCtrl.SetColumnWidth(1, 800)

		treeImageSize = (32, 32)
		self.treeImageList = wx.ImageList(treeImageSize[0], treeImageSize[1])

		self.treeImageFolder = self.treeImageList.Add(self.xpmDevice)
		self.treeImageOpenFolder = self.treeImageList.Add(self.xpmDevice)

		self.treeImageCalender = self.treeImageList.Add(self.xpmCalender)
		self.treeImageConfiguration = self.treeImageList.Add(self.xpmCongiguration)
		self.treeImageData = self.treeImageList.Add(self.xpmData)
		self.treeImageDeviceInfo = self.treeImageList.Add(self.xpmDeviceInfo)
		self.treeImageDevices = self.treeImageList.Add(self.xpmDevices)
		self.treeImageDeviceUnplugged = self.treeImageList.Add(self.xpmDeviceUnplugged)
		self.treeImageUsbDeviceUnplugged = self.treeImageList.Add(self.xpmUsbDeviceUnplugged)
		self.treeImagePNPdeviceUnplugged = self.treeImageList.Add(self.xpmPNPdeviceUnplugged)
		self.treeImageDevice = self.treeImageList.Add(self.xpmDevice)
		self.treeImageUsbDevice = self.treeImageList.Add(self.xpmUsbDevice)
		self.treeImagePNPdevice = self.treeImageList.Add(self.xpmPNPdevice)
		self.treeImageRemoteDevice = self.treeImageList.Add(self.xpmRemoteDevice)
		self.treeImageRemoteDeviceUnplugged = self.treeImageList.Add(self.xpmRemoteDeviceUnplugged)
		self.treeImageRemoteDeviceInfo = self.treeImageList.Add(self.xpmRemoteDeviceInfo)
		self.treeImageMultisigs = self.treeImageList.Add(self.xpmMultisigs)
		self.treeImageMultisig = self.treeImageList.Add(self.xpmMultisig)
		self.treeImageRanges = self.treeImageList.Add(self.xpmRanges)
		self.treeImageRange = self.treeImageList.Add(self.xpmRange)
		self.treeImageSubdeviceAD = self.treeImageList.Add(self.xpmSubdeviceAD)
		self.treeImageSubdeviceCTR = self.treeImageList.Add(self.xpmSubdeviceCTR)
		self.treeImageSubdeviceDA = self.treeImageList.Add(self.xpmSubdeviceDA)
		self.treeImageSubdeviceDIO = self.treeImageList.Add(self.xpmSubdeviceDIO)
		self.treeImageSubdeviceDI = self.treeImageList.Add(self.xpmSubdeviceDI)
		self.treeImageSubdeviceDO = self.treeImageList.Add(self.xpmSubdeviceDO)
		self.treeImageSubdeviceInfo = self.treeImageList.Add(self.xpmSubdeviceInfo)
		self.treeImageSubdeviceIRQ = self.treeImageList.Add(self.xpmSubdeviceIRQ)
		self.treeImageSubdeviceLockedAD = self.treeImageList.Add(self.xpmSubdeviceLockedAD)
		self.treeImageSubdeviceLockedCTR = self.treeImageList.Add(self.xpmSubdeviceLockedCTR)
		self.treeImageSubdeviceLockedDA = self.treeImageList.Add(self.xpmSubdeviceLockedDA)
		self.treeImageSubdeviceLockedDIO = self.treeImageList.Add(self.xpmSubdeviceLockedDIO)
		self.treeImageSubdeviceLockedDI = self.treeImageList.Add(self.xpmSubdeviceLockedDI)
		self.treeImageSubdeviceLockedDO = self.treeImageList.Add(self.xpmSubdeviceLockedDO)
		self.treeImageSubdeviceLockedIRQ = self.treeImageList.Add(self.xpmSubdeviceLockedIRQ)
		self.treeImageSubdevices = self.treeImageList.Add(self.xpmSubdevices)

		self.treeCtrl.SetImageList(self.treeImageList)

		wx.EVT_TREE_ITEM_RIGHT_CLICK(self, treeCtrlID, self.OnRightClickCB)

		self.edited = 0 # Store if the XML file was edited

		try:
			if(self.configDir in os.listdir(self.configBasePath)):
				if(self.configFile in os.listdir(self.configPath)):
					self.doc = self.BuildDocFromXml(self.configPath + os.sep + self.configFile)
					self.UpdatePlugged(self.doc)
					self.NewDevices(self.doc)
				else:
					self.doc = self.BuildDocFromDriver()
			else:
				os.mkdir(self.configPath, 0755)
				self.doc = self.BuildDocFromDriver()
			self.BuildTreeCtrl(self.treeCtrl, self.doc)
			self.treeCtrl.Expand(self.treeCtrl.GetRootItem())
			if(self.edited == 1):
				dlg = wx.MessageDialog(self, "Hardware setup was changed.", "Information", wx.OK | wx.ICON_INFORMATION)
				dlg.ShowModal()
				dlg.Destroy()
		except DOMException, value:
			dlg = wx.MessageDialog(self, str(value), "Error from DOM implementation", wx.OK | wx.ICON_ERROR)
			dlg.ShowModal()
			dlg.Destroy()
			sys.exit(1)
		except ValueError, value:
			dlg = wx.MessageDialog(self, str(value), "Error while parsing configuration file", wx.OK | wx.ICON_ERROR)
			dlg.ShowModal()
			dlg.Destroy()
			sys.exit(1)
		except meDriver.error, value:
			dlg = wx.MessageDialog(self, str(value), "Error from ME driver system", wx.OK | wx.ICON_ERROR)
			dlg.ShowModal()
			dlg.Destroy()
			sys.exit(1)
		except:
			exc = sys.exc_info()
			txt = str(exc[0]) + '\n' + str(exc[1]) + '\n' + str(exc[2])
			dlg = wx.MessageDialog(self, txt, "Unexpected error while building DOM", wx.OK | wx.ICON_ERROR)
			dlg.ShowModal()
			dlg.Destroy()
			sys.exit(1)

	def NewDevices(self, doc):
		docRoot = doc.documentElement
		docDeviceList = docRoot.getElementsByTagName("device_list")[0]
		docDeviceEntries = docDeviceList.getElementsByTagName("device_entry")

		meDriver.meOpen(meDriver.ME_OPEN_NO_FLAGS)
		for i in range(len(docDeviceEntries), meDriver.meQueryNumberDevices()):
			self.edited = 1
			info = meDriver.meQueryInfoDevice(i)

			docDeviceEntry = doc.createElement("device_entry")
			docDeviceEntry.setAttribute("description", "Device")
			docDeviceEntry.setAttribute("device_number", str(i))
			docDeviceEntry.setAttribute("device_plugged", str(info[7]))
			docDeviceEntry.setAttribute("bus", str(info[3]))

			if((info[3] == meDriver.ME_BUS_TYPE_PCI) or (info[3] == meDriver.ME_BUS_TYPE_USB)):
				docDeviceEntry.setAttribute("access", str(meDriver.ME_ACCESS_TYPE_LOCAL))
			else:
				docDeviceEntry.setAttribute("access", str(meDriver.ME_ACCESS_TYPE_REMOTE))

			docDeviceList.appendChild(docDeviceEntry)

			docDeviceInfo = doc.createElement("device_info")
			docDeviceInfo.setAttribute("description", "Device info")
			docDeviceInfo.setAttribute("bus", str(info[3]))
			docDeviceEntry.appendChild(docDeviceInfo)

			docDeviceName = doc.createElement("device_name")
			docDeviceName.setAttribute("description", "Device name")
			docDeviceName.appendChild(doc.createTextNode(str(meDriver.meQueryNameDevice(i))))
			docDeviceInfo.appendChild(docDeviceName)

			docDeviceDescription = doc.createElement("device_description")
			docDeviceDescription.setAttribute("description", "Device description")
			docDeviceDescription.appendChild(doc.createTextNode(str(meDriver.meQueryDescriptionDevice(i))))
			docDeviceInfo.appendChild(docDeviceDescription)

			docDeviceVendorId = doc.createElement("vendor_id")
			docDeviceVendorId.setAttribute("description", "Vendor ID")
			docDeviceVendorId.appendChild(doc.createTextNode(str(info[0])))
			docDeviceInfo.appendChild(docDeviceVendorId)

			docDeviceDeviceId = doc.createElement("device_id")
			docDeviceDeviceId.setAttribute("description", "Device ID")
			docDeviceDeviceId.appendChild(doc.createTextNode(str(info[1])))
			docDeviceInfo.appendChild(docDeviceDeviceId)

			docDeviceSerialNo = doc.createElement("serial_no")
			docDeviceSerialNo.setAttribute("description", "Serial number")
			docDeviceSerialNo.appendChild(doc.createTextNode(str(info[2])))
			docDeviceInfo.appendChild(docDeviceSerialNo)

			if(info[3] == meDriver.ME_BUS_TYPE_PCI):
				docDevicePciBusNo = doc.createElement("pci_bus_no")
				docDevicePciBusNo.setAttribute("description", "PCI bus number")
				docDevicePciBusNo.appendChild(doc.createTextNode(str(info[4])))
				docDeviceInfo.appendChild(docDevicePciBusNo)

				docDevicePciDevNo = doc.createElement("pci_dev_no")
				docDevicePciDevNo.setAttribute("description", "PCI device number")
				docDevicePciDevNo.appendChild(doc.createTextNode(str(info[5])))
				docDeviceInfo.appendChild(docDevicePciDevNo)

				docDevicePciFuncNo = doc.createElement("pci_func_no")
				docDevicePciFuncNo.setAttribute("description", "PCI function number")
				docDevicePciFuncNo.appendChild(doc.createTextNode(str(info[6])))
				docDeviceInfo.appendChild(docDevicePciFuncNo)
			elif(info[3] == meDriver.ME_BUS_TYPE_USB):
				docDeviceUsbBusNo = doc.createElement("usb_root_hub_no")
				docDeviceUsbBusNo.setAttribute("description", "USB root hub number")
				docDeviceUsbBusNo.appendChild(doc.createTextNode(str(info[4])))
				docDeviceInfo.appendChild(docDeviceUsbBusNo)
			else:
				docDevicePNP = doc.createElement("remote_PNP")
				docDevicePNP.setAttribute("description", "Plug&Play remote device.")
				docDevicePNP.appendChild(doc.createTextNode(str("LOCAL NETWORK")))
				docDeviceInfo.appendChild(docDevicePNP)

			if(info[7] == meDriver.ME_PLUGGED_IN):
				docSubdeviceList = doc.createElement("subdevice_list")
				docSubdeviceList.setAttribute("description", "Subdevice list")
				docDeviceEntry.appendChild(docSubdeviceList)

				for j in range(meDriver.meQueryNumberSubdevices(i)):
					type = meDriver.meQuerySubdeviceType(i, j)

					docSubdeviceEntry = doc.createElement("subdevice_entry")
					docSubdeviceEntry.setAttribute("description", "Subdevice")
					docSubdeviceEntry.setAttribute("subdevice_number", str(j))
					docSubdeviceEntry.setAttribute("subdevice_type", str(type[0]))
					docSubdeviceEntry.setAttribute("subdevice_extension", str(meDriver.ME_EXTENSION_TYPE_NONE))
					docSubdeviceEntry.setAttribute("subdevice_lock", "0")
					docSubdeviceList.appendChild(docSubdeviceEntry)

					docSubdeviceInfo = doc.createElement("subdevice_info")
					docSubdeviceInfo.setAttribute("description", "Subdevice info")
					docSubdeviceEntry.appendChild(docSubdeviceInfo)

					docSubdeviceType = doc.createElement("subdevice_type")
					docSubdeviceType.setAttribute("description", "Subdevice type")
					docSubdeviceType.appendChild(doc.createTextNode(str(type[0])))
					docSubdeviceInfo.appendChild(docSubdeviceType)

					docSubdeviceSubtype = doc.createElement("subdevice_sub_type")
					docSubdeviceSubtype.setAttribute("description", "Subdevice subtype")
					docSubdeviceSubtype.appendChild(doc.createTextNode(str(type[1])))
					docSubdeviceInfo.appendChild(docSubdeviceSubtype)

					docSubdeviceNoChannels = doc.createElement("subdevice_number_channels")
					docSubdeviceNoChannels.setAttribute("description", "Number of channels")
					docSubdeviceNoChannels.appendChild(doc.createTextNode(str(meDriver.meQueryNumberChannels(i, j))))
					docSubdeviceInfo.appendChild(docSubdeviceNoChannels)

					if((type[0] == meDriver.ME_TYPE_AO) or (type[0] == meDriver.ME_TYPE_AI)):
						docRangeList = doc.createElement("range_list")
						docRangeList.setAttribute("description", "Range list")
						docSubdeviceEntry.appendChild(docRangeList)

						for k in range(meDriver.meQueryNumberRanges(i, j, meDriver.ME_UNIT_ANY)):
							docRangeEntry = doc.createElement("range_entry")
							docRangeEntry.setAttribute("description", "Range")
							docRangeEntry.setAttribute("range_number", str(k))
							docRangeList.appendChild(docRangeEntry)

							info = meDriver.meQueryRangeInfo(i, j, k)

							docRangeUnit = doc.createElement("range_unit")
							docRangeUnit.setAttribute("description", "Physical unit")
							docRangeUnit.appendChild(doc.createTextNode(str(info[0])))
							docRangeEntry.appendChild(docRangeUnit)

							docRangeMin = doc.createElement("range_min")
							docRangeMin.setAttribute("description", "Minimum physical value")
							docRangeMin.appendChild(doc.createTextNode(str(info[1])))
							docRangeEntry.appendChild(docRangeMin)

							docRangeMax = doc.createElement("range_max")
							docRangeMax.setAttribute("description", "Maximum physical value")
							docRangeMax.appendChild(doc.createTextNode(str(info[2])))
							docRangeEntry.appendChild(docRangeMax)

							docRangeMaxData = doc.createElement("range_max_data")
							docRangeMaxData.setAttribute("description", "Maximum digital value")
							docRangeMaxData.appendChild(doc.createTextNode(str(info[3])))
							docRangeEntry.appendChild(docRangeMaxData)
		meDriver.meClose(meDriver.ME_CLOSE_NO_FLAGS)

	def UpdatePlugged(self, doc):
		docRoot = doc.documentElement
		docDeviceList = docRoot.getElementsByTagName("device_list")[0]
		docDeviceEntries = docDeviceList.getElementsByTagName("device_entry")

		i = 0
		meDriver.meOpen(meDriver.ME_OPEN_NO_FLAGS)
		for docDeviceEntry in docDeviceEntries:
			try:
				info = meDriver.meQueryInfoDevice(i)
				if((int(docDeviceEntry.getAttribute("device_plugged")) == meDriver.ME_PLUGGED_IN) and info[7] == meDriver.ME_PLUGGED_OUT):
					self.edited = 1
					docDeviceEntry.setAttribute("device_plugged", str(meDriver.ME_PLUGGED_OUT))
				elif((int(docDeviceEntry.getAttribute("device_plugged")) == meDriver.ME_PLUGGED_OUT) and info[7] == meDriver.ME_PLUGGED_IN):
					self.edited = 1
					docDeviceEntry.setAttribute("device_plugged", str(meDriver.ME_PLUGGED_IN))

					docDeviceInfo = docDeviceEntry.getElementsByTagName("device_info")[0]

					docDeviceSerialNo = docDeviceInfo.getElementsByTagName("serial_no")[0]
					docDeviceSerialNo.firstChild.data = str(info[2])

					if(info[3] == meDriver.ME_BUS_TYPE_PCI):
						docDevicePciBusNo = docDeviceInfo.getElementsByTagName("pci_bus_no")[0]
						docDevicePciBusNo.firstChild.data = str(info[4])

						docDevicePciDevNo = docDeviceInfo.getElementsByTagName("pci_dev_no")[0]
						docDevicePciDevNo.firstChild.data = str(info[5])

						docDevicePciFuncNo = docDeviceInfo.getElementsByTagName("pci_func_no")[0]
						docDevicePciFuncNo.firstChild.data = str(info[6])
					elif(info[3] == meDriver.ME_BUS_TYPE_USB):
						docDeviceUsbBusNo = docDeviceInfo.getElementsByTagName("usb_root_hub_no")[0]
						docDeviceUsbBusNo.firstChild.data = str(info[4])
					else:
						docDevicePNP = docDeviceInfo.getElementsByTagName("remote_PNP")[0]
#						docDevicePNP.firstChild.data = str("LOCAL NETWORK")

					if(not docDeviceEntry.getElementsByTagName("subdevice_list")):
						docSubdeviceList = doc.createElement("subdevice_list")
						docSubdeviceList.setAttribute("description", "Subdevice list")
						docDeviceEntry.appendChild(docSubdeviceList)

						for j in range(meDriver.meQueryNumberSubdevices(i)):
							type = meDriver.meQuerySubdeviceType(i, j)

							docSubdeviceEntry = doc.createElement("subdevice_entry")
							docSubdeviceEntry.setAttribute("description", "Subdevice")
							docSubdeviceEntry.setAttribute("subdevice_number", str(j))
							docSubdeviceEntry.setAttribute("subdevice_type", str(type[0]))
							docSubdeviceEntry.setAttribute("subdevice_extension", str(meDriver.ME_EXTENSION_TYPE_NONE))
							docSubdeviceEntry.setAttribute("subdevice_lock", "0")
							docSubdeviceList.appendChild(docSubdeviceEntry)

							docSubdeviceInfo = doc.createElement("subdevice_info")
							docSubdeviceInfo.setAttribute("description", "Subdevice info")
							docSubdeviceEntry.appendChild(docSubdeviceInfo)

							docSubdeviceType = doc.createElement("subdevice_type")
							docSubdeviceType.setAttribute("description", "Subdevice type")
							docSubdeviceType.appendChild(doc.createTextNode(str(type[0])))
							docSubdeviceInfo.appendChild(docSubdeviceType)

							docSubdeviceSubtype = doc.createElement("subdevice_sub_type")
							docSubdeviceSubtype.setAttribute("description", "Subdevice subtype")
							docSubdeviceSubtype.appendChild(doc.createTextNode(str(type[1])))
							docSubdeviceInfo.appendChild(docSubdeviceSubtype)

							docSubdeviceNoChannels = doc.createElement("subdevice_number_channels")
							docSubdeviceNoChannels.setAttribute("description", "Number of channels")
							docSubdeviceNoChannels.appendChild(doc.createTextNode(str(meDriver.meQueryNumberChannels(i, j))))
							docSubdeviceInfo.appendChild(docSubdeviceNoChannels)

							if((type[0] == meDriver.ME_TYPE_AO) or (type[0] == meDriver.ME_TYPE_AI)):
								docRangeList = doc.createElement("range_list")
								docRangeList.setAttribute("description", "Range list")
								docSubdeviceEntry.appendChild(docRangeList)

								for k in range(meDriver.meQueryNumberRanges(i, j, meDriver.ME_UNIT_ANY)):
									docRangeEntry = doc.createElement("range_entry")
									docRangeEntry.setAttribute("description", "Range")
									docRangeEntry.setAttribute("range_number", str(k))
									docRangeList.appendChild(docRangeEntry)

									info = meDriver.meQueryRangeInfo(i, j, k)

									docRangeUnit = doc.createElement("range_unit")
									docRangeUnit.setAttribute("description", "Physical unit")
									docRangeUnit.appendChild(doc.createTextNode(str(info[0])))
									docRangeEntry.appendChild(docRangeUnit)

									docRangeMin = doc.createElement("range_min")
									docRangeMin.setAttribute("description", "Minimum physical value")
									docRangeMin.appendChild(doc.createTextNode(str(info[1])))
									docRangeEntry.appendChild(docRangeMin)

									docRangeMax = doc.createElement("range_max")
									docRangeMax.setAttribute("description", "Maximum physical value")
									docRangeMax.appendChild(doc.createTextNode(str(info[2])))
									docRangeEntry.appendChild(docRangeMax)

									docRangeMaxData = doc.createElement("range_max_data")
									docRangeMaxData.setAttribute("description", "Maximum digital value")
									docRangeMaxData.appendChild(doc.createTextNode(str(info[3])))
									docRangeEntry.appendChild(docRangeMaxData)
			except meDriver.error:
				if(int(docDeviceEntry.getAttribute("device_plugged")) == meDriver.ME_PLUGGED_IN):
					self.edited = 1
					docDeviceEntry.setAttribute("device_plugged", str(meDriver.ME_PLUGGED_OUT))

			i = i + 1

		meDriver.meClose(meDriver.ME_CLOSE_NO_FLAGS)

	def OnRightClickCB(self, event):
		if not hasattr(self, "delDeviceId"):
			self.delDeviceId = wx.NewId()
			self.upDeviceId = wx.NewId()
			self.downDeviceId = wx.NewId()
			wx.EVT_MENU(self, self.delDeviceId, self.DeleteDeviceEntryCB)
			wx.EVT_MENU(self, self.upDeviceId, self.UpCB)
			wx.EVT_MENU(self, self.downDeviceId, self.DownCB)

			self.newMux32mId = wx.NewId()
			wx.EVT_MENU(self, self.newMux32mId, self.NewMux32mEntryCB)

			self.newMux32sId = wx.NewId()
			wx.EVT_MENU(self, self.newMux32sId, self.NewMux32sEntryCB)

			self.newDemux32Id = wx.NewId()
			wx.EVT_MENU(self, self.newDemux32Id, self.NewDemux32EntryCB)

			self.delMux32mId = wx.NewId()
			self.editMux32mId = wx.NewId()
			wx.EVT_MENU(self, self.delMux32mId, self.DeleteMux32mEntryCB)
			wx.EVT_MENU(self, self.editMux32mId, self.EditMux32mEntryCB)

			self.delMux32sId = wx.NewId()
			wx.EVT_MENU(self, self.delMux32sId, self.DeleteMux32sEntryCB)

			self.delDemux32Id = wx.NewId()
			self.editDemux32Id = wx.NewId()
			wx.EVT_MENU(self, self.delDemux32Id, self.DeleteDemux32EntryCB)
			wx.EVT_MENU(self, self.editDemux32Id, self.EditDemux32EntryCB)

			self.addME1000ExtensionId = wx.NewId()
			self.deleteME1000ExtensionId = wx.NewId()
			wx.EVT_MENU(self, self.addME1000ExtensionId, self.AddME1000ExtensionCB)
			wx.EVT_MENU(self, self.deleteME1000ExtensionId, self.DeleteME1000ExtensionCB)

			self.addRemoteDeviceId = wx.NewId()
			wx.EVT_MENU(self, self.addRemoteDeviceId, self.AddRemoteDeviceCB)

		self.treeCtrl.SelectItem(event.GetItem())
		docNode = self.treeCtrl.GetPyData(event.GetItem())

		if(docNode.nodeName == "device_entry"):
			menu = wx.Menu()
			item = wx.MenuItem(menu, self.delDeviceId, "Delete device entry")
			item.SetBitmap(self.xpmDelete)
			menu.AppendItem(item)
			item = wx.MenuItem(menu, self.upDeviceId, "Move device entry up")
			item.SetBitmap(self.xpmUp)
			menu.AppendItem(item)
			item = wx.MenuItem(menu, self.downDeviceId, "Move device entry down")
			item.SetBitmap(self.xpmDown)
			menu.AppendItem(item)

			self.PopupMenu(menu, event.GetPoint())
			menu.Destroy()
		elif(docNode.nodeName == "mux32m"):
			menu = wx.Menu()
			item = wx.MenuItem(menu, self.delMux32mId, "Delete ME-MUX32-M")
			item.SetBitmap(self.xpmDelete)
			menu.AppendItem(item)
			item = wx.MenuItem(menu, self.editMux32mId, "Edit ME-MUX32-M")
			item.SetBitmap(self.xpmEdit)
			menu.AppendItem(item)

			self.PopupMenu(menu, event.GetPoint())
			menu.Destroy()
		elif(docNode.nodeName == "mux32s_entry"):
			menu = wx.Menu()
			item = wx.MenuItem(menu, self.delMux32sId, "Delete ME-MUX32-S")
			item.SetBitmap(self.xpmDelete)
			menu.AppendItem(item)

			self.PopupMenu(menu, event.GetPoint())
			menu.Destroy()
		elif(docNode.nodeName == "demux32"):
			menu = wx.Menu()
			item = wx.MenuItem(menu, self.delDemux32Id, "Delete ME-DEMUX32")
			item.SetBitmap(self.xpmDelete)
			menu.AppendItem(item)
			item = wx.MenuItem(menu, self.editDemux32Id, "Edit ME-DEMUX32")
			item.SetBitmap(self.xpmEdit)
			menu.AppendItem(item)

			self.PopupMenu(menu, event.GetPoint())
			menu.Destroy()
		elif((docNode.nodeName == "subdevice_entry") and (int(docNode.getAttribute("subdevice_type")) == meDriver.ME_TYPE_AI)):
			menu = wx.Menu()
			item = wx.MenuItem(menu, self.newMux32mId, "Register a ME-MUX32-M")
			item.SetBitmap(self.xpmMultisig)
			menu.AppendItem(item)

			self.PopupMenu(menu, event.GetPoint())
			menu.Destroy()
		elif((docNode.nodeName == "subdevice_entry") and (int(docNode.getAttribute("subdevice_type")) == meDriver.ME_TYPE_AO)):
			menu = wx.Menu()
			item = wx.MenuItem(menu, self.newDemux32Id, "Register a ME-DEMUX32")
			item.SetBitmap(self.xpmMultisig)
			menu.AppendItem(item)

			self.PopupMenu(menu, event.GetPoint())
			menu.Destroy()
		elif(docNode.nodeName == "mux32s_list"):
			menu = wx.Menu()
			item = wx.MenuItem(menu, self.newMux32sId, "Register a ME-MUX32-S")
			item.SetBitmap(self.xpmMultisig)
			menu.AppendItem(item)

			self.PopupMenu(menu, event.GetPoint())
			menu.Destroy()
		elif(docNode.nodeName == "subdevice_list"):
			docDeviceEntry = docNode.parentNode
			docDeviceInfo = docDeviceEntry.getElementsByTagName("device_info")[0]
			docDeviceId = docDeviceInfo.getElementsByTagName("device_id")[0]
			if(int(docDeviceId.firstChild.data) == 0x100B):
				if(len(docNode.getElementsByTagName("subdevice_entry")) == 2):
					menu = wx.Menu()
					item = wx.MenuItem(menu, self.addME1000ExtensionId, "Register a ME-1001")
					item.SetBitmap(self.xpmME1001Add)
					menu.AppendItem(item)

					self.PopupMenu(menu, event.GetPoint())
					menu.Destroy()
				elif(len(docNode.getElementsByTagName("subdevice_entry")) == 4):
					menu = wx.Menu()
					item = wx.MenuItem(menu, self.deleteME1000ExtensionId, "Delete ME-1001")
					item.SetBitmap(self.xpmME1001Delete)
					menu.AppendItem(item)

					self.PopupMenu(menu, event.GetPoint())
					menu.Destroy()
		elif(docNode.nodeName == "device_list"):
			menu = wx.Menu()
			item = wx.MenuItem(menu, self.addRemoteDeviceId, "Register a remote device")
			item.SetBitmap(self.xpmRemoteDevice)
			menu.AppendItem(item)

			self.PopupMenu(menu, event.GetPoint())
			menu.Destroy()

	def BuildDocFromDriver(self):
		impl = getDOMImplementation()
		doct = impl.createDocumentType("medrvconfig", None, self.configBasePath + os.sep + self.configDir + os.sep + self.dtdFile)
		doc = impl.createDocument(None, "medrvconfig", doct)

		docRoot = doc.documentElement

		docDate = doc.createElement("date")
		docDate.setAttribute("description", "Date last modified")
		docDate.appendChild(doc.createTextNode(time.asctime()))
		docRoot.appendChild(docDate)

		docDeviceList = doc.createElement("device_list")
		docDeviceList.setAttribute("description", "Device list")
		docRoot.appendChild(docDeviceList)

		meDriver.meOpen(meDriver.ME_OPEN_NO_FLAGS)
		for i in range(meDriver.meQueryNumberDevices()):
			info = meDriver.meQueryInfoDevice(i)

			docDeviceEntry = doc.createElement("device_entry")
			docDeviceEntry.setAttribute("description", "Device")
			docDeviceEntry.setAttribute("device_number", str(i))
			docDeviceEntry.setAttribute("device_plugged", str(info[7]))
			docDeviceEntry.setAttribute("access", str(meDriver.ME_ACCESS_TYPE_LOCAL))
			docDeviceEntry.setAttribute("bus", str(info[3]))
			docDeviceList.appendChild(docDeviceEntry)

			docDeviceInfo = doc.createElement("device_info")
			docDeviceInfo.setAttribute("description", "Device info")
			docDeviceInfo.setAttribute("bus", str(info[3]))
			docDeviceEntry.appendChild(docDeviceInfo)

			docDeviceName = doc.createElement("device_name")
			docDeviceName.setAttribute("description", "Device name")
			docDeviceName.appendChild(doc.createTextNode(str(meDriver.meQueryNameDevice(i))))
			docDeviceInfo.appendChild(docDeviceName)

			docDeviceDescription = doc.createElement("device_description")
			docDeviceDescription.setAttribute("description", "Device description")
			docDeviceDescription.appendChild(doc.createTextNode(str(meDriver.meQueryDescriptionDevice(i))))
			docDeviceInfo.appendChild(docDeviceDescription)

			docDeviceVendorId = doc.createElement("vendor_id")
			docDeviceVendorId.setAttribute("description", "Vendor ID")
			docDeviceVendorId.appendChild(doc.createTextNode(str(info[0])))
			docDeviceInfo.appendChild(docDeviceVendorId)

			docDeviceDeviceId = doc.createElement("device_id")
			docDeviceDeviceId.setAttribute("description", "Device ID")
			docDeviceDeviceId.appendChild(doc.createTextNode(str(info[1])))
			docDeviceInfo.appendChild(docDeviceDeviceId)

			docDeviceSerialNo = doc.createElement("serial_no")
			docDeviceSerialNo.setAttribute("description", "Serial number")
			docDeviceSerialNo.appendChild(doc.createTextNode(str(info[2])))
			docDeviceInfo.appendChild(docDeviceSerialNo)

			if(info[3] == meDriver.ME_BUS_TYPE_PCI):
				docDevicePciBusNo = doc.createElement("pci_bus_no")
				docDevicePciBusNo.setAttribute("description", "PCI bus number")
				docDevicePciBusNo.appendChild(doc.createTextNode(str(info[4])))
				docDeviceInfo.appendChild(docDevicePciBusNo)

				docDevicePciDevNo = doc.createElement("pci_dev_no")
				docDevicePciDevNo.setAttribute("description", "PCI device number")
				docDevicePciDevNo.appendChild(doc.createTextNode(str(info[5])))
				docDeviceInfo.appendChild(docDevicePciDevNo)

				docDevicePciFuncNo = doc.createElement("pci_func_no")
				docDevicePciFuncNo.setAttribute("description", "PCI function number")
				docDevicePciFuncNo.appendChild(doc.createTextNode(str(info[6])))
				docDeviceInfo.appendChild(docDevicePciFuncNo)
			elif(info[3] == meDriver.ME_BUS_TYPE_USB):
				docDeviceUsbBusNo = doc.createElement("usb_root_hub_no")
				docDeviceUsbBusNo.setAttribute("description", "USB root hub number")
				docDeviceUsbBusNo.appendChild(doc.createTextNode(str(info[4])))
				docDeviceInfo.appendChild(docDeviceUsbBusNo)
			else:
				docDevicePNP = doc.createElement("remote_PNP")
				docDevicePNP.setAttribute("description", "Plug&Play remote device.")
				docDevicePNP.appendChild(doc.createTextNode(str("LOCAL NETWORK")))
				docDeviceInfo.appendChild(docDevicePNP)

			if(info[7] == meDriver.ME_PLUGGED_IN):
				docSubdeviceList = doc.createElement("subdevice_list")
				docSubdeviceList.setAttribute("description", "Subdevice list")
				docDeviceEntry.appendChild(docSubdeviceList)

				for j in range(meDriver.meQueryNumberSubdevices(i)):
					type = meDriver.meQuerySubdeviceType(i, j)

					docSubdeviceEntry = doc.createElement("subdevice_entry")
					docSubdeviceEntry.setAttribute("description", "Subdevice")
					docSubdeviceEntry.setAttribute("subdevice_number", str(j))
					docSubdeviceEntry.setAttribute("subdevice_type", str(type[0]))
					docSubdeviceEntry.setAttribute("subdevice_extension", str(meDriver.ME_EXTENSION_TYPE_NONE))
					docSubdeviceEntry.setAttribute("subdevice_lock", "0")
					docSubdeviceList.appendChild(docSubdeviceEntry)

					docSubdeviceInfo = doc.createElement("subdevice_info")
					docSubdeviceInfo.setAttribute("description", "Subdevice info")
					docSubdeviceEntry.appendChild(docSubdeviceInfo)

					docSubdeviceType = doc.createElement("subdevice_type")
					docSubdeviceType.setAttribute("description", "Subdevice type")
					docSubdeviceType.appendChild(doc.createTextNode(str(type[0])))
					docSubdeviceInfo.appendChild(docSubdeviceType)

					docSubdeviceSubtype = doc.createElement("subdevice_sub_type")
					docSubdeviceSubtype.setAttribute("description", "Subdevice subtype")
					docSubdeviceSubtype.appendChild(doc.createTextNode(str(type[1])))
					docSubdeviceInfo.appendChild(docSubdeviceSubtype)

					docSubdeviceNoChannels = doc.createElement("subdevice_number_channels")
					docSubdeviceNoChannels.setAttribute("description", "Number of channels")
					docSubdeviceNoChannels.appendChild(doc.createTextNode(str(meDriver.meQueryNumberChannels(i, j))))
					docSubdeviceInfo.appendChild(docSubdeviceNoChannels)

					if((type[0] == meDriver.ME_TYPE_AO) or (type[0] == meDriver.ME_TYPE_AI)):
						docRangeList = doc.createElement("range_list")
						docRangeList.setAttribute("description", "Range list")
						docSubdeviceEntry.appendChild(docRangeList)

						for k in range(meDriver.meQueryNumberRanges(i, j, meDriver.ME_UNIT_ANY)):
							docRangeEntry = doc.createElement("range_entry")
							docRangeEntry.setAttribute("description", "Range")
							docRangeEntry.setAttribute("range_number", str(k))
							docRangeList.appendChild(docRangeEntry)

							info = meDriver.meQueryRangeInfo(i, j, k)

							docRangeUnit = doc.createElement("range_unit")
							docRangeUnit.setAttribute("description", "Physical unit")
							docRangeUnit.appendChild(doc.createTextNode(str(info[0])))
							docRangeEntry.appendChild(docRangeUnit)

							docRangeMin = doc.createElement("range_min")
							docRangeMin.setAttribute("description", "Minimum physical value")
							docRangeMin.appendChild(doc.createTextNode(str(info[1])))
							docRangeEntry.appendChild(docRangeMin)

							docRangeMax = doc.createElement("range_max")
							docRangeMax.setAttribute("description", "Maximum physical value")
							docRangeMax.appendChild(doc.createTextNode(str(info[2])))
							docRangeEntry.appendChild(docRangeMax)

							docRangeMaxData = doc.createElement("range_max_data")
							docRangeMaxData.setAttribute("description", "Maximum digital value")
							docRangeMaxData.appendChild(doc.createTextNode(str(info[3])))
							docRangeEntry.appendChild(docRangeMaxData)

		meDriver.meClose(meDriver.ME_CLOSE_NO_FLAGS)
		return doc

	def RemoveWhiteSpaceFromDoc(self, node, unlink=False):
		"""Removes all of the whitespace-only text decendants of a DOM node.

		When creating a DOM from an XML source, XML parsers are required to
		consider several conditions when deciding whether to include
		whitespace-only text nodes. This function ignores all of those
		conditions and removes all whitespace-only text decendants of the
		specified node. If the unlink flag is specified, the removed text
		nodes are unlinked so that their storage can be reclaimed. If the
		specified node is a whitespace-only text node then it is left
		unmodified."""

		remove_list = []
		for child in node.childNodes:
			if(child.nodeType == Node.TEXT_NODE):
				if(not child.data.strip()):
					remove_list.append(child)
				else:
					child.data = child.data.strip()
			elif child.hasChildNodes():
				self.RemoveWhiteSpaceFromDoc(child, unlink)
		for node in remove_list:
			node.parentNode.removeChild(node)
			if unlink:
				node.unlink()

	def BuildDocFromXml(self, filename):
		doc = parse(filename)
		self.RemoveWhiteSpaceFromDoc(doc.documentElement, True)
		doc.normalize()
		return doc

	def BuildTreeCtrl(self, treeCtrl, doc):
		docRoot = doc.documentElement
		treeRoot = treeCtrl.AddRoot(docRoot.getAttribute("description"))
		treeCtrl.SetItemImage(treeRoot, self.treeImageConfiguration, which = wx.TreeItemIcon_Normal)
		treeCtrl.SetItemImage(treeRoot, self.treeImageConfiguration, which = wx.TreeItemIcon_Expanded)
		treeCtrl.SetPyData(treeRoot, docRoot)

		self.BuildSubtree(treeRoot, docRoot)

	def BuildSubtree(self, treeRoot, docRoot):
		for child in docRoot.childNodes:
			if child.nodeType == Node.ELEMENT_NODE:
				if child.nodeName == "date":
					treeSub = self.treeCtrl.AppendItem(treeRoot, child.getAttribute("description"))
					self.treeCtrl.SetItemImage(treeSub, self.treeImageCalender, which = wx.TreeItemIcon_Normal)
					self.treeCtrl.SetItemImage(treeSub, self.treeImageCalender, which = wx.TreeItemIcon_Expanded)
					self.treeCtrl.SetItemText(treeSub, self.DefineToString(child.firstChild), 1)
				elif child.nodeName == "device_list":
					treeSub = self.treeCtrl.AppendItem(treeRoot, child.getAttribute("description"))
					self.treeCtrl.SetItemImage(treeSub, self.treeImageDevices, which = wx.TreeItemIcon_Normal)
					self.treeCtrl.SetItemImage(treeSub, self.treeImageDevices, which = wx.TreeItemIcon_Expanded)
				elif child.nodeName == "device_entry":
					treeSub = self.treeCtrl.AppendItem(treeRoot, child.getAttribute("description") + " " + child.getAttribute("device_number"))
					if(int(child.getAttribute("access")) == meDriver.ME_ACCESS_TYPE_LOCAL):
						if(int(child.getAttribute("bus")) == meDriver.ME_BUS_TYPE_PCI):
							if(int(child.getAttribute("device_plugged")) == meDriver.ME_PLUGGED_IN):
								self.treeCtrl.SetItemImage(treeSub, self.treeImageDevice, which = wx.TreeItemIcon_Normal)
								self.treeCtrl.SetItemImage(treeSub, self.treeImageDevice, which = wx.TreeItemIcon_Expanded)
							else:
								self.treeCtrl.SetItemImage(treeSub, self.treeImageDeviceUnplugged, which = wx.TreeItemIcon_Normal)
								self.treeCtrl.SetItemImage(treeSub, self.treeImageDeviceUnplugged, which = wx.TreeItemIcon_Expanded)
						elif(int(child.getAttribute("bus")) == meDriver.ME_BUS_TYPE_USB):
							if(int(child.getAttribute("device_plugged")) == meDriver.ME_PLUGGED_IN):
								self.treeCtrl.SetItemImage(treeSub, self.treeImageUsbDevice, which = wx.TreeItemIcon_Normal)
								self.treeCtrl.SetItemImage(treeSub, self.treeImageUsbDevice, which = wx.TreeItemIcon_Expanded)
							else:
								self.treeCtrl.SetItemImage(treeSub, self.treeImageUsbDeviceUnplugged, which = wx.TreeItemIcon_Normal)
								self.treeCtrl.SetItemImage(treeSub, self.treeImageUsbDeviceUnplugged, which = wx.TreeItemIcon_Expanded)
						else:
							if(int(child.getAttribute("device_plugged")) == meDriver.ME_PLUGGED_IN):
								self.treeCtrl.SetItemImage(treeSub, self.treeImagePNPdevice, which = wx.TreeItemIcon_Normal)
								self.treeCtrl.SetItemImage(treeSub, self.treeImagePNPdevice, which = wx.TreeItemIcon_Expanded)
							else:
								self.treeCtrl.SetItemImage(treeSub, self.treeImagePNPdeviceUnplugged, which = wx.TreeItemIcon_Normal)
								self.treeCtrl.SetItemImage(treeSub, self.treeImagePNPdeviceUnplugged, which = wx.TreeItemIcon_Expanded)
					else:
						if(int(child.getAttribute("device_plugged")) == meDriver.ME_PLUGGED_IN):
							self.treeCtrl.SetItemImage(treeSub, self.treeImageRemoteDevice, which = wx.TreeItemIcon_Normal)
							self.treeCtrl.SetItemImage(treeSub, self.treeImageRemoteDevice, which = wx.TreeItemIcon_Expanded)
						else:
							self.treeCtrl.SetItemImage(treeSub, self.treeImageRemoteDeviceUnplugged, which = wx.TreeItemIcon_Normal)
							self.treeCtrl.SetItemImage(treeSub, self.treeImageRemoteDeviceUnplugged, which = wx.TreeItemIcon_Expanded)
				elif child.nodeName == "tcpip":
					treeSub = self.treeCtrl.AppendItem(treeRoot, child.getAttribute("description"))
					self.treeCtrl.SetItemImage(treeSub, self.treeImageRemoteDeviceInfo, which = wx.TreeItemIcon_Normal)
					self.treeCtrl.SetItemImage(treeSub, self.treeImageRemoteDeviceInfo, which = wx.TreeItemIcon_Expanded)
				elif child.nodeName == "device_info":
					treeSub = self.treeCtrl.AppendItem(treeRoot, child.getAttribute("description"))
					self.treeCtrl.SetItemImage(treeSub, self.treeImageDeviceInfo, which = wx.TreeItemIcon_Normal)
					self.treeCtrl.SetItemImage(treeSub, self.treeImageDeviceInfo, which = wx.TreeItemIcon_Expanded)
				elif child.nodeName == "subdevice_list":
					treeSub = self.treeCtrl.AppendItem(treeRoot, child.getAttribute("description"))
					self.treeCtrl.SetItemImage(treeSub, self.treeImageSubdevices, which = wx.TreeItemIcon_Normal)
					self.treeCtrl.SetItemImage(treeSub, self.treeImageSubdevices, which = wx.TreeItemIcon_Expanded)
				elif child.nodeName == "subdevice_entry":
					treeSub = self.treeCtrl.AppendItem(treeRoot, child.getAttribute("description") + " " + child.getAttribute("subdevice_number"))
					if(int(child.getAttribute("subdevice_type")) == meDriver.ME_TYPE_AO):
						if(int(child.getAttribute("subdevice_lock"))):
							self.treeCtrl.SetItemImage(treeSub, self.treeImageSubdeviceLockedDA, which = wx.TreeItemIcon_Normal)
							self.treeCtrl.SetItemImage(treeSub, self.treeImageSubdeviceLockedDA, which = wx.TreeItemIcon_Expanded)
						else:
							self.treeCtrl.SetItemImage(treeSub, self.treeImageSubdeviceDA, which = wx.TreeItemIcon_Normal)
							self.treeCtrl.SetItemImage(treeSub, self.treeImageSubdeviceDA, which = wx.TreeItemIcon_Expanded)
					elif(int(child.getAttribute("subdevice_type")) == meDriver.ME_TYPE_AI):
						if(int(child.getAttribute("subdevice_lock"))):
							self.treeCtrl.SetItemImage(treeSub, self.treeImageSubdeviceLockedAD, which = wx.TreeItemIcon_Normal)
							self.treeCtrl.SetItemImage(treeSub, self.treeImageSubdeviceLockedAD, which = wx.TreeItemIcon_Expanded)
						else:
							self.treeCtrl.SetItemImage(treeSub, self.treeImageSubdeviceAD, which = wx.TreeItemIcon_Normal)
							self.treeCtrl.SetItemImage(treeSub, self.treeImageSubdeviceAD, which = wx.TreeItemIcon_Expanded)
					elif(int(child.getAttribute("subdevice_type")) == meDriver.ME_TYPE_DIO):
						if(int(child.getAttribute("subdevice_lock"))):
							self.treeCtrl.SetItemImage(treeSub, self.treeImageSubdeviceLockedDIO, which = wx.TreeItemIcon_Normal)
							self.treeCtrl.SetItemImage(treeSub, self.treeImageSubdeviceLockedDIO, which = wx.TreeItemIcon_Expanded)
						else:
							self.treeCtrl.SetItemImage(treeSub, self.treeImageSubdeviceDIO, which = wx.TreeItemIcon_Normal)
							self.treeCtrl.SetItemImage(treeSub, self.treeImageSubdeviceDIO, which = wx.TreeItemIcon_Expanded)
					elif(int(child.getAttribute("subdevice_type")) == meDriver.ME_TYPE_DO):
						if(int(child.getAttribute("subdevice_lock"))):
							self.treeCtrl.SetItemImage(treeSub, self.treeImageSubdeviceLockedDO, which = wx.TreeItemIcon_Normal)
							self.treeCtrl.SetItemImage(treeSub, self.treeImageSubdeviceLockedDO, which = wx.TreeItemIcon_Expanded)
						else:
							self.treeCtrl.SetItemImage(treeSub, self.treeImageSubdeviceDO, which = wx.TreeItemIcon_Normal)
							self.treeCtrl.SetItemImage(treeSub, self.treeImageSubdeviceDO, which = wx.TreeItemIcon_Expanded)
					elif(int(child.getAttribute("subdevice_type")) == meDriver.ME_TYPE_DI):
						if(int(child.getAttribute("subdevice_lock"))):
							self.treeCtrl.SetItemImage(treeSub, self.treeImageSubdeviceLockedDI, which = wx.TreeItemIcon_Normal)
							self.treeCtrl.SetItemImage(treeSub, self.treeImageSubdeviceLockedDI, which = wx.TreeItemIcon_Expanded)
						else:
							self.treeCtrl.SetItemImage(treeSub, self.treeImageSubdeviceDI, which = wx.TreeItemIcon_Normal)
							self.treeCtrl.SetItemImage(treeSub, self.treeImageSubdeviceDI, which = wx.TreeItemIcon_Expanded)
					elif(int(child.getAttribute("subdevice_type")) == meDriver.ME_TYPE_CTR):
						if(int(child.getAttribute("subdevice_lock"))):
							self.treeCtrl.SetItemImage(treeSub, self.treeImageSubdeviceLockedCTR, which = wx.TreeItemIcon_Normal)
							self.treeCtrl.SetItemImage(treeSub, self.treeImageSubdeviceLockedCTR, which = wx.TreeItemIcon_Expanded)
						else:
							self.treeCtrl.SetItemImage(treeSub, self.treeImageSubdeviceCTR, which = wx.TreeItemIcon_Normal)
							self.treeCtrl.SetItemImage(treeSub, self.treeImageSubdeviceCTR, which = wx.TreeItemIcon_Expanded)
					elif(int(child.getAttribute("subdevice_type")) == meDriver.ME_TYPE_EXT_IRQ):
						if(int(child.getAttribute("subdevice_lock"))):
							self.treeCtrl.SetItemImage(treeSub, self.treeImageSubdeviceLockedIRQ, which = wx.TreeItemIcon_Normal)
							self.treeCtrl.SetItemImage(treeSub, self.treeImageSubdeviceLockedIRQ, which = wx.TreeItemIcon_Expanded)
						else:
							self.treeCtrl.SetItemImage(treeSub, self.treeImageSubdeviceIRQ, which = wx.TreeItemIcon_Normal)
							self.treeCtrl.SetItemImage(treeSub, self.treeImageSubdeviceIRQ, which = wx.TreeItemIcon_Expanded)
					elif(int(child.getAttribute("subdevice_type")) == meDriver.ME_TYPE_FREQ_O):
						if(int(child.getAttribute("subdevice_lock"))):
							self.treeCtrl.SetItemImage(treeSub, self.treeImageSubdeviceLockedCTR, which = wx.TreeItemIcon_Normal)
							self.treeCtrl.SetItemImage(treeSub, self.treeImageSubdeviceLockedCTR, which = wx.TreeItemIcon_Expanded)
						else:
							self.treeCtrl.SetItemImage(treeSub, self.treeImageSubdeviceCTR, which = wx.TreeItemIcon_Normal)
							self.treeCtrl.SetItemImage(treeSub, self.treeImageSubdeviceCTR, which = wx.TreeItemIcon_Expanded)
					elif(int(child.getAttribute("subdevice_type")) == meDriver.ME_TYPE_FREQ_I):
						if(int(child.getAttribute("subdevice_lock"))):
							self.treeCtrl.SetItemImage(treeSub, self.treeImageSubdeviceLockedCTR, which = wx.TreeItemIcon_Normal)
							self.treeCtrl.SetItemImage(treeSub, self.treeImageSubdeviceLockedCTR, which = wx.TreeItemIcon_Expanded)
						else:
							self.treeCtrl.SetItemImage(treeSub, self.treeImageSubdeviceCTR, which = wx.TreeItemIcon_Normal)
							self.treeCtrl.SetItemImage(treeSub, self.treeImageSubdeviceCTR, which = wx.TreeItemIcon_Expanded)
				elif child.nodeName == "subdevice_info":
					treeSub = self.treeCtrl.AppendItem(treeRoot, child.getAttribute("description"))
					self.treeCtrl.SetItemImage(treeSub, self.treeImageSubdeviceInfo, which = wx.TreeItemIcon_Normal)
					self.treeCtrl.SetItemImage(treeSub, self.treeImageSubdeviceInfo, which = wx.TreeItemIcon_Expanded)
				elif child.nodeName == "range_list":
					treeSub = self.treeCtrl.AppendItem(treeRoot, child.getAttribute("description"))
					self.treeCtrl.SetItemImage(treeSub, self.treeImageRanges, which = wx.TreeItemIcon_Normal)
					self.treeCtrl.SetItemImage(treeSub, self.treeImageRanges, which = wx.TreeItemIcon_Expanded)
				elif child.nodeName == "range_entry":
					treeSub = self.treeCtrl.AppendItem(treeRoot, child.getAttribute("description") + " " + child.getAttribute("range_number"))
					self.treeCtrl.SetItemImage(treeSub, self.treeImageRange, which = wx.TreeItemIcon_Normal)
					self.treeCtrl.SetItemImage(treeSub, self.treeImageRange, which = wx.TreeItemIcon_Expanded)
				elif child.nodeName == "mux32m":
					treeSub = self.treeCtrl.AppendItem(treeRoot, child.getAttribute("description"))
					self.treeCtrl.SetItemImage(treeSub, self.treeImageMultisig, which = wx.TreeItemIcon_Normal)
					self.treeCtrl.SetItemImage(treeSub, self.treeImageMultisig, which = wx.TreeItemIcon_Expanded)
				elif child.nodeName == "mux32s_list":
					treeSub = self.treeCtrl.AppendItem(treeRoot, child.getAttribute("description"))
					self.treeCtrl.SetItemImage(treeSub, self.treeImageMultisigs, which = wx.TreeItemIcon_Normal)
					self.treeCtrl.SetItemImage(treeSub, self.treeImageMultisigs, which = wx.TreeItemIcon_Expanded)
				elif child.nodeName == "mux32s_entry":
					treeSub = self.treeCtrl.AppendItem(treeRoot, child.getAttribute("description") + " " + child.getAttribute("mux32s_number"))
					self.treeCtrl.SetItemImage(treeSub, self.treeImageMultisig, which = wx.TreeItemIcon_Normal)
					self.treeCtrl.SetItemImage(treeSub, self.treeImageMultisig, which = wx.TreeItemIcon_Expanded)
				elif child.nodeName == "demux32":
					treeSub = self.treeCtrl.AppendItem(treeRoot, child.getAttribute("description"))
					self.treeCtrl.SetItemImage(treeSub, self.treeImageMultisig, which = wx.TreeItemIcon_Normal)
					self.treeCtrl.SetItemImage(treeSub, self.treeImageMultisig, which = wx.TreeItemIcon_Expanded)
				elif((child.nodeName == "remote_host") or
					(child.nodeName == "remote_device_number") or
					(child.nodeName == "device_name") or
					(child.nodeName == "device_description") or
					(child.nodeName == "vendor_id") or
					(child.nodeName == "device_id") or
					(child.nodeName == "serial_no") or
					(child.nodeName == "pci_bus_no") or
					(child.nodeName == "pci_dev_no") or
					(child.nodeName == "pci_func_no") or
					(child.nodeName == "usb_root_hub_no") or
					(child.nodeName == "subdevice_type") or
					(child.nodeName == "subdevice_sub_type") or
					(child.nodeName == "subdevice_number_channels") or
					(child.nodeName == "range_unit") or
					(child.nodeName == "range_min") or
					(child.nodeName == "range_max") or
					(child.nodeName == "range_max_data") or
					(child.nodeName == "mux32m_ai_channel") or
					(child.nodeName == "mux32m_dio_device") or
					(child.nodeName == "mux32m_dio_subdevice") or
					(child.nodeName == "mux32m_timer_device") or
					(child.nodeName == "mux32m_timer_subdevice") or
					(child.nodeName == "demux32_ao_channel") or
					(child.nodeName == "demux32_dio_device") or
					(child.nodeName == "demux32_dio_subdevice") or
					(child.nodeName == "demux32_timer_device") or
					(child.nodeName == "demux32_timer_subdevice")):
					treeSub = self.treeCtrl.AppendItem(treeRoot, child.getAttribute("description"))
					self.treeCtrl.SetItemImage(treeSub, self.treeImageData, which = wx.TreeItemIcon_Normal)
					self.treeCtrl.SetItemImage(treeSub, self.treeImageData, which = wx.TreeItemIcon_Expanded)
					self.treeCtrl.SetItemText(treeSub, self.DefineToString(child.firstChild), 1)
				else:
					treeSub = self.treeCtrl.AppendItem(treeRoot, child.getAttribute("description"))

				self.treeCtrl.SetPyData(treeSub, child)
				self.BuildSubtree(treeSub, child)

	def DefineToString(self, child):
		if(child.parentNode.nodeName == "vendor_id"):
			return "0x%X" % int(child.data)
		elif(child.parentNode.nodeName == "device_id"):
			return "0x%X" % int(child.data)
		elif(child.parentNode.nodeName == "serial_no"):
			return "0x%08X" % int(child.data)
		elif(child.parentNode.nodeName == "subdevice_type"):
			if(int(child.data) == meDriver.ME_TYPE_AO):
				return "Analog output"
			elif(int(child.data) == meDriver.ME_TYPE_AI):
				return "Analog input"
			elif(int(child.data) == meDriver.ME_TYPE_DIO):
				return "Digital I/O"
			elif(int(child.data) == meDriver.ME_TYPE_DO):
				return "Digital output"
			elif(int(child.data) == meDriver.ME_TYPE_DI):
				return "Digital input"
			elif(int(child.data) == meDriver.ME_TYPE_CTR):
				return "Counter"
			elif(int(child.data) == meDriver.ME_TYPE_EXT_IRQ):
				return "External interrupt"
			elif(int(child.data) == meDriver.ME_TYPE_FREQ_I):
				return "Frequency input"
			elif(int(child.data) == meDriver.ME_TYPE_FREQ_O):
				return "Frequency output"
			else:
				raise ValueError, "Invalid value for subdevice type specified."
		elif(child.parentNode.nodeName == "subdevice_sub_type"):
			if(int(child.data) == meDriver.ME_SUBTYPE_SINGLE):
				return "Single"
			elif(int(child.data) == meDriver.ME_SUBTYPE_STREAMING):
				return "Streaming"
			elif(int(child.data) == meDriver.ME_SUBTYPE_CTR_8254):
				return "8254"
			else:
				raise ValueError, "Invalid value for subdevice subtype specified."
		elif(child.parentNode.nodeName == "range_unit"):
			if(int(child.data) == meDriver.ME_UNIT_VOLT):
				return "Volt"
			elif(int(child.data) == meDriver.ME_UNIT_AMPERE):
				return "Ampere"
			else:
				raise ValueError, "Invalid physical unit specified."
		else:
			return string.strip(str(child.data))

	def SaveCB(self, event):
		docRoot = self.doc.documentElement
		docDate = docRoot.getElementsByTagName("date")[0];
		docDate.firstChild.data = time.asctime()
		self.UpdateTreeCtrl()

		try:
			stream = open(self.configPath + os.sep + self.configFile, 'w')
			stream.write(self.doc.toprettyxml())
			stream.close()
			self.edited = 0

		except IOError, value:
			dlg = wx.MessageDialog(self, str(value), "Error while saving configuration", wx.OK | wx.ICON_ERROR)
			dlg.ShowModal()
			dlg.Destroy()

	def QuitCB(self, event):
		if(self.edited != 0):
			dlg = wx.MessageDialog(self, "There are unsaved changes.\nDo you really want to quit?", "Quit application", wx.YES_NO | wx.ICON_QUESTION)
			ok = dlg.ShowModal()
			dlg.Destroy()
			if(ok == wx.ID_YES):
				self.Destroy()
				return
		else:
			self.Destroy()
			return

	def InfoCB(self, event):
		dlg = wx.MessageDialog(self, "Version 1.1\n\nCopyright (2007) Meilhaus Electronic GmbH\n\nAutors: Guenter Gebhardt & Krzysztof Gantzke\n\nhttp://www.meilhaus.com", "Program information", wx.OK | wx.ICON_INFORMATION)
		dlg.ShowModal()
		dlg.Destroy()

	def HelpCB(self, event):
		win = helpDialog.helpDialog(self, self.rootPath + os.sep + "help" + os.sep + "help.html")
		win.Show(True)

	def EditMux32mEntryCB(self, event):
		treeSelected = self.treeCtrl.GetSelection()
		docSelected = self.treeCtrl.GetPyData(treeSelected)
		editMux32mDialog.editMux32mDialog(self, self.doc, self.treeCtrl, treeSelected)
		self.UpdateTreeCtrl()
		self.edited = 1

	def EditDemux32EntryCB(self, event):
		treeSelected = self.treeCtrl.GetSelection()
		docSelected = self.treeCtrl.GetPyData(treeSelected)
		editDemux32Dialog.editDemux32Dialog(self, self.doc, self.treeCtrl, treeSelected)
		self.UpdateTreeCtrl()
		self.edited = 1

	def AddME1000ExtensionCB(self, event):
		treeSubdeviceList = self.treeCtrl.GetSelection()
		docSubdeviceList = self.treeCtrl.GetPyData(treeSubdeviceList)
		docSubdeviceEntry = docSubdeviceList.getElementsByTagName("subdevice_entry")[0]

		docThirdSubdeviceEntry = docSubdeviceEntry.cloneNode(1)
		docThirdSubdeviceEntry.setAttribute("subdevice_number", "2")
		docSubdeviceList.appendChild(docThirdSubdeviceEntry)
		treeThirdSubdeviceEntry = self.treeCtrl.AppendItem(treeSubdeviceList, docThirdSubdeviceEntry.getAttribute("description") + " " + str("2"))
		self.treeCtrl.SetPyData(treeThirdSubdeviceEntry, docThirdSubdeviceEntry)
		self.BuildSubtree(treeThirdSubdeviceEntry, docThirdSubdeviceEntry)

		docFourthSubdeviceEntry = docSubdeviceEntry.cloneNode(1)
		docFourthSubdeviceEntry.setAttribute("subdevice_number", "3")
		docSubdeviceList.appendChild(docFourthSubdeviceEntry)
		treeFourthSubdeviceEntry = self.treeCtrl.AppendItem(treeSubdeviceList, docFourthSubdeviceEntry.getAttribute("description") + " " + str("3"))
		self.treeCtrl.SetPyData(treeFourthSubdeviceEntry, docFourthSubdeviceEntry)
		self.BuildSubtree(treeFourthSubdeviceEntry, docFourthSubdeviceEntry)

		self.UpdateTreeCtrl()
		self.edited = 1

	def DeleteME1000ExtensionCB(self, event):
		treeSubdeviceList = self.treeCtrl.GetSelection()
		docSubdeviceList = self.treeCtrl.GetPyData(treeSubdeviceList)

		for i in range(2):
			id, cookie = self.treeCtrl.GetFirstChild(treeSubdeviceList)
			while(id.IsOk()):
				docData = self.treeCtrl.GetPyData(id)
				if((docData.nodeName == "subdevice_entry") and
						((docData.getAttribute("subdevice_number") == "2") or
						(docData.getAttribute("subdevice_number") == "3"))):
					self.treeCtrl.Delete(id)
					docData = docData.parentNode.removeChild(docData)
					docData.unlink()
					break
				else:
					id, cookie = self.treeCtrl.GetNextChild(treeSubdeviceList, cookie)

		self.UpdateTreeCtrl()
		self.edited = 1

	def AddRemoteDeviceCB(self, event):
		treeDeviceList = self.treeCtrl.GetSelection()
		newRemoteDeviceDialog.newRemoteDeviceDialog(self, self.treeCtrl, treeDeviceList, self.doc, "Register a remote device", self.xpmRemoteDevice);
		self.UpdateTreeCtrl()
		self.edited = 1

	def RemoveSubdeviceLocks(self, deviceNo):
		docRoot = self.doc.documentElement
		docDeviceList = docRoot.getElementsByTagName("device_list")[0]
		docDevices = docDeviceList.getElementsByTagName("device_entry")

		for docDeviceEntry in docDevices:
			docSubdeviceList = docDeviceEntry.getElementsByTagName("subdevice_list")[0]
			docSubdevices = docSubdeviceList.getElementsByTagName("subdevice_entry")

			for docSubdeviceEntry in docSubdevices:
				if(docSubdeviceEntry.getAttribute("subdevice_lock") == "1"):
					if(int(docSubdeviceEntry.getAttribute("lock_device")) == deviceNo):
						docSubdeviceEntry.setAttribute("subdevice_lock", "0")

	def DevicePlugged(self, deviceNo):
		docRoot = self.doc.documentElement
		docDeviceList = docRoot.getElementsByTagName("device_list")[0]
		docDeviceEntry = docDeviceList.getElementsByTagName("device_entry")[deviceNo]

		if(docDeviceEntry.getAttribute("device_plugged") == str(meDriver.ME_PLUGGED_IN)):
			return True
		else:
			return False

	def DeviceLocked(self, deviceNo):
		docRoot = self.doc.documentElement
		docDeviceList = docRoot.getElementsByTagName("device_list")[0]
		docDeviceEntry = docDeviceList.getElementsByTagName("device_entry")[deviceNo]
		docSubdeviceList = docDeviceEntry.getElementsByTagName("subdevice_list")[0]
		docSubdevices = docSubdeviceList.getElementsByTagName("subdevice_entry")

		for docSubdeviceEntry in docSubdevices:
			if(docSubdeviceEntry.getAttribute("subdevice_lock") == "1") and (int(docSubdeviceEntry.getAttribute("lock_device")) != deviceNo):
				return True

		return False

	def RemoveDeviceEntry(self, deviceNo):
		docRoot = self.doc.documentElement
		docDeviceList = docRoot.getElementsByTagName("device_list")[0]
		docDeviceEntry = docDeviceList.getElementsByTagName("device_entry")[deviceNo]

		treeRoot = self.treeCtrl.GetRootItem()
		treeDeviceList = None
		treeDeviceEntry = None

		# Get list control id of device list
		id, cookie = self.treeCtrl.GetFirstChild(treeRoot)
		while(id.IsOk()):
			docData = self.treeCtrl.GetPyData(id)
			if docData.nodeName == "device_list":
				treeDeviceList = id
				break
			else:
				id, cookie = self.treeCtrl.GetNextChild(treeRoot, cookie)

		# Get list control id of device entry
		id, cookie = self.treeCtrl.GetFirstChild(treeDeviceList)
		while(id.IsOk()):
			docData = self.treeCtrl.GetPyData(id)
			if(docData.nodeName == "device_entry") and (int(docData.getAttribute("device_number")) == deviceNo):
				treeDeviceEntry = id
				break
			else:
				id, cookie = self.treeCtrl.GetNextChild(treeDeviceList, cookie)

		# Remove the entries from the control list and xml document
		self.treeCtrl.Delete(treeDeviceEntry)
		docDeviceEntry = docDeviceList.removeChild(docDeviceEntry)
		docDeviceEntry.unlink()

		# Update the device number in the control list and xml document
		i = 0
		id, cookie = self.treeCtrl.GetFirstChild(treeDeviceList)
		while id.IsOk():
			docData = self.treeCtrl.GetPyData(id)
			if docData.nodeName == "device_entry":
				docData.setAttribute("device_number", str(i))
				self.treeCtrl.SetItemText(id, docData.getAttribute("description") + " " + str(i))
				i = i + 1
			id, cookie = self.treeCtrl.GetNextChild(treeDeviceList, cookie)

	def DeleteDeviceEntryCB(self, event):
		#
		# Get the selected device
		#
		treeDeviceEntry = self.treeCtrl.GetSelection()
		docDeviceEntry = self.treeCtrl.GetPyData(treeDeviceEntry)
		deviceNo = int(docDeviceEntry.getAttribute("device_number"))

		if((not self.DevicePlugged(deviceNo)) or (int(docDeviceEntry.getAttribute("access")) == meDriver.ME_ACCESS_TYPE_REMOTE)):
			if(self.DeviceLocked(deviceNo)):
				dlg = wx.MessageDialog(self, "A subdevice is used by another device", "Delete device entry", wx.OK | wx.ICON_ERROR)
				dlg.ShowModal()
				dlg.Destroy()
				return

			# Confirm deleting
			dlg = wx.MessageDialog(self, "Do you really want to delete this entry?", "Delete device entry", wx.OK | wx.CANCEL | wx.ICON_QUESTION)
			ok = dlg.ShowModal()
			dlg.Destroy()

			if(ok == wx.ID_OK):
				self.edited = 1
				self.RemoveSubdeviceLocks(deviceNo)
				self.RemoveDeviceEntry(deviceNo)

			self.UpdateTreeCtrl()
			self.edited = 1
		else:
			dlg = wx.MessageDialog(
						self,
						"This device is still plugged in.\nRemove it from the system before deleting it!",
						"Delete device entry", wx.OK | wx.ICON_ERROR)
			dlg.ShowModal()
			dlg.Destroy()
			return

	def RemoveMux32mEntry(self, deviceNo, subdeviceNo):
		docRoot = self.doc.documentElement
		docDeviceList = docRoot.getElementsByTagName("device_list")[0]
		docDeviceEntry = docDeviceList.getElementsByTagName("device_entry")[deviceNo]
		docSubdeviceList = docDeviceEntry.getElementsByTagName("subdevice_list")[0]
		docSubdeviceEntry = docSubdeviceList.getElementsByTagName("subdevice_entry")[subdeviceNo]
		docMux32mEntry = docSubdeviceEntry.getElementsByTagName("mux32m")[0]
		docMux32mDioDeviceEntry = docMux32mEntry.getElementsByTagName("mux32m_dio_device")[0]
		docMux32mDioSubdeviceEntry = docMux32mEntry.getElementsByTagName("mux32m_dio_subdevice")[0]
		if(docMux32mEntry.getAttribute("timed") == "1"):
			docMux32mTimerDevice = docMux32mEntry.getElementsByTagName("mux32m_timer_device")[0]
			docMux32mTimerSubdevice = docMux32mEntry.getElementsByTagName("mux32m_timer_subdevice")[0]
		else:
			docMux32mTimerDevice = None
			docMux32mTimerSubdevice = None

		dioDeviceNo = int(string.strip(docMux32mDioDeviceEntry.childNodes[0].data))
		dioSubdeviceNo = int(string.strip(docMux32mDioSubdeviceEntry.childNodes[0].data))

		if(docMux32mTimerDevice):
			timerDeviceNo = int(string.strip(docMux32mTimerDevice.firstChild.data))
		else:
			timerDeviceNo = None

		if(docMux32mTimerSubdevice):
			timerSubdeviceNo = int(string.strip(docMux32mTimerSubdevice.firstChild.data))
		else:
			timerSubdeviceNo = None

		treeRoot = self.treeCtrl.GetRootItem()
		treeDeviceList = None
		treeDeviceEntry = None
		treeSubdeviceList = None
		treeSubdeviceEntry = None
		treeMux32mEntry = None

		# Get list control id of device list
		id, cookie = self.treeCtrl.GetFirstChild(treeRoot)
		while(id.IsOk()):
			docData = self.treeCtrl.GetPyData(id)
			if docData.nodeName == "device_list":
				treeDeviceList = id
				break
			else:
				id, cookie = self.treeCtrl.GetNextChild(treeRoot, cookie)

		# Get list control id of device entry
		id, cookie = self.treeCtrl.GetFirstChild(treeDeviceList)
		while(id.IsOk()):
			docData = self.treeCtrl.GetPyData(id)
			if(docData.nodeName == "device_entry") and (int(docData.getAttribute("device_number")) == deviceNo):
				treeDeviceEntry = id
				break
			else:
				id, cookie = self.treeCtrl.GetNextChild(treeDeviceList, cookie)

		# Get list control id of subdevice list
		id, cookie = self.treeCtrl.GetFirstChild(treeDeviceEntry)
		while(id.IsOk()):
			docData = self.treeCtrl.GetPyData(id)
			if docData.nodeName == "subdevice_list":
				treeSubdeviceList = id
				break
			else:
				id, cookie = self.treeCtrl.GetNextChild(treeDeviceEntry, cookie)

		# Get list control id of subdevice entry
		id, cookie = self.treeCtrl.GetFirstChild(treeSubdeviceList)
		while(id.IsOk()):
			docData = self.treeCtrl.GetPyData(id)
			if(docData.nodeName == "subdevice_entry") and (int(docData.getAttribute("subdevice_number")) == subdeviceNo):
				treeSubdeviceEntry = id
				break
			else:
				id, cookie = self.treeCtrl.GetNextChild(treeSubdeviceList, cookie)

		# Get list control id of mux32m entry
		id, cookie = self.treeCtrl.GetFirstChild(treeSubdeviceEntry)
		while(id.IsOk()):
			docData = self.treeCtrl.GetPyData(id)
			if(docData.nodeName == "mux32m"):
				treeMux32mEntry = id
				break
			else:
				id, cookie = self.treeCtrl.GetNextChild(treeSubdeviceEntry, cookie)

		# Mark the digital i/o subdevice as unlocked
		docDioDeviceEntry = docDeviceList.getElementsByTagName("device_entry")[dioDeviceNo]
		docDioSubdeviceList = docDioDeviceEntry.getElementsByTagName("subdevice_list")[0]
		docDioSubdeviceEntry = docDioSubdeviceList.getElementsByTagName("subdevice_entry")[dioSubdeviceNo]
		docDioSubdeviceEntry.setAttribute("subdevice_lock", "0")

		# Mark the timer subdevice as unlocked
		if(docMux32mTimerDevice):
			docTimerDeviceEntry = docDeviceList.getElementsByTagName("device_entry")[timerDeviceNo]
			docTimerSubdeviceList = docTimerDeviceEntry.getElementsByTagName("subdevice_list")[0]
			docTimerSubdeviceEntry = docTimerSubdeviceList.getElementsByTagName("subdevice_entry")[timerSubdeviceNo]
			docTimerSubdeviceEntry.setAttribute("subdevice_lock", "0")

		# Remove mux32m entry from control list and xml document
		self.treeCtrl.Delete(treeMux32mEntry)
		docSubdeviceEntry.setAttribute("subdevice_extension", str(meDriver.ME_EXTENSION_TYPE_NONE))
		docMux32mEntry = docSubdeviceEntry.removeChild(docMux32mEntry)
		docMux32mEntry.unlink()

	def DeleteMux32mEntryCB(self, event):
		# Get the device and subdevice number
		treeMux32mEntry = self.treeCtrl.GetSelection()
		docMux32mEntry = self.treeCtrl.GetPyData(treeMux32mEntry)
		docSubdeviceEntry = docMux32mEntry.parentNode
		docDeviceEntry = docSubdeviceEntry.parentNode.parentNode

		deviceNo = int(docDeviceEntry.getAttribute("device_number"))
		subdeviceNo = int(docSubdeviceEntry.getAttribute("subdevice_number"))

		dlg = wx.MessageDialog(self, "Do you really want to delete this entry?", "Delete ME-MUX32-M", wx.OK | wx.CANCEL | wx.ICON_QUESTION)
		ok = dlg.ShowModal()
		dlg.Destroy()
		if(ok == wx.ID_OK):
			self.edited = 1
			self.RemoveMux32mEntry(deviceNo, subdeviceNo)

			self.UpdateTreeCtrl()
			self.edited = 1

	def RemoveMux32sEntry(self, deviceNo, subdeviceNo, mux32sNo):
		docRoot = self.doc.documentElement
		docDeviceList = docRoot.getElementsByTagName("device_list")[0]
		docDeviceEntry = docDeviceList.getElementsByTagName("device_entry")[deviceNo]
		docSubdeviceList = docDeviceEntry.getElementsByTagName("subdevice_list")[0]
		docSubdeviceEntry = docSubdeviceList.getElementsByTagName("subdevice_entry")[subdeviceNo]
		docMux32mEntry = docSubdeviceEntry.getElementsByTagName("mux32m")[0]
		docMux32sList = docMux32mEntry.getElementsByTagName("mux32s_list")[0]
		docMux32sEntry = docMux32sList.getElementsByTagName("mux32s_entry")[mux32sNo]

		treeRoot = self.treeCtrl.GetRootItem()
		treeDeviceList = None
		treeDeviceEntry = None
		treeSubdeviceList = None
		treeSubdeviceEntry = None
		treeMux32mEntry = None
		treeMux32sList = None
		treeMux32sEntry = None

		# Get list control id of device list
		id, cookie = self.treeCtrl.GetFirstChild(treeRoot)
		while(id.IsOk()):
			docData = self.treeCtrl.GetPyData(id)
			if docData.nodeName == "device_list":
				treeDeviceList = id
				break
			else:
				id, cookie = self.treeCtrl.GetNextChild(treeRoot, cookie)

		# Get list control id of device entry
		id, cookie = self.treeCtrl.GetFirstChild(treeDeviceList)
		while(id.IsOk()):
			docData = self.treeCtrl.GetPyData(id)
			if(docData.nodeName == "device_entry") and (int(docData.getAttribute("device_number")) == deviceNo):
				treeDeviceEntry = id
				break
			else:
				id, cookie = self.treeCtrl.GetNextChild(treeDeviceList, cookie)

		# Get list control id of subdevice list
		id, cookie = self.treeCtrl.GetFirstChild(treeDeviceEntry)
		while(id.IsOk()):
			docData = self.treeCtrl.GetPyData(id)
			if docData.nodeName == "subdevice_list":
				treeSubdeviceList = id
				break
			else:
				id, cookie = self.treeCtrl.GetNextChild(treeDeviceEntry, cookie)

		# Get list control id of subdevice entry
		id, cookie = self.treeCtrl.GetFirstChild(treeSubdeviceList)
		while(id.IsOk()):
			docData = self.treeCtrl.GetPyData(id)
			if(docData.nodeName == "subdevice_entry") and (int(docData.getAttribute("subdevice_number")) == subdeviceNo):
				treeSubdeviceEntry = id
				break
			else:
				id, cookie = self.treeCtrl.GetNextChild(treeSubdeviceList, cookie)

		# Get list control id of mux32m entry
		id, cookie = self.treeCtrl.GetFirstChild(treeSubdeviceEntry)
		while(id.IsOk()):
			docData = self.treeCtrl.GetPyData(id)
			if(docData.nodeName == "mux32m"):
				treeMux32mEntry = id
				break
			else:
				id, cookie = self.treeCtrl.GetNextChild(treeSubdeviceEntry, cookie)

		# Get list control id of mux32s list
		id, cookie = self.treeCtrl.GetFirstChild(treeMux32mEntry)
		while(id.IsOk()):
			docData = self.treeCtrl.GetPyData(id)
			if docData.nodeName == "mux32s_list":
				treeMux32sList = id
				break
			else:
				id, cookie = self.treeCtrl.GetNextChild(treeMux32mEntry, cookie)

		# Get list control id of mux32s entry
		id, cookie = self.treeCtrl.GetFirstChild(treeMux32sList)
		while(id.IsOk()):
			docData = self.treeCtrl.GetPyData(id)
			if(docData.nodeName == "mux32s_entry") and (int(docData.getAttribute("mux32s_number")) == mux32sNo):
				treeMux32sEntry = id
				break
			else:
				id, cookie = self.treeCtrl.GetNextChild(treeMux32sList, cookie)

		# Remove mux32m entry from control list and xml document
		self.treeCtrl.Delete(treeMux32sEntry)
		docMux32sEntry = docMux32sList.removeChild(docMux32sEntry)
		docMux32sEntry.unlink()

		# Update tree control and xml document
		i = 0
		id, cookie = self.treeCtrl.GetFirstChild(treeMux32sList)
		while id.IsOk():
			docMux32sEntry = self.treeCtrl.GetPyData(id)
			docMux32sEntry.setAttribute("mux32s_number", str(i))
			self.treeCtrl.SetItemText(id, docMux32sEntry.getAttribute("description") + " " + str(i))
			i = i + 1
			id, cookie = self.treeCtrl.GetNextChild(treeMux32sList, cookie)

	def DeleteMux32sEntryCB(self, event):
		# Get the device and subdevice number
		treeMux32sEntry = self.treeCtrl.GetSelection()
		docMux32sEntry = self.treeCtrl.GetPyData(treeMux32sEntry)
		docMux32sList = docMux32sEntry.parentNode
		docMux32mEntry = docMux32sList.parentNode
		docSubdeviceEntry = docMux32mEntry.parentNode
		docSubdeviceList = docSubdeviceEntry.parentNode
		docDeviceEntry = docSubdeviceList.parentNode

		deviceNo = int(docDeviceEntry.getAttribute("device_number"))
		subdeviceNo = int(docSubdeviceEntry.getAttribute("subdevice_number"))
		mux32sNo = int(docMux32sEntry.getAttribute("mux32s_number"))

		dlg = wx.MessageDialog(self, "Do you really want to delete this entry?", "Delete ME-MUX32-S", wx.OK | wx.CANCEL | wx.ICON_QUESTION)
		ok = dlg.ShowModal()
		dlg.Destroy()
		if(ok == wx.ID_OK):
			self.edited = 1
			self.RemoveMux32sEntry(deviceNo, subdeviceNo, mux32sNo)

			self.UpdateTreeCtrl()
			self.edited = 1

	def RemoveDemux32Entry(self, deviceNo, subdeviceNo):
		docRoot = self.doc.documentElement
		docDeviceList = docRoot.getElementsByTagName("device_list")[0]
		docDeviceEntry = docDeviceList.getElementsByTagName("device_entry")[deviceNo]
		docSubdeviceList = docDeviceEntry.getElementsByTagName("subdevice_list")[0]
		docSubdeviceEntry = docSubdeviceList.getElementsByTagName("subdevice_entry")[subdeviceNo]
		docDemux32Entry = docSubdeviceEntry.getElementsByTagName("demux32")[0]
		docDemux32DioDeviceEntry = docDemux32Entry.getElementsByTagName("demux32_dio_device")[0]
		docDemux32DioSubdeviceEntry = docDemux32Entry.getElementsByTagName("demux32_dio_subdevice")[0]
		if(docDemux32Entry.getAttribute("timed") == "1"):
			docDemux32TimerDevice = docDemux32Entry.getElementsByTagName("demux32_timer_device")[0]
			docDemux32TimerSubdevice = docDemux32Entry.getElementsByTagName("demux32_timer_subdevice")[0]
		else:
			docDemux32TimerDevice = None
			docDemux32TimerSubdevice = None

		dioDeviceNo = int(string.strip(docDemux32DioDeviceEntry.childNodes[0].data))
		dioSubdeviceNo = int(string.strip(docDemux32DioSubdeviceEntry.childNodes[0].data))

		if(docDemux32TimerDevice):
			timerDeviceNo = int(string.strip(docDemux32TimerDevice.firstChild.data))
		else:
			timerDeviceNo = None

		if(docDemux32TimerSubdevice):
			timerSubdeviceNo = int(string.strip(docDemux32TimerSubdevice.firstChild.data))
		else:
			timerSubdeviceNo = None

		treeRoot = self.treeCtrl.GetRootItem()
		treeDeviceList = None
		treeDeviceEntry = None
		treeSubdeviceList = None
		treeSubdeviceEntry = None
		treeDemux32Entry = None

		# Get list control id of device list
		id, cookie = self.treeCtrl.GetFirstChild(treeRoot)
		while(id.IsOk()):
			docData = self.treeCtrl.GetPyData(id)
			if docData.nodeName == "device_list":
				treeDeviceList = id
				break
			else:
				id, cookie = self.treeCtrl.GetNextChild(treeRoot, cookie)

		# Get list control id of device entry
		id, cookie = self.treeCtrl.GetFirstChild(treeDeviceList)
		while(id.IsOk()):
			docData = self.treeCtrl.GetPyData(id)
			if(docData.nodeName == "device_entry") and (int(docData.getAttribute("device_number")) == deviceNo):
				treeDeviceEntry = id
				break
			else:
				id, cookie = self.treeCtrl.GetNextChild(treeDeviceList, cookie)

		# Get list control id of subdevice list
		id, cookie = self.treeCtrl.GetFirstChild(treeDeviceEntry)
		while(id.IsOk()):
			docData = self.treeCtrl.GetPyData(id)
			if docData.nodeName == "subdevice_list":
				treeSubdeviceList = id
				break
			else:
				id, cookie = self.treeCtrl.GetNextChild(treeDeviceEntry, cookie)

		# Get list control id of subdevice entry
		id, cookie = self.treeCtrl.GetFirstChild(treeSubdeviceList)
		while(id.IsOk()):
			docData = self.treeCtrl.GetPyData(id)
			if(docData.nodeName == "subdevice_entry") and (int(docData.getAttribute("subdevice_number")) == subdeviceNo):
				treeSubdeviceEntry = id
				break
			else:
				id, cookie = self.treeCtrl.GetNextChild(treeSubdeviceList, cookie)

		# Get list control id of demux32 entry
		id, cookie = self.treeCtrl.GetFirstChild(treeSubdeviceEntry)
		while(id.IsOk()):
			docData = self.treeCtrl.GetPyData(id)
			if(docData.nodeName == "demux32"):
				treeDemux32Entry = id
				break
			else:
				id, cookie = self.treeCtrl.GetNextChild(treeSubdeviceEntry, cookie)

		# Mark the digital i/o subdevice as unlocked
		docDioDeviceEntry = docDeviceList.getElementsByTagName("device_entry")[dioDeviceNo]
		docDioSubdeviceList = docDioDeviceEntry.getElementsByTagName("subdevice_list")[0]
		docDioSubdeviceEntry = docDioSubdeviceList.getElementsByTagName("subdevice_entry")[dioSubdeviceNo]
		docDioSubdeviceEntry.setAttribute("subdevice_lock", "0")

		# Mark the timer subdevice as unlocked
		if(docDemux32TimerDevice):
			docTimerDeviceEntry = docDeviceList.getElementsByTagName("device_entry")[timerDeviceNo]
			docTimerSubdeviceList = docTimerDeviceEntry.getElementsByTagName("subdevice_list")[0]
			docTimerSubdeviceEntry = docTimerSubdeviceList.getElementsByTagName("subdevice_entry")[timerSubdeviceNo]
			docTimerSubdeviceEntry.setAttribute("subdevice_lock", "0")

		# Remove demux32 entry from control list and xml document
		self.treeCtrl.Delete(treeDemux32Entry)
		docSubdeviceEntry.setAttribute("subdevice_extension", str(meDriver.ME_EXTENSION_TYPE_NONE))
		docDemux32Entry = docSubdeviceEntry.removeChild(docDemux32Entry)
		docDemux32Entry.unlink()

	def DeleteDemux32EntryCB(self, event):
		# Get the device and subdevice number
		treeDemux32Entry = self.treeCtrl.GetSelection()
		docDemux32Entry = self.treeCtrl.GetPyData(treeDemux32Entry)
		docSubdeviceEntry = docDemux32Entry.parentNode
		docDeviceEntry = docSubdeviceEntry.parentNode.parentNode

		deviceNo = int(docDeviceEntry.getAttribute("device_number"))
		subdeviceNo = int(docSubdeviceEntry.getAttribute("subdevice_number"))

		dlg = wx.MessageDialog(self, "Do you really want to delete this entry?", "Delete entry", wx.OK | wx.CANCEL | wx.ICON_QUESTION)
		ok = dlg.ShowModal()
		dlg.Destroy()
		if(ok == wx.ID_OK):
			self.edited = 1
			self.RemoveDemux32Entry(deviceNo, subdeviceNo)

			self.UpdateTreeCtrl()
			self.edited = 1

	def DownCB(self, event):
		treeSelected = self.treeCtrl.GetSelection()
		docSelected = self.treeCtrl.GetPyData(treeSelected)
		if(docSelected.nodeName == "device_entry"):
			treeNext = self.treeCtrl.GetNextSibling(treeSelected)
			if(treeNext.IsOk()):
				docNext = self.treeCtrl.GetPyData(treeNext)
				numSelected = docSelected.getAttribute("device_number")
				numNext = docNext.getAttribute("device_number")
				self.treeCtrl.SetItemText(treeSelected, docSelected.getAttribute("description") + " " + numNext)
				self.treeCtrl.SetItemText(treeNext, docNext.getAttribute("description") + " " + numSelected)
				docSelected.setAttribute("device_number", numNext)
				docNext.setAttribute("device_number", numSelected)
				docParent = docSelected.parentNode
				docParent.removeChild(docNext)
				docParent.insertBefore(docNext, docSelected)
				self.treeCtrl.SortChildren(self.treeCtrl.GetItemParent(treeSelected))

				self.ExchangeLinks(docNext.parentNode, numSelected, numNext)
				self.UpdateTreeCtrl()
				self.edited = 1
		else:
			dlg = wx.MessageDialog(self, "Function not supported for the selected entry!", "Error", wx.OK | wx.ICON_ERROR)
			dlg.ShowModal()
			dlg.Destroy()
			return

	def UpCB(self, event):
		treeSelected = self.treeCtrl.GetSelection()
		docSelected = self.treeCtrl.GetPyData(treeSelected)
		if(docSelected.nodeName == "device_entry"):
			treePrevious = self.treeCtrl.GetPrevSibling(treeSelected)
			if(treePrevious.IsOk()):
				docPrevious = self.treeCtrl.GetPyData(treePrevious)
				numSelected = docSelected.getAttribute("device_number")
				numPrevious = docPrevious.getAttribute("device_number")
				self.treeCtrl.SetItemText(treeSelected, docSelected.getAttribute("description") + " " + numPrevious)
				self.treeCtrl.SetItemText(treePrevious, docPrevious.getAttribute("description") + " " + numSelected)
				docSelected.setAttribute("device_number", numPrevious)
				docPrevious.setAttribute("device_number", numSelected)
				docParent = docSelected.parentNode
				docParent.removeChild(docSelected)
				docParent.insertBefore(docSelected, docPrevious)
				self.treeCtrl.SortChildren(self.treeCtrl.GetItemParent(treeSelected))

				self.ExchangeLinks(docPrevious.parentNode, numSelected, numPrevious)
				self.UpdateTreeCtrl()
				self.edited = 1
		else:
			dlg = wx.MessageDialog(self, "Function not supported for the selected entry!", "Error", wx.OK | wx.ICON_ERROR)
			dlg.ShowModal()
			dlg.Destroy()
			return

	def ExchangeLinks(self, docDeviceList, numDevice1, numDevice2):
		for docDeviceEntry in docDeviceList.getElementsByTagName("device_entry"):
			docSubdeviceList = docDeviceEntry.getElementsByTagName("subdevice_list")[0]
			for docSubdeviceEntry in docSubdeviceList.getElementsByTagName("subdevice_entry"):
				if docSubdeviceEntry.getAttribute("subdevice_lock") == "1":
					if docSubdeviceEntry.getAttribute("lock_device") == numDevice2:
						docSubdeviceEntry.setAttribute("lock_device", numDevice1)
					elif docSubdeviceEntry.getAttribute("lock_device") == numDevice1:
						docSubdeviceEntry.setAttribute("lock_device", numDevice2)
				else:
					for docMux32m in docSubdeviceEntry.getElementsByTagName("mux32m"):
						for docMux32mDioDevice in docMux32m.getElementsByTagName("mux32m_dio_device"):
							if string.strip(docMux32mDioDevice.firstChild.data) == numDevice2:
								docMux32mDioDevice.firstChild.data = numDevice1
							elif string.strip(docMux32mDioDevice.firstChild.data) == numDevice1:
								docMux32mDioDevice.firstChild.data = numDevice2
					for docDemux32 in docSubdeviceEntry.getElementsByTagName("demux32"):
						for docDemux32DioDevice in docDemux32.getElementsByTagName("demux32_dio_device"):
							if string.strip(docDemux32DioDevice.firstChild.data) == numDevice2:
								docDemux32DioDevice.firstChild.data = numDevice1
							elif string.strip(docDemux32DioDevice.firstChild.data) == numDevice1:
								docDemux32DioDevice.firstChild.data = numDevice2

	def UpdateTreeCtrl(self):
		treeRoot = self.treeCtrl.GetRootItem()
		self.UpdateSubtree(treeRoot)

	def UpdateSubtree(self, treeRoot):
		docRoot = self.treeCtrl.GetPyData(treeRoot)
 
		if docRoot.nodeType == Node.ELEMENT_NODE:
			if docRoot.nodeName == "date":
				self.treeCtrl.SetItemText(treeRoot, docRoot.getAttribute("description"), 0)
				self.treeCtrl.SetItemText(treeRoot, self.DefineToString(docRoot.firstChild), 1)
				self.treeCtrl.SetItemImage(treeRoot, self.treeImageCalender, which = wx.TreeItemIcon_Normal)
				self.treeCtrl.SetItemImage(treeRoot, self.treeImageCalender, which = wx.TreeItemIcon_Expanded)
			elif docRoot.nodeName == "device_list":
				self.treeCtrl.SetItemText(treeRoot, docRoot.getAttribute("description"), 0)
				self.treeCtrl.SetItemImage(treeRoot, self.treeImageDevices, which = wx.TreeItemIcon_Normal)
				self.treeCtrl.SetItemImage(treeRoot, self.treeImageDevices, which = wx.TreeItemIcon_Expanded)
			elif docRoot.nodeName == "tcpip":
				self.treeCtrl.SetItemText(treeRoot, docRoot.getAttribute("description"), 0)
				self.treeCtrl.SetItemImage(treeRoot, self.treeImageRemoteDeviceInfo, which = wx.TreeItemIcon_Normal)
				self.treeCtrl.SetItemImage(treeRoot, self.treeImageRemoteDeviceInfo, which = wx.TreeItemIcon_Expanded)
			elif docRoot.nodeName == "device_entry":
				self.treeCtrl.SetItemText(treeRoot, docRoot.getAttribute("description") + " " + docRoot.getAttribute("device_number"), 0)
				if(int(docRoot.getAttribute("access")) == meDriver.ME_ACCESS_TYPE_LOCAL):
					if(int(docRoot.getAttribute("bus")) == meDriver.ME_BUS_TYPE_PCI):
						if(int(docRoot.getAttribute("device_plugged")) == meDriver.ME_PLUGGED_IN):
							self.treeCtrl.SetItemImage(treeRoot, self.treeImageDevice, which = wx.TreeItemIcon_Normal)
							self.treeCtrl.SetItemImage(treeRoot, self.treeImageDevice, which = wx.TreeItemIcon_Expanded)
						else:
							self.treeCtrl.SetItemImage(treeRoot, self.treeImageDeviceUnplugged, which = wx.TreeItemIcon_Normal)
							self.treeCtrl.SetItemImage(treeRoot, self.treeImageDeviceUnplugged, which = wx.TreeItemIcon_Expanded)
					elif(int(docRoot.getAttribute("bus")) == meDriver.ME_BUS_TYPE_PCI):
						if(int(docRoot.getAttribute("device_plugged")) == meDriver.ME_PLUGGED_IN):
							self.treeCtrl.SetItemImage(treeRoot, self.treeImageUsbDevice, which = wx.TreeItemIcon_Normal)
							self.treeCtrl.SetItemImage(treeRoot, self.treeImageUsbDevice, which = wx.TreeItemIcon_Expanded)
						else:
							self.treeCtrl.SetItemImage(treeRoot, self.treeImageUsbDeviceUnplugged, which = wx.TreeItemIcon_Normal)
							self.treeCtrl.SetItemImage(treeRoot, self.treeImageUsbDeviceUnplugged, which = wx.TreeItemIcon_Expanded)
					else:
						if(int(docRoot.getAttribute("device_plugged")) == meDriver.ME_PLUGGED_IN):
							self.treeCtrl.SetItemImage(treeRoot, self.treeImagePNPdevice, which = wx.TreeItemIcon_Normal)
							self.treeCtrl.SetItemImage(treeRoot, self.treeImagePNPdevice, which = wx.TreeItemIcon_Expanded)
						else:
							self.treeCtrl.SetItemImage(treeRoot, self.treeImagePNPdeviceUnplugged, which = wx.TreeItemIcon_Normal)
							self.treeCtrl.SetItemImage(treeRoot, self.treeImagePNPdeviceUnplugged, which = wx.TreeItemIcon_Expanded)
				else:
					if(int(docRoot.getAttribute("device_plugged")) == meDriver.ME_PLUGGED_IN):
						self.treeCtrl.SetItemImage(treeRoot, self.treeImageRemoteDevice, which = wx.TreeItemIcon_Normal)
						self.treeCtrl.SetItemImage(treeRoot, self.treeImageRemoteDevice, which = wx.TreeItemIcon_Expanded)
					else:
						self.treeCtrl.SetItemImage(treeRoot, self.treeImageRemoteDeviceUnplugged, which = wx.TreeItemIcon_Normal)
						self.treeCtrl.SetItemImage(treeRoot, self.treeImageRemoteDeviceUnplugged, which = wx.TreeItemIcon_Expanded)
			elif docRoot.nodeName == "device_info":
				self.treeCtrl.SetItemText(treeRoot, docRoot.getAttribute("description"), 0)
				self.treeCtrl.SetItemImage(treeRoot, self.treeImageDeviceInfo, which = wx.TreeItemIcon_Normal)
				self.treeCtrl.SetItemImage(treeRoot, self.treeImageDeviceInfo, which = wx.TreeItemIcon_Expanded)
			elif docRoot.nodeName == "subdevice_list":
				self.treeCtrl.SetItemText(treeRoot, docRoot.getAttribute("description"), 0)
				self.treeCtrl.SetItemImage(treeRoot, self.treeImageSubdevices, which = wx.TreeItemIcon_Normal)
				self.treeCtrl.SetItemImage(treeRoot, self.treeImageSubdevices, which = wx.TreeItemIcon_Expanded)
			elif docRoot.nodeName == "subdevice_entry":
				self.treeCtrl.SetItemText(treeRoot, docRoot.getAttribute("description") + " " + docRoot.getAttribute("subdevice_number"), 0)
				if(int(docRoot.getAttribute("subdevice_type")) == meDriver.ME_TYPE_AO):
					if(int(docRoot.getAttribute("subdevice_lock"))):
						self.treeCtrl.SetItemImage(treeRoot, self.treeImageSubdeviceLockedDA, which = wx.TreeItemIcon_Normal)
						self.treeCtrl.SetItemImage(treeRoot, self.treeImageSubdeviceLockedDA, which = wx.TreeItemIcon_Expanded)
					else:
						self.treeCtrl.SetItemImage(treeRoot, self.treeImageSubdeviceDA, which = wx.TreeItemIcon_Normal)
						self.treeCtrl.SetItemImage(treeRoot, self.treeImageSubdeviceDA, which = wx.TreeItemIcon_Expanded)
				elif(int(docRoot.getAttribute("subdevice_type")) == meDriver.ME_TYPE_AI):
					if(int(docRoot.getAttribute("subdevice_lock"))):
						self.treeCtrl.SetItemImage(treeRoot, self.treeImageSubdeviceLockedAD, which = wx.TreeItemIcon_Normal)
						self.treeCtrl.SetItemImage(treeRoot, self.treeImageSubdeviceLockedAD, which = wx.TreeItemIcon_Expanded)
					else:
						self.treeCtrl.SetItemImage(treeRoot, self.treeImageSubdeviceAD, which = wx.TreeItemIcon_Normal)
						self.treeCtrl.SetItemImage(treeRoot, self.treeImageSubdeviceAD, which = wx.TreeItemIcon_Expanded)
				elif(int(docRoot.getAttribute("subdevice_type")) == meDriver.ME_TYPE_DIO):
					if(int(docRoot.getAttribute("subdevice_lock"))):
						self.treeCtrl.SetItemImage(treeRoot, self.treeImageSubdeviceLockedDIO, which = wx.TreeItemIcon_Normal)
						self.treeCtrl.SetItemImage(treeRoot, self.treeImageSubdeviceLockedDIO, which = wx.TreeItemIcon_Expanded)
					else:
						self.treeCtrl.SetItemImage(treeRoot, self.treeImageSubdeviceDIO, which = wx.TreeItemIcon_Normal)
						self.treeCtrl.SetItemImage(treeRoot, self.treeImageSubdeviceDIO, which = wx.TreeItemIcon_Expanded)
				elif(int(docRoot.getAttribute("subdevice_type")) == meDriver.ME_TYPE_DO):
					if(int(docRoot.getAttribute("subdevice_lock"))):
						self.treeCtrl.SetItemImage(treeRoot, self.treeImageSubdeviceLockedDO, which = wx.TreeItemIcon_Normal)
						self.treeCtrl.SetItemImage(treeRoot, self.treeImageSubdeviceLockedDO, which = wx.TreeItemIcon_Expanded)
					else:
						self.treeCtrl.SetItemImage(treeRoot, self.treeImageSubdeviceDO, which = wx.TreeItemIcon_Normal)
						self.treeCtrl.SetItemImage(treeRoot, self.treeImageSubdeviceDO, which = wx.TreeItemIcon_Expanded)
				elif(int(docRoot.getAttribute("subdevice_type")) == meDriver.ME_TYPE_DI):
					if(int(docRoot.getAttribute("subdevice_lock"))):
						self.treeCtrl.SetItemImage(treeRoot, self.treeImageSubdeviceLockedDI, which = wx.TreeItemIcon_Normal)
						self.treeCtrl.SetItemImage(treeRoot, self.treeImageSubdeviceLockedDI, which = wx.TreeItemIcon_Expanded)
					else:
						self.treeCtrl.SetItemImage(treeRoot, self.treeImageSubdeviceDI, which = wx.TreeItemIcon_Normal)
						self.treeCtrl.SetItemImage(treeRoot, self.treeImageSubdeviceDI, which = wx.TreeItemIcon_Expanded)
				elif(int(docRoot.getAttribute("subdevice_type")) == meDriver.ME_TYPE_CTR):
					if(int(docRoot.getAttribute("subdevice_lock"))):
						self.treeCtrl.SetItemImage(treeRoot, self.treeImageSubdeviceLockedCTR, which = wx.TreeItemIcon_Normal)
						self.treeCtrl.SetItemImage(treeRoot, self.treeImageSubdeviceLockedCTR, which = wx.TreeItemIcon_Expanded)
					else:
						self.treeCtrl.SetItemImage(treeRoot, self.treeImageSubdeviceCTR, which = wx.TreeItemIcon_Normal)
						self.treeCtrl.SetItemImage(treeRoot, self.treeImageSubdeviceCTR, which = wx.TreeItemIcon_Expanded)
				elif(int(docRoot.getAttribute("subdevice_type")) == meDriver.ME_TYPE_EXT_IRQ):
					if(int(docRoot.getAttribute("subdevice_lock"))):
						self.treeCtrl.SetItemImage(treeRoot, self.treeImageSubdeviceLockedIRQ, which = wx.TreeItemIcon_Normal)
						self.treeCtrl.SetItemImage(treeRoot, self.treeImageSubdeviceLockedIRQ, which = wx.TreeItemIcon_Expanded)
					else:
						self.treeCtrl.SetItemImage(treeRoot, self.treeImageSubdeviceIRQ, which = wx.TreeItemIcon_Normal)
						self.treeCtrl.SetItemImage(treeRoot, self.treeImageSubdeviceIRQ, which = wx.TreeItemIcon_Expanded)
				elif(int(docRoot.getAttribute("subdevice_type")) == meDriver.ME_TYPE_FREQ_I):
					if(int(docRoot.getAttribute("subdevice_lock"))):
						self.treeCtrl.SetItemImage(treeRoot, self.treeImageSubdeviceLockedCTR, which = wx.TreeItemIcon_Normal)
						self.treeCtrl.SetItemImage(treeRoot, self.treeImageSubdeviceLockedCTR, which = wx.TreeItemIcon_Expanded)
					else:
						self.treeCtrl.SetItemImage(treeRoot, self.treeImageSubdeviceCTR, which = wx.TreeItemIcon_Normal)
						self.treeCtrl.SetItemImage(treeRoot, self.treeImageSubdeviceCTR, which = wx.TreeItemIcon_Expanded)
				elif(int(docRoot.getAttribute("subdevice_type")) == meDriver.ME_TYPE_FREQ_O):
					if(int(docRoot.getAttribute("subdevice_lock"))):
						self.treeCtrl.SetItemImage(treeRoot, self.treeImageSubdeviceLockedCTR, which = wx.TreeItemIcon_Normal)
						self.treeCtrl.SetItemImage(treeRoot, self.treeImageSubdeviceLockedCTR, which = wx.TreeItemIcon_Expanded)
					else:
						self.treeCtrl.SetItemImage(treeRoot, self.treeImageSubdeviceCTR, which = wx.TreeItemIcon_Normal)
						self.treeCtrl.SetItemImage(treeRoot, self.treeImageSubdeviceCTR, which = wx.TreeItemIcon_Expanded)
			elif docRoot.nodeName == "subdevice_info":
				self.treeCtrl.SetItemText(treeRoot, docRoot.getAttribute("description"), 0)
				self.treeCtrl.SetItemImage(treeRoot, self.treeImageSubdeviceInfo, which = wx.TreeItemIcon_Normal)
				self.treeCtrl.SetItemImage(treeRoot, self.treeImageSubdeviceInfo, which = wx.TreeItemIcon_Expanded)
			elif docRoot.nodeName == "range_list":
				self.treeCtrl.SetItemText(treeRoot, docRoot.getAttribute("description"), 0)
				self.treeCtrl.SetItemImage(treeRoot, self.treeImageRanges, which = wx.TreeItemIcon_Normal)
				self.treeCtrl.SetItemImage(treeRoot, self.treeImageRanges, which = wx.TreeItemIcon_Expanded)
			elif docRoot.nodeName == "range_entry":
				self.treeCtrl.SetItemText(treeRoot, docRoot.getAttribute("description") + " " + docRoot.getAttribute("range_number"), 0)
				self.treeCtrl.SetItemImage(treeRoot, self.treeImageRange, which = wx.TreeItemIcon_Normal)
				self.treeCtrl.SetItemImage(treeRoot, self.treeImageRange, which = wx.TreeItemIcon_Expanded)
			elif docRoot.nodeName == "mux32m":
				self.treeCtrl.SetItemText(treeRoot, docRoot.getAttribute("description"), 0)
				self.treeCtrl.SetItemImage(treeRoot, self.treeImageMultisig, which = wx.TreeItemIcon_Normal)
				self.treeCtrl.SetItemImage(treeRoot, self.treeImageMultisig, which = wx.TreeItemIcon_Expanded)
			elif docRoot.nodeName == "mux32s_list":
				self.treeCtrl.SetItemText(treeRoot, docRoot.getAttribute("description"), 0)
				self.treeCtrl.SetItemImage(treeRoot, self.treeImageMultisigs, which = wx.TreeItemIcon_Normal)
				self.treeCtrl.SetItemImage(treeRoot, self.treeImageMultisigs, which = wx.TreeItemIcon_Expanded)
			elif docRoot.nodeName == "mux32s_entry":
				self.treeCtrl.SetItemText(treeRoot, docRoot.getAttribute("description") + " " + docRoot.getAttribute("mux32s_number"), 0)
				self.treeCtrl.SetItemImage(treeRoot, self.treeImageMultisig, which = wx.TreeItemIcon_Normal)
				self.treeCtrl.SetItemImage(treeRoot, self.treeImageMultisig, which = wx.TreeItemIcon_Expanded)
			elif docRoot.nodeName == "demux32":
				self.treeCtrl.SetItemText(treeRoot, docRoot.getAttribute("description"), 0)
				self.treeCtrl.SetItemImage(treeRoot, self.treeImageMultisig, which = wx.TreeItemIcon_Normal)
				self.treeCtrl.SetItemImage(treeRoot, self.treeImageMultisig, which = wx.TreeItemIcon_Expanded)
			elif((docRoot.nodeName == "remote_host") or
				(docRoot.nodeName == "remote_device_number") or
				(docRoot.nodeName == "device_name") or
				(docRoot.nodeName == "device_description") or
				(docRoot.nodeName == "vendor_id") or
				(docRoot.nodeName == "device_id") or
				(docRoot.nodeName == "serial_no") or
				(docRoot.nodeName == "pci_bus_no") or
				(docRoot.nodeName == "pci_dev_no") or
				(docRoot.nodeName == "pci_func_no") or
				(docRoot.nodeName == "usb_root_hub_no") or
				(docRoot.nodeName == "subdevice_type") or
				(docRoot.nodeName == "subdevice_sub_type") or
				(docRoot.nodeName == "subdevice_number_channels") or
				(docRoot.nodeName == "range_unit") or
				(docRoot.nodeName == "range_min") or
				(docRoot.nodeName == "range_max") or
				(docRoot.nodeName == "range_max_data") or
				(docRoot.nodeName == "mux32m_ai_channel") or
				(docRoot.nodeName == "mux32m_dio_device") or
				(docRoot.nodeName == "mux32m_dio_subdevice") or
				(docRoot.nodeName == "mux32m_timer_device") or
				(docRoot.nodeName == "mux32m_timer_subdevice") or
				(docRoot.nodeName == "demux32_ao_channel") or
				(docRoot.nodeName == "demux32_dio_device") or
				(docRoot.nodeName == "demux32_dio_subdevice") or
				(docRoot.nodeName == "demux32_timer_device") or
				(docRoot.nodeName == "demux32_timer_subdevice")):
				self.treeCtrl.SetItemText(treeRoot, docRoot.getAttribute("description"), 0)
				self.treeCtrl.SetItemText(treeRoot, self.DefineToString(docRoot.firstChild), 1)
				self.treeCtrl.SetItemImage(treeRoot, self.treeImageData, which = wx.TreeItemIcon_Normal)
				self.treeCtrl.SetItemImage(treeRoot, self.treeImageData, which = wx.TreeItemIcon_Expanded)
			else:
				self.treeCtrl.SetItemText(treeRoot, docRoot.getAttribute("description"), 0)

			treeSubtree, cookie = self.treeCtrl.GetFirstChild(treeRoot)
			while(treeSubtree.IsOk()):
				self.UpdateSubtree(treeSubtree)
				treeSubtree, cookie = self.treeCtrl.GetNextChild(treeRoot, cookie)

	def NewMux32mEntryCB(self, event):
		treeSelected = self.treeCtrl.GetSelection()

		newMux32mDialog.newMux32mDialog(self, self.doc, self.treeCtrl, treeSelected)
		self.UpdateTreeCtrl()
		self.edited = 1

	def NewDemux32EntryCB(self, event):
		treeSelected = self.treeCtrl.GetSelection()

		newDemux32Dialog.newDemux32Dialog(self, self.doc, self.treeCtrl, treeSelected)
		self.UpdateTreeCtrl()
		self.edited = 1

	def NewMux32sEntryCB(self, event):
		treeMux32sList = self.treeCtrl.GetSelection()
		docMux32sList = self.treeCtrl.GetPyData(treeMux32sList)

		if(self.treeCtrl.GetChildrenCount(treeMux32sList, recursively = 0) >= 7):
			dlg = wx.MessageDialog(self, 'There are already 7 ME-MUX32-S present!', "Error", wx.OK | wx.ICON_ERROR)
			dlg.ShowModal()
			dlg.Destroy()
			self.Destroy()
			return

		# Append slave mux device to list
		docMux32s = self.doc.createElement("mux32s_entry")
		docMux32s.setAttribute("description", "ME-MUX32-S")
		docMux32s.setAttribute("mux32s_number", str(self.treeCtrl.GetChildrenCount(treeMux32sList, recursively = 0)))
		docMux32sList.appendChild(docMux32s)
		treeMux32s = self.treeCtrl.AppendItem(treeMux32sList, docMux32s.getAttribute("description") + " " + docMux32s.getAttribute("mux32s_number"))
		self.treeCtrl.SetPyData(treeMux32s, docMux32s)

		self.UpdateTreeCtrl()
		self.edited = 1
