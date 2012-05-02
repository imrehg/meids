# Copyright (2004) Meilhaus Electronic GmbH.
# All rights reserved.

import wx
import os
import string

import meDriver

class newDemux32Dialog(wx.Dialog):
	"""Configuration dialog to add a demux device."""
	def __init__(self, parent, doc, treeCtrl, treeSubdeviceEntrySelected):
		wx.Dialog.__init__(self, parent, -1, "Register a ME-DEMUX32", size = wx.Size(400, 400))

		topBox = wx.BoxSizer(wx.VERTICAL)

		upperBox = wx.BoxSizer(wx.VERTICAL)
		lowerBox = wx.BoxSizer(wx.HORIZONTAL)

		topBox.Add(upperBox)
		topBox.Add(lowerBox)

		docSubdeviceEntrySelected = treeCtrl.GetPyData(treeSubdeviceEntrySelected)
		if(docSubdeviceEntrySelected.getElementsByTagName("demux32")):
			dlg = wx.MessageDialog(self, 'There was already a ME-DEMUX32 registered for this subdevice', "Error", wx.OK | wx.ICON_ERROR)
			dlg.ShowModal()
			dlg.Destroy()
			self.Destroy()
			return

		# Get the number of channels of this AO subdevice
		docSubdeviceInfo = docSubdeviceEntrySelected.getElementsByTagName("subdevice_info")[0]
		docSubdeviceChannels = docSubdeviceInfo.getElementsByTagName("subdevice_number_channels")[0]

		aiChannelTxt = wx.StaticText(self, -1, "AO channel:", wx.DefaultPosition, wx.DefaultSize)
		aiChannel = wx.SpinCtrl(self, -1, '0', wx.DefaultPosition, wx.DefaultSize,
						min = 0, max = string.atoi(string.strip(docSubdeviceChannels.firstChild.data)) - 1)

		# Get the available DIO subdevices
		dioList = self.GetDIOSubdeviceList(treeCtrl.GetPyData(treeCtrl.GetRootItem()))
		if(len(dioList) == 0):
			dlg = wx.MessageDialog(self, 'There are no digital I/O subdevices available', "Error", wx.OK | wx.ICON_ERROR)
			dlg.ShowModal()
			dlg.Destroy()
			self.Destroy()
			return

		dioTxtList = []
		for dio in dioList:
			dioTxtList.append("Device %s, Subdevice %s (%s Channels)" % (dio[0], dio[1], dio[2]))

		dioSubdeviceTxt = wx.StaticText(self, -1, "Digital I/O:", wx.DefaultPosition, wx.DefaultSize)
		dioSubdevice = wx.Choice(self, -1, wx.DefaultPosition, wx.DefaultSize, dioTxtList)
		for i in range(len(dioTxtList)):
			dioSubdevice.SetClientData(i, dioList[i][3])
		dioSubdevice.SetSelection(0)

		# Timing subdevice control
		timingSubdevice = wx.CheckBox(self, -1, "Operation mode 'Streaming Output'")
		timingSubdevice.Enable(False)
		docDeviceEntry = docSubdeviceEntrySelected.parentNode.parentNode
		docDeviceInfo = docDeviceEntry.getElementsByTagName("device_info")[0]
		docDeviceId = docDeviceInfo.getElementsByTagName("device_id")[0]
		deviceId = int(docDeviceId.firstChild.data)

		timer_device = -1
		timer_subdevice = -1
		docTimerSubdeviceEntry = None
		capable = False

		if((deviceId == 0x4680) or (deviceId == 0x4681) or (deviceId == 0x4682) or (deviceId == 0x4683)):
			docSubdeviceList = docDeviceEntry.getElementsByTagName("subdevice_list")[0]
			docSubdeviceEntries = docSubdeviceList.getElementsByTagName("subdevice_entry")
			for docSubdeviceEntry in docSubdeviceEntries:
				docSubdeviceInfo = docSubdeviceEntry.getElementsByTagName("subdevice_info")[0]
				docSubdeviceType = docSubdeviceInfo.getElementsByTagName("subdevice_type")[0]
				subdevice_type = int(docSubdeviceType.firstChild.data)
				if(subdevice_type == meDriver.ME_TYPE_AO):
					if docSubdeviceEntrySelected.getAttribute("subdevice_number") == docSubdeviceEntry.getAttribute("subdevice_number"):
						capable = True
					else:
						capable = False
					break

			if(capable):
				i = 0
				for docSubdeviceEntry in docSubdeviceEntries:
					docSubdeviceInfo = docSubdeviceEntry.getElementsByTagName("subdevice_info")[0]
					docSubdeviceType = docSubdeviceInfo.getElementsByTagName("subdevice_type")[0]
					subdevice_type = int(docSubdeviceType.firstChild.data)
					if(subdevice_type == meDriver.ME_TYPE_AO):
						i = i + 1
						if(i == 4):
							docTimerSubdeviceEntry = docSubdeviceEntry
							if(docTimerSubdeviceEntry.getAttribute("subdevice_lock") == "0"):
								timer_device = int(docDeviceEntry.getAttribute("device_number"))
								timer_subdevice = int(docSubdeviceEntry.getAttribute("subdevice_number"))
								timingSubdevice.Enable(True)

		# Add controls to upper box
		upperBox.Add(aiChannelTxt, 0, wx.LEFT | wx.RIGHT | wx.TOP, 10)
		upperBox.Add(aiChannel, 0, wx.LEFT | wx.RIGHT | wx.BOTTOM, 10)
		upperBox.Add(dioSubdeviceTxt, 0, wx.LEFT | wx.RIGHT | wx.TOP, 10)
		upperBox.Add(dioSubdevice, 0, wx.LEFT | wx.RIGHT | wx.BOTTOM, 10)
		upperBox.Add(timingSubdevice, 0, wx.LEFT | wx.RIGHT | wx.BOTTOM, 10)

		# Add controls to lower box
		okBtn = wx.Button(self, wx.ID_OK, " OK ", wx.DefaultPosition, wx.DefaultSize)
		cancelBtn = wx.Button(self, wx.ID_CANCEL, " Cancel ", wx.DefaultPosition, wx.DefaultSize)
		lowerBox.Add(okBtn, flag = wx.ALL, border = 60)
		lowerBox.Add(cancelBtn, flag = wx.ALL, border = 60)

		# Enable auto layout
		self.SetAutoLayout(True)
		self.SetSizer(topBox)
		topBox.Fit(self)

		# Show the dialog
		val = self.ShowModal()

		if val == wx.ID_OK:
			if(timingSubdevice.GetValue() == True):
				if(docSubdeviceEntrySelected.parentNode.parentNode.getAttribute("device_number") != dioList[dioSubdevice.GetSelection()][0]):
					dlg = wx.MessageDialog(self,
							'Use of the operation mode "Streaming Output" is only possible with a\nDIO subdevice on the same device', "Error", wx.OK | wx.ICON_ERROR)
					dlg.ShowModal()
					dlg.Destroy()
					self.Destroy()
					return

			# Append demux device to subdevice
			docDemux32 = doc.createElement("demux32")
			docDemux32.setAttribute("description", "ME-DEMUX32")
			docSubdeviceEntrySelected.appendChild(docDemux32)
			docSubdeviceEntrySelected.setAttribute("subdevice_extension", str(meDriver.ME_EXTENSION_TYPE_DEMUX32))
			treeDemux32 = treeCtrl.AppendItem(treeSubdeviceEntrySelected, docDemux32.getAttribute("description"))
			treeCtrl.SetPyData(treeDemux32, docDemux32)

			docDemux32AoChannel = doc.createElement("demux32_ao_channel")
			docDemux32AoChannel.setAttribute("description", "Analog output channel")
			docDemux32.appendChild(docDemux32AoChannel)
			docDemux32AoChannelTxt = doc.createTextNode(str(aiChannel.GetValue()))
			docDemux32AoChannel.appendChild(docDemux32AoChannelTxt)
			treeDemux32AoChannel = treeCtrl.AppendItem(treeDemux32, docDemux32AoChannel.getAttribute("description"))
			treeCtrl.SetPyData(treeDemux32AoChannel, docDemux32AoChannel)
			treeCtrl.SetItemText(treeDemux32AoChannel, string.strip(docDemux32AoChannelTxt.data), 1)

			docDemux32DioDevice = doc.createElement("demux32_dio_device")
			docDemux32DioDevice.setAttribute("description", "Digital I/O device")
			docDemux32.appendChild(docDemux32DioDevice)
			docDemux32DioDeviceTxt = doc.createTextNode(str(dioList[dioSubdevice.GetSelection()][0]))
			docDemux32DioDevice.appendChild(docDemux32DioDeviceTxt)
			treeDemux32DioDevice = treeCtrl.AppendItem(treeDemux32, docDemux32DioDevice.getAttribute("description"))
			treeCtrl.SetPyData(treeDemux32DioDevice, docDemux32DioDevice)
			treeCtrl.SetItemText(treeDemux32DioDevice, string.strip(docDemux32DioDeviceTxt.data), 1)

			docDemux32DioSubdevice = doc.createElement("demux32_dio_subdevice")
			docDemux32DioSubdevice.setAttribute("description", "Digital I/O subdevice")
			docDemux32.appendChild(docDemux32DioSubdevice)
			docDemux32DioSubdeviceTxt = doc.createTextNode(str(dioList[dioSubdevice.GetSelection()][1]))
			docDemux32DioSubdevice.appendChild(docDemux32DioSubdeviceTxt)
			treeDemux32DioSubdevice = treeCtrl.AppendItem(treeDemux32, docDemux32DioSubdevice.getAttribute("description"))
			treeCtrl.SetPyData(treeDemux32DioSubdevice, docDemux32DioSubdevice)
			treeCtrl.SetItemText(treeDemux32DioSubdevice, string.strip(docDemux32DioSubdeviceTxt.data), 1)

			# Mark the selected digital i/o subdevice as locked
			docDioSubdevice = dioSubdevice.GetClientData(dioSubdevice.GetSelection())
			docDioSubdevice.setAttribute("subdevice_lock", "1")
			docDioSubdevice.setAttribute("lock_device", docSubdeviceEntrySelected.parentNode.parentNode.getAttribute("device_number"))
			docDioSubdevice.setAttribute("lock_subdevice", docSubdeviceEntrySelected.getAttribute("subdevice_number"))

			if(timingSubdevice.GetValue() == True):
				docDemux32.setAttribute("timed", "1")

				docDemux32TimerDevice = doc.createElement("demux32_timer_device")
				docDemux32TimerDevice.setAttribute("description", "Timer device")
				docDemux32.appendChild(docDemux32TimerDevice)
				docDemux32TimerDeviceTxt = doc.createTextNode(str(timer_device))
				docDemux32TimerDevice.appendChild(docDemux32TimerDeviceTxt)
				treeDemux32TimerDevice = treeCtrl.AppendItem(treeDemux32, docDemux32TimerDevice.getAttribute("description"))
				treeCtrl.SetPyData(treeDemux32TimerDevice, docDemux32TimerDevice)
				treeCtrl.SetItemText(treeDemux32TimerDevice, docDemux32TimerDeviceTxt.data, 1)

				docDemux32TimerSubdevice = doc.createElement("demux32_timer_subdevice")
				docDemux32TimerSubdevice.setAttribute("description", "Timer subdevice")
				docDemux32.appendChild(docDemux32TimerSubdevice)
				docDemux32TimerSubdeviceTxt = doc.createTextNode(str(timer_subdevice))
				docDemux32TimerSubdevice.appendChild(docDemux32TimerSubdeviceTxt)
				treeDemux32TimerSubdevice = treeCtrl.AppendItem(treeDemux32, docDemux32TimerSubdevice.getAttribute("description"))
				treeCtrl.SetPyData(treeDemux32TimerSubdevice, docDemux32TimerSubdevice)
				treeCtrl.SetItemText(treeDemux32TimerSubdevice, docDemux32TimerSubdeviceTxt.data, 1)

				# Mark the analog output as locked
				docTimerSubdeviceEntry.setAttribute("subdevice_lock", "1")
				docTimerSubdeviceEntry.setAttribute("lock_device", docSubdeviceEntrySelected.parentNode.parentNode.getAttribute("device_number"))
				docTimerSubdeviceEntry.setAttribute("lock_subdevice", docSubdeviceEntrySelected.getAttribute("subdevice_number"))
			else:
				docDemux32.setAttribute("timed", "0")

		self.Destroy()

	def GetDIOSubdeviceList(self, docRoot):
		retList = []
		docDeviceList = docRoot.getElementsByTagName("device_list")[0]
		for docDevice in docDeviceList.getElementsByTagName("device_entry"):
			docSubdeviceList = docDevice.getElementsByTagName("subdevice_list")[0]
			for docSubdeviceEntry in docSubdeviceList.getElementsByTagName("subdevice_entry"):
				if(docSubdeviceEntry.getAttribute("subdevice_lock") == "0"):
					docSubdeviceInfo = docSubdeviceEntry.getElementsByTagName("subdevice_info")[0]
					docSubdeviceType = docSubdeviceInfo.getElementsByTagName("subdevice_type")[0]
					if((string.atoi(string.strip(docSubdeviceType.firstChild.data)) == meDriver.ME_TYPE_DIO) or
						(string.atoi(string.strip(docSubdeviceType.firstChild.data)) == meDriver.ME_TYPE_DO)):
						docSubdeviceNumberChannels = docSubdeviceInfo.getElementsByTagName("subdevice_number_channels")[0]
						retList.append((docDevice.getAttribute("device_number"),
							docSubdeviceEntry.getAttribute("subdevice_number"), string.strip(docSubdeviceNumberChannels.firstChild.data), docSubdeviceEntry))
		return retList
