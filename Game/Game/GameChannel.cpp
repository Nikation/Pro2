#include "GameChannel.h"
#include "GameProtocol.h"
#include "GameRole.h"

GameChannel::GameChannel(int _fd)
	:ZinxTcpData(_fd)
{
}

GameChannel::~GameChannel()
{
	//��GameChannel�ͷŵ�ʱ��ҲҪͬʱ�ͷ�Protocol �Լ�Role����
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
	//������GameChannel��ʱ��,������������ GameProtocol �Լ�GameRole Ҳһ������
	auto channel = new GameChannel(_fd);
	auto protocol = new GameProtocol;
	auto role = new GameRole;

	//������������֮��Ĺ���
	channel->mProtocol = protocol;
	channel->mRole = role;

	protocol->mChannel = channel;
	protocol->mRole = role;

	role->mProtocol = protocol;
	role->mChannel = channel;


	//��role�Լ�protocol ��ӵ� zinx����й���
	ZinxKernel::Zinx_Add_Proto(*protocol);
	ZinxKernel::Zinx_Add_Role(*role);

	return channel;
}
