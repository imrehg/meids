# Copyright (2004) Meilhaus Electronic GmbH.
# All rights reserved.

import wx
from wx.wizard import *
from xml.dom import *
from xml.dom.minidom import *
import string

import meDriver

class newRemoteDeviceDialog(Wizard):
	"""Configuration dialog to add a remote device."""
	def __init__(self, parent, treeCtrl, treeDeviceList, doc, title = "Register a remote device", bitmap = wx.NullBitmap):
		self.id = wx.NewId()
		self.treeCtrl = treeCtrl
		self.treeDeviceList = treeDeviceList
		self.doc = doc
		Wizard.__init__(self, parent, self.id, title, bitmap);

		self.page1 = WizardPageSimple(self)
		sizer1 = wx.BoxSizer(wx.VERTICAL)
		hostTxt = wx.StaticText(self.page1, -1, "Remote host (IP address / DNS name):")
		self.hostTxtCtrl = wx.TextCtrl(self.page1, -1, size = wx.Size(250, 30))
		sizer1.Add(hostTxt)
		sizer1.Add(self.hostTxtCtrl)
		self.page1.SetSizer(sizer1)

		self.page2 = WizardPageSimple(self)
		sizer2 = wx.BoxSizer(wx.VERTICAL)
		listTxt = wx.StaticText(self.page2, -1, "Please select device to add")
		self.listBox = wx.ListBox(self.page2, -1, size = wx.DefaultSize, style = wx.LB_SINGLE)
		sizer2.Add(listTxt)
		sizer2.Add(self.listBox)
		self.page2.SetSizer(sizer2)

		WizardPageSimple_Chain(self.page1, self.page2)

		areaSizer = self.GetPageAreaSizer()
		areaSizer.Add(self.page1)
		areaSizer.Add(self.page2)

		EVT_WIZARD_PAGE_CHANGING(self, self.id, self.WizardPageChangingCB)

		self.RunWizard(self.page1)
		self.Destroy()

	def WizardPageChangingCB(self, event):
		dir = event.GetDirection()
		page = event.GetPage()

		try:
			if(dir):
				if(page == self.page1):
					host = self.hostTxtCtrl.GetValue()
					devices = meDriver.meRQueryNumberDevices(host)
					list = []
					for i in range(devices):
						name = meDriver.meRQueryNameDevice(host, i)
						serno = meDriver.meRQueryInfoDevice(host, i)[2]
						list.append("%s (%08X)" % (name, serno))
					self.listBox.Clear()
					self.listBox.InsertItems(list, 0)
				elif(page == self.page2):
					host = self.hostTxtCtrl.GetValue()
					selections = self.listBox.GetSelections()
					if(not len(selections)):
						return

					i = selections[0]

					info = meDriver.meRQueryInfoDevice(host, i)

					docDeviceList = self.treeCtrl.GetPyData(self.treeDeviceList)

					device_number = len(docDeviceList.getElementsByTagName("device_entry"))

					docDeviceEntry = self.doc.createElement("device_entry")
					docDeviceEntry.setAttribute("description", "Device")
					docDeviceEntry.setAttribute("device_number", str(device_number))
					docDeviceEntry.setAttribute("device_plugged", str(info[7]))
					docDeviceEntry.setAttribute("access", str(meDriver.ME_ACCESS_TYPE_REMOTE))
					docDeviceEntry.setAttribute("bus", str(meDriver.ME_BUS_TYPE_INVALID))
					docDeviceList.appendChild(docDeviceEntry)

					docTcpip = self.doc.createElement("tcpip")
					docTcpip.setAttribute("description", "Network information")
					docDeviceEntry.appendChild(docTcpip)

					docRemoteHost = self.doc.createElement("remote_host")
					docRemoteHost.setAttribute("description", "Hostname")
					docRemoteHost.appendChild(self.doc.createTextNode(host))
					docTcpip.appendChild(docRemoteHost)

					docRemoteDeviceNumber = self.doc.createElement("remote_device_number")
					docRemoteDeviceNumber.setAttribute("description", "Remote device number")
					docRemoteDeviceNumber.appendChild(self.doc.createTextNode(str(i)))
					docTcpip.appendChild(docRemoteDeviceNumber)

					docDeviceInfo = self.doc.createElement("device_info")
					docDeviceInfo.setAttribute("description", "Device info")
					docDeviceInfo.setAttribute("bus", str(info[3]))
					docDeviceEntry.appendChild(docDeviceInfo)

					docDeviceName = self.doc.createElement("device_name")
					docDeviceName.setAttribute("description", "Device name")
					docDeviceName.appendChild(self.doc.createTextNode(str(meDriver.meRQueryNameDevice(host, i))))
					docDeviceInfo.appendChild(docDeviceName)

					docDeviceDescription = self.doc.createElement("device_description")
					docDeviceDescription.setAttribute("description", "Device description")
					docDeviceDescription.appendChild(self.doc.createTextNode(str(meDriver.meRQueryDescriptionDevice(host, i))))
					docDeviceInfo.appendChild(docDeviceDescription)

					docDeviceVendorId = self.doc.createElement("vendor_id")
					docDeviceVendorId.setAttribute("description", "Vendor ID")
					docDeviceVendorId.appendChild(self.doc.createTextNode(str(info[0])))
					docDeviceInfo.appendChild(docDeviceVendorId)

					docDeviceDeviceId = self.doc.createElement("device_id")
					docDeviceDeviceId.setAttribute("description", "Device ID")
					docDeviceDeviceId.appendChild(self.doc.createTextNode(str(info[1])))
					docDeviceInfo.appendChild(docDeviceDeviceId)

					docDeviceSerialNo = self.doc.createElement("serial_no")
					docDeviceSerialNo.setAttribute("description", "Serial number")
					docDeviceSerialNo.appendChild(self.doc.createTextNode(str(info[2])))
					docDeviceInfo.appendChild(docDeviceSerialNo)

					if(info[3] == meDriver.ME_BUS_TYPE_PCI):
						docDevicePciBusNo = self.doc.createElement("pci_bus_no")
						docDevicePciBusNo.setAttribute("description", "PCI bus number")
						docDevicePciBusNo.appendChild(self.doc.createTextNode(str(info[4])))
						docDeviceInfo.appendChild(docDevicePciBusNo)

						docDevicePciDevNo = self.doc.createElement("pci_dev_no")
						docDevicePciDevNo.setAttribute("description", "PCI device number")
						docDevicePciDevNo.appendChild(self.doc.createTextNode(str(info[5])))
						docDeviceInfo.appendChild(docDevicePciDevNo)

						docDevicePciFuncNo = self.doc.createElement("pci_func_no")
						docDevicePciFuncNo.setAttribute("description", "PCI function number")
						docDevicePciFuncNo.appendChild(self.doc.createTextNode(str(info[6])))
						docDeviceInfo.appendChild(docDevicePciFuncNo)
					else:
						docDeviceUsbBusNo = self.doc.createElement("usb_root_hub_no")
						docDeviceUsbBusNo.setAttribute("description", "USB root hub number")
						docDeviceUsbBusNo.appendChild(self.doc.createTextNode(str(info[4])))
						docDeviceInfo.appendChild(docDeviceUsbBusNo)

					if(info[7] == meDriver.ME_PLUGGED_IN):
						docSubdeviceList = self.doc.createElement("subdevice_list")
						docSubdeviceList.setAttribute("description", "Subdevice list")
						docDeviceEntry.appendChild(docSubdeviceList)

						for j in range(meDriver.meRQueryNumberSubdevices(host, i)):
							type = meDriver.meRQuerySubdeviceType(host, i, j)

							docSubdeviceEntry = self.doc.createElement("subdevice_entry")
							docSubdeviceEntry.setAttribute("description", "Subdevice")
							docSubdeviceEntry.setAttribute("subdevice_number", str(j))
							docSubdeviceEntry.setAttribute("subdevice_type", str(type[0]))
							docSubdeviceEntry.setAttribute("subdevice_extension", str(meDriver.ME_EXTENSION_TYPE_NONE))
							docSubdeviceEntry.setAttribute("subdevice_lock", "0")
							docSubdeviceList.appendChild(docSubdeviceEntry)

							docSubdeviceInfo = self.doc.createElement("subdevice_info")
							docSubdeviceInfo.setAttribute("description", "Subdevice info")
							docSubdeviceEntry.appendChild(docSubdeviceInfo)

							docSubdeviceType = self.doc.createElement("subdevice_type")
							docSubdeviceType.setAttribute("description", "Subdevice type")
							docSubdeviceType.appendChild(self.doc.createTextNode(str(type[0])))
							docSubdeviceInfo.appendChild(docSubdeviceType)

							docSubdeviceSubtype = self.doc.createElement("subdevice_sub_type")
							docSubdeviceSubtype.setAttribute("description", "Subdevice subtype")
							docSubdeviceSubtype.appendChild(self.doc.createTextNode(str(type[1])))
							docSubdeviceInfo.appendChild(docSubdeviceSubtype)

							docSubdeviceNoChannels = self.doc.createElement("subdevice_number_channels")
							docSubdeviceNoChannels.setAttribute("description", "Number of channels")
							docSubdeviceNoChannels.appendChild(self.doc.createTextNode(str(meDriver.meRQueryNumberChannels(host, i, j))))
							docSubdeviceInfo.appendChild(docSubdeviceNoChannels)

							if((type[0] == meDriver.ME_TYPE_AO) or (type[0] == meDriver.ME_TYPE_AI)):
								docRangeList = self.doc.createElement("range_list")
								docRangeList.setAttribute("description", "Range list")
								docSubdeviceEntry.appendChild(docRangeList)

								for k in range(meDriver.meRQueryNumberRanges(host, i, j, meDriver.ME_UNIT_ANY)):
									docRangeEntry = self.doc.createElement("range_entry")
									docRangeEntry.setAttribute("description", "Range")
									docRangeEntry.setAttribute("range_number", str(k))
									docRangeList.appendChild(docRangeEntry)

									info = meDriver.meRQueryRangeInfo(host, i, j, k)

									docRangeUnit = self.doc.createElement("range_unit")
									docRangeUnit.setAttribute("description", "Physical unit")
									docRangeUnit.appendChild(self.doc.createTextNode(str(info[0])))
									docRangeEntry.appendChild(docRangeUnit)

									docRangeMin = self.doc.createElement("range_min")
									docRangeMin.setAttribute("description", "Minimum physical value")
									docRangeMin.appendChild(self.doc.createTextNode(str(info[1])))
									docRangeEntry.appendChild(docRangeMin)

									docRangeMax = self.doc.createElement("range_max")
									docRangeMax.setAttribute("description", "Maximum physical value")
									docRangeMax.appendChild(self.doc.createTextNode(str(info[2])))
									docRangeEntry.appendChild(docRangeMax)

									docRangeMaxData = self.doc.createElement("range_max_data")
									docRangeMaxData.setAttribute("description", "Maximum digital value")
									docRangeMaxData.appendChild(self.doc.createTextNode(str(info[3])))
									docRangeEntry.appendChild(docRangeMaxData)

					treeDeviceEntry = self.treeCtrl.AppendItem(self.treeDeviceList, docDeviceEntry.getAttribute("description"))
					self.treeCtrl.SetPyData(treeDeviceEntry, docDeviceEntry)

					self.BuildSubtree(treeDeviceEntry, docDeviceEntry)
		except meDriver.error, value:
			dlg = wx.MessageDialog(self, str(value), "Error from ME driver system", wx.OK | wx.ICON_ERROR)
			dlg.ShowModal()
			dlg.Destroy()

	def BuildSubtree(self, treeRoot, docRoot):
		for child in docRoot.childNodes:
			if child.nodeType == Node.ELEMENT_NODE:
				if child.nodeName == "date":
					treeSub = self.treeCtrl.AppendItem(treeRoot, child.getAttribute("description"))
					self.treeCtrl.SetItemText(treeSub, self.DefineToString(child.firstChild), 1)
				elif child.nodeName == "device_list":
					treeSub = self.treeCtrl.AppendItem(treeRoot, child.getAttribute("description"))
				elif child.nodeName == "device_entry":
					treeSub = self.treeCtrl.AppendItem(treeRoot, child.getAttribute("description") + " " + child.getAttribute("device_number"))
				elif child.nodeName == "tcpip":
					treeSub = self.treeCtrl.AppendItem(treeRoot, child.getAttribute("description"))
				elif child.nodeName == "device_info":
					treeSub = self.treeCtrl.AppendItem(treeRoot, child.getAttribute("description"))
				elif child.nodeName == "subdevice_list":
					treeSub = self.treeCtrl.AppendItem(treeRoot, child.getAttribute("description"))
				elif child.nodeName == "subdevice_entry":
					treeSub = self.treeCtrl.AppendItem(treeRoot, child.getAttribute("description") + " " + child.getAttribute("subdevice_number"))
				elif child.nodeName == "subdevice_info":
					treeSub = self.treeCtrl.AppendItem(treeRoot, child.getAttribute("description"))
				elif child.nodeName == "range_list":
					treeSub = self.treeCtrl.AppendItem(treeRoot, child.getAttribute("description"))
				elif child.nodeName == "range_entry":
					treeSub = self.treeCtrl.AppendItem(treeRoot, child.getAttribute("description") + " " + child.getAttribute("range_number"))
				elif child.nodeName == "mux32m":
					treeSub = self.treeCtrl.AppendItem(treeRoot, child.getAttribute("description"))
				elif child.nodeName == "mux32s_list":
					treeSub = self.treeCtrl.AppendItem(treeRoot, child.getAttribute("description"))
				elif child.nodeName == "mux32s_entry":
					treeSub = self.treeCtrl.AppendItem(treeRoot, child.getAttribute("description") + " " + child.getAttribute("mux32s_number"))
				elif child.nodeName == "demux32":
					treeSub = self.treeCtrl.AppendItem(treeRoot, child.getAttribute("description"))
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


