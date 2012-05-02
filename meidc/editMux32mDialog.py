# Copyright (2004) Meilhaus Electronic GmbH.
# All rights reserved.

import wx
import os
import string

import meDriver

class editMux32mDialog(wx.Dialog):
	"""Configuration dialog to edit a mux master device."""
	def __init__(self, parent, doc, treeCtrl, treeMux32m):
		wx.Dialog.__init__(self, parent, -1, "Edit a mux master device", size = wx.Size(400, 400))

		# Box sizers
		topBox = wx.BoxSizer(wx.VERTICAL)
		upperBox = wx.BoxSizer(wx.VERTICAL)
		lowerBox = wx.BoxSizer(wx.HORIZONTAL)
		topBox.Add(upperBox)
		topBox.Add(lowerBox)

		# Get the number of channels of this AI subdevice
		docMux32m = treeCtrl.GetPyData(treeMux32m)
		docSubdeviceEntry = docMux32m.parentNode
		docSubdeviceInfo = docSubdeviceEntry.getElementsByTagName("subdevice_info")[0]
		docSubdeviceChannels = docSubdeviceInfo.getElementsByTagName("subdevice_number_channels")[0]
		aiChannelCount = int(docSubdeviceChannels.firstChild.data)

		# AI channel selection
		aiChannelTxt = wx.StaticText(self, -1, "AI channel:", wx.DefaultPosition, wx.DefaultSize)
		aiChannel = wx.SpinCtrl(self, -1, '0', wx.DefaultPosition, wx.DefaultSize, min = 0, max = aiChannelCount - 1)

		# Get the available DIO subdevices
		docMux32mDioDevice = docMux32m.getElementsByTagName("mux32m_dio_device")[0]
		docMux32mDioSubdevice = docMux32m.getElementsByTagName("mux32m_dio_subdevice")[0]
		actDevice = string.strip(docMux32mDioDevice.firstChild.data)
		actSubdevice = string.strip(docMux32mDioSubdevice.firstChild.data)
		dioList = self.GetDIOSubdeviceList(treeCtrl.GetPyData(treeCtrl.GetRootItem()), actDevice, actSubdevice)
		if(len(dioList) == 0):
			dlg = wx.MessageDialog(self, 'There are no digital I/O subdevices available', "Error", wx.OK | wx.ICON_ERROR)
			dlg.ShowModal()
			dlg.Destroy()
			self.Destroy()
			return

		# DIO selection
		dioTxtList = []
		for dio in dioList:
			dioTxtList.append("Device %s, Subdevice %s (%s Channels)" % (dio[0], dio[1], dio[2]))
		dioSubdeviceTxt = wx.StaticText(self, -1, "Digital I/O:", wx.DefaultPosition, wx.DefaultSize)
		dioSubdevice = wx.Choice(self, -1, wx.DefaultPosition, wx.DefaultSize, dioTxtList)
		for i in range(len(dioTxtList)):
			dioSubdevice.SetClientData(i, dioList[i][3])
		dioSubdevice.SetSelection(0)

		# Timer subdevice selection
		timingSubdevice = wx.CheckBox(self, -1, "Operation mode 'Streaming Input'")
		docDeviceEntry = docMux32m.parentNode.parentNode.parentNode
		docDeviceInfo = docDeviceEntry.getElementsByTagName("device_info")[0]
		docDeviceId = docDeviceInfo.getElementsByTagName("device_id")[0]
		deviceId = int(docDeviceId.firstChild.data)
		timer_device = -1
		timer_subdevice = -1
		docTimerSubdeviceEntry = None

		# Only enable timer subdevice selection for ME-4680 boards
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
						if((docTimerSubdeviceEntry.getAttribute("subdevice_lock") == "0") or
							((docTimerSubdeviceEntry.getAttribute("subdevice_lock") == "1") and
							(docMux32m.getAttribute("timed") == "1"))):
							timer_device = int(docDeviceEntry.getAttribute("device_number"))
							timer_subdevice = int(docSubdeviceEntry.getAttribute("subdevice_number"))
						else:
							timingSubdevice.Enable(False)
		else:
			timingSubdevice.Enable(False)

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
				if(docMux32m.parentNode.parentNode.parentNode.getAttribute("device_number") != dioList[dioSubdevice.GetSelection()][0]):
					dlg = wx.MessageDialog(self,
							'Use of the operation mode "Streaming Input" is only possible with a\nDIO subdevice on the same device', "Error", wx.OK | wx.ICON_ERROR)
					dlg.ShowModal()
					dlg.Destroy()
					self.Destroy()
					return

			docMux32mAiChannel = docMux32m.getElementsByTagName("mux32m_ai_channel")[0]
			docMux32mAiChannel.firstChild.data = aiChannel.GetValue()

			docMux32mDioDevice.firstChild.data = str(dioList[dioSubdevice.GetSelection()][0])
			docMux32mDioSubdevice.firstChild.data = str(dioList[dioSubdevice.GetSelection()][1])

			# Mark the selected digital i/o subdevice as locked
			for docDioSubdevice in dioList:
				docDioSubdevice[3].setAttribute("subdevice_lock", "0")
			docDioSubdevice = dioSubdevice.GetClientData(dioSubdevice.GetSelection())
			docDioSubdevice.setAttribute("subdevice_lock", "1")
			docDioSubdevice.setAttribute("lock_device", docMux32m.parentNode.parentNode.parentNode.getAttribute("device_number"))
			docDioSubdevice.setAttribute("lock_subdevice", docMux32m.parentNode.getAttribute("subdevice_number"))

			if(timingSubdevice.GetValue() == True):
				docMux32m.setAttribute("timed", "1")

				if(len(docMux32m.getElementsByTagName("mux32m_timer_device")) == 0):
					cnt = treeCtrl.GetChildrenCount(treeMux32m)
					docMux32mTimerDevice = doc.createElement("mux32m_timer_device")
					docMux32mTimerDevice.setAttribute("description", "Timer device")
					docMux32m.insertBefore(docMux32mTimerDevice, docMux32m.getElementsByTagName("mux32s_list")[0])
					docMux32mTimerDeviceTxt = doc.createTextNode(str(timer_device))
					docMux32mTimerDevice.appendChild(docMux32mTimerDeviceTxt)
					treeMux32mTimerDevice = treeCtrl.InsertItemBefore(treeMux32m, cnt - 1, docMux32mTimerDevice.getAttribute("description"))
					treeCtrl.SetPyData(treeMux32mTimerDevice, docMux32mTimerDevice)
					treeCtrl.SetItemText(treeMux32mTimerDevice, docMux32mTimerDeviceTxt.data, 1)

					cnt = treeCtrl.GetChildrenCount(treeMux32m)
					docMux32mTimerSubdevice = doc.createElement("mux32m_timer_subdevice")
					docMux32mTimerSubdevice.setAttribute("description", "Timer subdevice")
					docMux32m.insertBefore(docMux32mTimerSubdevice, docMux32m.getElementsByTagName("mux32s_list")[0])
					docMux32mTimerSubdeviceTxt = doc.createTextNode(str(timer_subdevice))
					docMux32mTimerSubdevice.appendChild(docMux32mTimerSubdeviceTxt)
					treeMux32mTimerSubdevice = treeCtrl.InsertItemBefore(treeMux32m, cnt - 1, docMux32mTimerSubdevice.getAttribute("description"))
					treeCtrl.SetPyData(treeMux32mTimerSubdevice, docMux32mTimerSubdevice)
					treeCtrl.SetItemText(treeMux32mTimerSubdevice, docMux32mTimerSubdeviceTxt.data, 1)
				else:
					docMux32mTimerDevice = docMux32m.getElementsByTagName("mux32m_timer_device")[0]
					docMux32mTimerDevice.firstChild.data = str(timer_device)
					docMux32mTimerSubdevice = docMux32m.getElementsByTagName("mux32m_timer_subdevice")[0]
					docMux32mTimerSubdevice.firstChild.data = str(timer_subdevice)

				# Mark the analog output as locked
				docTimerSubdeviceEntry.setAttribute("subdevice_lock", "1")
				docTimerSubdeviceEntry.setAttribute("lock_device", docMux32m.parentNode.parentNode.parentNode.getAttribute("device_number"))
				docTimerSubdeviceEntry.setAttribute("lock_subdevice", docMux32m.parentNode.getAttribute("subdevice_number"))
			else:
				docMux32m.setAttribute("timed", "0")
				if(len(docMux32m.getElementsByTagName("mux32m_timer_device")) != 0):
					docMux32mTimerDevice = docMux32m.getElementsByTagName("mux32m_timer_device")[0]
					docMux32mTimerSubdevice = docMux32m.getElementsByTagName("mux32m_timer_subdevice")[0]

					id, cookie = treeCtrl.GetFirstChild(treeMux32m)
					while(id.IsOk()):
						if(treeCtrl.GetPyData(id).nodeName == "mux32m_timer_device"):
							treeCtrl.Delete(id)
							break
						id, cookie = treeCtrl.GetNextChild(treeMux32m, cookie)

					id, cookie = treeCtrl.GetFirstChild(treeMux32m)
					while(id.IsOk()):
						if(treeCtrl.GetPyData(id).nodeName == "mux32m_timer_subdevice"):
							treeCtrl.Delete(id)
							break
						id, cookie = treeCtrl.GetNextChild(treeMux32m, cookie)

					docMux32mTimerDevice = docMux32m.removeChild(docMux32mTimerDevice)
					docMux32mTimerDevice.unlink()
					docMux32mTimerSubdevice = docMux32m.removeChild(docMux32mTimerSubdevice)
					docMux32mTimerSubdevice.unlink()

					# Mark the analog output as unlocked
					docTimerSubdeviceEntry.setAttribute("subdevice_lock", "0")

		self.Destroy()

	def GetDIOSubdeviceList(self, docRoot, actDevice, actSubdevice):
		retList = []
		docDeviceList = docRoot.getElementsByTagName("device_list")[0]
		for docDevice in docDeviceList.getElementsByTagName("device_entry"):
			docSubdeviceList = docDevice.getElementsByTagName("subdevice_list")[0]
			for docSubdeviceEntry in docSubdeviceList.getElementsByTagName("subdevice_entry"):
				if((docSubdeviceEntry.getAttribute("subdevice_lock") == "0") or
					(docSubdeviceEntry.getAttribute("subdevice_lock") == "1" and
					docDevice.getAttribute("device_number") == actDevice and
					docSubdeviceEntry.getAttribute("subdevice_number") == actSubdevice)):
					docSubdeviceInfo = docSubdeviceEntry.getElementsByTagName("subdevice_info")[0]
					docSubdeviceType = docSubdeviceInfo.getElementsByTagName("subdevice_type")[0]
					if((string.atoi(string.strip(docSubdeviceType.firstChild.data)) == meDriver.ME_TYPE_DIO) or
						(string.atoi(string.strip(docSubdeviceType.firstChild.data)) == meDriver.ME_TYPE_DO)):
						docSubdeviceNumberChannels = docSubdeviceInfo.getElementsByTagName("subdevice_number_channels")[0]
						retList.append((docDevice.getAttribute("device_number"),
							docSubdeviceEntry.getAttribute("subdevice_number"), string.strip(docSubdeviceNumberChannels.firstChild.data), docSubdeviceEntry))
		return retList
