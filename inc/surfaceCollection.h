
#ifndef SURFACECOLLECTION_SURFACECOLLECTION_H
#define SURFACECOLLECTION_SURFACECOLLECTION_H

#include "helper.h"

#include <vector>

#include <gp_Pnt.hxx>
#include <gp_Pnt2d.hxx>
#include <TopoDS_Edge.hxx>
#include <gp_Lin.hxx>
#include <TopoDS_Face.hxx>
#include <gp_Vec.hxx>
#include <TopoDS_Wire.hxx>
#include <memory>


class EvaluationPoint {
private:
	gp_Pnt thePoint_;

	TopoDS_Edge evalEdge_;
	gp_Lin evalLin_;

	bool isVisible_ = true;

public:
	EvaluationPoint(const gp_Pnt& p);
	bool isVisible() const { return isVisible_; }
	void setInvisible() { isVisible_ = false; }

	const gp_Pnt getPoint() const { return thePoint_; }
	const TopoDS_Edge& getEvalEdge() const { return evalEdge_; }
	const gp_Lin& getEvalLine() const { return evalLin_; }
};


// contains the data of a single TopoDS_Face with its matching grid
class SurfaceGridPair {
private: 
	TopoDS_Face theFace_;

	// bounding box 3D data
	gp_Pnt lllPoint_;
	gp_Pnt urrPoint_;

	// information about the face
	double topHeight_;
	double avHeight_;
	int vertCount_ = 0;

	// usability information
	bool visibility_ = true;
	bool isSmall_ = false;

	std::vector<EvaluationPoint> pointGrid_;
	bool overlap(const SurfaceGridPair& other);

	void Merge(const std::vector<SurfaceGridPair>& otherPairList, const TopoDS_Face& theCompleteFace);

public:
	SurfaceGridPair(const TopoDS_Face& theFace);

	const TopoDS_Face getFace() const { return theFace_; }

	gp_Pnt getLLLPoint() const { return lllPoint_; }
	gp_Pnt getURRPoint() const { return urrPoint_; }

	double getAvHeight() const { return avHeight_; }
	double getTopHeight() const { return topHeight_; }

	int getVertCount() { return vertCount_; }

	std::vector<EvaluationPoint>* getPointGridPtr() { return &pointGrid_; }

	bool isVisible() const { return visibility_; }
	void setIsHidden() { visibility_ = false; }

	void populateGrid(double distance);

	bool testIsVisable(const std::vector<std::shared_ptr<SurfaceGridPair>>& otherSurfaces, bool preFilter = false);
	bool testIsVisable(const bgi::rtree<std::pair<BoostBox3D, std::shared_ptr<SurfaceGridPair>>, bgi::rstar<25>>& otherSurfacesIndx, bool preFilter = false);
};


class RCollection {
private:
	// the faces
	std::vector<TopoDS_Face> theFaceCollection_;

	// the flat face geometry of the complete face collection for LoD13
	TopoDS_Face theFlatFace_;

	// bounding box 3D data
	gp_Pnt lllPoint_ = gp_Pnt(999999, 999999, 999999);
	gp_Pnt urrPoint_ = gp_Pnt(-999999, -999999, -999999);
public:
	RCollection(const std::vector<TopoDS_Face>& theFaceColletion);

	const std::vector<TopoDS_Face> getFaces() const { return theFaceCollection_; }
	const TopoDS_Face& getFlatFace() const { return theFlatFace_; }

	const TopoDS_Face getProjectedFace() const;

	const gp_Pnt getLLLPoint() { return lllPoint_; }
	const gp_Pnt getURRPoint() { return urrPoint_; }
};

class Edge {
private:
	TopoDS_Edge theEdge_;
	gp_Pnt startPoint_;
	gp_Pnt endPoint_;

	bool hasEdgeEval_ = false;
	bool isOuter_ = false;

public:
	explicit Edge(const TopoDS_Edge& edge);

	const TopoDS_Edge& getEdge() const { return theEdge_; }

	TopoDS_Edge* getEdgePtr() { return &theEdge_; }

	gp_Pnt getStart(bool projected) const;

	gp_Pnt getEnd(bool projected) const;
};
#endif // SURFACECOLLECTION_SURFACECOLLECTION_H