#include "DataManager.h"
#include "helper.h"
#include "stringManager.h"
#include "errorCollection.h"
#include "DebugUtils.h"

#include <BOPAlgo_Splitter.hxx>
#include <BRepClass3d_SolidClassifier.hxx>

#include <boost/make_shared.hpp>
#include <boost/optional.hpp>

#include <ifcgeom_schema_agnostic/IfcGeomFilter.h>
#include <ifcgeom_schema_agnostic/IfcGeomIterator.h>

#include <thread>
#include <shared_mutex> 
#include <mutex> 

template nlohmann::json DataManager::getBuildingInformation<IfcSchema::IfcBuilding>();
#if defined(USE_IFC4x2) || defined(USE_IFC4x3) || defined(USE_IFC4x3add1) || defined(USE_IFC4x3add2)
template nlohmann::json DataManager::getBuildingInformation<IfcSchema::IfcBridge>();
#endif
#if defined(USE_IFC4x3) || defined(USE_IFC4x3add1) || defined(USE_IFC4x3add2)
template nlohmann::json DataManager::getBuildingInformation<IfcSchema::IfcRoad>();
template nlohmann::json DataManager::getBuildingInformation<IfcSchema::IfcRailway>();
template nlohmann::json DataManager::getBuildingInformation<IfcSchema::IfcMarineFacility>();
#endif

template std::string DataManager::getIfcObjectName<IfcSchema::IfcBuilding>(const std::string& objectTypeName, bool isLong);
template std::string DataManager::getIfcObjectName<IfcSchema::IfcSite>(const std::string& objectTypeName, bool isLong);
#if defined(USE_IFC4x2) || defined(USE_IFC4x3) || defined(USE_IFC4x3add1) || defined(USE_IFC4x3add2)
template std::string DataManager::getIfcObjectName<IfcSchema::IfcBridge>(const std::string& objectTypeName, bool isLong);
#endif
#if defined(USE_IFC4x3) || defined(USE_IFC4x3add1) || defined(USE_IFC4x3add2)
template std::string DataManager::getIfcObjectName<IfcSchema::IfcRoad>(const std::string& objectTypeName, bool isLong);
template std::string DataManager::getIfcObjectName<IfcSchema::IfcRailway>(const std::string& objectTypeName, bool isLong);
template std::string DataManager::getIfcObjectName<IfcSchema::IfcMarineFacility>(const std::string& objectTypeName, bool isLong);
#endif

template std::string DataManager::getIfcObjectName<IfcSchema::IfcBuilding>(const std::string& objectTypeName, IfcParse::IfcFile* filePtr, bool isLong);
template std::string DataManager::getIfcObjectName<IfcSchema::IfcSite>(const std::string& objectTypeName, IfcParse::IfcFile* filePtr, bool isLong);
#if defined(USE_IFC4x2) || defined(USE_IFC4x3) || defined(USE_IFC4x3add1) || defined(USE_IFC4x3add2)
template std::string DataManager::getIfcObjectName<IfcSchema::IfcBridge>(const std::string& objectTypeName, IfcParse::IfcFile* filePtr, bool isLong);
#endif
#if defined(USE_IFC4x3) || defined(USE_IFC4x3add1) || defined(USE_IFC4x3add2)
template std::string DataManager::getIfcObjectName<IfcSchema::IfcRoad>(const std::string& objectTypeName, IfcParse::IfcFile* filePtr, bool isLong);
template std::string DataManager::getIfcObjectName<IfcSchema::IfcRailway>(const std::string& objectTypeName, IfcParse::IfcFile* filePtr, bool isLong);
#endif

IfcProductSpatialData::IfcProductSpatialData(IfcSchema::IfcProduct* productPtr, const TopoDS_Shape& productShape)
{
	ErrorCollection& errorCollection = ErrorCollection::getInstance();

	productPtr_ = std::make_unique<IfcSchema::IfcProduct>(*productPtr);
	productShape_ = productShape;
	std::string objectType = productPtr->data().type()->name();

	for (TopExp_Explorer expl(productShape_, TopAbs_FACE); expl.More(); expl.Next())
	{
		TopoDS_Face productFace = TopoDS::Face(expl.Current());

		TopLoc_Location loc;
		auto mesh = BRep_Tool::Triangulation(productFace, loc);

		if (mesh.IsNull()) { 
			errorCollection.addError(ErrorID::warningUnableToMesh, productPtr->GlobalId());
			continue; 
		}

		for (int i = 1; i <= mesh.get()->NbTriangles(); i++)
		{
			const Poly_Triangle& theTriangle = mesh->Triangles().Value(i);

			std::array<gp_Pnt, 3> trianglePoints{
				mesh->Nodes().Value(theTriangle(1)).Transformed(loc),
				mesh->Nodes().Value(theTriangle(2)).Transformed(loc),
				mesh->Nodes().Value(theTriangle(3)).Transformed(loc)
			};

			auto box = helperFunctions::createBBox(trianglePoints);
			triangleIndex_.insert(std::make_pair(box, productTrianglePoints_.size()));
			productTrianglePoints_.emplace_back(MeshTriangle(trianglePoints));
		}
	}

	std::string productType = productPtr_->data().type()->name();
	if (productType == "IfcDoor" || productType == "IfcWindow")
	{
		isDetailed_ = true;
	}
}

double fileKernelCollection::getSiPrefixValue(const IfcSchema::IfcSIUnit& unitItem) {
	boost::optional<IfcSchema::IfcSIPrefix::Value> prefixOption = unitItem.Prefix();
	if (!prefixOption) { return 1; }

	switch (*prefixOption) {
	case IfcSchema::IfcSIPrefix::IfcSIPrefix_EXA:   return 1e18;
	case IfcSchema::IfcSIPrefix::IfcSIPrefix_PETA:  return 1e15;
	case IfcSchema::IfcSIPrefix::IfcSIPrefix_TERA:  return 1e12;
	case IfcSchema::IfcSIPrefix::IfcSIPrefix_GIGA:  return 1e9;
	case IfcSchema::IfcSIPrefix::IfcSIPrefix_MEGA:  return 1e6;
	case IfcSchema::IfcSIPrefix::IfcSIPrefix_KILO:  return 1e3;
	case IfcSchema::IfcSIPrefix::IfcSIPrefix_HECTO: return 1e2;
	case IfcSchema::IfcSIPrefix::IfcSIPrefix_DECA:  return 10;
	case IfcSchema::IfcSIPrefix::IfcSIPrefix_DECI:  return 1e-1;
	case IfcSchema::IfcSIPrefix::IfcSIPrefix_CENTI: return 1e-2;
	case IfcSchema::IfcSIPrefix::IfcSIPrefix_MILLI: return 1e-3;
	case IfcSchema::IfcSIPrefix::IfcSIPrefix_MICRO: return 1e-6;
	case IfcSchema::IfcSIPrefix::IfcSIPrefix_NANO:  return 1e-9;
	case IfcSchema::IfcSIPrefix::IfcSIPrefix_PICO:  return 1e-12;
	case IfcSchema::IfcSIPrefix::IfcSIPrefix_FEMTO: return 1e-15;
	case IfcSchema::IfcSIPrefix::IfcSIPrefix_ATTO:	return 1e-18;
	default: return 0;
	}
}

double fileKernelCollection::getSiScaleValue(const IfcSchema::IfcSIUnit& unitItem) {

	double prefixValue = getSiPrefixValue(unitItem);
	if (!prefixValue) { return 0; }

	IfcSchema::IfcSIUnitName::Value unitType = unitItem.Name();
	if (unitType == IfcSchema::IfcSIUnitName::IfcSIUnitName_METRE)
	{
		return prefixValue;
	}
	else if (unitType == IfcSchema::IfcSIUnitName::IfcSIUnitName_SQUARE_METRE)
	{
		return prefixValue * prefixValue;

	}
	else if (unitType == IfcSchema::IfcSIUnitName::IfcSIUnitName_SQUARE_METRE)
	{
		return prefixValue * prefixValue * prefixValue;
	}
	return 0;
}

fileKernelCollection::fileKernelCollection(const std::string& filePath)
{
	file_ = new IfcParse::IfcFile(filePath);
	if (!file_->good()) { return; }
	kernel_ = std::make_unique<IfcGeom::Kernel>(file_);
	IfcGeom::Kernel* kernelObject = kernel_.get();
	kernel_.get()->setValue(kernelObject->GV_PRECISION, SettingsCollection::getInstance().linearTolerance());
	setUnits();
}

void fileKernelCollection::setUnits()
{
	double length = 0;
	double area = 0;
	double volume = 0;

	IfcSchema::IfcUnitAssignment::list::ptr assignedUnitListObjects = file_->instances_by_type<IfcSchema::IfcUnitAssignment>();
	if (assignedUnitListObjects.get()->size() == 0) {
		ErrorCollection::getInstance().addError(ErrorID::errorNoUnits);
		std::cout << errorWarningStringEnum::getString(ErrorID::errorNoUnits) << std::endl;
	}
	else if (assignedUnitListObjects.get()->size() > 1)
	{
		ErrorCollection::getInstance().addError(ErrorID::warningMultipleUnits);
		std::cout << errorWarningStringEnum::getString(ErrorID::warningMultipleUnits) << std::endl;
	}
	else
	{
		IfcSchema::IfcUnitAssignment* assignedUnitListObject = *assignedUnitListObjects->begin();
		IfcSchema::IfcUnit::list::ptr assignedUnitList = assignedUnitListObject->Units();

		for (IfcSchema::IfcUnit::list::it unitIterator = assignedUnitList->begin(); unitIterator != assignedUnitList->end(); ++unitIterator)
		{
			IfcSchema::IfcUnit* currentUnit = *unitIterator;
			if (currentUnit->declaration().name() == "IfcSIUnit") //comput SI units
			{
				IfcSchema::IfcSIUnit* currentSiUnit = currentUnit->as<IfcSchema::IfcSIUnit>();
				if (currentSiUnit->UnitType() == IfcSchema::IfcUnitEnum::IfcUnit_LENGTHUNIT)
				{
					length = getSiScaleValue(*currentSiUnit);
					continue;
				}
			}
			if (length != 0)
			{
				break;
			}
		}
	}

	//internalize the data
	if (!length)
	{
		length = 1;
		ErrorCollection::getInstance().addError(ErrorID::errorNoLengthUnit);
		std::cout << errorWarningStringEnum::getString(ErrorID::errorNoLengthUnit) << std::endl;
	}

	length_ = length;

	// print found data to user
	std::cout << CommunicationStringEnum::getString(CommunicationStringID::infoFoundUnits) << std::endl;
	std::cout << "\tLength multiplier = " << length_ << std::endl << std::endl;
	return;
}


DataManager::DataManager(const std::vector<std::string>& pathList) {
	for (const std::string& path : pathList)
	{
		std::cout << CommunicationStringEnum::getString(CommunicationStringID::infoParsingFile) << path << std::endl;
		if (!findSchema(path)) { 
			continue;
		}

		// make new collection
		std::unique_ptr<fileKernelCollection> dataCollection = std::make_unique<fileKernelCollection>(path);
		
		if (!dataCollection.get()->isGood())
		{
			std::cout << errorWarningStringEnum::getString(ErrorID::warningIfcUnableToParse) << " " << path << std::endl;
			continue;
		}

		std::cout << CommunicationStringEnum::getString(CommunicationStringID::indentValidIFCFound) << std::endl;
		std::cout << std::endl;

		datacollection_.emplace_back(std::move(dataCollection));
		isPopulated_ = true;
	}
	return;
}

bool DataManager::findSchema(const std::string& path, bool quiet) {
	std::ifstream infile(path);
	std::string line;
	int linecount = 0;
	std::unordered_set<std::string> ifcVersionList = SettingsCollection::getInstance().getSupportedIfcVersionList();

	while (linecount < 100 && std::getline(infile, line))
	{
		if (line[0] == '#')
		{
			if (!quiet)
			{
				std::cout << errorWarningStringEnum::getString(ErrorID::warningIfcNoSchema) << path << std::endl;
				ErrorCollection::getInstance().addError(ErrorID::warningIfcNoSchema, path);
			}
			infile.close();
			return false;
		}
		if (line.find("FILE_SCHEMA") == std::string::npos)
		{
			linecount++;
			continue;
		}

		for (std::string ifcVersion : ifcVersionList)
		{
			if (line.find(ifcVersion) == std::string::npos) {
				continue;
			}

			if (buildVersion != ifcVersion)
			{
				if (!quiet)
				{
					std::cout << errorWarningStringEnum::getString(ErrorID::warningIfcIncomp) + ifcVersion << std::endl;
					ErrorCollection::getInstance().addError(ErrorID::warningIfcIncomp, path);
				}
				infile.close();
				return false;
			}
			else {
				if (!quiet)
				{
					std::cout << CommunicationStringEnum::getString(CommunicationStringID::indentcompIFCFound) + ifcVersion << std::endl;
				}
				infile.close();
				return true;
			}			
		}
	}
	infile.close();
	std::cout << errorWarningStringEnum::getString(ErrorID::warningIfcNoSchema) << path << std::endl;
	ErrorCollection::getInstance().addError(ErrorID::warningIfcNoSchema, path);
	return false;
}


void DataManager::elementCountSummary()
{
	// count the proxy amount
	int proxyCount = 0;
	int objectCount = 0;

	for (size_t i = 0; i < getSourceFileCount(); i++)
	{
		IfcParse::IfcFile* fileObject = datacollection_[i]->getFilePtr();
		IfcSchema::IfcProduct::list::ptr products = fileObject->instances_by_type<IfcSchema::IfcProduct>();
		IfcSchema::IfcBuildingElementProxy::list::ptr proxyProducts = fileObject->instances_by_type<IfcSchema::IfcBuildingElementProxy>();

		objectCount += products->size();
		proxyCount += proxyProducts->size();
	}

	std::cout << CommunicationStringImportanceEnum::getString(CommunicationStringImportanceID::indent) << 
		objectCount << " objects found\n";
	std::cout << CommunicationStringImportanceEnum::getString(CommunicationStringImportanceID::indent) << 
		proxyCount << " IfcBuildingElementProxy objects found\n\n";

	SettingsCollection::getInstance().setProxyCount(proxyCount);
	SettingsCollection::getInstance().setObjectCount(objectCount);
	return;
}

gp_Vec DataManager::computeObjectTranslation()
{
	double precision = SettingsCollection::getInstance().linearTolerance();
	gp_Vec translationVec = computeObjectTranslation("IfcSlab");
	if (translationVec.Magnitude() > precision) { return translationVec; }

	ErrorCollection::getInstance().addError(ErrorID::warningIfcNoSlab);
	std::cout << errorWarningStringEnum::getString(ErrorID::warningIfcNoSlab) << std::endl;

	if (SettingsCollection::getInstance().useDefaultDiv())
	{
		for (const std::string& currentType : SettingsCollection::getInstance().getDefaultDivList())
		{
			if (currentType == "IfcSlab") { continue; }
			translationVec = computeObjectTranslation(currentType);
			if (translationVec.Magnitude() > precision) { return translationVec; }
		}
	}
	if (SettingsCollection::getInstance().useProxy())
	{
		translationVec = computeObjectTranslation("IfcBuildingElementProxy");
		if (translationVec.Magnitude() > precision) { return translationVec; }
	}

	for (const std::string& currentType : SettingsCollection::getInstance().getCustomDivList())	
	{
		translationVec = computeObjectTranslation(currentType);
		if (translationVec.Magnitude() > precision) { return translationVec; }
	}

	ErrorCollection::getInstance().addError(ErrorID::warningIFCNoRotationClass);
	std::cout << errorWarningStringEnum::getString(ErrorID::warningIFCNoRotationClass) << std::endl;

	return gp_Vec();
}

gp_Vec DataManager::computeObjectTranslation(const std::string& objectType)
{
	for (size_t i = 0; i < getSourceFileCount(); i++)
	{
		aggregate_of_instance::ptr productList = datacollection_[i]->getFilePtr()->instances_by_type(objectType);
		if (productList == nullptr) { continue; }
		if (!productList->size()) { continue; }

		for (auto et = productList->begin(); et != productList->end(); ++et)
		{
			IfcUtil::IfcBaseClass* test = *et;
			IfcSchema::IfcProduct* product = (*et)->as<IfcSchema::IfcProduct>();

			gp_Pnt lllPoint;
			gp_Pnt urrPoint;
			TopoDS_Shape slabShape = getObjectShape(product, false, true);

			if (slabShape.IsNull()) { continue; }
			helperFunctions::bBoxDiagonal(helperFunctions::getPoints(slabShape), &lllPoint, &urrPoint, 0);

			std::cout << "	Translation based on " << objectType << " class\n";

			return gp_Vec(-lllPoint.X(), -lllPoint.Y(), -lllPoint.Z());
		}
	}
	ErrorCollection::getInstance().addError(ErrorID::warningIFCMissingType, objectType);
	return gp_Vec();
}

void DataManager::timedAddObjectListToIndex(const std::string& typeName, std::unordered_set<std::string>& uniqueKeySet, bool addToRoomIndx)
{
	auto startTime = std::chrono::high_resolution_clock::now();
	std::string preFixString = typeName + " objects ";

	SettingsCollection& settings = SettingsCollection::getInstance();

	bool isSimple = true;
	int simplefyGeoGrade = SettingsCollection::getInstance().ignoreVoidGrade();
	std::unordered_set<std::string> openingObjects = settings.getOpeningObjectsList();
	if (simplefyGeoGrade == 0) { isSimple = false; }
	else if (simplefyGeoGrade == 2) { isSimple = true; }
	else if (openingObjects.find(typeName) == openingObjects.end()) { isSimple = false; }

	std::vector<IfcGeom::filter_t> filterFuncs;
	filterFuncs.emplace_back(IfcGeom::entity_filter(true, true, { typeName }));

	int fileNum = 0;
	for (const std::unique_ptr<fileKernelCollection>& collectionItem : datacollection_)
	{
		fileNum++;

		IfcGeom::Iterator it(
			settings.iteratorSettings(isSimple),
			collectionItem->getFilePtr(),
			filterFuncs,
			settings.threadcount()
		);
		if (!it.initialize()) { continue; }

		int itCount = 0;

		std::vector<IfcGeom::BRepElement*> shapeList;
		shapeList.reserve(collectionItem->getFilePtr()->instances_by_type(typeName)->size()); //TODO: optimize this

		int currentItemNum = 0;
		do { 
			shapeList.emplace_back((it.get_native()));

			if (currentItemNum % 100 == 0)
			{
				std::cout << "\t" << preFixString << "- Parsing file: " << fileNum << "; Object - " << currentItemNum << "\r";
			}
			currentItemNum++;
		} 
		while (it.next());
		std::cout << "\t" << preFixString << "- Parsing file: " << fileNum << "; Object - " << currentItemNum << "\r";


		int coreUse = settings.threadcount() - 1;
		if (shapeList.empty()) { continue; }
		if (shapeList.size() < coreUse) { coreUse = shapeList.size(); }
		int splitListSize = static_cast<int>(std::floor(shapeList.size() / coreUse));

		std::vector<std::thread> threadList;
		int processedItemCount = 0;
		std::mutex counterMutex;

		for (size_t i = 0; i < coreUse; i++)
		{
			auto startIdx = shapeList.begin() + i * splitListSize;
			auto endIdx = (i == coreUse - 1) ? shapeList.end() : startIdx + splitListSize;
			std::vector<IfcGeom::BRepElement*> sublist(startIdx, endIdx);
			threadList.emplace_back([=, &uniqueKeySet, &processedItemCount, &counterMutex]() { AddBRepElementToIndex(sublist, uniqueKeySet, processedItemCount, counterMutex, addToRoomIndx); });
		}

		std::cout << "                                                                                          \r";
		std::string currentIndication = preFixString + "- Indexing file: " + std::to_string(fileNum) + "; Object";

		threadList.emplace_back([&] {helperFunctions::updateCounter(currentIndication, shapeList.size(), processedItemCount, counterMutex, false);  });

		for (auto& thread : threadList) {
			if (thread.joinable()) {
				thread.join();
			}
		}
		std::cout << "                                                                                          \r";
	}
	std::cout << "\t" << preFixString << "finished in: " <<
		std::chrono::duration_cast<std::chrono::seconds>(std::chrono::high_resolution_clock::now() - startTime).count() <<
		UnitStringEnum::getString(UnitStringID::seconds) << "                                                " << std::endl;
}

IfcGeom::Kernel* DataManager::getKernelObject(const std::string& productGuid)
{
	if (getSourceFileCount() == 1)
	{
		return datacollection_[0]->getKernelPtr();
	}
	else {
		for (size_t i = 0; i < getSourceFileCount(); i++)
		{
			try { datacollection_[i]->getFilePtr()->instance_by_guid(productGuid)->data().toString(); }
			catch (const std::exception&) { continue; }

			return datacollection_[i]->getKernelPtr();
		}
	}
	return nullptr;
}

int DataManager::getObjectShapeLocation(IfcSchema::IfcProduct* product)
{
	std::string objectType = product->data().type()->name();
	const std::shared_lock<std::shared_mutex> lock(indexMutex_);
	auto typeSearch = productIndxLookup_.find(objectType);
	if (typeSearch == productIndxLookup_.end()) { return -1; }

	auto objectSearch = typeSearch->second.find(product->GlobalId());
	if (objectSearch == typeSearch->second.end()) { return -1; }
	return objectSearch->second;
}


IfcSchema::IfcRepresentation* DataManager::getProductRepPtr(IfcSchema::IfcProduct* product)
{
	IfcSchema::IfcRepresentation* ifc_representation = nullptr;
	if (product->Representation())
	{
		IfcSchema::IfcProductRepresentation* prodrep = product->Representation();
		IfcSchema::IfcRepresentation::list::ptr reps = prodrep->Representations();

		for (IfcSchema::IfcRepresentation::list::it it = reps->begin(); it != reps->end(); ++it) {
			IfcSchema::IfcRepresentation* rep = *it;
			if (rep->RepresentationIdentifier().get() == "Body") {
				ifc_representation = rep;
				break;
			}
		}
	}
	return ifc_representation;
}

IfcSchema::IfcProduct::list::ptr DataManager::getNestedProductList(IfcSchema::IfcProduct* product)
{
	IfcSchema::IfcProduct::list::ptr outputList = boost::make_shared<IfcSchema::IfcProduct::list>();

#if defined(USE_IFC2x3)
	IfcSchema::IfcRelDecomposes::list::ptr decomposedProducts = product->IsDecomposedBy();
#else
	IfcSchema::IfcRelAggregates::list::ptr decomposedProducts = product->IsDecomposedBy();
#endif // USE_IFC2x3

	if (decomposedProducts->size() > 0)
	{
		for (auto et = decomposedProducts->begin(); et != decomposedProducts->end(); ++et) {
#if defined(USE_IFC2x3)
			IfcSchema::IfcRelDecomposes* aggregates = *et;
#else
			IfcSchema::IfcRelAggregates* aggregates = *et;
#endif // USE_IFC2x3

			IfcSchema::IfcObjectDefinition::list::ptr aggDef = aggregates->RelatedObjects();

			for (auto rt = aggDef->begin(); rt != aggDef->end(); ++rt) {

				IfcSchema::IfcObjectDefinition* aggDef = *rt;
				IfcSchema::IfcProduct* addprod = aggDef->as<IfcSchema::IfcProduct>();
				outputList->push(addprod);
			}
		}
	}
	return outputList;
}

std::vector<gp_Pnt> DataManager::getObjectListPoints(const std::string& classTypeName, bool simple)
{
	std::vector<gp_Pnt> pointList;
	for (const auto& fileObject : datacollection_)
	{
		aggregate_of_instance::ptr objectList = fileObject->getFilePtr()->instances_by_type(classTypeName);
		if (objectList == nullptr) { continue; }

		for (auto it = objectList->begin(); it != objectList->end(); ++it) {
			IfcSchema::IfcProduct* product = (*it)->as<IfcSchema::IfcProduct>();
			std::vector<gp_Pnt> temp = getObjectPoints(product, simple);

			for (const auto& point : temp) {
				pointList.emplace_back(point);
			}
		}
	}
	return std::vector<gp_Pnt>(pointList);
}

std::vector<gp_Pnt> DataManager::getObjectPoints(IfcSchema::IfcProduct* product, bool simple)
{
	TopoDS_Shape productShape = getObjectShape(product, true, simple);
 	std::vector<gp_Pnt> pointList = helperFunctions::getPoints(productShape);
	return pointList;
}

void DataManager::updateShapeMemory(IfcSchema::IfcProduct* product, TopoDS_Shape shape)
{
	if (!product->Representation()) { return; }

	helperFunctions::triangulateShape(shape);
	std::string objectType = product->data().type()->name();

	// filter with lookup
	std::lock_guard<std::shared_mutex> lock(indexMutex_);
	if (productIndxLookup_.find(objectType) == productIndxLookup_.end()) { return; }
	if (productIndxLookup_[objectType].find(product->GlobalId()) == productIndxLookup_[objectType].end()) { return; }

	IfcProductSpatialData& currentLookupvalue = *productLookup_[productIndxLookup_[objectType][product->GlobalId()]];
	currentLookupvalue.setProductShape(shape);
	return;
}


void DataManager::applyVoids()
{
	//TODO: make this work with the opening objects list 
	std::cout << CommunicationStringEnum::getString(CommunicationStringID::infoIgnoreVoids) << std::endl;
	timedVoidShapeAdjust<IfcSchema::IfcWall>("IfcWall");
	timedVoidShapeAdjust<IfcSchema::IfcSlab>("IfcSlab");
	timedVoidShapeAdjust<IfcSchema::IfcRoof>("IfcRoof");
	std::cout << "\n";
}

template <typename T>
void DataManager::timedVoidShapeAdjust(const std::string& typeName)
{
	std::cout << "\t" + typeName + " objects ";
	auto startTime = std::chrono::high_resolution_clock::now();

	for (size_t i = 0; i < getSourceFileCount(); i++) //TODO: multithread
	{
		IfcParse::IfcFile* fileObject = datacollection_[i]->getFilePtr();
		voidShapeAdjust(datacollection_[i]->getFilePtr()->instances_by_type<T>());

		std::cout << "finished in: " <<
			std::chrono::duration_cast<std::chrono::seconds>(std::chrono::high_resolution_clock::now() - startTime).count() <<
			UnitStringEnum::getString(UnitStringID::seconds) << std::endl;
		return;
	}
}

template <typename T>
void DataManager::voidShapeAdjust(T productList)
{
	for (auto it = productList->begin(); it != productList->end(); ++it)
	{
		IfcSchema::IfcProduct* wallProduct = *it;

		// get the voids
		IfcSchema::IfcElement* objectElement = wallProduct->as<IfcSchema::IfcElement>();
		IfcSchema::IfcRelVoidsElement::list::ptr voidElementList = objectElement->HasOpenings();
		std::vector<TopoDS_Shape> validVoidShapes = computeEmptyVoids(voidElementList);

		if (validVoidShapes.size() == 0)
		{
			continue;
		}
		else if (validVoidShapes.size() == voidElementList->size())
		{
			TopoDS_Shape finalShape = getObjectShape(wallProduct, false, false);
			updateShapeMemory(wallProduct, finalShape);
		}

		TopoDS_Shape untrimmedWallShape = getObjectShape(wallProduct, false, true);
		TopoDS_Shape finalShape = applyVoidtoShape(untrimmedWallShape, validVoidShapes);

		if (finalShape.IsNull()) { continue; }
		updateShapeMemory(wallProduct, finalShape);
	}
}

nlohmann::json DataManager::get2x3GeoData()
{
	IfcParse::IfcFile* fileObject = datacollection_[0]->getFilePtr();
	IfcSchema::IfcPropertySet::list::ptr psets = fileObject->instances_by_type<IfcSchema::IfcPropertySet>();

	bool isSite = false;
	bool isProject = false;

	nlohmann::json grefData;

	for (auto it = psets->begin(); it != psets->end(); ++it)
	{
		IfcSchema::IfcPropertySet* pset = *it;

		if (!pset->Name() || pset->Name().get() != "ePSet_MapConversion") { continue; }
#if defined(USE_IFC2x3)
		auto rels = pset->PropertyDefinitionOf();

		for (auto relIt = rels->begin(); relIt != rels->end(); ++relIt)
		{
			IfcSchema::IfcRelDefinesByProperties* rel = *relIt;

			auto objects = rel->RelatedObjects();

			for (auto objIt = objects->begin(); objIt != objects->end(); ++objIt)
			{
				if ((*objIt)->as<IfcSchema::IfcSite>() && !isSite)
				{
					isSite = true;
					grefData = helperFunctions::getAttributes(*pset);
					break;
				}
				else if ((*objIt)->as<IfcSchema::IfcProject>()&& !isProject)
				{
					isProject = true;
					grefData = helperFunctions::getAttributes(*pset);
					break;
				}
			}
		}
#endif
	}

	if (isSite)
	{
		//TODO: add error
	}

	if (isSite && isProject) {
		//TODO: add error
	}
	return grefData;
}

std::vector<TopoDS_Shape> DataManager::computeEmptyVoids(IfcSchema::IfcRelVoidsElement::list::ptr voidElementList)
{
	// find if the voids are filled or not
	std::vector<TopoDS_Shape> emptyVoidShapeList;
	std::unordered_set<std::string> cuttingObjects = SettingsCollection::getInstance().getCuttingObjectsList();

	for (auto et = voidElementList->begin(); et != voidElementList->end(); ++et)
	{
		IfcSchema::IfcRelVoidsElement* voidElement = *et;
		IfcSchema::IfcFeatureElementSubtraction* openingElement = voidElement->RelatedOpeningElement();
		TopoDS_Shape substractionShape = getObjectShape(openingElement, false);

		// find void occupying objects
		gp_Pnt lllPoint;
		gp_Pnt urrPoint;
		helperFunctions::bBoxDiagonal(getObjectPoints(openingElement), &lllPoint, &urrPoint, 0);

		std::vector<Value> qResult;
		bg::model::box<BoostPoint3D> qBox = bg::model::box<BoostPoint3D>(
			helperFunctions::Point3DOTB(lllPoint),
			helperFunctions::Point3DOTB(urrPoint)
			);
		index_.query(bgi::intersects(qBox), std::back_inserter(qResult));

		// if no occupying objects found void can be applied
		if (qResult.size() == 0)
		{
			emptyVoidShapeList.emplace_back(substractionShape);
			continue;
		}

		BRepClass3d_SolidClassifier insideChecker;
		insideChecker.Load(substractionShape);

		bool intersects = false;
		for (size_t i = 0; i < qResult.size(); i++)
		{
			const IfcProductSpatialData& lookup = getLookup(qResult[i].second);
			IfcSchema::IfcProduct* qProduct = lookup.getProductPtr();
			if (cuttingObjects.find(qProduct->data().type()->name()) == cuttingObjects.end()) { continue; }

			TopoDS_Shape qShape = getObjectShape(qProduct, false);
			for (TopExp_Explorer expl(qShape, TopAbs_VERTEX); expl.More(); expl.Next())
			{
				insideChecker.Perform(BRep_Tool::Pnt(TopoDS::Vertex(expl.Current())), 0.01);
				if (insideChecker.State() || insideChecker.IsOnAFace())
				{
					intersects = true;
					break;
				}
			}
			if (intersects)
			{
				break;
			}
		}

		if (!intersects)
		{
			emptyVoidShapeList.emplace_back(substractionShape);
			continue;
		}
	}
	return emptyVoidShapeList;
}

TopoDS_Shape DataManager::applyVoidtoShape(const TopoDS_Shape& untrimmedShape, std::vector<TopoDS_Shape>& voidObjectList)
{
	double precision = SettingsCollection::getInstance().linearTolerance();

	// bool out voidShape
	BOPAlgo_Splitter aSplitter;
	TopTools_ListOfShape aLSObjects;
	aLSObjects.Append(untrimmedShape);
	TopTools_ListOfShape aLSTools;

	for (size_t i = 0; i < voidObjectList.size(); i++)
	{
		aLSTools.Append(voidObjectList[i]);
	}

	aLSTools.Reverse();
	aSplitter.SetArguments(aLSObjects);
	aSplitter.SetTools(aLSTools);
	aSplitter.SetRunParallel(Standard_True);
	aSplitter.SetNonDestructive(Standard_True);
	aSplitter.Perform();

	const TopoDS_Shape& aResult = aSplitter.Shape(); // result of the operation

	TopoDS_Shape finalShape;
	// get a basepoint of the wall
	gp_Pnt anchorPoint = helperFunctions::getFirstPointShape(untrimmedShape);
	for (TopExp_Explorer solidExpl(aResult, TopAbs_SOLID); solidExpl.More(); solidExpl.Next())
	{
		TopoDS_Solid currentSolid = TopoDS::Solid(solidExpl.Current());
		for (TopExp_Explorer expl(currentSolid, TopAbs_VERTEX); expl.More(); expl.Next()) {
			gp_Pnt evalPoint = BRep_Tool::Pnt(TopoDS::Vertex(expl.Current()));
			if (anchorPoint.IsEqual(evalPoint, precision))
			{
				return currentSolid;
			}
		}
	}
	return TopoDS_Shape();
}


void DataManager::updateBoudingData(const bg::model::box<BoostPoint3D>& box)
{
	if (lllPoint_.X() > box.min_corner().get<0>()) { lllPoint_.SetX(box.min_corner().get<0>()); }
	if (lllPoint_.Y() > box.min_corner().get<1>()) { lllPoint_.SetY(box.min_corner().get<1>()); }
	if (lllPoint_.Z() > box.min_corner().get<2>()) { lllPoint_.SetZ(box.min_corner().get<2>()); }
	if (urrPoint_.X() < box.max_corner().get<0>()) { urrPoint_.SetX(box.max_corner().get<0>()); }
	if (urrPoint_.Y() < box.max_corner().get<1>()) { urrPoint_.SetY(box.max_corner().get<1>()); }
	if (urrPoint_.Z() < box.max_corner().get<2>()) { urrPoint_.SetZ(box.max_corner().get<2>()); }
	return;
}


bool DataManager::validateProjectionData(const nlohmann::json& sitePropertySetData)
{
	std::vector<std::string> missingObjects;

	if (!sitePropertySetData.contains("TargetCRS"))
	{
		missingObjects.emplace_back("TargetCRS");
	}
	else if (!sitePropertySetData["TargetCRS"].is_string())
	{
		missingObjects.emplace_back("TargetCRS");
	}

	if (!sitePropertySetData.count("Scale"))
	{
		missingObjects.emplace_back("Scale");
	}
	else if (!sitePropertySetData["Scale"].is_number())
	{
		missingObjects.emplace_back("Scale");
	}

	if (!sitePropertySetData.contains("Eastings"))
	{
		missingObjects.emplace_back("Eastings");
	}
	else if (!sitePropertySetData["Eastings"].contains("value"))
	{
		missingObjects.emplace_back("Eastings");
	}
	else if (!sitePropertySetData["Eastings"]["value"].is_number())
	{
		missingObjects.emplace_back("Eastings");
	}

	if (!sitePropertySetData.contains("Northings"))
	{
		missingObjects.emplace_back("Northings");
	}
	else if (!sitePropertySetData["Northings"].contains("value"))
	{
		missingObjects.emplace_back("Northings");
	}
	else if (!sitePropertySetData["Northings"]["value"].is_number())
	{
		missingObjects.emplace_back("Northings");
	}

	if (!sitePropertySetData.contains("OrthogonalHeight"))
	{
		missingObjects.emplace_back("OrthogonalHeight");
	}
	else if (!sitePropertySetData["OrthogonalHeight"].contains("value"))
	{
		missingObjects.emplace_back("OrthogonalHeight");
	}
	else if (!sitePropertySetData["OrthogonalHeight"]["value"].is_number())
	{
		missingObjects.emplace_back("OrthogonalHeight");
	}

	if (!sitePropertySetData.count("XAxisAbscissa"))
	{
		missingObjects.emplace_back("XAxisAbscissa");
	}
	else if (!sitePropertySetData["XAxisAbscissa"].is_number())
	{
		missingObjects.emplace_back("XAxisAbscissa");
	}


	if (!sitePropertySetData.count("XAxisOrdinate"))
	{
		missingObjects.emplace_back("XAxisOrdinate");
	}
	else if (!sitePropertySetData["XAxisOrdinate"].is_number())
	{
		missingObjects.emplace_back("XAxisOrdinate");
	}

	if (missingObjects.empty())
	{
		return true;
	}
	ErrorCollection::getInstance().addError(ErrorID::warningIfcMissingGeoreference, missingObjects);
	return false;
}

void DataManager::getScaleAndProjection(CJT::ObjectTransformation* transformation, CJT::metaDataObject* metaData)
{
#if defined(USE_IFC2x3)

	nlohmann::json grefSetData = get2x3GeoData();
	if (grefSetData.empty()) { return; }

	if (grefSetData.contains("TargetCRS"))
	{
		if (grefSetData["TargetCRS"].is_string())
		{
			metaData->setReferenceSystem(grefSetData["TargetCRS"]);
		}
	}
	if (grefSetData.contains("Scale"))
	{
		if (grefSetData["Scale"].is_number())
		{
			transformation->setScale(transformation->getScale()[0] * grefSetData["Scale"].get<float>());
		}
	}
#else
	IfcParse::IfcFile* fileObject = datacollection_[0]->getFilePtr();
	IfcSchema::IfcMapConversion::list::ptr mapList = fileObject->instances_by_type<IfcSchema::IfcMapConversion>();
	if (mapList->size() != 0) {
		if (mapList->size() > 1) {
			ErrorCollection::getInstance().addError(ErrorID::warningIfcMultipleProjections);
			std::cout << errorWarningStringEnum::getString(ErrorID::warningIfcMultipleProjections) << std::endl;
		}

		IfcSchema::IfcMapConversion* mapConversion = *(mapList->begin());

#if defined(USE_IFC4x3add1) || defined(USE_IFC4x3add2)
		boost::optional<std::string> targetCRSOptionalName = mapConversion->TargetCRS()->Name();
		if (targetCRSOptionalName->empty()) { return; }
		metaData->setReferenceSystem(*targetCRSOptionalName);
#else
		metaData->setReferenceSystem(mapConversion->TargetCRS()->Name());
#endif // defined(USE_IFC4x3add2)

		if (mapConversion->Scale().has_value())
		{
			std::array<double, 3> scaleCity = transformation->getScale();
			double scaleIfc = mapConversion->Scale().get();

			for (size_t i = 0; i < scaleCity.size(); i++)
			{
				scaleCity[i] = scaleCity[i] * scaleIfc;
			}
			transformation->setScale(scaleCity);
		}
	}
#endif // !USE_IFC4
	return;
}

void DataManager::populateAttributeLookup()
{
	for (size_t i = 0; i < getSourceFileCount(); i++)
	{
		IfcSchema::IfcRelDefinesByProperties::list::ptr relDefList = datacollection_[i]->getFilePtr()->instances_by_type <IfcSchema::IfcRelDefinesByProperties>();
		for (auto reldefIt = relDefList->begin(); reldefIt != relDefList->end(); reldefIt++)
		{
			IfcSchema::IfcRelDefinesByProperties* relDefItem = *reldefIt;

#if defined(USE_IFC2x3)
			IfcSchema::IfcObject::list::ptr relatedObjectList = relDefItem->RelatedObjects();
#else
			IfcSchema::IfcObjectDefinition::list::ptr relatedObjectList = relDefItem->RelatedObjects();		
#endif

			std::vector<std::string> GuidList;
			GuidList.reserve(relatedObjectList->size());
			for (auto objectIt = relatedObjectList->begin(); objectIt != relatedObjectList->end(); objectIt++)
			{
				GuidList.emplace_back((*objectIt)->GlobalId());
			}

#if defined(USE_IFC2x3)
			IfcSchema::IfcPropertySetDefinition* propertyDef = relDefItem->RelatingPropertyDefinition();
#else
			IfcSchema::IfcPropertySetDefinitionSelect* propertyDef = relDefItem->RelatingPropertyDefinition();
#endif

			if (propertyDef == nullptr) { continue; }
			if (propertyDef->data().type()->name() != "IfcPropertySet") { continue; }
			IfcSchema::IfcPropertySet* propertySet = relDefItem->RelatingPropertyDefinition()->as<IfcSchema::IfcPropertySet>();

			for (const std::string currentGuid : GuidList)
			{
				if (attributeLookup_.find(currentGuid) != attributeLookup_.end())
				{
					attributeLookup_[currentGuid].emplace_back(propertySet);
					continue;
				}

				std::vector< IfcSchema::IfcPropertySet*> propertySetList = { propertySet };
				attributeLookup_.emplace(std::make_pair(currentGuid, propertySetList));
			}
		}
	}
	return;
}

void DataManager::AddBRepElementToIndex(const std::vector<IfcGeom::BRepElement*>& shapeList, std::unordered_set<std::string>& uniqueKeySet, int& counter, std::mutex& counterMutex, bool isRoom)
{
	SettingsCollection& settings = SettingsCollection::getInstance();
	bool ignoreIsExternal = settings.ignoreIsExternal();
	bool simplefyGeo = settings.simplefyGeo();
	bool makeLoD41 = settings.make41();
	double gridRotation = settings.gridRotation();
	const std::vector<std::string>& ignoreList = settings.getIgnoreSimplificationList();

	for (IfcGeom::BRepElement* boundaryRepElem : shapeList)
	{
		counterMutex.lock();
		counter++;
		counterMutex.unlock();

		if (!boundaryRepElem)
		{ 
			continue;
		}
		
		TopoDS_Shape shape = boundaryRepElem->geometry().as_compound();

		gp_Trsf ifcPlacement = boundaryRepElem->transformation().data();
		shape = shape.Moved(ifcPlacement);
		shape.Move(objectTranslation_);
		
		gp_Trsf trs;
		trs.SetRotation(gp_Ax1(gp_Pnt(0, 0, 0), gp_Dir(0, 0, 1)), gridRotation);
		shape.Move(trs);

		if (shape.ShapeType() != TopAbs_SOLID)
		{
			solidSemanticMutex_.lock();
			shape = helperFunctions::addSolidSemantic(shape);
			solidSemanticMutex_.unlock();
		}

		auto product = boundaryRepElem->product()->as<IfcSchema::IfcProduct>();

		if (product == nullptr) { continue; }

		bool storeLookupOnly = false;
		if (!ignoreIsExternal)
		{
			if (!helperFunctions::isExternal(product)) 
			{ 
				if (!makeLoD41) { continue; }
				storeLookupOnly = true;
			}
		}

		std::string productType = product->data().type()->name();
		std::string productGuid = product->GlobalId();

		uniqueKeyMutex_.lock();
		if (uniqueKeySet.find(productGuid) != uniqueKeySet.end()) 
		{ 
			uniqueKeyMutex_.unlock();
			continue;
		}
		uniqueKeySet.emplace(productGuid);
		uniqueKeyMutex_.unlock();

		if (simplefyGeo)
		{
			if (productType == "IfcDoor" || productType == "IfcWindow")
			{
				if (std::find(ignoreList.begin(), ignoreList.end(), product->GlobalId()) == ignoreList.end())
				{
					shape = helperFunctions::boxSimplefyShape(shape);
					if (shape.IsNull())
					{
						ErrorCollection::getInstance().addError(ErrorID::warningFailedObjectSimplefication, product->GlobalId());
						continue;
					}
				}
			}
		}
		bg::model::box <BoostPoint3D> box;
		try
		{
			box = helperFunctions::createBBox(shape, 0);
		}
		catch (const ErrorID&)
		{
			ErrorCollection::getInstance().addError(ErrorID::warningFailedObjectSimplefication, product->GlobalId());
			continue;
		}
		if (!helperFunctions::hasVolume(box))
		{
			ErrorCollection::getInstance().addError(ErrorID::warningFailedObjectSimplefication, product->GlobalId());
			continue;
		}

		for (TopExp_Explorer expl(shape, TopAbs_FACE); expl.More(); expl.Next())
		{
			const TopoDS_Face& currentFace = TopoDS::Face(expl.Current());
			helperFunctions::triangulateShape(currentFace);
		}
		std::unique_ptr<IfcProductSpatialData> lookup = std::make_unique<IfcProductSpatialData>(product, shape);

		if (isRoom)
		{
			std::lock_guard<std::mutex> spaceLock(spaceIndexMutex_);
			spaceIndex_.insert(std::make_pair(box, (int)spaceIndex_.size()));
			SpaceLookup_.emplace_back(std::move(lookup));
			continue;
		}

		indexMutex_.lock();
		int locationIdx = (int)productLookup_.size();
		if (!storeLookupOnly)
		{
			index_.insert(std::make_pair(box, locationIdx));
			updateBoudingData(box);
		}

		productLookup_.emplace_back(std::move(lookup));

		auto typeSearch = productIndxLookup_.find(productType);
		if (typeSearch == productIndxLookup_.end())
		{
			productIndxLookup_.insert({ productType, std::unordered_map < std::string, int >() });
		}
		productIndxLookup_[productType].insert({ product->GlobalId(), locationIdx });
		indexMutex_.unlock();
	}
	return;
}


bool DataManager::hasSetUnits() {
	for (size_t i = 0; i < getSourceFileCount(); i++)
	{
		if (!datacollection_[i]->getLengthMultiplier()) { return false; }
	}
	return true; 
}

std::vector<IfcParse::IfcFile*> DataManager::getSourceFiles() const
{
	std::vector<IfcParse::IfcFile*> ptrList;
	int sourceFileCount = getSourceFileCount();
	ptrList.reserve(sourceFileCount);
	for (int i = 0; 1 < sourceFileCount; i ++) { ptrList.emplace_back(datacollection_[i].get()->getFilePtr()); }
	return ptrList;
}


std::vector<TopoDS_Shape> DataManager::getIndexedShapes()
{
	std::vector<TopoDS_Shape> shapeList;
	auto spatialIndx = getIndexPointer();
	for (auto it = spatialIndx->begin(); it != spatialIndx->end(); ++it)
	{
		Value test = *it;
		const IfcProductSpatialData& lookup = getLookup(test.second);
		TopoDS_Shape currentShape = lookup.getProductShape();
		if (currentShape.IsNull()) { continue; }
		shapeList.emplace_back(currentShape);
	}
	return shapeList;
}

std::vector<Value> DataManager::getIndexedValues()
{
	std::vector<Value> valueList;
	auto spatialIndx = getIndexPointer();
	for (auto it = spatialIndx->begin(); it != spatialIndx->end(); ++it)
	{
		Value test = *it;
		valueList.emplace_back(test);
	}
	return valueList;
}

void DataManager::internalizeGeo()
{
	std::cout << CommunicationStringEnum::getString(CommunicationStringID::infoInternalizingGeo) << std::endl;
	auto startTime = std::chrono::high_resolution_clock::now();
	//combine the georef transformation from the ifc file with the local origin offset
	gp_Trsf geoTrsf = getProjectionTransformation();

	objectTranslation_.SetRotation(geoTrsf.GetRotation()); //set the objectranslation to the rotation only
	gp_Vec ifcTrsf(0,0,0);
	if (SettingsCollection::getInstance().correctPlacement())
	{
		ifcTrsf = computeObjectTranslation();
		std::cout << "\n";
		objectTranslation_.SetTranslationPart(ifcTrsf);
	}
	objectIfcTranslation_.SetTranslationPart(-ifcTrsf + geoTrsf.TranslationPart());
	elementCountSummary();

	if (SettingsCollection::getInstance().objectCount() == 0)
	{
		ErrorCollection::getInstance().addError(ErrorID::errorNoObjects);
		throw std::string(errorWarningStringEnum::getString(ErrorID::errorNoObjects));
	}

	std::cout << 
		CommunicationStringEnum::getString(CommunicationStringID::indentSuccesFinished) <<
		std::chrono::duration_cast<std::chrono::seconds>(std::chrono::high_resolution_clock::now() - startTime).count() << 
		UnitStringEnum::getString(UnitStringID::seconds) << "\n" << std::endl;
	return;
}

void DataManager::fetchGroundFloorElevation()
{
	if (!SettingsCollection::getInstance().detectFootprintElevation()) { return; }

	std::cout << CommunicationStringEnum::getString(CommunicationStringID::infoFetchGroundfloorElev) << std::endl;
	auto startTime = std::chrono::high_resolution_clock::now();

	std::vector<std::string> groundFloorCodes = { "0 ", "00 ", "0-" , "00-" };

	std::vector<double> groundfloorElevationList;
	for (const std::unique_ptr<fileKernelCollection>& dataItem : datacollection_)
	{
		IfcSchema::IfcBuildingStorey::list::ptr storeyList = dataItem->getFilePtr()->instances_by_type<IfcSchema::IfcBuildingStorey>();

		for (auto it = storeyList->begin(); it != storeyList->end(); ++it)
		{
			IfcSchema::IfcBuildingStorey* storeyObject = *it;
			if (!storeyObject->Elevation().has_value()) { continue; }

			auto optionalName = storeyObject->Name();
			if (optionalName.is_initialized())
			{
				bool found = false;
				for (const std::string& floorCode : groundFloorCodes)
				{
					if (optionalName->size() >= floorCode.size() &&
						optionalName->substr(0, floorCode.size()) == floorCode)
					{
						groundfloorElevationList.emplace_back(storeyObject->Elevation().get());
						found = true;
						break;
					}
				}	
				if (found) { continue; }
			}

			auto optionalLongName = storeyObject->LongName();
			if (optionalLongName.is_initialized())
			{
				bool found = false;
				for (const std::string& floorCode : groundFloorCodes)
				{
					if (optionalName->size() >= floorCode.size() &&
						optionalName->substr(0, floorCode.size()) == floorCode)
					{
						groundfloorElevationList.emplace_back(storeyObject->Elevation().get());
						found = true;
						break;
					}
				}
				if (found) { continue; }
			}
		}

	}

	if (groundfloorElevationList.empty())
	{
		ErrorCollection::getInstance().addError(ErrorID::errorNoGroundFLoorFound);
		throw std::string(errorWarningStringEnum::getString(ErrorID::errorNoGroundFLoorFound));
	}

	if (groundfloorElevationList.size() != getSourceFileCount())
	{
		ErrorCollection::getInstance().addError(ErrorID::errorInconsistentGroundFLoorNumbers);
		throw std::string(errorWarningStringEnum::getString(ErrorID::errorInconsistentGroundFLoorNumbers));
	}

	double firstElevation = groundfloorElevationList[0];
	double precision = SettingsCollection::getInstance().linearTolerance();
	for (double elevation : groundfloorElevationList)
	{
		if (firstElevation - precision < elevation && firstElevation + precision > elevation) { continue; }
		ErrorCollection::getInstance().addError(ErrorID::errorInconsistentGroundFloorElevations);
		throw std::string(errorWarningStringEnum::getString(ErrorID::errorInconsistentGroundFloorElevations));
	}
	
	SettingsCollection::getInstance().setFootprintElevation(firstElevation);
	std::cout << CommunicationStringEnum::getString(CommunicationStringID::infoGroundfloorElev) << firstElevation << " m\n";

	std::cout <<
		CommunicationStringEnum::getString(CommunicationStringID::indentSuccesFinished) <<
		std::chrono::duration_cast<std::chrono::seconds>(std::chrono::high_resolution_clock::now() - startTime).count() <<
		UnitStringEnum::getString(UnitStringID::seconds) << "\n" << std::endl;
	
	return;
}

void DataManager::indexGeo()
{
	// this indexing is done based on the rotated bboxes of the objects
	// the bbox does thus comply with the model bbox but not with the actual objects original location
	SettingsCollection& settingsCollection = SettingsCollection::getInstance();

	if (index_.size() > 0) { return; }
	std::cout << CommunicationStringEnum::getString(CommunicationStringID::infoCreateSpatialIndex) << std::endl;

	if (!settingsCollection.requireIndex())
	{
		std::cout << "[INFO] No index required\n\n";
		return;
	}

	std::unordered_set<std::string> uniqueKeySet;
	if (settingsCollection.useDefaultDiv())
	{
		bool addToRoomIndex = false;
		timedAddObjectListToIndex("IfcSlab", uniqueKeySet);
		timedAddObjectListToIndex("IfcRoof", uniqueKeySet);
		timedAddObjectListToIndex("IfcWall", uniqueKeySet);
		timedAddObjectListToIndex("IfcCovering", uniqueKeySet);
		timedAddObjectListToIndex("IfcBeam", uniqueKeySet);
		timedAddObjectListToIndex("IfcColumn", uniqueKeySet);
		timedAddObjectListToIndex("IfcPlate", uniqueKeySet);
		timedAddObjectListToIndex("IfcMember", uniqueKeySet);
		timedAddObjectListToIndex("IfcWindow", uniqueKeySet);
		timedAddObjectListToIndex("IfcDoor", uniqueKeySet);
		//addObjectListToIndex<IfcSchema::IfcCurtainWall>("IfcCurtainWall", addToRoomIndex);

		if (settingsCollection.useProxy())
		{
			bool addToRoomIndex = false;
			timedAddObjectListToIndex("IfcBuildingElementProxy", uniqueKeySet, addToRoomIndex);
		}
	}
	else // add custom set div objects
	{
		std::vector<std::string> customDivTypeList = settingsCollection.getCustomDivList();
		for (const std::string& customDivType : customDivTypeList)
		{
			timedAddObjectListToIndex(customDivType, uniqueKeySet);
		}
	}

	if (settingsCollection.makeInterior())
	{
		bool addToRoomIndex = true;
		timedAddObjectListToIndex("IfcSpace", uniqueKeySet, addToRoomIndex);
	}
	std::cout << std::endl;

	// find valid voids
	if (settingsCollection.ignoreVoidGrade() == 1)
	{
		applyVoids();
	}

	if (index_.empty())
	{
		if (!SettingsCollection::getInstance().ignoreIsExternal())
		{
			std::cout << "[WARNING] No objects could be found, possibly IsExternal is not well set in file" << std::endl;
			std::cout << "\tConsider enable ignoring the IsExternal attribute in the configuration" << std::endl;
		}

		ErrorCollection::getInstance().addError(ErrorID::errorNoObjects);
		if (!SettingsCollection::getInstance().make42() && !SettingsCollection::getInstance().make41())
		{
			throw std::string(errorWarningStringEnum::getString(ErrorID::errorNoObjects));
		}

		std::cout << std::string(errorWarningStringEnum::getString(ErrorID::errorNoObjects)) << std::endl;
		std::cout << "[INFO] Continue processing LoD4.1 and/or LoD4.2 only\n\n";
		SettingsCollection::getInstance().disableClassSelectiveLoD();
		return;
	}
	return;
}


gp_Trsf DataManager::getProjectionTransformation()
{
	IfcParse::IfcFile* fileObject = datacollection_[0]->getFilePtr();

#if defined(USE_IFC2x3)

	nlohmann::json grefSetData = get2x3GeoData();
	if (!validateProjectionData(grefSetData)) { return gp_Trsf(); }

	double Eastings = grefSetData["Eastings"]["value"];
	double Northings = grefSetData["Northings"]["value"];
	double OrthogonalHeight = grefSetData["OrthogonalHeight"]["value"];
	double XAA = grefSetData["XAxisAbscissa"];
	double XAO = grefSetData["XAxisOrdinate"];

	gp_Trsf trsf;
	trsf.SetValues(
		XAA, -XAO, 0, 0,
		XAO, XAA, 0, 0,
		0, 0, 1, 0
	);
	trsf.SetTranslationPart(gp_Vec(Eastings, Northings, OrthogonalHeight));
#else
	IfcSchema::IfcMapConversion::list::ptr mapList = fileObject->instances_by_type<IfcSchema::IfcMapConversion>();
	if (mapList->size() == 0) { 
		ErrorCollection::getInstance().addError(ErrorID::warningIfcMissingGeoreference);
		return gp_Trsf();
	}
	if (mapList->size() > 1) {
		ErrorCollection::getInstance().addError(ErrorID::warningIfcMultipleProjections);
		std::cout << errorWarningStringEnum::getString(ErrorID::warningIfcMultipleProjections) << std::endl;
	}

	gp_Trsf trsf;
	IfcSchema::IfcMapConversion* mapConversion = *(mapList->begin());

	if (!mapConversion->XAxisAbscissa().has_value() || !mapConversion->XAxisOrdinate().has_value()) { return gp_Trsf(); }
	double XAO = mapConversion->XAxisOrdinate().get();
	double XAA = mapConversion->XAxisAbscissa().get();
	trsf.SetValues(
		XAA, -XAO, 0, 0,
		XAO, XAA, 0, 0,
		0, 0, 1, 0
	);

	trsf.SetTranslationPart(gp_Vec(mapConversion->Eastings(), mapConversion->Northings(), mapConversion->OrthogonalHeight()));
#endif // !USE_IFC2x3

	return trsf;
}


void DataManager::getProjectionData(CJT::ObjectTransformation* transformation, CJT::metaDataObject* metaData)
{
	// get and set the scale and projection
	getScaleAndProjection(transformation, metaData);

	gp_XYZ invertedObjectTrsf = objectIfcTranslation_.TranslationPart();
	if (metaData->getReferenceDate() == "EPSG:7415")
	{
		if (invertedObjectTrsf.X() < 646.361 || invertedObjectTrsf.X() > 284347.25 ||
			invertedObjectTrsf.Y() < 308289.558 || invertedObjectTrsf.Y() > 637111.025)
		{
			ErrorCollection::getInstance().addError(ErrorID::warningIfcIncorrectGeoRefTranslation, metaData->getReferenceDate());
			std::cout << errorWarningStringEnum::getString(ErrorID::warningIfcIncorrectGeoRefTranslation) << std::endl;
		}
	}

	// apply the georef + ifc translation
	
	transformation->setTranslation(
		invertedObjectTrsf.X(),
		invertedObjectTrsf.Y(),
		invertedObjectTrsf.Z()
	);

	return;
}

template <typename T>
nlohmann::json DataManager::getBuildingInformation()
{
	nlohmann::json dictionary;

	for (size_t i = 0; i < getSourceFileCount(); i++)
	{
		IfcParse::IfcFile* fileObject = datacollection_[i]->getFilePtr();

		auto buildingList = fileObject->instances_by_type<T>();

		for (auto it = buildingList->begin(); it != buildingList->end(); ++it) {
			T* building = *it;

			if (building->Description().has_value()) { dictionary[CJObjectEnum::getString(CJObjectID::ifcDescription)] = building->Description().get(); }
			if (building->ObjectType().has_value()) { dictionary[CJObjectEnum::getString(CJObjectID::ifcObjectType)] = building->ObjectType().get(); }
			if (building->Name().has_value()) { dictionary[CJObjectEnum::getString(CJObjectID::ifcName)] = building->Name().get(); }
			if (building->LongName().has_value()) { dictionary[CJObjectEnum::getString(CJObjectID::ifcLongName)] = building->LongName().get(); }

			nlohmann::json psetMapList = helperFunctions::getAttributes(building);

			for (auto jsonObIt = psetMapList.begin(); jsonObIt != psetMapList.end(); ++jsonObIt) {
				dictionary[sourceIdentifierEnum::getString(sourceIdentifierID::ifc) + jsonObIt.key()] = jsonObIt.value();
			}
		}
	}
	return dictionary;
}

template <typename T>
std::string DataManager::getIfcObjectName(const std::string& objectTypeName, bool isLong)
{
	std::vector<std::string> stringList;
	for (size_t i = 0; i < getSourceFileCount(); i++)
	{
		IfcParse::IfcFile* fileObject = datacollection_[i]->getFilePtr();
		std::string nameString = getIfcObjectName<T>(objectTypeName, fileObject, isLong);
		if (nameString == "") { continue; }
		stringList.emplace_back(nameString);
	}

	if (stringList.size() == 0) { return ""; }
	std::string baseString = stringList[0];
	for (size_t i = 1; i < getSourceFileCount(); i++)
	{
		if (baseString != stringList[i])
		{
			ErrorCollection::getInstance().addError(ErrorID::warningIfcObjectDifferentName, objectTypeName);
			break;
		}
	}
	return baseString;
}

template <typename T>
std::string DataManager::getIfcObjectName(const std::string& objectTypeName, IfcParse::IfcFile* filePtr, bool isLong)
{
	typename T::list::ptr objectList = filePtr->instances_by_type<T>();

	if (objectList->size() > 1)
	{
		ErrorCollection::getInstance().addError(ErrorID::warningIfcMultipleUniqueObjects, objectTypeName);
		return "";
	}

	for (T* object : *objectList)
	{
		if (isLong)
		{
			if (object->LongName().has_value())
			{
				return object->LongName().get();
			}
		}
		else
		{
			if (object->Name().has_value())
			{
				return object->Name().get();
			}
		}
	}
	ErrorCollection::getInstance().addError(ErrorID::warningIfcNoObjectName, objectTypeName);
	return "";
}

TopoDS_Shape DataManager::getObjectShapeFromMem(IfcSchema::IfcProduct* product, bool isSimple)
{
	// filter with lookup
	std::string objectType = product->data().type()->name();
	std::unordered_set<std::string> openingObjects = SettingsCollection::getInstance().getOpeningObjectsList();

	if (openingObjects.find(objectType) == openingObjects.end() &&
		objectType != "IfcDoor" && objectType != "IfcWindow"
		) { isSimple = false; }

	int obbjectShapeLocation = getObjectShapeLocation(product);

	if (obbjectShapeLocation == -1) { return {}; }

	std::shared_lock<std::shared_mutex> lookupMutex(indexMutex_);
	const IfcProductSpatialData& currentProductData = *productLookup_[obbjectShapeLocation];
	return currentProductData.getProductShape();
}


TopoDS_Shape DataManager::getObjectShape(IfcSchema::IfcProduct* product, bool getNested, bool isSimple, bool fromMemOnly)
{
	// filter with lookup
	if (product == nullptr) { return {}; }

	std::string objectType = product->data().type()->name();
	const std::unordered_set<std::string>& openingObjects = SettingsCollection::getInstance().getOpeningObjectsList();

	int simplefyGeoGrade = SettingsCollection::getInstance().ignoreVoidGrade();

	if (simplefyGeoGrade == 0) { isSimple = false; }
	else if (simplefyGeoGrade == 2) { isSimple = true; }
	else if (openingObjects.find(objectType) == openingObjects.end()) { isSimple = false; }

	// get the object from memory if available
	const TopoDS_Shape& potentialShape = getObjectShapeFromMem(product, isSimple);
	if (!potentialShape.IsNull()) { return potentialShape; }



	IfcSchema::IfcRepresentation* ifc_representation = getProductRepPtr(product);

	if (ifc_representation == nullptr)
	{
		if (!getNested) { return{}; }
		IfcSchema::IfcProduct::list::ptr nestedProductList = getNestedProductList(product);

		BRep_Builder builder;
		TopoDS_Compound collection;
		builder.MakeCompound(collection);

		for (auto productIt = nestedProductList->begin(); productIt != nestedProductList->end(); ++productIt) {
			IfcSchema::IfcProduct* addprod = *productIt;
			TopoDS_Shape addshapeSimple = getObjectShape(addprod, getNested, isSimple, fromMemOnly);
			if (addshapeSimple.IsNull()) { continue; }
			builder.Add(collection, addshapeSimple);
		}
		if (!collection.NbChildren()) { return {}; }
		return collection;
	}	
	else
	{
		if (fromMemOnly) { return {}; }
	}

	IfcGeom::Kernel* kernelObject = getKernelObject(product->GlobalId());

	if (kernelObject == nullptr) {
		//TODO: add error
		return {}; 
	}

	gp_Trsf trsf;
	kernelObject->convert_placement(product->ObjectPlacement(), trsf);

	IfcGeom::IteratorSettings iteratorSettings = SettingsCollection::getInstance().iteratorSettings(isSimple);


	IfcGeom::BRepElement* brep = nullptr;
	try
	{
		convertMutex_.lock(); //TODO: I want those removed in update
		brep = kernelObject->convert(iteratorSettings, ifc_representation, product);
		convertMutex_.unlock();
	}
	catch (const std::exception&)
	{
		//TODO: add error
	}

	if (brep == nullptr) { 
		//TODO: add error
		return {}; 
	}

	gp_Trsf placement;
	gp_Trsf trs;

	trs.SetRotation(gp_Ax1(gp_Pnt(0, 0, 0), gp_Dir(0, 0, 1)), SettingsCollection::getInstance().gridRotation());
	kernelObject->convert_placement(ifc_representation, placement);

	auto shapeCollection = brep->geometry().shapes();

	int collectionSize = shapeCollection.size();

	BRep_Builder builder;
	TopoDS_Compound collection;
	builder.MakeCompound(collection);
	for (auto it = shapeCollection.begin(); it != shapeCollection.end(); ++it)
	{
		TopoDS_Shape currentShape = (*it).Shape();
		currentShape.Move((*it).Placement().Trsf());
		if (currentShape.ShapeType() == TopAbs_COMPOUND)
		{
			currentShape = helperFunctions::addSolidSemantic(currentShape);
		}

		helperFunctions::triangulateShape(currentShape);

		//if (collectionSize < 2) { return currentShape; }
		builder.Add(collection, currentShape);	
	}
	collection.Move(trsf * placement); // location in global space
	collection.Move(objectTranslation_);
	collection.Move(trs);

	return collection;
}
