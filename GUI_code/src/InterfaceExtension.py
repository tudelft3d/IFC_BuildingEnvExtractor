# this file contains all the functions related to extending the main interface functionality
import tkinter
from tkinter import filedialog
from pathlib import Path

class Tooltip:
    def __init__(self, widget, text):
        self.widget = widget
        self.text = text
        self.tooltip_window = None
        self.after_id = None
        self.widget.bind("<Enter>", self.on_enter)
        self.widget.bind("<Leave>", self.on_leave)
        return

    def on_enter(self, event=None):
        self.after_id = self.widget.after(500, self.show_tooltip)
        return

    def on_leave(self, event=None):
        # Cancel tooltip if mouse leaves before it shows
        if self.after_id:
            self.widget.after_cancel(self.after_id)
            self.after_id = None
        self.hide_tooltip()
        return

    def show_tooltip(self):
        if not self.tooltip_window:
            x, y, _, _ = self.widget.bbox("insert")
            x += self.widget.winfo_rootx() + 15
            y += self.widget.winfo_rooty() - 20
            self.tooltip_window = tkinter.Toplevel(self.widget)
            self.tooltip_window.wm_overrideredirect(True)
            self.tooltip_window.wm_geometry(f"+{x}+{y}")
            label = tkinter.Label(self.tooltip_window, text=self.text, background="white", borderwidth=1, relief="solid")
            label.pack()
        return

    def hide_tooltip(self):
        if self.tooltip_window:
            self.tooltip_window.destroy()
            self.tooltip_window = None
        return

def toggleEnableDiv(widget, default_toggle, proxy_toggle, settings):

    if  widget['state'] == "disabled":
        widget['state'] = tkinter.NORMAL
        widget['bg'] = 'SystemWindow'
        widget['fg'] = 'SystemWindowText'

        default_toggle["state"] = tkinter.DISABLED
        proxy_toggle["state"] = tkinter.DISABLED
    else:
        widget['state'] = tkinter.DISABLED
        widget['bg'] = "#F0F0F0"
        widget['fg'] = "#707070"

        default_toggle["state"] = tkinter.NORMAL
        proxy_toggle["state"] = tkinter.NORMAL

        updateDivMessage(widget, settings)
    return

def toggleMakeInterior(interior_widget, settings):
    # the relevant bools:
    rel_lod = {settings.lod.lod02.get(),
                    settings.lod.lod03.get(),
                    settings.lod.lod12.get(),
                    settings.lod.lod22.get(),
                    settings.lod.lod32.get(),
                    settings.lod.lod50.get()}

    if any ( b == True for b in rel_lod):
        interior_widget['state'] = tkinter.NORMAL
    else:
        interior_widget['state'] = tkinter.DISABLED
    return

def toggleIgnoreIsExternal(ignore_IsExternal_widget, settings):
    # the relevant bools:
    rel_lod = {settings.lod.lod00.get(), settings.lod.lod02.get(), settings.lod.lod03.get(),
               settings.lod.lod10.get(), settings.lod.lod12.get(), settings.lod.lod13.get(),
               settings.lod.lod22.get(), settings.lod.lode1.get(), settings.lod.lod32.get(),
               settings.lod.lod50.get(), settings.lod.lod40.get()}

    if any(b == True for b in rel_lod):
        ignore_IsExternal_widget['state'] = tkinter.NORMAL
    else:
        ignore_IsExternal_widget['state'] = tkinter.DISABLED
    return

def toggleMakeFootprint(footprint_widges, settings):
    # the relevant bools:
    rel_lod = {settings.lod.lod02.get(),
               settings.lod.lod03.get(),
               settings.lod.lod04.get(),}

    if any ( b == True for b in rel_lod):
        footprint_widges['state'] = tkinter.NORMAL
    else:
        footprint_widges['state'] = tkinter.DISABLED
    return

def toggleMakeRoofOutline(roofPrint_widges, settings):
    # the relevant bools:
    rel_lod = {settings.lod.lod02.get()}

    if any ( b == True for b in rel_lod):
        roofPrint_widges['state'] = tkinter.NORMAL
    else:
        roofPrint_widges['state'] = tkinter.DISABLED
    return

def toggleMakeFootprintBased(footprint_widges, settings):
    # the relevant bools:
    rel_lod = {settings.lod.lod12.get(),
                    settings.lod.lod13.get(),
                    settings.lod.lod22.get()}

    if any ( b == True for b in rel_lod):
        footprint_widges['state'] = tkinter.NORMAL
    else:
        footprint_widges['state'] = tkinter.DISABLED
    return

def toggleManualFootprintEleve(detect_footprint_elev_widges, settings):
    for widged in detect_footprint_elev_widges:
        if settings.footprint.find_footprint_elev.get():
            widged['state'] = tkinter.DISABLED
        else:
            widged['state'] = tkinter.NORMAL
    return

def browse_(box, is_folder, window, initial_file):
    folder_path = ""

    if (not is_folder):
        folder_path =  filedialog.askopenfilenames(
            filetypes=[("IFC file", ".ifc")],
            defaultextension=".ifc"
        )
    else:
        # get the inital filename
        initial_file_name =  Path(initial_file).name
        folder_path = filedialog.asksaveasfilename(
            filetypes=[("JSON file", ".json"), ("CityJSON file", ".city.json")],
            defaultextension="city.json",
            initialfile=Path(initial_file_name).stem + ".json"
        )

    if len(folder_path) == 0:
        return

    box.delete(0, tkinter.END)
    box.insert(0, folder_path)
    window.focus_force()
    return

def increment(value_field, increment_value):
    try:
        float(value_field.get())
    except:
        value_field.delete(0, tkinter.END)
        value_field.insert(0, "0")

    incremented_Value = round(float(value_field.get()) + increment_value, 2)
    value_field.delete(0, tkinter.END)
    value_field.insert(0, incremented_Value)
    return

def decrement(value_field, increment_value):
    try:
        float(value_field.get())
    except:
        value_field.delete(0, tkinter.END)
        value_field.insert(0, "0")

    incremented_Value = round(float(value_field.get()) - increment_value, 2)
    value_field.delete(0, tkinter.END)
    value_field.insert(0, incremented_Value)
    return

def updateDivMessage(message_window, settings):
    message_window['state'] = tkinter.NORMAL

    if settings.div.use_default.get():
        message_window.delete('1.0', tkinter.END)
        message_window.insert(tkinter.INSERT, settings.getDefaultDivObjects() + "\t")
    else:
        message_window.delete('1.0', tkinter.END)
    if not settings.div.ignore_proxy.get():
        message_window.insert(tkinter.END, "IfcBuildingElementProxy")
    message_window['state'] = tkinter.DISABLED
    return

def makeUnitWindow(frame_location, unit_variable):
    unit_options = ["m", "cm", "mm"]
    if unit_variable.get() not in unit_options:
        unit_variable.set(unit_options[0])

    entry_unit = tkinter.Menubutton(
        frame_location,
        textvariable=unit_variable,
        relief="raised",
        borderwidth=1.5,
        highlightthickness=0,
        direction="below")

    # Add options to the Menu
    menu = tkinter.Menu(entry_unit, tearoff=0)
    for option in unit_options:
        menu.add_command(label=option, command=lambda value=option: unit_variable.set(value))

    # Attach the menu to the button
    entry_unit.configure(menu=menu)
    return entry_unit