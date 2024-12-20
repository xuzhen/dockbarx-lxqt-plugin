#!/usr/bin/python3
#
# Copyright (C) 2023-2024 Xu Zhen

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
import json

class DockBarApplet(Gtk.Window):
    def __init__ (self, app, orient, size, max_size, icon_theme, scaling_factor):
        Gtk.Window.__init__(self)
        self.set_skip_taskbar_hint(True)
        self.set_skip_pager_hint(True)
        self.set_accept_focus(False)
        self.set_decorated(False)
        self.set_resizable(False)
        self.set_app_paintable(True)
        self.gdk_screen = Gdk.Screen.get_default()
        visual = self.gdk_screen.get_rgba_visual()
        if visual is None: visual = self.gdk_screen.get_system_visual()
        self.set_visual(visual)
        self.app_r = weakref.ref(app);
        self.color_pattern = None
        self.image_pattern = None
        self.wid = None
        self.scaling_factor = scaling_factor
        self.size = size
        self.scaled_size = round(size * scaling_factor)
        self.orient = orient
        self.prev_alloc = (self.scaled_size, self.scaled_size)
        self.set_icon_theme(icon_theme, reload=False)
        #self.get_settings().connect("notify::gtk-theme-name",self.theme_changed)
        self._realize_sid = self.connect("realize", self.__on_realize)
        self.dockbar = dockbarx.dockbar.DockBar(self)
        self.dockbar.set_orient(orient)
        self.dockbar.load()
        self.dockbar.set_size(self.scaled_size)
        self.overflow_manager = dockbarx.dockbar.GroupList.manage_size_overflow
        self.set_max_size(max_size)
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
            if len(self.dockbar.groups.get_shown_groups()) == 0:
                allocation.height = self.scaled_size
                allocation.width = 0
                self.size_allocate(allocation);
            elif allocation.height > self.scaled_size or allocation.width == 1:
                allocation.height = self.scaled_size
                allocation.width = self.prev_alloc[0]
                self.size_allocate(allocation);
                return
        else:
            if len(self.dockbar.groups.get_shown_groups()) == 0:
                allocation.width = self.scaled_size
                allocation.height = 0
                self.size_allocate(allocation);
            elif allocation.width > self.scaled_size or allocation.height == 1:
                allocation.width = self.scaled_size
                allocation.height = self.prev_alloc[1]
                self.size_allocate(allocation);
                return
        self.prev_alloc = (allocation.width, allocation.height)
        ratio = self.scaling_factor
        self.app_r().announce_size_changed(round(allocation.width / ratio), round(allocation.height / ratio))

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
        self.scaled_size = round(size * self.scaling_factor)
        self.dockbar.set_size(self.scaled_size)
        self.queue_resize()
        return True

    def set_max_size(self, size):
        if size <= 0:
            dockbarx.dockbar.GroupList.manage_size_overflow = lambda x: None
            self.dockbar.set_max_size(32767)
        else:
            dockbarx.dockbar.GroupList.manage_size_overflow = self.overflow_manager
            self.dockbar.set_max_size(int(size * self.scaling_factor))
        self.queue_resize()
        self.queue_draw()
        return True

    def set_background(self, color, image, offsetX, offsetY, panelWidth, panelHeight):
        if color != "":
            if color[0] == "{":
                try:
                    args = json.loads(color)
                except:
                    logger.error("Failed to parse gradient: " + color)
                    return False
                self.color_pattern = cairo.LinearGradient(args["x1"], args["y1"], args["x2"], args["y2"])
                for pair in args["stops"]:
                    rgba = Gdk.RGBA()
                    if not rgba.parse(pair[1]):
                        logger.error("Failed to parse color: " + pair[1])
                        return False
                    self.color_pattern.add_color_stop_rgba(pair[0], rgba.red, rgba.green, rgba.blue, rgba.alpha)
                matrix = cairo.Matrix(xx=1.0/panelWidth/self.scaling_factor, yy=1.0/panelHeight/self.scaling_factor, x0=offsetX/panelWidth, y0=offsetY/panelHeight)
                self.color_pattern.set_matrix(matrix)
            else:
                rgba = Gdk.RGBA()
                if not rgba.parse(color):
                    logger.error("Failed to parse color: " + color)
                    return False
                if Gdk.Screen.get_default().get_rgba_visual() is None:
                    rgba.alpha = 1
                self.color_pattern = cairo.SolidPattern(rgba.red, rgba.green, rgba.blue, rgba.alpha)
        else:
            self.color_pattern = None
        if image != "":
            try:
                pixbuf = GdkPixbuf.Pixbuf.new_from_file(image)
            except:
                logger.error("Failed to load image: " + image)
                self.image_pattern = None
            else:
                surface = Gdk.cairo_surface_create_from_pixbuf(pixbuf, 0)
                self.image_pattern = cairo.SurfacePattern(surface)
                self.image_pattern.set_extend(cairo.EXTEND_REPEAT)
                matrix = cairo.Matrix(xx=1.0/self.scaling_factor, yy=1.0/self.scaling_factor, x0=offsetX, y0=offsetY)
                self.image_pattern.set_matrix(matrix)
        else:
            self.image_pattern = None
        self.queue_draw()
        return True

    def set_icon_theme(self, name, reload=True):
        if name is None or name == "":
            Gtk.Settings.get_default().reset_property("gtk-icon-theme-name")
        else:
            Gtk.Settings.get_default().set_property("gtk-icon-theme-name", name)
        if reload:
            self.dockbar.reload()
        return True

    def set_scaling_factor(self, factor):
        if factor <= 0:
            return False
        if self.scaling_factor != factor:
            self.scaling_factor = factor
            self.scaled_size = round(self.size * factor)
            self.dockbar.set_size(self.scaled_size)
            self.queue_resize()
        return True

    def on_draw (self, widget, ctx):
        a = widget.get_allocation()
        if self.color_pattern is None and self.image_pattern is None:
            context = widget.get_style_context()
            Gtk.render_background(context, ctx, a.x, a.y, a.width, a.height)
            return
        ctx.save()
        ctx.set_antialias(cairo.ANTIALIAS_NONE)
        ctx.set_operator(cairo.OPERATOR_OVER)
        ctx.rectangle(a.x, a.y, a.width, a.height)
        ctx.clip()
        if self.color_pattern:
            ctx.set_source(self.color_pattern)
            ctx.paint()
        if self.image_pattern:
            ctx.set_source(self.image_pattern)
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
        self.max_size = None
        self.icon_theme = None
        self.scaling_factor = None
        self.iface_info = None
        self.add_main_option("orient", ord("o"), GLib.OptionFlags.IN_MAIN, GLib.OptionArg.STRING, "Orient", None)
        self.add_main_option("size", ord("s"), GLib.OptionFlags.IN_MAIN, GLib.OptionArg.INT, "Size", None)
        self.add_main_option("max", ord("m"), GLib.OptionFlags.IN_MAIN, GLib.OptionArg.INT, "Max Size", None)
        self.add_main_option("icon", ord("i"), GLib.OptionFlags.IN_MAIN, GLib.OptionArg.STRING, "Icon theme", None)
        self.add_main_option("factor", ord("f"), GLib.OptionFlags.IN_MAIN, GLib.OptionArg.DOUBLE, "Scaling factor", None)

    def do_startup(self):
        Gtk.Application.do_startup(self)
        self.register_dbus()
        
    def do_activate(self):
        if not self.window:
            self.window = DockBarApplet(self, self.orient, self.size, self.max_size, self.icon_theme, self.scaling_factor)
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

        if "max" in options:
            self.max_size = int(options["max"])
        else:
            self.max_size = -1

        if "icon" in options:
            self.icon_theme = str(options["icon"])

        if "factor" in options:
            self.scaling_factor = float(options["factor"])
        else:
            self.scaling_factor = 1.0

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
                "<method name='SetMaxSize'>" \
                  "<arg type='i' name='size' direction='in'/>" \
                "</method>" \
                "<method name='SetOrient'>" \
                  "<arg type='s' name='orient' direction='in'/>" \
                "</method>" \
                "<method name='SetBackground'>" \
                  "<arg type='s' name='color' direction='in'/>" \
                  "<arg type='s' name='image' direction='in'/>" \
                  "<arg type='i' name='offsetX' direction='in'/>" \
                  "<arg type='i' name='offsetY' direction='in'/>" \
                  "<arg type='i' name='panelWidth' direction='in'/>" \
                  "<arg type='i' name='panelHeight' direction='in'/>" \
                "</method>" \
                "<method name='SetIconTheme'>" \
                  "<arg type='s' name='name' direction='in'/>" \
                "</method>" \
                "<method name='SetScalingFactor'>" \
                  "<arg type='d' name='factor' direction='in'/>" \
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
            elif method_name == "SetMaxSize":
                if self.window.set_max_size(parameters[0]):
                    ret = GLib.Variant.new_tuple()
            elif method_name == "SetOrient":
                if self.window.set_orient(parameters[0]):
                    ret = GLib.Variant.new_tuple()
            elif method_name == "SetBackground":
                if self.window.set_background(parameters[0], parameters[1], parameters[2], parameters[3], parameters[4], parameters[5]):
                    ret = GLib.Variant.new_tuple()
            elif method_name == "SetIconTheme":
                if self.window.set_icon_theme(parameters[0]):
                    ret = GLib.Variant.new_tuple()
            elif method_name == "SetScalingFactor":
                if self.window.set_scaling_factor(parameters[0]):
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
