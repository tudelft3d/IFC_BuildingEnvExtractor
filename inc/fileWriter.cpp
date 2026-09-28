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

template void FileWriter::dumpToCopy<TopoDS_Face>(const TopoDS_Face& theShapeList, const FileWriter::FileNameID& targetLoD);
template void FileWriter::dumpToCopy<TopoDS_Shell>(const TopoDS_Shell& theShapeList, const FileWriter::FileNameID& targetLoD);
template void FileWriter::dumpToCopy<TopoDS_Solid>(const TopoDS_Solid& theShapeList, const FileWriter::FileNameID& targetLoD);
template void FileWriter::dumpToCopy<TopoDS_Shape>(const TopoDS_Shape& theShapeList, const FileWriter::FileNameID& targetLoD);
template void FileWriter::dumpToCopy<TopoDS_Compound>(const TopoDS_Compound& theShapeList, const FileWriter::FileNameID& targetLoD);

template void FileWriter::dumpToCopy<TopoDS_Face>(const std::vector<TopoDS_Face>& theShapeList, const FileWriter::FileNameID& targetLoD, bool isExerior);
template void FileWriter::dumpToCopy<TopoDS_Shell>(const std::vector<TopoDS_Shell>& theShapeList, const FileWriter::FileNameID& targetLoD, bool isExerior);
template void FileWriter::dumpToCopy<TopoDS_Solid>(const std::vector<TopoDS_Solid>& theShapeList, const FileWriter::FileNameID& targetLoD, bool isExerior);
template void FileWriter::dumpToCopy<TopoDS_Shape>(const std::vector<TopoDS_Shape>& theShapeList, const FileWriter::FileNameID& targetLoD, bool isExerior);
template void FileWriter::dumpToCopy<TopoDS_Compound>(const std::vector<TopoDS_Compound>& theShapeList, const FileWriter::FileNameID& targetLoD, bool isExerior);

template void FileWriter::dumpToCopy<TopoDS_Face>(const std::vector<std::pair<TopoDS_Face, storageObject>>& theShapeList, const FileWriter::FileNameID& targetLoD, bool isExterior);
template void FileWriter::dumpToCopy<TopoDS_Shell>(const std::vector<std::pair<TopoDS_Shell, storageObject>>& theShapeList, const FileWriter::FileNameID& targetLoD, bool isExterior);
template void FileWriter::dumpToCopy<TopoDS_Solid>(const std::vector<std::pair<TopoDS_Solid, storageObject>>& theShapeList, const FileWriter::FileNameID& targetLoD, bool isExterior);
template void FileWriter::dumpToCopy<TopoDS_Shape>(const std::vector<std::pair<TopoDS_Shape, storageObject>>& theShapeList, const FileWriter::FileNameID& targetLoD, bool isExterior);
template void FileWriter::dumpToCopy<TopoDS_Compound>(const std::vector<std::pair<TopoDS_Compound, storageObject>>& theShapeList, const FileWriter::FileNameID& targetLoD, bool isExterior);

struct IntXYZ {
	int64_t x, y, z;

	IntXYZ(const gp_XYZ& p, double invTol) {
		x = static_cast<int64_t>(std::floor(p.X() * invTol + 0.5));
		y = static_cast<int64_t>(std::floor(p.Y() * invTol + 0.5));
		z = static_cast<int64_t>(std::floor(p.Z() * invTol + 0.5));
	}
	IntXYZ(const gp_Vec& p, double invTol) {
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
	int floatingPre = 4;

	std::string outStringFloat = "";
	if (std::abs(theValue) < std::pow(0.1, floatingPre))
	{
		outStringFloat = "0.0";
	}
	else
	{
		std::stringstream outSteam;
		outSteam << std::fixed << std::setprecision(floatingPre);
		outSteam << theValue;
		outStringFloat = outSteam.str();
	}

	outStringFloat.erase(outStringFloat.find_last_not_of('0') + 1, std::string::npos);
	if (!outStringFloat.empty() && outStringFloat.back() == '.')
		outStringFloat += "0";
	return outStringFloat;
}


template <typename T>
std::string pointToDXFString(const T& thePoint, const std::string& layerName)
{
	std::stringstream outSteam;

	outSteam << "0\n";
	outSteam << "VERTEX\n";
	outSteam << "8\n";
	outSteam << layerName << "\n";

	outSteam << "10" << "\n";
	outSteam << floatToString(thePoint.X()) << "\n";
	outSteam << "20" << "\n";
	outSteam << floatToString(thePoint.Y()) << "\n";
	outSteam << "30" << "\n";
	outSteam << floatToString(thePoint.Z()) << "\n";

	outSteam << "70\n";
	outSteam << "192\n";

	return outSteam.str();
}


std::string triangleToDXFString(int triangleIdx, const std::string& layerName) {

	std::stringstream outSteam;

	outSteam << "0\n";
	outSteam << "VERTEX\n";
	outSteam << "8\n";
	outSteam << layerName << "\n";

	outSteam << "71" << "\n";
	outSteam << triangleIdx + 1 << "\n";
	outSteam << "72" << "\n";
	outSteam << triangleIdx + 2 << "\n";
	outSteam << "73" << "\n";
	outSteam << triangleIdx + 3 << "\n";

	outSteam << "70\n";
	outSteam << "128\n";

	return outSteam.str();
}

template <typename T>
std::string pointToOBJString(const T& thePoint)
{
	std::stringstream outSteam;
	outSteam << "v " << floatToString(thePoint.X()) << " " << floatToString(thePoint.Y()) << " " << floatToString(thePoint.Z()) << "\n";
	return outSteam.str();
}

template <typename T>
std::string normalToOBJString(const T& thePoint)
{
	std::stringstream outSteam;
	outSteam << "vn " << floatToString(thePoint.X()) << " " << floatToString(thePoint.Y()) << " " << floatToString(thePoint.Z()) << "\n";
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

template <typename T>
void FileWriter::writeToSTEP(const std::vector<std::pair<T, storageObject>>& theShapeList, const std::string& targetPath)
{
	std::stringstream buffer;
	std::streambuf* originalBuffer = std::cout.rdbuf(buffer.rdbuf());

	TopoDS_Compound compound;
	BRep_Builder builder;
	builder.MakeCompound(compound);
	for (const auto& shape : theShapeList)
	{
		builder.Add(compound, shape.first);
	}

	STEPControl_Writer writer;
	writer.Transfer(compound, STEPControl_AsIs);
	IFSelect_ReturnStatus stat = writer.Write(targetPath.c_str());

	std::cout.rdbuf(originalBuffer);
	return;
}

template <typename T>
void FileWriter::writeToOBJ(const std::vector<std::pair<T, storageObject>>& theShapeList, const std::string& targetPath)
{
	if (theShapeList.empty()) { return; }

	double precision = SettingsCollection::getInstance().linearTolerance();

	std::stringstream faceStringStream;
	std::stringstream vertexNormalStream;
	std::ofstream objFile(targetPath);
	int vertIdxOffset = 1;
	int normalIdxOffset = 1;
	std::unordered_map<IntXYZ, int, IntXYZ_Hash, IntXYZ_Equal> vertMap;
	std::unordered_map<IntXYZ, int, IntXYZ_Hash, IntXYZ_Equal> normalMap;

	int counter = 0;
	for (const auto& theShapePair : theShapeList)
	{
		storageObject currentShapeData = theShapePair.second;
		if (currentShapeData.isNull())
		{
			faceStringStream << "g " << counter << "\n";
		}
		else
		{
			faceStringStream << "g " << currentShapeData.storeyName << " " << currentShapeData.objectType << " " << currentShapeData.guid << "\n";
		}

		for (TopExp_Explorer exp(theShapePair.first, TopAbs_FACE); exp.More(); exp.Next()) {
			TopoDS_Face face = TopoDS::Face(exp.Current());
			TopLoc_Location loc;

			gp_Vec currentNormal = helperFunctions::computeFaceNormal(face);

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
				faceStringStream << "f";

				for (int i = 1; i <= 3; ++i)
				{
					// store the face vert indx
					const int index = flipTriangle ? 4 - i : i;
					gp_XYZ xyz = mesh->Node(theTriangle(index)).Transformed(loc).Coord();
					IntXYZ intXyz = IntXYZ(xyz, 1 / precision);
					if (vertMap.find(intXyz) != vertMap.end())
					{
						faceStringStream << " " << vertMap[intXyz];
					}
					else
					{
						faceStringStream << " " << vertIdxOffset;

						// store the vert
						objFile << pointToOBJString(xyz);
						vertMap.try_emplace(intXyz, vertIdxOffset);
						vertIdxOffset++;
					}

					// store the face vert normal

					IntXYZ intNormal = IntXYZ(currentNormal, 1 / precision);
					if (normalMap.find(intNormal) != normalMap.end())
					{
						faceStringStream << "//" << normalMap[intNormal];
					}
					else
					{
						faceStringStream << "//" << normalIdxOffset;

						vertexNormalStream << normalToOBJString(currentNormal);
						normalMap.try_emplace(intNormal, normalIdxOffset);
						normalIdxOffset++;
					}
				}
				faceStringStream << "\n";
			}
		}
		counter++;
	}

	objFile << vertexNormalStream.str();
	objFile << faceStringStream.str();
	objFile.close();
	return;
}

template<typename T>
void FileWriter::writeToDXF(const std::vector<std::pair<T, storageObject>>& theShapeList, const std::string& targetPath)
{
	if (theShapeList.empty()) { return; }
	double precision = SettingsCollection::getInstance().linearTolerance();

	std::map<std::string, std::vector<int>> storeyGroupMap;

	std::ofstream dxfFile(targetPath);

	// create the DXF header
	dxfFile << "0\n";
	dxfFile << "SECTION\n";
	dxfFile << "2\n";
	dxfFile << "HEADER\n";

	dxfFile << "9\n";
	dxfFile << "$ACADVER\n";
	dxfFile << "1\n";
	dxfFile << "AC1009\n";

	dxfFile << "9\n";
	dxfFile << "$INSUNITS\n";
	dxfFile << "70\n";
	dxfFile << "6\n";

	dxfFile << "0\n";
	dxfFile << "ENDSEC\n";

	// create the body

	dxfFile << "0\n";
	dxfFile << "SECTION\n";
	dxfFile << "2\n";
	dxfFile << "ENTITIES\n";

	int objectCounter = 0;
	for (const auto& theShapePair : theShapeList)
	{
		objectCounter++;

		storageObject currentShapeData = theShapePair.second;
		std::string layerName = currentShapeData.isNull() ? "Default" : currentShapeData.objectType;

		if (!currentShapeData.isNull())
		{
			if (storeyGroupMap.find(currentShapeData.storeyName) == storeyGroupMap.end())
			{
				storeyGroupMap[currentShapeData.storeyName] = { objectCounter };
			}
			else
			{
				storeyGroupMap.at(currentShapeData.storeyName).emplace_back(objectCounter);
			}
		}

		dxfFile << "0\n";
		dxfFile << "POLYLINE\n";
		if (!currentShapeData.isNull())
		{
			dxfFile << "5\n";
			dxfFile << objectCounter << "\n";
		}

		dxfFile << "8\n";
		dxfFile << layerName << "\n";

		if (!currentShapeData.isNull())
		{
			if (currentShapeData.objectType == "IfcWindow")
			{
				dxfFile << "62\n";
				dxfFile << "5\n";
			}
			else if (currentShapeData.objectType == "IfcDoor")
			{
				dxfFile << "62\n";
				dxfFile << "34\n";
			}
		}
		
		dxfFile << "66\n";
		dxfFile << "1\n";
		dxfFile << "70\n";
		dxfFile << "64\n";

		int counter = 0;
		for (TopExp_Explorer exp(theShapePair.first, TopAbs_FACE); exp.More(); exp.Next()) {
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

				for (int i = 1; i <= 3; ++i)
				{
					const int index = flipTriangle ? 4 - i : i;
					gp_XYZ xyz = mesh->Node(theTriangle(index)).Transformed(loc).Coord();
					dxfFile << pointToDXFString(xyz, layerName);
				}
				counter++;
			}			
		}

		for (size_t i = 0; i < counter; i++)
		{
			dxfFile << triangleToDXFString(3 * i, layerName);
		}

		dxfFile << "0\n";
		dxfFile << "SEQEND\n";
		dxfFile << "8\n";
		dxfFile << layerName << "\n";
	}

	for (const std::pair<std::string, std::vector<int>>& currentStorey : storeyGroupMap)
	{
		objectCounter++;
		dxfFile << "0\n";
		dxfFile << "GROUP\n";
		dxfFile << "5\n";
		dxfFile << objectCounter << "\n";

		dxfFile << "100\n";
		dxfFile << "AcDbGroup\n";
		dxfFile << "300\n";
		dxfFile << currentStorey.first << "\n";

		dxfFile << "70\n";
		dxfFile << "0\n";
		dxfFile << "71\n";
		dxfFile << currentStorey.second.size() << "\n";


		for (int currentId : currentStorey.second)
		{
			dxfFile << "340\n";
			dxfFile << currentId << "\n";
		}
	}

	dxfFile << "0\n";
	dxfFile << "ENDSEC\n";
	dxfFile << "0\n";
	dxfFile << "EOF";
	dxfFile.close();
	return;
}

template<typename T>
void FileWriter::dumpToCopy(const T& theShape, const FileWriter::FileNameID& targetLoD)
{
	std::vector<std::pair<T, storageObject > >shapeList = { std::make_pair(theShape, storageObject()) };
	dumpToCopy(shapeList, targetLoD);
	return;
}

template<typename T>
void FileWriter::dumpToCopy(const std::vector<T>& theShapeList, const FileWriter::FileNameID& targetLoD, bool isExterior)
{
	std::vector<std::pair<T, storageObject>> copyList;

	for (const T& theShape : theShapeList)
	{
		copyList.emplace_back(std::make_pair(theShape, storageObject()));
	}
	dumpToCopy(copyList, targetLoD, isExterior);
}


template<typename T>
void FileWriter::dumpToCopy(const std::vector<std::pair<T, storageObject>>& theShapeList, const FileWriter::FileNameID& targetLoD, bool isExterior)
{
	SettingsCollection& settingsCollection = SettingsCollection::getInstance();
	std::string basePath = settingsCollection.getOutputBasePath();

	if (settingsCollection.createOBJ())
	{
		writeToOBJ(theShapeList, basePath + FileNameEnum::getString(targetLoD, FileWriter::FileExtensionID::OBJ, isExterior));
	}

	if (settingsCollection.createSTEP())
	{
		writeToSTEP(theShapeList, basePath + FileNameEnum::getString(targetLoD, FileWriter::FileExtensionID::STEP, isExterior));
	}

	if (settingsCollection.createDXF())
	{
		writeToDXF(theShapeList, basePath + FileNameEnum::getString(targetLoD, FileWriter::FileExtensionID::DXF, isExterior));
	}
	return;
}
