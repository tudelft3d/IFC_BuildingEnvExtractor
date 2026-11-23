#include "helper.h"
#include "IOManager.h"
#include "cjCreator.h"
#include "stringManager.h"
#include "errorCollection.h"

#include <unordered_set>
#include <string>
#include <vector>

#include <sys/stat.h>
#include <boost/filesystem.hpp>


void addTimeToJSON(nlohmann::json* j, const std::string& valueName, const std::chrono::steady_clock::time_point& startTime, const std::chrono::steady_clock::time_point& endTime)
{
	long long duration = std::chrono::duration_cast<std::chrono::seconds>(endTime - startTime).count();
	if (duration < 5) { (*j)[valueName + UnitStringEnum::getString(UnitStringID::milliseconds)] = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime).count(); }
	else { (*j)[valueName + UnitStringEnum::getString(UnitStringID::seconds)] = duration; }
	return;
}

template<typename T>
void addTimeToJSON(nlohmann::json* j, const std::string& valueName, T duration)
{
	std::string timeUnitString = "Unit";
	std::string timeDurationString = "Duration";
	nlohmann::json timeSet;
	if (duration == 0) { return; }
	else if (duration < 5000)
	{
		timeSet[timeDurationString] = duration;
		timeSet[timeUnitString] = UnitStringEnum::getString(UnitStringID::milliseconds);
	}
	else
	{
		timeSet[timeDurationString] = duration / 1000;
		timeSet[timeUnitString] = UnitStringEnum::getString(UnitStringID::seconds);
	}
	(*j)[valueName] = timeSet;
	return;
}


bool IOManager::hasExtension(const std::string& string, const std::string& ext)
{
	std::string substring = boost::to_lower_copy<std::string>(string.substr(string.find_last_of(".") + 1));
	if (substring == ext) { return true; }
	return false;
}

bool IOManager::isValidPath(const std::string& path)
{
	struct stat info;
	if (stat(path.c_str(), &info) != 0) { return false; }
	return true;
}

std::string IOManager::getTargetPath()
{
	SettingsCollection& settingsCollection = SettingsCollection::getInstance();

	// preload communcation strings
	std::string stringJSONRequest = CommunicationStringEnum::getString(CommunicationStringID::infoJsonRequest);
	std::string stringNoFilePath = CommunicationStringEnum::getString(CommunicationStringID::infoNoFilePath);
	std::string stringNoValFilePath = CommunicationStringEnum::getString(CommunicationStringID::infoNoValFilePath);

	std::cout << stringJSONRequest << "\n";

	while (true)
	{
		std::cout << "Path: ";
		std::string singlepath = "";
		std::getline(std::cin, singlepath);

		if (singlepath.size() == 0 && settingsCollection.getIfcPathList().size() == 0)
		{
			std::cout << stringNoFilePath << "\n";
			std::cout << stringJSONRequest << "\n";
			continue;
		}
		if (singlepath.front() == '"' && singlepath.back() == '"')
		{
			singlepath = singlepath.substr(1, singlepath.size() - 2);
		}
		if (!hasExtension(singlepath, "json"))
		{
			std::cout << stringNoValFilePath << "\n";
			std::cout << stringJSONRequest << "\n";
			continue;
		}
		if (!isValidPath(singlepath))
		{
			std::cout << stringNoValFilePath << "\n";
			std::cout << stringJSONRequest << "\n";
			continue;
		}
		return singlepath;
	}
	return "";
}

bool IOManager::getJSONValues(const std::string& inputPath)
{
	SettingsCollection& settingsCollection = SettingsCollection::getInstance();

	// test if input configuration path is valid
	try { settingsCollection.setInputJSONPath(inputPath); }
	catch (const std::string& errorString) { throw errorString; }

	// read config file
	std::ifstream f(settingsCollection.getInputJSONPath());
	nlohmann::json json = nlohmann::json::parse(f);

	// in and output related settings
	// get filepath object
	try { 
		settingsCollection.setIOPaths(json); 
		settingsCollection.setWriteReport(json);
		settingsCollection.setThreadcount(json);
		settingsCollection.setTolerances(json);
		settingsCollection.setLoD(json);
		settingsCollection.setJSONRelatedSettings(json);
		settingsCollection.setVoxelRelatedSettings(json);
		settingsCollection.setIFCRelatedSettings(json);
		settingsCollection.setFormatRelatedSettings(json);
		settingsCollection.generateGeneralSettings();
	}
	catch (const std::string& errorString) { throw errorString; }
	return true;
}

void IOManager::printIOSummary() 
{
	SettingsCollection& settingsCollection = SettingsCollection::getInstance();

	std::cout << CommunicationStringImportanceEnum::getString(CommunicationStringImportanceID::info) << "I/O settings\n";
	std::cout << "- Configuration file:\n";
	std::cout << CommunicationStringImportanceEnum::getString(CommunicationStringImportanceID::indent) << settingsCollection.getInputJSONPath() << "\n";
	std::cout << "- Input File(s):\n";
	for (const std::string& inputPath : settingsCollection.getIfcPathList()) { std::cout << CommunicationStringImportanceEnum::getString(CommunicationStringImportanceID::indent) << inputPath << "\n"; }
	std::cout << "- Output File:\n";
	if (settingsCollection.createJSON())
	{
		std::cout << CommunicationStringImportanceEnum::getString(CommunicationStringImportanceID::indent) << settingsCollection.getOutputJSONPath() << "\n";
	}
	if (settingsCollection.createSTEP())
	{
		std::cout << CommunicationStringImportanceEnum::getString(CommunicationStringImportanceID::indent) << settingsCollection.getOutputSTEPPath() << "\n";
	}
	if (settingsCollection.createOBJ())
	{
		std::cout << CommunicationStringImportanceEnum::getString(CommunicationStringImportanceID::indent) << settingsCollection.getOutputOBJPath() << "\n";
	}
	if (settingsCollection.createDXF())
	{
		std::cout << CommunicationStringImportanceEnum::getString(CommunicationStringImportanceID::indent) << settingsCollection.getOutputDXFPath() << "\n";
	}

	std::cout << "- Create Report:\n";
	std::cout << boolToString(settingsCollection.writeReport()) << "\n";
	std::cout << "- Report File:\n";
	std::cout << CommunicationStringImportanceEnum::getString(CommunicationStringImportanceID::indent) << settingsCollection.getOutputReportPath() << "\n";
	std::cout << "- LoD export enabled:\n";
	std::cout << CommunicationStringImportanceEnum::getString(CommunicationStringImportanceID::indent) << getLoDEnabled() << "\n";
	std::cout << "- Output format:\n";
	std::string formatString = "";
	if (settingsCollection.createJSON()) { formatString += "JSON, "; }
	if (settingsCollection.createSTEP()) { formatString += "STEP, "; }
	if (settingsCollection.createOBJ()) { formatString += "OBJ, "; }
	if (settingsCollection.createDXF()) { formatString += "DXF, "; }
	formatString.erase(formatString.size() - 2);
	std::cout << CommunicationStringImportanceEnum::getString(CommunicationStringImportanceID::indent) << formatString << "\n\n";
	return;
}

void IOManager::printTolSummary()
{
	SettingsCollection& settingsCollection = SettingsCollection::getInstance();

	std::cout << CommunicationStringImportanceEnum::getString(CommunicationStringImportanceID::info) << "Tolerances\n";
	std::cout << "- Spatial tolerance:\n";
	std::cout << CommunicationStringImportanceEnum::getString(CommunicationStringImportanceID::indent) << settingsCollection.linearTolerance() << "\n";
	std::cout << "- angular tolerance:\n";
	std::cout << CommunicationStringImportanceEnum::getString(CommunicationStringImportanceID::indent) << settingsCollection.angularTolerance() << "\n";
	std::cout << "- area tolerance:\n";
	std::cout << CommunicationStringImportanceEnum::getString(CommunicationStringImportanceID::indent) << settingsCollection.areaTolerance() << "\n";
	std::cout << "- mesh linear deflection:\n";
	std::cout << CommunicationStringImportanceEnum::getString(CommunicationStringImportanceID::indent) << settingsCollection.meshLinearDeflection() << "\n";
	std::cout << "- grid resolution:\n";
	std::cout << CommunicationStringImportanceEnum::getString(CommunicationStringImportanceID::indent) << settingsCollection.surfaceGridSize() << "\n\n";
	return;
}

void IOManager::printIFCSummary()
{
	SettingsCollection& settingsCollection = SettingsCollection::getInstance();

	std::cout << CommunicationStringImportanceEnum::getString(CommunicationStringImportanceID::info) << "IFC settings\n";
	std::cout << "- Model rotation:\n";
	if (settingsCollection.autoRotateGrid()) { std::cout << CommunicationStringImportanceEnum::getString(CommunicationStringImportanceID::indent) << "automatic\n"; } //TODO: add true north?
	else
	{
		std::cout << CommunicationStringImportanceEnum::getString(CommunicationStringImportanceID::indent) << settingsCollection.desiredRotation() << "\n";
	}
	std::cout << "- Correct local placement\n";
	std::cout << boolToString(settingsCollection.correctPlacement()) << "\n";
	std::cout << "- Ignore voids:\n";
	std::cout << CommunicationStringImportanceEnum::getString(CommunicationStringImportanceID::indent) << settingsCollection.ignoreVoids() << "\n";
	std::cout << "- Simplify geometry:\n";
	std::cout << boolToString(settingsCollection.simplefyGeo()) << "\n";
	std::cout << "- Ignore IsExternal:\n";
	std::cout << boolToString(settingsCollection.ignoreIsExternal()) << "\n";
	std::cout << "- Ignore Complex:\n";
	std::cout << boolToString(settingsCollection.ignoreComplex()) << "\n";
	std::cout << "- Ignore simplification for: \n";
	std::cout << CommunicationStringImportanceEnum::getString(CommunicationStringImportanceID::indent) << settingsCollection.getIgnoreSimplificationList().size() << " object(s)" << std::endl;
	std::cout << "- Space dividing objects:\n";
	if (settingsCollection.useDefaultDiv())
	{
		const std::vector<std::string>& divObjects = settingsCollection.getDefaultDivList();
		for (auto it = divObjects.begin(); it != divObjects.end(); ++it) { std::cout << CommunicationStringImportanceEnum::getString(CommunicationStringImportanceID::indent) << *it << "\n";; }
	}
	if (settingsCollection.useProxy())
	{
		std::cout << CommunicationStringImportanceEnum::getString(CommunicationStringImportanceID::indent) << "IfcBuildingElementProxy\n";
	}
	std::vector<std::string> addDivObjects = settingsCollection.getCustomDivList();
	for (auto it = addDivObjects.begin(); it != addDivObjects.end(); ++it) { std::cout << CommunicationStringImportanceEnum::getString(CommunicationStringImportanceID::indent) << *it << "\n"; }
	std::cout << "\n";
	return;
}

void IOManager::printVoxelSummary() {
	SettingsCollection& settingsCollection = SettingsCollection::getInstance();

	std::cout << CommunicationStringImportanceEnum::getString(CommunicationStringImportanceID::info) << "voxel settings\n";
	std::cout << "- Voxel size:\n";
	std::cout << CommunicationStringImportanceEnum::getString(CommunicationStringImportanceID::indent) << settingsCollection.voxelSize() << "\n";
	std::cout << "- Voxel logic:\n";
	if (settingsCollection.intersectionLogic() == 2) { std::cout << CommunicationStringImportanceEnum::getString(CommunicationStringImportanceID::indent) << "plane\n"; }
	if (settingsCollection.intersectionLogic() == 3) { std::cout << CommunicationStringImportanceEnum::getString(CommunicationStringImportanceID::indent) << "solid\n"; }
	std::cout << "- Store voxel summary/approximation:\n";
	std::cout << boolToString(settingsCollection.summaryVoxels()) << "\n";
	std::cout << "- Coarse filter with voxel:\n";
	std::cout << boolToString(settingsCollection.voxelBasedFiltering()) << "\n\n";
	return;
}

void IOManager::printJSONSummary() {
	SettingsCollection& settingsCollection = SettingsCollection::getInstance();

	std::cout << CommunicationStringImportanceEnum::getString(CommunicationStringImportanceID::info) << "json settings\n";
	std::cout << "- Footprint Elevation:\n";
	if (settingsCollection.detectFootprintElevation())
	{
		std::cout << CommunicationStringImportanceEnum::getString(CommunicationStringImportanceID::indent) << "Fetch from file \n";
	}
	else
	{
		std::cout << CommunicationStringImportanceEnum::getString(CommunicationStringImportanceID::indent) << settingsCollection.footprintElevation() << "\n";
	}

	std::cout << "- BAG LoD0.2:\n";
	std::cout << boolToString(settingsCollection.makeBAG02()) << "\n";
	std::cout << "- Footrint based extraction:\n";
	std::cout << boolToString(settingsCollection.footPrintBased()) << "\n";
	std::cout << "- horizontal section offset\n";
	std::cout << CommunicationStringImportanceEnum::getString(CommunicationStringImportanceID::indent) << settingsCollection.horizontalSectionOffset() << "\n";;
	if (settingsCollection.make02())
	{
		std::cout << "- Create footprint:\n";
		std::cout << boolToString(settingsCollection.makeFootPrint()) << "\n";

		std::cout << "- Store Lod0.2 roof outline:\n";
		std::cout << boolToString(settingsCollection.makeRoofPrint()) << "\n";
	}
	std::cout << "- Generate interior:\n";
	std::cout << boolToString(settingsCollection.makeInterior()) << "\n";
	std::cout << "- Generate exterior:\n";
	std::cout << boolToString(settingsCollection.makeExterior()) << "\n";
	std::cout << "- Output site:\n";
	std::cout << boolToString(settingsCollection.makeSite()) << "\n";
	std::cout << "- Georeference:\n";
	std::cout << boolToString(settingsCollection.geoReference()) << "\n";
	std::cout << "- Merge semantic objects:\n";
	std::cout << boolToString(settingsCollection.mergeSemantics()) << "\n\n";
	return;
}

void IOManager::printSummary()
{
	SettingsCollection& settingsCollection = SettingsCollection::getInstance();

	std::cout << CommunicationStringImportanceEnum::getString(CommunicationStringImportanceID::seperator) << "\n\n";
	std::cout << CommunicationStringImportanceEnum::getString(CommunicationStringImportanceID::info) << "Used settings:\n\n";

	printIOSummary();
	printTolSummary();
	printIFCSummary();
	printVoxelSummary();
	printJSONSummary();

	std::cout << CommunicationStringImportanceEnum::getString(CommunicationStringImportanceID::info) << "other settings:\n";
	std::cout << "- Max thread count\n";
	std::cout << CommunicationStringImportanceEnum::getString(CommunicationStringImportanceID::indent) << settingsCollection.threadcount() << "\n\n";

	bool ignoreVoids = settingsCollection.ignoreVoids();
	if (ignoreVoids && settingsCollection.summaryVoxels() ||
		ignoreVoids && settingsCollection.makeV() ||
		ignoreVoids && settingsCollection.make32() ||
		ignoreVoids && settingsCollection.make40() ||
		ignoreVoids && settingsCollection.make41() ||
		ignoreVoids && settingsCollection.makee1())
	{
		std::cout << errorWarningStringEnum::getString(ErrorID::warningSimplefication) << "\n\n";
		ErrorCollection::getInstance().addError(ErrorID::warningSimplefication);
	}

	std::cout << CommunicationStringImportanceEnum::getString(CommunicationStringImportanceID::seperator) << "\n\n";
}

void IOManager::printErrors()
{
	SettingsCollection& settingsCollection = SettingsCollection::getInstance();
	ErrorCollection& errorCol = ErrorCollection::getInstance();

	std::cout << "[INFO] Warnings/Errors:\n";
	if (!errorCol.hasError())
	{
		std::cout << "\tCode 0\n";
		return;
	}

	for (const auto& error : errorCol.getErrorCollection())
	{
		ErrorObject currentError = error.second;
		std::cout << "\tCode " << currentError.errorCode_ << " : " << currentError.errorDescript_ << "\n";
	}

	if (!settingsCollection.writeReport())
	{
		std::cout << "\n[INFO] Rerun with generate report for more info\n";
	}
	else
	{
		std::cout << "\n[INFO] See report file for more info:\n\t" << settingsCollection.getOutputReportPath() << "\n";
	}
	return;
}

std::string IOManager::boolToString(const bool boolValue)
{
	if (boolValue) { return CommunicationStringImportanceEnum::getString(CommunicationStringImportanceID::indent) + "yes"; }
	return CommunicationStringImportanceEnum::getString(CommunicationStringImportanceID::indent) + "no";
}


std::string IOManager::getLoDEnabled()
{
	SettingsCollection& settingsCollection = SettingsCollection::getInstance();

	std::string summaryString = "";

	if (settingsCollection.make00()) { summaryString += ", 0.0"; }
	if (settingsCollection.make02()) { summaryString += ", 0.2"; }
	if (settingsCollection.make03()) { summaryString += ", 0.3"; }
	if (settingsCollection.make04()) { summaryString += ", 0.4"; }
	if (settingsCollection.make10()) { summaryString += ", 1.0"; }
	if (settingsCollection.make12()) { summaryString += ", 1.2"; }
	if (settingsCollection.make13()) { summaryString += ", 1.3"; }
	if (settingsCollection.make22()) { summaryString += ", 2.2"; }
	if (settingsCollection.makeb0()) { summaryString += ", b.0"; }
	if (settingsCollection.makec1()) { summaryString += ", c.1"; }
	if (settingsCollection.makec2()) { summaryString += ", c.2"; }
	if (settingsCollection.maked1()) { summaryString += ", d.1"; }
	if (settingsCollection.maked2()) { summaryString += ", d.2"; }
	if (settingsCollection.make40()) { summaryString += ", 4.0"; }
	if (settingsCollection.make41()) { summaryString += ", 4.1"; }
	if (settingsCollection.make42()) { summaryString += ", 4.2"; }
	if (settingsCollection.makee1()) { summaryString += ", e.1"; }
	if (settingsCollection.make32()) { summaryString += ", 3.2"; }
	if (settingsCollection.makeV()) { summaryString += ", 5.0 (V)"; }

	summaryString.erase(0, 2);

	return summaryString;
}

std::vector<std::string> IOManager::getLoDListEnabled() {
	SettingsCollection& settingsCollection = SettingsCollection::getInstance();

	std::vector<std::string> LoDList;
	if (settingsCollection.make00()) { LoDList.emplace_back("0.0"); }
	if (settingsCollection.make02()) { LoDList.emplace_back("0.2"); }
	if (settingsCollection.make03()) { LoDList.emplace_back("0.3"); }
	if (settingsCollection.make04()) { LoDList.emplace_back("0.4"); }
	if (settingsCollection.make10()) { LoDList.emplace_back("1.0"); }
	if (settingsCollection.make12()) { LoDList.emplace_back("1.2"); }
	if (settingsCollection.make13()) { LoDList.emplace_back("1.3"); }
	if (settingsCollection.make22()) { LoDList.emplace_back("2.2"); }
	if (settingsCollection.makeb0()) { LoDList.emplace_back("b.0"); }
	if (settingsCollection.makec1()) { LoDList.emplace_back("c.1"); }
	if (settingsCollection.makec2()) { LoDList.emplace_back("c.2"); }
	if (settingsCollection.maked1()) { LoDList.emplace_back("d.1"); }
	if (settingsCollection.maked2()) { LoDList.emplace_back("d.2"); }
	if (settingsCollection.makee1()) { LoDList.emplace_back("e.1"); }
	if (settingsCollection.make40()) { LoDList.emplace_back("4.0"); }
	if (settingsCollection.make41()) { LoDList.emplace_back("4.1"); }
	if (settingsCollection.make42()) { LoDList.emplace_back("4.2"); }
	if (settingsCollection.make32()) { LoDList.emplace_back("3.2"); }
	if (settingsCollection.makeV()) { LoDList.emplace_back("5.0"); }
	return LoDList;
}

nlohmann::json IOManager::getTolSettings() 
{
	SettingsCollection& settingsCollection = SettingsCollection::getInstance();

	nlohmann::json tolJSON;
	std::string spatialTolOName = JsonObjectInEnum::getString(JsonObjectInID::tolerancesLinear);
	std::string angularTolOName = JsonObjectInEnum::getString(JsonObjectInID::tolerancesAngular);
	std::string areaTolOName = JsonObjectInEnum::getString(JsonObjectInID::tolerancesArea);
	std::string meshLinDefOName = JsonObjectInEnum::getString(JsonObjectInID::meshLinearDeflection);
	std::string sufraceGridOName = JsonObjectInEnum::getString(JsonObjectInID::gridResolution);

	tolJSON[spatialTolOName] = settingsCollection.linearTolerance();
	tolJSON[angularTolOName] = settingsCollection.angularTolerance();
	tolJSON[areaTolOName] = settingsCollection.areaTolerance();
	tolJSON[meshLinDefOName] = settingsCollection.meshLinearDeflection();
	tolJSON[sufraceGridOName] = settingsCollection.surfaceGridSize();
	return tolJSON;
}

nlohmann::json IOManager::getVoxelSettings()
{
	SettingsCollection& settingsCollection = SettingsCollection::getInstance();

	nlohmann::json voxelJSON;
	std::string voxelSizOName = JsonObjectInEnum::getString(JsonObjectInID::voxelSize);
	std::string voxelSummarizeOName = JsonObjectInEnum::getString(JsonObjectInID::voxelSummarize);
	std::string voxelIntersectionOName = JsonObjectInEnum::getString(JsonObjectInID::voxelIntersection);
	std::string voxelFiltinerOName = JsonObjectInEnum::getString(JsonObjectInID::voxelFilter);

	voxelJSON[voxelSizOName] = settingsCollection.voxelSize();
	voxelJSON[voxelSummarizeOName] = settingsCollection.summaryVoxels();
	voxelJSON[voxelIntersectionOName] = settingsCollection.intersectionLogic();
	voxelJSON[voxelFiltinerOName] = settingsCollection.voxelBasedFiltering();
	return voxelJSON;
}

nlohmann::json IOManager::getIFCSettings()
{
	SettingsCollection& settingsCollection = SettingsCollection::getInstance();

	nlohmann::json ifcJSON;
	std::string ifcRotationOName = JsonObjectInEnum::getString(JsonObjectInID::IFCRotationAngle);
	std::string ifcDivOName = JsonObjectInEnum::getString(JsonObjectInID::IFCDivObject);
	std::string ifcignoreVoidOName = JsonObjectInEnum::getString(JsonObjectInID::IFCignoreVoids);
	std::string ifcSimpleOName = JsonObjectInEnum::getString(JsonObjectInID::IFCsimplefyGeo);
	std::string ifcIgnoreSimpleOName = JsonObjectInEnum::getString(JsonObjectInID::IFCignoreSimple);
	std::string ifcIgnoreComplex = JsonObjectInEnum::getString(JsonObjectInID::IFCignoreComplex);
	std::string IFCignoreIsExternalOName = JsonObjectInEnum::getString(JsonObjectInID::IFCignoreIsExternal);
	std::string ifcDetectFootprintElevation = JsonObjectInEnum::getString(JsonObjectInID::IFCDetectFootprintElev);

	ifcJSON[ifcRotationOName] = settingsCollection.gridRotation();
	std::vector<std::string> DivList;
	if (settingsCollection.useDefaultDiv())
	{
		const std::vector<std::string>& divObjects = settingsCollection.getDefaultDivList();
		for (auto it = divObjects.begin(); it != divObjects.end(); ++it)
		{
			DivList.emplace_back(*it);
		}
		if (settingsCollection.useProxy()) { DivList.emplace_back("IfcBuildingElementProxy"); }
		ifcJSON[ifcDivOName] = DivList;
	}
	else
	{
		ifcJSON[ifcDivOName] = settingsCollection.getCustomDivList();
	}
	ifcJSON[ifcignoreVoidOName] = settingsCollection.ignoreVoids();
	ifcJSON[ifcSimpleOName] = settingsCollection.simplefyGeo();
	ifcJSON[ifcIgnoreSimpleOName] = settingsCollection.getIgnoreSimplificationList();
	ifcJSON[ifcIgnoreComplex] = settingsCollection.ignoreComplex();
	ifcJSON[IFCignoreIsExternalOName] = settingsCollection.ignoreIsExternal();
	ifcJSON[ifcDetectFootprintElevation] = settingsCollection.detectFootprintElevation();
	return ifcJSON;
}

nlohmann::json IOManager::getJSONSettings()
{
	SettingsCollection& settingsCollection = SettingsCollection::getInstance();

	nlohmann::json jsonJSON;
	std::string jsonBAGrefNAme = JsonObjectInEnum::getString(JsonObjectInID::JSONBAG02);
	std::string jsonFootprintElevOName = JsonObjectInEnum::getString(JsonObjectInID::JSONFootprintElev);
	std::string jsonFootprintBSOName = JsonObjectInEnum::getString(JsonObjectInID::JSONFootprintBShape);
	std::string jsonSecOffsetOName = JsonObjectInEnum::getString(JsonObjectInID::JSONSecOffset);
	std::string jsonGenFootprintOName = JsonObjectInEnum::getString(JsonObjectInID::JSONGenFootPrint);
	std::string jsonGenRootprintOName = JsonObjectInEnum::getString(JsonObjectInID::JSONGenRoofOutline);
	std::string jsonGenInteriorOName = JsonObjectInEnum::getString(JsonObjectInID::JSONGenInterior);
	std::string jsonGenExteriorOName = JsonObjectInEnum::getString(JsonObjectInID::JSONGenExterior);
	std::string jsonGensiteOName = JsonObjectInEnum::getString(JsonObjectInID::JSONGenSite);
	std::string jsonGeoreferenceOName = JsonObjectInEnum::getString(JsonObjectInID::JSONGeoreference);
	std::string jsonMergeSemanticsOName = JsonObjectInEnum::getString(JsonObjectInID::JSONMergeSemantics);

	jsonJSON[jsonBAGrefNAme] = settingsCollection.makeBAG02();
	jsonJSON[jsonFootprintElevOName] = settingsCollection.footprintElevation();
	jsonJSON[jsonFootprintBSOName] = settingsCollection.footPrintBased();
	jsonJSON[jsonSecOffsetOName] = settingsCollection.horizontalSectionOffset();
	jsonJSON[jsonGenFootprintOName] = settingsCollection.makeFootPrint();
	if (settingsCollection.make02())
	{
		jsonJSON[jsonGenFootprintOName] = settingsCollection.makeFootPrint();
		jsonJSON[jsonGenRootprintOName] = settingsCollection.makeRoofPrint();
	}
	jsonJSON[jsonGenInteriorOName] = settingsCollection.makeInterior();
	jsonJSON[jsonGenExteriorOName] = settingsCollection.makeExterior();
	jsonJSON[jsonGensiteOName] = settingsCollection.makeSite();
	jsonJSON[jsonGeoreferenceOName] = settingsCollection.geoReference();
	jsonJSON[jsonMergeSemanticsOName] = settingsCollection.mergeSemantics();
	return jsonJSON;
}

nlohmann::json IOManager::settingsToJSON()
{
	SettingsCollection& settingsCollection = SettingsCollection::getInstance();

	nlohmann::json settingsJSON; // overal jsonFile

	// store the filepath data
	nlohmann::json ioJSON;
	std::string filePathsOName = JsonObjectInEnum::getString(JsonObjectInID::filePaths);
	std::string inputOName = JsonObjectInEnum::getString(JsonObjectInID::filePathsInput);
	std::string outputOName = JsonObjectInEnum::getString(JsonObjectInID::filePathOutput);

	ioJSON[inputOName] = settingsCollection.getIfcPathList();
	ioJSON[outputOName] = settingsCollection.getOutputJSONPath();
	settingsJSON[filePathsOName] = ioJSON;

	// store the report data
	settingsJSON[JsonObjectInEnum::getString(JsonObjectInID::outputReport)] = "true";

	// store the thread data
	settingsJSON[JsonObjectInEnum::getString(JsonObjectInID::maxThread)] = settingsCollection.threadcount();

	// store the LoD output data
	settingsJSON[JsonObjectInEnum::getString(JsonObjectInID::lodOutput)] = getLoDListEnabled();

	// store the tolerance data
	settingsJSON[JsonObjectInEnum::getString(JsonObjectInID::tolerances)] = getTolSettings();

	// store the voxel data
	settingsJSON[JsonObjectInEnum::getString(JsonObjectInID::voxel)] = getVoxelSettings();

	//store the ifc data
	settingsJSON[JsonObjectInEnum::getString(JsonObjectInID::IFC)] = getIFCSettings();

	// store the json data
	settingsJSON[JsonObjectInEnum::getString(JsonObjectInID::JSON)] = getJSONSettings();

	return settingsJSON;
}

void IOManager::internalizeGeo()
{
	// Time Collection Starts
	auto internalizingTime = std::chrono::high_resolution_clock::now();

	// internalize the helper data
	try
	{
		internalDataManager_->internalizeGeo();
		internalDataManager_->fetchGroundFloorElevation();
		internalDataManager_->indexGeo();
	}
	catch (const std::string& exceptionString)
	{
		throw exceptionString;
	}
	timeInternalizing_ = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - internalizingTime).count();
	return;
}

void IOManager::setMetaData(std::shared_ptr<CJT::CityCollection> collection)
{
	double scale = collection->getPrecision();
	CJT::ObjectTransformation transformation(scale);
	CJT::metaDataObject metaData;

	SettingsCollection& settingsCollection = SettingsCollection::getInstance();
	metaData.setTitle(CJObjectEnum::getString(CJObjectID::metaDataTitle));
	if (settingsCollection.geoReference())
	{
		internalDataManager_->getProjectionData(&transformation, &metaData);
	}

	transformation.setScale(transformation.getScale()[0]);

	// compute the extends
	gp_Pnt lll = internalDataManager_->getLllPoint();
	gp_Pnt urr = internalDataManager_->getUrrPoint();

	gp_Trsf gridRotation;
	gridRotation.SetRotation(gp_Ax1(gp_Pnt(0, 0, 0), gp_Vec(0, 0, 1)), -settingsCollection.gridRotation());
	TopoDS_Shape bboxGeo = helperFunctions::createBBOXOCCT(lll, urr).Moved(gridRotation);
	bboxGeo.Move(internalDataManager_->getObjectNormalizedTranslation());
	bg::model::box <BoostPoint3D> extents = helperFunctions::createBBox(bboxGeo);

	metaData.setExtend(
		CJT::CJTPoint(extents.min_corner().get<0>(), extents.min_corner().get<1>(), extents.min_corner().get<2>()),
		CJT::CJTPoint(extents.max_corner().get<0>(), extents.max_corner().get<1>(), extents.max_corner().get<2>())
	);

	collection->setTransformation(transformation); // set transformation early to avoid geo compression
	collection->setVersion(CJObjectEnum::getString(CJObjectID::v11));
	collection->setMetaData(metaData);
	return;
}

void IOManager::setDefaultSemantic(CJT::CityObject& cityBuildingObject, CJT::CityObject& cityComplexObject, CJT::CityObject& cityOuterShellObject, CJT::CityObject& cityInnerShellObject)
{
	std::string buildingName = "";
	nlohmann::json buildingAttributes;
	CJT::Building_Type rootType = CJT::Building_Type::Building;
	CJT::Building_Type childType = CJT::Building_Type::BuildingPart;

	int buildingTypeId = findBuildingType(internalDataManager_->getSourceFiles());

	switch (buildingTypeId)
	{
#if defined(USE_IFC4x2) || defined(USE_IFC4x3) || defined(USE_IFC4x3add1) || defined(USE_IFC4x3add2)
	case 2: // is brdige
		buildingName = internalDataManager_->getIfcObjectName<IfcSchema::IfcBridge>("IfcBridge", false);
		buildingAttributes = internalDataManager_->getBuildingInformation<IfcSchema::IfcBridge>();
		rootType = CJT::Building_Type::Bridge;
		childType = CJT::Building_Type::BridgePart;
		break;
#endif // USE_IFC4x3
#if defined(USE_IFC4x3) || defined(USE_IFC4x3add1) || defined(USE_IFC4x3add2)
	case 3:  // is road
		buildingName = internalDataManager_->getIfcObjectName<IfcSchema::IfcRoad>("IfcRoad", false);
		buildingAttributes = internalDataManager_->getBuildingInformation<IfcSchema::IfcRoad>();
		rootType = CJT::Building_Type::Road;
		childType = CJT::Building_Type::Road;
		break;
	case 4: // is railway
		buildingName = internalDataManager_->getIfcObjectName<IfcSchema::IfcRailway>("IfcRailway", false);
		buildingAttributes = internalDataManager_->getBuildingInformation<IfcSchema::IfcRailway>();
		rootType = CJT::Building_Type::Railway;
		childType = CJT::Building_Type::Railway;
		break;
	case 5: // is marinefacilty
		buildingName = internalDataManager_->getIfcObjectName<IfcSchema::IfcMarineFacility>("IfcMarineFacility", false);
		buildingAttributes = internalDataManager_->getBuildingInformation<IfcSchema::IfcMarineFacility>();
		break;
#endif
	default:
	case 1: // is building
		buildingName = internalDataManager_->getIfcObjectName<IfcSchema::IfcBuilding>("IfcBuilding", false);
		buildingAttributes = internalDataManager_->getBuildingInformation<IfcSchema::IfcBuilding>();
		break;
	}

	// Set up objects and their relationships
	if (buildingName == "") { buildingName = internalDataManager_->getIfcObjectName<IfcSchema::IfcSite>("IfcSite", false); }
	cityBuildingObject.addAttributes(buildingAttributes);

	cityBuildingObject.setName(buildingName);
	cityComplexObject.setName("Complex");

	cityOuterShellObject.setName(CJObjectEnum::getString(CJObjectID::outerShell));
	cityInnerShellObject.setName(CJObjectEnum::getString(CJObjectID::innerShell));

	cityBuildingObject.setType(rootType);
	cityComplexObject.setType(childType);
	cityOuterShellObject.setType(childType);
	cityInnerShellObject.setType(childType);

	if (SettingsCollection::getInstance().makeExterior()) { cityBuildingObject.addChild(&cityOuterShellObject); }
	if (SettingsCollection::getInstance().makeInterior()) { cityBuildingObject.addChild(&cityInnerShellObject); }
	if (SettingsCollection::getInstance().makeComplex()) { cityBuildingObject.addChild(&cityComplexObject); }
	return;
}

void IOManager::setComputedSemantic(CJGeoCreator* geoCreator, CJT::CityObject& cityBuildingObject, CJT::CityObject& cityOuterShellObject, CJT::CityObject& cityInnerShellObject)
{
	SettingsCollection& settingsCollection = SettingsCollection::getInstance();
	cityOuterShellObject.addAttribute(sourceIdentifierEnum::getString(sourceIdentifierID::envExtractor) + "footprint elevation", settingsCollection.footprintElevation());
	cityBuildingObject.addAttribute(sourceIdentifierEnum::getString(sourceIdentifierID::envExtractor) + "buildingHeight", internalDataManager_->getUrrPoint().Z() - settingsCollection.footprintElevation());

	int storeyFloors = 0;
	int basementFloors = 0;
	ComputeStoreysAboveGround(&storeyFloors, &basementFloors);
	cityBuildingObject.addAttribute(sourceIdentifierEnum::getString(sourceIdentifierID::envExtractor) + "storeysAboveGround", storeyFloors);
	cityBuildingObject.addAttribute(sourceIdentifierEnum::getString(sourceIdentifierID::envExtractor) + "storeysBelowGround", basementFloors);

	// find location of main entrance (CHEK related code)
	std::unique_ptr<gp_Pnt> entranceCoord = nullptr;
	entranceCoord = FetchMainEntranceLocation(std::move(entranceCoord));
	if (entranceCoord != nullptr)
	{
		cityBuildingObject.addAttribute(sourceIdentifierEnum::getString(sourceIdentifierID::envExtractor) + "Main Entrance coordinate", 
			std::to_string(entranceCoord->X()) + ", " + 
			std::to_string(entranceCoord->Y()) + ", " +
			std::to_string(entranceCoord->Z()));
	}

	if (settingsCollection.summaryVoxels())
	{
		geoCreator->extractOuterVoxelSummary(
			&cityOuterShellObject,
			internalDataManager_.get(),
			settingsCollection.footprintElevation(),
			-settingsCollection.gridRotation()
		);

		geoCreator->extractInnerVoxelSummary(
			&cityInnerShellObject,
			internalDataManager_.get()
		);
	}
	return;
}

void IOManager::ComputeStoreysAboveGround(int* storeysAboveGround, int* storeysBelowGround)
{
	IfcSchema::IfcBuildingStorey::list::ptr storeyList = internalDataManager_->getSourceFile(0)->instances_by_type<IfcSchema::IfcBuildingStorey>();

	double groundHeight = SettingsCollection::getInstance().footprintElevation();
	double smallestDistance = 1000000;
	double groundFloorHeight = 0;
	std::vector<double> storeyHeights;
	for (auto it = storeyList->begin(); it != storeyList->end(); ++it)
	{
		IfcSchema::IfcBuildingStorey* storeyObject = *it;
		double floorHeight = storeyObject->Elevation().get() * internalDataManager_->getScaler(0);
		storeyHeights.emplace_back(floorHeight);

		double distanceToGround = abs(floorHeight - groundHeight);

		if (distanceToGround < smallestDistance)
		{
			smallestDistance = distanceToGround;
			groundFloorHeight = floorHeight;
		}
	}

	int storeyFloors = 0;
	int basementFloors = 0;
	for (double floorHeight : storeyHeights)
	{
		if (floorHeight >= groundFloorHeight) { storeyFloors++; }
		else { basementFloors++; }
	}
	*storeysAboveGround = storeyFloors;
	*storeysBelowGround = basementFloors;

	return;
}

std::unique_ptr<gp_Pnt> IOManager::FetchMainEntranceLocation(std::unique_ptr<gp_Pnt> location)
{
	for (size_t i = 0; i < internalDataManager_->getSourceFileCount(); i++)
	{
		IfcParse::IfcFile* currentFile = internalDataManager_->getSourceFile(i);
		IfcSchema::IfcDoor::list::ptr doorlist  = currentFile->instances_by_type<IfcSchema::IfcDoor>();

		for (auto it = doorlist->begin(); it != doorlist->end(); ++it)
		{
			IfcSchema::IfcDoor* currentDoor = *it;
			nlohmann::json attributeList = helperFunctions::getAttributes(currentDoor);

			if (!attributeList.contains("CHEK_IsMainEntrance"))
			{
				continue;
			}

			// if is main entrance
			TopoDS_Shape currentShape = internalDataManager_->getObjectShape(currentDoor, false, true);
			gp_Pnt lll;
			gp_Pnt urr;
			helperFunctions::bBoxDiagonal(currentShape, &lll, &urr);

			gp_Pnt centerPoint = gp_Pnt(
			(lll.X() - urr.X()) / 2,
			(lll.Y() - urr.Y()) / 2,
			lll.Z());

			location = std::make_unique<gp_Pnt>(centerPoint);
			return location;
		}
	}
	return nullptr;
}

int IOManager::findBuildingType(const std::vector<IfcParse::IfcFile*>& fileList )
{
	int buildingCount = fileList[0]->instances_by_type<IfcSchema::IfcBuilding>()->size();
	int bridgeCount = 0;
	int roadCount = 0;
	int railwayCount = 0;
	int marineCount = 0;

#if defined(USE_IFC4x2) || defined(USE_IFC4x3) || defined(USE_IFC4x3add1) || defined(USE_IFC4x3add2)
	bridgeCount = fileList[0]->instances_by_type<IfcSchema::IfcBridge>()->size();
#endif // USE_IFC4x3

#if defined(USE_IFC4x3) || defined(USE_IFC4x3add1) || defined(USE_IFC4x3add2)
	roadCount = fileList[0]->instances_by_type<IfcSchema::IfcRoad>()->size();
	railwayCount = fileList[0]->instances_by_type<IfcSchema::IfcRailway>()->size();
	marineCount = fileList[0]->instances_by_type<IfcSchema::IfcMarineFacility>()->size();
#endif

	if ((buildingCount > 0) + (bridgeCount > 0) + (roadCount > 0) + (railwayCount > 0) + (marineCount > 0) > 1)
	{
		std::cout << errorWarningStringEnum::getString(ErrorID::warningIfcMistypedSpatialStructures) << "\n\n";
	}

	auto checkCount = [](int count, const char* name)
	{
		std::cout << CommunicationStringEnum::getString(CommunicationStringID::infoConstructionTypeFound) << name << "\n\n";
		if (count > 1)
		{
			std::cout << errorWarningStringEnum::getString( ErrorID::warningIfcMultipleSpatialStructures) << name << "\n\n";
		}
		return;
	};

	if (buildingCount != 0)
	{
		checkCount(buildingCount, "IfcBuilding");
		return 1;		
	}
	if (bridgeCount > 1)
	{
		checkCount(bridgeCount, "IfcBridge");
		return 2;		
	}
	if (roadCount > 1)
	{
		checkCount(roadCount, "IfcRoad");
		return 3;	
	}
	if (railwayCount > 1)
	{
		checkCount(railwayCount, "IfcRailway");
		return 4;		
	}
	if (marineCount > 1)
	{
		checkCount(marineCount, "IfcMarineFacility");
		return 5;		
	}
	return 0;
}

void IOManager::createCompanionFile()
{
	CJT::ObjectTransformation transformation(1);
	CJT::metaDataObject metaData;

	SettingsCollection& settingsCollection = SettingsCollection::getInstance();
	metaData.setTitle(CJObjectEnum::getString(CJObjectID::metaDataTitle));
	gp_Trsf translation = internalDataManager_->getObjectNormalizedTranslation();

	nlohmann::json companionJson;
	nlohmann::json referencingJson;

	referencingJson["referenceSystem"] = internalDataManager_->getProjection();
	referencingJson["translate"] = { translation.TranslationPart().X(), translation.TranslationPart().Y(), translation.TranslationPart().Z() };
	companionJson["georeference"] = referencingJson;

	std::ofstream companionFile(settingsCollection.getOutputBasePath() + "_georef_companion.json");
	companionFile << companionJson.dump(4);
	companionFile.close();
	return;
}


void IOManager::processExternalLoD(CJGeoCreator* geoCreator, CJT::CityObject& cityOuterShellObject, CJT::Kernel* kernel)
{
	std::cout << CommunicationStringEnum::getString(CommunicationStringID::infoComputingExterior) << std::endl;
	SettingsCollection& settingsCollection = SettingsCollection::getInstance();

	if (settingsCollection.make02() && settingsCollection.makeFootPrint() ||
		settingsCollection.make03() && settingsCollection.makeFootPrint() ||
		settingsCollection.make04() && settingsCollection.makeFootPrint() ||
		settingsCollection.footPrintBased() ||
		settingsCollection.makeb0()
		)
	{
		try
		{
			geoCreator->makeFootprint(internalDataManager_.get());
		}
		catch (const std::exception&)
		{
			ErrorCollection::getInstance().addError(ErrorID::errorFootprintFailed);
			succesfullExit_ = 0;
		}
	}

	processExternalLoD(settingsCollection.make00(), [&]() {
		return std::vector<CJT::GeoObject>{geoCreator->makeLoD00(internalDataManager_.get(), kernel, 1)};
		}, cityOuterShellObject, ErrorID::failedLoD00, timeLoD00_);

	processExternalLoD(settingsCollection.make02(), [&]() {
		return std::vector<CJT::GeoObject>{geoCreator->makeLoD02(internalDataManager_.get(), kernel, 1)};
		}, cityOuterShellObject, ErrorID::failedLoD02, timeLoD02_);

	processExternalLoD(settingsCollection.make03(), [&]() {
		return std::vector<CJT::GeoObject>{geoCreator->makeLoD03(internalDataManager_.get(), kernel, 1)};
		}, cityOuterShellObject, ErrorID::failedLoD03, timeLoD03_);

	processExternalLoD(settingsCollection.make04(), [&]() {
		return std::vector<CJT::GeoObject>{geoCreator->makeLoD04(internalDataManager_.get(), kernel, 1)};
		}, cityOuterShellObject, ErrorID::failedLoD04, timeLoD04_);

	processExternalLoD(settingsCollection.make10(), [&]() {
		return std::vector<CJT::GeoObject>{geoCreator->makeLoD10(internalDataManager_.get(), kernel, 1)};
		}, cityOuterShellObject, ErrorID::failedLoD10, timeLoD10_);

	processExternalLoD(settingsCollection.make12(), [&]() {
		return std::vector<CJT::GeoObject>{geoCreator->makeLoD12(internalDataManager_.get(), kernel, 1)};
		}, cityOuterShellObject, ErrorID::failedLoD12, timeLoD12_);

	processExternalLoD(settingsCollection.make13(), [&]() {
		return std::vector<CJT::GeoObject>{geoCreator->makeLoD13(internalDataManager_.get(), kernel, 1)};
		}, cityOuterShellObject, ErrorID::failedLoD13, timeLoD13_);

	processExternalLoD(settingsCollection.make22(), [&]() {
		return std::vector<CJT::GeoObject>{geoCreator->makeLoD22(internalDataManager_.get(), kernel, 1)};
		}, cityOuterShellObject, ErrorID::failedLoD22, timeLoD22_);

	processExternalLoD(settingsCollection.makeb0(), [&]() {
		return std::vector<CJT::GeoObject>{geoCreator->makeLoDb0(internalDataManager_.get(), kernel, 1)};
		}, cityOuterShellObject, ErrorID::failedLoDb0, timeLoDb0_);

	processExternalLoD(settingsCollection.makec1(), [&]() {
		return std::vector<CJT::GeoObject>{geoCreator->makeLoDc1(internalDataManager_.get(), kernel, 1)};
		}, cityOuterShellObject, ErrorID::failedLoDc1, timeLoDc1_);

	processExternalLoD(settingsCollection.makec2(), [&]() {
		return std::vector<CJT::GeoObject>{geoCreator->makeLoDc2(internalDataManager_.get(), kernel, 1)};
		}, cityOuterShellObject, ErrorID::failedLoDc2, timeLoDc2_);

	processExternalLoD(settingsCollection.maked1(), [&]() {
		return std::vector<CJT::GeoObject>{geoCreator->makeLoDd1(internalDataManager_.get(), kernel, 1)};
		}, cityOuterShellObject, ErrorID::failedLoDd1, timeLoDd1_);

	processExternalLoD(settingsCollection.maked2(), [&]() {
		return std::vector<CJT::GeoObject>{geoCreator->makeLoDd2(internalDataManager_.get(), kernel, 1)};
		}, cityOuterShellObject, ErrorID::failedLoDd2, timeLoDd2_);

	processExternalLoD(settingsCollection.makee1(), [&]() {
		return std::vector<CJT::GeoObject>{geoCreator->makeLoDe1(internalDataManager_.get(), kernel, 1)};
		}, cityOuterShellObject, ErrorID::failedLoDe1, timeLoDe1_);

	processExternalLoD(settingsCollection.make32(), [&]() {
		return std::vector<CJT::GeoObject>{geoCreator->makeLoD32(internalDataManager_.get(), kernel, 1)};
		}, cityOuterShellObject, ErrorID::failedLoD32, timeLoD32_);

	processExternalLoD(settingsCollection.makeV(), [&]() {
		return std::vector<CJT::GeoObject>{geoCreator->makeV(internalDataManager_.get(), kernel, 1)};
		}, cityOuterShellObject, ErrorID::failedLoD50, timeV_);

	std::cout << "\n";
	return;
}

void IOManager::processExternalLoD(
	bool process,
	const std::function<std::vector<CJT::GeoObject>()>& lodCreationFunc,
	CJT::CityObject& cityOuterShellObject,
	ErrorID errorID,
	long long& timeRecord
)
{
	if (process)
	{
		auto startTimeGeoCreation = std::chrono::high_resolution_clock::now();
		try
		{
			std::vector<CJT::GeoObject> geoObjects = lodCreationFunc();
			for (size_t i = 0; i < geoObjects.size(); i++) { cityOuterShellObject.addGeoObject(geoObjects[i]); }
		}
		catch (const std::exception&)
		{
			ErrorCollection::getInstance().addError(errorID);
			succesfullExit_ = 0;
		}
		timeRecord = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - startTimeGeoCreation).count();
	}
	return;
}

void IOManager::processComplexlLoD(CJGeoCreator* geoCreator, CJT::CityObject& cityComplexObject, CJT::Kernel* kernel)
{
	std::cout << CommunicationStringEnum::getString(CommunicationStringID::infoComputingComplex) << std::endl;
	SettingsCollection& settingsCollection = SettingsCollection::getInstance();

	processExternalLoD(settingsCollection.make40(), [&]() {
		return std::vector<CJT::GeoObject>{geoCreator->makeLoD40(internalDataManager_.get(), kernel, 1)};
		}, cityComplexObject, ErrorID::failedLoD40, timeLoD40_);

	processExternalLoD(settingsCollection.make41(), [&]() {
		return std::vector<CJT::GeoObject>{geoCreator->makeLoD41(internalDataManager_.get(), kernel, 1)};
		}, cityComplexObject, ErrorID::failedLoD41, timeLoD41_);

	processExternalLoD(settingsCollection.make42(), [&]() {
		return std::vector<CJT::GeoObject>{geoCreator->makeLoD42(internalDataManager_.get(), kernel, 1)};
		}, cityComplexObject, ErrorID::failedLoD42, timeLoD42_);

	return;
}


void IOManager::processInteriorLod(CJGeoCreator* geoCreator, std::shared_ptr<CJT::CityCollection> collection, CJT::CityObject* cityInnerShellObject, CJT::Kernel* kernel)
{
	std::cout << CommunicationStringEnum::getString(CommunicationStringID::infoComputingInterior) << std::endl;

	SettingsCollection& settingsCollection = SettingsCollection::getInstance();

	// get storey semantic objects
	std::vector<CJT::CityObject> storeyObjects = geoCreator->makeStoreyObjects(internalDataManager_.get());
	std::vector<CJT::CityObject> roomObjects = {};

	if (settingsCollection.make02() || settingsCollection.make12() || 
		settingsCollection.make22() || settingsCollection.make32() ||
		settingsCollection.makeV())
	{
		roomObjects = geoCreator->makeRoomObjects(internalDataManager_.get(), storeyObjects);
	}

	// storeys
	if (settingsCollection.make02())
	{
		geoCreator->make2DStoreys(internalDataManager_.get(), kernel, storeyObjects, 1, false);
	}
	if (settingsCollection.make03())
	{
		geoCreator->make2DStoreys(internalDataManager_.get(), kernel, storeyObjects, 1, true);
	}
	if (settingsCollection.make02() || settingsCollection.make12() || settingsCollection.make22())
	{
		geoCreator->makeSimpleLodRooms(internalDataManager_.get(), kernel, roomObjects, 1);
	}

	if (settingsCollection.make32())
	{
		geoCreator->makeComplexLoDRooms(internalDataManager_.get(), kernel, roomObjects, 1);
	}

	// rooms
	if (settingsCollection.makeV())
	{
		geoCreator->makeVRooms(internalDataManager_.get(), kernel, storeyObjects, roomObjects, 1);
	}

	for (size_t i = 0; i < storeyObjects.size(); i++)
	{
		storeyObjects[i].addParent(cityInnerShellObject);
		collection->addCityObject(storeyObjects[i]);
	}

	for (size_t i = 0; i < roomObjects.size(); i++)
	{
		collection->addCityObject(roomObjects[i]);
	}
	std::cout << "\n";
	return;
}

void IOManager::processSitelod(CJGeoCreator* geoCreator, std::shared_ptr<CJT::CityCollection> collection, CJT::CityObject* cityBuildingObject, CJT::Kernel* kernel)
{
	auto startTimeGeoCreation = std::chrono::high_resolution_clock::now();
	std::vector<CJT::CityObject> siteObjects = geoCreator->makeSite(internalDataManager_.get(), kernel, 1);
	for (size_t i = 0; i < siteObjects.size(); i++)
	{
		CJT::CityObject currentSiteObject = siteObjects[i];
		currentSiteObject.addParent(cityBuildingObject);
		collection->addCityObject(currentSiteObject);
	}
	return;
}


bool IOManager::init(const std::vector<std::string>& inputPathList)
{
#ifdef NDEBUG
	std::string buildMode = "Release";
#else
	std::string buildMode = "Debug";
#endif

	std::cout << CommunicationStringImportanceEnum::getString(CommunicationStringImportanceID::seperator) << std::endl;
	std::cout << "	IFC_BuildingEnvExtractor v" << iterationVersion << "-" << buildVersion << " " << buildMode << std::endl;
	std::cout << "    Experimental building shell extractor/approximation\n" << std::endl;
	std::cout << CommunicationStringImportanceEnum::getString(CommunicationStringImportanceID::seperator) << std::endl;
	std::cout << std::endl;

	std::string inputPath = "";
	if (inputPathList.size() == 0) { inputPath = getTargetPath(); }
	else if (inputPathList.size() > 1) { return false; } // too many args
	else { inputPath = inputPathList[0]; }

	try { getJSONValues(inputPath); }
	catch (const std::string& exceptionString)
	{
		throw exceptionString;
	}
	std::cout << std::endl;
	printSummary();
	
	internalDataManager_ = std::make_unique<DataManager>(SettingsCollection::getInstance().getIfcPathList());
	if (!internalDataManager_->isPopulated()) { return false; }
	if (!internalDataManager_->hasSetUnits()) { return false; }

	return true;
}

bool IOManager::run()
{
	SettingsCollection& settingsCollection = SettingsCollection::getInstance();

	// create the cjt objects
	std::shared_ptr<CJT::CityCollection> collection = nullptr;
	CJT::CityObject cityBuildingObject; //the overarching city object
	CJT::CityObject cityOuterShellObject; //container of outer shell geo objects
	CJT::CityObject cityComplexObject; // container of the ifc conversion objects
	CJT::CityObject cityInnerShellObject; //container of storey objects

	if (settingsCollection.createJSON())
	{
		setDefaultSemantic(cityBuildingObject, cityComplexObject, cityOuterShellObject, cityInnerShellObject);
	}

	// internalize the helper data
	try { internalizeGeo(); }
	catch (const std::string& exceptionString) 
	{ 
		std::cout << exceptionString << std::endl;
		return false;
	}

	if (settingsCollection.createJSON())
	{
		double scale = 0.001;
		collection = std::make_shared<CJT::CityCollection>(scale);	
		setMetaData(collection);
	}
	if (settingsCollection.createCADFile()) { createCompanionFile(); }

	// make the geometrycreator and voxelgrid
	CJGeoCreator geoCreator;

	auto voxelTime = std::chrono::high_resolution_clock::now();
	try
	{
		geoCreator.init(internalDataManager_.get(), settingsCollection.voxelSize());
	}
	catch (const ErrorID& exceptionId)
	{
		std::cout << errorWarningStringEnum::getString(exceptionId) << std::endl;
		ErrorCollection::getInstance().addError(exceptionId);
		return false;
	}

	timeVoxel_ = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - voxelTime).count();

	try
	{
		geoCreator.initializeBasic(internalDataManager_.get());
	}
	catch (const ErrorID& e)
	{
		ErrorCollection::getInstance().addError(ErrorID::errorFailedInit);
		return false;
	}

	// create the actual geometry
	std::unique_ptr<CJT::Kernel> kernel = nullptr;
	if (settingsCollection.createJSON())
	{
		kernel = std::make_unique<CJT::Kernel>(collection);
	}

	if (settingsCollection.makeInterior())
	{
		processInteriorLod(&geoCreator, collection, &cityInnerShellObject, kernel.get());
	}
	if (settingsCollection.makeExterior())
	{
		processExternalLoD(&geoCreator, cityOuterShellObject, kernel.get());
	}
	if (settingsCollection.make40() || settingsCollection.make41() || settingsCollection.make42())
	{
		processComplexlLoD(&geoCreator, cityComplexObject, kernel.get());
	}
	if (settingsCollection.makeSite())
	{
		processSitelod(&geoCreator, collection, &cityBuildingObject, kernel.get());
	}

	if (settingsCollection.createJSON())
	{
		setComputedSemantic(&geoCreator, cityBuildingObject, cityOuterShellObject, cityInnerShellObject);

		collection->addCityObject(cityBuildingObject);

		if (settingsCollection.makeExterior()) { collection->addCityObject(cityOuterShellObject); }
		if (settingsCollection.makeComplex()) { collection->addCityObject(cityComplexObject); }
		if (settingsCollection.makeInterior()) { collection->addCityObject(cityInnerShellObject); }

		collection->CleanVertices();
		cityCollection_ = collection;
	}

	printErrors();
	return succesfullExit_;
}

bool IOManager::write(bool reportOnly)
{
	SettingsCollection& settingsCollection = SettingsCollection::getInstance();

	if (settingsCollection.getOutputJSONPath() == "") {
		//TODO: add cout for no report file made
		return true; 
	} // no output path set yet, cannot write to unknown location

	if (!reportOnly)
	{
		cityCollection_->dumpJson(settingsCollection.getOutputJSONPath());
	}
	if (!settingsCollection.writeReport()) { return true; }
	nlohmann::json report;
	report["Input settings"] = settingsToJSON();

	nlohmann::json timeReport;
	addTimeToJSON(&timeReport, "Internalizing", timeInternalizing_);
	addTimeToJSON(&timeReport, "Voxel creation", timeVoxel_);
	addTimeToJSON(&timeReport, "LoD0.0 generation", timeLoD00_);
	addTimeToJSON(&timeReport, "LoD0.2 generation", timeLoD02_);
	addTimeToJSON(&timeReport, "LoD0.3 generation", timeLoD03_);
	addTimeToJSON(&timeReport, "LoD0.4 generation", timeLoD04_);
	addTimeToJSON(&timeReport, "LoD1.0 generation", timeLoD10_);
	addTimeToJSON(&timeReport, "LoD1.2 generation", timeLoD12_);
	addTimeToJSON(&timeReport, "LoD1.3 generation", timeLoD13_);
	addTimeToJSON(&timeReport, "LoD2.2 generation", timeLoD22_);
	addTimeToJSON(&timeReport, "LoDe.1/3.2 generation", timeLoD32_);
	addTimeToJSON(&timeReport, "LoD5.0 (V) generation", timeV_);
	addTimeToJSON(&timeReport, "LoDb.0 generation", timeLoDb0_);
	addTimeToJSON(&timeReport, "LoDc.1 generation", timeLoDc1_);
	addTimeToJSON(&timeReport, "LoDc.2 generation", timeLoDc2_);
	addTimeToJSON(&timeReport, "LoDd.1 generation", timeLoDd1_);
	addTimeToJSON(&timeReport, "LoDd.2 generation", timeLoDd2_);
	addTimeToJSON(&timeReport, "LoD4.0 generation", timeLoD40_);
	addTimeToJSON(&timeReport, "LoD4.1 generation", timeLoD41_);
	addTimeToJSON(&timeReport, "LoD4.2 generation", timeLoD42_);
	addTimeToJSON(&timeReport, "LoDe.1 generation", timeLoDe1_);
	addTimeToJSON(&timeReport, "Total Processing", std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - startTime_).count());

	report["Duration"] = timeReport;
	report["Errors"] = ErrorCollection::getInstance().toJson();

	std::ofstream reportFile(settingsCollection.getOutputReportPath());
	reportFile << report.dump(4);
	reportFile.close();
	return true;
}
