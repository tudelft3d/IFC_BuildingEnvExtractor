#define USE_IFC4x2
#define iterationVersion "0.4.2"

#ifdef USE_IFC2x3
#include <ifcparse/Ifc2x3.h>
#define IfcSchema Ifc2x3
#define buildVersion "IFC2X3"
#define SCHEMA_VERSIONS (2x3)
#define SCHEMA_SEQ (2x3)

#elif defined(USE_IFC4x1)
#include <ifcparse/Ifc4x1.h>
#define IfcSchema Ifc4x1
#define buildVersion "IFC4X1"
#define SCHEMA_VERSIONS (4x1)
#define SCHEMA_SEQ (4x1)

#elif defined(USE_IFC4x2)
#include <ifcparse/Ifc4x2.h>
#define IfcSchema Ifc4x2
#define buildVersion "IFC4X2"
#define SCHEMA_VERSIONS (4x2)
#define SCHEMA_SEQ (4x2)

#elif defined(USE_IFC4)
#include <ifcparse/Ifc4.h>
#define IfcSchema Ifc4
#define buildVersion "IFC4"
#define SCHEMA_VERSIONS (4)
#define SCHEMA_SEQ (4)

#elif defined(USE_IFC4x3)
#include <ifcparse/Ifc4x3.h>
#define IfcSchema Ifc4x3
#define buildVersion "IFC4X3"
#define SCHEMA_VERSIONS (4x3)
#define SCHEMA_SEQ (4x3)

#elif defined(USE_IFC4x3add1)
#include <ifcparse/Ifc4x3_add1.h>
#define IfcSchema Ifc4x3_add1
#define buildVersion "IFC4X3_ADD1"
#define SCHEMA_VERSIONS (4x3 add1)
#define SCHEMA_SEQ (4x3)

#elif defined(USE_IFC4x3add2)
#include <ifcparse/Ifc4x3_add2.h>
#define IfcSchema Ifc4x3_add2
#define buildVersion "IFC4X3_ADD2"
#define SCHEMA_VERSIONS (4x3 add2)
#define SCHEMA_SEQ (4x3)

#else
#error "No IFC version defined"
#endif // USE_IFC

#include "surfaceCollection.h"

// IfcOpenShell includes
#include <ifcparse/IfcFile.h>
#include <ifcgeom_schema_agnostic/Kernel.h>
#include <ifcgeom_schema_agnostic/Serialization.h>
#include <ifcparse/IfcHierarchyHelper.h>

// OpenCascade includes
#include <gp_Vec2d.hxx>
#include <gp_Vec.hxx>
#include <TopoDS.hxx>
#include <Poly_Triangulation.hxx>
#include <nlohmann/json.hpp>

#include <CJT.h>
#include <CJToKernel.h>

#include <chrono>
#include <unordered_set>
#include <memory>

namespace bg = boost::geometry;
namespace bgi = boost::geometry::index;

typedef bg::model::point<double, 3, bg::cs::cartesian> BoostPoint3D;
typedef bg::model::box<BoostPoint3D> BoostBox3D;
typedef std::pair<bg::model::box<BoostPoint3D>, int> Value;

#ifndef HELPER_HELPER_H
#define HELPER_HELPER_H

struct FaceComplex { //TODO: find better location for this
	std::vector<TopoDS_Face> faceList_;
};

struct HalfEdge
{
	gp_Pnt p1_;
	gp_Pnt p2_;

	bool isUsed_ = false;

	HalfEdge(const gp_Pnt& p1, const gp_Pnt& p2) {
		p1_ = p1; p2_ = p2;
	}

	HalfEdge(const TopoDS_Edge& theEdge);

	const gp_Pnt& getP1() const { return p1_; }
	const gp_Pnt& getP2() const { return p2_; }


	bool operator== (const HalfEdge& other) {
		if (!other.p1_.IsEqual(p1_, 1e-6)) { return false; }
		if (!other.p2_.IsEqual(p2_, 1e-6)) { return false; }
		return true;
	}

	gp_Vec getDir() const { return gp_Vec(p1_, p2_).Normalized(); }

	bool isPartner(const HalfEdge& otherEdge) const { return (otherEdge.p1_.IsEqual(p2_, 1e-6) && otherEdge.p2_.IsEqual(p1_, 1e-6)); }
	bool isSame(const HalfEdge& otherEdge) const { return (otherEdge.p1_.IsEqual(p1_, 1e-6) && otherEdge.p2_.IsEqual(p2_, 1e-6)); }
};

struct HalfEdgeLoop
{
	std::vector<HalfEdge> halfEdgeList_;

	HalfEdgeLoop(const std::vector<HalfEdge>& HalfEdgeList) { halfEdgeList_ = HalfEdgeList; }

	int size() const { return halfEdgeList_.size(); }

	double zHeight() const {
		
		if (halfEdgeList_.empty())
		{
			return 0;
		}
		return halfEdgeList_[0].p1_.Z(); 
	}

	TopoDS_Wire getWire() const {
		BRepBuilderAPI_MakeWire wireBuilder;
		for (const HalfEdge& currentEdge : halfEdgeList_) {
			TopoDS_Edge segment = BRepBuilderAPI_MakeEdge(currentEdge.p1_, currentEdge.p2_);
			wireBuilder.Add(segment);
		}
		wireBuilder.Build();
		return wireBuilder.Wire();
	}
};

// helper functions that can be utilised everywhere
struct helperFunctions{
	
	/// point managing functions
	
	/// Convert OpenCascade point to Boost point
	static BoostPoint3D Point3DOTB(const gp_Pnt& oP);
	/// Conver Boost point to OpenCascade point
	static gp_Pnt Point3DBTO(const BoostPoint3D& oP);
	/// get a list of unique points from a shape
	static std::vector<gp_Pnt> getUniquePoints(const TopoDS_Shape& inputShape);
	/// get a list of points from a shape
	static std::vector<gp_Pnt> getPoints(const TopoDS_Shape& shape);
	/// get the total count of unique points 
	static int getPointCount(const TopoDS_Shape& inputShape);
	///	Rotate OpenCascade point around 0,0,0
	static gp_Pnt rotatePointWorld(const gp_Pnt& p, double angle);
	///	Rotate Boost point around 0,0,0
	static BoostPoint3D rotatePointWorld(const BoostPoint3D& p, double angle);
	/// Rotate OpenCascade point p around point anchorP
	static gp_Pnt rotatePointPoint(const gp_Pnt& p, const gp_Pnt& anchorP, const double& angle);
	// get a grid of points that are placed over a surface
	static std::vector<gp_Pnt> getPointGridOnSurface(const TopoDS_Face& theface, const double& resolution);
	// get a grid of points that are placed alongside the wire on a face (not threadsave)
	static std::vector<gp_Pnt> getPointGridOnWire(const TopoDS_Face& theface, const double& resolution);
	/// check if points are equal
	static bool pointIsSame(const BoostPoint3D& lp, const BoostPoint3D& rp);

	/// bounding box creating code

	/// get the lllpoint and urr point of a list of TopoDS shape
	/// the rotation values will rotate the shape before creating the points of the box, afterwards the box needs to be rotated back to represent that actual bounding box
	template<typename T>
	static void bBoxDiagonal(const std::vector<T>& theShapeList, gp_Pnt* lllPoint, gp_Pnt* urrPoint, const double buffer = 0, const double angle = 0, const double secondAngle = 0);
	/// get the lllpoint and urr point of an TopoDS shape
	/// the rotation values will rotate the shape before creating the points of the box, afterwards the box needs to be rotated back to represent that actual bounding box
	template<typename T>
	static void bBoxDiagonal(const T& theShape, gp_Pnt* lllPoint, gp_Pnt* urrPoint, const double buffer = 0, const double angle = 0, const double secondAngle = 0);
	/// get the lllpoint and urr point of list of points
	/// the rotation values will rotate the shape before creating the points of the box, afterwards the box needs to be rotated back to represent that actual bounding box
	static bool bBoxDiagonal(const std::vector<gp_Pnt>& pointList, gp_Pnt* lllPoint, gp_Pnt* urrPoint, const double buffer = 0, const double angle = 0, const double secondAngle = 0);
	/// construct the smallest orientated bounding box
	static void bBoxOrientated(const std::vector<gp_Pnt>& pointList, gp_Pnt* lllPoint, gp_Pnt* urrPoint, double* rotationAngle, const double buffer = 0);
	/// construct a bbox from a shape 
	static bg::model::box <BoostPoint3D> createBBox(const TopoDS_Shape& shape, double buffer = 0.0);
	/// construct a bbox from a list of shapes
	static bg::model::box <BoostPoint3D> createBBox(const std::vector<TopoDS_Shape>& shape, double buffer = 0.0);
	/// construct a bbox from a list of points
	static bg::model::box <BoostPoint3D> createBBox(const std::vector<gp_Pnt>& pointList, double buffer = 0.0);
	/// construct a bbox from a list of points
	static bg::model::box <BoostPoint3D> createBBox(const std::array<gp_Pnt, 3>& pointList, double buffer = 0.0);
	/// construct a bbox from the urr and lll points
	static bg::model::box <BoostPoint3D> createBBox(const gp_Pnt& p1, const gp_Pnt& p2, double buffer = 0.0);
	/// construct a OCCTbbox from the urr and lll points
	static TopoDS_Shape createBBOXOCCT(const gp_Pnt& p1, const gp_Pnt& p2, double buffer = 0.0, double horizontalAngle = 0.0, double verticalAngle = 0.0);
	/// construct a Boostbbox from the urr and lll points (can not be rotated)
	/// simplefies shape by creating a smallest bbox around it that is fully orientated
	static TopoDS_Shape boxSimplefyShape(const TopoDS_Shape& shape);
	/// applies the buffer values to the lll and urr point
	static void applyBuffer(gp_Pnt* lllPoint, gp_Pnt* urrPoint, double buffer = 0.0);


	/// Height (z) computing code

	/// Get the lowest Z value of a shape. If areaFilter = true the lowest point of the largest face is taken
	template<typename T>
	static double getLowestZ(const T& shape);
	/// Get the highest Z value of a shape. If areaFilter = true the lowest point of the largest face is taken
	template<typename T>
	static double getHighestZ(const T& shape);
	/// Get the highest Z value of a list of shapes. If areaFilter = true the lowest point of the largest face is taken
	template<typename T>
	static double getHighestZ(const std::vector<T>& faceList);
	/// get the average height of a shape, computed by taking the average height of all the object's vertices
	template<typename T>
	static double getAverageZ(const T& shape);
	/// get the first vertex height of a shape
	template<typename T>
	static double getAZ(const T& shape);

	/// point on shape code

	/// get the middle of a triangle
	static gp_Pnt getTriangleCenter(const Handle(Poly_Triangulation)& mesh, const Poly_Triangle& theTriangle, const TopLoc_Location& loc);
	/// get the middlepoint of the face located in the first triangle of its triangulation
	static std::optional<gp_Pnt> getPointOnFace(const TopoDS_Face& theFace);
	/// get the middlepoint list of the face triangulation
	static std::vector<gp_Pnt> getPointListOnFace(const TopoDS_Face& theFace);
	/// get first point on shape (used for wires and edges)
	static gp_Pnt getFirstPointShape(const TopoDS_Shape& shape);
	/// get last point on shape (used for wires and edges)
	static gp_Pnt getLastPointShape(const TopoDS_Shape& shape);
	// check if point is in (solid) shape
	static bool pointInShape(const TopoDS_Shape& shape, const gp_Pnt& thePoint, double precision = 0.0);
	/// check if point is on shape
	static bool pointOnShape(const TopoDS_Shape& shape, const gp_Pnt& thePoint, double precision = 0.0);
	/// check if point is on face
	static bool pointOnFace(const TopoDS_Face& theFace, const gp_Pnt& thePoint, double precision = 0.0);
	/// check if point is on face
	static bool pointOnFace(const std::vector<TopoDS_Face>& theFace, const gp_Pnt& thePoint, double precision = 0.0);
	/// check if point is on mesh
	static bool pointOnMesh(const Handle(Poly_Triangulation)& theMesh, const TopLoc_Location& loc, const gp_Pnt& thePoint, double precision = 0.0);
	/// check if point is on triangle p1p2p3
	static bool pointOnTriangle(const gp_Pnt& thePoint, const gp_Pnt& p1, const gp_Pnt& p2, const gp_Pnt& p3);
	/// check if point is on triangle p1p2p3
	static bool pointOnTriangle(const gp_Pnt& thePoint, const gp_Pnt& p1, const gp_Pnt& p2, const gp_Pnt& p3, const gp_Vec& normal);
	/// check if 2D point is on 2D triangle p1p2p3
	static bool pointOnTriangle(const gp_Pnt2d& thePoint, const gp_Pnt2d& p1, const gp_Pnt2d& p2, const gp_Pnt2d& p3);
	/// check if uv/2D point is on uv range of mesh
	static bool uvPointOnMesh(const gp_Pnt2d& thePoint, const TColgp_Array1OfPnt2d& uvNodes, const Poly_Array1OfTriangle& triangles);
	/// check if point is on any edge of face
	static bool pointOnWire(const TopoDS_Face& theFace, const gp_Pnt& thePoint, double precision = 0.0);
	/// check if point is on any edge of wire
	static bool pointOnWire(const TopoDS_Wire& theWire, const gp_Pnt& thePoint, double precision = 0.0);
	/// check if point is on edge
	static bool pointOnEdge(const TopoDS_Edge& theEdge, const gp_Pnt& thePoint, double precision = 0.0);
	
	/// direction and angle code

	/// compute the edge dir 
	static gp_Vec computeEdgeDir(const TopoDS_Edge& theEdge);
	/// compute the face normal
	template<typename T>
	static gp_Vec computeFaceNormal(const T& theFace);
	/// compute normal of a pool of points in order representing a ring of edges
	static gp_Vec newellsNormal(const std::vector<gp_Pnt>& pointList);
	/// compute the smallest angle of the edges, returns 0 if not found
	static double computeSmallestAngle(const TopoDS_Face& theFace);
	/// compute the horizontal dir based on vector count
	static gp_Vec getShapedir(const std::vector<gp_Pnt>& pointList, bool isHorizontal);

	/// overlapping object code

	/// checks if two faces share and edge (if they are resting against eachother)
	static bool shareEdge(const TopoDS_Face& theFace, const TopoDS_Face& theotherFace);
	/// check if edges overlap by checking the endpoints triangular distance
	static bool edgeEdgeOVerlapping(const TopoDS_Edge& currentEdge, const TopoDS_Edge& otherEdge);
	/// check if edges are the same by checking their endpoints
	static bool edgeEdgeAreSame(const TopoDS_Edge& currentEdge, const TopoDS_Edge& otherEdge);
	/// check if upperface overlaps the lower face by checking the edges
	static bool faceFaceOverlapping(const TopoDS_Face& upperFace, const TopoDS_Face& lowerFace);
	/// checks if surface is encapsulated by another shape
	static bool surfaceIsIncapsulated(const TopoDS_Face& innerSurface, const TopoDS_Face& outerSurface);
	/// checks if surface is encapsulated by other faces
	static bool surfaceIsIncapsulated(const TopoDS_Face& innerSurface, const std::vector<TopoDS_Face>& outerSurfaceList, bool ignoreSelf = false);

	/// line surface intersection related code

	/// check if line intersects triangle
	static bool triangleIntersecting(const std::array<gp_Pnt, 2>& line, const std::array<gp_Pnt, 3>& triangle);
	/// check if line intersects triangle
	static bool triangleIntersecting(const std::array<gp_Pnt, 2>& line, const std::array<gp_Pnt, 3>& triangle, const gp_Vec& triangleNormal);
	/// test if point falls within the triangle
	static bool baryCentricTest(const gp_Pnt& point, const std::array<gp_Pnt, 3>& triangle);

	/// surface line intersection related code

	/// check if a line intersects with a face
	static bool LineShapeIntersection(const TopoDS_Face& theFace, const gp_Pnt& lP1, const gp_Pnt& lp2, bool inZdir = false);

	// surface and wire merging code

	/// merges the input wires in the correct order
	static TopoDS_Wire mergeWireOrientated(const TopoDS_Wire& baseWire, const TopoDS_Wire& mergingWire);
	/// merge faces that rest against eachother
	static std::vector<TopoDS_Face> mergeFaces(const std::vector<TopoDS_Face>& theFaceList, bool communicate = false);
	/// attempts to close an open wire
	static TopoDS_Wire closeWireOrientated(const TopoDS_Wire& baseWire);

	// face creation code

	/// creates a planar face between lll and urr with a rotation
	static TopoDS_Face createHorizontalFace(const gp_Pnt& lll, const gp_Pnt& urr, double rotationAngle, double z);
	/// creates a planar face by connecting the 4 points, make sure the 4 points are on a single plane
	static TopoDS_Face createPlanarFace(const gp_Pnt& p0, const gp_Pnt& p1, const gp_Pnt& p2, const gp_Pnt& p3);
	/// creates a planar face by connecting the 3 points, make sure the 3 points are on a single plane
	static TopoDS_Face createPlanarFace(const gp_Pnt& p0, const gp_Pnt& p1, const gp_Pnt& p2);
	/// creates a planar copy of input face at input height
	static TopoDS_Face projectFaceFlat(const TopoDS_Face& theFace, double height);
	/// creates a copy of the input face at the reference plane
	static TopoDS_Face projectFace(const TopoDS_Face& theFace, const gp_Pln& theReferencePlane);
	/// creates a planar copy of the input wire
	static TopoDS_Wire projectWireFlat(const TopoDS_Wire& theWire, double height);

	/// creates a clean shape of the input shape with no non-straight curves
	static TopoDS_Shape TesselateShape(const TopoDS_Shape& theShape);
	/// creates a clean copy of the input face with no non-straight curves
	static std::vector<TopoDS_Face> TessellateFace(const TopoDS_Face& theFace, bool knownIsFlat = false);
	/// creates a clean copy of the input face list with no non-straight curves
	static std::vector<TopoDS_Face> TessellateFace(const std::vector<TopoDS_Face>& theFaceList, bool knownIsFlat = false);
	/// creates a clean mesh approximation of the input face
	static std::vector<TopoDS_Face> TriangulateFace(const TopoDS_Face& theFace);
	/// creates a clean mesh approximation of the input facelist
	static std::vector<TopoDS_Face> TriangulateFace(const std::vector<TopoDS_Face>& theFaceList);
	/// fixes face if face is broken
	static bool fixFace(TopoDS_Face* theFace);

	/// @brief grows wires from unordered exterior edges
	static std::vector<TopoDS_Wire> growWires(const std::vector<TopoDS_Edge>& edgeList);

	/// @brief cleans the wires (removes redundant vertex)
	static std::vector<TopoDS_Wire> cleanWires(const std::vector<TopoDS_Wire>& wireList);
	static TopoDS_Wire cleanWire(const TopoDS_Wire& wire);
	static TopoDS_Face wireCluster2Faces(const std::vector<TopoDS_Wire>& wireList);

	/// trim argument face down to tool faces extents
	static std::vector<TopoDS_Face> TrimFaceToFace(const TopoDS_Face& argumentFace, const std::vector<TopoDS_Face>& toolFaceList, double faceHeight = 0);

	// planar simplification code

	/// creates face collection that represent the merged input shapes
	static std::vector<TopoDS_Face> planarFaces2Outline(const std::vector<TopoDS_Face>& planarFaces);
	/// fuses all the planar faces into a complex planar face structure
	static std::vector<TopoDS_Shape> planarFaces2Cluster(const std::vector<TopoDS_Face>& planarFaces); //TODO: i want this removed
	/// creates a cluster of non-intersecting and non-overlapping edges
	static std::vector<HalfEdge> planarFaces2EdgeCluster(const std::vector<TopoDS_Face>& planarFaces);
	/// creates the index for the edgecluster creation process, ignores meshing double edges 
	static bgi::rtree<std::pair<BoostBox3D, HalfEdge>, bgi::rstar<25>> makeEdgeClusterIndx(const std::vector<TopoDS_Face>& planarFaces);
	/// create loops or of a planar edge cluster
	static std::vector<HalfEdgeLoop> planarEdgeCluster2Loops(const std::vector<HalfEdge>& planarEdgeCluster);
	/// eliminate the non-vital loops
	static std::vector<HalfEdgeLoop> loops2Outer(const std::vector<HalfEdgeLoop>& planarLoopList, const std::vector<TopoDS_Face>& planarFaces);
	/// construct planar faces from the outerLoops
	static std::vector<TopoDS_Face> outerLoops2Faces(const std::vector<HalfEdgeLoop>& outerLoopList);
	/// split HalfEdge in multipleHalfEdges
	static std::vector<HalfEdge> splitHalfEdge(const HalfEdge& argument, const std::vector<HalfEdge>& toolList);
	/// split HalfEdge with another HalfEdge
	static bool splitHalfEdge(const HalfEdge& argument, const HalfEdge& tool, double precision, double& t);

	/// IFC related code

	/// get the z value stored in the IfcObjectPlacement 
	static double getObjectZOffset(IfcSchema::IfcObjectPlacement* objectPlacement, bool deepOnly);
	/// evaluates if product has glass material related to it
	static bool hasGlassMaterial(const IfcSchema::IfcProduct* ifcProduct);
	/// evaluates if product has the IsExternal attribute set to true
	static bool isExternal(const IfcSchema::IfcProduct* ifcProduct);
	/// returns all the attributes in json format
	static nlohmann::json getAttributes(const IfcSchema::IfcProduct* ifcProduct, const std::string& PsetName = "");
	/// returns all the attributes in json format
	static nlohmann::json getAttributes(const IfcSchema::IfcPropertySet& pset);

	/// write to file code

	/// write shape to step
	static void writeToSTEP(const TopoDS_Shape& theShape, const std::string& targetPath);
	/// write list of shapes to step
	template <typename T>
	static void writeToSTEP(const std::vector<T>& theShapeList, const std::string& targetPath);
	/// write list of shapes to step
	template <typename T>
	static void writeToSTEP(const std::vector<std::vector<T>>& theShapeList, const std::string& targetPath);
	
	/// write shape to obj
	template <typename T>
	static void writeToOBJ(const T& theShape, const std::string& targetPath);
	/// write list of shapes to obj
	template <typename T>
	static void writeToOBJ(const std::vector<T>& theShapeList, const std::string& targetPath);
	/// write list of shapes to obj
	template <typename T>
	static void writeToOBJ(const std::vector<std::vector<T>>& theShapeList, const std::string& targetPath);

	/// triangulation related code

	/// get a nested list represeting the triangulation of an object
	static void triangulateShape(const TopoDS_Shape& shape, bool force = false);

	/// approximates a curved edge with straight segmented approximation
	static TopoDS_Wire CurveToCompound(const TopoDS_Edge& theEdge);
	/// replaces curves from wires with straight segmented approximation
	static TopoDS_Wire replaceCurves(const TopoDS_Wire& theWire);
	/// replaces curves with straight segmented approximation
	static std::vector<TopoDS_Edge> replaceCurves(const std::vector<TopoDS_Edge>& theEdgeList);

	/// return true if the edge is a straight line
	static bool isStraight(const TopoDS_Edge& theEdge);

	/// other code

	/// get the total amount of faces in the shape
	static int getFaceCount(const TopoDS_Shape& inputShape);

	// check if face is valid
	static bool faceIsValid(const TopoDS_Face& theFace);

	/// outputs the time delta between the start and end time
	static void printTime(std::chrono::steady_clock::time_point startTime, std::chrono::steady_clock::time_point endTime);
 
	/// compute the area of a face
	static double computeArea(const TopoDS_Face& theFace);

	/// compute the area of a wire with a shoestring method.
	static double computeArea(const TopoDS_Wire& theWire);

	/// count the number of wires in a face
	static int wireCount(const TopoDS_Face& theFace);

	/// checks if a bbox has volume
	static bool hasVolume(const bg::model::box <BoostPoint3D>& bbox);
	/// checks if two bbox are equal
	static bool isSame(const bg::model::box <BoostPoint3D>& bboxL, const bg::model::box <BoostPoint3D>& bboxR);
	/// checks if two face have the same geo
	static bool isSame(const TopoDS_Face& faceL, const TopoDS_Face& faceR);
	/// returns list of unique faces where dubs have been removed
	static std::vector<TopoDS_Face> removeDubFaces(const std::vector<TopoDS_Face>& inputFaceList, bool fullProcessing = false);

	/// checks if the face is flat
	static bool isFlat(const TopoDS_Face& theFace);
	/// checks if all the faces in the shape are flat
	static bool isFlat(const TopoDS_Shape& theShape);

	/// check if all the edges in a shape are straight
	static bool isStraight(const TopoDS_Shape& theShape);
	/// check if the shape or its children (recursive) are or contain a solid object
	static bool containsSolid(const TopoDS_Shape& theShape);

	/// sorts the list of faces based on the sorting values
	template <typename T>
	static std::vector<T> sortShapes(const std::vector<T>& shapeList, const std::vector<double>& sortingValues);

	/// devide horizontal and vertical faces in seperate lists
	static void devideFaces(const TopoDS_Shape& inputShape, std::vector<TopoDS_Face>* horizontalFaces, std::vector<TopoDS_Face>* verticalFaces);

	/// forces solid semantic data on a solid shape that is missing it
	static TopoDS_Shape addSolidSemantic(const TopoDS_Shape& assumedSolid);

	/// creates a planar face based on the input face
	static bool face2Plane(const TopoDS_Face& theFace, gp_Pln* thePlane);

	/// monitor the progress of multithreaded code
	static void updateCounter(
		const std::string& prefixText,
		int totalObjects,
		int& processedObject,
		std::mutex& listmutex,
		bool endLine = true
	);

};
#endif // HELPER_HELPER_H