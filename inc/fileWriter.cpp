#include<string>

#include <STEPControl_Writer.hxx>
#include <Poly_Triangle.hxx>
#include <BRep_Tool.hxx>
#include <BRep_Builder.hxx>
#include <TopExp_Explorer.hxx>
#include <BRepTools_WireExplorer.hxx>

#include "settingsCollection.h"
#include "fileWriter.h"
#include "helper.h"

template void FileWriter::writeToSTEP<TopoDS_Face>(const std::vector<TopoDS_Face>& theShapeList, const std::string& targetPath);
template void FileWriter::writeToSTEP<TopoDS_Shell>(const std::vector<TopoDS_Shell>& theShapeList, const std::string& targetPath);
template void FileWriter::writeToSTEP<TopoDS_Solid>(const std::vector<TopoDS_Solid>& theShapeList, const std::string& targetPath);
template void FileWriter::writeToSTEP<TopoDS_Shape>(const std::vector<TopoDS_Shape>& theShapeList, const std::string& targetPath);

template void FileWriter::writeToSTEP<TopoDS_Face>(const std::vector<std::vector<TopoDS_Face>>& theShapeList, const std::string& targetPath);
template void FileWriter::writeToSTEP<TopoDS_Shell>(const std::vector<std::vector<TopoDS_Shell>>& theShapeList, const std::string& targetPath);
template void FileWriter::writeToSTEP<TopoDS_Solid>(const std::vector<std::vector<TopoDS_Solid>>& theShapeList, const std::string& targetPath);
template void FileWriter::writeToSTEP<TopoDS_Shape>(const std::vector<std::vector<TopoDS_Shape>>& theShapeList, const std::string& targetPath);

template void FileWriter::writeToOBJ<TopoDS_Face>(const TopoDS_Face& theShapeList, const std::string& targetPath);
template void FileWriter::writeToOBJ<TopoDS_Shell>(const TopoDS_Shell& theShapeList, const std::string& targetPath);
template void FileWriter::writeToOBJ<TopoDS_Solid>(const TopoDS_Solid& theShapeList, const std::string& targetPath);
template void FileWriter::writeToOBJ<TopoDS_Shape>(const TopoDS_Shape& theShapeList, const std::string& targetPath);
template void FileWriter::writeToOBJ<TopoDS_Compound>(const TopoDS_Compound& theShapeList, const std::string& targetPath);

template void FileWriter::writeToOBJ<TopoDS_Face>(const std::vector<TopoDS_Face>& theShapeList, const std::string& targetPath);
template void FileWriter::writeToOBJ<TopoDS_Shell>(const std::vector<TopoDS_Shell>& theShapeList, const std::string& targetPath);
template void FileWriter::writeToOBJ<TopoDS_Solid>(const std::vector<TopoDS_Solid>& theShapeList, const std::string& targetPath);
template void FileWriter::writeToOBJ<TopoDS_Shape>(const std::vector<TopoDS_Shape>& theShapeList, const std::string& targetPath);
template void FileWriter::writeToOBJ<TopoDS_Compound>(const std::vector<TopoDS_Compound>& theShapeList, const std::string& targetPath);

template void FileWriter::writeToOBJ<TopoDS_Face>(const std::vector<std::vector<TopoDS_Face>>& theShapeList, const std::string& targetPath);
template void FileWriter::writeToOBJ<TopoDS_Shell>(const std::vector<std::vector<TopoDS_Shell>>& theShapeList, const std::string& targetPath);
template void FileWriter::writeToOBJ<TopoDS_Solid>(const std::vector<std::vector<TopoDS_Solid>>& theShapeList, const std::string& targetPath);
template void FileWriter::writeToOBJ<TopoDS_Shape>(const std::vector<std::vector<TopoDS_Shape>>& theShapeList, const std::string& targetPath);

template void FileWriter::dumpToCopy<TopoDS_Face>(const TopoDS_Face& theShapeList, const FileWriter::FileNameID& targetLoD);
template void FileWriter::dumpToCopy<TopoDS_Shell>(const TopoDS_Shell& theShapeList, const FileWriter::FileNameID& targetLoD);
template void FileWriter::dumpToCopy<TopoDS_Solid>(const TopoDS_Solid& theShapeList, const FileWriter::FileNameID& targetLoD);
template void FileWriter::dumpToCopy<TopoDS_Shape>(const TopoDS_Shape& theShapeList, const FileWriter::FileNameID& targetLoD);
template void FileWriter::dumpToCopy<TopoDS_Compound>(const TopoDS_Compound& theShapeList, const FileWriter::FileNameID& targetLoD);

template void FileWriter::dumpToCopy<TopoDS_Face>(const std::vector<TopoDS_Face>& theShapeList, const FileWriter::FileNameID& targetLoD);
template void FileWriter::dumpToCopy<TopoDS_Shell>(const std::vector<TopoDS_Shell>& theShapeList, const FileWriter::FileNameID& targetLoD);
template void FileWriter::dumpToCopy<TopoDS_Solid>(const std::vector<TopoDS_Solid>& theShapeList, const FileWriter::FileNameID& targetLoD);
template void FileWriter::dumpToCopy<TopoDS_Shape>(const std::vector<TopoDS_Shape>& theShapeList, const FileWriter::FileNameID& targetLoD);

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


std::string floatToString(double theValue)
{
	int floatingPre = 6;

	if (std::abs(theValue) < std::pow(0.1, floatingPre))
	{
		theValue = 0;
	}

	std::stringstream outSteam;
	outSteam << std::fixed << std::setprecision(floatingPre);
	outSteam << theValue;

	std::string outStringFloat = outSteam.str();
	outStringFloat.erase(outStringFloat.find_last_not_of('0') + 1, std::string::npos);
	if (!outStringFloat.empty() && outStringFloat.back() == '.')
		outStringFloat += "0";
	return outStringFloat;
}


template <typename T>
std::string pointToString(const T& thePoint, int i)
{
	std::stringstream outSteam;
	outSteam << "1" << i << "\n";
	outSteam << floatToString(thePoint.X()) << "\n";
	outSteam << "2" << i << "\n";
	outSteam << floatToString(thePoint.Y()) << "\n";
	outSteam << "3" << i << "\n";
	outSteam << floatToString(thePoint.Z()) << "\n";
	return outSteam.str();
}

std::string FileWriter::FileExtensionEnum::getString(FileExtensionID id)
{
	switch (id) {
	case FileExtensionID::JSON:
		return ".json";
	case FileExtensionID::OBJ:
		return ".obj";
	case FileExtensionID::STEP:
		return ".step";
	case FileExtensionID::DXF:
		return ".dxf";

	default:
		return "";
	}
}

std::string FileWriter::FileNameEnum::getString(FileNameID name, FileExtensionID id, bool isExterior)
{
	std::string fileExternsion = FileExtensionEnum::getString(id);
	std::string locationTag = "i";
	if (isExterior) { locationTag = "e"; }
	std::string dash = "_";

	switch (name) {
	case FileNameID::LoD00:
		return dash + locationTag + "LoD00" + fileExternsion;
	case FileNameID::LoD02:
		return dash + locationTag + "LoD02" + fileExternsion;
	case FileNameID::LoD03:
		return dash + locationTag + "LoD03" + fileExternsion;
	case FileNameID::LoD04:
		return dash + locationTag + "LoD04" + fileExternsion;
	case FileNameID::LoD10:
		return dash + locationTag + "LoD10" + fileExternsion;
	case FileNameID::LoD12:
		return dash + locationTag + "LoD12" + fileExternsion;
	case FileNameID::LoD13:
		return dash + locationTag + "LoD13" + fileExternsion;
	case FileNameID::LoD22:
		return dash + locationTag + "LoD22" + fileExternsion;
	case FileNameID::LoDb:
		return dash + locationTag + "LoDb" + fileExternsion;
	case FileNameID::LoDc1:
		return dash + locationTag + "LoDc1" + fileExternsion;
	case FileNameID::LoDc2:
		return dash + locationTag + "LoDc2" + fileExternsion;
	case FileNameID::LoDd1:
		return dash + locationTag + "LoDd1" + fileExternsion;
	case FileNameID::LoD40:
		return dash + locationTag + "LoD40" + fileExternsion;
	case FileNameID::LoD41:
		return dash + locationTag + "LoD41" + fileExternsion;
	case FileNameID::LoD42:
		return dash + locationTag + "LoD42" + fileExternsion;
	case FileNameID::LoDe1:
		return dash + locationTag + "LoDe1" + fileExternsion;
	case FileNameID::LoD32:
		return dash + locationTag + "LoD32" + fileExternsion;
	case FileNameID::LoD50:
		return dash + locationTag + "LoD50" + fileExternsion;

	default:
		return "";
	}
}


void FileWriter::writeToSTEP(const TopoDS_Shape& theShape, const std::string& targetPath)
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
void FileWriter::writeToSTEP(const std::vector<T>& theShapeList, const std::string& targetPath)
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
void FileWriter::writeToSTEP(const std::vector<std::vector<T>>& theShapeList, const std::string& targetPath)
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
void FileWriter::writeToOBJ(const T& theShape, const std::string& targetPath)
{
	std::vector<T> shapeList = { theShape };
	writeToOBJ(shapeList, targetPath);
	return;
}

template <typename T>
void FileWriter::writeToOBJ(const std::vector<T>& theShapeList, const std::string& targetPath)
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
				helperFunctions::triangulateShape(face);
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
						gp_XYZ xyz = mesh->Node(theTriangle(i)).Transformed(loc).Coord();
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
				else
				{
					for (size_t i = 1; i <= 3; i++)
					{
						gp_XYZ xyz = mesh->Node(theTriangle(i)).Transformed(loc).Coord();
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
void FileWriter::writeToOBJ(const std::vector<std::vector<T>>& theShapeList, const std::string& targetPath)
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

template<typename T>
void FileWriter::writeToDXF(const T& theShape, const std::string& targetPath)
{
	std::vector<T> shapeList = { theShape };
	writeToDXF(shapeList, targetPath);
	return;
}

template<typename T>
void FileWriter::writeToDXF(const std::vector<T>& theShapeList, const std::string& targetPath)
{
	if (theShapeList.empty()) { return; }
	double precision = SettingsCollection::getInstance().linearTolerance();

	std::ofstream objFile(targetPath);

	// create the DXF header
	objFile << "0\n";
	objFile << "SECTION\n";
	objFile << "2\n";
	objFile << "HEADER\n";

	objFile << "9\n";
	objFile << "$ACADVER\n";
	objFile << "1\n";
	objFile << "AC1009\n";

	objFile << "9\n";
	objFile << "$INSUNITS\n";
	objFile << "70\n";
	objFile << "6\n";

	objFile << "0\n";
	objFile << "ENDSEC\n";

	// create the body

	objFile << "0\n";
	objFile << "SECTION\n";
	objFile << "2\n";
	objFile << "ENTITIES\n";

	for (const T& theShape : theShapeList)
	{
		for (TopExp_Explorer exp(theShape, TopAbs_FACE); exp.More(); exp.Next()) {
			TopoDS_Face face = TopoDS::Face(exp.Current());
			TopLoc_Location loc;

			std::string layerName = "0\n";  //TODO: make this proper layer structure

			auto mesh = BRep_Tool::Triangulation(face, loc);

			if (mesh.IsNull())
			{
				helperFunctions::triangulateShape(face);
				mesh = BRep_Tool::Triangulation(face, loc);
			}
			if (mesh.IsNull()) { continue; }

			bool flipTriangle = false;
			if (face.Orientation() == TopAbs_REVERSED)
			{
				flipTriangle = true;
			}

			if (helperFunctions::getPointCount(face) == 4)
			{
				objFile << "0\n";
				objFile << "3DFACE\n";
				objFile << "8\n";
				objFile << layerName;

				for (TopExp_Explorer faceExpl(face, TopAbs_WIRE); faceExpl.More(); faceExpl.Next())
				{
					TopoDS_Wire wire = TopoDS::Wire(faceExpl.Current());
					BRepTools_WireExplorer wireExpl(wire);

					int counter = 0;
					while (wireExpl.More())
					{
						TopoDS_Vertex v = wireExpl.CurrentVertex();
						gp_Pnt p = BRep_Tool::Pnt(v);
						objFile << pointToString(p, counter);			
						counter++;
						wireExpl.Next();
					}
				}
				continue;
			}

			for (int j = 1; j <= mesh.get()->NbTriangles(); j++)
			{
				const Poly_Triangle& theTriangle = mesh->Triangles().Value(j);

				objFile << "0\n";
				objFile << "3DFACE\n";
				objFile << "8\n";
				objFile << layerName;

				for (size_t i = 1; i <= 4; i++)
				{
					gp_XYZ xyz;
					if (i <= 3) { xyz = mesh->Node(theTriangle(i)).Transformed(loc).Coord(); }
					else { xyz = mesh->Node(theTriangle(3)).Transformed(loc).Coord(); }			
					objFile << pointToString(xyz, i - 1);
				}
			}
		}
	}

	objFile << "0\n";
	objFile << "ENDSEC\n";
	objFile << "0\n";
	objFile << "EOF";
	objFile.close();
	return;
}

template<typename T>
void FileWriter::dumpToCopy(const T& theShape, const FileWriter::FileNameID& targetLoD)
{
	std::vector<T> shapeList = { theShape };
	dumpToCopy(shapeList, targetLoD);
	return;
}

template<typename T>
void FileWriter::dumpToCopy(const std::vector<T>& theShapeList, const FileWriter::FileNameID& targetLoD)
{
	SettingsCollection& settingsCollection = SettingsCollection::getInstance();
	std::string basePath = settingsCollection.getOutputBasePath();

	if (settingsCollection.createOBJ())
	{
		writeToOBJ(theShapeList, basePath + FileNameEnum::getString(targetLoD, FileWriter::FileExtensionID::OBJ));
	}

	if (settingsCollection.createSTEP())
	{
		writeToSTEP(theShapeList, basePath + FileNameEnum::getString(targetLoD, FileWriter::FileExtensionID::STEP));
	}

	if (settingsCollection.createDXF())
	{
		writeToDXF(theShapeList, basePath + FileNameEnum::getString(targetLoD, FileWriter::FileExtensionID::DXF));
	}
	return;
}
