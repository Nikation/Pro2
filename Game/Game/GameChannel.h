#pragma once
#include "ZinxTCP.h"
class GameProtocol;
class GameRole;

class GameChannel :
	public ZinxTcpData
{
public:
	GameChannel(int _fd);
	virtual ~GameChannel();
	virtual AZinxHandler * GetInputNextStage(BytesMsg & _oInput)override;


	GameProtocol *mProtocol;
	GameRole *mRole;
};

//GameChannel的工厂类
class GameChannelFactory : public IZinxTcpConnFact
{
public:
	virtual ZinxTcpData *CreateTcpDataChannel(int _fd)override;
};


