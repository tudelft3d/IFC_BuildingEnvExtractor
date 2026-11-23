#include "helper.h"
#include "settingsCollection.h"
#include "GeometryCache.h"

// Boost includes
#include <boost/algorithm/string.hpp>
#include <boost/geometry.hpp>
#include <boost/geometry/index/rtree.hpp>
#include <boost/log/core.hpp>
#include <boost/log/trivial.hpp>

// IfcOpenShell includes
#include <ifcparse/IfcFile.h>
#include <ifcgeom/kernels/opencascade/OpenCascadeKernel.h>

#include <ifcgeom/Iterator.h>
#include <ifcgeom/ConversionSettings.h>
#include <ifcgeom/IfcGeomElement.h>
#include <ifcgeom/IfcGeomRepresentation.h>

#include <ifcparse/IfcHierarchyHelper.h>

// OpenCascade includes
#include <gp_Vec2d.hxx>
#include <gp_Vec.hxx>
#include <TopoDS.hxx>

#include <shared_mutex> 
#include <mutex> 

#ifndef DATAMANAGER_DATAMANAGER_H
#define DATAMANAGER_DATAMANAGER_H


class fileContainer 
{
private:

	std::unique_ptr<IfcParse::IfcFile> file_ = nullptr;

	// The unit multipliers found
	double length_ = 0;

	double getSiPrefixValue(const IfcSchema::IfcSIUnit& unitItem);
	double getSiScaleValue(const IfcSchema::IfcSIUnit& unitItem);

public:
	fileContainer(const std::string& file);

	fileContainer(const fileContainer&) = delete;
	fileContainer& operator=(const fileContainer&) = delete;

	fileContainer(fileContainer&&) noexcept = default;
	fileContainer& operator=(fileContainer&&) noexcept = default;

	/// returns the pointer to the file object
	IfcParse::IfcFile* getFilePtr() {return file_.get();}
	/// returns the length multiplier
	double getLengthMultiplier() const { return length_; }
	/// returns if the file object is good (functioning)
	bool isGood() { return file_->good(); }

	/// internalizes the units that are stored in the file
	void setUnits();
};

/// <summary>
/// Manages the IFC file collection
/// </summary>
class DataManager
{
private:
	bool isPopulated_ = false;

	gp_Pnt lllPoint_;
	gp_Pnt urrPoint_;

	// translation needed for accuracy if object is very far away from origin 
	gp_Trsf objectTranslation_;
	// translation that is adding the object translation to the ifc georef translation
	gp_Trsf objectIfcTranslation_;

	std::vector<fileContainer> datacollection_;

	static const int treeDepth = 5;

	/// stores the actual geometry instance
	//std::unordered_map< GeometryFingerprint, GeometryInstance, GeometryFingerprintHash > geometryInstanceCache_;
	/// stores data about geo, points to inscance cache
	GeometryCache geometryCache_;
	
	std::shared_mutex indexMutex_;
	bgi::rtree<Value, bgi::rstar<treeDepth>> index_;

	std::mutex spaceIndexMutex_;
	bgi::rtree<Value, bgi::rstar<treeDepth>> spaceIndex_;

	std::map <std::string, std::unordered_map < std::string, int >> productIndxLookup_;
	std::unordered_map<std::string, int> spaceIndxLookup_;

	std::unordered_map<std::string, std::vector<IfcSchema::IfcPropertySet*>> attributeLookup_;

	/// finds the ifc schema that is used in the supplied file
	bool findSchema(const std::string& path, bool quiet = false);
	/// count the elements in the file and set the related bools
	void elementCountSummary();

	/// compute the inital lll point, urr point and the rotation related to the apporximated smallest bbox around ino type of object
	void computeIfcModelRotation(gp_Pnt* lllPoint, gp_Pnt* urrPoint);
	/// compute vector from the lll corner to the originpoint based on the first object it can find (prefers slab objects)
	gp_Vec computeObjectTranslation();
	/// compute vector from the lll corner to the originpoint based on the first object of the input type
	gp_Vec computeObjectTranslation(const std::string& objectType);

	/// adds all instances of the type to the index and reports to user
	void timedAddObjectListToIndex(const std::string& typeName, std::unordered_set<std::string>& uniqueKeySet, bool addToRoomIndx = false);
	
	/// get the kernel which contains the product with the supplied product guid
	IfcGeom::Iterator* getIteratorPtr(const std::string& productGuid);	
	/// gets shapes from memory without checking for correct adjusted boolean
	int getObjectShapeLocation(const IfcSchema::IfcProduct* product, bool isSpace = false);

	/// get the product representation from the object from the kernel
	IfcSchema::IfcRepresentation* getProductRepPtr(const IfcSchema::IfcProduct* product);
	/// get the products that are nested inside of the main product
	IfcSchema::IfcProduct::list::ptr getNestedProductList(const IfcSchema::IfcProduct* product);
	/// get flat pointlist of the input product list
	std::vector<gp_Pnt> getObjectListPoints(const std::string& classTypeName, bool simple = false);
	/// get flat pointlist of the input product 
	std::vector<gp_Pnt> getObjectPoints(IfcSchema::IfcProduct* product, bool simple = false);

	// update the lll and urr point
	void updateBoudingData(const bg::model::box <BoostPoint3D>& box);

	// fetch the ePSet_MapConversion data in an IFC2x3 file
	nlohmann::json get2x3GeoData();

	// check if the site contains all the required data for georeferencing according to IfcGRef (ifc2x3) only
	bool validateProjectionData(const nlohmann::json& sitePropertySetData);

	// get the scale and projection data from the file
	void getScaleAndProjection(CJT::ObjectTransformation* transformation, CJT::metaDataObject* metaData);

	// populate a map that has all the guid related propertysets 
	void populateAttributeLookup();

	// add the geometry mememory data to the shape index and init all the required data for mem
	void AddBRepElementToIndex();
	// add the geometry mememory data to the shape index and init all the required data for mem as subgroup
	void AddBRepElementToIndex(int startIndx, int endIndx, std::mutex& countMutex, int& counter);

public:
	/*
	construct and populate a helper
	creates and stores SI unit mulitpliers for length, area and volume
	creates and stores the file and kernel for quick acess
	*/
	explicit DataManager() {};
	explicit DataManager(const std::vector<std::string>& path);

	/// returns true if helper is well populated
	bool isPopulated() { return isPopulated_; }
	/// returns true when length, area and volume multiplier of the internal geo ifc kernels are not 0
	bool hasSetUnits();
	/// returns a pointer to the sourcefile
	IfcParse::IfcFile* getSourceFile(int i) { return datacollection_[i].getFilePtr(); }
	/// returns a vector of pointers to the sourcefiles
	std::vector<IfcParse::IfcFile*> getSourceFiles();
	/// get the total amount of items in the datacollection
	int getSourceFileCount() const { return datacollection_.size(); }
	/// get the length multiplier of a sourcefile
	double getScaler(int i) const { return datacollection_[i].getLengthMultiplier(); }
	/// get the lll point of the ifc bbox
	const gp_Pnt& getLllPoint() { return lllPoint_; }
	/// get the urr point of the ifc bbox
	const gp_Pnt& getUrrPoint() { return urrPoint_; }
	/// get the translation of the ifc model
	const gp_Trsf& getObjectTranslation() { return objectTranslation_; }
	/// get the translation of the ifc + georeference model
	const gp_Trsf& getObjectNormalizedTranslation() { return objectIfcTranslation_; }
	/// get the pointer to the space dividing objects index
	const bgi::rtree<Value, bgi::rstar<treeDepth>>* getIndexPointer() { return &index_; }
	/// get the pointer to the space objects index
	const bgi::rtree<Value, bgi::rstar<treeDepth>>* getSpaceIndexPointer() { return &spaceIndex_; }
	/// get the spatial/product data related to the space dividing objects index
	const GeometryCache& getLookup() { return geometryCache_; }
	/// get the spatial/product data related to the space dividing objects index
	const GeometryCacheItem& getLookup(int i) { return geometryCache_.getItem(i); }
	/// get the spatial/product data related to the space dividing objects index
	const GeometryCacheItem* getLookupPtr(int i) { return geometryCache_.getItemPtr(i); }
	/// get all indexed object shapes
	std::vector<TopoDS_Shape> getIndexedShapes();
	/// get all interior BVO shapes
	std::vector<TopoDS_Shape> getIndexedBVOShapes();
	/// get all indexed lookup values
	std::vector<Value> getIndexedValues();

	// internalises the geometry while approximating a smallest bbox around the geometry
	void internalizeGeo();
	/// fetch the ground floor elevation from file
	void fetchGroundFloorElevation();
	// makes a spatial index for the geometry
	void indexGeo();

	/// gets the projectiodata if present
	gp_Trsf getProjectionTransformation();
	/// gets the projectiodata if present
	void getProjectionData(CJT::ObjectTransformation* transformation, CJT::metaDataObject* metaData);
	// get the projection name from the file
	std::string getProjection();
	/// gets the generic building information
	template <typename T>
	nlohmann::json getBuildingInformation();
	/// gets the object list name or long name
	template <typename T>
	std::string getIfcObjectName(const std::string& objectTypeName, bool isLong);
	/// gets the object name or long name
	template <typename T>
	std::string getIfcObjectName(const std::string& objectTypeName, IfcParse::IfcFile* filePtr, bool isLong);

	/// search the object shape from memory only
	TopoDS_Shape getObjectShapeFromMem(const IfcSchema::IfcProduct* product, bool isSimple);

	/// get the shape of an ifcproduct
	TopoDS_Shape getObjectShape(const IfcSchema::IfcProduct* product, bool getNested = false, bool isSimple = false, bool fromMemOnly = false); //TODO: remove this


};

#endif // DATAMANAGER_DATAMANAGER_H