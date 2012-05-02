# Copyright (2004) Meilhaus Electronic GmbH.
# All rights reserved.

import wx
import os
import string

import meDriver

class newMux32mDialog(wx.Dialog):
	"""Configuration dialog to add a mux master device."""
	def __init__(self, parent, doc, treeCtrl, treeSubdeviceEntrySelected):
		wx.Dialog.__init__(self, parent, -1, "Register a ME-MUX32-M", size = wx.Size(400, 400))

		topBox = wx.BoxSizer(wx.VERTICAL)

		upperBox = wx.BoxSizer(wx.VERTICAL)
		lowerBox = wx.BoxSizer(wx.HORIZONTAL)

		topBox.Add(upperBox)
		topBox.Add(lowerBox)

		docSubdeviceEntrySelected = treeCtrl.GetPyData(treeSubdeviceEntrySelected)
		if(docSubdeviceEntrySelected.getElementsByTagName("mux32m")):
			dlg = wx.MessageDialog(self, 'There was already a ME-MUX32-M registered for this subdevice', "Error", wx.OK | wx.ICON_ERROR)
			dlg.ShowModal()
			dlg.Destroy()
			self.Destroy()
			return

		# Get the number of channels of this AI subdevice
		docSubdeviceInfo = docSubdeviceEntrySelected.getElementsByTagName("subdevice_info")[0]
		docSubdeviceChannels = docSubdeviceInfo.getElementsByTagName("subdevice_number_channels")[0]

		aiChannelTxt = wx.StaticText(self, -1, "AI channel:", wx.DefaultPosition, wx.DefaultSize)
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

		timingSubdevice = wx.CheckBox(self, -1, "Operation mode 'Streaming Input'")

		# Check whether this device is able to do timing for mux
		docDeviceEntry = docSubdeviceEntrySelected.parentNode.parentNode
		docDeviceInfo = docDeviceEntry.getElementsByTagName("device_info")[0]
		docDeviceId = docDeviceInfo.getElementsByTagName("device_id")[0]
		deviceId = int(docDeviceId.firstChild.data)

		timer_device = -1
		timer_subdevice = -1
		docTimerSubdeviceEntry = None

		if((deviceId == 0x4680) or (deviceId == 0x4681) or (deviceId == 0x4682) or (deviceId == 0x4683)):
			docSubdeviceList = docDeviceEntry.getElementsByTagName("subdevice_list")[0]
			docSubdeviceEntries = docSubdeviceList.getElementsByTagName("subdevice_entry")
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
						else:
							timingSubdevice.Enable(False)
		else:
			timingSubdevice.Enable(False)

		# Add to upper box
		upperBox.Add(aiChannelTxt, 0, wx.LEFT | wx.RIGHT | wx.TOP, 10)
		upperBox.Add(aiChannel, 0, wx.LEFT | wx.RIGHT | wx.BOTTOM, 10)
		upperBox.Add(dioSubdeviceTxt, 0, wx.LEFT | wx.RIGHT | wx.TOP, 10)
		upperBox.Add(dioSubdevice, 0, wx.LEFT | wx.RIGHT | wx.BOTTOM, 10)
		upperBox.Add(timingSubdevice, 0, wx.LEFT | wx.RIGHT | wx.BOTTOM, 10)

		# Add to lower box
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
							'Use of the operation mode "Streaming Input" is only possible with a\nDIO subdevice on the same device', "Error", wx.OK | wx.ICON_ERROR)
					dlg.ShowModal()
					dlg.Destroy()
					self.Destroy()
					return

			# Append master mux device to subdevice
			docMux32m = doc.createElement("mux32m")
			docMux32m.setAttribute("description", "ME-MUX32-M")
			docSubdeviceEntrySelected.appendChild(docMux32m)
			docSubdeviceEntrySelected.setAttribute("subdevice_extension", str(meDriver.ME_EXTENSION_TYPE_MUX32M))
			treeMux32m = treeCtrl.AppendItem(treeSubdeviceEntrySelected, docMux32m.getAttribute("description"))
			treeCtrl.SetPyData(treeMux32m, docMux32m)

			docMux32mAiChannel = doc.createElement("mux32m_ai_channel")
			docMux32mAiChannel.setAttribute("description", "Analog input channel")
			docMux32m.appendChild(docMux32mAiChannel)
			docMux32mAiChannelTxt = doc.createTextNode(str(aiChannel.GetValue()))
			docMux32mAiChannel.appendChild(docMux32mAiChannelTxt)
			treeMux32mAiChannel = treeCtrl.AppendItem(treeMux32m, docMux32mAiChannel.getAttribute("description"))
			treeCtrl.SetPyData(treeMux32mAiChannel, docMux32mAiChannel)
			treeCtrl.SetItemText(treeMux32mAiChannel, string.strip(docMux32mAiChannelTxt.data), 1)

			docMux32mDioDevice = doc.createElement("mux32m_dio_device")
			docMux32mDioDevice.setAttribute("description", "Digital I/O device")
			docMux32m.appendChild(docMux32mDioDevice)
			docMux32mDioDeviceTxt = doc.createTextNode(str(dioList[dioSubdevice.GetSelection()][0]))
			docMux32mDioDevice.appendChild(docMux32mDioDeviceTxt)
			treeMux32mDioDevice = treeCtrl.AppendItem(treeMux32m, docMux32mDioDevice.getAttribute("description"))
			treeCtrl.SetPyData(treeMux32mDioDevice, docMux32mDioDevice)
			treeCtrl.SetItemText(treeMux32mDioDevice, string.strip(docMux32mDioDeviceTxt.data), 1)

			docMux32mDioSubdevice = doc.createElement("mux32m_dio_subdevice")
			docMux32mDioSubdevice.setAttribute("description", "Digital I/O subdevice")
			docMux32m.appendChild(docMux32mDioSubdevice)
			docMux32mDioSubdeviceTxt = doc.createTextNode(str(dioList[dioSubdevice.GetSelection()][1]))
			docMux32mDioSubdevice.appendChild(docMux32mDioSubdeviceTxt)
			treeMux32mDioSubdevice = treeCtrl.AppendItem(treeMux32m, docMux32mDioSubdevice.getAttribute("description"))
			treeCtrl.SetPyData(treeMux32mDioSubdevice, docMux32mDioSubdevice)
			treeCtrl.SetItemText(treeMux32mDioSubdevice, string.strip(docMux32mDioSubdeviceTxt.data), 1)

			# Mark the selected digital i/o subdevice as locked
			docDioSubdevice = dioSubdevice.GetClientData(dioSubdevice.GetSelection())
			docDioSubdevice.setAttribute("subdevice_lock", "1")
			docDioSubdevice.setAttribute("lock_device", docSubdeviceEntrySelected.parentNode.parentNode.getAttribute("device_number"))
			docDioSubdevice.setAttribute("lock_subdevice", docSubdeviceEntrySelected.getAttribute("subdevice_number"))

			if(timingSubdevice.GetValue() == True):
				docMux32m.setAttribute("timed", "1")

				docMux32mTimerDevice = doc.createElement("mux32m_timer_device")
				docMux32mTimerDevice.setAttribute("description", "Timer device")
				docMux32m.appendChild(docMux32mTimerDevice)
				docMux32mTimerDeviceTxt = doc.createTextNode(str(timer_device))
				docMux32mTimerDevice.appendChild(docMux32mTimerDeviceTxt)
				treeMux32mTimerDevice = treeCtrl.AppendItem(treeMux32m, docMux32mTimerDevice.getAttribute("description"))
				treeCtrl.SetPyData(treeMux32mTimerDevice, docMux32mTimerDevice)
				treeCtrl.SetItemText(treeMux32mTimerDevice, docMux32mTimerDeviceTxt.data, 1)

				docMux32mTimerSubdevice = doc.createElement("mux32m_timer_subdevice")
				docMux32mTimerSubdevice.setAttribute("description", "Timer subdevice")
				docMux32m.appendChild(docMux32mTimerSubdevice)
				docMux32mTimerSubdeviceTxt = doc.createTextNode(str(timer_subdevice))
				docMux32mTimerSubdevice.appendChild(docMux32mTimerSubdeviceTxt)
				treeMux32mTimerSubdevice = treeCtrl.AppendItem(treeMux32m, docMux32mTimerSubdevice.getAttribute("description"))
				treeCtrl.SetPyData(treeMux32mTimerSubdevice, docMux32mTimerSubdevice)
				treeCtrl.SetItemText(treeMux32mTimerSubdevice, docMux32mTimerSubdeviceTxt.data, 1)

				# Mark the analog output as locked
				docTimerSubdeviceEntry.setAttribute("subdevice_lock", "1")
				docTimerSubdeviceEntry.setAttribute("lock_device", docSubdeviceEntrySelected.parentNode.parentNode.getAttribute("device_number"))
				docTimerSubdeviceEntry.setAttribute("lock_subdevice", docSubdeviceEntrySelected.getAttribute("subdevice_number"))
			else:
				docMux32m.setAttribute("timed", "0")

			docMux32sList = doc.createElement("mux32s_list")
			docMux32sList.setAttribute("description", "ME-MUX32-S list")
			docMux32m.appendChild(docMux32sList)
			treeMux32sList = treeCtrl.AppendItem(treeMux32m, docMux32sList.getAttribute("description"))
			treeCtrl.SetPyData(treeMux32sList, docMux32sList)

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
						retList.append(
								(docDevice.getAttribute("device_number"),
								 docSubdeviceEntry.getAttribute("subdevice_number"),
								 string.strip(docSubdeviceNumberChannels.firstChild.data),
								 docSubdeviceEntry))
		return retList
