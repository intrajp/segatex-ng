# consumeservice.py
# consumes a method in a service on the dbus

import dbus,sys
# not to show systrace when error
#sys.tracebacklimit=0

bus = dbus.SystemBus()
setroubleshootservice = bus.get_object('org.fedoraprojcet.Setroubleshootd', '/org/fedoraproject/Setroubleshootd')
#hello = setroubleshootservice.get_dbus_method('start', 'org.fedoraproject.SetroubleshootdIface')

#print hello()
