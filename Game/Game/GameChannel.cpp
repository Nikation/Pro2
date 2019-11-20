#include "GameChannel.h"
#include "GameProtocol.h"
#include "GameRole.h"

GameChannel::GameChannel(int _fd)
	:ZinxTcpData(_fd)
{
}

GameChannel::~GameChannel()
{
	//在GameChannel释放的时候也要同时释放Protocol 以及Role对象
	if (mProtocol)
	{
		ZinxKernel::Zinx_Del_Proto(*mProtocol);
		delete mProtocol;
	}
	if (mRole)
	{
		ZinxKernel::Zinx_Del_Role(*mRole);
		delete mRole;
	}
}

AZinxHandler * GameChannel::GetInputNextStage(BytesMsg & _oInput)
{
	return mProtocol;
}

ZinxTcpData * GameChannelFactory::CreateTcpDataChannel(int _fd)
{
	//当创建GameChannel的时候,其他两个对象 GameProtocol 以及GameRole 也一并创建
	auto channel = new GameChannel(_fd);
	auto protocol = new GameProtocol;
	auto role = new GameRole;

	//建立以上三者之间的关联
	channel->mProtocol = protocol;
	channel->mRole = role;

	protocol->mChannel = channel;
	protocol->mRole = role;

	role->mProtocol = protocol;
	role->mChannel = channel;


	//将role以及protocol 添加到 zinx框架中管理
	ZinxKernel::Zinx_Add_Proto(*protocol);
	ZinxKernel::Zinx_Add_Role(*role);

	return channel;
}
