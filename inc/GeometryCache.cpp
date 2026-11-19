#include "helper.h"
#include "GeometryCache.h"
#include "settingsCollection.h"
#include "errorCollection.h"


gp_Trsf getTransformationBrepElement(const IfcGeom::BRepElement* element)
{
	const auto& m = element->transformation().data()->components();

	gp_Trsf trsf;
	trsf.SetValues(
		m(0, 0), m(0, 1), m(0, 2), m(0, 3),
		m(1, 0), m(1, 1), m(1, 2), m(1, 3),
		m(2, 0), m(2, 1), m(2, 2), m(2, 3)
	);
	return trsf;
}

void GeometryCacheItem::init()
{
	std::string productType = productPtr_->declaration().name();
	if (productType == "IfcDoor" || productType == "IfcWindow")
	{
		instancedGeometryPtr_->init(productPtr_->GlobalId(), true);
	}
	instancedGeometryPtr_->init(productPtr_->GlobalId(), false);

	// create bbox
	TopoDS_Shape currentShape = getGeo(true);

	try
	{
		boundingBox_ = helperFunctions::createBBox(currentShape, 0);
	}
	catch (const ErrorID&)
	{
		ErrorCollection::getInstance().addError(ErrorID::warningFailedObjectSimplefication, productPtr_->GlobalId());
		return;
	}
	if (!helperFunctions::hasVolume(boundingBox_))
	{
		ErrorCollection::getInstance().addError(ErrorID::warningFailedObjectSimplefication, productPtr_->GlobalId());
		return;
	}

	return;
}

TopoDS_Shape GeometryCacheItem::getGeo(bool transformed) const
{
	if (!transformed)
	{
		return instancedGeometryPtr_->getGeo();
	}

	TopoDS_Shape movedShape = instancedGeometryPtr_->getGeo().Moved(transformation_);
	return movedShape;
}

void GeometryInstance::init(const std::string& guid, bool forceSimple)
{
	if (active_)
	{
		return;
	}
	active_ = true;

	SettingsCollection& settings = SettingsCollection::getInstance();

	// forcing solid
	if (forceSimple)
	{
		geometryShape_ = helperFunctions::boxSimplefyShape(geometryShape_);
	}
	else if (false && geometryShape_.ShapeType() != TopAbs_SOLID)
	{
		if (!helperFunctions::containsSolid(geometryShape_))
		{
			geometryShape_ = helperFunctions::addSolidSemantic(geometryShape_);
		}
	}

	// force triangulation
	for (TopExp_Explorer expl(geometryShape_, TopAbs_FACE); expl.More(); expl.Next())
	{
		const TopoDS_Face& currentFace = TopoDS::Face(expl.Current());
		helperFunctions::triangulateShape(currentFace);
	}

	if (helperFunctions::getFaceCount(geometryShape_) > 500)
	{
		ErrorCollection::getInstance().addError(ErrorID::warningDenseMesh, guid);
		settings.setDenseMesh(true);

		if (settings.ignoreComplex()) { return; }
	}

	// make triangleIndx
	for (TopExp_Explorer expl(geometryShape_, TopAbs_FACE); expl.More(); expl.Next())
	{
		TopoDS_Face productFace = TopoDS::Face(expl.Current());

		TopLoc_Location loc;
		auto mesh = BRep_Tool::Triangulation(productFace, loc);

		if (mesh.IsNull()) {
			ErrorCollection::getInstance().addError(ErrorID::warningUnableToMesh, guid);
			continue;
		}

		for (int i = 1; i <= mesh.get()->NbTriangles(); i++)
		{
			const Poly_Triangle& theTriangle = mesh->Triangles().Value(i);

			std::array<gp_Pnt, 3> trianglePoints{
				mesh->Node(theTriangle(1)).Transformed(loc),
				mesh->Node(theTriangle(2)).Transformed(loc),
				mesh->Node(theTriangle(3)).Transformed(loc)
			};

			auto box = helperFunctions::createBBox(trianglePoints);

			triangleIndex_.insert(std::make_pair(box, MeshTriangle(trianglePoints)));
		}
	}

	return;
}

void GeometryCache::addItem(IfcGeom::BRepElement* brepElem, const gp_Trsf& objectTranslation, const double& gridRotation)
{
	if (!brepElem) { return; }
	const IfcSchema::IfcProduct* product = brepElem->product()->as<IfcSchema::IfcProduct>();

	auto* compound = brepElem->geometry().as_compound();
	auto* occtShape = dynamic_cast<ifcopenshell::geometry::OpenCascadeShape*>(compound);

	if (!occtShape) { throw std::runtime_error("Expected OpenCascadeShape"); }
	TopoDS_Shape shape = occtShape->shape();

	GeometryFingerprint fingerprint = GeometryFingerprint(shape);
	auto it = geometryInstanceCache_.find(fingerprint);
	GeometryInstance* instancePtr = nullptr;
	if (it != geometryInstanceCache_.end())
	{
		instancePtr = &it->second;
		shape = instancePtr->getGeo();
	}
	else
	{
		auto [insertedIt, success] = geometryInstanceCache_.emplace(fingerprint, GeometryInstance(shape));
		instancePtr = &insertedIt->second;
	}

	gp_Trsf trsf = getTransformationBrepElement(brepElem) * objectTranslation;
	gp_Trsf rotation;
	rotation.SetRotation(gp_Ax1(gp_Pnt(0, 0, 0), gp_Dir(0, 0, 1)), gridRotation);
	trsf.Multiply(rotation);

	GeometryCacheItem geometryCache = GeometryCacheItem(product, instancePtr, trsf);
	geometryCacheList_.emplace_back(geometryCache);
}

const GeometryCacheItem& GeometryCache::getItem(int indx) const
{
	if (indx >= geometryCacheList_.size())
	{
		//TODO: add error
	}

	return geometryCacheList_[indx];
}

const GeometryCacheItem* GeometryCache::getItemPtr(int indx)
{
	if (indx >= geometryCacheList_.size())
	{
		//TODO: add error
		return nullptr;
	}
	return &geometryCacheList_[indx];
}
