#include "helper.h"
#include "voxelGrid.h"
#include "voxel.h"
#include "stringManager.h"
#include "DebugUtils.h"

#include <algorithm>
#include <execution>
#include <thread>  

void VoxelGrid::init(DataManager* h)
{
	anchor_ = h->getLllPoint();

	SettingsCollection& settingsCollection = SettingsCollection::getInstance();

	voxelsize_ = settingsCollection.voxelSize();

	gp_Pnt urrPoint = h->getUrrPoint();

	// resize to allow full voxel encapsulation
	anchor_.SetX(anchor_.X() - (voxelsize_ * 2));
	anchor_.SetY(anchor_.Y() - (voxelsize_ * 2));
	anchor_.SetZ(anchor_.Z() - (voxelsize_ * 2));

	urrPoint.SetX(urrPoint.X() + (voxelsize_ * 2));
	urrPoint.SetY(urrPoint.Y() + (voxelsize_ * 2));
	urrPoint.SetZ(urrPoint.Z() + (voxelsize_ * 2));

	// set range
	double xRange = urrPoint.X() - anchor_.X();
	double yRange = urrPoint.Y() - anchor_.Y();
	double zRange = urrPoint.Z() - anchor_.Z();

	xRelRange_ = static_cast<int>(ceil(xRange / voxelsize_) + 1);
	yRelRange_ = static_cast<int>(ceil(yRange / voxelsize_) + 1);
	zRelRange_ = static_cast<int>((int)ceil(zRange / voxelsize_) + 1);

	if (!fitsMemoryBudget())
	{
		throw ErrorID::errorVoxelCountOverflow;
	}

	planeRotation_ = settingsCollection.gridRotation();

	totalVoxels_ = xRelRange_ * yRelRange_ * zRelRange_;
	if (totalVoxels_ > VoxelLookup_.max_size())
	{
		//TODO: add error
		return;
	}

	VoxelLookup_.resize(totalVoxels_);

	return;
}

bool VoxelGrid::fitsMemoryBudget() const
{
	std::uint64_t voxelCount =
		static_cast<std::uint64_t>(xRelRange_) *
		yRelRange_ *
		zRelRange_;

	return voxelCount <= static_cast<std::uint64_t>(std::numeric_limits<int>::max());
}

void VoxelGrid::populateVoxelGrid(DataManager* h)
{
	std::chrono::steady_clock::time_point startTime = std::chrono::steady_clock::now();
	std::cout << CommunicationStringEnum::getString(CommunicationStringID::infoPopulateGrid) << std::endl;

	// compute column scores
	std::vector<int> columScoreList = computeColumnScore(h);
	int columSumScore = std::accumulate(columScoreList.begin(), columScoreList.end(), 0);

	if (columSumScore == 0) {
		//TODO: add error
		return;
	}

	// split the range over cores
	int coreUse = SettingsCollection::getInstance().threadcount() - 1;
	int threadScore = columSumScore / coreUse;
	int voxelsGrown = 0;
	int beginIdx = 0;

	std::vector<std::thread> threadList;
	for (int i = 0; i < coreUse; i++)
	{
		int endIdx = 0;
		// compute the core processing list size
		if (i == coreUse - 1)
		{
			endIdx = xRelRange_ * yRelRange_;
		}
		else
		{
			endIdx = computeEndColumnIdx(columScoreList, threadScore, beginIdx);
		}

		threadList.emplace_back([=, &voxelsGrown]()
			{
				addVoxelColumn(beginIdx, endIdx, h, &voxelsGrown);
			});

		// set begin of the next step to the end of the current step
		beginIdx = endIdx;
	}

	// count the voxels that have been generated
	std::thread countThread([&]() { countVoxels(&voxelsGrown); });
	for (size_t i = 0; i < threadList.size(); i++) {
		if (threadList[i].joinable()) { threadList[i].join(); }
	}
	if (countThread.joinable()) { countThread.join(); }

	std::cout << std::endl;
	helperFunctions::printTime(startTime, std::chrono::steady_clock::now());	
}

void VoxelGrid::countVoxels(const int* voxelGrowthCount)
{
	while (true)
	{
		std::this_thread::sleep_for(std::chrono::seconds(1));
		std::unique_lock<std::mutex> voxelCountLock(voxelGrowthMutex_);
		int count = *voxelGrowthCount;
		voxelCountLock.unlock();

		std::cout.flush();
		std::cout << "\t" << count << " of " << totalVoxels_ << "\r";

		if (count == totalVoxels_) { return; }
		if (activeThreads_ == 0) { return; }
	}
}


bool VoxelGrid::addVoxel(int indx, DataManager* h)
{
	auto midPoint = relPointToWorld(linearToRelative<BoostPoint3D>(indx));
	SettingsCollection& settingsCollection = SettingsCollection::getInstance();

	std::unique_ptr<voxel> boxel = std::make_unique<voxel>(voxel());
	
	std::vector<gp_Pnt> pointList;
	if (settingsCollection.intersectionLogic() == 2) { pointList = getPlanePoints(indx); }
	else if (settingsCollection.intersectionLogic() == 3) { pointList = getCornerPoints(indx); }

	// make a pointlist 0 - 3 lower ring, 4 - 7 upper ring
	auto boxelGeo = getVoxelGeo(indx);
	gp_Pnt midPointOCCT = helperFunctions::Point3DBTO(midPoint);

	std::vector<Value> qResult;
	qResult.clear(); // no clue why, but avoids a random indexing issue that can occur
	h->getIndexPointer()->query(bgi::intersects(boxelGeo), std::back_inserter(qResult));
	for (const Value& resultItem : qResult)
	{
		IfcProductSpatialData* lookup = h->getLookupPtr(resultItem.second);
		if (boxel->checkIntersecting(*lookup, pointList, midPointOCCT, settingsCollection.intersectionLogic()))
		{
			boxel->addInternalProduct(resultItem);
		}
	}

	bool isIntersecting = boxel->getIsIntersecting();

	if (!isIntersecting)
	{
		return isIntersecting;
	}

	VoxelLookup_[indx] = std::move(boxel);

	return isIntersecting;
}

void VoxelGrid::addVoxelColumn(int beginIindx, int endIdx, DataManager* h, int* voxelGrowthCount)
{
	addActiveThread();
	bool fullVoxelReq = SettingsCollection::getInstance().requireFullVoxels();
	// get top plate begin to start growing voxels down from
	int topPlateBeginIndx = (zRelRange_ -1) * xRelRange_  * yRelRange_ ;

	for (int i = beginIindx; i < endIdx; i++)
	{
		int voxelIndx = topPlateBeginIndx + i;
		while (true)
		{
			if ( addVoxel(voxelIndx, h) && !fullVoxelReq) { break; }

			// add to the voxel counter after adding
			std::unique_lock<std::mutex> voxelCountLock(voxelGrowthMutex_);
			(*voxelGrowthCount)++;
			voxelCountLock.unlock();

			voxelIndx = getLowerNeighbour(voxelIndx);
			if (voxelIndx == -1) { break; }
		}
	}
	removeActiveThread();
}


std::vector<int> VoxelGrid::computeColumnScore(DataManager* h)
{
	SettingsCollection& settingsCollection = SettingsCollection::getInstance();
	double voxelSize = settingsCollection.voxelSize();
	int loopRange = xRelRange_ * yRelRange_;
	int plateIndx = (zRelRange_ - 1) * xRelRange_ * yRelRange_;

	std::vector<int> columScoreList;
	for (int i = 0; i < loopRange; i++)
	{
		int plate = plateIndx + i;

		BoostPoint3D coneCenter = relPointToWorld(linearToRelative<BoostPoint3D>(plate));
		BoostPoint3D lll = BoostPoint3D(
			coneCenter.get<0>() - voxelSize / 2,
			coneCenter.get<1>() - voxelSize / 2,
			h->getLllPoint().Z() - voxelSize
		);

		BoostPoint3D urr = BoostPoint3D(
			coneCenter.get<0>() + voxelSize / 2,
			coneCenter.get<1>() + voxelSize / 2,
			coneCenter.get<2>() + voxelSize / 2
		);

		std::vector<Value> qResult;
		qResult.clear(); // no clue why, but avoids a random indexing issue that can occur
		h->getIndexPointer()->query(
			bgi::intersects(
				bg::model::box<BoostPoint3D>(lll, urr)
			), std::back_inserter(qResult)
		);

		int columnScore = static_cast<int>(qResult.size());
		columScoreList.emplace_back(columnScore);
	}
	return columScoreList;
}


void VoxelGrid::addActiveThread()
{
	std::unique_lock<std::mutex> threadCountLock(voxelGrowthMutex_);
	activeThreads_++;
	threadCountLock.unlock();
}

void VoxelGrid::removeActiveThread()
{
	std::unique_lock<std::mutex> threadCountLock(voxelGrowthMutex_);
	activeThreads_--;
	threadCountLock.unlock();
}

int VoxelGrid::computeEndColumnIdx(const std::vector<int>& columScoreList, const int threadScore, const int beginIdx)
{
	int score = 0;
	int endIdx = 0;
	int loopRange = xRelRange_ * yRelRange_;

	for (int j = beginIdx; j < loopRange; j++)
	{
		score += columScoreList[j];
		if (score > threadScore)
		{
			return j;
		}
	}
	return -1;
}


template<typename T>
T VoxelGrid::linearToRelative(int i) const
{
	int x = i % xRelRange_;
	int y = static_cast<int>(floor((i / xRelRange_) % yRelRange_));
	int z = static_cast<int>(floor(i / (xRelRange_ * yRelRange_)));

	return T(x, y, z);
}

int VoxelGrid::relativeToLinear(const BoostPoint3D& p) const
{
	int x = static_cast<int>(std::round(p.get<0>()));
	int y = static_cast<int>(std::round(p.get<1>()));
	int z = static_cast<int>(std::round(p.get<2>()));

	int i = z * xRelRange_ * yRelRange_ + y * xRelRange_ + x;

	return i;
}

BoostPoint3D VoxelGrid::relPointToWorld(const BoostPoint3D& p) const
{
	double voxelSize = SettingsCollection::getInstance().voxelSize();

	double xCoord = anchor_.X() + (bg::get<0>(p) * voxelSize) + voxelSize / 2;
	double yCoord = anchor_.Y() + (bg::get<1>(p) * voxelSize) + voxelSize / 2;
	double zCoord = anchor_.Z() + (bg::get<2>(p) * voxelSize) + voxelSize / 2;

	return BoostPoint3D(xCoord, yCoord, zCoord);
}

BoostPoint3D VoxelGrid::worldToRelPoint(BoostPoint3D p) const
{
	double voxelSize = SettingsCollection::getInstance().voxelSize();

	double xCoord = (bg::get<0>(p) - anchor_.X() - voxelSize / 2) / voxelSize;
	double yCoord = (bg::get<1>(p) - anchor_.Y() - voxelSize / 2) / voxelSize;
	double zCoord = (bg::get<2>(p) - anchor_.Z() - voxelSize / 2) / voxelSize;

	return BoostPoint3D(xCoord, yCoord, zCoord);
}

std::array<int, 6> VoxelGrid::getDirNeighbours(int voxelIndx) const
{
	std::array<int, 6> neightbours;
	gp_Pnt loc3D = linearToRelative<gp_Pnt>(voxelIndx);

	bool xSmall = loc3D.X() - 1 >= 0;
	bool xBig = loc3D.X() + 1 < xRelRange_;

	bool ySmall = loc3D.Y() - 1 >= 0;
	bool yBig = loc3D.Y() + 1 < yRelRange_;

	bool zSmall = loc3D.Z() - 1 >= 0;
	bool zBig = loc3D.Z() + 1 < zRelRange_;

	// connectivity
	if (xSmall) { neightbours[0] = voxelIndx - 1; }
	else { neightbours[0] = -1; }
	if (xBig) { neightbours[1] = voxelIndx + 1; }
	else { neightbours[1] = -1; }
	if (ySmall) { neightbours[2] = voxelIndx - xRelRange_; }
	else { neightbours[2] = -1; }
	if (yBig) { neightbours[3] = voxelIndx + xRelRange_; }
	else { neightbours[3] = -1; }
	if (zSmall) { neightbours[4] = voxelIndx - xRelRange_ * yRelRange_; }
	else { neightbours[4] = -1; }
	if (zBig) { neightbours[5] = voxelIndx + xRelRange_ * yRelRange_; }
	else { neightbours[5] = -1; }

	return neightbours;
}


void VoxelGrid::growSpace(int startIndx, int roomnum, DataManager* h)
{
	// set up starting data
	std::vector<int> buffer = { startIndx };
	
	int totalRoomSize = 0;
	int totalCount = 100000;
	int counter = 100000;

	if (counter > VoxelLookup_.size())
	{
		counter = static_cast<int>(std::pow(10, static_cast<int>(std::log10(VoxelLookup_.size()))));
	}

	VoxelLookup_[startIndx]->setRoomNum(roomnum);
	VoxelLookup_[startIndx]->setOutside(roomnum);

	bool isOutSide = false;
	if (roomnum == 0) { isOutSide = true; }

	while (!buffer.empty())
	{
		std::vector<int> tempBuffer;
		for (const int currentIdx : buffer)
		{
			// only output process if outside is processed
			if (roomnum == 0)
			{
				if (totalCount <= 0)
				{
					std::cout.flush();
					std::cout << "\tSize: " << totalRoomSize << "\r";
					totalCount = 100000;
				}
			}

			voxel& externalVoxel = *VoxelLookup_[currentIdx];
	
			// find neighbours
			std::array<int, 6> neighbourIndxList = getDirNeighbours(currentIdx);

			for (int i = 0; i < neighbourIndxList.size(); i++)
			{
				int neighbourIdx = neighbourIndxList[i];
				if (neighbourIdx == -1) { continue; }

				voxel& potentialIntVoxel = *VoxelLookup_[neighbourIdx];
				// bypass if neighbour already part of a room
				if (potentialIntVoxel.getRoomNum() != -1) { continue; }

				// set trans faces if neighbour is intersecting
				if (potentialIntVoxel.getIsIntersecting())
				{
					potentialIntVoxel.setTransFace(i ^ 1);
					externalVoxel.setTransFace(i);
					if (roomnum == 0) { potentialIntVoxel.setIsShell(); }
					continue;
				}
				potentialIntVoxel.setOutside(isOutSide);
				potentialIntVoxel.setRoomNum(roomnum);

				tempBuffer.emplace_back(neighbourIdx);
			}
			totalRoomSize++;
			totalCount--;
		}
		buffer.clear();
		buffer = tempBuffer;
	}
	return;
}

void VoxelGrid::growExterior(DataManager* h)
{
	std::cout << CommunicationStringEnum::getString(CommunicationStringID::infoExteriorSpaceGrowing) << std::endl;

	if (VoxelLookup_[0] != nullptr && VoxelLookup_[0]->getIsIntersecting())
	{
		std::cout << CommunicationStringEnum::getString(CommunicationStringID::indentNoExteriorSpace) << std::endl;
		return;
	}

	// set up starting data
	std::vector<bool> evalList(VoxelLookup_.size());
	std::vector<int> buffer = { 0 };
	evalList[0] = true;

	int totalRoomSize = 0;
	int totalCount = 100000;
	int counter = 100000;

	if (counter > VoxelLookup_.size())
	{
		counter = static_cast<int>(std::pow(10, static_cast<int>(std::log10(VoxelLookup_.size()))));
	}


	while (!buffer.empty())
	{
		std::vector<int> tempBuffer;
		for (const int currentIdx : buffer)
		{
			totalCount--;
			if (totalCount <= 0)
			{
				std::cout.flush();
				std::cout << "\tSize: " << totalRoomSize << "\r";
				totalCount = 100000;
			}

			voxel* externalVoxel = VoxelLookup_[currentIdx].get();

			// find neighbours
			std::array<int, 6> neighbourIndxList = getDirNeighbours(currentIdx);

			std::vector<int> intFaceList;
			for (int i = 0; i < neighbourIndxList.size(); i++)
			{
				int neighbourIdx = neighbourIndxList[i];
				if (neighbourIdx == -1) { continue; }

				voxel* potentialIntVoxel = VoxelLookup_[neighbourIdx].get();

				// set trans faces if neighbour is intersecting
				if (potentialIntVoxel != nullptr && potentialIntVoxel->getIsIntersecting())
				{
					if (externalVoxel == nullptr) { intFaceList.emplace_back(i); }
					else { externalVoxel->setTransFace(i); }

					potentialIntVoxel->setTransFace(i ^ 1);
					potentialIntVoxel->setIsShell();
					continue;
				}
				if (evalList[neighbourIdx] == true) { continue; }
				evalList[neighbourIdx] = true;
				tempBuffer.emplace_back(neighbourIdx);
			}
			if (!intFaceList.empty())
			{
				auto boxel = std::make_unique<voxel>(voxel());
				for (int intFaceIndx : intFaceList)
				{
					boxel->setTransFace(intFaceIndx);
				}
				boxel->setOutside(true);
				boxel->setRoomNum(0);
				VoxelLookup_[currentIdx] = std::move(boxel);
				externalVoxel = VoxelLookup_[currentIdx].get();
			}
			
			totalRoomSize++;
			totalCount--;
		}
		buffer.clear();
		buffer = tempBuffer;
	}

	// populate voxelgrid for the growing of interiors
	for (size_t i = 0; i < evalList.size(); i++)
	{
		if (evalList[i]) { continue; }
		if (VoxelLookup_[i] != nullptr) { continue; }
		auto boxel = std::make_unique<voxel>(voxel());
		VoxelLookup_[i] = std::move(boxel);
	}

	std::cout << std::endl;
	std::cout << CommunicationStringEnum::getString(CommunicationStringID::indentExteriorSpaceGrown) << std::endl;
	return;
}

void VoxelGrid::growInterior(DataManager* h)
{
	std::cout << CommunicationStringEnum::getString(CommunicationStringID::infoInterioSpacesGrowing) << std::endl;
	roomSize_ = 1;

	for (int i = 0; i < totalVoxels_; i++)
	{
		if (VoxelLookup_[i] == nullptr) { continue; }
		if (!VoxelLookup_[i]->getIsIntersecting() && VoxelLookup_[i]->getRoomNum() == -1)
		{
			growSpace(i, roomSize_, h);
			roomSize_++;
		}
	}

	std::cout << CommunicationStringEnum::getString(CommunicationStringID::indentInteriorSpaceGrown) << std::endl;
}


void VoxelGrid::pairVoxels()
{
	std::cout << CommunicationStringEnum::getString(CommunicationStringID::infoPairVoxels) << std::endl;
	for (int i = 0; i < totalVoxels_; i++)
	{
		if (VoxelLookup_[i] == nullptr) {
			room2VoxelIdx_[0].emplace_back(i);
			continue;
		}
		if (VoxelLookup_[i]->getIsIntersecting()) { continue; }

		int roomNum = VoxelLookup_[i]->getRoomNum();
		if (roomNum == -1) { continue; }

		if (room2VoxelIdx_.find(roomNum) == room2VoxelIdx_.end())
		{
			room2VoxelIdx_[roomNum] = { i };
			continue;
		}
		room2VoxelIdx_[roomNum].emplace_back(i);
	}

	int buildingNum = 0;
	for (int i = 0; i < totalVoxels_; i++)
	{
		if (VoxelLookup_[i] == nullptr) { continue; }
		if (VoxelLookup_[i]->getIsIntersecting() && VoxelLookup_[i]->getBuildingNum() == -1)
		{
			markVoxelBuilding(i, buildingNum);
			buildingNum++;
		}
	}

	std::cout << CommunicationStringEnum::getString(CommunicationStringID::indentPairedVoxels) << std::endl;
	std::cout << "\t" << buildingNum << " buildings(s) found" << std::endl << std::endl;
}

void VoxelGrid::markVoxelBuilding(int startIndx, int buildnum) {

	VoxelLookup_[startIndx]->setBuildingNum(buildnum);
	std::vector<int> buffer = { startIndx };
	std::vector<int> potentialBuildingVoxels;

	while (true)
	{
		for (const int currentIdx : buffer)
		{
			std::array<int, 6> neighbours = getDirNeighbours(currentIdx);

			for (const int otherIdx : neighbours)
			{
				if (otherIdx == - 1) { continue; }

				voxel& otherVoxel = *VoxelLookup_[otherIdx];

				if (!otherVoxel.getIsIntersecting()) { continue; }
				if (otherVoxel.getBuildingNum() != -1) { continue; }

				otherVoxel.setBuildingNum(buildnum);
				potentialBuildingVoxels.emplace_back(otherIdx);
			}
		}
		if (potentialBuildingVoxels.size() == 0)
		{
			break;
		}
		buffer = potentialBuildingVoxels;
		potentialBuildingVoxels.clear();
	}
	return;
}


bool VoxelGrid::voxelBeamWindowIntersection(DataManager* h, int voxelIdx, int indxDir)
{
	SettingsCollection& settingsCollection = SettingsCollection::getInstance();

	double windowSearchDepth = 0.3;
	double windowArea = 0;

	// get a beam
	double voxelJump = settingsCollection.voxelSize();
	std::vector<int> voxelBeam;
	int loopingCurrentVoxelIdx = voxelIdx;
	voxelBeam.emplace_back(loopingCurrentVoxelIdx);

	bool windowFound = false;

	if (!VoxelLookup_[loopingCurrentVoxelIdx]->hasFace(indxDir))
	{
		return false;
	}
	while (true)
	{
		int loopingCurrentIndx = getNeighbour(loopingCurrentVoxelIdx, indxDir);
		if (loopingCurrentIndx == -1) { break; }

		loopingCurrentVoxelIdx = loopingCurrentIndx;
		if (!VoxelLookup_[loopingCurrentVoxelIdx]->getIsIntersecting()) { break; }

		voxelBeam.emplace_back(loopingCurrentVoxelIdx);
		voxelJump += settingsCollection.voxelSize();
		if (windowSearchDepth < voxelJump) { break; }
	}

	for (int beamVoxelIdx : voxelBeam)
	{
		std::vector<Value> intersectingValues = VoxelLookup_[beamVoxelIdx]->getInternalProductList();
		for (auto valueIt = intersectingValues.begin(); valueIt != intersectingValues.end(); ++valueIt)
		{
			//std::string productTypeName = h->getLookup(valueIt->second).getProductPtr()->data().type()->name();
			std::string productTypeName = h->getLookup(valueIt->second).getProductPtr()->declaration().name();

			if (productTypeName == "IfcDoor" || productTypeName == "IfcWindow")
			{
				return true;
			}
		}
	}
	return false;
}


int VoxelGrid::invertDir(int dirIndx) { //TODO: this seems voxel related code?
	if (dirIndx % 2 == 1) { return dirIndx - 1; }
	else { return dirIndx + 1; }
}

void VoxelGrid::setSemanticVoxelFace(DataManager* h, int voxelIdx, int dirIndx, const std::vector<Value>& intersectingValues)
{
	voxel* voxel = VoxelLookup_[voxelIdx].get();

	if (intersectingValues.size() == 0) { return; }

	bool hasWindowSurface = false;
	bool hasDoorSurface = false;
	bool hasNormalSurface = false;
	bool hasRoofSurface = false;

	for (auto valueIt = intersectingValues.begin(); valueIt != intersectingValues.end(); ++valueIt)
	{
		const IfcSchema::IfcProduct* intersectedProduct = h->getLookup(valueIt->second).getProductPtr();
		//std::string productTypeName = intersectedProduct->data().type()->name();
		std::string productTypeName = intersectedProduct->declaration().name();
		
		if (productTypeName == "IfcDoor")
		{
			hasDoorSurface = true;
			break;
		}

		if (productTypeName == "IfcWindow")
		{
			hasWindowSurface = true;
			break;
		}

		if (productTypeName == "IfcPlate")
		{
			if (helperFunctions::hasGlassMaterial(intersectedProduct))
			{
				hasWindowSurface = true;
				break;
			}
		}
		if (productTypeName == "IfcWall" || productTypeName == "IfcWallStandardCase" || productTypeName == "IfcColumn" || 
			productTypeName == "IfcBeam" || productTypeName == "IfcCovering" || productTypeName == "IfcMember" || 
			productTypeName == "IfcPlate" )
		{
			hasNormalSurface = true;
			continue;
		}
		if (productTypeName == "IfcRoof" || productTypeName == "IfcSlab")
		{
			hasRoofSurface = true;
			continue;
		}

	}

	if (hasDoorSurface)
	{
		if (dirIndx == 4)
		{
			voxel->addRoofSemantic(dirIndx);
			return;
		}
		if (dirIndx == 5)
		{
			voxel->addOuterCeilingSemantic(dirIndx);
			return;
		}

		voxel->addDoorSemantic(dirIndx);
		return;
	}

	if (hasWindowSurface)
	{
		voxel->addWindowSemantic(dirIndx);
		return;
	}

	if (hasRoofSurface)
	{
		auto midPoint = relPointToWorld(linearToRelative<BoostPoint3D>(voxelIdx));
		if (bg::get<2>(midPoint) < SettingsCollection::getInstance().footprintElevation())
		{
			voxel->addGroundSemantic(dirIndx);
		}
		else if (dirIndx == 5)
		{
			voxel->addOuterCeilingSemantic(dirIndx); 
		}
		else
		{
			voxel->addRoofSemantic(dirIndx); //TODO: add check to see if roof or not
		}
		return;
	}

	if (hasNormalSurface)
	{
		if (dirIndx == 4)
		{
			voxel->addRoofSemantic(dirIndx);
			return;
		}
		if (dirIndx == 5)
		{
			voxel->addOuterCeilingSemantic(dirIndx);
			return;
		}

		voxel->addWallSemantic(dirIndx);
		return;
	}
	return;
}


VoxelGrid::VoxelGrid(DataManager* h)
{
	SettingsCollection& settingsCollection = SettingsCollection::getInstance();
	if (!settingsCollection.requireVoxels()) { 
		std::cout << CommunicationStringEnum::getString(CommunicationStringID::infoNoVoxelizationReq) << std::endl;
		return; 
	} // no voxels needed for lod0.0 and 1.0 only
	
	// init the basic data
	try
	{
		init(h);
		populateVoxelGrid(h);
	}
	catch (const ErrorID& exceptionId)
	{
		throw exceptionId;
	}


	if (!settingsCollection.requireFullVoxels()) { 
		std::cout << CommunicationStringEnum::getString(CommunicationStringID::infoNocompleteVoxelizationReq) << std::endl;
		std::cout << std::endl;
		return; 
	}
	growExterior(h);

	if (settingsCollection.makeInterior())
	{
		growInterior(h);
	}
	pairVoxels();

	return;
}

bg::model::box<BoostPoint3D> VoxelGrid::getVoxelGeo(int voxelIdx) const
{
	auto midPoint = relPointToWorld(linearToRelative<BoostPoint3D>(voxelIdx));

	double offsetXY = voxelsize_ / 2;
	double offsetZ = voxelsize_ / 2;

	double xCoord = bg::get<0>(midPoint);
	double yCoord = bg::get<1>(midPoint);
	double zCoord = bg::get<2>(midPoint);

	BoostPoint3D lll(xCoord - offsetXY, yCoord - offsetXY, zCoord - offsetZ);
	BoostPoint3D urr(xCoord + offsetXY, yCoord + offsetXY, zCoord + offsetZ);
	return bg::model::box<BoostPoint3D>(lll, urr);
}

void VoxelGrid::computeSurfaceSemantics(DataManager* h)
{
	if (hasSemanticSurfaces_) { return; } // if already processed no reprocessing is needed
	hasSemanticSurfaces_ = true;

	// compute external surfaces
	std::vector<int> intersectingVoxels = getIntersectingVoxels();

	for (int currentIdx : intersectingVoxels)
	{
		voxel* currentVoxel = VoxelLookup_[currentIdx].get();

		// find the types
		for (int indxdir = 0; indxdir < 6; indxdir++)
		{
			int neighbourIndx = getNeighbour(currentIdx, indxdir);
			voxel& neighbourVoxel = *VoxelLookup_[neighbourIndx];
			int invertDir = indxdir ^ 1;
			if (neighbourVoxel.getIsIntersecting()) { continue; }
			if (neighbourVoxel.getRoomNum() == -1) { continue; }
			if (!neighbourVoxel.hasFace(invertDir)) { continue; }

			std::vector<Value> intersectingValues = currentVoxel->getInternalProductList();
			setSemanticVoxelFace(h, neighbourIndx, invertDir, intersectingValues);
		}	
	}
}

std::vector<int> VoxelGrid::getIntersectingVoxels() const
{
	std::vector<int> intersectingVoxels;

	for (size_t i = 0; i < VoxelLookup_.size(); i++)
	{
		voxel* currentVoxel = VoxelLookup_[i].get();
		if (currentVoxel == nullptr) { continue; }
		if (!currentVoxel->getIsIntersecting()) { continue; }
		if (currentVoxel->getBuildingNum() == -1) { continue; }
		intersectingVoxels.emplace_back(i);
	}
	return intersectingVoxels;
}

std::vector<int> VoxelGrid::getOuterIntersectingVoxels() const
{
	std::vector<int> intersectingVoxels;
	for (size_t i = 0; i < VoxelLookup_.size(); i++)
	{
		voxel* currentVoxel = VoxelLookup_[i].get();
		if (!currentVoxel->getIsIntersecting()) { continue; }
		if (currentVoxel->getBuildingNum() == -1) { continue; }

		std::array<int, 6> neighbours = getDirNeighbours(i);
		for (const int otherIdx : neighbours)
		{
			if (otherIdx == -1) { continue; }

			const voxel& otherVoxel = *VoxelLookup_.at(otherIdx);
			if (otherVoxel.getRoomNum() == 0)
			{
				intersectingVoxels.emplace_back(i);
				break;
			}
		}
	}
	return intersectingVoxels;
}


std::vector<int> VoxelGrid::getExternalVoxels() const
{
	std::vector<int> externalVoxels;
	for (size_t i = 0; i < VoxelLookup_.size(); i++)
	{
		voxel* currentVoxel = VoxelLookup_[i].get();
		if (currentVoxel == nullptr)
		{
			externalVoxels.emplace_back(i);
			continue;
		}
		if (currentVoxel->getIsInside()) { continue; }
		externalVoxels.emplace_back(i);
	}
	return externalVoxels;
}

std::vector<int> VoxelGrid::getInternalVoxels() const
{
	std::vector<int> internalVoxels;
	for (size_t i = 0; i < VoxelLookup_.size(); i++)
	{
		voxel* currentVoxel = VoxelLookup_[i].get();
		if (!currentVoxel->getIsInside()) { continue; }
		internalVoxels.emplace_back(i);
	}
	return internalVoxels;
}


std::vector<int> VoxelGrid::getVoxels() const
{
	std::vector<int> externalVoxels;
	for (size_t i = 0; i < VoxelLookup_.size(); i++)
	{
		externalVoxels.emplace_back(i);
	}
	return externalVoxels;
}

double VoxelGrid::getRoomArea(int roomNum)
{
	double voxelSize = SettingsCollection::getInstance().voxelSize();
	double voxelarea = voxelSize * voxelSize;

	double roomArea = 0;
	for (const auto& currentVoxel : VoxelLookup_)
	{
		if (currentVoxel->getRoomNum() != roomNum) { continue; }
		if (!currentVoxel->hasFace(5)) { continue; }
		roomArea += voxelarea;
	}
	return roomArea;
}

std::vector<gp_Pnt> VoxelGrid::getCornerPoints(int voxelIdx) const
{
	double offsetXY = voxelsize_ / 2;
	double offsetZ = voxelsize_ / 2;

	auto midPoint = relPointToWorld(linearToRelative<BoostPoint3D>(voxelIdx));

	double xCenCoord = bg::get<0>(midPoint);
	double yCenCoord = bg::get<1>(midPoint);
	double zCenCoord = bg::get<2>(midPoint);

	// make a pointlist 0 - 3 lower ring, 4 - 7 upper ring
	std::vector<gp_Pnt> pointList;
	pointList.emplace_back(gp_Pnt(xCenCoord - offsetXY, yCenCoord - offsetXY, zCenCoord - offsetZ));
	pointList.emplace_back(gp_Pnt(xCenCoord + offsetXY, yCenCoord - offsetXY, zCenCoord - offsetZ));
	pointList.emplace_back(gp_Pnt(xCenCoord + offsetXY, yCenCoord + offsetXY, zCenCoord - offsetZ));
	pointList.emplace_back(gp_Pnt(xCenCoord - offsetXY, yCenCoord + offsetXY, zCenCoord - offsetZ));
	pointList.emplace_back(gp_Pnt(xCenCoord + offsetXY, yCenCoord + offsetXY, zCenCoord + offsetZ));
	pointList.emplace_back(gp_Pnt(xCenCoord + offsetXY, yCenCoord - offsetXY, zCenCoord + offsetZ));
	pointList.emplace_back(gp_Pnt(xCenCoord - offsetXY, yCenCoord - offsetXY, zCenCoord + offsetZ));
	pointList.emplace_back(gp_Pnt(xCenCoord - offsetXY, yCenCoord + offsetXY, zCenCoord + offsetZ));

	return pointList;
}

gp_Pnt VoxelGrid::getCornerPoint(int voxelIdx, int indx) const
{
	double offsetXY = voxelsize_ / 2;
	double offsetZ = voxelsize_ / 2;

	auto midPoint = relPointToWorld(linearToRelative<BoostPoint3D>(voxelIdx));

	double xCenCoord = bg::get<0>(midPoint);
	double yCenCoord = bg::get<1>(midPoint);
	double zCenCoord = bg::get<2>(midPoint);

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

std::vector<gp_Pnt> VoxelGrid::getPlanePoints(int indx) const
{
	auto boxelGeo = getVoxelGeo(indx);

	auto minPoint = helperFunctions::Point3DBTO(boxelGeo.min_corner());
	auto maxPoint = helperFunctions::Point3DBTO(boxelGeo.max_corner());

	std::vector<gp_Pnt> pointList;
	double offset = voxelsize_ / 2;
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

gp_Pnt VoxelGrid::getMidPoint(int voxelIndx) const 
{
	return helperFunctions::Point3DBTO(relPointToWorld(linearToRelative<BoostPoint3D>(voxelIndx)));
}

std::vector<std::pair<std::vector<TopoDS_Edge>, CJObjectID>> VoxelGrid::getDirectionalFaces(int dirIndx, double angle, int roomNum)
{
	if (room2VoxelIdx_.find(roomNum) == room2VoxelIdx_.end()) { return {}; }

	const std::vector<int>& roomVoxLoc = room2VoxelIdx_.at(roomNum);

	std::vector<std::pair<std::vector<TopoDS_Edge>, CJObjectID>> clusteredEdges = {};

	std::vector<bool> evaluated(VoxelLookup_.size());
	std::vector<bool> evaluatedGrowth(VoxelLookup_.size());
	std::vector<int> allowedNeighbourDir = {};

	if (dirIndx == 0 || dirIndx == 1) { allowedNeighbourDir = { 2,3,4,5 }; }
	if (dirIndx == 2 || dirIndx == 3) { allowedNeighbourDir = { 0,1,4,5 }; }
	if (dirIndx == 4 || dirIndx == 5) { allowedNeighbourDir = { 0,1,2,3 }; }

	int searchStartIdx = 0;
	while (true)
	{
		// find the start of the face growth
		std::vector<int> buffer = {};
		CJObjectID search4Type = CJObjectID::CJTypeNone;

		for (size_t i = searchStartIdx; i <roomVoxLoc.size(); i++)
		{
			int currentIdx = roomVoxLoc.at(i);
			const voxel& potentialVoxel = *VoxelLookup_[currentIdx];
			if (evaluated[currentIdx] == true) { continue; }

			if (&potentialVoxel == nullptr) { continue; }
			if (potentialVoxel.getRoomNum() != roomNum) { continue; }
			if (!potentialVoxel.hasFace(dirIndx)) { continue; }

			search4Type = potentialVoxel.faceType(dirIndx);
			buffer.emplace_back(currentIdx);
			evaluated[currentIdx] = true;
			evaluatedGrowth[currentIdx] = true;
			searchStartIdx = i;
			break;
		}
		if (!buffer.size()) { break; }
		// find edges for faces
		std::vector<TopoDS_Edge> edgeList = {};

		while (buffer.size())
		{
			// growth
			std::vector<int> tempBuffer = {};
			for (auto bufferIT = buffer.begin(); bufferIT != buffer.end(); bufferIT++)
			{
				int bufferIndx = *bufferIT;
				const voxel& currentVoxel = *VoxelLookup_[bufferIndx];

				bool isEdge = false;

				std::array<int, 6> neighbourIndxList = getDirNeighbours(bufferIndx);
				for (size_t i = 0; i < 4; i++)
				{
					int neighbourIndx = neighbourIndxList[allowedNeighbourDir[i]];
					if (neighbourIndx == -1) { continue; }
					const voxel& neighbourVoxel = *VoxelLookup_[neighbourIndx];

					// find neighbour to grow into 
					if ( &neighbourVoxel != nullptr && 
						neighbourVoxel.hasFace(dirIndx) && 
						neighbourVoxel.faceType(dirIndx) == search4Type )
					{
						if (evaluatedGrowth[neighbourIndx] == true) { continue; }
						evaluatedGrowth[neighbourIndx] = true;
						tempBuffer.emplace_back(neighbourIndx);
						continue;
					}

					std::vector<gp_Pnt> voxelPoints = getCornerPoints(bufferIndx);
					// create edge based on the normal dir
					if (dirIndx == 1)
					{
						if (i == 0)
						{
							edgeList.emplace_back(BRepBuilderAPI_MakeEdge(voxelPoints[1], voxelPoints[5]));
						}
						else if (i == 1)
						{
							edgeList.emplace_back(BRepBuilderAPI_MakeEdge(voxelPoints[4], voxelPoints[2]));
						}
						else if (i == 2)
						{
							edgeList.emplace_back(BRepBuilderAPI_MakeEdge(voxelPoints[2], voxelPoints[1]));
						}
						else if (i == 3)
						{
							edgeList.emplace_back(BRepBuilderAPI_MakeEdge(voxelPoints[5], voxelPoints[4]));
						}
					}
					else if (dirIndx == 0)
					{
						if (i == 0)
						{
							edgeList.emplace_back(BRepBuilderAPI_MakeEdge(voxelPoints[6], voxelPoints[0]));
						}
						else if (i == 1)
						{
							edgeList.emplace_back(BRepBuilderAPI_MakeEdge(voxelPoints[3], voxelPoints[7]));
						}
						else if (i == 2)
						{
							edgeList.emplace_back(BRepBuilderAPI_MakeEdge(voxelPoints[0], voxelPoints[3]));
						}
						else if (i == 3)
						{
							edgeList.emplace_back(BRepBuilderAPI_MakeEdge(voxelPoints[7], voxelPoints[6]));
						}
					}
					else if (dirIndx == 3)
					{
						if (i == 0)
						{
							edgeList.emplace_back(BRepBuilderAPI_MakeEdge(voxelPoints[7], voxelPoints[3]));
						}
						else if (i == 1)
						{
							edgeList.emplace_back(BRepBuilderAPI_MakeEdge(voxelPoints[2], voxelPoints[4]));
						}
						else if (i == 2)
						{
							edgeList.emplace_back(BRepBuilderAPI_MakeEdge(voxelPoints[3], voxelPoints[2]));
						}
						else if (i == 3)
						{
							edgeList.emplace_back(BRepBuilderAPI_MakeEdge(voxelPoints[4], voxelPoints[7]));
						}
					}
					else if (dirIndx == 2)
					{
						if (i == 0)
						{
							edgeList.emplace_back(BRepBuilderAPI_MakeEdge(voxelPoints[0], voxelPoints[6]));
						}
						else if (i == 1)
						{
							edgeList.emplace_back(BRepBuilderAPI_MakeEdge(voxelPoints[5], voxelPoints[1]));
						}
						else if (i == 2)
						{
							edgeList.emplace_back(BRepBuilderAPI_MakeEdge(voxelPoints[1], voxelPoints[0]));
						}
						else if (i == 3)
						{
							edgeList.emplace_back(BRepBuilderAPI_MakeEdge(voxelPoints[6], voxelPoints[5]));
						}
					}
					else if (dirIndx == 5)
					{
						if (i == 0)
						{
							edgeList.emplace_back(BRepBuilderAPI_MakeEdge(voxelPoints[6], voxelPoints[7]));
						}
						else if (i == 1)
						{
							edgeList.emplace_back(BRepBuilderAPI_MakeEdge(voxelPoints[4], voxelPoints[5]));
						}
						else if (i == 2)
						{
							edgeList.emplace_back(BRepBuilderAPI_MakeEdge(voxelPoints[6], voxelPoints[5]));
						}
						else if (i == 3)
						{
							edgeList.emplace_back(BRepBuilderAPI_MakeEdge(voxelPoints[7], voxelPoints[4]));
						}
					}
					else if (dirIndx == 4)
					{
						if (i == 0)
						{
							edgeList.emplace_back(BRepBuilderAPI_MakeEdge(voxelPoints[3], voxelPoints[0]));
						}
						else if (i == 1)
						{
							edgeList.emplace_back(BRepBuilderAPI_MakeEdge(voxelPoints[2], voxelPoints[1]));
						}
						else if (i == 2)
						{
							edgeList.emplace_back(BRepBuilderAPI_MakeEdge(voxelPoints[1], voxelPoints[0]));
						}
						else if (i == 3)
						{
							edgeList.emplace_back(BRepBuilderAPI_MakeEdge(voxelPoints[3], voxelPoints[2]));
						}
					}
				}
			}
			buffer = tempBuffer;
		}

		for (size_t i = 0; i < roomVoxLoc.size(); i++)
		{
			int indx = roomVoxLoc[i];
			if (evaluated[indx] == false && evaluatedGrowth[indx] == true) { evaluated[indx] = true; }
			evaluatedGrowth[indx] = false;
		}

		if (!edgeList.empty())
		{
			clusteredEdges.emplace_back(std::make_pair(edgeList, search4Type));
		}
	}
	return clusteredEdges;
}

gp_Pnt VoxelGrid::getPointInRoom(int roomNum)
{
	for (size_t i = 0; i < VoxelLookup_.size(); i++)
	{
		voxel* currentVoxel = VoxelLookup_[i].get();
		if (currentVoxel == nullptr) { continue; }
		if (currentVoxel->getRoomNum() == roomNum)
		{
			auto midPoint = relPointToWorld(linearToRelative<BoostPoint3D>(i));
			return helperFunctions::Point3DBTO(midPoint);
		}
	}
	return gp_Pnt();
}


std::vector<int> VoxelGrid::getTopBoxelIndx() 
{
	std::vector<int> voxelIndx;

	for (int i = xRelRange_ * yRelRange_ * zRelRange_ - xRelRange_ * yRelRange_;
		i < xRelRange_ * yRelRange_ * zRelRange_ - 1;
		i++)
	{
		voxelIndx.emplace_back(i);
	}
	return voxelIndx;
}


std::vector<int> VoxelGrid::getVoxelPlate(double platelvl) 
{
	double voxelCount = (double) VoxelLookup_.size();
	double zlvls = voxelCount / (static_cast<double>(xRelRange_) * static_cast<double>(yRelRange_));
	double smallestDistanceToLvl = 999999;

	int plateVoxelLvl;

	for (int i = 0; i < zlvls; i++)
	{
		int indx = i * xRelRange_ * yRelRange_;
		auto midPoint = relPointToWorld(linearToRelative<BoostPoint3D>(i));
		double coreHeight = bg::get<2>(midPoint);
		double distanceToLvl = abs(platelvl - coreHeight);

		if (distanceToLvl < smallestDistanceToLvl)
		{
			smallestDistanceToLvl = distanceToLvl;
			plateVoxelLvl = i;
			continue;
		}
		break;
	}

	int lvl = plateVoxelLvl * xRelRange_ * yRelRange_;
	int topLvL = (plateVoxelLvl + 1) * xRelRange_ * yRelRange_ - 1;

	std::vector<int> plateVoxels;
	for (int i = 0; i < VoxelLookup_.size(); i++)
	{
		int currentVoxelIdx = i;
		if (currentVoxelIdx < lvl || currentVoxelIdx > topLvL) { continue; }
		plateVoxels.emplace_back(currentVoxelIdx);
	}
	return plateVoxels;
}

int VoxelGrid::getNeighbour(int voxelIndx, int dir)
{
	std::array<int, 6> neighbours = getDirNeighbours(voxelIndx);

	if (neighbours.size() == 0)
	{
		return -1;
	}
	return neighbours[dir];
}

int VoxelGrid::getLowerNeighbour(int voxelIndx, bool connect6)
{
	gp_Pnt loc3D = linearToRelative<gp_Pnt>(voxelIndx);

	bool zSmall = loc3D.Z() - 1 >= 0;

	if (!zSmall) { return -1; }

	return voxelIndx - (xRelRange_) * (yRelRange_);
}

int VoxelGrid::getCloseByVoxel(const gp_Pnt& targetPoint)
{
	BoostPoint3D relPoint = worldToRelPoint(helperFunctions::Point3DOTB(targetPoint));
	return relativeToLinear(relPoint);
}
