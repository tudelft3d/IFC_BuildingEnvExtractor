# this file contains all the functions related to extending the main interface functionality
import tkinter
from tkinter import filedialog
from pathlib import Path
import os
import json

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

def toggleEnableDiv(toggleDict, settings):

    widget = toggleDict["message_div_objects"]
    default_toggle = toggleDict["useDefault_toggle"]
    proxy_toggle = toggleDict["igoreproxy_toggle"]

    if  settings.div.custom_enabled.get() == 1:
        widget['state'] = tkinter.NORMAL
        widget['bg'] = 'SystemWindow'
        widget['fg'] = 'SystemWindowText'

        default_toggle["state"] = tkinter.DISABLED
        proxy_toggle["state"] = tkinter.DISABLED

    elif settings.div.custom_enabled.get() == 0:
        widget['state'] = tkinter.DISABLED
        widget['bg'] = "#F0F0F0"
        widget['fg'] = "#707070"

        default_toggle["state"] = tkinter.NORMAL
        proxy_toggle["state"] = tkinter.NORMAL

        updateDivMessage(toggleDict, settings)
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

def toggleManualFootprintEleve(toggleDict, settings):
    for toggle_name, toggle in toggleDict.items():
        if toggle_name == "entry_footprint" or \
            toggle_name == "button_plus_footprint" or \
            toggle_name ==  "button_min_footprint" or \
            toggle_name == "button_unit_toggle":

            if settings.footprint.find_footprint_elev.get():
                toggle['state'] = tkinter.DISABLED
            else:
                toggle['state'] = tkinter.NORMAL
    return

def checkActiveToggles(toggleDict, settings):
    toggleMakeFootprint(toggleDict["make_footprint"], settings),
    toggleMakeRoofOutline(toggleDict["make_roofprint"], settings),
    toggleMakeFootprintBased(toggleDict["make_footprint_based"], settings),
    toggleMakeInterior(toggleDict["make_interior"], settings),
    toggleIgnoreIsExternal(toggleDict["make_ignore_IsExternal"], settings)
    toggleManualFootprintEleve(toggleDict, settings)
    toggleEnableDiv(toggleDict, settings)
    updateDivMessage(toggleDict, settings)
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

def updateDivMessage(toggleDict, settings):
    message_window = toggleDict["message_div_objects"]
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

def isConfigJSON(json_file):
    if "Tolerances" not in json_file and \
        "Voxel" not in json_file and \
        "IFC" not in json_file and \
        "JSON" not in json_file and \
        "Generate report" not in json_file and \
        "LoD output" not in json_file and \
        "Output format" not in json_file and \
        "Filepaths" not in json_file:
            return False
    return True

def load_custom_config(toggleDict, settings):
    json_filepath = filedialog.askopenfilenames(
        filetypes=[("ConfigJSON", ".json")],
        defaultextension=".json")

    if len(json_filepath) == 0:
        return

    if not os.path.exists(json_filepath[0]):
        tkinter.messagebox.showerror("Processing Error",
                                     "Error: cannot find submitted config file")
        return

    load_config(json_filepath[0], toggleDict, settings)
    return

def load_config(path, toggleDict, settings):
    if not os.path.exists(path):
        tkinter.messagebox.showerror("Processing Error",
                                     "Error: cannot find default config files")
        return

    with open(path, 'r') as file:
        json_data = json.load(file)

    settings.set_from_json(json_data)

    # set the ui
    checkActiveToggles(toggleDict, settings)



    return

def populateConfigJson(load_config_menu, toggleDict, settings):
    config_folder = "./default_data"
    if os.path.isdir(config_folder):
        for file in os.scandir(config_folder):

            max_files = 10
            current_file_count = 0

            if file.is_file() and file.name.lower().endswith(".json"):
                pathstring = "./default_data/" + file.name

                with open(pathstring, 'r') as json_file:
                    json_data = json.load(json_file)

                if not isConfigJSON(json_data):
                    continue

                config_name = file.name
                if "Alias" in json_data:
                    if isinstance(json_data["Alias"], str):
                        config_name = json_data["Alias"]

                load_config_menu.add_command(label=config_name,
                                             command=lambda p=pathstring:
                                             load_config(p, toggleDict, settings)
                                             )
                current_file_count += 1
                if current_file_count >= max_files:
                    break
    return

def summarywindow(settings):
    main_window = tkinter.Tk()
    main_window.geometry('500x590')
    main_window.resizable(1, 0)
    main_window.title("IfcEnvExtactor summary")

    settings.dump_to_json(False, False)
    # json to file
    json_string = getNestedJsonString(settings.json)

    message_json_sum = tkinter.Text(main_window, width=300, bg="#F0F0F0")
    message_json_sum.insert(tkinter.INSERT, json_string)
    message_json_sum.pack(fill='both', expand=True, padx=5, pady=10)

    return

def getNestedJsonString(json_data, indent = ""):
    summary_string = ""
    for key, value in json_data.items():
        if isinstance(value, dict):
            summary_string += str(key) + ":\n"
            summary_string += getNestedJsonString(value, "\t")
        else:
            summary_string +=indent + str(key) + ": " + str(value) + "\n"
    return summary_string