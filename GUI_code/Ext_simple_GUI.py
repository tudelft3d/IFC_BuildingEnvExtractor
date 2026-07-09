import os
import time
import tkinter
from tkinter import ttk, filedialog, messagebox, PhotoImage
import subprocess
import threading
import re
import json
import webbrowser

from pathlib import Path

import src.Settings as Settings
import src.InterfaceExtension as IExtension

# global var
# GUI version
is_simple = True

def findValidPath(code_path, addition):
    if (os.path.isfile(os.path.abspath(code_path))):
        return code_path
    else:
        if not (os.path.isfile(os.path.abspath(code_path))):
            return None
        return code_path

def runCode(preferences, settings, message_div_objects, is_gen, main_window = {}):
    # because a text object has no variable we have to manually update the div objects when required
    settings.div.div_objects.set(message_div_objects.get('1.0', tkinter.END))

    input_path_list = re.split(r'; ', settings.paths.input_path.get())
    json_path_end = "_config.json"

    config_path = ""
    if is_gen:
        config_path = filedialog.asksaveasfilename(
            filetypes=[("JSON file", ".json")],
            defaultextension=".json",
            initialfile=Path(input_path_list[0]).stem + json_path_end,
            initialdir= preferences.config_save_browse_path
        )

        if (len(config_path) == 0):
            return

        preferences.config_save_browse_path = os.path.dirname(config_path)

    else:
        config_path = "~temp" + json_path_end

    if settings.cast_to_json(bool_run= not is_gen):
        with open(config_path, "w") as outfile:
            json.dump(settings.json, outfile)
    else:
        return

    # get schema of the file
    if is_gen:
        tkinter.messagebox.showinfo("succes", "Info: Config file has been successfully created")

        if "Alias" in settings.json:
            main_window.title(main_window.title().split("|")[0] + "|    " + settings.json["Alias"])
        else:
            main_window.title(main_window.title().split("|")[0] + "|    " + os.path.basename(config_path))
        return

    code_path = ""
    scheme_found = False
    exe_folder_path = preferences.exe_path
    for path in input_path_list:
        counter = 0;
        for line in open(path):
            for exe_key, exe_name in preferences.exe_names.items():
                if "FILE_SCHEMA(('{}'))".format(exe_key.upper()) in line or "FILE_SCHEMA (('{}'))".format(exe_key.upper()) in line:
                    code_path = findValidPath(exe_folder_path + "\\" + exe_name, exe_key)
                    if code_path == "":
                        code_path = findValidPath(".\\" + exe_name, exe_key)
                    if code_path !=  "":
                        scheme_found = True
                        break
                    else:
                        tkinter.messagebox.showerror("Exe Error",
                                                     "Error: Unable to find suitable executable (" + exe_key + ")")
                        return
                    break
            if scheme_found:
                break

            if counter == 100:
                tkinter.messagebox.showerror("File Error",
                                             "Error: Was unable to find IFC schema in file")
                return
            counter += 1


    if scheme_found:
        runExe(code_path, config_path)
    os.remove(config_path)
    return

def runExe(code_path, json_path):

    try:
        stop_event = threading.Event()
        stop_event.clear()
        # Run the executable and capture its output
        env_extractor_process = subprocess.Popen(
            [
                code_path,
                json_path
            ],
        )

        def poll_process():
            while not stop_event.is_set():
                if env_extractor_process.poll() is not None:  # Check if process has finished
                    break
                time.sleep(0.5)

            if stop_event.is_set():  # If the loop exits because of `stop_event`, terminate the process
                if env_extractor_process.poll() is None:  # If process is still running
                    env_extractor_process.terminate()
                    tkinter.messagebox.showinfo("Process cancelled", "Process cancelled by user")
            else:
                if env_extractor_process.returncode == 0:
                    tkinter.messagebox.showinfo("Success", "Success: Process completed successfully")
                elif env_extractor_process.returncode == 1:
                    tkinter.messagebox.showerror("Processing Error", "Error: Error during process\n "
                                                                     "See report file for more info")
                else:
                    tkinter.messagebox.showerror("Processing Error",
                                                 "Error: application encountered an unexpected error")
                    
            run_button.config(text="Run", command=lambda: runCode(preferences, settings, message_div_objects,  False))
            close_button.config(state="normal")
        def stop_process():
            stop_event.set()

        # swap out run button
        run_button.config(text="Cancel", command=lambda:stop_process())
        close_button.config(state="disabled")

        # Start the polling thread
        time.sleep(0.5)
        threading.Thread(target=poll_process, daemon=True).start()

    except subprocess.CalledProcessError as e:
        tkinter.messagebox.showerror("Processing Error",
                                     "Error: application encountered an unexpected error")
    return

def on_closing():
    IExtension.update_pref(preferences, "", "")
    main_window.destroy()

# main variables
size_entry_small = 13
size_button_small = 2
size_button_normal = 8
main_window_name_base = "IfcEnvExtractor GUI    |    "

# setup the window and the grid
main_window = tkinter.Tk()
if is_simple:
    main_window.geometry('500x293')
else:
    main_window.geometry('500x590')
main_window.resizable(1,0)
main_window.title(main_window_name_base + "Untitled")
main_window.protocol("WM_DELETE_WINDOW", on_closing)
# create settings classes
settings = Settings.GuiSettings()
preferences = Settings.Preferences()

IExtension.loadMem(preferences)

# the entry functions for the main ifc file
text_file_browse = tkinter.Label(main_window, text="Input IFC path(s):")
text_file_browse.pack(pady=4)
frame_file_browse = tkinter.Frame(main_window)
frame_file_browse.pack(fill=tkinter.X)
entry_inputpath = tkinter.Entry(frame_file_browse, text= "Input path(s)", textvariable=settings.paths.input_path)
entry_inputpath.pack(side=tkinter.LEFT, fill=tkinter.X, expand=True, padx=4)
button_browse = tkinter.Button(frame_file_browse, text="Browse", width=size_button_normal,
                               command= lambda: IExtension.browse_(
                                   entry_inputpath,
                                   False,
                                   main_window,
                                   "",
                                   preferences)
                               )
button_browse.pack(side=tkinter.LEFT, padx=4)

separator = ttk.Separator(main_window, orient='horizontal')
separator.pack(fill='x', pady=10)

# the entry functions for the output file
text_folder_browse = tkinter.Label(main_window, text="Output file path:")
text_folder_browse.pack()
frame_folder_browse = tkinter.Frame(main_window)
frame_folder_browse.pack(fill=tkinter.X)
entry_outputpath = tkinter.Entry(frame_folder_browse, text= "Output path", textvariable= settings.paths.output_path)
entry_outputpath.pack(side=tkinter.LEFT, fill=tkinter.X, expand=True, padx=4)
button_browse2 = tkinter.Button(frame_folder_browse, text="Browse" , width=size_button_normal,
                                command= lambda: IExtension.browse_(
                                    entry_outputpath,
                                    True,
                                    main_window,
                                    entry_inputpath.get(),
                                    preferences)
                                )
button_browse2.pack(side=tkinter.LEFT, padx=4)

separator = ttk.Separator(main_window, orient='horizontal')
separator.pack(fill='x', pady=10)

# make a frame to split the two different lod input settings
frame_lod_settings_complete = tkinter.Frame(main_window)
frame_lod_settings_complete.pack()

# the LoD levels that are desired to be output
frame_lod_settings_gen = tkinter.Frame(frame_lod_settings_complete)
frame_lod_settings_gen.pack(side=tkinter.LEFT, padx=5)

frame_lod_settings1 = tkinter.Frame(frame_lod_settings_gen)
frame_lod_settings2 = tkinter.Frame(frame_lod_settings_gen)
frame_lod_settings3 = tkinter.Frame(frame_lod_settings_gen)
frame_lod_settings4 = tkinter.Frame(frame_lod_settings_gen)

toggle_makelod00 = ttk.Checkbutton(frame_lod_settings1, text="LoD0.0", variable=settings.lod.lod00,
                                   command=lambda: [
                                       IExtension.checkActiveToggles(toggle_dictionary, settings),
                                   ])
toggle_makelod02 = ttk.Checkbutton(frame_lod_settings1, text="LoD0.2", variable=settings.lod.lod02,
                                   command=lambda: [
                                       IExtension.checkActiveToggles(toggle_dictionary, settings),
                                   ])
toggle_makelod03 = ttk.Checkbutton(frame_lod_settings1, text="LoD0.3", variable=settings.lod.lod03,
                                   command=lambda: [
                                       IExtension.checkActiveToggles(toggle_dictionary, settings),
                                   ])
toggle_makelod04 = ttk.Checkbutton(frame_lod_settings1, text="LoD0.4", variable=settings.lod.lod04,
                                   command=lambda: [
                                       IExtension.checkActiveToggles(toggle_dictionary, settings),
                                   ])
toggle_makelod10 = ttk.Checkbutton(frame_lod_settings2, text="LoD1.0", variable=settings.lod.lod10,
                                   command=lambda: [
                                       IExtension.checkActiveToggles(toggle_dictionary, settings),
                                   ])
toggle_makelod12 = ttk.Checkbutton(frame_lod_settings2, text="LoD1.2", variable=settings.lod.lod12,
                                   command=lambda: [
                                       IExtension.checkActiveToggles(toggle_dictionary, settings),
                                   ])
toggle_makelod13 = ttk.Checkbutton(frame_lod_settings2, text="LoD1.3", variable=settings.lod.lod13,
                                   command=lambda: [
                                       IExtension.checkActiveToggles(toggle_dictionary, settings),
                                   ])
toggle_makelod22 = ttk.Checkbutton(frame_lod_settings2, text="LoD2.2", variable=settings.lod.lod22,
                                   command=lambda: [
                                       IExtension.checkActiveToggles(toggle_dictionary, settings),
                                   ])

toggle_makelode1 = ttk.Checkbutton(frame_lod_settings3, text="LoDe.1", variable=settings.lod.lode1,
                                   command=lambda: [
                                       IExtension.checkActiveToggles(toggle_dictionary, settings),
                                   ])

toggle_makelod32 = ttk.Checkbutton(frame_lod_settings3, text="LoD3.2", variable=settings.lod.lod32,
                                   command=lambda: [
                                       IExtension.checkActiveToggles(toggle_dictionary, settings),
                                   ])

toggle_makelod50 = ttk.Checkbutton(frame_lod_settings3, text="LoDV.0", variable=settings.lod.lod50,
                                   command=lambda: [
                                       IExtension.checkActiveToggles(toggle_dictionary, settings),
                                   ])

toggle_makelod40 = ttk.Checkbutton(frame_lod_settings4, text="LoD4.0",  variable=settings.lod.lod40,
                                   command=lambda: [
                                       IExtension.checkActiveToggles(toggle_dictionary, settings),
                                   ])

toggle_makelod41 = ttk.Checkbutton(frame_lod_settings4, text="LoD4.1",  variable=settings.lod.lod41,
                                   command=lambda: [
                                       IExtension.checkActiveToggles(toggle_dictionary, settings),
                                   ])

toggle_makelod42 = ttk.Checkbutton(frame_lod_settings4, text="LoD4.2",  variable=settings.lod.lod42,
                                   command=lambda: [
                                       IExtension.checkActiveToggles(toggle_dictionary, settings),
                                   ])

text_format_settings = tkinter.Label(frame_lod_settings_gen, text="Additional format:")
frame_format_settings = tkinter.Frame(frame_lod_settings_gen)

toggle_make_obj = ttk.Checkbutton(frame_format_settings, text=".OBJ", variable=settings.other.make_obj)
toggle_make_step = ttk.Checkbutton(frame_format_settings, text=".STEP", variable=settings.other.make_step)

if not is_simple:
    text_lod_settings = tkinter.Label(frame_lod_settings_gen, text="Desired LoD generation:")
    text_lod_settings.pack()

    frame_lod_settings1.pack()
    frame_lod_settings2.pack()
    frame_lod_settings3.pack()
    frame_lod_settings4.pack()

    toggle_makelod00.pack(side=tkinter.LEFT)
    toggle_makelod02.pack(side=tkinter.LEFT)
    toggle_makelod03.pack(side=tkinter.LEFT)
    toggle_makelod04.pack(side=tkinter.LEFT)
    toggle_makelod10.pack(side=tkinter.LEFT)
    toggle_makelod12.pack(side=tkinter.LEFT)
    toggle_makelod13.pack(side=tkinter.LEFT)
    toggle_makelod22.pack(side=tkinter.LEFT)
    toggle_makelode1.pack(side=tkinter.LEFT)
    toggle_makelod32.pack(side=tkinter.LEFT)
    toggle_makelod50.pack(side=tkinter.LEFT)
    toggle_makelod40.pack(side=tkinter.LEFT)
    toggle_makelod41.pack(side=tkinter.LEFT)
    toggle_makelod42.pack(side=tkinter.LEFT)

    text_format_settings.pack(pady=[5, 0])
    frame_format_settings.pack()
    toggle_make_obj.pack(side=tkinter.LEFT, padx=5)
    toggle_make_step.pack(side=tkinter.LEFT, padx=5)

# makeSplit
frame_lod_settings_sep = ttk.Separator(frame_lod_settings_complete, orient=tkinter.VERTICAL)

separator_lod_settings = ttk.Separator(frame_lod_settings_sep, orient=tkinter.VERTICAL)
separator_lod_settings.pack(fill='y', pady=40, padx=5, expand=True)

# toggle if footprint or roofoutline is desired
frame_lod_settings_foot = tkinter.Frame(frame_lod_settings_complete)
frame_lod_settings_foot.pack(side=tkinter.RIGHT)

toggle_makeexterior = ttk.Checkbutton(frame_lod_settings_foot, text="Generate exteriors", variable=settings.other.make_exterior)
toggle_makeinterior = ttk.Checkbutton(frame_lod_settings_foot, text="Generate interiors", variable=settings.other.make_interior)
toggle_makefootprint = ttk.Checkbutton(frame_lod_settings_foot, text="Export footprint", variable=settings.footprint.make_footprint)
toggle_makeroofprint = ttk.Checkbutton(frame_lod_settings_foot, text="Export roof outline", variable=settings.footprint.make_roofprint)
toggle_footprint_based = ttk.Checkbutton(frame_lod_settings_foot, text="Footprint based abstraction", variable=settings.footprint.footprint_based)
toggle_ignore_IsExternal = ttk.Checkbutton(frame_lod_settings_foot, text="Ignore IsExternal", variable=settings.other.ignoreIsExternal)

separator2 = ttk.Separator(main_window, orient='horizontal')

if not is_simple:
    frame_lod_settings_sep.pack(side=tkinter.LEFT, expand=True)

    text_lod_settings = tkinter.Label(frame_lod_settings_foot, text="Additional settings")
    text_lod_settings.pack()

    toggle_makeexterior.pack(side=tkinter.TOP, fill=tkinter.X)
    toggle_makeinterior.pack(side=tkinter.TOP, fill=tkinter.X)
    toggle_makefootprint.pack(side=tkinter.TOP, fill=tkinter.X)
    toggle_makeroofprint.pack(side=tkinter.TOP, fill=tkinter.X)
    toggle_footprint_based.pack(side=tkinter.TOP, fill=tkinter.X)
    toggle_ignore_IsExternal.pack(side=tkinter.TOP, fill=tkinter.X)

    separator2.pack(fill='x', pady=10)

# the voxel size that is desired
frame_foot_voxel = tkinter.Frame(main_window)
frame_foot_voxel.pack()

frame_voxel = tkinter.Frame(frame_foot_voxel)
frame_voxel.pack(side=tkinter.LEFT, padx=30)

text_voxel_settings = tkinter.Label(frame_voxel, text="Voxel size:")
text_voxel_settings.pack()

entry_voxelsize = tkinter.Entry(
    frame_voxel,
    text= "voxelsize",
    width=size_entry_small,
    textvariable=settings.voxel.voxel_size
)

entry_voxelsize.pack(side=tkinter.LEFT)

button_min_voxelsize = tkinter.Button(frame_voxel, text="-", width=size_button_small,
                                      command= lambda : IExtension.decrement(entry_voxelsize, 0.1))
button_min_voxelsize.pack(side=tkinter.LEFT, padx=(5,0))
button_plus_voxelsize = tkinter.Button(frame_voxel, text="+", width=size_button_small,
                                       command= lambda : IExtension.increment(entry_voxelsize, 0.1))
button_plus_voxelsize.pack(side=tkinter.LEFT)

voxel_unit_toggle = IExtension.makeUnitWindow(frame_voxel, settings.voxel.voxel_unit)
voxel_unit_toggle.pack(side=tkinter.LEFT)

# the footprint height
frame_footprint = tkinter.Frame(frame_foot_voxel)
frame_footprint.pack(side=tkinter.LEFT)

text_footprint_settings = tkinter.Label(frame_footprint, text="Footprint elevation:")
text_footprint_settings.pack()

entry_footprint = tkinter.Entry(
    frame_footprint,
    text= "footprint elevation",
    width=size_entry_small,
    textvariable= settings.footprint.footprint_elevation
)
entry_footprint.pack(side=tkinter.LEFT)

button_min_footprint = tkinter.Button(frame_footprint, text="-", width=size_button_small,
                                      command= lambda : IExtension.decrement(entry_footprint, 0.01))
button_min_footprint.pack(side=tkinter.LEFT, padx=(5,0))
button_plus_footprint = tkinter.Button(frame_footprint, text="+", width=size_button_small,
                                       command= lambda : IExtension.increment(entry_footprint, 0.01))
button_plus_footprint.pack(side=tkinter.LEFT)

footprint_unit_toggle= IExtension.makeUnitWindow(frame_footprint, settings.footprint.footprint_unit)
footprint_unit_toggle.pack(side=tkinter.LEFT)

# the footprint height detection
auto_detect_foot_elev_toggle = ttk.Checkbutton(frame_footprint,
                                               text="Automatic",
                                               variable=settings.footprint.find_footprint_elev,
                                               command= lambda : IExtension.checkActiveToggles(toggle_dictionary, settings))
auto_detect_foot_elev_toggle.pack(side=tkinter.LEFT, padx=10)

separatorFootprint = ttk.Separator(main_window, orient='horizontal')
separatorFootprint.pack(fill='x', pady=10)

# The div objects
message_div_objects = tkinter.Text(main_window,  width=300, height=5, bg="#F0F0F0", fg="#707070")

frame_div_objects = tkinter.Frame(main_window)
frame_div_objects.pack()

igoreproxy_toggle = ttk.Checkbutton(frame_div_objects,
                                    text="Ignore proxy elements",
                                    variable=settings.div.ignore_proxy,
                                    command= lambda : IExtension.checkActiveToggles(toggle_dictionary,settings))
igoreproxy_toggle.pack(side=tkinter.LEFT, padx=10)

useDefault_toggle = ttk.Checkbutton(frame_div_objects,
                                    text="Use default div objects",
                                    variable=settings.div.use_default,
                                    command= lambda : IExtension.checkActiveToggles(toggle_dictionary, settings))

enableCustom_toggle = ttk.Checkbutton(frame_div_objects,
                                    text="Custom div objects",
                                    variable=settings.div.custom_enabled,
                                    command= lambda : IExtension.checkActiveToggles(toggle_dictionary, settings))

if not is_simple:
    useDefault_toggle.pack(side=tkinter.LEFT, padx=10)
    enableCustom_toggle.pack(side=tkinter.LEFT)

# div communication
message_div_objects.insert(tkinter.INSERT, settings.getDefaultDivObjects())
message_div_objects['state'] = tkinter.DISABLED

frame_final_objects = tkinter.Frame(main_window)
simpleGeo_toggle = ttk.Checkbutton(frame_final_objects,
                                    text="Use simple geo",
                                    variable=settings.div.simple_geo)

highTol_toggle = ttk.Checkbutton(frame_final_objects,
                                    text="Use high precision",
                                    variable=settings.other.highTol_toggle)

voxelFil_toggle = ttk.Checkbutton(frame_final_objects,
                                    text="Use voxel filtering",
                                    variable=settings.voxel.voxel_filter)

if not is_simple:
    message_div_objects.pack(fill='x', padx=5, pady=10)
    frame_final_objects.pack()
    simpleGeo_toggle.pack(side=tkinter.LEFT, padx=5)
    highTol_toggle.pack(side=tkinter.LEFT, padx=5)
    voxelFil_toggle.pack(side=tkinter.LEFT, padx=5)

# other buttons
separator3 = ttk.Separator(main_window, orient='horizontal')
separator3.pack(fill='x', pady=10)

frame_other = tkinter.Frame(main_window)
frame_other.pack(fill=tkinter.X)

run_button = tkinter.Button(frame_other, text="Run", width=size_button_normal, command=lambda: runCode(preferences, settings, message_div_objects, False))
run_button.pack(side=tkinter.LEFT, padx=(5,0))

text_toolTip = tkinter.Label(frame_other, text="hover over settings for tooltip")
text_toolTip.pack(side=tkinter.LEFT, padx=(15,0))

close_button = tkinter.Button(frame_other, text="Close", width=size_button_normal,
                              command= lambda : [IExtension.update_pref(preferences, "", ""),
                                                 main_window.destroy()])

close_button.pack(side=tkinter.RIGHT, padx=(0,5))

#tooltips
IExtension.Tooltip(entry_inputpath, "Input path(s), supports multifile IFC models")
IExtension.Tooltip(button_browse, "Input path(s), supports multifile IFC models")

IExtension.Tooltip(entry_outputpath, "output path, application can not create new folders")
IExtension.Tooltip(button_browse2, "output path, application can not create new folders")

desired_lod_tooltip_txt = "Desired LoD abstractions to be created"
IExtension.Tooltip(frame_lod_settings1, desired_lod_tooltip_txt)
IExtension.Tooltip(frame_lod_settings2, desired_lod_tooltip_txt)
IExtension.Tooltip(frame_lod_settings3, desired_lod_tooltip_txt)
IExtension.Tooltip(frame_lod_settings4, desired_lod_tooltip_txt)

IExtension.Tooltip(toggle_make_obj, "If active output is copied to wavefront .OBJ file(s)")
IExtension.Tooltip(toggle_make_step, "If active output is copied to .STEP (ISO 10303) file(s)")

IExtension.Tooltip(toggle_makefootprint, "If active a footprint will be created at the footprint elevation (lod0.0, 0.2, 0.3 and 0.4 only)")
IExtension.Tooltip(toggle_makeroofprint, "If active a roof outline will be created (lod0.0, 0.2, 0.3 and 0.4 only)")
IExtension.Tooltip(toggle_footprint_based, "If active the footprint will be used to restrict the output (LoD1.2, 1.3 & 2.2)")
IExtension.Tooltip(toggle_makeexterior, "If active exterior shells will be stored")
IExtension.Tooltip(toggle_makeinterior, "If active spaces will be stored (Lod0.2, 1.2, 2.2, 3.2 & voxels) and storey "
                             "objects will be created (loD0.2 and 0.3 )")
IExtension.Tooltip(toggle_ignore_IsExternal, "If active IsExternal attribute will be ignored and the application will determine which objects are external")

IExtension.Tooltip(entry_voxelsize, "Voxel size to be used for the analysis")
IExtension.Tooltip(button_min_voxelsize, "Decrement size by 0.1")
IExtension.Tooltip(button_plus_voxelsize, "increment size by 0.1")

IExtension.Tooltip(entry_footprint, "Z height at which the footprint section is taken and the ground floor of the abstractions will be placed")
IExtension.Tooltip(button_min_footprint, "Decrement size by 0.01")
IExtension.Tooltip(button_plus_footprint, "Decrement size by 0.01")

IExtension.Tooltip(auto_detect_foot_elev_toggle, "If active the tool will fetch the ground floor IfcBuildingStorey object elevation, naming based on the BIM Base IDS")

IExtension.Tooltip(igoreproxy_toggle, "If active IfcBuildingProxyElements will be excluded from the process")
IExtension.Tooltip(useDefault_toggle, "If active default div objects will be used during the process")
IExtension.Tooltip(message_div_objects, "Used div objects during the process, separated by a space. Can be unlocked by toggling the Custom div objects button")
IExtension.Tooltip(enableCustom_toggle, "Enables the custom div objects list")
IExtension.Tooltip(simpleGeo_toggle, "Use simplefied geometry for the evaluations (voids are ignored)")

IExtension.Tooltip(run_button, "Run the tool")
IExtension.Tooltip(close_button, "Exit the application")

# set up dictionary with all toggles that can be turned off
toggle_dictionary = {
    "make_footprint" : toggle_makefootprint,
    "make_roofprint" : toggle_makeroofprint,
    "make_footprint_based" : toggle_footprint_based,
    "make_interior" : toggle_makeinterior,
    "make_ignore_IsExternal" : toggle_ignore_IsExternal,

    "entry_footprint": entry_footprint,
    "button_plus_footprint": button_plus_footprint,
    "button_min_footprint": button_min_footprint,
    "button_unit_toggle": footprint_unit_toggle,

    "message_div_objects" : message_div_objects,
    "useDefault_toggle" : useDefault_toggle,
    "igoreproxy_toggle" : igoreproxy_toggle
}
IExtension.checkActiveToggles(toggle_dictionary, settings)

# top bar
menubar = tkinter.Menu(main_window)
menubar.config(fg="black", activeforeground="black", activeborderwidth=1, font="Monaco 11")

File_menu = tkinter.Menu(menubar, tearoff=False)
load_config_menu = tkinter.Menu(File_menu, tearoff=False)
IExtension.populateConfigJson(load_config_menu, toggle_dictionary, settings, preferences.preSet_path, main_window)
load_config_menu.add_separator()
load_config_menu.add_command(label="Custom pre-set",
                             command= lambda:IExtension.load_custom_config(toggle_dictionary, settings, main_window, preferences))

File_menu.add_cascade(label="Load config", menu=load_config_menu)
if not is_simple:
    File_menu.add_cascade(label="Store config", command= lambda: runCode(preferences, settings, message_div_objects, True, main_window= main_window))
File_menu.add_separator()
if not is_simple:
    File_menu.add_cascade(label="Clean JSON", command= lambda: settings.clear_custom())
File_menu.add_cascade(label="Show summary", command= lambda: IExtension.summarywindow(settings))
File_menu.add_separator()
File_menu.add_cascade(label="Preferences", command= lambda: IExtension.preferencesWindow(main_window, settings, size_button_normal, preferences))
menubar.add_cascade(label="File", menu=File_menu)
Settings_menu = tkinter.Menu(menubar, tearoff=False)

Settings_menu.add_cascade(label="About", command= lambda: webbrowser.open("https://github.com/tudelft3d/IFC_BuildingEnvExtractor"))
menubar.add_cascade(label="Help", menu=Settings_menu)

main_window.config(menu=menubar)

main_window.mainloop()