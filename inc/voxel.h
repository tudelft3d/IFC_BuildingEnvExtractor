#include "helper.h"
#include "DataManager.h"
#include "stringManager.h"

#include <gp_Pnt.hxx>

#ifndef VOXEL_VOXEL_H
#define VOXEL_VOXEL_H

class voxelFace {
private:
	CJObjectID voxelType_ = CJObjectID::CJTypeNone;

public:
	void setIsDoor() { voxelType_ = CJObjectID::CJTypeDoor; }
	void setIsWall() { voxelType_ = CJObjectID::CJTypeWallSurface; }
	void setIsWindow() { voxelType_ = CJObjectID::CJTypeWindow; }
	void setIsRoof() { voxelType_ = CJObjectID::CJTypeRoofSurface; }
	void setIsExternalCeiling() { voxelType_ = CJObjectID::CJTTypeOuterCeilingSurface; }
	void setIsGround() { voxelType_ = CJObjectID::CJTypeGroundSurface; }

	bool isDoor() const { return (voxelType_ == CJObjectID::CJTypeDoor); }
	bool isWall() const { return (voxelType_ == CJObjectID::CJTypeWallSurface); }
	bool isWindow() const { return (voxelType_ == CJObjectID::CJTypeWindow); }
	bool isRoof() const { return (voxelType_ == CJObjectID::CJTypeRoofSurface); }
	bool isGround() const { return (voxelType_ == CJObjectID::CJTypeGroundSurface); }

	const CJObjectID& getType() const { return voxelType_; }
};


class voxel {
private:
	/// the voxel center point in processing space (not world space)
	BoostPoint3D center_;

	/// the size of the voxel in the xy plane
	double sizeXY_;
	/// the size of the voxel in the z direction 
	double sizeZ_;

	/// flag signifying if the voxel intersects with ifc product geo
	bool isIntersecting_ = false;
	/// flag signifying if the voxel is inside of the building
	bool isInside_ = true;
	/// flag signifying if the voxel is part of the outer ring of intersecting voxels
	bool isShell_ = false;
	/// int grouping the voxel to a building
	int buildingNum_ = -1;
	/// int grouping the voxel to a room
	int roomNum_ = -1;

	/// collection of the ifc product the voxel intersects with
	std::vector<Value> internalProducts_;

	// transitional faces (faces between different types of voxels) 
	// voids hold their own containers, intersecting voxels hold the outer shell
	bool hasFace0_ = false; // -x
	bool hasFace1_ = false; // +x
	bool hasFace2_ = false; // -y
	bool hasFace3_ = false; // +y
	bool hasFace4_ = false; // -z
	bool hasFace5_ = false; // +z

	// map containing the semantic voxel faces, the int signifies the dir
	std::map<int, voxelFace> faceMap_ = {};

	/// integers representing the voxel mesh that comply with the getCornerPoints output
	inline static const std::vector<std::array<int, 3>> voxelTriangles = {
		{ 0, 1, 5 }, // side    
		{ 0, 5, 6 },
		{ 1, 2, 4 },
		{ 1, 4, 5 },
		{ 2, 3, 7 },
		{ 2, 7, 4 },
		{ 3, 0, 6 },
		{ 3, 6, 7 },
		{ 6, 5, 4 }, // top
		{ 6, 4, 7 },
		{ 0, 3, 2 }, // bottom
		{ 0, 2, 1 }
	};

	/// integers representing the voxel faces that comply with the getCornerPoints output
	inline static const std::vector<std::array<int, 4>> voxelFaces = {
		{ 1, 2, 4, 5 },
		{ 3, 0, 6, 7 },
		{ 2, 3, 7, 4 },
		{ 0, 1, 5, 6 },
		{ 6, 5, 4, 7 }, // top
		{ 0, 3, 2, 1 }
	};

	/// integers representing the voxel edges that comply with the getCornerPoints output
	inline static const std::vector<std::array<int, 2>> voxelEdges = {
		{ 0, 1},
		{ 1, 2},
		{ 2, 3},
		{ 3, 0},
		{ 4, 5},
		{ 5, 6},
		{ 6, 7},
		{ 7, 4},
		{ 1, 5},
		{ 2, 4},
		{ 3, 7},
		{ 0, 6}
	};

	/// integers of the voxel planes' mesh that comply with the getPlanePoints output
	inline static const std::vector<std::array<int, 3>> voxel::planeTriangles = {
		{0, 1, 3},
		{1, 2, 3},
		{4, 5, 7},
		{5, 6, 7},
		{8, 9, 11},
		{9, 10, 11}
	};

	/// integers of the voxel planes' edges that comply with the getPlanePoints output
	inline static const std::vector<std::array<int, 2>> planeEdges = {
		{ 0, 1},
		{ 1, 2},
		{ 2, 3},
		{ 3, 0},
		{ 4, 5},
		{ 5, 6},
		{ 6, 7},
		{ 7, 4},
		{ 8, 9},
		{ 9, 10},
		{ 10, 11},
		{ 11, 8}
	};

	/// get the points of the product mesh
	std::vector<gp_Pnt> getPotentialMeshObjectPoints(const std::vector<gp_Pnt>& voxelPoints, IfcProductSpatialData& lookup);
	/// check if any cornerpoints fall inside voxel
	bool linearEqIntersection(const std::vector<gp_Pnt>& productPoints, const std::vector<gp_Pnt>& voxelPoints);

	/// check if the centerpoint of the voxel falls within a product shape
	bool voxelCoreIsInShape(const gp_Pnt& centerPoint, IfcProductSpatialData& lookup);
	/// check if any edge of the product mesh intersects with the voxel
	bool productEdgeIntersectsVoxel(const std::vector<gp_Pnt>& voxelPoints, const std::vector<gp_Pnt>& productPoints, int intersectionLogic);
	/// check if any edge of the voxel mesh intersects with the prodcut
	bool voxelEdgeIntersectsProduct(const std::vector<gp_Pnt>& voxelPoints, const std::vector<gp_Pnt>& productPoints, int intersectionLogic);

public:

	/// greates an axis aligned voxel
	explicit voxel(const BoostPoint3D& center, double sizeXY, double sizeZ);

	/// returns the lll and urr point of a voxel in axis aligned space
	bg::model::box<BoostPoint3D> getVoxelGeo() const;
	/// return the cornerpoints of a voxel based on the angle
	std::vector<gp_Pnt> getCornerPoints() const;
	/// return the points represeinging the three intersection planes of a voxel based on the angle
	std::vector<gp_Pnt> getPlanePoints() const;

	/// check the intersection of a triangluted product and a voxel
	bool checkIntersecting(IfcProductSpatialData& lookup, const std::vector<gp_Pnt>& voxelPoints, const gp_Pnt& centerPoint, int intersectionLogic = 4);

	/// returns integers representing the voxel mesh that comply with the getCornerPoints output
	static const std::vector<std::array<int, 3>>& getVoxelTriangles() { return voxelTriangles; }
	/// returns integers representing the voxel faces that comply with the getCornerPoints output
	static const std::vector<std::array<int, 4>>& getVoxelFaces() { return voxelFaces; }
	/// returns integers representing the voxel edges that comply with the getCornerPoints output
	static const std::vector<std::array<int, 2>>& getVoxelEdges() { return voxelEdges; }

	/// returns integers of the voxel planes' mesh that comply with the getPlanePoints output
	const std::vector<std::array<int, 3>>& getplaneTriangles() { return planeTriangles; }
	/// returns integers of the voxel planes' edges that comply with the getPlanePoints output
	static const std::vector<std::array<int, 2>>& getPlaneEdges() { return planeEdges; }

	/// sets number representing to which building the voxel belongs
	void setBuildingNum(int num) { buildingNum_ = num; }

	/// sets flag signifying if voxel is part of the exterior shell
	void setIsShell() { isShell_ = true; }
	/// get flag signifying if voxel is part of the exterior shell
	bool getIsShell() const { return isShell_; }

	/// set flag notifying if the voxel is part of the outside of a building 
	void setOutside(bool b) { isInside_ = !b; }
	/// returns flag representing if the voxel is part of the interior of a building
	const bool getIsInside() const { return isInside_; }

	/// returns the centerpoint of a voxel at its virtual location
	BoostPoint3D getCenterPoint() const { return center_; }
	/// returns the centerpoint of a voxel at its virtual location in OCCT format
	gp_Pnt getOCCTCenterPoint() const { return gp_Pnt(center_.get<0>(), center_.get<1>(), center_.get<2>()); }

	/// returns the intersecting flag
	bool getIsIntersecting() const { return isIntersecting_; }
	/// sets the intersecting flag
	void isIntersecting() { isIntersecting_ = true; }

	/// returns the building number
	int getBuildingNum() const { return buildingNum_; }

	/// internalize an object, used to internalize the objects the voxel intersects with
	void addInternalProduct(const Value& prodValue) { internalProducts_.emplace_back(prodValue); }
	/// returns the list with internalized products
	std::vector<Value> getInternalProductList() const { return internalProducts_; }

	/// returns boolean if the face in that direction is present, if no number input returns if any face is present
	/// 0 = -X
	/// 1 = +X
	/// 2 = -Y
	/// 3 = +Y
	/// 4 = +Z
	/// 5 = -Z
	bool hasFace(int dirNum = -1) const;

	/// returns the number of transfaces
	int numberOfFaces() const;

	/// sets a transitionalface
	void setTransFace(int dirNum);

	/// sets the roomnum the voxel is related to
	void setRoomNum(int roomNum) { roomNum_ = roomNum; }
	/// get the roomnum the voxel is related to
	int getRoomNum() const { return roomNum_; }

	/// set voxel face to roof
	void addRoofSemantic(int indx);
	/// set voxel face to outer ceiling
	void addOuterCeilingSemantic(int indx);
	/// set voxel face to wall
	void addWallSemantic(int indx);
	/// set voxel face to window
	void addWindowSemantic(int indx);
	/// set voxel face to door
	void addDoorSemantic(int indx);
	/// set voxel face to ground
	void addGroundSemantic(int indx);

	/// return the type of the voxel face
	CJObjectID faceType(int dirNum) const;
};

#endif // VOXEL_VOXEL_H
