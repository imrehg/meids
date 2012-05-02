#!/usr/bin/env python
# Copyright (2006) Meilhaus Electronic GmbH.
# All rights reserved.

import wx
import myMainWindow


class MyApp(wx.App):
    """Meilhaus intelligent Device Configuration Utility (ME-iDC).
    """
    def OnInit(self):
        mainWindow = myMainWindow.MyMainWindow(None, -1, "Meilhaus intelligent Device Configuration Utility (ME-iDC)")
        mainWindow.Show(True)
        self.SetTopWindow(mainWindow)
        return True

# Create the main Window and run the main loop
app = MyApp(0)
app.MainLoop()
