#include "helper.h"
#include "settingsCollection.h"
#include "stringManager.h"
#include "errorCollection.h"
#include "DebugUtils.h"

#include <CJToKernel.h>
#include <iostream>
#include <string>
#include <filesystem>

#include <sys/stat.h>

#include <Poly_Triangulation.hxx>

#include <BOPAlgo_Splitter.hxx>
#include <BRepBuilderAPI_MakeFace.hxx>
#include <BRepBuilderAPI_MakeWire.hxx>
#include <BRepBuilderAPI_Sewing.hxx>
#include <BRepBuilderAPI_Transform.hxx>
#include <BRepClass3d_SolidClassifier.hxx>
#include <BRepExtrema_DistShapeShape.hxx>
#include <BRepGProp.hxx>
#include <BRep_Builder.hxx>
#include <gp_Vec2d.hxx>
#include <gp_Vec.hxx>
#include <GProp_GProps.hxx>
#include <IntTools_EdgeFace.hxx>
#include <STEPControl_Writer.hxx>
#include <STEPControl_StepModelType.hxx>
#include <TopoDS.hxx>
#include <BRepTools.hxx>
#include <BRepBuilderAPI_GTransform.hxx>
#include <BRepClass_FaceClassifier.hxx>
#include <BRepOffsetAPI_MakeOffset.hxx>
#include <BRepAlgoAPI_Fuse.hxx>
#include <BRepAlgoAPI_Splitter.hxx>
#include <TColgp_Array1OfPnt.hxx>
#include <BRepMesh_IncrementalMesh.hxx>
#include <Poly_Triangle.hxx>
#include <BRepCheck_Analyzer.hxx>
#include <BRepCheck_Face.hxx>
#include <BRepCheck_Wire.hxx>
#include <BRepOffsetAPI_MakeOffset.hxx>
#include <TopExp.hxx>
#include <BRep_Tool.hxx>
#include <ShapeFix_Face.hxx>
#include <ShapeFix_Wire.hxx>
#include <Geom_Curve.hxx>
#include <Geom_Line.hxx>
#include <Standard_Type.hxx>
#include <GCPnts_UniformAbscissa.hxx>
#include <BRepTools_WireExplorer.hxx>
#include <ProjLib_ProjectOnPlane.hxx>
#include <GeomAPI.hxx>
#include <ShapeFix_Edge.hxx>
#include <ShapeFix_Wire.hxx>
#include <BRepTools_ReShape.hxx>
#include <GCPnts_AbscissaPoint.hxx>
#include <BRepBuilderAPI_FindPlane.hxx>
#include <BRepLib.hxx>
#include <ShapeFix_Shape.hxx>
#include <IntAna_IntConicQuad.hxx>
#include <GeomAPI_ProjectPointOnSurf.hxx>

#include <Prs3d_ShapeTool.hxx>

#include <Geom_TrimmedCurve.hxx>
#include <gp_Lin.hxx>
#include <gp_Pln.hxx>

#include <gp_Quaternion.hxx>
#include <TopTools_IndexedMapOfShape.hxx>

template double helperFunctions::getLowestZ<TopoDS_Face>(const TopoDS_Face& shape);
template double helperFunctions::getLowestZ<TopoDS_Shell>(const TopoDS_Shell& shape);
template double helperFunctions::getLowestZ<TopoDS_Solid>(const TopoDS_Solid& shape);
template double helperFunctions::getLowestZ<TopoDS_Shape>(const TopoDS_Shape& shape);

template double helperFunctions::getAverageZ<TopoDS_Face>(const TopoDS_Face& shape);
template double helperFunctions::getAverageZ<TopoDS_Shell>(const TopoDS_Shell& shape);
template double helperFunctions::getAverageZ<TopoDS_Solid>(const TopoDS_Solid& shape);
template double helperFunctions::getAverageZ<TopoDS_Shape>(const TopoDS_Shape& shape);

template double helperFunctions::getHighestZ<TopoDS_Face>(const TopoDS_Face& shape);
template double helperFunctions::getHighestZ<TopoDS_Shell>(const TopoDS_Shell& shape);
template double helperFunctions::getHighestZ<TopoDS_Solid>(const TopoDS_Solid& shape);
template double helperFunctions::getHighestZ<TopoDS_Shape>(const TopoDS_Shape& shape);

template double helperFunctions::getHighestZ<TopoDS_Face>(const std::vector<TopoDS_Face>& faceList);
template double helperFunctions::getHighestZ<TopoDS_Shell>(const std::vector<TopoDS_Shell>& faceList);
template double helperFunctions::getHighestZ<TopoDS_Solid>(const std::vector<TopoDS_Solid>& faceList);
template double helperFunctions::getHighestZ<TopoDS_Shape>(const std::vector<TopoDS_Shape>& faceList);

template void helperFunctions::bBoxDiagonal<TopoDS_Edge>(const std::vector<TopoDS_Edge>& theShapeList, gp_Pnt* lllPoint, gp_Pnt* urrPoint, const double buffer, const double angle, const double secondAngle);
template void helperFunctions::bBoxDiagonal<TopoDS_Face>(const std::vector<TopoDS_Face>& theShapeList, gp_Pnt* lllPoint, gp_Pnt* urrPoint, const double buffer, const double angle, const double secondAngle);
template void helperFunctions::bBoxDiagonal<TopoDS_Shell>(const std::vector<TopoDS_Shell>& theShapeList, gp_Pnt* lllPoint, gp_Pnt* urrPoint, const double buffer, const double angle, const double secondAngle);
template void helperFunctions::bBoxDiagonal<TopoDS_Solid>(const std::vector<TopoDS_Solid>& theShapeList, gp_Pnt* lllPoint, gp_Pnt* urrPoint, const double buffer, const double angle, const double secondAngle);
template void helperFunctions::bBoxDiagonal<TopoDS_Shape>(const std::vector<TopoDS_Shape>& theShapeList, gp_Pnt* lllPoint, gp_Pnt* urrPoint, const double buffer, const double angle, const double secondAngle);

template void helperFunctions::bBoxDiagonal<TopoDS_Edge>(const TopoDS_Edge& theShapeList, gp_Pnt* lllPoint, gp_Pnt* urrPoint, const double buffer, const double angle, const double secondAngle);
template void helperFunctions::bBoxDiagonal<TopoDS_Face>(const TopoDS_Face& theShapeList, gp_Pnt* lllPoint, gp_Pnt* urrPoint, const double buffer, const double angle, const double secondAngle);
template void helperFunctions::bBoxDiagonal<TopoDS_Shell>(const TopoDS_Shell& theShapeList, gp_Pnt* lllPoint, gp_Pnt* urrPoint, const double buffer, const double angle, const double secondAngle);
template void helperFunctions::bBoxDiagonal<TopoDS_Solid>(const TopoDS_Solid& theShapeList, gp_Pnt* lllPoint, gp_Pnt* urrPoint, const double buffer, const double angle, const double secondAngle);
template void helperFunctions::bBoxDiagonal<TopoDS_Shape>(const TopoDS_Shape& theShapeList, gp_Pnt* lllPoint, gp_Pnt* urrPoint, const double buffer, const double angle, const double secondAngle);

template gp_Vec helperFunctions::computeFaceNormal(const TopoDS_Face& theFace);
template gp_Vec helperFunctions::computeFaceNormal(const TopoDS_Wire& theFace);

template void helperFunctions::writeToSTEP<TopoDS_Face>(const std::vector<TopoDS_Face>& theShapeList, const std::string& targetPath);
template void helperFunctions::writeToSTEP<TopoDS_Shell>(const std::vector<TopoDS_Shell>& theShapeList, const std::string& targetPath);
template void helperFunctions::writeToSTEP<TopoDS_Solid>(const std::vector<TopoDS_Solid>& theShapeList, const std::string& targetPath);
template void helperFunctions::writeToSTEP<TopoDS_Shape>(const std::vector<TopoDS_Shape>& theShapeList, const std::string& targetPath);

template void helperFunctions::writeToSTEP<TopoDS_Face>(const std::vector<std::vector<TopoDS_Face>>& theShapeList, const std::string& targetPath);
template void helperFunctions::writeToSTEP<TopoDS_Shell>(const std::vector<std::vector<TopoDS_Shell>>& theShapeList, const std::string& targetPath);
template void helperFunctions::writeToSTEP<TopoDS_Solid>(const std::vector<std::vector<TopoDS_Solid>>& theShapeList, const std::string& targetPath);
template void helperFunctions::writeToSTEP<TopoDS_Shape>(const std::vector<std::vector<TopoDS_Shape>>& theShapeList, const std::string& targetPath);

template void helperFunctions::writeToOBJ<TopoDS_Face>(const TopoDS_Face& theShapeList, const std::string& targetPath);
template void helperFunctions::writeToOBJ<TopoDS_Shell>(const TopoDS_Shell& theShapeList, const std::string& targetPath);
template void helperFunctions::writeToOBJ<TopoDS_Solid>(const TopoDS_Solid& theShapeList, const std::string& targetPath);
template void helperFunctions::writeToOBJ<TopoDS_Shape>(const TopoDS_Shape& theShapeList, const std::string& targetPath);
template void helperFunctions::writeToOBJ<TopoDS_Compound>(const TopoDS_Compound& theShapeList, const std::string& targetPath);

template void helperFunctions::writeToOBJ<TopoDS_Face>(const std::vector<TopoDS_Face>& theShapeList, const std::string& targetPath);
template void helperFunctions::writeToOBJ<TopoDS_Shell>(const std::vector<TopoDS_Shell>& theShapeList, const std::string& targetPath);
template void helperFunctions::writeToOBJ<TopoDS_Solid>(const std::vector<TopoDS_Solid>& theShapeList, const std::string& targetPath);
template void helperFunctions::writeToOBJ<TopoDS_Shape>(const std::vector<TopoDS_Shape>& theShapeList, const std::string& targetPath);
template void helperFunctions::writeToOBJ<TopoDS_Compound>(const std::vector<TopoDS_Compound>& theShapeList, const std::string& targetPath);

template void helperFunctions::writeToOBJ<TopoDS_Face>(const std::vector<std::vector<TopoDS_Face>>& theShapeList, const std::string& targetPath);
template void helperFunctions::writeToOBJ<TopoDS_Shell>(const std::vector<std::vector<TopoDS_Shell>>& theShapeList, const std::string& targetPath);
template void helperFunctions::writeToOBJ<TopoDS_Solid>(const std::vector<std::vector<TopoDS_Solid>>& theShapeList, const std::string& targetPath);
template void helperFunctions::writeToOBJ<TopoDS_Shape>(const std::vector<std::vector<TopoDS_Shape>>& theShapeList, const std::string& targetPath);

template std::vector<TopoDS_Face> helperFunctions::sortShapes(const std::vector<TopoDS_Face>& shapeList, const std::vector<double>& sortingValues);
template std::vector<TopoDS_Shape> helperFunctions::sortShapes(const std::vector<TopoDS_Shape>& shapeList, const std::vector<double>& sortingValues);


struct IntXYZ {
	int64_t x, y, z;

	IntXYZ(const gp_XYZ& p, double invTol) {
		x = static_cast<int64_t>(std::floor(p.X() * invTol + 0.5));
		y = static_cast<int64_t>(std::floor(p.Y() * invTol + 0.5));
		z = static_cast<int64_t>(std::floor(p.Z() * invTol + 0.5));
	}
};

struct IntXYZ_Hash {
	std::size_t operator()(const IntXYZ& p) const {
		std::size_t hx = std::hash<long long>()(p.x);
		std::size_t hy = std::hash<long long>()(p.y);
		std::size_t hz = std::hash<long long>()(p.x);
		return hx ^ (hy << 1) ^ (hz << 2);
	}
};

struct IntXYZ_Equal {
	bool operator()(const IntXYZ& a, const IntXYZ& b) const {
		return (a.x == b.x && a.y == b.y && a.z == b.z);
	}
};

// merges halfedges with the same direction into one single halfedge
std::vector<HalfEdge> cleanHalfEdgeList(std::vector<HalfEdge> halfEdgeList) 
{
	if (halfEdgeList.empty()) { return{}; }

	double precision = SettingsCollection::getInstance().linearTolerance();
	std::vector<HalfEdge> cleanedHalfEdgeList;
	gp_Vec startDir = halfEdgeList[0].getDir();
	for (size_t i = 1; i < halfEdgeList.size(); i++)
	{
		const HalfEdge& currentEdge = halfEdgeList[i];
		if (startDir.IsParallel(currentEdge.getDir(), precision)) { continue; }
		std::rotate(halfEdgeList.begin(), halfEdgeList.begin() + i, halfEdgeList.end());
		break;
	}

	gp_Pnt basePoint = halfEdgeList[0].p1_;
	gp_Vec baseDir = halfEdgeList[0].getDir();
	for (size_t i = 1; i < halfEdgeList.size(); i++)
	{
		const HalfEdge& currentEdge = halfEdgeList[i];
		if (baseDir.IsParallel(currentEdge.getDir(), precision)) { continue; }
		if (basePoint.IsEqual(currentEdge.p1_, precision)) { continue; }

		HalfEdge cleanedEdge = HalfEdge(basePoint, currentEdge.p1_);
		cleanedHalfEdgeList.emplace_back(cleanedEdge);
		basePoint = currentEdge.p1_;
		baseDir = currentEdge.getDir();
	}

	if (!basePoint.IsEqual(halfEdgeList.begin()->p1_, precision))
	{
		cleanedHalfEdgeList.emplace_back(HalfEdge(basePoint, halfEdgeList.begin()->p1_));
	}
	
	return cleanedHalfEdgeList;
}

BoostPoint3D helperFunctions::Point3DOTB(const gp_Pnt& oP) {
	return BoostPoint3D(oP.X(), oP.Y(), oP.Z());
}

gp_Pnt helperFunctions::Point3DBTO(const BoostPoint3D& oP) {
	return gp_Pnt(bg::get<0>(oP), bg::get<1>(oP), bg::get<2>(oP));
}

double triangleArea2D(const gp_Pnt& p1, const gp_Pnt& p2, const gp_Pnt& p3) {
	return abs((p1.X() * (p2.Y() - p3.Y()) + p2.X() * (p3.X() - p1.Y()) + p3.X() * (p1.Y() - p2.Y())) / 2.0);
}

std::vector<gp_Pnt> helperFunctions::getUniquePoints(const TopoDS_Shape& inputShape) //TODO: check for triangles
{
	std::vector<gp_Pnt> uniquePoints;
	TopTools_IndexedMapOfShape vertexMap;
	TopExp::MapShapes(inputShape, TopAbs_VERTEX, vertexMap);

	for (int i = 1; i <= vertexMap.Extent(); ++i) {
		TopoDS_Vertex vertex = TopoDS::Vertex(vertexMap(i));
		uniquePoints.emplace_back(BRep_Tool::Pnt(vertex));
	}
	return uniquePoints;
}

std::vector<gp_Pnt> helperFunctions::getPoints(const TopoDS_Shape& shape)
{
	if (shape.IsNull()) { return {}; }

	std::vector<gp_Pnt> pointList;
	for (TopExp_Explorer expl(shape, TopAbs_VERTEX); expl.More(); expl.Next())
	{
		TopoDS_Vertex vertex = TopoDS::Vertex(expl.Current());
		pointList.emplace_back(BRep_Tool::Pnt(vertex));
	}
	return pointList;
}

int helperFunctions::getPointCount(const TopoDS_Shape& inputShape)
{
	TopTools_IndexedMapOfShape vertexMap;
	TopExp::MapShapes(inputShape, TopAbs_VERTEX, vertexMap);
	return vertexMap.Extent();
}

gp_Pnt helperFunctions::rotatePointWorld(const gp_Pnt& p, double angle) {
	double pX = p.X();
	double pY = p.Y();
	double pZ = p.Z();

	return gp_Pnt(pX * cos(angle) - pY * sin(angle), pY * cos(angle) + pX * sin(angle), pZ);
}

BoostPoint3D helperFunctions::rotatePointWorld(const BoostPoint3D& p, double angle) {
	double pX = bg::get<0>(p);
	double pY = bg::get<1>(p);
	double pZ = bg::get<2>(p);

	return BoostPoint3D(pX * cos(angle) - pY * sin(angle), pY * cos(angle) + pX * sin(angle), pZ);
}

gp_Pnt helperFunctions::rotatePointPoint(const gp_Pnt& p, const gp_Pnt& anchorP, const double& angle)
{
	gp_Pnt translatedP = p.Translated(gp_Vec(-anchorP.X(), -anchorP.Y(), -anchorP.Z()));
	gp_Pnt rotatedP = helperFunctions::rotatePointWorld(translatedP, angle);
	return rotatedP.Translated(gp_Vec(anchorP.X(), anchorP.Y(), anchorP.Z()));
}

std::vector<gp_Pnt> helperFunctions::getPointGridOnSurface(const TopoDS_Face& theface, const double& resolution)
{
	SettingsCollection& settingsCollection = SettingsCollection::getInstance();
	double precision = settingsCollection.linearTolerance();
	int minSurfacePoints = settingsCollection.minGridPointCount(); 

	// greate points on grid over surface
	// get the uv bounds to create a point grid on the surface
	Standard_Real uMin, uMax, vMin, vMax;
	BRepTools::UVBounds(theface, BRepTools::OuterWire(theface), uMin, uMax, vMin, vMax);

	int numUPoints = static_cast<int>(ceil(abs(uMax - uMin) / resolution));
	int numVPoints = static_cast<int>(ceil(abs(vMax - vMin) / resolution));

	// set num of points if min/max rule is not met
	if (numUPoints <= minSurfacePoints) { numUPoints = minSurfacePoints; }
	if (numVPoints <= minSurfacePoints) { numVPoints = minSurfacePoints; }

	std::vector<gp_Pnt> gridPointList;
	// create grid
	std::vector<TopoDS_Wire> wires;
	for (TopExp_Explorer expl(theface, TopAbs_WIRE); expl.More(); expl.Next())
	{
		wires.push_back(TopoDS::Wire(expl.Current()));
	}

	triangulateShape(theface);
	TopLoc_Location loc;
	Handle(Poly_Triangulation) tri = BRep_Tool::Triangulation(theface, loc);
	if (tri.IsNull()) { return {}; }
	if (!tri->HasUVNodes()) { return {}; }

	Handle(Geom_Surface) surface = BRep_Tool::Surface(theface);
	double uStep = (uMax - uMin) / (numUPoints - 1);
	double vStep = (vMax - vMin) / (numVPoints - 1);

	bool offset = false;
	for (int i = 0; i < numUPoints; ++i)
	{
		double u = uMin + i * uStep;
		for (int j = 0; j < numVPoints; ++j)
		{
			double v = vMin + j * vStep + 0.5 * vStep;
			if (offset) { v += 0.5 * vStep; }

			gp_Pnt2d uvCoord(u, v);
			gp_Pnt point;
			surface->D0(u, v, point);

			if (!pointOnMesh(tri, loc, point)) { continue; }

			bool notOnWire = true;
			for (const TopoDS_Wire& currentWire : wires)
			{
				if (helperFunctions::pointOnWire(currentWire, point, precision * 10))
				{
					notOnWire = false;
					break;
				}
			}
			if (!notOnWire)
			{
				continue;
			}
			gridPointList.emplace_back(point);
		}
		offset = !offset;
	}
	return gridPointList;
}

std::vector<gp_Pnt> helperFunctions::getPointGridOnWire(const TopoDS_Face& theface, const double& resolution)
{
	SettingsCollection& settingsCollection = SettingsCollection::getInstance();
	double precision = settingsCollection.linearTolerance();

	//if (helperFunctions::computeArea(theface) < 1e-5) { return {}; }
	std::vector<gp_Pnt> wirePointList;

	int edgeCount = 0;
	TopoDS_Face faceLocalCopy = theface;
	for (TopExp_Explorer exp(theface, TopAbs_EDGE); exp.More(); exp.Next()) {
		edgeCount++;
		const TopoDS_Edge& edge = TopoDS::Edge(exp.Current());
		Standard_Real first, last;
		Handle(Geom_Curve) curve = BRep_Tool::Curve(edge, first, last);
		if (curve.IsNull()) {
			return {};  // Skip degenerated edges
		}
		if (!curve->IsKind(STANDARD_TYPE(Geom_Line))) {

			std::vector<TopoDS_Face> tesselatedFace = TessellateFace(theface);
			if (tesselatedFace.size() != 1)
			{
				return {};
			}
			faceLocalCopy = tesselatedFace[0];
		}
	}
	if (edgeCount < 3) { return {}; }

	SettingsCollection::getInstance().getWireOffsetterMutex()->lock();
	BRepOffsetAPI_MakeOffset offsetter(BRepTools::OuterWire(theface), GeomAbs_Intersection);
	offsetter.Perform(-precision * 10);
	SettingsCollection::getInstance().getWireOffsetterMutex()->unlock();

	if (!offsetter.IsDone()) { return {}; }
	const TopoDS_Shape offsettedFace = offsetter.Shape();
	if (offsettedFace.IsNull()) { return {}; }

	for (TopExp_Explorer expl(offsetter.Shape(), TopAbs_EDGE); expl.More(); expl.Next())
	{
		const TopoDS_Edge& currentEdge = TopoDS::Edge(expl.Current());
		BRepAdaptor_Curve curveAdaptor(currentEdge);

		double uStart = curveAdaptor.Curve().FirstParameter();
		double uEnd = curveAdaptor.Curve().LastParameter();
		int numUPoints = static_cast<int>(ceil(abs(uStart - uEnd)) / resolution);

		if (numUPoints < 2) { numUPoints = 2; }
		else if (numUPoints > 10) { numUPoints = 10; }

		double uStep = abs(uStart - uEnd) / (numUPoints - 1);
		bool t = false;
		for (double u = uStart; u < uEnd; u += uStep) {
			gp_Pnt point;
			curveAdaptor.D0(u, point);
			wirePointList.emplace_back(point);
			t = true;
		}
	}

	return wirePointList;
}

bool helperFunctions::pointIsSame(const BoostPoint3D& lp, const BoostPoint3D& rp)
{
	double precision = SettingsCollection::getInstance().linearTolerance();

	if (abs(lp.get<0>() - rp.get<0>()) > precision ) { return false; }
	if (abs(lp.get<1>() - rp.get<1>()) > precision ) { return false; }
	if (abs(lp.get<2>() - rp.get<2>()) > precision ) { return false; }
	return true;
}


template<typename T>
void helperFunctions::bBoxDiagonal(const std::vector<T>& theShapeList, gp_Pnt* lllPoint, gp_Pnt* urrPoint, const double buffer, const double angle, const double secondAngle)
{
	for (const T& theShape : theShapeList) { bBoxDiagonal(theShape, lllPoint, urrPoint, buffer, angle, secondAngle); }
}

template<typename T>
void helperFunctions::bBoxDiagonal(const T& theShape, gp_Pnt* lllPoint, gp_Pnt* urrPoint, const double buffer, const double angle, const double secondAngle)
{
	if (lllPoint->Distance(*urrPoint) < SettingsCollection::getInstance().linearTolerance() && 
		lllPoint->Distance(gp_Pnt(0,0,0)) < SettingsCollection::getInstance().linearTolerance() &&
		urrPoint->Distance(gp_Pnt(0, 0, 0)) < SettingsCollection::getInstance().linearTolerance())
	{
		lllPoint->SetCoord(9999999, 9999999, 9999999);
		urrPoint->SetCoord(-9999999, -9999999, -9999999);
	}

	for (TopExp_Explorer expl(theShape, TopAbs_VERTEX); expl.More(); expl.Next())
	{
		TopoDS_Vertex vertex = TopoDS::Vertex(expl.Current());
		gp_Pnt point = helperFunctions::rotatePointWorld(BRep_Tool::Pnt(vertex), angle).Rotated(gp_Ax1(gp_Pnt(0, 0, 0), gp_Dir(0, 1, 0)), -secondAngle);

		if (point.X() > urrPoint->X()) { urrPoint->SetX(point.X()); }
		if (point.Y() > urrPoint->Y()) { urrPoint->SetY(point.Y()); }
		if (point.Z() > urrPoint->Z()) { urrPoint->SetZ(point.Z()); }

		if (point.X() < lllPoint->X()) { lllPoint->SetX(point.X()); }
		if (point.Y() < lllPoint->Y()) { lllPoint->SetY(point.Y()); }
		if (point.Z() < lllPoint->Z()) { lllPoint->SetZ(point.Z()); }
	}

	applyBuffer(lllPoint, urrPoint, buffer);
	return;
}

bool helperFunctions::bBoxDiagonal(const std::vector<gp_Pnt>& pointList, gp_Pnt* lllPoint, gp_Pnt* urrPoint, const double buffer, const double angle, const double secondAngle)
{
	*lllPoint = helperFunctions::rotatePointWorld(pointList[0], angle).Rotated(gp_Ax1(gp_Pnt(0, 0, 0), gp_Dir(0, 1, 0)), -secondAngle);
	*urrPoint = helperFunctions::rotatePointWorld(pointList[0], angle).Rotated(gp_Ax1(gp_Pnt(0, 0, 0), gp_Dir(0, 1, 0)), -secondAngle);

	for (size_t i = 1; i < pointList.size(); i++)
	{
		gp_Pnt point = helperFunctions::rotatePointWorld(pointList[i], angle).Rotated(gp_Ax1(gp_Pnt(0, 0, 0), gp_Dir(0, 1, 0)), -secondAngle);

		if (point.X() < lllPoint->X()) { lllPoint->SetX(point.X()); }
		if (point.Y() < lllPoint->Y()) { lllPoint->SetY(point.Y()); }
		if (point.Z() < lllPoint->Z()) { lllPoint->SetZ(point.Z()); }

		if (point.X() > urrPoint->X()) { urrPoint->SetX(point.X()); }
		if (point.Y() > urrPoint->Y()) { urrPoint->SetY(point.Y()); }
		if (point.Z() > urrPoint->Z()) { urrPoint->SetZ(point.Z()); }
	}
	if (lllPoint->IsEqual(*urrPoint, 0.01)) { return false; }

	applyBuffer(lllPoint, urrPoint, buffer);
	return true;
}

void helperFunctions::bBoxOrientated(const std::vector<gp_Pnt>& pointList, gp_Pnt* lllPoint, gp_Pnt* urrPoint, double* rotationAngle, const double buffer)
{
	SettingsCollection& settingsCollection = SettingsCollection::getInstance();

	// approximate smalles bbox
	double angle = 22.5 * (M_PI / 180);
	int maxIt = 15;
	double smallestDistance = lllPoint->Distance(*urrPoint);

	for (size_t i = 0; i < maxIt; i++)
	{
		std::tuple<gp_Pnt, gp_Pnt, double> left;
		std::tuple<gp_Pnt, gp_Pnt, double> right;

		gp_Pnt leftLllPoint;
		gp_Pnt leftUrrPoint;
		gp_Pnt rghtLllPoint;
		gp_Pnt rghtUrrPoint;

		bBoxDiagonal(pointList, &leftLllPoint, &leftUrrPoint, 0, *rotationAngle - angle);
		bBoxDiagonal(pointList, &rghtLllPoint, &rghtUrrPoint, 0, *rotationAngle + angle);

		double leftDistance = leftLllPoint.Distance(leftUrrPoint);
		double rghtDistance = rghtLllPoint.Distance(rghtUrrPoint);

		if (leftDistance > rghtDistance && smallestDistance > rghtDistance)
		{
			*rotationAngle = *rotationAngle + angle;
			smallestDistance = rghtDistance;
			*lllPoint = rghtLllPoint;
			*urrPoint = rghtUrrPoint;
		}
		else if (smallestDistance > leftDistance)
		{
			*rotationAngle = *rotationAngle - angle;
			smallestDistance = leftDistance;
			*lllPoint = leftLllPoint;
			*urrPoint = leftUrrPoint;
		}
		angle = angle / 2;
	}
	applyBuffer(lllPoint, urrPoint, buffer);
	return;
}

bg::model::box <BoostPoint3D> helperFunctions::createBBox(const TopoDS_Shape& shape, double buffer)
{
	return createBBox(std::vector<TopoDS_Shape>{shape}, buffer);
}

bg::model::box <BoostPoint3D> helperFunctions::createBBox(const std::vector<TopoDS_Shape>& shape, double buffer) 
{
	Bnd_Box boundingBox;
	for (size_t i = 0; i < shape.size(); i++)
	{
		BRepBndLib::Add(shape[i], boundingBox);
	}

	if (boundingBox.IsVoid()) { return {}; }

	Standard_Real minX, minY, minZ, maxX, maxY, maxZ;
	boundingBox.Get(minX, minY, minZ, maxX, maxY, maxZ);

	return  bg::model::box < BoostPoint3D >(
		BoostPoint3D(minX - buffer, minY - buffer, minZ - buffer),
		BoostPoint3D(maxX + buffer, maxY + buffer, maxZ + buffer)
		);
}

bg::model::box <BoostPoint3D> helperFunctions::createBBox(const std::vector<gp_Pnt>& pointList, double buffer)
{
	gp_Pnt lll;
	gp_Pnt urr;

	bBoxDiagonal(pointList, &lll, &urr, 0);
	return  bg::model::box < BoostPoint3D >(
		BoostPoint3D(lll.X() - buffer, lll.Y() - buffer, lll.Z() - buffer),
		BoostPoint3D(urr.X() + buffer, urr.Y() + buffer, urr.Z() + buffer)
		);
}

bg::model::box<BoostPoint3D> helperFunctions::createBBox(const std::array<gp_Pnt, 3>& pointList, double buffer)
{
	// get proper order for the bbox
	gp_Pnt p1 = pointList[0];
	gp_Pnt p2 = pointList[1];
	gp_Pnt p3 = pointList[2];

	gp_Pnt lll(
		std::min({ p1.X(), p2.X(), p3.X() }),
		std::min({ p1.Y(), p2.Y(), p3.Y() }),
		std::min({ p1.Z(), p2.Z(), p3.Z() })
	);

	gp_Pnt urr(
		std::max({ p1.X(), p2.X(), p3.X() }),
		std::max({ p1.Y(), p2.Y(), p3.Y() }),
		std::max({ p1.Z(), p2.Z(), p3.Z() })
	);

	BoostPoint3D boostlllPoint = BoostPoint3D(lll.X() - buffer, lll.Y() - buffer, lll.Z() - buffer);
	BoostPoint3D boosturrPoint = BoostPoint3D(urr.X() + buffer, urr.Y() + buffer, urr.Z() + buffer);
	bg::model::box <BoostPoint3D> box = bg::model::box < BoostPoint3D >(boostlllPoint, boosturrPoint);

	return box;
}

bg::model::box <BoostPoint3D>  helperFunctions::createBBox(const gp_Pnt& p1, const gp_Pnt& p2, double buffer) 
{
	// get proper order for the bbox
	gp_Pnt lll(
		std::min(p1.X(), p2.X()),
		std::min(p1.Y(), p2.Y()),
		std::min(p1.Z(), p2.Z())
	);

	gp_Pnt urr(
		std::max(p1.X(), p2.X()),
		std::max(p1.Y(), p2.Y()),
		std::max(p1.Z(), p2.Z())
	);

	BoostPoint3D boostlllPoint = BoostPoint3D(lll.X() - buffer, lll.Y() - buffer, lll.Z() - buffer);
	BoostPoint3D boosturrPoint = BoostPoint3D(urr.X() + buffer, urr.Y() + buffer, urr.Z() + buffer);
	bg::model::box <BoostPoint3D> box = bg::model::box < BoostPoint3D >(boostlllPoint, boosturrPoint);

	return box;
}

TopoDS_Shape helperFunctions::createBBOXOCCT(const gp_Pnt& lll, const gp_Pnt& urr, double buffer, double horizontalAngle, double verticalAngle) 
{
	if (abs(urr.X() - lll.X()) < SettingsCollection::getInstance().linearTolerance()) { return TopoDS_Solid(); }
	if (abs(urr.Y() - lll.Y()) < SettingsCollection::getInstance().linearTolerance()) { return TopoDS_Solid(); }
	if (abs(urr.Z() - lll.Z()) < SettingsCollection::getInstance().linearTolerance()) { return TopoDS_Solid(); }
	
	gp_Ax1 vertRotation(gp_Pnt(0, 0, 0), gp_Dir(0, 1, 0));

	BRep_Builder brepBuilder;
	TopoDS_Shell shell;
	brepBuilder.MakeShell(shell);
	TopoDS_Solid solidbox;
	brepBuilder.MakeSolid(solidbox);

	gp_Pnt p0(helperFunctions::rotatePointWorld(lll.Rotated(vertRotation, verticalAngle), -horizontalAngle));
	gp_Pnt p1 = helperFunctions::rotatePointWorld(gp_Pnt(lll.X(), urr.Y(), lll.Z()).Rotated(vertRotation, verticalAngle), -horizontalAngle);
	gp_Pnt p2 = helperFunctions::rotatePointWorld(gp_Pnt(urr.X(), urr.Y(), lll.Z()).Rotated(vertRotation, verticalAngle), -horizontalAngle);
	gp_Pnt p3 = helperFunctions::rotatePointWorld(gp_Pnt(urr.X(), lll.Y(), lll.Z()).Rotated(vertRotation, verticalAngle), -horizontalAngle);

	gp_Pnt p4(helperFunctions::rotatePointWorld(urr.Rotated(vertRotation, verticalAngle), -horizontalAngle));
	gp_Pnt p5 = helperFunctions::rotatePointWorld(gp_Pnt(lll.X(), urr.Y(), urr.Z()).Rotated(vertRotation, verticalAngle), -horizontalAngle);
	gp_Pnt p6 = helperFunctions::rotatePointWorld(gp_Pnt(lll.X(), lll.Y(), urr.Z()).Rotated(vertRotation, verticalAngle), -horizontalAngle);
	gp_Pnt p7 = helperFunctions::rotatePointWorld(gp_Pnt(urr.X(), lll.Y(), urr.Z()).Rotated(vertRotation, verticalAngle), -horizontalAngle);

	std::vector<TopoDS_Face> faceList;
	faceList.emplace_back(helperFunctions::createPlanarFace(p0, p1, p2, p3)); // bottom
	faceList.emplace_back(helperFunctions::createPlanarFace(p4, p5, p6, p7)); // top
	faceList.emplace_back(helperFunctions::createPlanarFace(p0, p3, p7, p6)); // front 
	faceList.emplace_back(helperFunctions::createPlanarFace(p1, p5, p4, p2)); // back
	faceList.emplace_back(helperFunctions::createPlanarFace(p0, p6, p5, p1)); // left
	faceList.emplace_back(helperFunctions::createPlanarFace(p4, p7, p3, p2)); // right

	BRepBuilderAPI_Sewing brepSewer;
	for (size_t k = 0; k < faceList.size(); k++) { brepSewer.Add(faceList[k]); }
	brepSewer.Perform();
	brepBuilder.Add(solidbox, brepSewer.SewedShape());
	return solidbox;
}

TopoDS_Shape helperFunctions::boxSimplefyShape(const TopoDS_Shape& shape)
{
	double angularTolerance = SettingsCollection::getInstance().angularTolerance();
	double precision = SettingsCollection::getInstance().linearTolerance();
	// get the vectors of the shape
	std::vector<gp_Pnt> pointList = getPoints(shape);
	gp_Vec hVector = getShapedir(pointList, true);
	gp_Vec vVector = getShapedir(pointList, false);

	if (hVector.Magnitude()  < precision) { hVector = gp_Vec(1, 0, 0); }
	if (vVector.Magnitude()  < precision) { vVector = gp_Vec(0, 0, 1); }

	// compute rotation around z axis
	gp_Pnt p1 = gp_Pnt(0, 0, 0);
	gp_Pnt p2 = p1.Translated(hVector);

	double angleFlat = 0;
	// apply rotation around z axis if required
	if (abs(p1.Y() - p2.Y()) > angularTolerance)
	{
		double os = abs(p1.Y() - p2.Y()) / p1.Distance(p2);
		angleFlat = asin(os);

		gp_Pnt tempP = helperFunctions::rotatePointPoint(p2, p1, angleFlat);

		// mirror the rotation if incorrect
		if (Abs(p1.X() - tempP.X()) > angularTolerance &&
			Abs(p1.Y() - tempP.Y()) > angularTolerance)
		{
			angleFlat = -angleFlat;
		}
	}

	// rotate the box around the x axis to correctly place the roation axis for the x rotation
	gp_Pnt p3 = gp_Pnt(0, 0, 0);
	gp_Pnt p4 = helperFunctions::rotatePointPoint(p3.Translated(vVector), p3, angleFlat);
	if (abs(p3.X() - p4.X()) < angularTolerance)
	{
		p3 = helperFunctions::rotatePointWorld(p3, M_PI / 2.0);
		p4 = helperFunctions::rotatePointWorld(p4, M_PI / 2.0);
		angleFlat += M_PI / 2.0;
	}

	// compute vertical rotation
	double angleVert = acos(abs(p4.Z() - p3.Z()) / p3.Distance(p4));
	p4.Rotate(gp_Ax1(p3, gp_Vec(0,1,0)), angleVert);
	// mirror the rotation if incorrect
	if (abs(p3.X() - p4.X()) < angularTolerance)
	{
		angleVert = -angleVert;
		p4.Rotate(gp_Ax1(p3, gp_Vec(0, 1, 0)), 2 * angleVert);
	}

	gp_Pnt lllPoint;
	gp_Pnt urrPoint;
	helperFunctions::bBoxDiagonal(pointList, &lllPoint, &urrPoint, SettingsCollection::getInstance().windowBuffer(), angleFlat, angleVert);
	if (lllPoint.IsEqual(urrPoint, SettingsCollection::getInstance().linearTolerance())) { return TopoDS_Shape(); }
	TopoDS_Shape boxShape = helperFunctions::createBBOXOCCT(lllPoint, urrPoint, 0.0, angleFlat, angleVert);
	helperFunctions::triangulateShape(boxShape);
	return boxShape;
}

void helperFunctions::applyBuffer(gp_Pnt* lllPoint, gp_Pnt* urrPoint, double buffer)
{
	urrPoint->SetX(urrPoint->X() + buffer);
	urrPoint->SetY(urrPoint->Y() + buffer);
	urrPoint->SetZ(urrPoint->Z() + buffer);
	lllPoint->SetX(lllPoint->X() - buffer);
	lllPoint->SetY(lllPoint->Y() - buffer);
	lllPoint->SetZ(lllPoint->Z() - buffer);
	return;
}


template<typename T>
double helperFunctions::getLowestZ(const T& shape)
{
	double lowestZ = 999999999;
	for (TopExp_Explorer expl(shape, TopAbs_VERTEX); expl.More(); expl.Next())
	{
		TopoDS_Vertex vertex = TopoDS::Vertex(expl.Current());
		gp_Pnt p = BRep_Tool::Pnt(vertex);

		if (p.Z() < lowestZ) { lowestZ = p.Z(); }
	}
	return lowestZ;
}

template<typename T>
double helperFunctions::getHighestZ(const T& shape)
{
	double highestZ = -999999999;
	for (TopExp_Explorer expl(shape, TopAbs_VERTEX); expl.More(); expl.Next())
	{
		TopoDS_Vertex vertex = TopoDS::Vertex(expl.Current());
		gp_Pnt p = BRep_Tool::Pnt(vertex);

		if (p.Z() > highestZ) { highestZ = p.Z(); }
	}
	return highestZ;
}

template<typename T>
double helperFunctions::getHighestZ(const std::vector<T>& faceList) {
	double maxHeight = -999999;

	for (const T& currentFace : faceList)
	{
		double currentHeight = getHighestZ(currentFace);
		if (currentHeight > maxHeight) { maxHeight = currentHeight; }
	}
	return maxHeight;
}

template<typename T>
double helperFunctions::getAverageZ(const T& shape) {
	double totalZ = 0;
	int pCount = 0;
	for (TopExp_Explorer expl(shape, TopAbs_VERTEX); expl.More(); expl.Next())
	{
		TopoDS_Vertex vertex = TopoDS::Vertex(expl.Current());
		gp_Pnt p = BRep_Tool::Pnt(vertex);

		totalZ += p.Z();
		pCount++;
	}
	return totalZ / pCount;
}

template<typename T>
double helperFunctions::getAZ(const T& shape)
{
	for (TopExp_Explorer expl(shape, TopAbs_VERTEX); expl.More(); expl.Next())
	{
		TopoDS_Vertex vertex = TopoDS::Vertex(expl.Current());
		gp_Pnt p = BRep_Tool::Pnt(vertex);
		return p.Z();
	}
	return 0.0;
}


gp_Pnt helperFunctions::getTriangleCenter(const Handle(Poly_Triangulation)& mesh, const Poly_Triangle& theTriangle, const TopLoc_Location& loc) {

	gp_Pnt p1 = mesh->Node(theTriangle(1)).Transformed(loc);
	gp_Pnt p2 = mesh->Node(theTriangle(2)).Transformed(loc);
	gp_Pnt p3 = mesh->Node(theTriangle(3)).Transformed(loc);

	gp_Pnt middlePoint = gp_Pnt(
		(p1.X() + p2.X() + p3.X()) / 3,
		(p1.Y() + p2.Y() + p3.Y()) / 3,
		(p1.Z() + p2.Z() + p3.Z()) / 3
	);
	return middlePoint;
}

std::optional<gp_Pnt> helperFunctions::getPointOnFace(const TopoDS_Face& theFace) 
{
	triangulateShape(theFace);
	TopLoc_Location loc;
	Handle(Poly_Triangulation) mesh = BRep_Tool::Triangulation(theFace, loc);

	if (mesh.IsNull()) { return std::nullopt; }
	if (mesh.get()->NbTriangles() == 1) { return std::nullopt; }

	double bestArea = -1.0;
	gp_Pnt bestPoint;
	for (int i = 1; i <= mesh.get()->NbTriangles(); i++) 
	{
		const Poly_Triangle& tri = mesh->Triangles().Value(i);

		Standard_Integer n1, n2, n3;
		tri.Get(n1, n2, n3);

		gp_Pnt p1 = mesh->Node(n1).Transformed(loc.Transformation());
		gp_Pnt p2 = mesh->Node(n2).Transformed(loc.Transformation());
		gp_Pnt p3 = mesh->Node(n3).Transformed(loc.Transformation());

		gp_Vec v1(p1, p2);
		gp_Vec v2(p1, p3);

		double area = v1.Crossed(v2).Magnitude();

		if (area > bestArea)
		{
			bestArea = area;
			bestPoint.SetXYZ((p1.XYZ() + p2.XYZ() + p3.XYZ()) / 3.0);
		}
	}

	return bestPoint;
}

std::vector<gp_Pnt> helperFunctions::getPointListOnFace(const TopoDS_Face& theFace)
{
	if (theFace.IsNull())
	{
		return {};
	}
	triangulateShape(theFace);
	try
	{
		TopLoc_Location loc;
		auto mesh = BRep_Tool::Triangulation(theFace, loc);
		if (mesh.IsNull()) { return {}; }

		std::vector<gp_Pnt> pointList;
		for (int i = 1; i <= mesh.get()->NbTriangles(); i++)
		{
			const Poly_Triangle& theTriangle = mesh->Triangles().Value(i);
			pointList.emplace_back(getTriangleCenter(mesh, theTriangle, loc));
		}
		return pointList;
	}
	catch (const std::exception&)
	{
		return {};
	}
}

gp_Pnt helperFunctions::getFirstPointShape(const TopoDS_Shape& shape) {
	TopExp_Explorer vertexExplorer(shape, TopAbs_VERTEX);
	TopoDS_Vertex startVertex;

	if (vertexExplorer.More()) {
		startVertex = TopoDS::Vertex(vertexExplorer.Current());
	}

	gp_Pnt startPoint = BRep_Tool::Pnt(startVertex);
	return startPoint;
}

gp_Pnt helperFunctions::getLastPointShape(const TopoDS_Shape& shape) {
	TopExp_Explorer vertexExplorer(shape, TopAbs_VERTEX);
	TopoDS_Vertex endVertex;

	while (vertexExplorer.More()) {
		endVertex = TopoDS::Vertex(vertexExplorer.Current());
		vertexExplorer.Next();
	}
	return BRep_Tool::Pnt(endVertex);;
}

bool helperFunctions::pointInShape(const TopoDS_Shape& shape, const gp_Pnt& thePoint, double precision)
{
	if (precision == 0.0) { precision = SettingsCollection::getInstance().linearTolerance(); }

	gp_Pnt endPoint = thePoint.Translated(gp_Vec(0, 0, 1000));

	int intCounter = 0;
	for (TopExp_Explorer faceExpl(shape, TopAbs_FACE); faceExpl.More(); faceExpl.Next())
	{
		const TopoDS_Face& currentFace = TopoDS::Face(faceExpl.Current());
		if (LineShapeIntersection(currentFace, thePoint, endPoint))
		{
			intCounter++;
		}
	}
	return intCounter % 2;
}

bool helperFunctions::pointOnShape(const TopoDS_Shape& shape, const gp_Pnt& thePoint, double precision)
{
	if (precision == 0.0) { precision = SettingsCollection::getInstance().linearTolerance(); }

	for (TopExp_Explorer faceExpl(shape, TopAbs_FACE); faceExpl.More(); faceExpl.Next())
	{
		TopoDS_Face currentFace = TopoDS::Face(faceExpl.Current());
		if (pointOnFace(currentFace, thePoint, precision)) { return true; }
	}
	return false;
}

bool helperFunctions::pointOnFace(const TopoDS_Face& theFace, const gp_Pnt& thePoint, double precision)
{
	if (theFace.IsNull()) { return false; }
	if (precision == 0.0) { precision = SettingsCollection::getInstance().linearTolerance(); }
	
	TopLoc_Location loc;
	auto mesh = BRep_Tool::Triangulation(theFace, loc);

	if (mesh.IsNull())
	{
		helperFunctions::triangulateShape(theFace);
		mesh = BRep_Tool::Triangulation(theFace, loc);
	}
	if (mesh.IsNull()) { return false; }

	for (int j = 1; j <= mesh.get()->NbTriangles(); j++) //TODO: if large num indx?
	{
		const Poly_Triangle& theTriangle = mesh->Triangles().Value(j);

		gp_Pnt p1 = mesh->Node(theTriangle(1)).Transformed(loc);
		gp_Pnt p2 = mesh->Node(theTriangle(2)).Transformed(loc);
		gp_Pnt p3 = mesh->Node(theTriangle(3)).Transformed(loc);

		if (pointOnTriangle(thePoint, p1,p2,p3)) { return true; }
	}
	return false;
}

bool helperFunctions::pointOnFace(const std::vector<TopoDS_Face>& theFace, const gp_Pnt& thePoint, double precision)
{
	for (const TopoDS_Face& currentFace : theFace)
	{
		if (pointOnFace(currentFace, thePoint, precision))
		{
			return true;
		}
	}
	return false;
}

bool helperFunctions::pointOnMesh(const Handle(Poly_Triangulation)& theMesh, const TopLoc_Location& loc, const gp_Pnt& thePoint, double precision)
{
	for (int j = 1; j <= theMesh->NbTriangles(); j++) //TODO: if large num indx?
	{
		const Poly_Triangle& theTriangle = theMesh->Triangles().Value(j);

		gp_Pnt p1 = theMesh->Node(theTriangle(1)).Transformed(loc);
		gp_Pnt p2 = theMesh->Node(theTriangle(2)).Transformed(loc);
		gp_Pnt p3 = theMesh->Node(theTriangle(3)).Transformed(loc);

		if (pointOnTriangle(thePoint, p1, p2, p3)) { return true; }
	}
	return false;
}

bool helperFunctions::pointOnTriangle(const gp_Pnt& thePoint, const gp_Pnt& p1, const gp_Pnt& p2, const gp_Pnt& p3)
{
	double precision = SettingsCollection::getInstance().linearTolerance();

	gp_Vec v12 = gp_Vec(p1, p2);
	if (v12.Magnitude() < precision) { return false; }
	gp_Vec v13 = gp_Vec(p1, p3);
	if (v13.Magnitude() < precision) { return false; }

	gp_Vec triangleNormal = v12.Crossed(v13);
	return pointOnTriangle(thePoint, p1, p2, p3, triangleNormal);
	
}

bool helperFunctions::pointOnTriangle(const gp_Pnt& thePoint, const gp_Pnt& p1, const gp_Pnt& p2, const gp_Pnt& p3, const gp_Vec& normal)
{
	double precision = SettingsCollection::getInstance().linearTolerance();

	double area2 = normal.SquareMagnitude();
	if (area2 < precision * precision) { return false; }

	double normalMag = normal.Magnitude();
	double distancePlanePoint = normal.Dot(gp_Vec(p1, thePoint)) / normalMag;
	if (std::abs(distancePlanePoint) > precision) { return false; }

	gp_Vec unitNormal = normal / normalMag; // unit normal
	gp_Pnt projected = thePoint.Translated(-unitNormal * distancePlanePoint);

	if (baryCentricTest(projected, { p1, p2, p3 })) { return true; }
	return false;
}

bool helperFunctions::pointOnTriangle(const gp_Pnt2d& thePoint, const gp_Pnt2d& p1, const gp_Pnt2d& p2, const gp_Pnt2d& p3)
{
	gp_Pnt thePoint3D(thePoint.X(), thePoint.Y(), 0);
	gp_Pnt p13D(p1.X(), p1.Y(), 0);
	gp_Pnt p23D(p2.X(), p2.Y(), 0);
	gp_Pnt p33D(p3.X(), p3.Y(), 0);

	return baryCentricTest(thePoint3D, { p13D, p23D, p33D });
}

bool helperFunctions::uvPointOnMesh(const gp_Pnt2d& thePoint, const TColgp_Array1OfPnt2d& uvNodes, const Poly_Array1OfTriangle& triangles)
{
	for (Standard_Integer i = triangles.Lower(); i <= triangles.Upper(); ++i)
	{
		const Poly_Triangle& thePolyTriangle = triangles(i);

		Standard_Integer n1, n2, n3;
		thePolyTriangle.Get(n1, n2, n3);
		if (helperFunctions::pointOnTriangle(thePoint, uvNodes(n1), uvNodes(n2), uvNodes(n3))) {
			return true;
		}
	}
	return false;
}

bool helperFunctions::pointOnWire(const TopoDS_Face& theFace, const gp_Pnt& thePoint, double precision)
{
	for (TopExp_Explorer expl(theFace, TopAbs_WIRE); expl.More(); expl.Next())
	{
		TopoDS_Wire currentWire = TopoDS::Wire(expl.Current());
		if (pointOnWire(currentWire, thePoint, precision))
		{
			return true;
		}
	}
	return false;
}

bool helperFunctions::pointOnWire(const TopoDS_Wire& theWire, const gp_Pnt& thePoint, double precision)
{
	for (TopExp_Explorer currentExpl(theWire, TopAbs_EDGE); currentExpl.More(); currentExpl.Next())
	{
		TopoDS_Edge currentEdge = TopoDS::Edge(currentExpl.Current());
		if (pointOnEdge(currentEdge, thePoint, precision)) { return true; }
	}
	return false;
}

bool helperFunctions::pointOnEdge(const TopoDS_Edge& theEdge, const gp_Pnt& thePoint, double precision)
{
	if (precision == 0.0) { precision = SettingsCollection::getInstance().linearTolerance(); }

	gp_Pnt p1 = getFirstPointShape(theEdge);
	gp_Pnt p2 = getLastPointShape(theEdge);

	if (p1.Distance(thePoint) < precision) { return true; }
	if (p2.Distance(thePoint) < precision) { return true; }
	if (p1.Distance(p2) < precision) return false;

	gp_Vec v12 = gp_Vec(p1, p2);
	gp_Vec v1p = gp_Vec(p1, thePoint);

	double projectionPar = v1p.Dot(v12) / v12.Dot(v12);

	if (projectionPar < 0.0) projectionPar = 0.0;
	else if (projectionPar > 1.0) projectionPar = 1.0;

	gp_Pnt projectionPoint = p1.XYZ() + projectionPar * v12.XYZ();
	if (projectionPoint.Distance(thePoint) > precision )
	{
		return false;
	}
	return true;
}


gp_Vec helperFunctions::computeEdgeDir(const TopoDS_Edge& theEdge)
{
	double precision = SettingsCollection::getInstance().linearTolerance();
	gp_Pnt startpoint = getFirstPointShape(theEdge);
	gp_Pnt endpoint = getLastPointShape(theEdge);

	if (startpoint.IsEqual(endpoint, precision)) { return gp_Vec(0, 0, 0); }
	return gp_Vec(startpoint, endpoint).Normalized();
}

template<typename T>
gp_Vec helperFunctions::computeFaceNormal(const T& theFace)
{
	if (theFace.IsNull()) { return gp_Vec(0, 0, 0); }

	TopAbs_ShapeEnum shapeType = theFace.ShapeType();
	double precision = SettingsCollection::getInstance().linearTolerance();

	if (shapeType == TopAbs_WIRE)
	{
		std::vector<gp_Pnt> pts;
		for (BRepTools_WireExplorer expl(TopoDS::Wire(theFace)); expl.More(); expl.Next()) // pick midpoints to accomodate for curves
		{
			TopoDS_Edge currentEdge = TopoDS::Edge(expl.Current());
			Standard_Real sParam, lParam;
			Handle(Geom_Curve) currentCurve = BRep_Tool::Curve(currentEdge, sParam, lParam);
			if (currentCurve.IsNull()) continue;

			Standard_Real mParam = 0.5 * (sParam + lParam);
			pts.emplace_back(currentCurve->Value(mParam));
		}
		return newellsNormal(pts);

	}
	if (shapeType == TopAbs_FACE)
	{
		TopoDS_Face currentFace = TopoDS::Face(theFace);

		Standard_Real umin, umax, vmin, vmax;
		BRepTools::UVBounds(currentFace, umin, umax, vmin, vmax);
		Handle(Geom_Surface) aSurface = BRep_Tool::Surface(currentFace);
		GeomLProp_SLProps props(aSurface, umin, vmin, 1, precision);
		gp_Dir normal = props.Normal();
		gp_Vec normalVec = (gp_Vec(normal)).Normalized();
		if (currentFace.Orientation() != TopAbs_FORWARD)
		{
			normalVec.Reverse();
		}
		return normalVec;
	}
	return gp_Vec(0, 0, 0);
}

gp_Vec helperFunctions::newellsNormal(const std::vector<gp_Pnt>& pointList)
{
	gp_Vec normal(0, 0, 0);
	if (pointList.size() < 3) { normal; }

	for (size_t i = 0; i < pointList.size(); ++i) { // compute normal with newell's method
		const gp_Pnt& p0 = pointList[i];
		const gp_Pnt& p1 = pointList[(i + 1) % pointList.size()];
		normal.SetX(normal.X() + (p0.Y() - p1.Y()) * (p0.Z() + p1.Z()));
		normal.SetY(normal.Y() + (p0.Z() - p1.Z()) * (p0.X() + p1.X()));
		normal.SetZ(normal.Z() + (p0.X() - p1.X()) * (p0.Y() + p1.Y()));
	}

	if (normal.Magnitude() < SettingsCollection::getInstance().linearTolerance())
	{
		return normal;
	}

	normal.Normalized();
	return normal.Normalized();
}


double helperFunctions::computeSmallestAngle(const TopoDS_Face& theFace)
{
	double precision = SettingsCollection::getInstance().linearTolerance();

	std::vector<gp_Pnt> pointList = getUniquePoints(theFace);
	if (pointList.size() != 3) { std::cout << "smallest angle only works for triangles\n"; }

	gp_Vec v01(pointList[0], pointList[1]);
	gp_Vec v10 = v01.Reversed();
	gp_Vec v12(pointList[1], pointList[2]);
	gp_Vec v21 = v12.Reversed();
	gp_Vec v20(pointList[2], pointList[0]);
	gp_Vec v02 = v20.Reversed();

	double angle0 = v20.Angle(v10);
	double angle1 = v01.Angle(v21);
	double angle2 = v02.Angle(v12);

	return std::min({ angle0, angle1, angle2 });
}

gp_Vec helperFunctions::getShapedir(const std::vector<gp_Pnt>& pointList, bool isHorizontal)
{
	std::vector<std::pair<gp_Vec, int>> vecCountMap;
	double precision = SettingsCollection::getInstance().linearTolerance();

	// compute median lenght of all edges
	std::vector<double> distances;
	for (size_t i = 0; i < pointList.size(); i += 2)
	{
		gp_Pnt p1 = pointList[i];
		gp_Pnt p2 = pointList[i + 1];

		distances.emplace_back(p1.Distance(p2));
	}
	std::sort(distances.begin(), distances.end());

	double medianDistance;
	if (distances.size() % 2 != 0)
	{
		medianDistance = distances[distances.size() / 2];
	}
	else
	{
		medianDistance = distances[(distances.size() - 1) / 2 + (distances.size() + 1) / 2] / 2;
	}
	double minDistance = medianDistance * 0.05;

	while (true)
	{
		for (size_t i = 0; i < pointList.size(); i += 2)
		{
			gp_Pnt p1 = pointList[i];
			gp_Pnt p2 = pointList[i + 1];

			if (isHorizontal)
			{
				p1.SetZ(0);
				p2.SetZ(0);
			}

			double distance = p1.Distance(p2);
			if (distance < minDistance) { continue; }
			if (distance < precision) { continue; }
			gp_Vec vec = gp_Vec(p1, p2);

			if (!isHorizontal)
			{
				if (abs(vec.Z()) < 0.001) {
					continue;
				}
			}

			bool vFound = false;
			for (auto& vecPair : vecCountMap)
			{
				if (vecPair.first.IsParallel(vec, precision))
				{
					vecPair.second += 1;
					vFound = true;
					break;
				}
			}
			if (vFound) { continue; }
			vecCountMap.emplace_back(std::pair<gp_Vec, int>(vec, 1));
		}
		if (!vecCountMap.empty()) { break; }
		if (minDistance < precision) { return gp_Vec(); }
		minDistance = 0;
	}
	
	std::pair<gp_Vec, int> RotationVecPair = *vecCountMap.begin();
	for (auto& vecPair : vecCountMap)
	{
		if (RotationVecPair.second < vecPair.second)
		{
			RotationVecPair = vecPair;
		}
	}
	return RotationVecPair.first.Normalized();
}

bool helperFunctions::shareEdge(const TopoDS_Face& theFace, const TopoDS_Face& theotherFace)
{
	double precision = SettingsCollection::getInstance().linearTolerance();
	for (TopExp_Explorer currentExpl(theFace, TopAbs_EDGE); currentExpl.More(); currentExpl.Next())
	{
		TopoDS_Edge currentEdge = TopoDS::Edge(currentExpl.Current());
		for (TopExp_Explorer otherExpl(theotherFace, TopAbs_EDGE); otherExpl.More(); otherExpl.Next())
		{
			TopoDS_Edge otherEdge = TopoDS::Edge(otherExpl.Current());
			if (edgeEdgeOVerlapping(currentEdge, otherEdge)) { return true; }
		}
	}
	return false;
}

bool helperFunctions::edgeEdgeOVerlapping(const TopoDS_Edge& currentEdge, const TopoDS_Edge& otherEdge)
{
	gp_Pnt cP0 = getFirstPointShape(currentEdge);
	gp_Pnt cP1 = getLastPointShape(currentEdge);
	gp_Pnt oP0 = getFirstPointShape(otherEdge);
	gp_Pnt oP1 = getLastPointShape(otherEdge);

	double precision = SettingsCollection::getInstance().linearTolerance();

	// check if edges are parallel
	gp_Vec currentVec = gp_Vec(cP0, cP1);
	gp_Vec otherVec = gp_Vec(oP0, oP1);

	if (currentVec.Magnitude() < precision || otherVec.Magnitude() < precision) { return false; }

	if (!currentVec.IsParallel(otherVec, precision)) { return false; }

	// check if edges are identical
	if (cP0.IsEqual(oP0, precision) && cP1.IsEqual(oP1, precision) ||
		cP1.IsEqual(oP0, precision) && cP0.IsEqual(oP1, precision))
	{
		return true;
	}

	// if the distance between 3 points of the edges is the same as the full length of one edge. the edges are overlapping
	double currentFullDistance = cP0.Distance(cP1);
	if (abs(currentFullDistance - (cP0.Distance(oP0) + oP0.Distance(cP1))) < precision && cP0.Distance(oP0) > precision && oP0.Distance(cP1) > precision ||
		abs(currentFullDistance - (cP0.Distance(oP1) + oP1.Distance(cP1))) < precision && cP0.Distance(oP1) > precision && oP1.Distance(cP1) > precision)
	{
		return true;
	}
	double otherFullDistance = oP0.Distance(oP1);
	if (abs(otherFullDistance - (oP0.Distance(cP0) + cP0.Distance(oP1))) < precision && oP0.Distance(cP0) > precision && cP0.Distance(oP1) > precision  ||
		abs(otherFullDistance - (oP0.Distance(cP1) + cP1.Distance(oP1))) < precision && oP0.Distance(cP1) > precision && cP1.Distance(oP1) > precision)
	{
		return true;
	}
	return false;
}

bool helperFunctions::edgeEdgeAreSame(const TopoDS_Edge& currentEdge, const TopoDS_Edge& otherEdge)
{
	double precision = SettingsCollection::getInstance().linearTolerance();
	gp_Pnt currentP1 = helperFunctions::getFirstPointShape(currentEdge);
	gp_Pnt currentP2 = helperFunctions::getLastPointShape(currentEdge);
	gp_Pnt otherP1 = helperFunctions::getFirstPointShape(otherEdge);
	gp_Pnt otherP2 = helperFunctions::getLastPointShape(otherEdge);

	if (currentP1.IsEqual(otherP1, precision) && currentP2.IsEqual(otherP2, precision) ||
		currentP1.IsEqual(otherP2, precision) && currentP2.IsEqual(otherP1, precision))
	{
		return true;
	}
	return false;
}

bool helperFunctions::faceFaceOverlapping(const TopoDS_Face& upperFace, const TopoDS_Face& lowerFace)
{
	// compute area
	double setPresicion = SettingsCollection::getInstance().linearTolerance();
	if (abs(computeArea(upperFace) - computeArea(lowerFace)) > setPresicion) { return false; }

	// align verts
	for (TopExp_Explorer currentVertExpl(upperFace, TopAbs_VERTEX); currentVertExpl.More(); currentVertExpl.Next())
	{
		gp_Pnt currentPoint = BRep_Tool::Pnt(TopoDS::Vertex(currentVertExpl.Current()));

		bool vertFound = false;
		for (TopExp_Explorer otherVertExpl(lowerFace, TopAbs_VERTEX); otherVertExpl.More(); otherVertExpl.Next())
		{
			gp_Pnt otherPoint = BRep_Tool::Pnt(TopoDS::Vertex(otherVertExpl.Current()));

			double hDistance = sqrt(pow(currentPoint.X() - otherPoint.X(), 2) + pow(currentPoint.Y() - otherPoint.Y(), 2));

			if (hDistance > setPresicion) { continue; }
			if (currentPoint.Z() < otherPoint.Z()) { continue; }

			vertFound = true;
			break;
		}

		if (!vertFound)
		{
			return false;
		}
	}
	return true;
}

bool helperFunctions::surfaceIsIncapsulated(const TopoDS_Face& innerSurface, const TopoDS_Face& outerSurface)
{
	double precision = SettingsCollection::getInstance().linearTolerance();
	for (TopExp_Explorer explorer(innerSurface, TopAbs_VERTEX); explorer.More(); explorer.Next())
	{
		const TopoDS_Vertex& vertex = TopoDS::Vertex(explorer.Current());
		gp_Pnt currentPoint = BRep_Tool::Pnt(vertex);

		if (!pointOnFace(outerSurface, currentPoint) && !pointOnWire(outerSurface, currentPoint))
		{
			return false;
		}
	}

	std::vector<gp_Pnt> pointList = getPointListOnFace(innerSurface);
	for (const gp_Pnt& currentPoint : pointList)
	{
		if (!pointOnFace(outerSurface, currentPoint))
		{
			return false;
		}
	}
	return true;
}

bool helperFunctions::surfaceIsIncapsulated(const TopoDS_Face& innerSurface, const std::vector<TopoDS_Face>& outerSurfaceList, bool ignoreSelf)
{
	double precision = SettingsCollection::getInstance().linearTolerance();
	for (TopExp_Explorer explorer(innerSurface, TopAbs_VERTEX); explorer.More(); explorer.Next())
	{
		const TopoDS_Vertex& vertex = TopoDS::Vertex(explorer.Current());
		gp_Pnt currentPoint = BRep_Tool::Pnt(vertex);

		bool isOnFace = false;
		for (const TopoDS_Face& outerSurface : outerSurfaceList)
		{
			if (ignoreSelf)
			{
				if (outerSurface.IsEqual(innerSurface)) { continue; }
			}

			if (pointOnFace(outerSurface, currentPoint) || pointOnWire(outerSurface, currentPoint))
			{
				isOnFace = true;
				break;
			}
		}
		if (isOnFace)
		{
			continue;
		}
		return false;
	}

	std::vector<gp_Pnt> pointList = getPointGridOnSurface(innerSurface, SettingsCollection::getInstance().surfaceGridSize());
	std::vector<gp_Pnt> wireGridList = helperFunctions::getPointGridOnWire(innerSurface, SettingsCollection::getInstance().surfaceGridSize());
	pointList.insert(pointList.end(), wireGridList.begin(), wireGridList.end());

	for (const gp_Pnt& currentPoint : pointList)
	{
		bool isOnFace = false;
		for (const TopoDS_Face& outerSurface : outerSurfaceList)
		{
			if (ignoreSelf)
			{
				if (outerSurface.IsEqual(innerSurface)) { continue; }
			}

			if (pointOnFace(outerSurface, currentPoint))
			{
				isOnFace = true;
				break;
			}
		}
		if (isOnFace)
		{
			continue;
		}
		return false;
	}
	return true;
}


bool helperFunctions::triangleIntersecting(const std::array<gp_Pnt, 2>& line, const std::array<gp_Pnt, 3>& triangle)
{
	gp_Vec triangleNormal = gp_Vec(triangle[0], triangle[1]).Crossed(gp_Vec(triangle[0], triangle[2]));
	triangleNormal.Normalize();
	return triangleIntersecting(line, triangle, triangleNormal);
}

bool helperFunctions::triangleIntersecting(const std::array<gp_Pnt, 2>& line, const std::array<gp_Pnt, 3>& triangle, const gp_Vec& triangleNormal)
{
	double precision = SettingsCollection::getInstance().linearTolerance();
	if (triangleNormal.Magnitude() < precision) { return false; }


	const gp_Pnt& lineStart = line[0];
	const gp_Pnt& lineEnd = line[1];

	double distancePlaneLineStart = triangleNormal.Dot(gp_Vec(triangle[0], lineStart));
	double distancePlaneLineEnd = triangleNormal.Dot(gp_Vec(triangle[0], lineEnd));

	if (std::abs(distancePlaneLineStart) < precision &&
		std::abs(distancePlaneLineEnd) < precision)
	{
		return false;
	}
	if ((distancePlaneLineStart > 0 && distancePlaneLineEnd > 0) || (distancePlaneLineStart < 0 && distancePlaneLineEnd < 0))
	{
		return false;
	}

	double lineParameter = distancePlaneLineStart / (distancePlaneLineStart - distancePlaneLineEnd);
	if (lineParameter < -precision || lineParameter > 1.0 + precision)
		return false; // intersection outside segment

	gp_Pnt pIntersect = lineStart.XYZ() + lineParameter * gp_Vec(lineStart, lineEnd).XYZ();

	return baryCentricTest(pIntersect, triangle);
}

bool helperFunctions::baryCentricTest(const gp_Pnt& point, const std::array<gp_Pnt, 3>& triangle)
{
	double precision = SettingsCollection::getInstance().linearTolerance();

	gp_Vec v0(triangle[0], triangle[2]);
	gp_Vec v1(triangle[0], triangle[1]);
	gp_Vec v2(triangle[0], point);

	double dot00 = v0.Dot(v0);
	double dot01 = v0.Dot(v1);
	double dot02 = v0.Dot(v2);
	double dot11 = v1.Dot(v1);
	double dot12 = v1.Dot(v2);

	double denom = dot00 * dot11 - dot01 * dot01;
	if (std::abs(denom) < precision) { return false; }

	double invDenom = 1.0 / denom;
	double u = (dot11 * dot02 - dot01 * dot12) * invDenom;
	double v = (dot00 * dot12 - dot01 * dot02) * invDenom;
	double w = 1 - u - v;

	return (u >= -precision && v >= -precision &&  w >= -precision);
}


bool helperFunctions::LineShapeIntersection(const TopoDS_Face& theFace, const gp_Pnt& lP1, const gp_Pnt& lp2, bool inZdir)
{
	TopLoc_Location loc;
	auto mesh = BRep_Tool::Triangulation(theFace, loc);

	if (mesh.IsNull())
	{
		helperFunctions::triangulateShape(theFace);
		mesh = BRep_Tool::Triangulation(theFace, loc);
	}
	if (mesh.IsNull()) { return false; }

	for (int j = 1; j <= mesh.get()->NbTriangles(); j++) //TODO: if large num indx?
	{
		const Poly_Triangle& theTriangle = mesh->Triangles().Value(j);
		
		gp_Pnt p1 = mesh->Node(theTriangle(1)).Transformed(loc);
		gp_Pnt p2 = mesh->Node(theTriangle(2)).Transformed(loc);
		gp_Pnt p3 = mesh->Node(theTriangle(3)).Transformed(loc);
		
		if (helperFunctions::triangleIntersecting({ lP1, lp2 }, {p1, p2, p3}))
		{
			return true;
		}
	}
	return false;
}

TopoDS_Wire helperFunctions::mergeWireOrientated(const TopoDS_Wire& baseWire, const TopoDS_Wire& mergingWire) {
	double precision = SettingsCollection::getInstance().linearTolerance();
	
	gp_Pnt connectionPoint1 = getFirstPointShape(baseWire);
	gp_Pnt connectionPoint2 = getLastPointShape(baseWire);

	gp_Pnt p1 = getFirstPointShape(mergingWire);
	gp_Pnt p2 = getLastPointShape(mergingWire);

	BRepBuilderAPI_MakeWire wiremaker;

	if (connectionPoint1.Distance(p2) < precision) // correct orentation placed in front
	{

		for (TopExp_Explorer explorer(mergingWire, TopAbs_EDGE); explorer.More(); explorer.Next())
		{
			const TopoDS_Edge& edge = TopoDS::Edge(explorer.Current());
			wiremaker.Add(edge);
		}

		for (TopExp_Explorer explorer(baseWire, TopAbs_EDGE); explorer.More(); explorer.Next())
		{
			const TopoDS_Edge& edge = TopoDS::Edge(explorer.Current());
			wiremaker.Add(edge);
		}

		if (wiremaker.IsDone()) { return wiremaker.Wire(); }
	}
	if (connectionPoint1.Distance(p1) < precision) // wrong orentation placed in front
	{
		std::vector<TopoDS_Edge> tempEdgeList;
		for (TopExp_Explorer explorer(mergingWire, TopAbs_EDGE); explorer.More(); explorer.Next())
		{
			const TopoDS_Edge& edge = TopoDS::Edge(explorer.Current());
			gp_Pnt beginPoint = helperFunctions::getFirstPointShape(edge);
			gp_Pnt endPoint = helperFunctions::getLastPointShape(edge);
			tempEdgeList.emplace_back(BRepBuilderAPI_MakeEdge(endPoint, beginPoint));
		}

		for (std::vector<TopoDS_Edge>::reverse_iterator i = tempEdgeList.rbegin();
			i != tempEdgeList.rend(); ++i) {

			wiremaker.Add(*i);
		}

		for (TopExp_Explorer explorer(baseWire, TopAbs_EDGE); explorer.More(); explorer.Next())
		{
			const TopoDS_Edge& edge = TopoDS::Edge(explorer.Current());
			wiremaker.Add(edge);
		}

		if (wiremaker.IsDone()) { return wiremaker.Wire(); }
	}
	if (connectionPoint2.Distance(p1) < precision) // correct orentation placed after
	{
		for (TopExp_Explorer explorer(baseWire, TopAbs_EDGE); explorer.More(); explorer.Next())
		{
			const TopoDS_Edge& edge = TopoDS::Edge(explorer.Current());
			wiremaker.Add(edge);
		}

		for (TopExp_Explorer explorer(mergingWire, TopAbs_EDGE); explorer.More(); explorer.Next())
		{
			const TopoDS_Edge& edge = TopoDS::Edge(explorer.Current());
			wiremaker.Add(edge);
		}

		if (wiremaker.IsDone()) { return wiremaker.Wire(); }
	}
	if (connectionPoint2.Distance(p2) < precision) // wrong orentation placed after
	{

		for (TopExp_Explorer explorer(baseWire, TopAbs_EDGE); explorer.More(); explorer.Next())
		{
			const TopoDS_Edge& edge = TopoDS::Edge(explorer.Current());
			wiremaker.Add(edge);
		}

		std::vector<TopoDS_Edge> tempEdgeList;
		for (TopExp_Explorer explorer(mergingWire, TopAbs_EDGE); explorer.More(); explorer.Next())
		{
			const TopoDS_Edge& edge = TopoDS::Edge(explorer.Current());
			gp_Pnt beginPoint = helperFunctions::getFirstPointShape(edge);
			gp_Pnt endPoint = helperFunctions::getLastPointShape(edge);
			tempEdgeList.emplace_back(BRepBuilderAPI_MakeEdge(endPoint, beginPoint));
		}

		for (std::vector<TopoDS_Edge>::reverse_iterator i = tempEdgeList.rbegin();
			i != tempEdgeList.rend(); ++i) {

			wiremaker.Add(*i);
		}

		if (wiremaker.IsDone()) { return wiremaker.Wire(); }
	}
	return TopoDS_Wire();
}

std::vector<TopoDS_Face> helperFunctions::mergeFaces(const std::vector<TopoDS_Face>& theFaceList, bool communicate)
{
	if (theFaceList.size() == 1) { return theFaceList; }
	double precision = SettingsCollection::getInstance().linearTolerance();

	std::vector<gp_Vec> faceNormalList;
	std::vector<TopoDS_Face> faceCopyList;
	for (const TopoDS_Face surfacePair : theFaceList)
	{
		gp_Vec currentVec = helperFunctions::computeFaceNormal(surfacePair);
		if (currentVec.Magnitude() < precision) { continue; }
		faceNormalList.emplace_back(currentVec);
		faceCopyList.emplace_back(surfacePair);
	}

	std::vector<int> evalList(faceNormalList.size(), 0);
	std::vector<TopoDS_Face> cleanedFaceCollection;
	bool hasMergedFaces = false;

	if (communicate)
	{
		std::cout << "\tProcess surfaces - 0 of " << theFaceList.size() << "\r";
	}
	int currentCount = 0;

	for (size_t i = 0; i < faceNormalList.size(); i++)
	{
		if (evalList[i] == 1) { continue; }
		std::vector<TopoDS_Face> mergingPairList;

		const TopoDS_Face& currentFace = faceCopyList[i];
		const gp_Vec& currentNormal = faceNormalList[i];
		mergingPairList.emplace_back(currentFace);
		evalList[i] = 1;

		for (size_t j = 0; j < faceNormalList.size(); j++)
		{
			if (j == i) { continue; }
			if (evalList[j] == 1) { continue; }

			const gp_Vec& otherNormal = faceNormalList[j];

			if (!currentNormal.IsParallel(otherNormal, precision)) { continue; }
			evalList[j] = 1;
			mergingPairList.emplace_back(faceCopyList[j]);
		}

		if (mergingPairList.size() == 1)
		{
			cleanedFaceCollection.emplace_back(currentFace);
			if (communicate)
			{
				currentCount++;
				std::cout << "\tProcess surfaces - " << currentCount << " of " << theFaceList.size() << "\r";
			}
			continue;
		}

		std::vector<TopoDS_Face> mergedFaceList = planarFaces2Outline(mergingPairList);
		if (communicate)
		{
			currentCount += mergingPairList.size();
			std::cout << "\tProcess surfaces - " << currentCount << " of " << theFaceList.size() << "\r";
		}

		if (mergedFaceList.empty())
		{
			for (const TopoDS_Face& mergedFace : mergingPairList)
			{
				cleanedFaceCollection.emplace_back(mergedFace);
			}
			continue;
		}

		for (TopoDS_Face mergedFace : mergedFaceList)
		{
			if (fixFace(&mergedFace))
			{
				cleanedFaceCollection.emplace_back(mergedFace);
			}
		}
	}
	if (communicate)
	{
		std::cout << "\tProcess surfaces - " << currentCount << " of " << theFaceList.size() << "\n";
	}
	return cleanedFaceCollection;
}

TopoDS_Wire helperFunctions::closeWireOrientated(const TopoDS_Wire& baseWire) {
	gp_Pnt p1 = helperFunctions::getFirstPointShape(baseWire);
	gp_Pnt p2 = helperFunctions::getLastPointShape(baseWire);

	if (p1.Distance(p2) < SettingsCollection::getInstance().linearTolerance()) { return baseWire; }

	TopoDS_Wire closingWire = BRepBuilderAPI_MakeWire(BRepBuilderAPI_MakeEdge(p2, p1));

	return mergeWireOrientated(baseWire, closingWire);
}


TopoDS_Face helperFunctions::createHorizontalFace(const gp_Pnt& lll, const gp_Pnt& urr, double rotationAngle, double z) {
	gp_Pnt p0 = helperFunctions::rotatePointWorld(gp_Pnt(lll.X(), lll.Y(), z), rotationAngle);
	gp_Pnt p1 = helperFunctions::rotatePointWorld(gp_Pnt(urr.X(), lll.Y(), z), rotationAngle);
	gp_Pnt p2 = helperFunctions::rotatePointWorld(gp_Pnt(urr.X(), urr.Y(), z), rotationAngle);
	gp_Pnt p3 = helperFunctions::rotatePointWorld(gp_Pnt(lll.X(), urr.Y(), z), rotationAngle);

	return createPlanarFace(p0, p1, p2, p3);
}

TopoDS_Face helperFunctions::createPlanarFace(const gp_Pnt& p0, const gp_Pnt& p1, const gp_Pnt& p2, const gp_Pnt& p3) {

	double precision = SettingsCollection::getInstance().linearTolerance();
	if (p0.IsEqual(p1, precision)) { return {}; }
	if (p1.IsEqual(p2, precision)) { return {}; }
	if (p2.IsEqual(p3, precision)) { return {}; }
	if (p3.IsEqual(p0, precision)) { return {}; }

	TopoDS_Edge edge0 = BRepBuilderAPI_MakeEdge(p0, p1);
	TopoDS_Edge edge1 = BRepBuilderAPI_MakeEdge(p1, p2);
	TopoDS_Edge edge2 = BRepBuilderAPI_MakeEdge(p2, p3);
	TopoDS_Edge edge3 = BRepBuilderAPI_MakeEdge(p3, p0);

	return BRepBuilderAPI_MakeFace(BRepBuilderAPI_MakeWire(edge0, edge1, edge2, edge3));
}

TopoDS_Face helperFunctions::createPlanarFace(const gp_Pnt& p0, const gp_Pnt& p1, const gp_Pnt& p2)
{
	double precision = SettingsCollection::getInstance().linearTolerance();
	if (p0.IsEqual(p1, precision)) { return {}; }
	if (p1.IsEqual(p2, precision)) { return {}; }
	if (p2.IsEqual(p0, precision)) { return {}; }

	TopoDS_Edge edge0 = BRepBuilderAPI_MakeEdge(p0, p1);
	TopoDS_Edge edge1 = BRepBuilderAPI_MakeEdge(p1, p2);
	TopoDS_Edge edge2 = BRepBuilderAPI_MakeEdge(p2, p0);

	return BRepBuilderAPI_MakeFace(BRepBuilderAPI_MakeWire(edge0, edge1, edge2));
}

TopoDS_Face helperFunctions::projectFaceFlat(const TopoDS_Face& theFace, double height) {

	double precision = SettingsCollection::getInstance().linearTolerance();
	double angularTolerance = SettingsCollection::getInstance().angularTolerance();
	if (theFace.IsNull()) { return TopoDS_Face(); }
	// check if face is flat
	gp_Vec faceNormal = computeFaceNormal(theFace);
	if (abs(faceNormal.Z()) < angularTolerance) { return TopoDS_Face(); }

	gp_Vec flatFaceNormal(0,0,1);
	if (faceNormal.Z() < 0)
	{
		flatFaceNormal.SetZ(-1);
	}

	TopoDS_Face flatFace;
	if (abs(faceNormal.X()) < angularTolerance && abs(faceNormal.Y()) < angularTolerance)
	{
		gp_Trsf trsf;
		double faceHeight = getLowestZ(theFace);
		if (abs(height - faceHeight) < precision) { return theFace; }
		trsf.SetTranslationPart(gp_XYZ(0, 0, height - faceHeight));
		BRepBuilderAPI_Transform transformer(theFace, trsf);
		if (!transformer.IsDone()) {  return TopoDS_Face(); }
		flatFace = TopoDS::Face(transformer.Shape());
	}
	else
	{
		gp_Pnt p0 = getFirstPointShape(theFace);
		p0.SetZ(height);
		Handle(Geom_Plane) plane = new Geom_Plane(p0, flatFaceNormal);

		TopoDS_Wire outerWire = BRepTools::OuterWire(theFace);
		if (outerWire.IsNull()) { return TopoDS_Face(); }
		if (!outerWire.Closed()) { return TopoDS_Face(); }

		TopoDS_Wire flattenedWire = projectWireFlat(outerWire, height);
		if (flattenedWire.IsNull()) { return TopoDS_Face(); }
		if (faceNormal.Dot(computeFaceNormal(flattenedWire)) < 0) { flattenedWire.Reverse(); }

		BRepBuilderAPI_MakeFace faceMaker(plane, flattenedWire, precision);
		for (TopExp_Explorer expl(theFace, TopAbs_WIRE); expl.More(); expl.Next())
		{
			TopoDS_Wire currentWire = TopoDS::Wire(expl.Current());
			if (currentWire.IsEqual(outerWire)) { continue; }
			TopoDS_Wire currentFlatWire = projectWireFlat(currentWire, height);
			if (currentFlatWire.IsNull()) { continue; }
			if (!currentFlatWire.Closed()) { continue; }
			if (faceNormal.Dot(computeFaceNormal(currentFlatWire)) > 0) { currentFlatWire.Reverse(); }

			if (computeArea(currentFlatWire) < 0.001) { continue; }
			faceMaker.Add(currentFlatWire);
		}

		if (!faceMaker.IsDone()) { return TopoDS_Face(); }

		flatFace = faceMaker.Face();
	}
	fixFace(&flatFace);
	gp_Vec resultNormal = computeFaceNormal(flatFace);
	if (resultNormal.Dot(faceNormal) < 0) { flatFace.Reverse(); }
	return flatFace;
}

TopoDS_Face helperFunctions::projectFace(const TopoDS_Face& theFace, const gp_Pln& theReferencePlane)
{
	double precision = SettingsCollection::getInstance().linearTolerance();

	std::vector<TopoDS_Wire> wireList;
	for (TopExp_Explorer wireExplorer(theFace, TopAbs_WIRE); wireExplorer.More(); wireExplorer.Next())
	{
		TopoDS_Wire currentWire = TopoDS::Wire(wireExplorer.Current());

		int edgeCount = 0;
		BRepBuilderAPI_MakeWire wireMaker;
		for (BRepTools_WireExplorer expl(currentWire); expl.More(); expl.Next())
		{
			const TopoDS_Edge& edge = TopoDS::Edge(expl.Current());

			gp_Pnt p0;
			gp_Pnt p1;
			if (edge.Orientation() == TopAbs_FORWARD)
			{
				p0 = helperFunctions::getFirstPointShape(edge);
				p1 = helperFunctions::getLastPointShape(edge);
			}
			else
			{
				p0 = helperFunctions::getLastPointShape(edge);
				p1 = helperFunctions::getFirstPointShape(edge);
			}
			if (p0.IsEqual(p1, precision)) { continue; }

			gp_Lin line1(p0, gp_Dir(0, 0, 1));
			gp_Lin line2(p1, gp_Dir(0, 0, 1));

			// Intersect line with plane
			IntAna_IntConicQuad intersector1(line1, theReferencePlane, Precision::Confusion());
			IntAna_IntConicQuad intersector2(line2, theReferencePlane, Precision::Confusion());

			if (!intersector1.IsDone() || intersector1.NbPoints() == 0)
				continue;
			if (!intersector2.IsDone() || intersector2.NbPoints() == 0)
				continue;

			gp_Pnt projectedP0 = intersector1.Point(1);
			gp_Pnt projectedP1 = intersector2.Point(1);

			TopoDS_Edge topEdge = BRepBuilderAPI_MakeEdge(projectedP0, projectedP1);
			wireMaker.Add(topEdge);
		}
		if (!wireMaker.IsDone()) { continue; }
		wireList.emplace_back(wireMaker.Wire());
	}

	if (wireList.empty()) { return TopoDS_Face(); }
	if (wireList[0].IsNull()) { return TopoDS_Face(); }

	BRepBuilderAPI_MakeFace faceMaker(theReferencePlane, wireList[0], precision);
	for (size_t i = 1; i < wireList.size(); i++)
	{
		faceMaker.Add(wireList[i]);
	}
	if (!faceMaker.IsDone())
	{
		return TopoDS_Face();
	}

	TopoDS_Face angledSubFace = faceMaker.Face();
	helperFunctions::fixFace(&angledSubFace);

	return angledSubFace;
}

TopoDS_Wire helperFunctions::projectWireFlat(const TopoDS_Wire& theWire, double height)
{
	BRepBuilderAPI_MakeWire builder;
	double precision = SettingsCollection::getInstance().linearTolerance();
	for (BRepTools_WireExplorer expl(theWire); expl.More(); expl.Next()) {

		TopoDS_Edge edge = TopoDS::Edge(expl.Current());
		gp_Pnt p0 = helperFunctions::getFirstPointShape(edge);
		gp_Pnt p1 = helperFunctions::getLastPointShape(edge);

		p0.SetZ(height);
		p1.SetZ(height);

		if (p0.IsEqual(p1, precision))
		{
			continue;
		}

		BRepBuilderAPI_MakeEdge edgeMaker(p0, p1);
		if (!edgeMaker.IsDone())
		{
			continue;
		}
		builder.Add(edgeMaker.Edge());
	}

	if (!builder.IsDone())
	{
		return TopoDS_Wire();
	}

	TopoDS_Wire flattenedWire = builder.Wire();


	if (theWire.Closed() != flattenedWire.Closed())
	{
		return TopoDS_Wire();
	}

	gp_Vec inputNormal = computeFaceNormal(theWire);
	gp_Vec outputNormal = computeFaceNormal(flattenedWire);

	if (inputNormal.Dot(outputNormal) < 0)
	{
		flattenedWire = TopoDS::Wire(flattenedWire.Reversed());
	}

	return flattenedWire;
}

TopoDS_Shape helperFunctions::TesselateShape(const TopoDS_Shape& theShape)
{
	BRep_Builder compBuilder;
	TopoDS_Compound collection;
	compBuilder.MakeCompound(collection);

	if (isFlat(theShape) && isStraight(theShape))
	{
		return theShape;
	}

	for (TopExp_Explorer solidExpl(theShape, TopAbs_SOLID); solidExpl.More(); solidExpl.Next())
	{
		bool haNewFace = false;

		BRepBuilderAPI_Sewing brepSewer;
		TopoDS_Solid currentSolid = TopoDS::Solid(solidExpl.Current());

		for (TopExp_Explorer faceExpl(currentSolid, TopAbs_FACE); faceExpl.More(); faceExpl.Next())
		{
			TopoDS_Face currentFace = TopoDS::Face(faceExpl.Current());


			std::vector<TopoDS_Face> collapsedTriangles = TessellateFace(currentFace);

			if (collapsedTriangles.empty())
			{
				continue;
			}

			if (collapsedTriangles.size() > 1)
			{
				haNewFace = true;
			}
			else if (!collapsedTriangles[0].IsEqual(currentFace))
			{
				haNewFace = true;
			}

			for (const TopoDS_Face& cleanFace : collapsedTriangles)
			{
				brepSewer.Add(cleanFace);
			}			
		}

		if (!haNewFace)
		{
			compBuilder.Add(collection, currentSolid);
			continue;
		}

		brepSewer.Perform();

		BRep_Builder brepBuilder;
		TopoDS_Shell shell;
		brepBuilder.MakeShell(shell);
		TopoDS_Solid solidbox;
		brepBuilder.MakeSolid(solidbox);
		TopoDS_Shape sewedShape = brepSewer.SewedShape();

		if (sewedShape.IsNull())
		{
			continue;
		}

		if (sewedShape.Closed())
		{
			brepBuilder.Add(solidbox, sewedShape);
			compBuilder.Add(collection, solidbox);
			continue;
		}
	}
	if (collection.NbChildren())
	{
		return collection;
	}
	else
	{
		//TODO: add triangles
	}

	return {};

}

std::vector<TopoDS_Face> helperFunctions::TessellateFace(const TopoDS_Face& theFace, bool knownIsFlat)
{
	// check if needed to be tesselated
	bool isTesselated = true;
	bool isFlatFace = isFlat(theFace);

	if (isFlatFace && isStraight(theFace))
	{
		return { theFace };
	}


	//double precision = SettingsCollection::getInstance().linearTolerance();
	//if (isFlat(theFace) || knownIsFlat) //if flat only curves need to be replaced
	//{
	//	TopoDS_Wire outerWire = BRepTools::OuterWire(theFace);
	//	if (outerWire.IsNull()) { return {}; }
	//	if (!outerWire.Closed()) { return {}; }

	//	TopoDS_Wire cleanWire = replaceCurves(outerWire);

	//	gp_Pnt p0 = getFirstPointShape(cleanWire);
	//	Handle(Geom_Plane) plane = new Geom_Plane(p0, computeFaceNormal(theFace));
	//	BRepBuilderAPI_MakeFace faceMaker(plane, cleanWire, precision);

	//	if (!faceMaker.IsDone())
	//	{
	//		return {};
	//	}

	//	std::vector<TopoDS_Wire> wireList = {};
	//	for (TopExp_Explorer expl(theFace, TopAbs_WIRE); expl.More(); expl.Next())
	//	{
	//		TopoDS_Wire currentWire = TopoDS::Wire(expl.Current());
	//		if (currentWire.IsEqual(outerWire)) { continue; }
	//		
	//		TopoDS_Wire cleanWire = replaceCurves(currentWire);
	//		faceMaker.Add(cleanWire);

	//	}
	//	if (!faceMaker.IsDone())
	//	{
	//		return {};
	//	}

	//	TopoDS_Face tesselatedFace = faceMaker.Face();
	//	helperFunctions::fixFace(&tesselatedFace);
	//	return { tesselatedFace };
	//}

	// if not flat surface has to be meshed and the flat components have to be joined
	std::vector<TopoDS_Face> triangulatedFaces = TriangulateFace(theFace);
	return mergeFaces(triangulatedFaces);
}

bool helperFunctions::fixFace(TopoDS_Face* theFace)
{
	if (faceIsValid(*theFace))
	{
		return true;
	}

	ShapeFix_Face faceFixer(*theFace);
	if (wireCount(*theFace) > 1)
	{
		faceFixer.FixOrientation(); // fixes the innerwire invalid issue
	}
	faceFixer.FixIntersectingWires();
	faceFixer.SetPrecision(SettingsCollection::getInstance().linearTolerance());
	faceFixer.Perform();
	TopoDS_Face fixedFace = faceFixer.Face();

	if (!faceIsValid(fixedFace))
	{
		return false;
	}
	if (fixedFace.IsNull()) { return false; }

	*theFace = fixedFace;
	return true;
}

std::vector<TopoDS_Face> helperFunctions::TessellateFace(const std::vector<TopoDS_Face>& theFaceList, bool knownIsFlat)
{
	std::vector<TopoDS_Face> outputList;
	outputList.reserve(theFaceList.size());
	for (const TopoDS_Face& currentFace : theFaceList)
	{
		std::vector<TopoDS_Face> cleanedFaceList = TessellateFace(currentFace, knownIsFlat);

		if (cleanedFaceList.empty())
		{
			outputList.emplace_back(currentFace);
			continue;
		}

		for (const TopoDS_Face& cleanedFace : cleanedFaceList)
		{
			outputList.emplace_back(cleanedFace);
		}
	}
	return outputList;
}

std::vector<TopoDS_Face> helperFunctions::TriangulateFace(const std::vector<TopoDS_Face>& theFaceList)
{
	std::vector<TopoDS_Face> triangulatedFaces;
	for (const TopoDS_Face& currentFace : theFaceList)
	{
		std::vector<TopoDS_Face> triangles = TriangulateFace(currentFace);
		triangulatedFaces.insert(triangulatedFaces.end(), triangles.begin(), triangles.end());
	}
	return triangulatedFaces;
}

std::vector<TopoDS_Face> helperFunctions::TriangulateFace(const TopoDS_Face& theFace)
{
	TopLoc_Location loc;
	helperFunctions::triangulateShape(theFace);
	auto mesh = BRep_Tool::Triangulation(theFace, loc);
	if (mesh.IsNull()) { return {}; }

	double angularTol = SettingsCollection::getInstance().angularTolerance();
	double precision = SettingsCollection::getInstance().linearTolerance();
	gp_Vec currentNormal = helperFunctions::computeFaceNormal(theFace);

	std::vector<TopoDS_Face> triangleFaceList;
	for (int i = 1; i <= mesh->NbTriangles(); i++)
	{
		const Poly_Triangle& theTriangle = mesh->Triangles().Value(i);

		int i1, i2, i3;
		theTriangle.Get(i1, i2, i3);

		gp_Pnt p1 = mesh->Node(i1).Transformed(loc);
		gp_Pnt p2 = mesh->Node(i2).Transformed(loc);
		gp_Pnt p3 = mesh->Node(i3).Transformed(loc);
		gp_Vec otherNormal = helperFunctions::newellsNormal({ p1,p2,p3 });

		if (otherNormal.Magnitude() < precision) { continue; }

		TopoDS_Face triangleFace;
		if (currentNormal.IsOpposite(otherNormal, angularTol))
		{
			triangleFace = createPlanarFace(p3, p2, p1);
		}
		else
		{
			triangleFace = createPlanarFace(p1, p2, p3);
		}

		if (triangleFace.IsNull()) { continue; }
		triangleFaceList.emplace_back(triangleFace);
	}
	return triangleFaceList;
}


std::vector<TopoDS_Wire> helperFunctions::growWires(const std::vector<TopoDS_Edge>& edgeList) {
	std::vector<TopoDS_Wire> wireCollection;
	std::vector<TopoDS_Wire> wireCollectionClosed;
	std::vector<TopoDS_Edge> tempEdgeList;

	std::vector<TopoDS_Edge> cleanEdgelist;
	for (const TopoDS_Edge& currentEdge : replaceCurves(edgeList))
	{
		cleanEdgelist.emplace_back(currentEdge);
	}

	//BRepBuilderAPI_MakeWire wireMaker;
	bool loopFound = false;

	TopoDS_Edge currentEdge = cleanEdgelist[0];
	std::vector<int> evaluated(cleanEdgelist.size());
	evaluated[0] = 1;

	gp_Pnt originPoint = helperFunctions::getFirstPointShape(cleanEdgelist[0]); // the original point of the original edge
	gp_Pnt extendingPoint = helperFunctions::getLastPointShape(cleanEdgelist[0]); // the point from which will be extended

	tempEdgeList.emplace_back(currentEdge);

	double precision = SettingsCollection::getInstance().linearTolerance();
	bool isReversed = false;
	while (true)
	{
		bool hasStapped = false; // true if a stap is found in the while iteration
		bool closed = false; // true if the extensionpoint meets the originpoint

		for (size_t i = 0; i < cleanEdgelist.size(); i++)
		{
			if (evaluated[i] != 0) { continue; } // pass over evaluated edges
			TopoDS_Edge otherEdge = cleanEdgelist[i];

			gp_Pnt p1 = helperFunctions::getFirstPointShape(otherEdge);
			gp_Pnt p2 = helperFunctions::getLastPointShape(otherEdge);

			if (p1.IsEqual(extendingPoint, precision)) // check if edge is neighbour
			{
				extendingPoint = p2;
				evaluated[i] = 1;
				hasStapped = true;
				if (isReversed) { tempEdgeList.insert(tempEdgeList.begin(), BRepBuilderAPI_MakeEdge(p2, p1).Edge()); }
				else { tempEdgeList.emplace_back(otherEdge); }
				break;
			}
			else if (p2.IsEqual(extendingPoint, precision)) // check if reversed edge is neighbour 
			{
				extendingPoint = p1;
				evaluated[i] = 1;
				hasStapped = true;
				if (isReversed) { tempEdgeList.insert(tempEdgeList.begin(), otherEdge); }
				else { tempEdgeList.emplace_back(BRepBuilderAPI_MakeEdge(p2, p1).Edge()); }
				break;
			}
			else if (extendingPoint.IsEqual(originPoint, precision)) // check if a closed loop is found if no new neighbour is there
			{
				closed = true;
				break;
			}
		}

		if (hasStapped) { continue; } // if step is taken, try to make a next step

		if (!closed && !isReversed) // reverse the search and contine in the loop
		{
			gp_Pnt tempPoint = extendingPoint;
			extendingPoint = originPoint;
			originPoint = tempPoint;
			isReversed = true;
			continue;
		}

		BRepBuilderAPI_MakeWire wireMaker;
		for (size_t i = 0; i < tempEdgeList.size(); i++) { wireMaker.Add(tempEdgeList[i]); }
		tempEdgeList.clear();
		bool newRingStarted = false;
		wireMaker.Build();

		if (wireMaker.IsDone())
		{
			TopoDS_Wire wire = wireMaker.Wire();

			if (wire.Closed()) { wireCollectionClosed.emplace_back(wire); }
			else { wireCollection.emplace_back(wireMaker.Wire()); }
			wireMaker = BRepBuilderAPI_MakeWire();
		}


		for (size_t i = 0; i < cleanEdgelist.size(); i++) // search next unused edge to create new wire
		{
			if (evaluated[i] != 0) { continue; } // pass over evaluated edges

			originPoint = helperFunctions::getFirstPointShape(cleanEdgelist[i]); // the original point of the original edge
			extendingPoint = helperFunctions::getLastPointShape(cleanEdgelist[i]); // the point from which will be extended
			tempEdgeList.emplace_back(cleanEdgelist[i]);

			evaluated[i] = 1;
			isReversed = false;
			newRingStarted = true;
			break;
		}

		if (!newRingStarted)
		{
			break;
		}
	}

	if (wireCollection.size() != 0) {

		BRepBuilderAPI_MakeWire wireMaker = BRepBuilderAPI_MakeWire();
		TopoDS_Wire currentWire = wireCollection[0];
		wireCollection.erase(wireCollection.begin());

		double maxWireDistance = 1.5;

		int currentWireIdx = 0;
		while (true) // merge the openWires
		{
			bool stepped = false;

			double distance = 99999999999;
			int idxMatch = -1;
			TopoDS_Edge connectionEdge;

			gp_Pnt startpoint = helperFunctions::getFirstPointShape(currentWire);
			gp_Pnt endpoint = helperFunctions::getLastPointShape(currentWire);

			for (int i = 0; i < wireCollection.size(); i++)
			{
				TopoDS_Wire otherwire = wireCollection[i];
				gp_Pnt otherStartpoint = helperFunctions::getFirstPointShape(otherwire);
				gp_Pnt otherEndpoint = helperFunctions::getLastPointShape(otherwire);

				double d1 = startpoint.Distance(otherStartpoint);
				double d2 = startpoint.Distance(otherEndpoint);
				double d3 = endpoint.Distance(otherStartpoint);
				double d4 = endpoint.Distance(otherEndpoint);

				if (d1 < maxWireDistance && d1 < distance)
				{
					idxMatch = i;
					distance = d1;
					connectionEdge = BRepBuilderAPI_MakeEdge(startpoint, otherStartpoint);
				}
				if (d2 < maxWireDistance && d2 < distance)
				{
					idxMatch = i;
					distance = d2;
					connectionEdge = BRepBuilderAPI_MakeEdge(startpoint, otherEndpoint);
				}
				if (d3 < maxWireDistance && d3 < distance)
				{
					idxMatch = i;
					distance = d3;
					connectionEdge = BRepBuilderAPI_MakeEdge(endpoint, otherStartpoint);
				}
				if (d4 < maxWireDistance && d4 < distance)
				{
					idxMatch = i;
					distance = d4;
					connectionEdge = BRepBuilderAPI_MakeEdge(endpoint, otherEndpoint);
				}
			}

			if (idxMatch != -1)
			{
				currentWire = helperFunctions::mergeWireOrientated(currentWire, BRepBuilderAPI_MakeWire(connectionEdge));
				currentWire = helperFunctions::mergeWireOrientated(currentWire, wireCollection[idxMatch]);
				wireCollection.erase(wireCollection.begin() + idxMatch);
				stepped = true;
			}

			if (!stepped)
			{
				wireCollectionClosed.emplace_back(helperFunctions::closeWireOrientated(currentWire));
				if (wireCollection.size() == 0) { break; }

				currentWireIdx++;
				currentWire = wireCollection[0];
				wireCollection.erase(wireCollection.begin());
			}
		}
	}

	for (const TopoDS_Wire& currentWire : wireCollectionClosed)
	{
		
		 
	}

	



	return wireCollectionClosed;
}

std::vector<TopoDS_Wire> helperFunctions::cleanWires(const std::vector<TopoDS_Wire>& wireList) {

	std::vector<TopoDS_Wire> cleanedWires;

	for (size_t i = 0; i < wireList.size(); i++)
	{
		TopoDS_Wire cleanWire = helperFunctions::cleanWire(wireList[i]);
		if (cleanWire.IsNull()) { return {}; }
		cleanedWires.emplace_back(cleanWire);
	}
	return cleanedWires;
}

TopoDS_Wire helperFunctions::cleanWire(const TopoDS_Wire& wire) {
	TopTools_IndexedDataMapOfShapeListOfShape vertexToEdges;
	TopExp::MapShapesAndAncestors(wire, TopAbs_VERTEX, TopAbs_EDGE, vertexToEdges);

	if (!wire.Closed())
	{
		return wire;
	}

	std::vector<TopoDS_Edge> allEdges;
	for (TopExp_Explorer exp(wire, TopAbs_EDGE); exp.More(); exp.Next()) {
		TopoDS_Edge currentEdge = TopoDS::Edge(exp.Current());
		if (currentEdge.Orientation() == TopAbs_INTERNAL) { continue; }
		allEdges.emplace_back(currentEdge);
	}

	double precision = SettingsCollection::getInstance().linearTolerance();

	TopTools_MapOfShape visited;
	BRepBuilderAPI_MakeWire wireMaker;
	for (const TopoDS_Edge& currentEdge : allEdges)
	{
		if (visited.Contains(currentEdge)) { continue; }
		visited.Add(currentEdge);

		std::vector<TopoDS_Edge> colinGroup{ currentEdge };
		gp_Vec currentDir = helperFunctions::computeEdgeDir(currentEdge);

		if (currentDir.Magnitude() < precision) { continue; }

		// grow forwards
		TopoDS_Vertex firstVertex = TopExp::FirstVertex(currentEdge, true);

		while (true)
		{
			const TopTools_ListOfShape& adjEdges = vertexToEdges.FindFromKey(firstVertex);
			bool found = false;
			for (const TopoDS_Shape& potentialShape : adjEdges) {
				TopoDS_Edge potentialEdge = TopoDS::Edge(potentialShape);
				if (visited.Contains(potentialEdge)) { continue; }
				
				gp_Vec otherDir = helperFunctions::computeEdgeDir(potentialEdge);
				if (otherDir.Magnitude() < precision) 
				{ 
					visited.Add(potentialEdge);
					continue; 
				}

				if (!currentDir.IsParallel(otherDir, precision)) { continue; }
				colinGroup.insert(colinGroup.begin(), potentialEdge);
				visited.Add(potentialEdge);
				firstVertex = TopExp::FirstVertex(potentialEdge, true);
				found = true;
			}
			if (!found)
			{
				break;
			}
		}

		// grow backwards
		TopoDS_Vertex lastVertex = TopExp::LastVertex(currentEdge, true);
		while (true)
		{
			const TopTools_ListOfShape& adjEdges = vertexToEdges.FindFromKey(lastVertex);
			bool found = false;
			for (const TopoDS_Shape& potentialShape : adjEdges) {
				TopoDS_Edge potentialEdge = TopoDS::Edge(potentialShape);
				if (visited.Contains(potentialEdge)) { continue; }
				gp_Vec otherDir = helperFunctions::computeEdgeDir(potentialEdge);
				if (otherDir.Magnitude() < precision)
				{
					visited.Add(potentialEdge);
					continue;
				}

				if (!currentDir.IsParallel(otherDir, precision)) { continue; }
				colinGroup.emplace_back(potentialEdge);
				visited.Add(potentialEdge);
				lastVertex = TopExp::LastVertex(potentialEdge, true);
				found = true;
			}
			if (!found)
			{
				break;
			}
		}

		if (colinGroup.size() == 1)
		{
			wireMaker.Add(currentEdge);
			continue;
		}

		gp_Pnt p1 = BRep_Tool::Pnt(TopExp::FirstVertex(colinGroup.front(), true));
		gp_Pnt p2 = BRep_Tool::Pnt(TopExp::LastVertex(colinGroup.back(), true));

		if (p1.IsEqual(p2, precision)){continue;}
		TopoDS_Edge mergedEdge = BRepBuilderAPI_MakeEdge(p1,p2);	
		wireMaker.Add(mergedEdge);
	}

	if (!wireMaker.IsDone())
	{
		return wire;
	}
	TopoDS_Wire finalWire = wireMaker.Wire();
	if (finalWire.Closed())
	{
		return finalWire;
	}
	return wire;

}

TopoDS_Face helperFunctions::wireCluster2Faces(const std::vector<TopoDS_Wire>& wireList) {

	double precision = SettingsCollection::getInstance().linearTolerance();

	BRepBuilderAPI_MakeFace faceBuilder;
	std::vector<TopoDS_Face> faceList;
	gp_Vec normal = computeFaceNormal(wireList[0]);

	if (normal.Magnitude() < precision) { return TopoDS_Face(); }

	std::vector<gp_Pnt> pointList = getUniquePoints(wireList[0]);
	gp_Pnt originPoint = pointList[0];
	gp_Vec castingVector = gp_Vec(originPoint, pointList[1]);

	for (const TopoDS_Wire& currentWire : wireList)
	{
		if (helperFunctions::getUniquePoints(currentWire).size() < 3) { continue; }
		faceBuilder = BRepBuilderAPI_MakeFace(
			gp_Pln(originPoint, normal),
			currentWire
		);
		if (faceBuilder.Error() == BRepBuilderAPI_FaceDone) { faceList.emplace_back(faceBuilder.Face()); }
	}

	if (!faceList.size()) { return TopoDS_Face(); }
	if (faceList.size() == 1) { return faceList[0]; }

	// test which surfaces are inner loops
	std::vector<double> areaList;
	std::vector<TopoDS_Face> correctedFaceList;
	for (const TopoDS_Face& currentFace : faceList)
	{
		double area = computeArea(currentFace);
		if (area < 0.001) { continue; }
		areaList.emplace_back(area);
		correctedFaceList.emplace_back(currentFace);
	}

	std::vector<TopoDS_Face> orderedFootprintList;
	std::vector<double> orderedAreaList;
	std::vector<int> ordered(areaList.size());
	for (size_t i = 0; i < areaList.size(); i++)
	{
		double evalArea = 0;
		int evalIdx = -1;
		for (int j = 0; j < areaList.size(); j++)
		{
			if (ordered[j] == 1) { continue; }

			if (evalArea < areaList[j])
			{
				evalArea = areaList[j];
				evalIdx = j;
			}
		}

		orderedFootprintList.emplace_back(correctedFaceList[evalIdx]);
		orderedAreaList.emplace_back(areaList[evalIdx]);
		ordered[evalIdx] = 1;
	}

	std::vector<int> clipped(areaList.size());
	std::vector<TopoDS_Face> cleanedFaceList;

	TopoDS_Face clippedFace = orderedFootprintList[0];
	for (size_t i = 1; i < orderedFootprintList.size(); i++)
	{
		for (TopExp_Explorer expl(orderedFootprintList[i], TopAbs_WIRE); expl.More(); expl.Next())
		{
			TopoDS_Wire voidWire = TopoDS::Wire(expl.Current());
			voidWire = TopoDS::Wire(expl.Current().Reversed()); 
			try
			{
				BRepBuilderAPI_MakeFace merger = BRepBuilderAPI_MakeFace(clippedFace, voidWire);
				clippedFace = merger.Face();
			}
			catch (const std::exception&)
			{
				continue;
			}	
			break;
		}
	}
	return clippedFace;
}

std::vector<TopoDS_Face> helperFunctions::TrimFaceToFace(const TopoDS_Face& argumentFace, const std::vector<TopoDS_Face>& toolFaceList, double faceHeight)
{
	std::vector<TopoDS_Face> outList;
	
	double precision = SettingsCollection::getInstance().linearTolerance();
	TopTools_ListOfShape toolList;
	for (const TopoDS_Face& otherStoryFace : toolFaceList)
	{
		if (otherStoryFace.IsNull()) { continue; }
		TopoDS_Face flattenedFace = helperFunctions::projectFaceFlat(otherStoryFace, faceHeight);
		toolList.Append(flattenedFace);
	}

	TopTools_ListOfShape argumentList;
	argumentList.Append(argumentFace);

	BRepAlgoAPI_Splitter splitter;
	splitter.SetFuzzyValue(precision);
	splitter.SetArguments(argumentList);
	splitter.SetTools(toolList);
	splitter.Build();

	for (TopExp_Explorer explorer(splitter.Shape(), TopAbs_FACE); explorer.More(); explorer.Next())
	{
		const TopoDS_Face& currentFace = TopoDS::Face(explorer.Current());

		std::optional<gp_Pnt> optionalPoint = helperFunctions::getPointOnFace(currentFace);
		if (optionalPoint == std::nullopt) { continue; }

		for (const TopoDS_Shape& otherStoryFace : toolList)
		{
			if (!helperFunctions::pointOnShape(otherStoryFace, *optionalPoint)) { continue; }
			outList.emplace_back(currentFace);
			break;
		}
	}
	return outList;
}

std::vector<TopoDS_Face> helperFunctions::planarFaces2Outline(const std::vector<TopoDS_Face>& planarFaces)
{
	if (planarFaces.empty()) { return {}; }
	if (planarFaces.size() == 1) { return planarFaces; }

	gp_Trsf transform;
	double precision = SettingsCollection::getInstance().linearTolerance();
	double angularTol = SettingsCollection::getInstance().angularTolerance();

	// rotate the cluster so that all lie parallel to the xy plane
	gp_Vec clusterNormal = computeFaceNormal(planarFaces[0]);
	gp_Vec horizontalNormal = gp_Vec(0, 0, 1);

	std::vector<TopoDS_Face> flattenedFaceList;

	if (!clusterNormal.IsParallel(horizontalNormal, precision))
	{
		std::optional<gp_Pnt> optionalbasePoint = helperFunctions::getPointOnFace(planarFaces[0]);
		if (optionalbasePoint == std::nullopt) { return {}; }

		gp_Vec normalCrossProduct = clusterNormal ^ horizontalNormal;
		gp_Ax1 rotationAxis(*optionalbasePoint, normalCrossProduct);
		Standard_Real rotationAngle = clusterNormal.AngleWithRef(horizontalNormal, rotationAxis.Direction());

		transform.SetRotation(rotationAxis, rotationAngle);

		for (const TopoDS_Face& face : planarFaces) {
			BRepBuilderAPI_Transform transformer(face, transform);
			if (transformer.IsDone()) {
				TopoDS_Face dubface = TopoDS::Face(transformer.Shape());
				if (dubface.IsNull()) { continue; }
				flattenedFaceList.emplace_back(dubface);
			}
		}
	}
	else
	{
		flattenedFaceList = planarFaces;
	}

	// TODO: find a way to collapse mesehs instead of boolean
	std::vector<HalfEdge> edgeCluster = planarFaces2EdgeCluster(flattenedFaceList);
	std::vector<HalfEdgeLoop> loopList = planarEdgeCluster2Loops(edgeCluster);
	std::vector<HalfEdgeLoop> outerLoopList = loops2Outer(loopList, flattenedFaceList);

	//TODO: remove not required vertex
	std::vector<TopoDS_Face> clippedFaceList = outerLoops2Faces(outerLoopList);

	// return the surfaces to the input orientation
	std::vector<TopoDS_Face> orientedFaces;
	orientedFaces.reserve(clippedFaceList.size());

	transform.Invert();
	if (!clusterNormal.IsParallel(horizontalNormal, precision))
	{
		for (const TopoDS_Face& outlinedFace : clippedFaceList)
		{
			BRepBuilderAPI_Transform transformer(outlinedFace, transform);
			orientedFaces.emplace_back(TopoDS::Face(transformer.Shape()));
		}
	}
	else
	{
		orientedFaces = clippedFaceList;
	}
	return orientedFaces;
}

std::vector<TopoDS_Shape> helperFunctions::planarFaces2Cluster(const std::vector<TopoDS_Face>& planarFaces)
{
	double precisionCoarse = SettingsCollection::getInstance().linearTolerance(); //TODO: this has been uncoarsened 

	std::vector<TopoDS_Shape> clusteredShapeList;
	FaceComplex faceComplex;
	faceComplex.faceList_ = planarFaces;
	std::vector<FaceComplex> faceComplexList = { faceComplex };

	for (const FaceComplex& faceComplex : faceComplexList)
	{
		// merge the faces
		BRepAlgoAPI_Fuse fuser;
		TopTools_ListOfShape mergeList;
		fuser.SetFuzzyValue(precisionCoarse);
		for (const TopoDS_Face splitFace : faceComplex.faceList_)
		{
			mergeList.Append(splitFace);
		}
		fuser.SetArguments(mergeList);
		fuser.SetTools(mergeList);
		fuser.Build();

		if (!fuser.IsDone())
		{
			//TODO: add error
			continue;
		}
		TopoDS_Shape fusedShape = fuser.Shape();

		if (!fusedShape.IsNull())
		{
			clusteredShapeList.emplace_back(fusedShape);
			continue;
		}
	}
	return clusteredShapeList;
}

std::vector<HalfEdge> helperFunctions::planarFaces2EdgeCluster(const std::vector<TopoDS_Face>& planarFaces)
{
	// split all edges with eachother
	double precision = SettingsCollection::getInstance().linearTolerance();
	bgi::rtree<std::pair<BoostBox3D, HalfEdge>, bgi::rstar<25>> edgeIndex = makeEdgeClusterIndx(planarFaces);
	bgi::rtree<std::pair<BoostBox3D, HalfEdge>, bgi::rstar<25>> splitEdgeIndex;
	std::vector<HalfEdge> uniqueSplitEdges;

	std::vector<HalfEdge> toolList;
	for (const std::pair<BoostBox3D, HalfEdge>& currentPair : edgeIndex)
	{
		const HalfEdge& currentEdge = currentPair.second;
		toolList.clear();

		std::vector<std::pair<BoostBox3D, HalfEdge>> qResult;
		qResult.clear();
		edgeIndex.query(bgi::intersects(
			currentPair.first), std::back_inserter(qResult));

		const gp_Pnt& currentP1 = currentEdge.p1_;
		const gp_Pnt& currentP2 = currentEdge.p2_;
		gp_Vec currentVec = gp_Vec(currentP1, currentP2);

		if (qResult.empty()) { continue; }
		for (const std::pair<BoostBox3D, HalfEdge>& otherPair : qResult)
		{
			const HalfEdge& otherEdge = otherPair.second;

			const gp_Pnt& otherP1 = otherEdge.p1_;
			const gp_Pnt& otherP2 = otherEdge.p2_;

			if (currentP1.IsEqual(otherP1, precision) || currentP1.IsEqual(otherP2, precision) ||
				currentP2.IsEqual(otherP1, precision) || currentP2.IsEqual(otherP2, precision))
			{
				continue;
			}

			if (gp_Vec(otherP1, otherP2).IsParallel(currentVec, precision))
			{
				continue;
			}

			toolList.emplace_back(otherEdge);
		}

		std::vector<HalfEdge> SplitEdgeList;
		if (toolList.empty())
		{
			SplitEdgeList = { currentEdge };
		}
		else
		{
			SplitEdgeList =  splitHalfEdge(currentEdge, toolList);
		}

		for (const HalfEdge& currentTrimmedEdge: SplitEdgeList)
		{
			if (currentTrimmedEdge.p1_.IsEqual(currentTrimmedEdge.p2_, precision))
			{
				continue;
			}

			BoostBox3D splitBox = createBBox(currentTrimmedEdge.p1_, currentTrimmedEdge.p2_);

			std::vector<std::pair<BoostBox3D, HalfEdge>> qSplitResult;
			qSplitResult.clear();
			splitEdgeIndex.query(bgi::intersects(
				splitBox), std::back_inserter(qSplitResult));

			bool isUnique = true;
			for (const std::pair<BoostBox3D, HalfEdge>& otherSplitPair : qSplitResult)
			{
				const HalfEdge& otherTrimmedEdge = otherSplitPair.second;


				if (currentTrimmedEdge.p1_.IsEqual(otherTrimmedEdge.p1_, precision) && currentTrimmedEdge.p2_.IsEqual(otherTrimmedEdge.p2_, precision) ||
					currentTrimmedEdge.p1_.IsEqual(otherTrimmedEdge.p2_, precision) && currentTrimmedEdge.p2_.IsEqual(otherTrimmedEdge.p1_, precision))
				{
					isUnique = false;
					break;
				}
			}
			if (!isUnique) { continue; }
			splitEdgeIndex.insert(std::make_pair(splitBox, currentTrimmedEdge));
			uniqueSplitEdges.emplace_back(currentTrimmedEdge);
		}
	}
	return uniqueSplitEdges;
}

bgi::rtree<std::pair<BoostBox3D, HalfEdge>, bgi::rstar<25>> helperFunctions::makeEdgeClusterIndx(const std::vector<TopoDS_Face>& planarFaces)
{
	// remove triangle dubs and index
	double precision = SettingsCollection::getInstance().linearTolerance();
	bgi::rtree<std::pair<BoostBox3D, HalfEdge>, bgi::rstar<25>> triangleEdgeIndex;
	bgi::rtree<std::pair<BoostBox3D, HalfEdge>, bgi::rstar<25>> edgeIndexClean;

	for (const TopoDS_Face& currentEdge : planarFaces)
	{
		gp_Pnt p1 = getFirstPointShape(currentEdge);
		gp_Pnt p2 = getLastPointShape(currentEdge);

		if (p1.IsEqual(p2, precision)) { continue; }

		HalfEdge halfEdge = HalfEdge(p1, p2);
	}

	for (const TopoDS_Face currentFace : planarFaces)
	{
		if (getPointCount(currentFace) != 3)
		{
			for (TopExp_Explorer edgeExpl(currentFace, TopAbs_EDGE); edgeExpl.More(); edgeExpl.Next()) {
				TopoDS_Edge currentEdge = TopoDS::Edge(edgeExpl.Current());

				if (!isStraight(currentEdge))
				{
					TopoDS_Wire compoundWire = CurveToCompound(currentEdge);

					for (TopExp_Explorer exp(compoundWire, TopAbs_EDGE); exp.More(); exp.Next()) {
						const TopoDS_Edge& compundEdge = TopoDS::Edge(exp.Current());
						HalfEdge halfEdge(compundEdge);

						edgeIndexClean.insert(std::make_pair(createBBox(compundEdge), halfEdge));
					}
					continue;
				}
				HalfEdge halfEdge(currentEdge);
				edgeIndexClean.insert(std::make_pair(createBBox(currentEdge), halfEdge));
			}
			continue;
		}

		gp_Vec currentVec = computeFaceNormal(currentFace);
		bool isForwards = currentVec.Z() > 0;

		for (TopExp_Explorer wireExpl(currentFace, TopAbs_WIRE); wireExpl.More(); wireExpl.Next()) {
			const TopoDS_Wire& currentWire = TopoDS::Wire(wireExpl.Current());

			for (BRepTools_WireExplorer wireExpl(currentWire, currentFace); wireExpl.More(); wireExpl.Next()) {
				TopoDS_Edge currentEdge = TopoDS::Edge(wireExpl.Current());
				if ((currentEdge.Orientation() == TopAbs_REVERSED) ^ isForwards)
				{
					currentEdge.Reverse();
				}

				if (!isStraight(currentEdge))
				{
					TopoDS_Wire compoundWire = CurveToCompound(currentEdge);

					for (TopExp_Explorer exp(compoundWire, TopAbs_EDGE); exp.More(); exp.Next()) {
						const TopoDS_Edge& compundEdge = TopoDS::Edge(exp.Current());
						HalfEdge halfEdge(compundEdge);
						triangleEdgeIndex.insert(std::make_pair(createBBox(compundEdge), halfEdge));
					}
					continue;
				}
				HalfEdge halfEdge(currentEdge);
				triangleEdgeIndex.insert(std::make_pair(createBBox(currentEdge), halfEdge));
			}
		}
	}

	for (const std::pair<BoostBox3D, HalfEdge> currentPair : triangleEdgeIndex)
	{
		std::vector<std::pair<BoostBox3D, HalfEdge>> qResult;
		qResult.clear();
		triangleEdgeIndex.query(bgi::intersects(
			currentPair.first), std::back_inserter(qResult));

		const HalfEdge& currentEdge = currentPair.second;
		const gp_Pnt& currentP1 = currentEdge.p1_;
		const gp_Pnt& currentP2 = currentEdge.p2_;

		bool isUnique = true;
		for (const std::pair<BoostBox3D, HalfEdge>& otherPair : qResult)
		{
			const HalfEdge& otherEdge = otherPair.second;

			const gp_Pnt& otherP1 = otherEdge.p1_;
			const gp_Pnt& otherP2 = otherEdge.p2_;

			if (currentP1.IsEqual(otherP1, precision) && currentP2.IsEqual(otherP2, precision))
			{
				continue;
			}

			if (currentP1.IsEqual(otherP2, precision) && currentP2.IsEqual(otherP1, precision))
			{
				isUnique = false;
				break;
			}
		}
		if (!isUnique) { continue; }
		edgeIndexClean.insert(currentPair);
	}
	return edgeIndexClean;
}

std::vector<HalfEdgeLoop> helperFunctions::planarEdgeCluster2Loops(const std::vector<HalfEdge>& planarEdgeCluster)
{
	double precision = SettingsCollection::getInstance().linearTolerance();

	std::vector<HalfEdge> halfEdgeList;
	for (const HalfEdge& currentEdge : planarEdgeCluster)
	{
		gp_Pnt p1 = currentEdge.p1_;
		gp_Pnt p2 = currentEdge.p2_;

		if (p1.IsEqual(p2, precision)) { continue; }

		HalfEdge mirroredEdge = HalfEdge(p2, p1);
		halfEdgeList.emplace_back(currentEdge);
		halfEdgeList.emplace_back(mirroredEdge);
	}

	// grow loop
	HalfEdge* startEdge = &halfEdgeList[0];
	std::vector<HalfEdge> loopEdgeList;
	std::vector<HalfEdgeLoop> loopList;
	while (true)
	{
		loopEdgeList.emplace_back(*startEdge);
		startEdge->isUsed_ = true;

		const gp_Pnt endPoint = startEdge->p2_;
		gp_Vec baseDir = startEdge->getDir();

		HalfEdge* bestEdge = nullptr;
		double bestAngle = 1e30;

		for (HalfEdge& otherHalfEdge : halfEdgeList)
		{
			if (!otherHalfEdge.p1_.IsEqual(endPoint, precision)) { continue; }
			if (startEdge->isPartner(otherHalfEdge)) { continue; }

			gp_Vec dir = otherHalfEdge.getDir();
			double crossZ = baseDir.Crossed(dir).Z();
			double dot = baseDir.Dot(dir);
			double angle = std::atan2(crossZ, dot);

			if (!bestEdge || angle < bestAngle)
			{
				bestEdge = &otherHalfEdge;
				bestAngle = angle;
			}
		}

		if (bestEdge == nullptr) 
		{
			for (HalfEdge& currentHalfedge : halfEdgeList)
			{
				if (currentHalfedge.isUsed_) { continue; }
				startEdge = &currentHalfedge;
				break;
			}
			continue; 
		}

		if (bestEdge->isUsed_)
		{
			if (!loopEdgeList.empty() && *bestEdge == loopEdgeList[0])
			{
				std::vector<HalfEdge> cleanedLoopEdgeList = cleanHalfEdgeList(loopEdgeList);
				HalfEdgeLoop currentLoop = HalfEdgeLoop(cleanedLoopEdgeList);
				loopList.emplace_back(currentLoop);
			}
			loopEdgeList.clear();
			for (HalfEdge& currentHalfedge : halfEdgeList)
			{
				if (currentHalfedge.isUsed_) { continue; }
				startEdge = &currentHalfedge;
				break;
			}

			if (startEdge->isUsed_)
			{
				break;
			}
			continue;
		}
		startEdge = bestEdge;
	}

	return loopList;
}

std::vector<HalfEdgeLoop> helperFunctions::loops2Outer(const std::vector<HalfEdgeLoop>& planarLoopList, const std::vector<TopoDS_Face>& planarFaces)
{
	std::vector<HalfEdgeLoop> loopLists;
	SettingsCollection& settingCol = SettingsCollection::getInstance();
	double precision = settingCol.linearTolerance();
	double pointOffset = precision * 100;

	std::vector<TopoDS_Face> triangulatedSourceList = TriangulateFace(planarFaces);
 	bgi::rtree<std::pair<BoostBox3D, int>, bgi::rstar<25>> triangulatedShapeIndx;
	for (const TopoDS_Face& currentFace : triangulatedSourceList)
	{
		bg::model::box <BoostPoint3D> bbox = helperFunctions::createBBox(currentFace);
		triangulatedShapeIndx.insert(std::make_pair(bbox, triangulatedShapeIndx.size()));
	}

	for (const HalfEdgeLoop& currentLoop : planarLoopList)
	{
		int totalCount = currentLoop.halfEdgeList_.size() / 2;
		int innerCount = 0;
		bool isInner = false;

		for (const HalfEdge& currentHalfEdge : currentLoop.halfEdgeList_)
		{
			gp_Vec edgeDir = currentHalfEdge.getDir();
			gp_Vec perpDir = gp_Vec(edgeDir.Y(), -edgeDir.X(), 0) * pointOffset;

			const gp_Pnt& p1 = currentHalfEdge.p1_;
			const gp_Pnt& p2 = currentHalfEdge.p2_;
			gp_Pnt middlePoint(
				(p1.X() + p2.X()) / 2,
				(p1.Y() + p2.Y()) / 2,
				(p1.Z())
			);

			gp_Pnt evalP1 = middlePoint.Translated(perpDir);

			double currentZ = p1.Z();
			std::vector<std::pair<BoostBox3D, int>> qResult;
			qResult.clear();
			triangulatedShapeIndx.query(bgi::intersects(
				helperFunctions::createBBox(
					gp_Pnt(evalP1.X() - 0.1, evalP1.Y() - 0.1, evalP1.Z() - 0.1),
					gp_Pnt(evalP1.X() + 0.1, evalP1.Y() + 0.1, evalP1.Z() + 0.1)
				)), std::back_inserter(qResult));

			for (const std::pair<BoostBox3D, int>& resultPair : qResult)
			{
				if (isInner) { break; }
				const TopoDS_Face& sourceFace = triangulatedSourceList[resultPair.second];

				double otherZ = getAZ(sourceFace);
				if (abs(otherZ - currentZ) > precision) { continue; }

				std::vector<gp_Pnt> trianglePoints = getPoints(sourceFace);

				if (pointOnTriangle(
					gp_Pnt2d(evalP1.X(), evalP1.Y()),
					gp_Pnt2d(trianglePoints[0].X(), trianglePoints[0].Y()), 
					gp_Pnt2d(trianglePoints[2].X(), trianglePoints[2].Y()), 
					gp_Pnt2d(trianglePoints[4].X(), trianglePoints[4].Y()))
					)
				{
					innerCount++;
					if (innerCount>= totalCount) { isInner = true; }
					break;
				}
			}
			if (isInner) { break; }
		}
		if (isInner) { continue; }
		loopLists.emplace_back(currentLoop);
	}
	return loopLists;
}

std::vector<TopoDS_Face> helperFunctions::outerLoops2Faces(const std::vector<HalfEdgeLoop>& outerLoopList)
{
	std::vector<TopoDS_Wire> wireList;
	for (const HalfEdgeLoop& currentLoop : outerLoopList)
	{
		BRepBuilderAPI_MakeWire builder;
		for (const HalfEdge& currentEdge : currentLoop.halfEdgeList_)
		{
			TopoDS_Edge segment = BRepBuilderAPI_MakeEdge(currentEdge.p1_, currentEdge.p2_);
			builder.Add(segment);
		}
		builder.Build();
		if (builder.IsDone())
		{
			wireList.emplace_back(builder.Wire());
		}
	}

	if (wireList.empty())
	{
		return {};
	}

	if (wireList.size() == 1)
	{
		TopoDS_Wire& currentWire = wireList[0];
		gp_Vec currentVec = computeFaceNormal(currentWire);

		BRepBuilderAPI_MakeFace faceBuilder = BRepBuilderAPI_MakeFace(
			gp_Pln(getFirstPointShape(currentWire), currentVec),
			currentWire
		);
		if (faceBuilder.Error() == BRepBuilderAPI_FaceDone)
		{
			return { faceBuilder.Face() };
		}
	}

	// find inner and outer wires
	std::vector<TopoDS_Wire> outerWires;
	std::vector<TopoDS_Wire> innerWires;
	for (const TopoDS_Wire& currentWire : wireList)
	{
		gp_Vec currentVec = computeFaceNormal(currentWire);

		if (currentVec.Z() > 0)
		{
			outerWires.emplace_back(currentWire);
			continue;
		}
		innerWires.emplace_back(currentWire);
	}

	//get baseplane
	if (outerWires.empty())
	{
		//std::cout << "hit" << std::endl;
		return {};
	}

	// make the outer wires into faces
	std::vector<TopoDS_Face> faceList;
	std::vector<double> areaList;
	for (const TopoDS_Wire& currentWire : outerWires)
	{
		gp_Pnt p0 = getFirstPointShape(currentWire);
		gp_Pln basePlane = gp_Pln(p0, gp_Vec(0, 0, 1));
		TopoDS_Face currentFace = BRepBuilderAPI_MakeFace(basePlane, currentWire);

		BRepCheck_Analyzer check(currentFace);
		if (!check.IsValid()) {
			//TODO: add error
		}
		if (currentFace.IsNull()) { continue; }
		faceList.emplace_back(currentFace);
		areaList.emplace_back(computeArea(currentFace));
	}

	// sort facelist in such a way that the outer faces that are the smallest are evaluated first to prevent innerwires being mismatched
	faceList = helperFunctions::sortShapes(faceList, areaList);
	std::reverse(faceList.begin(), faceList.end());

	std::vector<TopoDS_Face> clippedFaceList;
	std::vector<int> innerIsUsed(innerWires.size());
	for (const TopoDS_Face& currentFace : faceList)
	{
		TopoDS_Face clippedFace = currentFace;
		for (int i = 0; i < innerWires.size(); i++)
		{
			if (innerIsUsed[i] != 0) { continue; }
			const TopoDS_Wire& currentWire = innerWires[i];
			gp_Pnt wirePoint = getFirstPointShape(currentWire);
			if (!pointOnFace(currentFace, wirePoint)) { continue; }

			BRepBuilderAPI_MakeFace merger = BRepBuilderAPI_MakeFace(clippedFace, currentWire);
			if (!merger.IsDone()) { continue; }

			TopoDS_Face localClipperFace = merger.Face();
			if (localClipperFace.IsNull()) {continue; }

			BRepCheck_Analyzer check(localClipperFace);
			if (!check.IsValid()) { continue;  }

			clippedFace = localClipperFace;
			innerIsUsed[i] = 1;
		}
		clippedFaceList.emplace_back(clippedFace);
	}
	return clippedFaceList;
}

std::vector<HalfEdge> helperFunctions::splitHalfEdge(const HalfEdge& argument, const std::vector<HalfEdge>& toolList)
{
	double precision = SettingsCollection::getInstance().linearTolerance();

	const gp_Pnt& startPoint = argument.p1_;
	const gp_Pnt& endPoint = argument.p2_;

	std::vector<double> tList;

	for (const HalfEdge& currentTool : toolList)
	{
		double t;
		if (!splitHalfEdge(argument, currentTool, precision, t))
		{
			continue;
		}
		tList.emplace_back(t);
	}

	std::sort(tList.begin(), tList.end());

	tList.erase(
		std::unique(
			tList.begin(),
			tList.end(),
			[&precision](double a, double b)
			{
				return std::abs(a - b) < precision;
			}),
		tList.end());

	std::vector<HalfEdge> outputList;

	gp_Pnt basePoint = startPoint;
	for (const double& t : tList)
	{
		gp_Vec dir(argument.p1_, argument.p2_);
		gp_Pnt intersection = argument.p1_.Translated(dir * t);

		outputList.emplace_back(HalfEdge(basePoint, intersection));
		basePoint = intersection;
	}
	outputList.emplace_back(HalfEdge(basePoint, endPoint));
	return outputList;
}

bool helperFunctions::splitHalfEdge(const HalfEdge& argument, const HalfEdge& tool, double precision, double& t)
{
	gp_XY p1(argument.p1_.X(), argument.p1_.Y());
	gp_XY p2(argument.p2_.X(), argument.p2_.Y());

	gp_XY p3(tool.p1_.X(), tool.p1_.Y());
	gp_XY p4(tool.p2_.X(), tool.p2_.Y());

	gp_XY r = p2 - p1;
	gp_XY s = p4 - p3;

	double denom = r.Crossed(s);

	if (std::abs(denom) < precision)
		return false;

	gp_XY diff = p3 - p1;

	t = diff.Crossed(s) / denom;
	double u = diff.Crossed(r) / denom;

	return (t > -precision && t < 1.0 + precision &&
		u > -precision && u < 1.0 + precision);
}

double helperFunctions::getObjectZOffset(IfcSchema::IfcObjectPlacement* objectPlacement, bool deepOnly)
{
	double offset = 0;
	if (objectPlacement->data().type()->name() != "IfcLocalPlacement") { return 0.0; }
	IfcSchema::IfcLocalPlacement* storeyLocalPlacement = objectPlacement->as<IfcSchema::IfcLocalPlacement>();
	IfcSchema::IfcObjectPlacement* localObjectPlacement = storeyLocalPlacement->PlacementRelTo();

	if (!deepOnly && localObjectPlacement != nullptr || localObjectPlacement == nullptr)
	{
		if (storeyLocalPlacement->RelativePlacement()->data().type()->name() != "IfcAxis2Placement3D") { return 0.0; }
		IfcSchema::IfcAxis2Placement3D* axisPlacement = storeyLocalPlacement->RelativePlacement()->as<IfcSchema::IfcAxis2Placement3D>();

#if defined(USE_IFC4x3) || defined (USE_IFC4x3add1)|| defined (USE_IFC4x3add2)
		offset = axisPlacement->Location()->as<IfcSchema::IfcCartesianPoint>()->Coordinates()[2];
#else
		try
		{
			std::vector<double> coord = axisPlacement->Location()->Coordinates();

			if (coord.size() >= 3)
			{
				offset = axisPlacement->Location()->Coordinates()[2];
			}
		}
		catch (const std::exception&)
		{

		}

#endif // DEBUG
	}
	if (localObjectPlacement == nullptr)
	{
		return offset;
	}
	return offset + getObjectZOffset(localObjectPlacement, deepOnly);
}

bool helperFunctions::hasGlassMaterial(const IfcSchema::IfcProduct* ifcProduct)
{
	IfcSchema::IfcRelAssociates::list::ptr associations = ifcProduct->HasAssociations();
	for (IfcSchema::IfcRelAssociates::list::it it = associations->begin(); it != associations->end(); ++it)
	{
		IfcSchema::IfcRelAssociates* IfcRelAssociates = *it;
		if (IfcRelAssociates->data().type()->name() != "IfcRelAssociatesMaterial") { continue; }

		IfcSchema::IfcRelAssociatesMaterial* MaterialAss = IfcRelAssociates->as<IfcSchema::IfcRelAssociatesMaterial>();
		if (MaterialAss->data().type()->name() != "IfcRelAssociatesMaterial") { continue; }

		IfcSchema::IfcMaterialSelect* relMaterial = MaterialAss->RelatingMaterial();
		if (relMaterial->data().type()->name() != "IfcMaterial") { continue; }

		IfcSchema::IfcMaterial* ifcMaterial = relMaterial->as<IfcSchema::IfcMaterial>();
		std::string materialName = boost::to_upper_copy(ifcMaterial->Name());

		if (materialName.find("GLASS") != std::string::npos ||
			materialName.find("GLAZED") != std::string::npos)
		{
			return true;
		}

#if defined(USE_IFC2x3) || defined(USE_IFC4) 

		// if material name is not glass or glazed search for render properties transparency
		IfcSchema::IfcMaterialDefinitionRepresentation::list::ptr materialRepresentation = ifcMaterial->HasRepresentation();
		IfcSchema::IfcStyledRepresentation* currentStyleRep = nullptr;

		for (auto propertyIt = materialRepresentation->begin(); propertyIt != materialRepresentation->end(); ++propertyIt)
		{
			bool found = false;
			IfcSchema::IfcMaterialDefinitionRepresentation* currentMaterialRepresenation = *propertyIt;

			IfcSchema::IfcRepresentation::list::ptr representationList = currentMaterialRepresenation->Representations();

			for (auto repIt = representationList->begin(); repIt != representationList->end(); ++repIt)
			{
				IfcSchema::IfcRepresentation* currentRep = *repIt;
				if (currentRep->data().type()->name() == "IfcStyledRepresentation")
				{
					currentStyleRep = currentRep->as<IfcSchema::IfcStyledRepresentation>();
					found = true;
					break;
				}
			}
			if (found) { break; }
		}

		// find via object material
		if (currentStyleRep != nullptr) {
			IfcSchema::IfcRepresentationItem::list::ptr representationList = currentStyleRep->Items();
			for (auto propertyIt = representationList->begin(); propertyIt != representationList->end(); ++propertyIt)
			{
				IfcSchema::IfcRepresentationItem* currentItem = *propertyIt;
				if (currentItem->data().type()->name() != "IfcStyledItem") { continue; }
				IfcSchema::IfcStyledItem* currentStyledItem = currentItem->as<IfcSchema::IfcStyledItem>();

#if defined(USE_IFC2x3) 
				IfcSchema::IfcPresentationStyleAssignment::list::ptr currenStyleAssList = currentStyledItem->Styles();
#elif defined(USE_IFC4) 
				IfcSchema::IfcStyleAssignmentSelect::list::ptr currenStyleAssList = currentStyledItem->Styles();
#endif

				for (auto currenStyleAssIt = currenStyleAssList->begin(); currenStyleAssIt != currenStyleAssList->end(); ++currenStyleAssIt)
				{
#if defined(USE_IFC2x3) 
					IfcSchema::IfcPresentationStyleAssignment* currentStyleAss = *currenStyleAssIt;
#elif defined(USE_IFC4) 
					IfcSchema::IfcStyleAssignmentSelect* currentStyleAss = *currenStyleAssIt;
#endif				
					if (currentStyleAss->data().type()->name() != "IfcPresentationStyleAssignment") { continue; }
					IfcSchema::IfcPresentationStyleAssignment* currentAss = currentStyleAss->as<IfcSchema::IfcPresentationStyleAssignment>();
					IfcSchema::IfcPresentationStyleSelect::list::ptr styleSelectList = currentAss->Styles();

					for (auto styleSelectIt = styleSelectList->begin(); styleSelectIt != styleSelectList->end(); ++styleSelectIt)
					{
						IfcSchema::IfcPresentationStyleSelect* currentStyleSelect = *styleSelectIt;
						if (currentStyleSelect->data().type()->name() != "IfcSurfaceStyle") { continue; }
						IfcSchema::IfcSurfaceStyle* currentStyle = currentStyleSelect->as<IfcSchema::IfcSurfaceStyle>();
						IfcSchema::IfcSurfaceStyleElementSelect::list::ptr elementSurfList = currentStyle->Styles();

						bool hasRenderingStyle = false;
						for (auto styleElementIt = elementSurfList->begin(); styleElementIt != elementSurfList->end(); ++styleElementIt)
						{
							IfcSchema::IfcSurfaceStyleElementSelect* currentElemStyle = *styleElementIt;
							if (currentElemStyle->data().type()->name() != "IfcSurfaceStyleRendering") { continue; }
							IfcSchema::IfcSurfaceStyleRendering* currentRenderStyle = currentElemStyle->as< IfcSchema::IfcSurfaceStyleRendering>();
							if (currentRenderStyle->Transparency() > 0.2) { return true; }
							hasRenderingStyle = true;
						}
						if (hasRenderingStyle) { return false; }
					}
				}
			}
		}
	}

	// find via geometry material
	IfcSchema::IfcProductRepresentation* currentProductRep = ifcProduct->Representation();
	IfcSchema::IfcRepresentation::list::ptr currentRepList = currentProductRep->Representations();

	std::vector< IfcSchema::IfcRepresentation*> representationList;
	for (auto repIt = currentRepList->begin(); repIt != currentRepList->end(); ++repIt)
	{
		IfcSchema::IfcRepresentation* currentRep = *repIt;
		if (!currentRep->RepresentationIdentifier()) { continue; }
		if (currentRep->RepresentationIdentifier().get() != "Body") { continue; }
		if (currentRep->RepresentationType().get() == "MappedRepresentation") // repesentation is used as container
		{
			IfcSchema::IfcRepresentationItem::list::ptr representationSubItemList = currentRep->Items();



			for (auto represenetationSubIt = representationSubItemList->begin(); represenetationSubIt != representationSubItemList->end(); ++represenetationSubIt)
			{
				IfcSchema::IfcRepresentationItem* subRepresentationItem = *represenetationSubIt;
				if (subRepresentationItem->data().type()->name() != "IfcMappedItem") { continue; }
				IfcSchema::IfcMappedItem* currentMappedItem = subRepresentationItem->as<IfcSchema::IfcMappedItem>();
				IfcSchema::IfcRepresentationMap* currentRepMap = currentMappedItem->MappingSource();
				IfcSchema::IfcRepresentation* subRep = currentRepMap->MappedRepresentation();
				representationList.emplace_back(subRep);
			}
			continue;
		}
		representationList.emplace_back(currentRep);
	}

	for (IfcSchema::IfcRepresentation* currentRep : representationList)
	{
		IfcSchema::IfcRepresentationItem::list::ptr representationSubItemList = currentRep->Items();

		for (auto RepresentationSubItemIt = representationSubItemList->begin(); RepresentationSubItemIt != representationSubItemList->end(); ++RepresentationSubItemIt)
		{
			IfcSchema::IfcRepresentationItem* RepresentationSubItem = *RepresentationSubItemIt;
			IfcSchema::IfcStyledItem::list::ptr StyledItemList = RepresentationSubItem->StyledByItem();

			for (auto styledItemIt = StyledItemList->begin(); styledItemIt != StyledItemList->end(); ++styledItemIt)
			{

				IfcSchema::IfcStyledItem* styledItem = *styledItemIt;

#if defined(USE_IFC2x3)
				IfcSchema::IfcPresentationStyleAssignment::list::ptr currenStyleAssList = styledItem->Styles();
#elif defined(USE_IFC4)
				IfcSchema::IfcStyleAssignmentSelect::list::ptr currenStyleAssList = styledItem->Styles();
#endif
				for (auto currenStyleAssIt = currenStyleAssList->begin(); currenStyleAssIt != currenStyleAssList->end(); ++currenStyleAssIt)
				{
#if defined(USE_IFC2x3)
					IfcSchema::IfcPresentationStyleAssignment* currentStyleAss = *currenStyleAssIt;
#elif defined(USE_IFC4)
					IfcSchema::IfcStyleAssignmentSelect* currentStyleAss = *currenStyleAssIt;
#endif
					if (currentStyleAss->data().type()->name() == "IfcSurfaceStyle") {

						IfcSchema::IfcSurfaceStyle* currentStyle = currentStyleAss->as<IfcSchema::IfcSurfaceStyle>();
						IfcSchema::IfcSurfaceStyleElementSelect::list::ptr elementSurfList = currentStyle->Styles();

						for (auto styleElementIt = elementSurfList->begin(); styleElementIt != elementSurfList->end(); ++styleElementIt)
						{
							IfcSchema::IfcSurfaceStyleElementSelect* currentElemStyle = *styleElementIt;
							if (currentElemStyle->data().type()->name() != "IfcSurfaceStyleRendering") { continue; }
							IfcSchema::IfcSurfaceStyleRendering* currentRenderStyle = currentElemStyle->as< IfcSchema::IfcSurfaceStyleRendering>();
							if (currentRenderStyle->Transparency() > 0.1) { return true; }
						}
					}
#if defined(USE_IFC2x3)
					if (currentStyleAss->data().type()->name() == "IfcPresentationStyleAssignment") {
						IfcSchema::IfcPresentationStyleSelect::list::ptr presentationStyles = currentStyleAss->Styles();
						for (auto styleElementIt = presentationStyles->begin(); styleElementIt != presentationStyles->end(); ++styleElementIt)
						{
							IfcSchema::IfcPresentationStyleSelect* presentationStyleSelect = *styleElementIt;
							if (presentationStyleSelect->data().type()->name() == "IfcSurfaceStyle") {

								IfcSchema::IfcSurfaceStyle* currentStyle = presentationStyleSelect->as<IfcSchema::IfcSurfaceStyle>();
								IfcSchema::IfcSurfaceStyleElementSelect::list::ptr elementSurfList = currentStyle->Styles();

								for (auto styleElementIt = elementSurfList->begin(); styleElementIt != elementSurfList->end(); ++styleElementIt)
								{
									IfcSchema::IfcSurfaceStyleElementSelect* currentElemStyle = *styleElementIt;
									if (currentElemStyle->data().type()->name() != "IfcSurfaceStyleRendering") { continue; }
									IfcSchema::IfcSurfaceStyleRendering* currentRenderStyle = currentElemStyle->as< IfcSchema::IfcSurfaceStyleRendering>();
									if (currentRenderStyle->Transparency() > 0.2) { return true; }
								}
							}
						}
					}
#endif
				}
			}
		}
	}
#else //TODO: implement for ifc4x3
	}
#endif
	return false;
}

bool helperFunctions::isExternal(const IfcSchema::IfcProduct* ifcProduct)
{
#if defined(USE_IFC2x3)
	IfcSchema::IfcRelDefines::list::ptr propertyDefinesList = ifcProduct->IsDefinedBy();
#else
	IfcSchema::IfcRelDefinesByProperties::list::ptr propertyDefinesList = ifcProduct->IsDefinedBy();
#endif
	for (auto propertyDefines = propertyDefinesList->begin(); propertyDefines != propertyDefinesList->end(); ++propertyDefines)
	{
#if defined(USE_IFC2x3)
		IfcSchema::IfcRelDefinesByProperties* prop = (*propertyDefines)->as<IfcSchema::IfcRelDefinesByProperties>();
		if (prop == nullptr) { continue; }
		IfcSchema::IfcPropertySet* pset = prop->RelatingPropertyDefinition()->as<IfcSchema::IfcPropertySet>();
#else
		IfcSchema::IfcPropertySet* pset = (*propertyDefines)->RelatingPropertyDefinition()->as<IfcSchema::IfcPropertySet>();
#endif

		if (pset == nullptr) { continue; }

		boost::optional<std::string> optionalName = pset->Name();
		if (!optionalName.has_value()) { continue; }
		if ((*optionalName).find("Common") == std::string::npos) { continue; }

		IfcSchema::IfcProperty::list::ptr propertyList = pset->HasProperties();
		for (auto propertyIt = propertyList->begin(); propertyIt != propertyList->end(); ++propertyIt)
		{
			IfcSchema::IfcProperty* currentProperty = *propertyIt;
			if (currentProperty->Name().find("IsExternal") == std::string::npos) { continue; }

			IfcSchema::IfcPropertySingleValue* propertyItem = (*propertyIt)->as<IfcSchema::IfcPropertySingleValue>();

			if (propertyItem == nullptr) { continue; }

			IfcSchema::IfcValue* ifcValue = ifcValue = propertyItem->NominalValue();
			if (ifcValue == nullptr) { continue; }

			std::string propertyIdName = ifcValue->data().type()->name();
			if (propertyIdName != "IfcBoolean") { continue; }
			return ifcValue->as<IfcSchema::IfcBoolean>()->operator bool();
		}
	}
	return false;
}

nlohmann::json helperFunctions::getAttributes(const IfcSchema::IfcProduct* ifcProduct, const std::string& PsetName)
{
	nlohmann::json attributesList;

	bool filter = !PsetName.empty();

	std::string noneType = "None";

	// get object data

	if (!filter || PsetName == "Element Specific")
	{
		attributesList["Guid"] = ifcProduct->GlobalId();
		attributesList["IfcEntity"] = ifcProduct->data().type()->name();

		boost::optional<std::string> optionalName = ifcProduct->Name();
		if (optionalName.has_value()) { attributesList["Name"] = optionalName.get(); }
		else { attributesList["Name"] = "None"; }

		boost::optional<std::string> description = ifcProduct->Description();
		if (description.has_value()) { attributesList["Description"] = description.get(); }
		else { attributesList["Description"] = "None"; }

		boost::optional<std::string> optionalType = ifcProduct->ObjectType();
		if (optionalType.has_value()) { attributesList["ObjectType"] = optionalType.get(); }
		else { attributesList["ObjectType"] = "None"; }

		attributesList["Tag"] = ifcProduct->identity();
			
		
		if (auto* ifcSpace = ifcProduct->as<IfcSchema::IfcSpace>())
		{
			
			boost::optional<std::string> optonalLongName = ifcSpace->LongName();
			if (optionalType.has_value()) { attributesList["LongName"] = optonalLongName.get(); }
			else { attributesList["LongName"] = "None"; }
		}
		if (auto* ifcBuilding = ifcProduct->as<IfcSchema::IfcBuilding>())
		{

			boost::optional<std::string> optonalLongName = ifcBuilding->LongName();
			if (optionalType.has_value()) { attributesList["LongName"] = optonalLongName.get(); }
			else { attributesList["LongName"] = "None"; }
		}
		if (auto* ifcStorey = ifcProduct->as<IfcSchema::IfcBuildingStorey>())
		{

			boost::optional<std::string> optonalLongName = ifcStorey->LongName();
			if (optionalType.has_value()) { attributesList["LongName"] = optonalLongName.get(); }
			else { attributesList["LongName"] = "None"; }
		}

	}

	// get pset data
#if defined(USE_IFC2x3)
	IfcSchema::IfcRelDefines::list::ptr propertyDefinesList = ifcProduct->IsDefinedBy();
#else
	IfcSchema::IfcRelDefinesByProperties::list::ptr propertyDefinesList = ifcProduct->IsDefinedBy();
#endif
	for (auto propertyDefines = propertyDefinesList->begin(); propertyDefines != propertyDefinesList->end(); ++propertyDefines)
	{
#if defined(USE_IFC2x3)
		IfcSchema::IfcRelDefinesByProperties* prop = (*propertyDefines)->as<IfcSchema::IfcRelDefinesByProperties>();
		if (prop == nullptr) { continue; }
		IfcSchema::IfcPropertySet* pset = prop->RelatingPropertyDefinition()->as<IfcSchema::IfcPropertySet>();
#else
		IfcSchema::IfcPropertySet* pset = (*propertyDefines)->RelatingPropertyDefinition()->as<IfcSchema::IfcPropertySet>();
#endif
		if (pset == nullptr) { continue; }

		if (filter)
		{
			boost::optional<std::string> optionalPsetName = pset->Name();
			if (optionalPsetName.get() != PsetName) { continue; }
		}

		nlohmann::json attributesListLocal = getAttributes(*pset);
		if (attributesListLocal.empty()) { continue; }

		for (auto& [key, value] : attributesListLocal.items())
		{
			attributesList.emplace(key, value);
		}
	}
	return attributesList;
}

nlohmann::json helperFunctions::getAttributes(const IfcSchema::IfcPropertySet& pset)
{
	nlohmann::json attributesList;
	IfcSchema::IfcProperty::list::ptr propertyList = pset.HasProperties();

	for (auto propertyIt = propertyList->begin(); propertyIt != propertyList->end(); propertyIt++)
	{
		if (*propertyIt == nullptr) { continue; }

		if ((*propertyIt)->data().type()->name() != "IfcPropertySingleValue") //TODO: implement IfcPropertyEnumeratedValue
		{
			continue;
		}

		IfcSchema::IfcPropertySingleValue* propertyItem = (*propertyIt)->as<IfcSchema::IfcPropertySingleValue>();
		IfcSchema::IfcValue* ifcValue = ifcValue = propertyItem->NominalValue();
		if (ifcValue == nullptr) { continue; }

		std::string propertyIdName = ifcValue->data().type()->name();

		if (propertyIdName == "IfcIdentifier")
		{
			IfcSchema::IfcIdentifier* propertyValueContainer = ifcValue->as<IfcSchema::IfcIdentifier>();
			attributesList[propertyItem->Name()] = propertyValueContainer->operator std::string();
		}
		else if (propertyIdName == "IfcText")
		{
			IfcSchema::IfcText* propertyValueContainer = ifcValue->as<IfcSchema::IfcText>();
			attributesList[propertyItem->Name()] = propertyValueContainer->operator std::string();
		}
		else if (propertyIdName == "IfcLabel")
		{
			IfcSchema::IfcLabel* propertyValueContainer = ifcValue->as<IfcSchema::IfcLabel>();
			attributesList[propertyItem->Name()] = propertyValueContainer->operator std::string();
		}
		else if (propertyIdName == "IfcLengthMeasure")
		{
			IfcSchema::IfcLengthMeasure* propertyValueContainer = ifcValue->as<IfcSchema::IfcLengthMeasure>();
			attributesList[propertyItem->Name()] = {
				{CJObjectEnum::getString(CJObjectID::jsonValue), propertyValueContainer->operator double() },
				{CJObjectEnum::getString(CJObjectID::jsonUom) , UnitStringEnum::getString(UnitStringID::meter) } //TODO: update to unit?
			};
		}
		else if (propertyIdName == "IfcAreaMeasure")
		{
			IfcSchema::IfcAreaMeasure* propertyValueContainer = ifcValue->as<IfcSchema::IfcAreaMeasure>();
			attributesList[propertyItem->Name()] = {
				{CJObjectEnum::getString(CJObjectID::jsonValue), propertyValueContainer->operator double() },
				{CJObjectEnum::getString(CJObjectID::jsonUom) , UnitStringEnum::getString(UnitStringID::sqrMeter) } //TODO: update to set unit?
			};
		}
		else if (propertyIdName == "IfcReal")
		{
			IfcSchema::IfcReal* propertyValueContainer = ifcValue->as<IfcSchema::IfcReal>();
			attributesList[propertyItem->Name()] = propertyValueContainer->operator double();
		}
		else if (propertyIdName == "IfcInteger")
		{
			IfcSchema::IfcInteger* propertyValueContainer = ifcValue->as<IfcSchema::IfcInteger>();
			attributesList[propertyItem->Name()] = propertyValueContainer->operator int();
		}
		else if (propertyIdName == "IfcPowerMeasure")
		{
			IfcSchema::IfcPowerMeasure* propertyValueContainer = ifcValue->as<IfcSchema::IfcPowerMeasure>();
			attributesList[propertyItem->Name()] = propertyValueContainer->operator double();
		}
		else if (propertyIdName == "IfcThermalTransmittanceMeasure")
		{
			IfcSchema::IfcThermalTransmittanceMeasure* propertyValueContainer = ifcValue->as<IfcSchema::IfcThermalTransmittanceMeasure>();
			attributesList[propertyItem->Name()] = propertyValueContainer->operator double();
		}
		else if (propertyIdName == "IfcBoolean")
		{
			IfcSchema::IfcBoolean* propertyValueContainer = ifcValue->as<IfcSchema::IfcBoolean>();
			attributesList[propertyItem->Name()] = propertyValueContainer->operator bool();
		}
		else if (propertyIdName == "IfcLogical")
		{
			IfcSchema::IfcLogical* propertyValueContainer = ifcValue->as<IfcSchema::IfcLogical>();
			boost::logic::tribool tribool = propertyValueContainer->operator boost::logic::tribool();

			if (tribool.value == tribool.false_value) { attributesList[propertyItem->Name()] = "FALSE"; }
			else if (tribool.value == tribool.true_value) { attributesList[propertyItem->Name()] = "TRUE"; }
			else { attributesList[propertyItem->Name()] = "UNKNOWN"; }
		}
		else
		{
			ErrorCollection::getInstance().addError(ErrorID::propertyNotImplemented, propertyIdName);
		}
	}
	return attributesList;
}

void helperFunctions::writeToSTEP(const TopoDS_Shape& theShape, const std::string& targetPath)
{
	std::stringstream buffer;
	std::streambuf* originalBuffer = std::cout.rdbuf(buffer.rdbuf());

	STEPControl_Writer writer;
	writer.Transfer(theShape, STEPControl_AsIs);
	IFSelect_ReturnStatus stat = writer.Write(targetPath.c_str());

	std::cout.rdbuf(originalBuffer);
	return;
}

template <typename T>
void helperFunctions::writeToSTEP(const std::vector<T>& theShapeList, const std::string& targetPath)
{
	std::stringstream buffer;
	std::streambuf* originalBuffer = std::cout.rdbuf(buffer.rdbuf());

	TopoDS_Compound compound;
	BRep_Builder builder;
	builder.MakeCompound(compound);
	for (const T& shape : theShapeList)
	{
		builder.Add(compound, shape);
	}

	STEPControl_Writer writer;
	writer.Transfer(compound, STEPControl_AsIs);
	IFSelect_ReturnStatus stat = writer.Write(targetPath.c_str());

	std::cout.rdbuf(originalBuffer);
	return;
}

template <typename T>
void helperFunctions::writeToSTEP(const std::vector<std::vector<T>>& theShapeList, const std::string& targetPath)
{
	std::stringstream buffer;
	std::streambuf* originalBuffer = std::cout.rdbuf(buffer.rdbuf());

	STEPControl_Writer writer;
	for (const auto& nestedList : theShapeList)
	{
		for (const T& shape : nestedList) { writer.Transfer(shape, STEPControl_AsIs); }
	}
	
	IFSelect_ReturnStatus stat = writer.Write(targetPath.c_str());

	std::cout.rdbuf(originalBuffer);
	return;
}

template <typename T>
void helperFunctions::writeToOBJ(const T& theShape, const std::string& targetPath)
{
	std::vector<T> shapeList = { theShape };
	writeToOBJ(shapeList, targetPath);
	return;
}

template <typename T>
void helperFunctions::writeToOBJ(const std::vector<T>& theShapeList, const std::string& targetPath)
{
	if (theShapeList.empty()) { return; }

	double precision = SettingsCollection::getInstance().linearTolerance();

	std::ofstream objFile(targetPath);
	int vertIdxOffset = 1;
	std::vector<std::vector<int>> nestedTriangleIndx;
	std::unordered_map<IntXYZ, int, IntXYZ_Hash, IntXYZ_Equal> vertMap;

	int counter = 0;
	for (const T& theShape : theShapeList)
	{
		for (TopExp_Explorer exp(theShape, TopAbs_FACE); exp.More(); exp.Next()) {
			TopoDS_Face face = TopoDS::Face(exp.Current());
			TopLoc_Location loc;

			auto mesh = BRep_Tool::Triangulation(face, loc);

			if (mesh.IsNull())
			{
				triangulateShape(face);
				mesh = BRep_Tool::Triangulation(face, loc);
			}
			if (mesh.IsNull()) { continue; }

			bool flipTriangle = false;
			if (face.Orientation() == TopAbs_REVERSED)
			{
				flipTriangle = true;
			}

			for (int j = 1; j <= mesh.get()->NbTriangles(); j++)
			{
				const Poly_Triangle& theTriangle = mesh->Triangles().Value(j);

				std::vector<int> triangleIndx = {};

				if (flipTriangle)
				{
					for (size_t i = 3; i >= 1; i--)
					{
						gp_XYZ xyz = mesh->Nodes().Value(theTriangle(i)).Transformed(loc).Coord();
						IntXYZ intXyz = IntXYZ(xyz, 1/ precision);

						if (vertMap.find(intXyz) != vertMap.end())
						{
							triangleIndx.emplace_back(vertMap[intXyz]);
							continue;
						}

						objFile << "v " << xyz.X() << " " << xyz.Y() << " " << xyz.Z() << "\n";
						triangleIndx.emplace_back(vertIdxOffset);
						vertMap.try_emplace(intXyz, vertIdxOffset);
						vertIdxOffset++;
					}
				}
				else
				{
					for (size_t i = 1; i <= 3; i++)
					{
						gp_XYZ xyz = mesh->Nodes().Value(theTriangle(i)).Transformed(loc).Coord();
						IntXYZ intXyz = IntXYZ(xyz, 1 / precision);

						if (vertMap.find(intXyz) != vertMap.end())
						{
							triangleIndx.emplace_back(vertMap[intXyz]);
							continue;
						}

						objFile << "v " << xyz.X() << " " << xyz.Y() << " " << xyz.Z() << "\n";
						triangleIndx.emplace_back(vertIdxOffset);
						vertMap.try_emplace(intXyz, vertIdxOffset);
						vertIdxOffset++;
					}
				}
				nestedTriangleIndx.emplace_back(triangleIndx);
			}
		}
	}
	for (const auto& triangleList : nestedTriangleIndx)
	{
		objFile << "f";
		for (int indx : triangleList)
		{
			objFile << " " << indx;
		}
		objFile << "\n";
	}
	objFile.close();
	return;
}

template <typename T>
void helperFunctions::writeToOBJ(const std::vector<std::vector<T>>& theShapeList, const std::string& targetPath)
{
	std::vector<T> flattenedSurfList;
	for (const auto& surfList : theShapeList)
	{
		for (const auto& theSurf : surfList)
		{
			flattenedSurfList.emplace_back(theSurf);
		}
	}
	writeToOBJ(flattenedSurfList, targetPath);
	return;
}

int helperFunctions::getFaceCount(const TopoDS_Shape& inputShape)
{
	TopTools_IndexedMapOfShape faces;
	TopExp::MapShapes(inputShape, TopAbs_FACE, faces);
	return faces.Extent();
}

bool helperFunctions::faceIsValid(const TopoDS_Face& theFace)
{
	if (theFace.IsNull())
	{
		return false;
	}
	BRepCheck_Analyzer brepanalyzer(theFace);
	if (brepanalyzer.IsValid())
	{
		return true;
	}

	BRepCheck_Face faceCheck(theFace); //TODO: add wire check

	// Get list of specific errors
	for (const auto& status : faceCheck.Status())
	{
		if (status == BRepCheck_NoError)
		{
			return true;
		}
	}
	return false;
}

void helperFunctions::printTime(std::chrono::steady_clock::time_point startTime, std::chrono::steady_clock::time_point endTime)
{
	long long duration = std::chrono::duration_cast<std::chrono::seconds>(endTime - startTime).count();
	if (duration < 5)
	{
		std::cout << CommunicationStringEnum::getString(CommunicationStringID::indentSuccesFinished) <<
			std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime).count() << UnitStringEnum::getString(UnitStringID::milliseconds) << std::endl;
	}
	else {
		std::cout << CommunicationStringEnum::getString(CommunicationStringID::indentSuccesFinished) <<
			std::chrono::duration_cast<std::chrono::seconds>(endTime - startTime).count() << UnitStringEnum::getString(UnitStringID::seconds) << std::endl;
	}
}

double helperFunctions::computeArea(const TopoDS_Face& theFace)
{
	GProp_GProps gprops;
	BRepGProp::SurfaceProperties(theFace, gprops);
	return gprops.Mass();
}

double helperFunctions::computeArea(const TopoDS_Wire& wire)
{
	std::vector<gp_Pnt> points;
	double precision = SettingsCollection::getInstance().linearTolerance();

	double zVal = 0;
	bool zValSet = false;

	for (BRepTools_WireExplorer exp(wire); exp.More(); exp.Next())
	{
		TopoDS_Edge edge = TopoDS::Edge(exp.Current());
		gp_Pnt currentPoint = getFirstPointShape(edge);

		if (!zValSet)
		{
			zVal = currentPoint.Z();
			zValSet = true;
		}
		else
		{
			if (std::abs(currentPoint.Z() - zVal) > precision)
			{
				throw std::string("surface is not flat");
			}
		}
	}

	if (points.size() < 3)
	{
		return 0.0;
	}

	double area = 0.0;

	for (size_t i = 0; i < points.size(); i++)
	{
		const gp_Pnt& p0 = points[i];
		const gp_Pnt& p1 = points[(i + 1) % points.size()];

		area += p0.X() * p1.Y() - p1.X() * p0.Y();
	}

	return abs(area) * 0.5;
}

void helperFunctions::triangulateShape(const TopoDS_Shape& shape, bool force)
{
	SettingsCollection& settingsCollection = SettingsCollection::getInstance();
	std::mutex* triangleMutex = settingsCollection.getTriangleMutex();

	double setDeflection = settingsCollection.meshLinearDeflection();
	for (TopExp_Explorer faceExpl(shape, TopAbs_FACE); faceExpl.More(); faceExpl.Next())
	{
		TopoDS_Face currentFace = TopoDS::Face(faceExpl.Current());

		if (currentFace.IsNull() || BRep_Tool::Surface(currentFace).IsNull()) {
			continue;
		}

		TopLoc_Location loc;
		Handle(Poly_Triangulation) triangulation = BRep_Tool::Triangulation(currentFace, loc);

		if (!triangulation.IsNull()) {
			if (!force)
			{
				continue;  // No triangulation present, skip to the next face
			}
			BRepTools::Clean(currentFace);
		}

		std::vector<gp_Pnt> uniquePointList = helperFunctions::getUniquePoints(currentFace);

		if (uniquePointList.size() < 3) { continue; }
		if (uniquePointList.size() == 3)
		{
			gp_Trsf inverseLoc = loc.Transformation().Inverted();
			Handle(Poly_Triangulation) triangulation = new Poly_Triangulation(3, 1, Standard_True);

			// add 3D points
			TColgp_Array1OfPnt nodes(1, 3);
			nodes.SetValue(1, uniquePointList[0].Transformed(inverseLoc));
			nodes.SetValue(2, uniquePointList[1].Transformed(inverseLoc));
			nodes.SetValue(3, uniquePointList[2].Transformed(inverseLoc));
			triangulation->ChangeNodes() = nodes;

			// add the triangular shape
			Poly_Array1OfTriangle triangles(1, 1);  // One triangle at index 1
			triangles.SetValue(1, Poly_Triangle(1, 2, 3));
			triangulation->ChangeTriangles() = triangles;

			// add uv data
			Handle(Geom_Surface) surf = BRep_Tool::Surface(currentFace);
			TColgp_Array1OfPnt2d uvNodes(1, 3);
			for (int i = 1; i <= 3; ++i)
			{
				GeomAPI_ProjectPointOnSurf proj(nodes.Value(i), surf);
				Standard_Real u, v;
				proj.LowerDistanceParameters(u, v);
				uvNodes.SetValue(i, gp_Pnt2d(u, v));
			}
			triangulation->ChangeUVNodes() = uvNodes;

			// add shape
			BRep_Builder builder;
			builder.UpdateFace(currentFace, triangulation);
			continue;
		}
		
		double area = computeArea(currentFace);
		double triangleCount = area / (0.5 * std::pow(setDeflection, 2) * std::sqrt(3.0));
		if (std::isinf(triangleCount)) { continue; }

		BRepTools::Clean(currentFace);

		triangleMutex->lock();
		BRepMesh_IncrementalMesh(currentFace, setDeflection, Standard_False);
		triangleMutex->unlock();

		TopLoc_Location locLocal;
		Handle(Poly_Triangulation) tri = BRep_Tool::Triangulation(currentFace, loc);
	}
	return;
}

TopoDS_Wire helperFunctions::CurveToCompound(const TopoDS_Edge& theEdge)
{
	double precision = SettingsCollection::getInstance().linearTolerance();
	double stepLenght = 0.3;

	Standard_Real first, last;
	Handle(Geom_Curve) curve = BRep_Tool::Curve(theEdge, first, last);
	if (curve.IsNull()) return {};
	GeomAdaptor_Curve adaptorCurve(curve, first, last);
	double curveLength = GCPnts_AbscissaPoint::Length(adaptorCurve, first, last);

	int splitSteps = std::ceil(curveLength / stepLenght);
	if (curveLength / 3 < SettingsCollection::getInstance().linearTolerance()) { splitSteps = 2; }
	else if (splitSteps < 2) { splitSteps = 3; }

	GCPnts_UniformAbscissa abscissa(adaptorCurve, splitSteps, first, last);
	if (!abscissa.IsDone()) return {};

	BRepBuilderAPI_MakeWire builder;
	for (int i = 1; i < abscissa.NbPoints(); ++i) {
		gp_Pnt p1 = adaptorCurve.Value(abscissa.Parameter(i));
		gp_Pnt p2 = adaptorCurve.Value(abscissa.Parameter(i + 1));

		if (p1.IsEqual(p2, precision))
		{ 
			continue;
		}

		TopoDS_Edge segment = BRepBuilderAPI_MakeEdge(p1, p2);
		builder.Add(segment);
	}

	builder.Build();
	if (builder.IsDone())
	{
		return builder.Wire();
	}
	return {};
}

TopoDS_Wire helperFunctions::replaceCurves(const TopoDS_Wire& theWire)
{

	std::vector<TopoDS_Edge> fixedEdges;
	double precision = SettingsCollection::getInstance().linearTolerance();
	bool isEdited = false;

	for (BRepTools_WireExplorer expl(theWire); expl.More(); expl.Next()) {
		TopoDS_Edge currentEdge = TopoDS::Edge(expl.Current());

		if (isStraight(currentEdge)) //TODO: make this smarter
		{
			gp_Pnt p1 = getFirstPointShape(currentEdge);
			gp_Pnt p2 = getLastPointShape(currentEdge);

			if (p1.IsEqual(p2, precision))
			{
				continue;
			}

			TopoDS_Edge segment = BRepBuilderAPI_MakeEdge(p1, p2);

			fixedEdges.emplace_back(segment);
			continue;
		}

		isEdited = true;
		TopoDS_Wire straightCurve = CurveToCompound(currentEdge);
		if (straightCurve.IsNull())
		{
			return theWire;
		}

		// if the wire is incorrectly ordered the order is reversed
		std::vector<TopoDS_Edge> straightEdgeList;
		for (BRepTools_WireExplorer expl2(straightCurve); expl2.More(); expl2.Next()) {
			TopoDS_Edge straightenedEdge = TopoDS::Edge(expl2.Current());
			straightEdgeList.emplace_back(straightenedEdge);
		}

		if (fixedEdges.empty())
		{
			fixedEdges.insert(std::end(fixedEdges), std::begin(straightEdgeList), std::end(straightEdgeList));
			continue;
		}

		TopoDS_Edge lastEdge = fixedEdges.back();
		TopoDS_Edge	firstEdge = fixedEdges[0];
		TopoDS_Edge connectingEdge = straightEdgeList[0];
		gp_Pnt lp1 = helperFunctions::getFirstPointShape(lastEdge);
		gp_Pnt lp2 = helperFunctions::getLastPointShape(lastEdge);
		gp_Pnt fp1 = helperFunctions::getFirstPointShape(firstEdge);
		gp_Pnt fp2 = helperFunctions::getLastPointShape(firstEdge);
		gp_Pnt sp1 = helperFunctions::getFirstPointShape(connectingEdge);
		gp_Pnt sp2 = helperFunctions::getLastPointShape(connectingEdge);

		if (!lp1.IsEqual(sp1, precision) && !lp1.IsEqual(sp2, precision) &&
			!lp2.IsEqual(sp1, precision) && !lp2.IsEqual(sp2, precision) &&
			!fp1.IsEqual(sp1, precision) && !fp1.IsEqual(sp2, precision) &&
			!fp2.IsEqual(sp1, precision) && !fp2.IsEqual(sp2, precision)
			)
		{
			std::reverse(straightEdgeList.begin(), straightEdgeList.end());
		}
		fixedEdges.insert(std::end(fixedEdges), std::begin(straightEdgeList), std::end(straightEdgeList));
	}

	if (!isEdited) {return theWire; }

	BRepBuilderAPI_MakeWire builder;
	for (const TopoDS_Edge& fixedEdge : fixedEdges)
	{
		builder.Add(fixedEdge);
	}

	builder.Build();
	if (!builder.IsDone())
	{
		return theWire;
	}

	TopoDS_Wire cleanedWire = builder.Wire();

	if (!cleanedWire.Closed())
	{
		//std::cout << "hit" << std::endl;
		//DebugUtils::printPoints(cleanedWire);
		return theWire;
	}

	return cleanedWire;
}

std::vector<TopoDS_Edge> helperFunctions::replaceCurves(const std::vector<TopoDS_Edge>& theEdgeList)
{
	std::vector<TopoDS_Edge> fixedEdges;
	for (const TopoDS_Edge& currentEdge : theEdgeList)
	{
		if (isStraight(currentEdge))
		{
			fixedEdges.emplace_back(currentEdge);
			continue;
		}

		TopoDS_Wire straightCurve = CurveToCompound(currentEdge);

		for (TopExp_Explorer expl2(straightCurve, TopAbs_EDGE); expl2.More(); expl2.Next()) {
			TopoDS_Edge straightenedEdge = TopoDS::Edge(expl2.Current());
			fixedEdges.emplace_back(straightenedEdge);
		}
	}
	return fixedEdges;
}

bool helperFunctions::isStraight(const TopoDS_Edge& theEdge)
{
	Standard_Real first, last;
	Handle(Geom_Curve) curve = BRep_Tool::Curve(theEdge, first, last);

	if (curve.IsNull()) {
		return false;
	}

	Handle(Geom_Line) line = Handle(Geom_Line)::DownCast(curve);
	return !line.IsNull();
}

bool helperFunctions::hasVolume(const bg::model::box<BoostPoint3D>& bbox)
{
	double precision = SettingsCollection::getInstance().linearTolerance();

	const auto& t1 = bbox.min_corner();
	const auto& t2 = bbox.max_corner();
	if (abs(t1.get<0>() - t2.get<0>()) < precision &&
		abs(t1.get<1>() - t2.get<1>()) < precision &&
		abs(t1.get<2>() - t2.get<2>()) < precision)
	{
		return false;
	}
	return true;
}

bool helperFunctions::isSame(const bg::model::box<BoostPoint3D>& bboxL, const bg::model::box<BoostPoint3D>& bboxR)
{
	BoostPoint3D minPointL = bboxL.min_corner();
	BoostPoint3D maxPointL = bboxL.max_corner();
	BoostPoint3D minPointR = bboxR.min_corner();
	BoostPoint3D maxPointR = bboxR.max_corner();

	if (!pointIsSame(minPointL, minPointR)) { return false; }
	if (!pointIsSame(maxPointL, maxPointR)) { return false; }
	return true;
}

bool helperFunctions::isSame(const TopoDS_Face& faceL, const TopoDS_Face& faceR)
{
	double precision = SettingsCollection::getInstance().linearTolerance();
	double areaTolerance = SettingsCollection::getInstance().areaTolerance();

	if (abs(computeArea(faceL) - computeArea(faceR)) > areaTolerance)
	{
		return false;
	}
	std::vector<gp_Pnt> uniqueLPoints = getUniquePoints(faceL);
	std::vector<gp_Pnt> uniqueRPoints = getUniquePoints(faceR);

	if (uniqueLPoints.size() != uniqueRPoints.size())
	{
		return false;
	}

	for (const gp_Pnt& uniqueLPoint : uniqueLPoints)
	{
		bool pointFound = false;
		for (const gp_Pnt& uniqueRPoint : uniqueRPoints)
		{
			if (uniqueLPoint.IsEqual(uniqueRPoint, precision))
			{
				pointFound = true;
				break;
			}
		}
		if (!pointFound)
		{
			return false;
		}
	}
	return true;
}


int helperFunctions::wireCount(const TopoDS_Face& theFace)
{
	int count = 0;
	for (TopExp_Explorer WireExpl(theFace, TopAbs_WIRE); WireExpl.More(); WireExpl.Next())
	{
		count++;
	}
	return count;
}

std::vector<TopoDS_Face> helperFunctions::removeDubFaces(const std::vector<TopoDS_Face>& inputFaceList, bool fullProcessing)
{
	std::vector<TopoDS_Face> cleanedFaceList;
	std::vector<double> areaList;

	double minArea = SettingsCollection::getInstance().areaTolerance();

	bgi::rtree<Value, bgi::rstar<25>> spatialIndex;
	for (const TopoDS_Face& currentFace : inputFaceList)
	{
		if (computeArea(currentFace) < minArea) { continue; }

		std::vector<Value> qResult;
		qResult.clear();
		bg::model::box <BoostPoint3D> bbox = helperFunctions::createBBox(currentFace);
		spatialIndex.query(bgi::intersects(
			bbox), std::back_inserter(qResult));

		bool isDub = false;
		for (const auto&  [otherBox, faceIndx] : qResult)
		{
			const TopoDS_Face& otherFace = cleanedFaceList[faceIndx];
			if (!isSame(currentFace, otherFace)) {continue; }
			isDub = true;
			break;

		}
		if (isDub) { continue; }

		spatialIndex.insert(std::make_pair(bbox, cleanedFaceList.size()));
		cleanedFaceList.emplace_back(currentFace);
		areaList.emplace_back(computeArea(currentFace));
	}
	if (!fullProcessing) {return cleanedFaceList; }

	std::vector<TopoDS_Face> filteredFaceList;
	for (size_t i = 0; i < cleanedFaceList.size(); i++)
	{
		const TopoDS_Face& currentFace = cleanedFaceList[i];
		double currentArea = areaList[i];

		std::vector<Value> qResult;
		qResult.clear();
		bg::model::box <BoostPoint3D> bbox = helperFunctions::createBBox(currentFace, 0.0);
		spatialIndex.query(bgi::intersects(
			bbox), std::back_inserter(qResult));

		bool isSurrounded = false;
		for (const auto& [otherBox, faceIndx] : qResult)
		{
			if (i == faceIndx) { continue; }
			double otherArea = areaList[faceIndx];
			TopoDS_Face otherFace = cleanedFaceList[faceIndx];

			if (currentArea > otherArea) { continue; }
			isSurrounded = true;
			for (TopExp_Explorer expl(currentFace, TopAbs_VERTEX); expl.More(); expl.Next())
			{
				TopoDS_Vertex vertex = TopoDS::Vertex(expl.Current());
				gp_Pnt p = BRep_Tool::Pnt(vertex);
				if (!pointOnShape(otherFace, p))
				{
					isSurrounded = false;
					break;
				}
			}

			if (!isSurrounded) { continue; }

			std::vector<gp_Pnt> pointFaceList = helperFunctions::getPointListOnFace(currentFace);
			for (const gp_Pnt& currentPoint : pointFaceList)
			{
				if (pointOnShape(otherFace, currentPoint)) { continue; }
				isSurrounded = false;
				break;
			}

			if (isSurrounded) { break; }
		}
		if (isSurrounded) { continue; }
		filteredFaceList.emplace_back(currentFace);
	}
	return filteredFaceList;
}

bool helperFunctions::isFlat(const TopoDS_Face& theFace)
{
	BRepAdaptor_Surface surface = BRepAdaptor_Surface(theFace);
	if (surface.GetType() == GeomAbs_Plane)
	{
		return true;
	}
	else
	{
		return false;
	}
}

bool helperFunctions::isFlat(const TopoDS_Shape& theShape)
{
	for (TopExp_Explorer explorer(theShape, TopAbs_FACE); explorer.More(); explorer.Next())
	{
		const TopoDS_Face& currentFace = TopoDS::Face(explorer.Current());
		if (!isFlat(currentFace)) { return false; }
	}
	return true;
}

bool helperFunctions::isStraight(const TopoDS_Shape& theShape)
{
	for (TopExp_Explorer expl(theShape, TopAbs_EDGE); expl.More(); expl.Next()) {
		TopoDS_Edge currentEdge = TopoDS::Edge(expl.Current());

		if (!isStraight(currentEdge)) {
			return false;
		}
	}
	return true;
}

bool helperFunctions::containsSolid(const TopoDS_Shape& theShape)
{
	if (theShape.ShapeType() == TopAbs_SOLID) { return true; }

	for (TopoDS_Iterator it(theShape); it.More(); it.Next())
	{
		const TopoDS_Shape& child = it.Value();
		if (child.ShapeType() == TopAbs_SOLID) { return true; }


		if (child.ShapeType() != TopAbs_COMPOUND) { continue; }
		if (containsSolid(child)) { return true; }
	}
	return false;
}


template <typename T>
std::vector<T> helperFunctions::sortShapes(const std::vector<T>& shapeList, const std::vector<double>& sortingValues)
{
	// sort the surfaces 
	if (shapeList.size() < 2) { return shapeList; }
	if (sortingValues.size() != shapeList.size()) { return {}; }

	std::vector<int> indices(shapeList.size());
	std::iota(indices.begin(), indices.end(), 0);
	std::sort(indices.begin(), indices.end(),
		[&](int A, int B) -> bool {
			return sortingValues[A] < sortingValues[B];
		});

	std::vector<T> sortedShapeColl;
	for (int currentIndx : indices)
	{
		sortedShapeColl.emplace_back(shapeList[currentIndx]);
	}
	return sortedShapeColl;
}

void helperFunctions::devideFaces(const TopoDS_Shape& inputShape, std::vector<TopoDS_Face>* horizontalFaces, std::vector<TopoDS_Face>* verticalFaces)
{
	double angularTolerance = SettingsCollection::getInstance().angularTolerance();
	for (TopExp_Explorer explorer(inputShape, TopAbs_FACE); explorer.More(); explorer.Next())
	{
		const TopoDS_Face& currentFace = TopoDS::Face(explorer.Current());

		std::vector<TopoDS_Face> tesselatedFaceList = helperFunctions::TessellateFace(currentFace);
		for (const TopoDS_Face& tesselatedFace : tesselatedFaceList)
		{
			if (abs(helperFunctions::computeFaceNormal(tesselatedFace).Z()) < angularTolerance)
			{
				horizontalFaces->emplace_back(tesselatedFace);
				continue;
			}
			verticalFaces->emplace_back(tesselatedFace);
		}
	}
	return;
}

TopoDS_Shape helperFunctions::addSolidSemantic(const TopoDS_Shape& assumedSolid)
{
	BRep_Builder builder;
	TopoDS_Shell shell;
	builder.MakeShell(shell);

	BRepBuilderAPI_Sewing brepSewer;

	for (TopExp_Explorer faceExpl(assumedSolid, TopAbs_FACE); faceExpl.More(); faceExpl.Next())
	{
		TopoDS_Face currentFace = TopoDS::Face(faceExpl.Current());
		if (currentFace.IsNull()) { continue; }
		brepSewer.Add(currentFace);
	}

	brepSewer.Perform();
	TopoDS_Shape sewedShape = brepSewer.SewedShape();

	if (sewedShape.IsNull())
	{
		return assumedSolid;
	}

	if (sewedShape.Closed() && sewedShape.ShapeType() == TopAbs_SHELL)
	{
		TopoDS_Shell shell = TopoDS::Shell(sewedShape);
		TopoDS_Solid solidrep;
		builder.MakeSolid(solidrep);
		builder.Add(solidrep, shell);
		if (!solidrep.Closed()) { return assumedSolid; }
		if (solidrep.IsNull()) { return assumedSolid; }

		BRepLib::OrientClosedSolid(solidrep);
		return solidrep;
	}
	return assumedSolid;
}

bool helperFunctions::face2Plane(const TopoDS_Face& theFace, gp_Pln* thePlane)
{
	// if not flat project to original plane
	Handle(Geom_Surface) currentSurf = BRep_Tool::Surface(theFace);
	if (currentSurf.IsNull()) { return false; }
	Handle(Geom_Plane) currentPlane = Handle(Geom_Plane)::DownCast(currentSurf);
	if (currentPlane.IsNull()) { return false; }
	gp_Pln plane = currentPlane->Pln();

	*thePlane = plane;
	return true;
}

void helperFunctions::updateCounter(
	const std::string& prefixText,
	int totalObjects,
	int& processedObject,
	std::mutex& listmutex,
	bool endLine
)
{
	bool running = true;
	while (running)
	{
		std::unique_lock<std::mutex> listlock(listmutex);
		int currentObjectCount = processedObject;
		listlock.unlock();

		std::cout
			<< "\t" << prefixText << " - " << currentObjectCount << " of " << totalObjects << "      \r";

		if (currentObjectCount == totalObjects)
		{
			break;
		}

		if (running)
		{
			std::this_thread::sleep_for(std::chrono::seconds(1));
		}
	}
	if (endLine)
	{
		std::cout << "\n";
	}
	
	return;
}

HalfEdge::HalfEdge(const TopoDS_Edge& theEdge)
{
	p1_ = helperFunctions::getFirstPointShape(theEdge);
	p2_ = helperFunctions::getLastPointShape(theEdge);
}
