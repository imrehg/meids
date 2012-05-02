# Copyright (2004) Meilhaus Electronic GmbH.
# All rights reserved.

import wx
from wx.html import *
import sys


class helpDialog(wx.Frame):
	"""Configuration dialog to add a demux device."""
	def __init__(self, parent, doc):
		wx.Frame.__init__(self, parent, -1, "Meilhaus Driver System configuration utility help", size = wx.Size(800, 600))

		# Determine the working directory
		if sys.path[0]:
			self.rootPath = sys.path[0] + '/'
		else:
			self.rootPath = ''

		self.SetIcon(wx.Icon(self.rootPath + 'bmp/MEIcon.xpm', wx.BITMAP_TYPE_XPM))

		w = HtmlWindow(self)
		w.LoadPage(doc)
