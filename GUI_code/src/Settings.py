# This file contains all the variables that are required to set up a config json and their related functions
import tkinter
import re
import os
import json
from pathlib import Path

class SettingsBase:
    settings = {}

    def __init__(self):
        for name, (var_type, var_value) in self.settings.items():
            setattr(self, name, var_type(value=var_value))
        return

class LoDSettings(SettingsBase):
    settings = {
        "lod00": (tkinter.IntVar, 1),
        "lod02": (tkinter.IntVar, 1),
        "lod03": (tkinter.IntVar, 1),
        "lod04": (tkinter.IntVar, 1),
        "lod10": (tkinter.IntVar, 1),
        "lod12": (tkinter.IntVar, 1),
        "lod13": (tkinter.IntVar, 1),
        "lod22": (tkinter.IntVar, 1),

        "lod40": (tkinter.IntVar, 0),
        "lod41": (tkinter.IntVar, 0),
        "lod42": (tkinter.IntVar, 0),
        "lode1": (tkinter.IntVar, 0),
        "lod32": (tkinter.IntVar, 0),
        "lod50": (tkinter.IntVar, 0)
    }

    def hasLoD(self):
        return any(
            getattr(self, name).get()
            for name in self.settings
        )

class VoxelSettings(SettingsBase):
    settings = {
        "voxel_size": (tkinter.DoubleVar, 1.0),
        "voxel_unit": (tkinter.StringVar, "m"),
        "voxel_filter": (tkinter.IntVar, 1)
    }

class FootprintSettings(SettingsBase):
    settings = {
        "make_footprint": (tkinter.IntVar, 1),
        "make_roofprint": (tkinter.IntVar, 1),
        "footprint_based": (tkinter.IntVar, 0),
        "footprint_elevation": (tkinter.DoubleVar, 0.0),
        "footprint_unit": (tkinter.StringVar, "m"),
        "find_footprint_elev": (tkinter.IntVar, 0)
    }

class DivSettings(SettingsBase):
    settings = {
        "ignore_proxy": (tkinter.IntVar, 1),
        "use_default": (tkinter.IntVar, 1),
        "custom_enabled": (tkinter.IntVar, 0),
        "simple_geo": (tkinter.IntVar, 0),
        "div_objects" : (tkinter.StringVar, "")
    }

    def __init__(self):
        super().__init__()

        self.default_div_ob = [
            "IfcWall",
            "IfcCurtainWall",
            "IfcWallStandardCase",
            "IfcRoof",
            "IfcSlab",
            "IfcWindow",
            "IfcColumn",
            "IfcBeam",
            "IfcDoor",
            "IfcCovering",
            "IfcMember",
            "IfcPlate"
        ]
        return

class OtherSettings(SettingsBase):
    settings = {
        "make_interior": (tkinter.IntVar, 0),
        "make_exterior": (tkinter.IntVar, 1),
        "summary_voxels": (tkinter.IntVar, 0),
        "ignoreIsExternal": (tkinter.IntVar, 1),
        "make_report": (tkinter.IntVar, 1),
        "make_obj": (tkinter.IntVar, 0),
        "make_step": (tkinter.IntVar, 0),
        "highTol_toggle": (tkinter.IntVar, 1)
    }

class PathSettings(SettingsBase):
    settings ={
        "input_path" : (tkinter.StringVar, ""),
        "output_path": (tkinter.StringVar, "")
    }

class GuiSettings:
    def __init__(self):
        self.paths = PathSettings()
        self.lod = LoDSettings()
        self.voxel = VoxelSettings()
        self.footprint = FootprintSettings()
        self.div = DivSettings()
        self.other = OtherSettings()
        self.json = {}; # container for advanced settings if a custom json is loaded
        return

    def clear_custom(self): #wipe the custom settings coming from external json file
        self.json = {};
        return;

    def getDefaultDivObjects(self):

        output_string = ""
        for div_ob in self.div.default_div_ob:
            output_string += div_ob + "\t"

        return output_string

    def dump_to_json(self):

        input_path =  self.paths.input_path.get()
        output_path = self.paths.output_path.get()

        lod_settings = self.lod
        voxel_settings = self.voxel
        footprint_settings = self.footprint
        other_settings = self.other
        div_settings = self.div

        # check voxel input
        try:
            float(voxel_settings.voxel_size.get())
        except:
            tkinter.messagebox.showerror("Settings Error", "Error: no valid voxel size supplied")
            return False

        if (float(voxel_settings.voxel_size.get()) <= 0):
            tkinter.messagebox.showerror("Settings Error", "Error: voxel size should be larger than 0")
            return False

        # check footprint input
        try:
            float(footprint_settings.footprint_elevation.get())
        except:
            tkinter.messagebox.showerror("Settings Error", "Error: no valid footprint elevation supplied")
            return False

        # check if an LoD output is selected
        if not (lod_settings.hasLoD()):
            tkinter.messagebox.showerror("Settings Error", "Error: no LoD output selected")
            return False

        if (lod_settings.lod02 and not footprint_settings.make_footprint and not footprint_settings.make_roofprint):
            tkinter.messagebox.showerror("Settings Error", "Error: no LoD0.2 footprint or roofoutline selected")
            return False

        # check paths
        input_path_list = re.split(r'(?<!{) (?![^{]*})', input_path)
        input_path_list = [part.replace('{', '').replace('}', '') for part in input_path_list]

        for path in input_path_list:
            if (not os.path.isfile(path)):
                tkinter.messagebox.showerror("Settings Error", "Error: No Valid input file supplied")
                return False

        if (not os.path.isdir(os.path.dirname(output_path)) or len(output_path) == 0):
            tkinter.messagebox.showerror("Settings Error",
                                         "Error: No Valid output folder supplied\n (GUI can not create new folders)")
            return False

        json_path = Path(input_path_list[0]).stem + "_config.json"

        # write data to json
        json_dictionary = self.json

        json_dictionary["Filepaths"] = {}
        json_dictionary["Filepaths"]["Input"] = input_path_list
        json_dictionary["Filepaths"]["Output"] = output_path

        voxel_size = float(voxel_settings.voxel_size.get())
        if voxel_settings.voxel_unit.get() == "mm":
            voxel_size /= 1000
        elif voxel_settings.voxel_unit.get() == "cm":
            voxel_size /= 100

        json_dictionary["Tolerances"] = {}
        if other_settings.highTol_toggle.get():
            json_dictionary["Tolerances"]["Spatial tolerance"] = 1e-6
            json_dictionary["Tolerances"]["Angular tolerance"] = 1e-4
            json_dictionary["Tolerances"]["Area tolerance"] = 1e-4
        else:
            json_dictionary["Tolerances"]["Spatial tolerance"] = 1e-4
            json_dictionary["Tolerances"]["Angular tolerance"] = 1e-3
            json_dictionary["Tolerances"]["Area tolerance"] = 1e-3

        json_dictionary["Voxel"] = {}
        json_dictionary["Voxel"]["Size"] = voxel_size
        # json_dictionary["Voxel"]["Store values"] = other_settings.summary_voxels.get()
        json_dictionary["Voxel"]["Coarse filter"] = voxel_settings.voxel_filter.get()

        json_dictionary["IFC"] = {}
        if not div_settings.custom_enabled.get():
            json_dictionary["IFC"]["Default div"] = div_settings.use_default.get()
            json_dictionary["IFC"]["Ignore proxy"] = div_settings.ignore_proxy.get()
            json_dictionary["IFC"]["Div objects"] = []
        else:
            json_dictionary["IFC"]["Default div"] = False
            json_dictionary["IFC"]["Ignore proxy"] = False
            json_dictionary["IFC"]["Div objects"] = settings.div.div_objects.split()

        if div_settings.simple_geo.get():
            json_dictionary["IFC"]["Ignore voids"] = 2
        else:
            json_dictionary["IFC"]["Ignore voids"] = 0

        json_dictionary["IFC"]["Fetch footprint elevation"] = footprint_settings.find_footprint_elev.get()
        json_dictionary["IFC"]["Ignore IsExternal"] = other_settings.ignoreIsExternal.get()

        footprint_elevation = float(footprint_settings.footprint_elevation.get())
        if footprint_settings.footprint_unit.get() == "mm":
            footprint_elevation /= 1000
        elif footprint_settings.footprint_unit.get() == "cm":
            footprint_elevation /= 100

        json_dictionary["JSON"] = {}
        if not footprint_settings.find_footprint_elev.get():
            json_dictionary["JSON"]["Footprint elevation"] = footprint_elevation

        json_dictionary["JSON"]["Generate exterior"] = other_settings.make_exterior.get()
        json_dictionary["JSON"]["Generate interior"] = other_settings.make_interior.get()

        json_dictionary["Generate report"] = other_settings.make_report.get()

        json_dictionary["Output format"] = {}
        json_dictionary["Output format"]["STEP file"] = other_settings.make_step.get()
        json_dictionary["Output format"]["OBJ file"] = other_settings.make_obj.get()

        lod_list = []
        if (lod_settings.lod00.get()):
            lod_list.append(0.0)
        if (lod_settings.lod02.get()):
            lod_list.append(0.2)
            json_dictionary["JSON"]["Generate footprint"] = footprint_settings.make_footprint.get()
            json_dictionary["JSON"]["Generate roof outline"] = footprint_settings.make_roofprint.get()
        if (lod_settings.lod03.get()):
            lod_list.append(0.3)
            json_dictionary["JSON"]["Generate footprint"] = footprint_settings.make_footprint.get()
            json_dictionary["JSON"]["Generate roof outline"] = footprint_settings.make_roofprint.get()
        if (lod_settings.lod04.get()):
            lod_list.append(0.4)
            json_dictionary["JSON"]["Generate footprint"] = footprint_settings.make_footprint.get()
            json_dictionary["JSON"]["Generate roof outline"] = footprint_settings.make_roofprint.get()
        if (lod_settings.lod10.get()):
            lod_list.append(1.0)
        if (lod_settings.lod12.get()):
            lod_list.append(1.2)
            json_dictionary["JSON"]["Footprint based"] = footprint_settings.footprint_based.get()
        if (lod_settings.lod13.get()):
            lod_list.append(1.3)
            json_dictionary["JSON"]["Footprint based"] = footprint_settings.footprint_based.get()
        if (lod_settings.lod22).get():
            lod_list.append(2.2)
            json_dictionary["JSON"]["Footprint based"] = footprint_settings.footprint_based.get()
        if (lod_settings.lode1.get()):
            lod_list.append("e.1")
        if (lod_settings.lod32.get()):
            lod_list.append(3.2)
        if (lod_settings.lod50.get()):
            lod_list.append(5.0)
        if (lod_settings.lod40.get()):
            lod_list.append("4.0")
        if (lod_settings.lod41.get()):
            lod_list.append("4.1")
        if (lod_settings.lod42.get()):
            lod_list.append("4.2")
        json_dictionary["LoD output"] = lod_list

        with open(json_path, "w") as outfile:
            json.dump(json_dictionary, outfile)

        return True;

    def set_from_json(self, json_data):
        return;