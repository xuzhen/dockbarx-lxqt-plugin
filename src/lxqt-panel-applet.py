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
from gi.repository import Gtk, GLib, Gio, Gdk, GdkPixbuf
import sys
import dbus
import signal
import dockbarx.dockbar
import weakref
import cairo

class DockBarApplet(Gtk.Window):
    def __init__ (self, app, orient, size):
        Gtk.Window.__init__(self)
        self.set_skip_taskbar_hint(True)
        self.set_skip_pager_hint(True)
        self.set_accept_focus(False)
        self.set_decorated(False)
        self.set_resizable(False)
        self.set_app_paintable(True)
        gtk_screen = Gdk.Screen.get_default()
        visual = gtk_screen.get_rgba_visual()
        if visual is None: visual = gtk_screen.get_system_visual()
        self.set_visual(visual)
        self.app_r = weakref.ref(app);
        self.pattern = None
        self.wid = None
        self.size = size
        self.orient = orient
        self.prev_alloc = (size, size)
        #self.get_settings().connect("notify::gtk-theme-name",self.theme_changed)
        self._realize_sid = self.connect("realize", self.__on_realize)
        self.dockbar = dockbarx.dockbar.DockBar(self)
        self.dockbar.set_orient(orient)
        self.dockbar.load()
        self.dockbar.set_size(size)
        self.dockbar.set_max_size(32767)
        self.add(self.dockbar.get_container())
        self.connect("draw", self.on_draw)
        self.block_autohide_patch()

    def __on_realize(self, widget):
        self.disconnect(self._realize_sid)
        self._realize_sid = None
        win = self.get_window()
        # ensure the window is not in WithdrawnState
        win.show_unraised()
        self.wid = win.get_xid()
        self.connect("size-allocate", self.__on_size_allocate)
        self.app_r().announce_ready(self.wid)

    def __on_size_allocate(self, widget, allocation):
        # bug? allocated a 640x480 or 1 pixel width size area sometimes.
        if self.orient in ("up", "down"):
            if allocation.height > self.size or (allocation.width == 1 and len(self.dockbar.groups) > 0):
                allocation.height = self.size
                allocation.width = self.prev_alloc[0]
                self.size_allocate(allocation);
                return
        else:
            if allocation.width > self.size or (allocation.height == 1 and len(self.dockbar.groups) > 0):
                allocation.width = self.size
                allocation.height = self.prev_alloc[1]
                self.size_allocate(allocation);
                return
        self.prev_alloc = (allocation.width, allocation.height)
        self.app_r().announce_size_changed(allocation.width, allocation.height)

    def reload(self):
        self.dockbar.reload()

    def set_orient(self, orient):
        if orient not in ("down", "up", "left", "right"):
            return False
        self.orient = orient
        self.dockbar.set_orient(orient)
        self.dockbar.reload()
        return True;

    def set_size(self, size):
        if size <= 0:
            return False
        self.size = size
        self.dockbar.set_size(size)
        self.queue_resize()
        return True

    def set_image(self, image, offsetX, offsetY):
        if image == "":
            return False;
        try:
            pixbuf = GdkPixbuf.Pixbuf.new_from_file(image)
        except:
            logger.error("Failed to load image " + image)
            self.pattern = None
        else:
            surface = Gdk.cairo_surface_create_from_pixbuf(pixbuf, 0)
            self.pattern = cairo.SurfacePattern(surface)
            self.pattern.set_extend(cairo.EXTEND_REPEAT)
            matrix = cairo.Matrix(x0=offsetX, y0=offsetY)
            self.pattern.set_matrix(matrix)
        self.queue_draw()
        return True

    def set_color(self, color):
        rgba = Gdk.RGBA()
        if not rgba.parse(color):
            return False;
        if Gdk.Screen.get_default().get_rgba_visual() is None:
            rgba.alpha = 1
        self.pattern = cairo.SolidPattern(rgba.red, rgba.green, rgba.blue, rgba.alpha)
        self.queue_draw()
        return True

    def on_draw (self, widget, ctx):
        a = widget.get_allocation()
        if self.pattern is None:
            context = widget.get_style_context()
            Gtk.render_background(context, ctx, a.x, a.y, a.width, a.height)
            return
        ctx.save()
        ctx.set_antialias(cairo.ANTIALIAS_NONE)
        ctx.set_operator(cairo.OPERATOR_SOURCE)
        ctx.rectangle(a.x, a.y, a.width, a.height)
        ctx.clip()
        ctx.set_source(self.pattern)
        ctx.paint()
        ctx.restore()

    def readd_container(self, container):
        self.add(container)

    # Terrible monkey patching... but this allows inhibiting autohide!
    def block_autohide_patch(self):
        import dockbarx.common as com
        def new_setattr (obj, name, value):
            super(com.Globals, obj).__setattr__(name, value)
            if name == "gtkmenu":
                self.app_r().announce_popup(value is not None)
            elif name == "shown_popup":
                self.app_r().announce_popup(value() is not None)
        com.Globals.__setattr__ = new_setattr


class LXQtApplet(Gtk.Application):
    def __init__(self, *args, **kwargs):
        super().__init__(*args, application_id="org.dockbarx.LXQtApplet", flags=Gio.ApplicationFlags.HANDLES_COMMAND_LINE, **kwargs)
        self.window = None
        self.orient = None
        self.size = None
        self.iface_info = None
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
                "<method name='SetBgImage'>" \
                  "<arg type='s' name='image' direction='in'/>" \
                  "<arg type='i' name='offsetX' direction='in'/>" \
                  "<arg type='i' name='offsetY' direction='in'/>" \
                "</method>" \
                "<method name='SetBgColor'>" \
                  "<arg type='s' name='color' direction='in'/>" \
                "</method>" \
                "<signal name='Ready'>" \
                  "<arg type='u' name='wid'/>" \
                "</signal>" \
                "<signal name='SizeChanged'>" \
                  "<arg type='ai' name='size'/>" \
                "</signal>" \
                "<signal name='Popup'>" \
                  "<arg type='b' name='shown'/>" \
                "</signal>" \
              "</interface>" \
            "</node>"
        info = Gio.DBusNodeInfo.new_for_xml(dbus_xml)
        self.iface_info = info.lookup_interface("org.dockbarx.LXQtApplet")
        dbus = self.get_dbus_connection()
        dbus_path = self.get_dbus_object_path()
        dbus.register_object(dbus_path, self.iface_info, self.dbus_method_call, None, None);

    def announce_ready(self, wid):
        self.emit_signal("Ready", GLib.Variant.new_uint32(wid));
   
    def announce_size_changed(self, width, height):
        array = GLib.Variant.new_array(GLib.VariantType("i"), (GLib.Variant.new_int32(width), GLib.Variant.new_int32(height)))
        self.emit_signal("SizeChanged", array);

    def announce_popup(self, shown):
        self.emit_signal("Popup", GLib.Variant.new_boolean(shown));

    def emit_signal(self, name, args):
        conn = self.get_dbus_connection()
        conn.emit_signal(None, "/org/dockbarx/LXQtApplet", "org.dockbarx.LXQtApplet", name, GLib.Variant.new_tuple(args))
 

    def dbus_method_call(self, connection, sender, object_path, interface_name, method_name, parameters, invocation):
        ret = None
        err = None
        if self.window is None:
            err = Gio.DBusError.FAILED
            err_message = "Not ready yet"
        elif self.window.wid is None:
            err = Gio.DBusError.ACCESS_DENIED
            err_message = "Not realized yet"
        elif self.iface_info.lookup_method(method_name) is None:
            err = Gio.DBusError.UNKNOWN_METHOD
            err_message = "No such method: %s" % method_name
        else:
            if method_name == "Reload":
                self.window.reload()
                ret = GLib.Variant.new_tuple()
            elif method_name == "SetSize":
                if self.window.set_size(parameters[0]):
                    ret = GLib.Variant.new_tuple()
            elif method_name == "SetOrient":
                if self.window.set_orient(parameters[0]):
                    ret = GLib.Variant.new_tuple()
            elif method_name == "SetBgImage":
                if self.window.set_image(parameters[0], parameters[1], parameters[2]):
                    ret = GLib.Variant.new_tuple()
            elif method_name == "SetBgColor":
                if self.window.set_color(parameters[0]):
                    ret = GLib.Variant.new_tuple()
            else:
                err = Gio.DBusError.UNKNOWN_METHOD
                err_message = "Not implemented yet: %s" % method_name
            if ret is None and err is None:
                err = Gio.DBusError.INVALID_ARGS
                err_message = "Invalid arguments"

        if ret is not None:
            invocation.return_value(ret)
        else:
            invocation.return_error_literal(err.quark(), err, err_message)


if __name__ == '__main__':
    LXQtApplet().run(sys.argv)
