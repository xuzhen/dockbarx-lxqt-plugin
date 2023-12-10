#!/usr/bin/python3
#
# Copyright (C) 2023 Xu Zhen

# This file is part of DockbarX LXQt panel plugin.

# DockbarX LXQt panel plugin is free software: you can redistribute it and/or
# modify it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or (at your
# option) any later version.

# DockbarX LXQt panel plugin is distributed in the hope that it will be
# useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General
# Public License for more details.

# You should have received a copy of the GNU General Public License along
# with this file. If not, see <http://www.gnu.org/licenses/>.

from dockbarx.log import *; log_to_file()

import gi
gi.require_version("Gtk", "3.0")
from gi.repository import Gtk, GLib, Gio, Gdk
import sys
import dbus
import signal
import dockbarx.dockbar
import weakref

class DockBarApplet(Gtk.Window):
    def __init__ (self, app, orient, size):
        Gtk.Window.__init__(self)
        self.set_skip_taskbar_hint(True)
        self.set_skip_pager_hint(True)
        self.set_accept_focus(False)
        self.set_decorated(False)
        self.set_resizable(False)
        self.app_r = weakref.ref(app);
        self.wid = None
        self.size = size
        self.orient = orient
        #self.get_settings().connect("notify::gtk-theme-name",self.theme_changed)
        self._realize_sid = self.connect("realize", self.__on_realize)
        self.dockbar = dockbarx.dockbar.DockBar(self)
        self.dockbar.set_orient(orient)
        self.dockbar.load()
        self.dockbar.set_size(size)
        self.dockbar.set_max_size(32767)
        self.add(self.dockbar.get_container())

    def __on_realize(self, widget):
        self.disconnect(self._realize_sid)
        self._realize_sid = None
        self.wid = self.get_window().get_xid()
        self.connect("size-allocate", self.__on_size_allocate)
        self.app_r().announce_ready(self.wid)

    def __on_size_allocate(self, widget, allocation):
        self.app_r().announce_size_changed(allocation.width, allocation.height)
    
    def reload(self):
        self.dockbar.reload()

    def set_orient(self, orient):
        self.orient = orient
        self.dockbar.set_orient(orient)
        self.dockbar.reload()

    def set_size(self, size):
        self.size = size
        self.dockbar.set_size(size)
        self.queue_resize()

    def readd_container(self, container):
        self.add(container)


class LXQtApplet(Gtk.Application):
    def __init__(self, *args, **kwargs):
        super().__init__(*args, application_id="org.dockbarx.LXQtApplet", flags=Gio.ApplicationFlags.HANDLES_COMMAND_LINE, **kwargs)
        self.window = None
        self.orient = None
        self.size = None
        self.add_main_option("orient", ord("o"), GLib.OptionFlags.IN_MAIN, GLib.OptionArg.STRING, "Orient", None)
        self.add_main_option("size", ord("s"), GLib.OptionFlags.IN_MAIN, GLib.OptionArg.INT, "Size", None)

    def do_startup(self):
        Gtk.Application.do_startup(self)
        self.register_dbus()
        
    def do_activate(self):
        if not self.window:
            self.window = DockBarApplet(self, self.orient, self.size)
            self.add_window(self.window)
            GLib.unix_signal_add(GLib.PRIORITY_DEFAULT, signal.SIGINT, self.window.destroy)
        self.window.present()

    def do_command_line(self, command_line):
        options = command_line.get_options_dict()
        options = options.end().unpack()

        if "orient" in options:
            self.orient = str(options["orient"])
            if self.orient not in ("down", "up", "left", "right"):
                logger.error("Invalid orient")
                return 1
        else:
            self.orient = "down"

        if "size" in options:
            self.size = int(options["size"])
            if self.size <= 0:
                logger.error("Invalid size")
                return 1
        else:
            self.size = 32

        self.activate()
        return 0

    def register_dbus(self):
        dbus_xml = \
            "<node>" \
              "<interface name='org.dockbarx.LXQtApplet'>" \
                "<method name='Reload'/>" \
                "<method name='SetSize'>" \
                  "<arg type='i' name='size' direction='in'/>" \
                "</method>" \
                "<method name='SetOrient'>" \
                  "<arg type='s' name='orient' direction='in'/>" \
                "</method>" \
                "<signal name='Ready'>" \
                  "<arg type='u' name='wid'/>" \
                "</signal>" \
                "<signal name='SizeChanged'>" \
                  "<arg type='ai' name='size' direction='out'/>" \
                "</signal>" \
              "</interface>" \
            "</node>"
        info = Gio.DBusNodeInfo.new_for_xml(dbus_xml)
        iface = info.lookup_interface("org.dockbarx.LXQtApplet")
        dbus = self.get_dbus_connection()
        dbus_path = self.get_dbus_object_path()
        dbus.register_object(dbus_path, iface, self.dbus_method_call, None, None);

    def announce_ready(self, wid):
        self.emit_signal("Ready", GLib.Variant.new_uint32(wid));
   
    def announce_size_changed(self, width, height):
        array = GLib.Variant.new_array(GLib.VariantType("i"), (GLib.Variant.new_int32(width), GLib.Variant.new_int32(height)))
        self.emit_signal("SizeChanged", array);

    def emit_signal(self, name, args):
        conn = self.get_dbus_connection()
        conn.emit_signal(None, "/org/dockbarx/LXQtApplet", "org.dockbarx.LXQtApplet", name, GLib.Variant.new_tuple(args))
 

    def dbus_method_call(self, connection, sender, object_path, interface_name, method_name, parameters, invocation):
        ret = None
        if self.window is None:
            err = Gio.DBusError.FAILED
            err_message = "Not ready yet"
        elif self.window.wid is None:
            err = Gio.DBusError.ACCESS_DENIED
            err_message = "Not realized yet"
        elif method_name == "Reload":
            self.window.reload()
            ret = GLib.Variant.new_tuple()
        elif method_name == "SetSize":
            if len(parameters) == 1 and type(parameters[0]) == int:
                self.window.set_size(parameters[0])
                ret = GLib.Variant.new_tuple()
            else:
                err = Gio.DBusError.INVALID_ARGS
                err_message = "Invalid argument"
        elif method_name == "SetOrient":
            if len(parameters) == 1 and parameters[0] in ("down", "up", "left", "right"):
                self.window.set_orient(parameters[0])
                ret = GLib.Variant.new_tuple()
            else:
                err = Gio.DBusError.INVALID_ARGS
                err_message = "Invalid argument"
        else:
            err = Gio.DBusError.UNKNOWN_METHOD
            err_message = "No such method: %s" % method_name

        if ret is not None:
            invocation.return_value(ret)
        else:
            invocation.return_error_literal(err.quark(), err, err_message)


if __name__ == '__main__':
    LXQtApplet().run(sys.argv)
