# this file contains all the functions related to extending the main interface functionality
import tkinter
from tkinter import filedialog, ttk
from pathlib import Path
import copy
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

def populateSettings():
    if not os.path.isdir("./config/"):
        os.mkdir("./config")

    default_settings = {
        "defaultConfigPath" : "./default_data/",
        "extractorLoc" : "./binary/"
    }

    json_str = json.dumps(default_settings)
    with open("./config/settings.json", "w") as f:
        f.write(json_str)

    return
def loadMem(preferences, is_flexible = True):
    settings_path = "./config/settings.json"
    if not os.path.exists(settings_path):
        populateSettings()

    with open(settings_path, "r") as f:
        settings_json = json.load(f)

        if "defaultConfigPath" in settings_json:
            dConfigPath = settings_json["defaultConfigPath"]
            preferences.preSet_path = os.path.abspath(dConfigPath)
        if "extractorLoc" in settings_json:
            appPath = settings_json["extractorLoc"]
            if not os.path.isdir(appPath):
                tkinter.messagebox.showerror("Init Error",
                                             "Error: unable to find app executables at "+ os.path.abspath(appPath) +
                                             " , please configure preferences")
            else:
                exe_found = False
                for exe_key, exe_name in preferences.exe_names.items():
                    if is_flexible:
                        if not os.path.isfile(appPath + "\\" + exe_name) and not os.path.isfile(".\\" + exe_name):
                            continue
                    else:
                        if not os.path.isfile(appPath + "\\" + exe_name):
                            continue

                    exe_found = True
                    break

                if not exe_found:
                    tkinter.messagebox.showerror("Init Error",
                                                 "Error: unable to find app executables at " + os.path.abspath( appPath)
                                                 + " , please configure preferences")

            preferences.exe_path = os.path.abspath(copy.copy(appPath))
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

def load_custom_config(toggleDict, settings, main_window):
    json_filepath = filedialog.askopenfilenames(
        filetypes=[("ConfigJSON", ".json")],
        defaultextension=".json")

    if len(json_filepath) == 0:
        return

    if not os.path.exists(json_filepath[0]):
        tkinter.messagebox.showerror("Processing Error",
                                     "Error: cannot find submitted config file")
        return

    load_config(json_filepath[0], toggleDict, settings, main_window)
    return

def load_config(path, toggleDict, settings, main_window):
    if not os.path.exists(path):
        tkinter.messagebox.showerror("Processing Error",
                                     "Error: cannot find default config files")
        return

    with open(path, 'r') as file:
        json_data = json.load(file)

    if not isConfigJSON(json_data):
        tkinter.messagebox.showerror("Config File Error",
                                     "Error: File is not a config file")
        return

    settings.set_from_json(json_data)

    # set the ui
    if "Alias" in json_data:
        main_window.title(main_window.title().split("|")[0] + "|    " + json_data["Alias"])
    else:
        main_window.title(main_window.title().split("|")[0] + "|    " + os.path.basename(path) )
    checkActiveToggles(toggleDict, settings)

    return

def populateConfigJson(load_config_menu, toggleDict, settings, config_folder, main_window):
    if os.path.isdir(config_folder):
        for file in os.scandir(config_folder):

            max_files = 10
            current_file_count = 0

            if file.is_file() and file.name.lower().endswith(".json"):
                pathstring = config_folder + "/" + file.name

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
                                             load_config(p, toggleDict, settings, main_window)
                                             )
                current_file_count += 1
                if current_file_count >= max_files:
                    break
    return

def pre_browse(text_field, text_var):
    new_entry = filedialog.askdirectory()
    if len(new_entry) == 0:
        return;

    text_var.set(new_entry)
    text_field.delete(0, tkinter.END)
    text_field.insert(0, new_entry)
    return new_entry

def update_pref(preferences, loc_app, loc_pre_set):
    # dump to json
    if not os.path.isdir("./config/"):
        os.mkdir("./config")

    preferences.preSet_path = loc_pre_set.get()
    preferences.exe_path = loc_app.get()

    default_settings = {
        "defaultConfigPath" :  preferences.preSet_path,
        "extractorLoc" :  preferences.exe_path
    }

    json_str = json.dumps(default_settings)
    with open("./config/settings.json", "w") as f:
        f.write(json_str)

    loadMem(preferences, False)

    return

def preferencesWindow(main_window, settings, size_button_normal, preferences):
    tk_app_path = tkinter.StringVar()
    tk_app_path.set(preferences.exe_path)
    tk_preSet_path = tkinter.StringVar()
    tk_preSet_path.set(preferences.preSet_path)

    preferencesWindow = tkinter.Toplevel(main_window)
    x_size = 500
    y_size = 170
    x_coord =  main_window.winfo_rootx() + (main_window.winfo_width() - x_size) // 2
    y_coord = main_window.winfo_rooty() + 30

    preferencesWindow.geometry('%dx%d+%d+%d' % (x_size, y_size, x_coord, y_coord))
    preferencesWindow.resizable(1, 0)

    preferencesWindow.title("IfcEnvExtactor preferences")

    preferencesWindow.transient(main_window)
    preferencesWindow.grab_set()
    preferencesWindow.focus_set()

    text_env_loc_browse = tkinter.Label(preferencesWindow, text="location of the env_extractor (dir):")
    text_env_loc_browse.pack(pady=4)
    frame_env_loc_browse = tkinter.Frame(preferencesWindow)
    frame_env_loc_browse.pack(fill=tkinter.X)
    entry_env_loc_path = tkinter.Entry(frame_env_loc_browse, textvariable=tk_app_path)
    entry_env_loc_path.pack(side=tkinter.LEFT, fill=tkinter.X, expand=True, padx=4)
    entry_env_loc_path.delete(0, tkinter.END)
    entry_env_loc_path.insert(0, preferences.exe_path)
    button_browse = tkinter.Button(frame_env_loc_browse, text="Browse", width=size_button_normal,
                                   command=lambda: pre_browse(text_field= entry_env_loc_path,
                                                              text_var=tk_app_path))
    button_browse.pack(side=tkinter.LEFT, padx=4)

    separator = ttk.Separator(preferencesWindow, orient='horizontal')
    separator.pack(fill='x', pady=10)

    # the entry functions for the output file
    text_config_browse = tkinter.Label(preferencesWindow, text="location of the pre-set files (dir):")
    text_config_browse.pack()
    frame_config_browse = tkinter.Frame(preferencesWindow)
    frame_config_browse.pack(fill=tkinter.X)
    entry_configpath = tkinter.Entry(frame_config_browse, text="Output path", textvariable=tk_preSet_path)
    entry_configpath.pack(side=tkinter.LEFT, fill=tkinter.X, expand=True, padx=4)
    entry_configpath.delete(0, tkinter.END)
    entry_configpath.insert(0, preferences.preSet_path)
    button_browse2 = tkinter.Button(frame_config_browse, text="Browse", width=size_button_normal,
                                    command=lambda: pre_browse(text_field= entry_configpath,
                                                               text_var= tk_preSet_path))
    button_browse2.pack(side=tkinter.LEFT, padx=4)

    save_button = tkinter.Button(preferencesWindow, text="Save", width=size_button_normal,
                                 command=lambda: [ update_pref(preferences, tk_app_path, tk_preSet_path),
                                                                    preferencesWindow.destroy()])
    save_button.pack(side=tkinter.LEFT, padx=(5,0))

    close_button = tkinter.Button(preferencesWindow, text="Cancel", width=size_button_normal,
                                  command=lambda: preferencesWindow.destroy())
    close_button.pack(side=tkinter.RIGHT, padx=(0, 5))

def summarywindow(settings):
    summary_window = tkinter.Tk()
    summary_window.geometry('500x590')
    summary_window.resizable(1, 0)
    summary_window.title("IfcEnvExtactor summary")

    settings.cast_to_json(bool_run=False)
    # json to file
    json_string = getNestedJsonString(settings.json)

    message_json_sum = tkinter.Text(summary_window, width=300, bg="#F0F0F0")
    message_json_sum.insert(tkinter.INSERT, json_string)
    message_json_sum.pack(fill='both', expand=True, padx=5, pady=(10, 5))

    close_button = tkinter.Button(summary_window, text="Close", width=8,
                                  command=lambda: summary_window.destroy())
    close_button.pack(side=tkinter.RIGHT, padx=(0, 5), pady=(0,5))

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