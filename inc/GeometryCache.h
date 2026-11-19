#ifndef GeometryCache_GeometryCache_H
#define GeometryCache_GeometryCache_H

#include "helper.h"

#include <array>

// OpenCascade includes
#include <gp_Vec2d.hxx>
#include <gp_Vec.hxx>
#include <TopoDS.hxx>

// container for mesh triangles
class MeshTriangle
{
private:
	/// the three points of the triangle (do not store any other larger vector)
	std::array<gp_Pnt, 3> points_ = {};
public:
	MeshTriangle(const std::array<gp_Pnt, 3>& points) { points_ = points; }
	/// return the three points of the triangle
	const std::array<gp_Pnt, 3>& getPoints() { return points_; }
};

//TODO: new class maybe
struct GeometryFingerprint
{
	long long sumX_ = 0;
	long long sumY_ = 0;
	long long sumZ_ = 0;
	long long sumPt_ = 0;

	GeometryFingerprint(const TopoDS_Shape& geom)
	{
		for (TopExp_Explorer expl(geom, TopAbs_VERTEX); expl.More(); expl.Next()) {
			gp_Pnt evalPoint = BRep_Tool::Pnt(TopoDS::Vertex(expl.Current()));
			sumX_ += static_cast<int>(std::ceil(evalPoint.X() * 1e6));
			sumY_ += static_cast<int>(std::ceil(evalPoint.Y() * 1e6));
			sumZ_ += static_cast<int>(std::ceil(evalPoint.Z() * 1e6));
			sumPt_++;
		}
	}

	bool operator==(const GeometryFingerprint& other) const
	{
		return
			sumX_ == other.sumX_ &&
			sumY_ == other.sumY_ &&
			sumZ_ == other.sumZ_ &&
			sumPt_ == other.sumPt_;
	}
};

struct GeometryFingerprintHash
{
	size_t operator()(const GeometryFingerprint& f) const
	{
		size_t seed = 0;

		auto combine = [&seed](size_t value)
		{
			seed ^= value + 0x9e3779b9 + (seed << 6) + (seed >> 2);
		};

		combine(std::hash<long long>{}(f.sumX_));
		combine(std::hash<long long>{}(f.sumY_));
		combine(std::hash<long long>{}(f.sumZ_));
		combine(std::hash<long long>{}(f.sumPt_));

		return seed;
	}
};

class GeometryInstance
{
private:
	TopoDS_Shape geometryShape_;
	bgi::rtree<std::pair<bg::model::box<BoostPoint3D>, MeshTriangle>, bgi::rstar<25>> triangleIndex_;
	bool active_ = false;
public:
	GeometryInstance(const TopoDS_Shape& instanceShape) { geometryShape_ = instanceShape; }

	void init(const std::string& guid, bool forceSimple);

	const TopoDS_Shape& getGeo() const { return geometryShape_; }

	const bgi::rtree<std::pair<bg::model::box<BoostPoint3D>, MeshTriangle>, bgi::rstar<25>>* getTriangleIndex() const { return &triangleIndex_; }
};

class GeometryCacheItem
{
private:
	const IfcSchema::IfcProduct* productPtr_;
	GeometryInstance* instancedGeometryPtr_;
	bg::model::box<BoostPoint3D> boundingBox_;
	gp_Trsf transformation_;

public:
	GeometryCacheItem(const IfcSchema::IfcProduct* productPtr, GeometryInstance* instancedGeometryPtr, const gp_Trsf& transformation) :
		productPtr_(productPtr), instancedGeometryPtr_(instancedGeometryPtr), transformation_(transformation)
	{}

	void init();

	const IfcSchema::IfcProduct* getProduct() const { return productPtr_; }

	const bg::model::box<BoostPoint3D>& getBoundingBox() const { return boundingBox_; };

	TopoDS_Shape getGeo(bool transformed) const;

	const gp_Trsf& getTransformation() const { return transformation_; }

	const bgi::rtree<std::pair<bg::model::box<BoostPoint3D>, MeshTriangle>, bgi::rstar<25>>* getMeshIndexPtr() const { return instancedGeometryPtr_->getTriangleIndex(); }
}; 

class GeometryCache {
private:
	std::unordered_map< GeometryFingerprint, GeometryInstance, GeometryFingerprintHash > geometryInstanceCache_;
	std::vector< GeometryCacheItem> geometryCacheList_ = {};
	// use size to check if init
	int size_ = 0;
public:

	void addItem(IfcGeom::BRepElement* brepElem, const gp_Trsf& objectTranslation, const double& gridRotation);

	int size() const { return geometryCacheList_.size(); }

	const GeometryCacheItem& getItem(int indx) const;

	const GeometryCacheItem* getItemPtr(int indx);

	const std::vector<GeometryCacheItem>& getItems() const { return geometryCacheList_; }

	void initItem(int indx) { geometryCacheList_[indx].init(); }
};

#endif // DATAMANAGER_DATAMANAGER_H
