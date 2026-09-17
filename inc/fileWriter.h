#include <TopoDS.hxx>
#include <vector>

#ifndef FILEWRITER_FILEWRITER_H
#define FILEWRITER_FILEWRITER_H

struct FileWriter {
	enum class FileExtensionID
	{
		JSON,
		STEP,
		OBJ,
		DXF
	};

	class FileExtensionEnum {
	public:
		static std::string getString(FileExtensionID id);
	};

	/// collects all the file extension of the output
	enum class FileNameID {
		LoD00,
		LoD02,
		LoD03,
		LoD04,
		LoD10,
		LoD12,
		LoD13,
		LoD22,
		LoDb,
		LoDc1,
		LoDc2,
		LoDd1,
		LoD40,
		LoD41,
		LoD42,
		LoDe1,
		LoD32,
		LoD50,
	};


	class FileNameEnum {
	public:
		static std::string getString(FileNameID name, FileExtensionID id, bool isExterior = true);
	};

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

	/// write shape to DXF
	template <typename T>
	static void writeToDXF(const T& theShape, const std::string& targetPath);

	/// write shape to DXF
	template <typename T>
	static void writeToDXF(const std::vector<T>& theShapeList, const std::string& targetPath);

	/// write file to non-cityJSON output files
	template <typename T>
	static void dumpToCopy(const T& theShape, const FileWriter::FileNameID& targetLoD);

	/// write file to non-cityJSON output files
	template <typename T>
	static void dumpToCopy(const std::vector<T>& theShapeList, const FileWriter::FileNameID& targetLoD, bool isExterior = true);
};
#endif // FILEWRITER_FILEWRITER_H