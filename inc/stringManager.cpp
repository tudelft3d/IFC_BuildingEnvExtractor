#include "stringManager.h"

#include <string>
#include <map>

#include <nlohmann/json.hpp>

std::string CommunicationStringImportanceEnum::getString(CommunicationStringImportanceID id)
{
	//Include the spaces on the end for spacing
	switch (id) {
	case CommunicationStringImportanceID::indent:
		return "\t";
	case CommunicationStringImportanceID::info:
		return "[INFO] ";
	case CommunicationStringImportanceID::warning: 
		return "\033[33m[WARNING] ";
	case CommunicationStringImportanceID::error:
		return "\033[31m[Error] ";
	case CommunicationStringImportanceID::end:
		return "\033[0m";
	case CommunicationStringImportanceID::seperator:
		return "=============================================================";
	default:
		return "";
	}
}

std::string UnitStringEnum::getString(UnitStringID id)
{
	//Include the spaces on the end for spacing
	switch (id) {
	case UnitStringID::seconds:
		return "s";
	case UnitStringID::milliseconds:
		return "ms";
	case UnitStringID::meter:
		return "m";
	case UnitStringID::sqrMeter:
		return "m^2";
	case UnitStringID::cubMeter:
		return "m^3";
	case UnitStringID::meterFull:
		return "meter";
	case UnitStringID::millimeter:
		return "mm";
	case UnitStringID::millimeterFull:
		return "millimeter";
	case UnitStringID::footFull:
		return "foot";
	default:
		return "";
	}
}

std::string CommunicationStringEnum::getString(CommunicationStringID id)
{
	switch (id) {
	case CommunicationStringID::infoJsonRequest:
		return "Enter filepath of the config JSON";
	case CommunicationStringID::infoNoFilePath:
		return CommunicationStringImportanceEnum::getString(CommunicationStringImportanceID::info) + "No filepath has been supplied";
	case CommunicationStringID::infoNoValFilePath:
		return CommunicationStringImportanceEnum::getString(CommunicationStringImportanceID::info) + "No valid filepath has been supplied";
	case CommunicationStringID::infoParsingFile:
		return CommunicationStringImportanceEnum::getString(CommunicationStringImportanceID::info) + "Parsing file: ";
	case CommunicationStringID::infoParsingFiles:
		return CommunicationStringImportanceEnum::getString(CommunicationStringImportanceID::info) + "Parsing file(s): ";
	case CommunicationStringID::infoInternalizingGeo:
		return CommunicationStringImportanceEnum::getString(CommunicationStringImportanceID::info) + "Internalizing Geometry of Construction Model";
	case CommunicationStringID::infoFetchGroundfloorElev:
		return CommunicationStringImportanceEnum::getString(CommunicationStringImportanceID::info) + "Fetching elevation of the footprint from input";	
	case CommunicationStringID::infoGroundfloorElev:
		return CommunicationStringImportanceEnum::getString(CommunicationStringImportanceID::info) + "Ground floor elevation found: ";
	case CommunicationStringID::infoCreateSpatialIndex:
		return CommunicationStringImportanceEnum::getString(CommunicationStringImportanceID::info) + "Create Spatial Index";
	case CommunicationStringID::infoIgnoreVoids:
		return CommunicationStringImportanceEnum::getString(CommunicationStringImportanceID::info) + "Ignore voids";
	case CommunicationStringID::infoFoundUnits:
		return CommunicationStringImportanceEnum::getString(CommunicationStringImportanceID::info) + "Found units:";
	case CommunicationStringID::infoDefaultVolumeUnit:
		return CommunicationStringImportanceEnum::getString(CommunicationStringImportanceID::info) + "SI unit for volume is set to cubic metre";

	case CommunicationStringID::infoPreProcessing:
		return CommunicationStringImportanceEnum::getString(CommunicationStringImportanceID::info) + "Pre proccessing";
	case CommunicationStringID::infoCoarseFiltering:
		return CommunicationStringImportanceEnum::getString(CommunicationStringImportanceID::info) + "Coarse filtering of roofing structures";
	case CommunicationStringID::infoReduceSurfaces:
		return CommunicationStringImportanceEnum::getString(CommunicationStringImportanceID::info) + "Reduce surfaces";
	case CommunicationStringID::infoFineFiltering:
		return CommunicationStringImportanceEnum::getString(CommunicationStringImportanceID::info) + "Fine filtering of roofing structures";
	case CommunicationStringID::infoRoofOutlineConstruction:
		return CommunicationStringImportanceEnum::getString(CommunicationStringImportanceID::info) + "Construct roof outlines";
	case CommunicationStringID::infoRoofStructureSorting:
		return CommunicationStringImportanceEnum::getString(CommunicationStringImportanceID::info) + "Sort roofing structures";
	case CommunicationStringID::infoRoofStructureMerging:
		return CommunicationStringImportanceEnum::getString(CommunicationStringImportanceID::info) + "merge roofing structures";
	case CommunicationStringID::infoCoasreFootFiltering:
		return CommunicationStringImportanceEnum::getString(CommunicationStringImportanceID::info) + "Coarse filtering footprint at z = ";

	case CommunicationStringID::infoNoVoxelizationReq:
		return CommunicationStringImportanceEnum::getString(CommunicationStringImportanceID::info) + "No voxelization required";
	case CommunicationStringID::infoNocompleteVoxelizationReq:
		return CommunicationStringImportanceEnum::getString(CommunicationStringImportanceID::info) + "No complete voxelization required";
	case CommunicationStringID::infoInterioSpacesGrowing:
		return CommunicationStringImportanceEnum::getString(CommunicationStringImportanceID::info) + "Interior spaces growing";
	case CommunicationStringID::infoPairVoxels:
		return CommunicationStringImportanceEnum::getString(CommunicationStringImportanceID::info) + "Pair voxels";

	case CommunicationStringID::infoComputingStoreys02:
		return CommunicationStringImportanceEnum::getString(CommunicationStringImportanceID::info) + "Computing LoD0.2 Storeys";
	case CommunicationStringID::infoComputingStoreys03:
		return CommunicationStringImportanceEnum::getString(CommunicationStringImportanceID::info) + "Computing LoD0.3 Storeys";
	case CommunicationStringID::infoComputingLoD00:
		return CommunicationStringImportanceEnum::getString(CommunicationStringImportanceID::info) + "Computing LoD0.0 Model";
	case CommunicationStringID::infoComputingLoD02:
		return CommunicationStringImportanceEnum::getString(CommunicationStringImportanceID::info) + "Computing LoD0.2 Model";
	case CommunicationStringID::infoComputingLoD03:
		return CommunicationStringImportanceEnum::getString(CommunicationStringImportanceID::info) + "Computing LoD0.3 Model";
	case CommunicationStringID::infoComputingLoD04:
		return CommunicationStringImportanceEnum::getString(CommunicationStringImportanceID::info) + "Computing LoD0.4 Model";
	case CommunicationStringID::infoComputingLoD10:
		return CommunicationStringImportanceEnum::getString(CommunicationStringImportanceID::info) + "Computing LoD1.0 Model";
	case CommunicationStringID::infoComputingLoD12:
		return CommunicationStringImportanceEnum::getString(CommunicationStringImportanceID::info) + "Computing LoD1.2 Model";
	case CommunicationStringID::infoComputingLoD13:
		return CommunicationStringImportanceEnum::getString(CommunicationStringImportanceID::info) + "Computing LoD1.3 Model";
	case CommunicationStringID::infoComputingLoD22:
		return CommunicationStringImportanceEnum::getString(CommunicationStringImportanceID::info) + "Computing LoD2.2 Model";
	case CommunicationStringID::infoComputingLoDb0:
		return CommunicationStringImportanceEnum::getString(CommunicationStringImportanceID::info) + "Computing LoDb.0 Model";
	case CommunicationStringID::infoComputingLoDc1:
		return CommunicationStringImportanceEnum::getString(CommunicationStringImportanceID::info) + "Computing LoDc.1 Model";
	case CommunicationStringID::infoComputingLoDc2:
		return CommunicationStringImportanceEnum::getString(CommunicationStringImportanceID::info) + "Computing LoDc.2 Model";
	case CommunicationStringID::infoComputingLoDd1:
		return CommunicationStringImportanceEnum::getString(CommunicationStringImportanceID::info) + "Computing LoDd.1 Model";
	case CommunicationStringID::infoComputingLoDd2:
		return CommunicationStringImportanceEnum::getString(CommunicationStringImportanceID::info) + "Computing LoDd.2 Model";
	case CommunicationStringID::infoComputingLoDe1:
		return CommunicationStringImportanceEnum::getString(CommunicationStringImportanceID::info) + "Computing LoDe.1 Model";
	case CommunicationStringID::infoComputingLoD32:
		return CommunicationStringImportanceEnum::getString(CommunicationStringImportanceID::info) + "Computing LoD3.2 Model";
	case CommunicationStringID::infoComputingLoD40:
		return CommunicationStringImportanceEnum::getString(CommunicationStringImportanceID::info) + "Computing LoD4.0 Model";
	case CommunicationStringID::infoComputingLoD41:
		return CommunicationStringImportanceEnum::getString(CommunicationStringImportanceID::info) + "Computing LoD4.1 Model";
	case CommunicationStringID::infoComputingLoD42:
		return CommunicationStringImportanceEnum::getString(CommunicationStringImportanceID::info) + "Computing LoD4.2 Model";
	case CommunicationStringID::infoComputingLoD50:
		return CommunicationStringImportanceEnum::getString(CommunicationStringImportanceID::info) + "Computing LoD5.0 Model";

	case CommunicationStringID::infoAlternativeLoDCreationReq:
		return CommunicationStringImportanceEnum::getString(CommunicationStringImportanceID::indent) + "Data required: ";
	case CommunicationStringID::infoContinueOriginalProcess:
		return CommunicationStringImportanceEnum::getString(CommunicationStringImportanceID::info) + "Continuing original process: ";


	case CommunicationStringID::infoComputingInterior:
		return CommunicationStringImportanceEnum::getString(CommunicationStringImportanceID::info) + "Computing Interior";
	case CommunicationStringID::infoComputingExterior:
		return CommunicationStringImportanceEnum::getString(CommunicationStringImportanceID::info) + "Computing Exterior";
	case CommunicationStringID::infoComputingComplex:
		return CommunicationStringImportanceEnum::getString(CommunicationStringImportanceID::info) + "Computing Complex";

	case CommunicationStringID::infoComputingLoD50Rooms:
		return CommunicationStringImportanceEnum::getString(CommunicationStringImportanceID::info) + "Computing LoD 5.0 Rooms";
	case CommunicationStringID::infoExtractingSite:
		return CommunicationStringImportanceEnum::getString(CommunicationStringImportanceID::info) + "Extracting Site Data";
	case CommunicationStringID::infoPopulateGrid:
		return CommunicationStringImportanceEnum::getString(CommunicationStringImportanceID::info) + "Populate Grid";
	case CommunicationStringID::infoExteriorSpaceGrowing:
		return CommunicationStringImportanceEnum::getString(CommunicationStringImportanceID::info) + "Exterior space growing";

	case CommunicationStringID::indentValidIFCFound:
		return CommunicationStringImportanceEnum::getString(CommunicationStringImportanceID::indent) + "Valid IFC file found";
	case CommunicationStringID::indentcompIFCFound:
		return CommunicationStringImportanceEnum::getString(CommunicationStringImportanceID::indent) + "Compatible scheme found: ";
	case CommunicationStringID::indentSuccesFinished:
		return CommunicationStringImportanceEnum::getString(CommunicationStringImportanceID::indent) + "Successfully finished in: ";
	case CommunicationStringID::indentUnsuccesful:
		return CommunicationStringImportanceEnum::getString(CommunicationStringImportanceID::indent) + "Unsuccessful";
	case CommunicationStringID::indentStoreyAtZ:
		return CommunicationStringImportanceEnum::getString(CommunicationStringImportanceID::indent) + "Floorlevel at z = ";
	case CommunicationStringID::indentExteriorSpaceGrown:
		return CommunicationStringImportanceEnum::getString(CommunicationStringImportanceID::indent) + "Exterior space succesfully grown";
	case CommunicationStringID::indentNoExteriorSpace:
		return CommunicationStringImportanceEnum::getString(CommunicationStringImportanceID::indent) + "No exterior space has been found";
	case CommunicationStringID::indentInteriorSpaceGrown:
		return CommunicationStringImportanceEnum::getString(CommunicationStringImportanceID::indent) + "Interior space succesfully grown";
	case CommunicationStringID::indentPairedVoxels:
		return CommunicationStringImportanceEnum::getString(CommunicationStringImportanceID::indent) + "Voxel pairing succesful";
	default:
		return "Output string not found";
	}
}

std::string errorWarningStringEnum::getString(ErrorID id, bool withImportance)
{
	CommunicationStringImportanceID importance;
	std::string coms = "";

	switch (id) {
	case ErrorID::errorNoValFilePaths: {
		coms = "No valid filepath has been supplied";
		importance = CommunicationStringImportanceID::error;
		break;
	}
	case ErrorID::errorUnableToProcessFile: {
		coms = "Unable to process file(s)"; 
		importance = CommunicationStringImportanceID::error;
		break;
	}
	case ErrorID::errorNoUnits: {
		coms = "No unit assignment has been found";
		importance = CommunicationStringImportanceID::error;
		break;
	}
	case ErrorID::warningMultipleUnits: {
		coms = "Multiple unit assignments have been found";
		importance = CommunicationStringImportanceID::warning;
		break;
	}
	case ErrorID::errorNoLengthUnit: {
		coms = "SI unit for length cannot be found";
		importance = CommunicationStringImportanceID::error;
		break;
	}
	case ErrorID::errorNoAreaUnit: {
		coms = "SI unit for area cannot be found";
		importance = CommunicationStringImportanceID::error;
		break;
	}

	case ErrorID::errorJsonInvalBool: {
		coms = "JSON file does not contain a valid bool for entry ";
		importance = CommunicationStringImportanceID::error;
		break;
	}
	case ErrorID::errorJsonInvalInt: {
		coms = "JSON file does not contain a valid int for entry ";
		importance = CommunicationStringImportanceID::error;
		break;
	}
	case ErrorID::errorJsonInvalNegInt: {
		coms = "JSON file contains an invalid negative int for entry ";
		importance = CommunicationStringImportanceID::error;
		break;
	}
	case ErrorID::errorJsonInvalZeroInt: {
		coms = "JSON file contains an invalid zero int for entry ";
		importance = CommunicationStringImportanceID::error;
		break;
	}
	case ErrorID::errorJsonInvalNum: {
		coms = "JSON file does not contain a valid numeric value for entry ";
		importance = CommunicationStringImportanceID::error;
		break;
	}
	case ErrorID::errorJsonInvalString: {
		coms = "JSON file does not contain a valid for entry ";
		importance = CommunicationStringImportanceID::error;
		break;
	}
	case ErrorID::errorJsonInvalPath: {
		coms = "JSON file contains a path to a file with incorrect type for entry ";
		importance = CommunicationStringImportanceID::error;
		break;
	}
	case ErrorID::errorJsonNoRealPath: {
		coms = "JSON file contains an invalid path for entry ";
		importance = CommunicationStringImportanceID::error;
		break;
	}
	case ErrorID::errorJsonInvalArray: {
		coms = "JSON file does not contain a valid array for entry ";
		importance = CommunicationStringImportanceID::error;
		break;
	}
	case ErrorID::errorJsonInvalEntry: {
		coms = "JSON file does not contain a valid value for entry ";
		importance = CommunicationStringImportanceID::error;
		break;
	}

	case ErrorID::errorJsonMissingEntry: {
		coms = "JSON file does not contain required entry ";
		importance = CommunicationStringImportanceID::error;
		break;
	}
	case ErrorID::errorJsonInvalidLogic: {
		coms = "JSON file does not contain valid logic number (2 or 3) entry ";
		importance = CommunicationStringImportanceID::error;
		break;
	}

	case ErrorID::errorJsonInvalidLod: {
		coms = "JSON file contains unsupported required LoD";
		importance = CommunicationStringImportanceID::error;
		break;
	}

	case ErrorID::errorJsonMissingLoD: {
		coms = "No desired LoD output can be found";
		importance = CommunicationStringImportanceID::error;
		break;
	}
	case ErrorID::errorJsonNoDivObjects: {
		coms = "No div objects are selected";
		importance = CommunicationStringImportanceID::error;
		break;
	}
	case ErrorID::errorNoPoints: {
		coms = "No points could be extracted from the IFC file";
		importance = CommunicationStringImportanceID::error;
		break;
	}
	case ErrorID::errorFootprintFailed: {
		coms = "Footprint extraction failed";
		importance = CommunicationStringImportanceID::error;
		break;
	}
	case ErrorID::errorStoreyFailed: {
		coms = "storey extraction failed";
		importance = CommunicationStringImportanceID::error;
		break;
	}
	case ErrorID::errorLoD02StoreyFailed: {
		coms = "LoD0.2 Storey shape extraction failed";
		importance = CommunicationStringImportanceID::error;
		break;
	}

	case ErrorID::errorNoGroundFLoorFound: {
		coms = "No valid ground floor could be found, make sure ground floor name(s) start with '0' or '00'. Make sure to disable automatic footprint elevation detection and set the height manually if the input does not comply with the BIM BASIC IDS.";
		importance = CommunicationStringImportanceID::error;
		break;
	}
	case ErrorID::errorInconsistentGroundFLoorNumbers: {
		coms = "Ground floor could not be found in all input files. Make sure to disable automatic footprint elevation detection and set the height manually if the input does not comply with the BIM BASIC IDS.";
		importance = CommunicationStringImportanceID::error;
		break;
	}
	case ErrorID::errorInconsistentGroundFloorElevations: {
		coms = "Ground floor elevations are inconsisten across input files. Make sure to disable automatic footprint elevation detection and set the height manually if the input does not comply with the BIM BASIC IDS.";
		importance = CommunicationStringImportanceID::error;
		break;
	}	
	case ErrorID::errorVoxelCountOverflow: {
		coms = "Max allowed voxel count of 2.147.483.647 is surpassed. This can be caused by a too small voxel size, a too large input model or broken IfcObject translation data.";
		importance = CommunicationStringImportanceID::error;
		break;
	}

	case ErrorID::warningIfcUnableToParse: {
		coms = "Unable to parse .ifc file";
		importance = CommunicationStringImportanceID::warning;
		break;
	}
	case ErrorID::warningIfcNotValid: {
		coms = "No valid ifc scheme found in file";
		importance = CommunicationStringImportanceID::warning;
		break;
	}
	case ErrorID::warningIfcNoSchema: {
		coms = "No scheme found in file";
		importance = CommunicationStringImportanceID::warning;
		break;
	}
	case ErrorID::warningIfcIncomp: {
		coms = "Incompatible scheme found ";
		importance = CommunicationStringImportanceID::warning;
		break;
	}
	case ErrorID::warningIfcNoSlab: {
		coms = "During model orientation object class IfcSlab can not be found, alternative class is searched for";
		importance = CommunicationStringImportanceID::warning;
		break;
	}
	case ErrorID::warningIFCMissingType: {
		coms = "During model orientation object class could not be found: ";
		importance = CommunicationStringImportanceID::warning;
		break;
	}
	case ErrorID::warningIFCNoRotationClass: {
		coms = "During model orientation no suitable object class could be found, no rotation is applied";
		importance = CommunicationStringImportanceID::warning;
		break;
	}
	case ErrorID::warningIfcMultipleProjections: {
		coms = "Multiple map projections detected";
		importance = CommunicationStringImportanceID::warning;
		break;
	}
	case ErrorID::warningIfcNoVolumeUnit: {
		coms = "SI unit for volume cannot be found";
		importance = CommunicationStringImportanceID::warning;
		break;
	}
	case ErrorID::warningIfcDubSites: {
		coms = "More than one Site Element found, site export terminated";
		importance = CommunicationStringImportanceID::warning;
		break;
	}
	case ErrorID::warningIfcNoSites: {
		coms = "No Geographic or Site Element was found";
		importance = CommunicationStringImportanceID::warning;
		break;
	}
	case ErrorID::warningIfcSiteReconstructionFailed: {
		coms = "No site could be reconstructed";
		importance = CommunicationStringImportanceID::warning;
		break;
	}
	case ErrorID::warningIfcNoRoomObjects: {
		coms = "No room objects present in model, generic semantic data is created";
		importance = CommunicationStringImportanceID::warning;
		break;
	}
	case ErrorID::warningIfcMultipleUniqueObjects: {
		coms = "Multiple assumed unique objects in file";
		importance = CommunicationStringImportanceID::warning;
		break;
	}
	case ErrorID::warningIfcNoObjectName: {
		coms = "Object name could not be found in file";
		importance = CommunicationStringImportanceID::warning;
		break;
	}
	case ErrorID::warningIfcNoObjectNameLong: {
		coms = "Long Object name could not be found in file";
		importance = CommunicationStringImportanceID::warning;
		break;
	}
	case ErrorID::warningIfcObjectDifferentName: {
		coms = "Objects have different names in different files, name of first object is taken";
		importance = CommunicationStringImportanceID::warning;
		break;
	}
	case ErrorID::warningIfcMissingGeoreference: {
		coms = "Data required for georeferencing is missing";
		importance = CommunicationStringImportanceID::warning;
		break;
	}
	case ErrorID::warningIfcMissingIsExternal: {
		coms = "No objects found that were external, possibly attribute IsExternal is not correctly populated";
		importance = CommunicationStringImportanceID::warning;
		break;
	}
	case ErrorID::warningIfcIncorrectGeoRefTranslation: {
		coms = "Georeference projected coordinates lie out of bounds";
		importance = CommunicationStringImportanceID::warning;
		break;
	}


	case ErrorID::warningIssueencountered: {
		coms = "Encountered an issue";
		importance = CommunicationStringImportanceID::warning;
		break;
	}
	case ErrorID::warningNoSolid: {
		coms = "Not all shapes could be converted to solids, output might be incorrect or inaccurate";
		importance = CommunicationStringImportanceID::warning;
		break;
	}
	case ErrorID::warningUnableToMesh: {
		coms = "Not all shapes could be completely meshed, some surfaces are ignored";
		importance = CommunicationStringImportanceID::warning;
		break;
	}
	case ErrorID::warningUnableToSimplefy: {
		coms = "Not all shapes could be simplefied, unsimplefied shape is stored";
		importance = CommunicationStringImportanceID::warning;
		break;
	}
	case ErrorID::warningUnableToExtrude: {
		coms = "Not all surfaces can be extruded into a prism, most likely caused by non straight edges in the input model";
		importance = CommunicationStringImportanceID::warning;
		break;
	}
	case ErrorID::warningNoRoofOutline: {
		coms = "No roofoutline surface has been found";
		importance = CommunicationStringImportanceID::warning;
		break;
	}
	case ErrorID::warningNoFootprint: {
		coms = "No footprint surface has been found";
		importance = CommunicationStringImportanceID::warning;
		break;
	}
	case ErrorID::warningNonLinearEdges: {
		coms = "Face partially bound by non-linear edge/wire";
		importance = CommunicationStringImportanceID::warning;
		break;
	}
	case ErrorID::warningSimplefication: {
		coms = "Simple geometry is used, this can cause issues with windows and door detection";
		importance = CommunicationStringImportanceID::warning;
		break;
	}
	case ErrorID::warningInputIncFootprintElev: {
		coms = "Footprint elevation falls outside of the bounds of the model, lower bounds z value is used";
		importance = CommunicationStringImportanceID::warning;
		break;
	}
	

	case ErrorID::warningFailedObjectSimplefication: {
		coms = "Simplefication of complex object has failed";
		importance = CommunicationStringImportanceID::warning;
		break;
	}
	case ErrorID::warningFailedObjectConversion: {
		coms = "Unable to convert object shape";
		importance = CommunicationStringImportanceID::warning;
		break;
	}
	case ErrorID::errorNoObjects: {
		coms = "Unable to find selected div object class geometery";
		importance = CommunicationStringImportanceID::warning;
		break;
	}	
	case ErrorID::errorNoSpacesBAG: {
		coms = "Unable to create BAG geometry bacause no IfcSpace objects are present that comply with the 'ILS voor ruimten in de omgevingswet', make sure that the model complies or disable BAG refined LoD0.2 output";
		importance = CommunicationStringImportanceID::error;
		break;
	}

	default:
		return "Output string not found";
	}

	if (withImportance)
	{
		return CommunicationStringImportanceEnum::getString(importance) + coms + CommunicationStringImportanceEnum::getString(CommunicationStringImportanceID::end);
	}
	return coms;
}


std::string fileExtensionEnum::getString(fileExtensionID id)
{
	switch (id) {
	case fileExtensionID::JSON:
		return ".json";
	case fileExtensionID::OBJ:
		return ".obj";
	case fileExtensionID::STEP:
		return ".step";
	case fileExtensionID::dash:
		return "_";

	case fileExtensionID::interior:
		return "i";
	case fileExtensionID::exterior:
		return "e";

	case fileExtensionID::OBJLoD00:
		return getString(fileExtensionID::dash) + getString(fileExtensionID::exterior) + "LoD00" + getString(fileExtensionID::OBJ);
	case fileExtensionID::OBJLoD02:
		return getString(fileExtensionID::dash) + getString(fileExtensionID::exterior) + "LoD02" + getString(fileExtensionID::OBJ);
	case fileExtensionID::OBJLoD03:
		return getString(fileExtensionID::dash) + getString(fileExtensionID::exterior) + "LoD03" + getString(fileExtensionID::OBJ);
	case fileExtensionID::OBJLoD04:
		return getString(fileExtensionID::dash) + getString(fileExtensionID::exterior) + "LoD04" + getString(fileExtensionID::OBJ);
	case fileExtensionID::OBJLoD10:
		return getString(fileExtensionID::dash) + getString(fileExtensionID::exterior) + "LoD10" + getString(fileExtensionID::OBJ);
	case fileExtensionID::OBJLoD12:
		return getString(fileExtensionID::dash) + getString(fileExtensionID::exterior) + "LoD12" + getString(fileExtensionID::OBJ);
	case fileExtensionID::OBJLoD13:
		return getString(fileExtensionID::dash) + getString(fileExtensionID::exterior) + "LoD13" + getString(fileExtensionID::OBJ);
	case fileExtensionID::OBJLoD22:
		return getString(fileExtensionID::dash) + getString(fileExtensionID::exterior) + "LoD22" + getString(fileExtensionID::OBJ);
	case fileExtensionID::OBJLoDb:
		return getString(fileExtensionID::dash) + getString(fileExtensionID::exterior) + "LoDb" + getString(fileExtensionID::OBJ);
	case fileExtensionID::OBJLoDc1:
		return getString(fileExtensionID::dash) + getString(fileExtensionID::exterior) + "LoDc1" + getString(fileExtensionID::OBJ);	
	case fileExtensionID::OBJLoDd1:
		return getString(fileExtensionID::dash) + getString(fileExtensionID::exterior) + "LoDd0" + getString(fileExtensionID::OBJ);	
	case fileExtensionID::OBJLoD40:
		return getString(fileExtensionID::dash) + getString(fileExtensionID::exterior) + "LoD40" + getString(fileExtensionID::OBJ);
	case fileExtensionID::OBJLoD41:
		return getString(fileExtensionID::dash) + getString(fileExtensionID::exterior) + "LoD41" + getString(fileExtensionID::OBJ);
	case fileExtensionID::OBJLoD42:
		return getString(fileExtensionID::dash) + getString(fileExtensionID::exterior) + "LoD42" + getString(fileExtensionID::OBJ);
	case fileExtensionID::OBJLoDe1:
		return getString(fileExtensionID::dash) + getString(fileExtensionID::exterior) + "LoDe1" + getString(fileExtensionID::OBJ);
	case fileExtensionID::OBJLoD32:
		return getString(fileExtensionID::dash) + getString(fileExtensionID::exterior) + "LoD32" + getString(fileExtensionID::OBJ);
	case fileExtensionID::OBJLoD50:
		return getString(fileExtensionID::dash) + getString(fileExtensionID::exterior) + "LoD50" + getString(fileExtensionID::OBJ);

	case fileExtensionID::STEPLoD00:
		return getString(fileExtensionID::dash) + getString(fileExtensionID::exterior) + "LoD00" + getString(fileExtensionID::STEP);
	case fileExtensionID::STEPLoD02:
		return getString(fileExtensionID::dash) + getString(fileExtensionID::exterior) + "LoD02" + getString(fileExtensionID::STEP);
	case fileExtensionID::STEPLoD03:
		return getString(fileExtensionID::dash) + getString(fileExtensionID::exterior) + "LoD03" + getString(fileExtensionID::STEP);
	case fileExtensionID::STEPLoD04:
		return getString(fileExtensionID::dash) + getString(fileExtensionID::exterior) + "LoD04" + getString(fileExtensionID::STEP);
	case fileExtensionID::STEPLoD10:
		return getString(fileExtensionID::dash) + getString(fileExtensionID::exterior) + "LoD10" + getString(fileExtensionID::STEP);
	case fileExtensionID::STEPLoD12:
		return getString(fileExtensionID::dash) + getString(fileExtensionID::exterior) + "LoD12" + getString(fileExtensionID::STEP);
	case fileExtensionID::STEPLoD13:
		return getString(fileExtensionID::dash) + getString(fileExtensionID::exterior) + "LoD13" + getString(fileExtensionID::STEP);
	case fileExtensionID::STEPLoD22:
		return getString(fileExtensionID::dash) + getString(fileExtensionID::exterior) + "LoD22" + getString(fileExtensionID::STEP);
	case fileExtensionID::STEPLoDb:
		return getString(fileExtensionID::dash) + getString(fileExtensionID::exterior) + "LoDb" + getString(fileExtensionID::STEP);
	case fileExtensionID::STEPLoDc1:
		return getString(fileExtensionID::dash) + getString(fileExtensionID::exterior) + "LoDc1" + getString(fileExtensionID::STEP);
	case fileExtensionID::STEPLoDd1:
		return getString(fileExtensionID::dash) + getString(fileExtensionID::exterior) + "LoDd1" + getString(fileExtensionID::STEP);
	case fileExtensionID::STEPLoD40:
		return getString(fileExtensionID::dash) + getString(fileExtensionID::exterior) + "LoD40" + getString(fileExtensionID::STEP);
	case fileExtensionID::STEPLoD41:
		return getString(fileExtensionID::dash) + getString(fileExtensionID::exterior) + "LoD41" + getString(fileExtensionID::STEP);
	case fileExtensionID::STEPLoD42:
		return getString(fileExtensionID::dash) + getString(fileExtensionID::exterior) + "LoD42" + getString(fileExtensionID::STEP);
	case fileExtensionID::STEPLoDe1:
		return getString(fileExtensionID::dash) + getString(fileExtensionID::exterior) + "LoDe1" + getString(fileExtensionID::STEP);
	case fileExtensionID::STEPLoD32:
		return getString(fileExtensionID::dash) + getString(fileExtensionID::exterior) + "LoD32" + getString(fileExtensionID::STEP);
	case fileExtensionID::STEPLoD50:
		return getString(fileExtensionID::dash) + getString(fileExtensionID::exterior) + "LoD50" + getString(fileExtensionID::STEP);

	case fileExtensionID::OBJLoD02Interior:
		return getString(fileExtensionID::dash) + getString(fileExtensionID::interior) + "LoD02" + getString(fileExtensionID::OBJ);
	case fileExtensionID::OBJLoD03Interior:
		return getString(fileExtensionID::dash) + getString(fileExtensionID::interior) + "LoD03" + getString(fileExtensionID::OBJ);

	case fileExtensionID::STEPLoD02Interior:
		return getString(fileExtensionID::dash) + getString(fileExtensionID::interior) + "LoD02" + getString(fileExtensionID::STEP);
	case fileExtensionID::STEPLoD03Interior:
		return getString(fileExtensionID::dash) + getString(fileExtensionID::interior) + "LoD03" + getString(fileExtensionID::STEP);
	case fileExtensionID::STEPLoD12Interior:
		return getString(fileExtensionID::dash) + getString(fileExtensionID::interior) + "LoD12" + getString(fileExtensionID::STEP);
	case fileExtensionID::STEPLoD32Interior:
		return getString(fileExtensionID::dash) + getString(fileExtensionID::interior) + "LoD32" + getString(fileExtensionID::STEP);
	case fileExtensionID::STEPLoD50Interior:
		return getString(fileExtensionID::dash) + getString(fileExtensionID::interior) + "LoD50" + getString(fileExtensionID::STEP);

	default:
		return "";
	}

}


std::string sourceIdentifierEnum::getString(sourceIdentifierID id)
{
	switch (id) {
	case sourceIdentifierID::envExtractor:
		return "Env_ex ";
	case sourceIdentifierID::envExtractorVApprox:
		return "Env_vox ";
	case sourceIdentifierID::voxel:
		return "V_ex ";
	case sourceIdentifierID::ifc:
		return "IFC ";
	case sourceIdentifierID::ifcProp:
		return "IFC property";
	default:
		return "";
	}
}

std::string JsonObjectInEnum::getString(JsonObjectInID id)
{
	switch (id) {
	case JsonObjectInID::filePaths:
		return "Filepaths";
	case JsonObjectInID::filePathsInput:
		return "Input";
	case JsonObjectInID::filePathOutput:
		return "Output";
	case JsonObjectInID::filePathReport:
		return "Report";

	case JsonObjectInID::outputReport:
		return "Output report";

	case JsonObjectInID::voxel:
		return "Voxel";
	case JsonObjectInID::voxelSize:
		return "Size";
	case JsonObjectInID::voxelSummarize:
		return "Store values";
	case JsonObjectInID::voxelIntersection:
		return "Logic";
	case JsonObjectInID::voxelFilter:
		return "Coarse filter";

	case JsonObjectInID::tolerances:
		return "Tolerances";
	case JsonObjectInID::tolerancesLinear:
		return "Linear tolerance";
	case JsonObjectInID::tolerancesAngular:
		return "Angular tolerance";
	case JsonObjectInID::tolerancesArea:
		return "Area tolerance";
	case JsonObjectInID::meshLinearDeflection:
		return "Mesh linear deflection";	
	case JsonObjectInID::meshAngularDeflection:
		return "Mesh angular deflection";
	case JsonObjectInID::gridResolution:
		return "Grid resolution";

	case JsonObjectInID::IFC:
		return "IFC";
	case JsonObjectInID::IFCRotationAuto:
		return "Rotation auto";
	case JsonObjectInID::IFCRotationAngle:
		return "Rotation angle";
	case JsonObjectInID::IFCDefaultDiv:
		return "Default div";
	case JsonObjectInID::IFCIgnoreProxy:
		return "Ignore proxy";
	case JsonObjectInID::IFCDivObject:
		return "Div objects";
	case JsonObjectInID::IFCsimplefyGeo:
		return "Simplify geometry";
	case JsonObjectInID::IFCignoreVoids:
		return "Ignore voids";
	case JsonObjectInID::IFCignoreSimple:
		return "Ignore simplification";
	case JsonObjectInID::IFCignoreIsExternal:
		return "Ignore IsExternal";
	case JsonObjectInID::IFCCorrentPlacement:
		return "Correct placement";
	case JsonObjectInID::IFCDetectFootprintElev:
		return "Fetch footprint elevation";

	case JsonObjectInID::JSON:
		return "JSON";
	case JsonObjectInID::JSONGenInterior:
		return "Generate interior";
	case JsonObjectInID::JSONGenExterior:
		return "Generate exterior";	
	case JsonObjectInID::JSONGenSite:
		return "Generate site";
	case JsonObjectInID::JSONGenFootPrint:
		return "Generate footprint";
	case JsonObjectInID::JSONGenRoofOutline:
		return "Generate roof outline";
	case JsonObjectInID::JSONFootprintElev:
		return "Footprint elevation";
	case JsonObjectInID::JSONFootprintBShape:
		return "Footprint based";
	case JsonObjectInID::JSONSecOffset:
		return "Horizontal section offset";
	case JsonObjectInID::JSONGeoreference:
		return "Georeference";
	case JsonObjectInID::JSONMergeSemantics:
		return "Merge semantic objects";

	case JsonObjectInID::outputFormat:
		return "Output format";
	case JsonObjectInID::outputFormatJSON:
		return "JSON file";
	case JsonObjectInID::outputFormatOBJ:
		return "OBJ file";
	case JsonObjectInID::outputFormatSTEP:
		return "STEP file";
	
	case JsonObjectInID::lodOutput:
		return "LoD output";

	case JsonObjectInID::generateReport:
		return "Generate report";
	case JsonObjectInID::maxThread:
		return "Threads";
	default:
		return "";
	}
}

std::string CJObjectEnum::getString(CJObjectID id)
{
	switch (id) {
	case CJObjectID::metaDataTitle:
		return "Auto export from IfcEnvExtractor";
	case CJObjectID::outerShell:
		return "Outer Shell";
	case CJObjectID::innerShell:
		return "Inner Shell";

	case CJObjectID::CJType:
		return "type";
	case CJObjectID::CJTypeStorey:
		return "BuildingStorey";
	case CJObjectID::CJTypeFloor:
		return "FloorSurface";
	case CJObjectID::CJTypeOuterFloor:
		return "OuterFloorSurface";
	case CJObjectID::CJTypeRoofSurface:
		return "RoofSurface";
	case CJObjectID::CJTypeFloorSurface:
		return "FloorSurface";
	case CJObjectID::CJTypeGroundSurface:
		return "GroundSurface";
	case CJObjectID::CJTypeWallSurface:
		return "WallSurface";	
	case CJObjectID::CJTypeInteriorWallSurface:
		return "InteriorWallSurface";
	case CJObjectID::CJTypeSiteObject:
		return "Site";
	case CJObjectID::CJTypeWindow:
		return "Window";
	case CJObjectID::CJTypeDoor:
		return "Door";
	case CJObjectID::CJTypeNone:
		return "+None";
	case CJObjectID::CJTTypeCeilingSurface:
		return "CeilingSurface";
	case CJObjectID::CJTTypeOuterCeilingSurface:
		return "OuterCeilingSurface";	
	case CJObjectID::CJTTypeProjectedRoofOutline:
		return "+ProjectedRoofOutline";
	case CJObjectID::CJTTypeProjectedCeilingOutline:
		return "+ProjectedCeilingOutline";
	case CJObjectID::CJAttHasWindow:
		return "+hasWindows";

	case CJObjectID::True:
		return "True";
	case CJObjectID::False:
		return "False";

	case CJObjectID::voxelApproxShellVolume:
		return sourceIdentifierEnum::getString(sourceIdentifierID::envExtractorVApprox) + "shell volume";
	case CJObjectID::voxelApproxBuildingShellVolume:
		return sourceIdentifierEnum::getString(sourceIdentifierID::envExtractorVApprox) + "basement shell volume";
	case CJObjectID::voxelApproxBasementShellVolume:
		return sourceIdentifierEnum::getString(sourceIdentifierID::envExtractorVApprox) + "building shell volume";
	case CJObjectID::voxelApproxRoomVolumeTotal:
		return sourceIdentifierEnum::getString(sourceIdentifierID::envExtractorVApprox) + "total room volume";
	case CJObjectID::voxelApproxRoomArea:
		return sourceIdentifierEnum::getString(sourceIdentifierID::envExtractorVApprox) + "room area";
	case CJObjectID::voxelApproxShellArea:
		return sourceIdentifierEnum::getString(sourceIdentifierID::envExtractorVApprox) + "shell area";
	case CJObjectID::voxelApproxBasementShellArea:
		return sourceIdentifierEnum::getString(sourceIdentifierID::envExtractorVApprox) + "basement shell area";
	case CJObjectID::voxelApproxBuildingShellArea:
		return sourceIdentifierEnum::getString(sourceIdentifierID::envExtractorVApprox) + "building shell area";
	case CJObjectID::voxelApproxFootprintArea:
		return sourceIdentifierEnum::getString(sourceIdentifierID::envExtractorVApprox) + "footprint shell area";
	case CJObjectID::voxelApproxFaceadeOpeningArea:
		return sourceIdentifierEnum::getString(sourceIdentifierID::envExtractorVApprox) + "facade opening area";

	case CJObjectID::EnvVoxelSize:
		return sourceIdentifierEnum::getString(sourceIdentifierID::envExtractor) + "voxel size";
	case CJObjectID::EnvVoxelAnchor:
		return sourceIdentifierEnum::getString(sourceIdentifierID::envExtractor) + "voxelGrid anchor";
	case CJObjectID::EnvVoxelRotation:
		return sourceIdentifierEnum::getString(sourceIdentifierID::envExtractor) + "voxelGrid rotation";

	case CJObjectID::EnvLoDfloorArea:
		return sourceIdentifierEnum::getString(sourceIdentifierID::envExtractor) + "floor area LoD";

	case CJObjectID::v11:
		return "1.1";
	case CJObjectID::v20:
		return "2.0";

	case CJObjectID::jsonUom:
		return "uom";
	case CJObjectID::jsonValue:
		return "value";

	case CJObjectID::ifcDescription:
		return sourceIdentifierEnum::getString(sourceIdentifierID::ifc) + "Description";
	case CJObjectID::ifcObjectType:
		return sourceIdentifierEnum::getString(sourceIdentifierID::ifc) + "ObjectType";
	case CJObjectID::ifcName:
		return sourceIdentifierEnum::getString(sourceIdentifierID::ifc) + "Name";
	case CJObjectID::ifcLongName:
		return sourceIdentifierEnum::getString(sourceIdentifierID::ifc) + "Long Name";
	case CJObjectID::ifcElevation:
		return sourceIdentifierEnum::getString(sourceIdentifierID::ifc) + "Elevation";
	case CJObjectID::ifcGuid:
		return sourceIdentifierEnum::getString(sourceIdentifierID::ifc) + "Guid";
	default:
		return "";
	}
}
