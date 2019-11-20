#include "WorldManager.h"

WorldManager WorldManager::smManager;
WorldManager & WorldManager::GetInstance()
{
	return smManager;
}

AOI_World * WorldManager::GetWorld(int id)
{
	//根据ID,返回第几个世界
	return mVecWorld[id];
}

WorldManager::WorldManager()
{
	//一开始就创建两个AOI世界场景
	this->mVecWorld.resize(3);
	mVecWorld[0] = nullptr;
	mVecWorld[1] = new AOI_World(85,410,75,400,10,20);
	mVecWorld[1]->mWorldId = 1;
	//战斗场景,没有AOI,此时设置x轴和y轴的切分段数为1 ,只有一个格子
	mVecWorld[2] = new AOI_World(0,140,0,140,1,1);
	mVecWorld[2]->mWorldId = 2;
}


WorldManager::~WorldManager()
{
	for (auto p : mVecWorld)
	{
		if (p != nullptr)
			delete p;
	}
}
