#pragma once
#include "zinx.h"
class GameChannel;
class GameRole;
class GameProtocol :
	public Iprotocol
{
public:
	GameProtocol();
	virtual ~GameProtocol();

	/*原始数据和业务数据相互函数，开发者重写该函数，实现协议*/
	virtual UserData *raw2request(std::string _szInput)override;
	
	/*原始数据和业务数据相互函数，开发者重写该函数，实现协议*/
	virtual std::string *response2raw(UserData &_oUserData)override;

	GameChannel *mChannel;
	GameRole *mRole;


protected:
    /*获取处理角色对象函数，开发者应该重写该函数，用来指定当前产生的用户数据消息应该传递给哪个角色处理*/
	virtual Irole *GetMsgProcessor(UserDataMsg &_oUserDataMsg)override;

	/*获取发送通道函数，开发者应该重写该函数，用来指定当前字节流应该由哪个通道对象发出*/
	virtual Ichannel *GetMsgSender(BytesMsg &_oBytes)override;


	//存储上次遗留下来的数据的buf
	std::string mLastBuff;

};

