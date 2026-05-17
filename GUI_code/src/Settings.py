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

    def reset(self):
        for name, (_, default_value) in self.settings.items():
            getattr(self, name).set(default_value)

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

    def clearLoD(self):
        for var in self.__dict__.values():
            var.set(int(0))

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

    def reset(self):
        super().reset()

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

    def reset(self):
        self.paths.reset()
        self.lod.reset()
        self.voxel.reset()
        self.footprint.reset()
        self.div.reset()
        self.other.reset()
        self.json = {}
        return

    def clear_custom(self): #wipe the custom settings coming from external json file
        self.json = {};
        return;

    def getDefaultDivObjects(self):

        output_string = ""
        for div_ob in self.div.default_div_ob:
            output_string += div_ob + "\t"

        return output_string

    def cast_to_json(self, bool_run):

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
        if not lod_settings.hasLoD() and bool_run:
            tkinter.messagebox.showerror("Settings Error", "Error: no LoD output selected")
            return False

        if (lod_settings.lod02 and not footprint_settings.make_footprint and not footprint_settings.make_roofprint):
            tkinter.messagebox.showerror("Settings Error", "Error: no LoD0.2 footprint or roofoutline selected")
            return False

        # check paths
        input_path_list = re.split(r'(?<!{) (?![^{]*})', input_path)
        input_path_list = [part.replace('{', '').replace('}', '') for part in input_path_list]

        for path in input_path_list:
            if (not os.path.isfile(path) and bool_run):
                tkinter.messagebox.showerror("Settings Error", "Error: No Valid input file supplied")
                return False

        if (not os.path.isdir(os.path.dirname(output_path)) or len(output_path) == 0) and bool_run:
            tkinter.messagebox.showerror("Settings Error",
                                         "Error: No Valid output folder supplied\n (GUI can not create new folders)")
            return False

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
            lod_list.append(4.0)
        if (lod_settings.lod41.get()):
            lod_list.append(4.1)
        if (lod_settings.lod42.get()):
            lod_list.append(4.2)
        json_dictionary["LoD output"] = lod_list

        self.json = json_dictionary

        return True;

    def set_from_json(self, json_data):
        self.reset()
        self.json = json_data

        if "Filepaths" in json_data:
            json_data_filepaths = json_data["Filepaths"]

            if "Input" in json_data_filepaths:
               json_input_path_list = json_data_filepaths["Input"]
               input_path = ""
               for path in json_input_path_list:
                   input_path += path + " "
               self.paths.input_path.set(input_path)

            if "Output" in json_data_filepaths:
                self.paths.output_path.set(json_data_filepaths["Output"])

        if "Voxel" in json_data:
            json_data_voxel = json_data["Voxel"]
            if "Size" in json_data_voxel:
                self.voxel.voxel_size.set(json_data_voxel["Size"])
                self.voxel.voxel_unit.set("m")
            if "Coarse" in json_data_voxel:
                self.voxel.voxel_filter.set(json_data["filter"])

        if "IFC" in json_data:
            json_data_ifc = json_data["IFC"]

            if "Default div" in json_data_ifc:
                self.div.use_default.set(json_data_ifc["Default div"])
            if "Ignore proxy" in json_data_ifc:
                self.div.ignore_proxy.set(json_data_ifc["Ignore proxy"])
            if "Div objects" in json_data_ifc:
                div_object_string = ""
                for div_object in json_data_ifc["Div objects"]:
                    div_object_string += div_object + "\t"

                self.div.div_objects.set(div_object_string)
            if "Ignore voids" in json_data_ifc:
                ignore_void_val = json_data_ifc["Ignore voids"]
                if ignore_void_val == 0:
                    self.div.simple_geo.set(False)
                else:
                    self.div.simple_geo.set(True)

            if "Fetch footprint elevation" in json_data_ifc:
                find_footprint_val = json_data_ifc["Fetch footprint elevation"]
                if find_footprint_val == True or find_footprint_val == 1:
                    self.footprint.find_footprint_elev.set(True)
                else:
                    self.footprint.find_footprint_elev.set(False)

            if "Ignore IsExternal" in json_data_ifc:
                ignore_IsExternal_val = json_data_ifc["Ignore IsExternal"]
                if ignore_IsExternal_val == True or ignore_IsExternal_val == 1:
                    self.other.ignoreIsExternal.set(True)
                else:
                    self.other.ignoreIsExternal.set(False)

        if "JSON" in json_data:
            json_data_json = json_data["JSON"]

            if "Footprint elevation" in json_data_json:
                self.footprint.footprint_elevation.set(json_data_json["Footprint elevation"])
                self.footprint.footprint_unit.set("m")

            if "Generate exterior" in json_data_json:
                gen_exterior_val = json_data_json["Generate exterior"]
                if gen_exterior_val == True or gen_exterior_val == 1:
                    self.other.make_exterior.set(True)
                else:
                    self.other.make_exterior.set(False)

            if "Generate interior" in json_data_json:
                gen_interior_val = json_data_json["Generate interior"]
                if gen_interior_val == True or gen_interior_val == 1:
                    self.other.make_interior.set(True)
                else:
                    self.other.make_interior.set(False)

            if "Generate footprint" in json_data_json:
                gen_footprint_val = json_data_json["Generate footprint"]
                if gen_footprint_val == True or gen_footprint_val == 1:
                    self.footprint.make_footprint.set(True)
                else:
                    self.footprint.make_footprint.set(False)

            if "Generate roof outline" in json_data_json:
                gen_roofprint_val = json_data_json["Generate roof outline"]
                if gen_roofprint_val == True or gen_roofprint_val == 1:
                    self.footprint.make_roofprint.set(True)
                else:
                    self.footprint.make_roofprint.set(False)

            if "Footprint based" in json_data_json:
                base_footprint_val = json_data_json["Footprint based"]
                if base_footprint_val == True or base_footprint_val == 1:
                    self.footprint.footprint_based.set(True)
                else:
                    self.footprint.footprint_based.set(False)

        if "Generate report" in json_data:
            gen_report_val = json_data["Generate report"]
            if gen_report_val == True or gen_report_val == 1:
                self.other.make_report.set(True)
            else:
                self.other.make_report.set(False)

        if "Output format" in json_data:
            json_data_format = json_data["Output format"]
            if "STEP file" in json_data_format:
                gen_step_val = json_data_format["STEP file"]
                if gen_step_val == True or gen_step_val == 1:
                    self.other.make_step.set(True)
            if "OBJ file" in json_data_format:
                gen_obj_val = json_data_format["OBJ file"]
                if gen_obj_val == True or gen_obj_val == 1:
                    self.other.make_obj.set(True)

        if "LoD output" in json_data:
            self.lod.clearLoD()
            json_data_lod = json_data["LoD output"]
            for lod in json_data_lod:

                if lod == "0.0" or lod == 0.0:
                    self.lod.lod00.set(1)
                if lod == "0.2" or lod == 0.2:
                    self.lod.lod02.set(1)
                if lod == "0.3" or lod == 0.3:
                    self.lod.lod03.set(1)
                if lod == "0.4" or lod == 0.4:
                    self.lod.lod04.set(1)
                if lod == "1.0" or lod == 1.0:
                    self.lod.lod10.set(1)
                if lod == "1.2" or lod == 1.2:
                    self.lod.lod12.set(1)
                if lod == "1.3" or lod == 1.3:
                    self.lod.lod13.set(1)
                if lod == "2.0" or lod == 2.0:
                    self.lod.lod20.set(1)
                if lod == "4.0" or lod == 4.0:
                    self.lod.lod40.set(1)
                if lod == "4.1" or lod == 4.1:
                    self.lod.lod41.set(1)
                if lod == "4.2" or lod == 4.2:
                    self.lod.lod42.set(1)
                if lod == "e.1":
                    self.lod.lode1.set(1)
                if lod == "3.2" or lod == 3.2:
                    self.lod.lod32.set(1)
                if lod == "5.0" or lod == 5.0:
                    self.lod.lod50.set(1)
        return;