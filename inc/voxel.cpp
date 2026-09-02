#include "helper.h"
#include "voxel.h"
#include "IOManager.h"

#include <gp_Pnt.hxx>

#include <ifcparse/IfcFile.h>
#include <ifcparse/IfcHierarchyHelper.h>

#include <BRepClass3d_SolidClassifier.hxx>

#include <thread>   

std::vector<gp_Pnt> voxel::getPotentialMeshObjectPoints(const std::vector<gp_Pnt>& voxelPoints, IfcProductSpatialData& lookup)
{
	std::vector<Value> qResultList;
	lookup.getIndxPointer()->query(bgi::intersects(helperFunctions::createBBox(voxelPoints)), std::back_inserter(qResultList));
	std::vector<gp_Pnt> productPoints;
	for (const Value& qResult : qResultList)
	{
		MeshTriangle triangle = lookup.getProductTriangleList(qResult.second);
		const std::array<gp_Pnt, 3> trianglePoints = triangle.getPoints();
		for (const gp_Pnt& currentPoint : trianglePoints)
		{
			productPoints.emplace_back(currentPoint);
		}
	}
	return productPoints;
}

bool voxel::linearEqIntersection(const std::vector<gp_Pnt>& productPoints, const std::vector<gp_Pnt>& voxelPoints)
{
	gp_Pnt p1 = voxelPoints[0];
	gp_Pnt p2 = voxelPoints[1];
	gp_Pnt p4 = voxelPoints[4];

	for (const gp_Pnt& currentPP : productPoints)
	{
		if (currentPP.X() < p1.X() || currentPP.X() > p4.X()) { continue; }
		if (currentPP.Y() < p1.Y() || currentPP.Y() > p4.Y()) { continue; }
		if (currentPP.Z() < p1.Z() || currentPP.Z() > p4.Z()) { continue; }

		return true;
	}
	return false;
}


bool voxel::voxelCoreIsInShape(const gp_Pnt& centerPoint, IfcProductSpatialData& lookup)
{
	// add check if voxel falls completely in the shape
	gp_Pnt offsetPoint = gp_Pnt(centerPoint.X(), centerPoint.Y(), centerPoint.Z() + 100);
	int counter = 0;

	std::vector<Value> qResultList;
	lookup.getIndxPointer()->query(bgi::intersects(helperFunctions::createBBox(centerPoint, offsetPoint)), std::back_inserter(qResultList));
	std::array<gp_Pnt, 2> castRay = { centerPoint,  offsetPoint };

	for (const Value& qResult : qResultList)
	{
		MeshTriangle triangle = lookup.getProductTriangleList(qResult.second);
		const std::array<gp_Pnt, 3>& trianglePoints = triangle.getPoints();
		if (helperFunctions::triangleIntersecting(castRay, trianglePoints))
		{
			counter++;
		}
	}
	if (counter % 2 == 1)
	{
		isIntersecting_ = true;
		return true;
	}
	return false;
}

bool voxel::productEdgeIntersectsVoxel(const std::vector<gp_Pnt>& voxelPoints, const std::vector<gp_Pnt>& productPoints, int intersectionLogic)
{
	// check if any object edges itersect with the voxel
	std::vector<std::array<int, 3>> triangleVoxels;
	if (intersectionLogic == 2) { triangleVoxels = getplaneTriangles(); }
	else if (intersectionLogic == 3) { triangleVoxels = getVoxelTriangles(); }

	const gp_Pnt& lll = voxelPoints[0];
	const gp_Pnt& urr = voxelPoints[4];

	double voxelLowZ = lll.Z();
	double voxelTopZ = urr.Z();

	double voxelLowX = lll.X();
	double voxelMaxX = urr.X();

	double voxelLowY = lll.Y();
	double voxelMaxY = urr.Y();

	for (const std::array<int, 3>& currentTriangle : triangleVoxels)
	{
		std::array<gp_Pnt, 3> voxelTriangle = { voxelPoints[currentTriangle[0]], voxelPoints[currentTriangle[1]], voxelPoints[currentTriangle[2]] }; //TODO: this can be an array

		for (size_t i = 0; i < productPoints.size(); i += 3)
		{
			// first check if voxel falls within the bounding box of the edge
			const gp_Pnt& p1 = productPoints[i + 0];
			const gp_Pnt& p2 = productPoints[i + 1];
			const gp_Pnt& p3 = productPoints[i + 2];
			double p1z = p1.Z();
			double p2z = p2.Z();
			double p3z = p3.Z();

			double maxZ = std::max({ p1z, p2z, p3z });
			double minZ = std::min({ p1z, p2z, p3z });

			if (minZ > voxelTopZ || maxZ < voxelLowZ) { continue; }

			double p1x = p1.X();
			double p2x = p2.X();
			double p3x = p3.X();

			double maxX = std::max({ p1x, p2x, p3x });
			double minX = std::min({ p1x, p2x, p3x });

			if (minX > voxelMaxX || maxX < voxelLowX) { continue; }

			double p1y = p1.Y();
			double p2y = p2.Y();
			double p3y = p3.Y();

			double maxY = std::max({ p1y, p2y, p3y });
			double minY = std::min({ p1y, p2y, p3y });

			if (minY > voxelMaxY || maxY < voxelLowY) { continue; }

			std::vector<std::array<gp_Pnt, 2>> lineList = {
				{p1, p2},
				{p2, p3},
				{p3, p1}
			};

			for (const std::array<gp_Pnt, 2>& line : lineList)
			{
				if (helperFunctions::triangleIntersecting(line, voxelTriangle))
				{
					isIntersecting_ = true;
					return true;
				}
			}
		}
	}
	return false;
}

bool voxel::voxelEdgeIntersectsProduct(const std::vector<gp_Pnt>& voxelPoints, const std::vector<gp_Pnt>& productPoints, int intersectionLogic)
{
	// check with triangulated object
	std::vector<std::array<int, 2>> vets;
	if (intersectionLogic == 2) { vets = getPlaneEdges(); }
	else if (intersectionLogic == 3) { vets = getVoxelEdges(); }

	for (size_t i = 0; i < productPoints.size(); i += 3)
	{
		const gp_Pnt& p1 = productPoints[i + 0];
		const gp_Pnt& p2 = productPoints[i + 1];
		const gp_Pnt& p3 = productPoints[i + 2];
		std::array<gp_Pnt, 3> triangle = { p1, p2, p3 };

		for (size_t j = 0; j < vets.size(); j++)
		{
			const std::array<int, 2> currentEdge = vets[j];
			if (helperFunctions::triangleIntersecting(
				{ voxelPoints[currentEdge[0]], voxelPoints[currentEdge[1]] },
				triangle) //TODO:fix this
				)
			{
				isIntersecting_ = true;
				return true;
			}
		}
	}
	return false;
}


voxel::voxel(const BoostPoint3D& center, double sizeXY, double sizeZ)
{
	sizeXY_ = sizeXY;
	sizeZ_ = sizeZ;
	center_ = center;

	double offsetXY = sizeXY / 2;
	double offsetZ = sizeZ / 2;

	double xCoord = bg::get<0>(center);
	double yCoord = bg::get<1>(center);
	double zCoord = bg::get<2>(center);

	gp_Pnt minPoint(xCoord - offsetXY, yCoord - offsetXY, zCoord - offsetZ);
	gp_Pnt maxPoint(xCoord + offsetXY, yCoord + offsetXY, zCoord + offsetZ);
}

bg::model::box<BoostPoint3D> voxel::getVoxelGeo() const
{
	double offsetXY = sizeXY_ / 2;
	double offsetZ = sizeZ_ / 2;

	double xCoord = bg::get<0>(center_);
	double yCoord = bg::get<1>(center_);
	double zCoord = bg::get<2>(center_);

	BoostPoint3D lll(xCoord - offsetXY, yCoord - offsetXY, zCoord - offsetZ);
	BoostPoint3D urr(xCoord + offsetXY, yCoord + offsetXY, zCoord + offsetZ);
	return bg::model::box<BoostPoint3D>(lll, urr);
}

std::vector<gp_Pnt> voxel::getCornerPoints() const
{
	auto boxelGeo = getVoxelGeo();

	auto minPoint = helperFunctions::Point3DBTO(boxelGeo.min_corner());
	auto maxPoint = helperFunctions::Point3DBTO(boxelGeo.max_corner());

	// make a pointlist 0 - 3 lower ring, 4 - 7 upper ring
	std::vector<gp_Pnt> pointList;
	pointList.emplace_back(minPoint);
	pointList.emplace_back(maxPoint.X(), minPoint.Y(), minPoint.Z());
	pointList.emplace_back(maxPoint.X(), maxPoint.Y(), minPoint.Z());
	pointList.emplace_back(minPoint.X(), maxPoint.Y(), minPoint.Z());
	pointList.emplace_back(maxPoint);
	pointList.emplace_back(maxPoint.X(), minPoint.Y(), maxPoint.Z());
	pointList.emplace_back(minPoint.X(), minPoint.Y(), maxPoint.Z());
	pointList.emplace_back(minPoint.X(), maxPoint.Y(), maxPoint.Z());

	return pointList;
}

gp_Pnt voxel::getCornerPoint(int indx) const
{
	double offsetXY = sizeXY_ / 2;
	double offsetZ = sizeZ_ / 2;

	double xCenCoord = bg::get<0>(center_);
	double yCenCoord = bg::get<1>(center_);
	double zCenCoord = bg::get<2>(center_);

	switch (indx)
	{
	case 0:
		return gp_Pnt(xCenCoord - offsetXY, yCenCoord - offsetXY, zCenCoord - offsetZ);
	case 1:
		return gp_Pnt(xCenCoord + offsetXY, yCenCoord - offsetXY, zCenCoord - offsetZ);
	case 2:
		return gp_Pnt(xCenCoord + offsetXY, yCenCoord + offsetXY, zCenCoord - offsetZ);
	case 3:
		return gp_Pnt(xCenCoord - offsetXY, yCenCoord + offsetXY, zCenCoord - offsetZ);
	case 4:
		return gp_Pnt(xCenCoord + offsetXY, yCenCoord + offsetXY, zCenCoord + offsetZ);
	case 5:
		return gp_Pnt(xCenCoord + offsetXY, yCenCoord - offsetXY, zCenCoord + offsetZ);
	case 6:
		return gp_Pnt(xCenCoord - offsetXY, yCenCoord - offsetXY, zCenCoord + offsetZ);
	case 7:
		return gp_Pnt(xCenCoord - offsetXY, yCenCoord + offsetXY, zCenCoord + offsetZ);
	default:
		return gp_Pnt();
	}
}

std::vector<gp_Pnt> voxel::getPlanePoints() const
{
	auto boxelGeo = getVoxelGeo();

	auto minPoint = helperFunctions::Point3DBTO(boxelGeo.min_corner());
	auto maxPoint = helperFunctions::Point3DBTO(boxelGeo.max_corner());

	std::vector<gp_Pnt> pointList;
	double offset = sizeXY_ / 2;
	// x plane
	pointList.emplace_back(minPoint.X() + offset, minPoint.Y(), minPoint.Z());
	pointList.emplace_back(minPoint.X() + offset, minPoint.Y(), maxPoint.Z());
	pointList.emplace_back(minPoint.X() + offset, maxPoint.Y(), maxPoint.Z());
	pointList.emplace_back(minPoint.X() + offset, maxPoint.Y(), minPoint.Z());
	// y plane
	pointList.emplace_back(minPoint.X(), minPoint.Y() + offset, minPoint.Z());
	pointList.emplace_back(maxPoint.X(), minPoint.Y() + offset, minPoint.Z());
	pointList.emplace_back(maxPoint.X(), minPoint.Y() + offset, maxPoint.Z());
	pointList.emplace_back(minPoint.X(), minPoint.Y() + offset, maxPoint.Z());
	// z plane
	pointList.emplace_back(minPoint.X(), minPoint.Y(), minPoint.Z() + offset);
	pointList.emplace_back(maxPoint.X(), minPoint.Y(), minPoint.Z() + offset);
	pointList.emplace_back(maxPoint.X(), maxPoint.Y(), minPoint.Z() + offset);
	pointList.emplace_back(minPoint.X(), maxPoint.Y(), minPoint.Z() + offset);

	return pointList;
}


bool voxel::checkIntersecting(IfcProductSpatialData& lookup, const std::vector<gp_Pnt>& voxelPoints, const gp_Pnt& centerPoint, int intersectionLogic)
{
	if (!voxelPoints.size()) { return false; }

	// add check if voxel falls completely in the shape
	if (voxelCoreIsInShape(centerPoint, lookup))
	{
		return true;
	}

	std::vector<gp_Pnt> productPoints = getPotentialMeshObjectPoints(voxelPoints, lookup);
	if (productPoints.size() == 0)
	{
		return false;
	}

	// check if any product triangle point falls within the voxel
	if (intersectionLogic == 3) // not required if the intersection is not volumetric
	{
		if (linearEqIntersection(productPoints, voxelPoints))
		{
			isIntersecting_ = true;
			return true;
		}
	}

	if (productEdgeIntersectsVoxel(voxelPoints, productPoints, intersectionLogic))
	{
		return true;
	}

	if (voxelEdgeIntersectsProduct(voxelPoints, productPoints, intersectionLogic))
	{
		return true;
	}

	return false;
}

bool voxel::hasFace(int dirNum) const
{
	if (dirNum == -1) // check if there is any face
	{
		if (!hasFace0_ && !hasFace1_ && !hasFace2_ && !hasFace3_ && !hasFace4_ && !hasFace5_) { return false; }
		return true;
	}

	if (dirNum == 0 && !hasFace0_) { return false; }
	if (dirNum == 1 && !hasFace1_) { return false; }
	if (dirNum == 2 && !hasFace2_) { return false; }
	if (dirNum == 3 && !hasFace3_) { return false; }
	if (dirNum == 4 && !hasFace4_) { return false; }
	if (dirNum == 5 && !hasFace5_) { return false; }

	return true;
}

int voxel::numberOfFaces() const
{
	int numFaces = 0;

	if (hasFace0_) { numFaces++; }
	if (hasFace1_) { numFaces++; }
	if (hasFace2_) { numFaces++; }
	if (hasFace3_) { numFaces++; }
	if (hasFace4_) { numFaces++; }
	if (hasFace5_) { numFaces++; }

	return numFaces;
}


void voxel::setTransFace(int dirNum)
{
	if (dirNum < 0 || dirNum > 6)
	{
		throw std::invalid_argument("dirNum arguments must be a value of 0 to 6");
	}

	if (dirNum == 0) { hasFace0_ = true; }
	if (dirNum == 1) { hasFace1_ = true; }
	if (dirNum == 2) { hasFace2_ = true; }
	if (dirNum == 3) { hasFace3_ = true; }
	if (dirNum == 4) { hasFace4_ = true; }
	if (dirNum == 5) { hasFace5_ = true; }
}

void voxel::addRoofSemantic(int indx)
{
	if (faceMap_.count(indx))
	{
		voxelFace currentFace = faceMap_[indx];
		currentFace.setIsRoof();
		faceMap_[indx] = currentFace;
		return;
	}
	voxelFace newFace;
	newFace.setIsRoof();
	faceMap_[indx] = newFace;
	return;
}

void voxel::addOuterCeilingSemantic(int indx)
{
	if (faceMap_.count(indx))
	{
		voxelFace currentFace = faceMap_[indx];
		currentFace.setIsExternalCeiling();
		faceMap_[indx] = currentFace;
		return;
	}
	voxelFace newFace;
	newFace.setIsExternalCeiling();
	faceMap_[indx] = newFace;
	return;
}

void voxel::addWallSemantic(int indx)
{
	if (faceMap_.count(indx))
	{
		voxelFace currentFace = faceMap_[indx];
		currentFace.setIsWall();
		faceMap_[indx] = currentFace;
		return;
	}
	voxelFace newFace;
	newFace.setIsWall();
	faceMap_[indx] = newFace;
	return;
}

void voxel::addWindowSemantic(int indx)
{
	if (faceMap_.count(indx))
	{
		voxelFace currentFace = faceMap_[indx];
		currentFace.setIsWindow();
		faceMap_[indx] = currentFace;
		return;
	}
	voxelFace newFace;
	newFace.setIsWindow();
	faceMap_[indx] = newFace;
	return;
}

void voxel::addDoorSemantic(int indx)
{
	if (faceMap_.count(indx))
	{
		voxelFace currentFace = faceMap_[indx];
		currentFace.setIsDoor();
		faceMap_[indx] = currentFace;
		return;
	}
	voxelFace newFace;
	newFace.setIsDoor();
	faceMap_[indx] = newFace;
	return;
}

void voxel::addGroundSemantic(int indx)
{
	if (faceMap_.count(indx))
	{
		voxelFace currentFace = faceMap_[indx];
		currentFace.setIsGround();
		faceMap_[indx] = currentFace;
		return;
	}
	voxelFace newFace;
	newFace.setIsGround();
	faceMap_[indx] = newFace;
	return;
}

CJObjectID voxel::faceType(int dirNum) const
{
	if (faceMap_.size() == 0) { return CJObjectID::CJTypeNone; }

	if (dirNum < 0 || dirNum > 5)
	{
		return CJObjectID::CJTypeNone;
	}
	else {
		if (faceMap_.count(dirNum))
		{
			voxelFace voxelFaceObject = faceMap_.at(dirNum);
			return voxelFaceObject.getType();
		}
		return CJObjectID::CJTypeNone;
	}
}
