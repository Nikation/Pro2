#include "WorldManager.h"

WorldManager WorldManager::smManager;
WorldManager & WorldManager::GetInstance()
{
	return smManager;
}

AOI_World * WorldManager::GetWorld(int id)
{
	//����ID,���صڼ�������
	return mVecWorld[id];
}

WorldManager::WorldManager()
{
	//һ��ʼ�ʹ�������AOI���糡��
	this->mVecWorld.resize(3);
	mVecWorld[0] = nullptr;
	mVecWorld[1] = new AOI_World(85,410,75,400,10,20);
	mVecWorld[1]->mWorldId = 1;
	//ս������,û��AOI,��ʱ����x���y����зֶ���Ϊ1 ,ֻ��һ������
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
