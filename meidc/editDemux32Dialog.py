# Copyright (2004) Meilhaus Electronic GmbH.
# All rights reserved.

import wx
import os
import string

import meDriver

class editDemux32Dialog(wx.Dialog):
	"""Configuration dialog to add a demux device."""
	def __init__(self, parent, doc, treeCtrl, treeDemux32):
		wx.Dialog.__init__(self, parent, -1, "Edit a demux device", size = wx.Size(400, 400))

		# Box sizers
		topBox = wx.BoxSizer(wx.VERTICAL)
		upperBox = wx.BoxSizer(wx.VERTICAL)
		lowerBox = wx.BoxSizer(wx.HORIZONTAL)
		topBox.Add(upperBox)
		topBox.Add(lowerBox)

		# Get the number of channels of this AO subdevice
		docDemux32 = treeCtrl.GetPyData(treeDemux32)
		docSubdeviceEntry = docDemux32.parentNode
		docSubdeviceInfo = docSubdeviceEntry.getElementsByTagName("subdevice_info")[0]
		docSubdeviceChannels = docSubdeviceInfo.getElementsByTagName("subdevice_number_channels")[0]

		# AO channel selection
		aoChannelTxt = wx.StaticText(self, -1, "AO channel:", wx.DefaultPosition, wx.DefaultSize)
		aoChannel = wx.SpinCtrl(self, -1, '0', wx.DefaultPosition, wx.DefaultSize,
						min = 0, max = string.atoi(string.strip(docSubdeviceChannels.firstChild.data)) - 1)

		# Get the available DIO subdevices
		docDemux32DioDevice = docDemux32.getElementsByTagName("demux32_dio_device")[0]
		docDemux32DioSubdevice = docDemux32.getElementsByTagName("demux32_dio_subdevice")[0]
		actDevice = string.strip(docDemux32DioDevice.firstChild.data)
		actSubdevice = string.strip(docDemux32DioSubdevice.firstChild.data)
		dioList = self.GetDIOSubdeviceList(treeCtrl.GetPyData(treeCtrl.GetRootItem()), actDevice, actSubdevice)
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

		# Timer subdevice selection
		timingSubdevice = wx.CheckBox(self, -1, "Operation mode 'Streaming Output'")
		timingSubdevice.Enable(False)
		docDeviceEntry = docDemux32.parentNode.parentNode.parentNode
		docDeviceInfo = docDeviceEntry.getElementsByTagName("device_info")[0]
		docDeviceId = docDeviceInfo.getElementsByTagName("device_id")[0]
		deviceId = int(docDeviceId.firstChild.data)
		timer_device = -1
		timer_subdevice = -1
		docTimerSubdeviceEntry = None

		if((deviceId == 0x4680) or (deviceId == 0x4681) or (deviceId == 0x4682) or (deviceId == 0x4683)):
			docSubdeviceList = docDeviceEntry.getElementsByTagName("subdevice_list")[0]
			docSubdeviceEntries = docSubdeviceList.getElementsByTagName("subdevice_entry")
			for docSubdeviceEntry in docSubdeviceEntries:
				docSubdeviceInfo = docSubdeviceEntry.getElementsByTagName("subdevice_info")[0]
				docSubdeviceType = docSubdeviceInfo.getElementsByTagName("subdevice_type")[0]
				subdevice_type = int(docSubdeviceType.firstChild.data)
				if(subdevice_type == meDriver.ME_TYPE_AO):
					if docDemux32.parentNode.getAttribute("subdevice_number") == docSubdeviceEntry.getAttribute("subdevice_number"):
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
							if((docTimerSubdeviceEntry.getAttribute("subdevice_lock") == "0") or
							((docTimerSubdeviceEntry.getAttribute("subdevice_lock") == "1") and
							(docDemux32.getAttribute("timed") == "1"))):
								timer_device = int(docDeviceEntry.getAttribute("device_number"))
								timer_subdevice = int(docSubdeviceEntry.getAttribute("subdevice_number"))
								timingSubdevice.Enable(True)

		# Add controls to upper box
		upperBox.Add(aoChannelTxt, 0, wx.LEFT | wx.RIGHT | wx.TOP, 10)
		upperBox.Add(aoChannel, 0, wx.LEFT | wx.RIGHT | wx.BOTTOM, 10)
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
				if(docDemux32.parentNode.parentNode.parentNode.getAttribute("device_number") != dioList[dioSubdevice.GetSelection()][0]):
					dlg = wx.MessageDialog(self,
							'Use of the operation mode "Streaming Output" is only possible with a\nDIO subdevice on the same device', "Error", wx.OK | wx.ICON_ERROR)
					dlg.ShowModal()
					dlg.Destroy()
					self.Destroy()
					return

			docDemux32AoChannel = docDemux32.getElementsByTagName("demux32_ao_channel")[0]
			docDemux32AoChannel.firstChild.data = aoChannel.GetValue()

			docDemux32DioDevice.firstChild.data = str(dioList[dioSubdevice.GetSelection()][0])
			docDemux32DioSubdevice.firstChild.data = str(dioList[dioSubdevice.GetSelection()][1])

			# Mark the selected digital i/o subdevice as locked
			for docDioSubdevice in dioList:
				docDioSubdevice[3].setAttribute("subdevice_lock", "0")
			docDioSubdevice = dioSubdevice.GetClientData(dioSubdevice.GetSelection())
			docDioSubdevice.setAttribute("subdevice_lock", "1")
			docDioSubdevice.setAttribute("lock_device", docSubdeviceEntry.parentNode.parentNode.getAttribute("device_number"))
			docDioSubdevice.setAttribute("lock_subdevice", docSubdeviceEntry.getAttribute("subdevice_number"))

			if(timingSubdevice.GetValue() == True):
				docDemux32.setAttribute("timed", "1")

				if(len(docDemux32.getElementsByTagName("demux32_timer_device")) == 0):
					cnt = treeCtrl.GetChildrenCount(treeDemux32)
					docDemux32TimerDevice = doc.createElement("demux32_timer_device")
					docDemux32TimerDevice.setAttribute("description", "Timer device")
					docDemux32.appendChild(docDemux32TimerDevice)
					docDemux32TimerDeviceTxt = doc.createTextNode(str(timer_device))
					docDemux32TimerDevice.appendChild(docDemux32TimerDeviceTxt)
					treeDemux32TimerDevice = treeCtrl.AppendItem(treeDemux32, docDemux32TimerDevice.getAttribute("description"))
					treeCtrl.SetPyData(treeDemux32TimerDevice, docDemux32TimerDevice)
					treeCtrl.SetItemText(treeDemux32TimerDevice, docDemux32TimerDeviceTxt.data, 1)

					cnt = treeCtrl.GetChildrenCount(treeDemux32)
					docDemux32TimerSubdevice = doc.createElement("demux32_timer_subdevice")
					docDemux32TimerSubdevice.setAttribute("description", "Timer subdevice")
					docDemux32.appendChild(docDemux32TimerSubdevice)
					docDemux32TimerSubdeviceTxt = doc.createTextNode(str(timer_subdevice))
					docDemux32TimerSubdevice.appendChild(docDemux32TimerSubdeviceTxt)
					treeDemux32TimerSubdevice = treeCtrl.AppendItem(treeDemux32, docDemux32TimerSubdevice.getAttribute("description"))
					treeCtrl.SetPyData(treeDemux32TimerSubdevice, docDemux32TimerSubdevice)
					treeCtrl.SetItemText(treeDemux32TimerSubdevice, docDemux32TimerSubdeviceTxt.data, 1)
				else:
					docDemux32TimerDevice = docDemux32.getElementsByTagName("demux32_timer_device")[0]
					docDemux32TimerDevice.firstChild.data = str(timer_device)
					docDemux32TimerSubdevice = docDemux32.getElementsByTagName("demux32_timer_subdevice")[0]
					docDemux32TimerSubdevice.firstChild.data = str(timer_subdevice)

				# Mark the analog output as locked
				docTimerSubdeviceEntry.setAttribute("subdevice_lock", "1")
				docTimerSubdeviceEntry.setAttribute("lock_device", docDemux32.parentNode.parentNode.parentNode.getAttribute("device_number"))
				docTimerSubdeviceEntry.setAttribute("lock_subdevice", docDemux32.parentNode.getAttribute("subdevice_number"))
			else:
				docDemux32.setAttribute("timed", "0")
				if(len(docDemux32.getElementsByTagName("demux32_timer_device")) != 0):
					docDemux32TimerDevice = docDemux32.getElementsByTagName("demux32_timer_device")[0]
					docDemux32TimerSubdevice = docDemux32.getElementsByTagName("demux32_timer_subdevice")[0]

					id, cookie = treeCtrl.GetFirstChild(treeDemux32)
					while(id.IsOk()):
						if(treeCtrl.GetPyData(id).nodeName == "demux32_timer_device"):
							treeCtrl.Delete(id)
							break
						id, cookie = treeCtrl.GetNextChild(treeDemux32, cookie)

					id, cookie = treeCtrl.GetFirstChild(treeDemux32)
					while(id.IsOk()):
						if(treeCtrl.GetPyData(id).nodeName == "demux32_timer_subdevice"):
							treeCtrl.Delete(id)
							break
						id, cookie = treeCtrl.GetNextChild(treeDemux32, cookie)

					docDemux32TimerDevice = docDemux32.removeChild(docDemux32TimerDevice)
					docDemux32TimerDevice.unlink()
					docDemux32TimerSubdevice = docDemux32.removeChild(docDemux32TimerSubdevice)
					docDemux32TimerSubdevice.unlink()

					# Mark the analog output as unlocked
					docTimerSubdeviceEntry.setAttribute("subdevice_lock", "0")

		self.Destroy()

	def GetDIOSubdeviceList(self, docRoot, actDevice, actSubdevice):
		retList = []
		docDeviceList = docRoot.getElementsByTagName("device_list")[0]
		for docDevice in docDeviceList.getElementsByTagName("device_entry"):
			docSubdeviceList = docDevice.getElementsByTagName("subdevice_list")[0]
			for docSubdevice in docSubdeviceList.getElementsByTagName("subdevice_entry"):
				if((docSubdevice.getAttribute("subdevice_lock") == "0") or
					(docSubdevice.getAttribute("subdevice_lock") == "1" and
					docDevice.getAttribute("device_number") == actDevice and
					docSubdevice.getAttribute("subdevice_number") == actSubdevice)):
					docSubdeviceInfo = docSubdevice.getElementsByTagName("subdevice_info")[0]
					docSubdeviceType = docSubdeviceInfo.getElementsByTagName("subdevice_type")[0]
					if((string.atoi(string.strip(docSubdeviceType.firstChild.data)) == meDriver.ME_TYPE_DIO) or
						(string.atoi(string.strip(docSubdeviceType.firstChild.data)) == meDriver.ME_TYPE_DO)):
						docSubdeviceNumberChannels = docSubdeviceInfo.getElementsByTagName("subdevice_number_channels")[0]
						retList.append((docDevice.getAttribute("device_number"),
							docSubdevice.getAttribute("subdevice_number"), string.strip(docSubdeviceNumberChannels.firstChild.data), docSubdevice))
		return retList
