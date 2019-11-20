#include "GameRole.h"
#include "GameMsg.h"
#include <iostream>
#include "GameChannel.h"
#include "GameProtocol.h"
#include <random>
#include <algorithm>
#include "RandomName.h"
#include "WorldManager.h"
using namespace pb;
using namespace std;

int GameRole::smRoleCount = 1;

//定义随机数引擎
default_random_engine g_rand_engine(time(nullptr));

GameRole::GameRole()
{
	//初始化ID以及名字
	this->mPlayerID = smRoleCount++;
	//this->mPlayerName = string("Player_") + to_string(mPlayerID);
	//从随机姓名池子中获取一个随机名字
	this->mPlayerName = RandomName::GetInstance().GetName();

	//初始位置
	x = 100+(g_rand_engine()%20);
	y = 0;
	z = 100+(g_rand_engine()%20);
	v = 0;

	//hp 满血是1000
	hp = 1000;

}

GameRole::~GameRole()
{
	//归还名字
	RandomName::GetInstance().ReleaseName(this->mPlayerName);
}

bool GameRole::Init()
{
	//当玩家上线的时候就会触发这个函数
	//在这里实现一些上线的业务逻辑
	//发送当前玩家ID以及姓名给玩家
	auto msg = MakeLogonSyncPid();
	//将消息发送出去
	ZinxKernel::Zinx_SendOut(*msg, *mProtocol);

	//发给当前玩家初始出生点位置信息
	msg = MakeInitPosBroadcast();
	ZinxKernel::Zinx_SendOut(*msg, *mProtocol);

	//上线的时候要将当前玩家添加到AOI世界里边
	this->mCurrentWorld = WorldManager::GetInstance().GetWorld(1);
	mCurrentWorld->AddPlayer(this);
	
	//创建周边玩家的消息
	msg = MakeSurPlays();
	ZinxKernel::Zinx_SendOut(*msg, *mProtocol);

	//获取世界所有玩家,发送上线消息给对方,也要发送一条对方上线的消息给自己
	auto players = mCurrentWorld->GetSurPlayers(this);

	for (auto &r : players)
	{
		if (r == this)
			continue;
		auto role = dynamic_cast<GameRole*>(r);
		cout << "Init sur players : " << role->mPlayerName << endl;

		//产生上线消息
		msg = MakeInitPosBroadcast();
		//注意发送给对方的protocol层
		ZinxKernel::Zinx_SendOut(*msg, *role->mProtocol);
	}


	return true;
}

//这个函数主要就是处理由protocol 传递过来的 UserData消息
UserData * GameRole::ProcMsg(UserData & _poUserData)
{
	GameMsg &msg = dynamic_cast<GameMsg&>(_poUserData);
	for(auto single : msg.m_MsgList)
	{
		//解析客户端传递过来的每一条消息
		switch (single->m_MsgType)
		{
		case GameSingleTLV::GAME_MSG_NEW_POSTION:
		{
			auto pbMsg = dynamic_cast<pb::Position*>(single->m_poGameMsg);
			this->ProcNewPosition(pbMsg->x(), pbMsg->y(), pbMsg->z(), pbMsg->v());
			break;
		}
		case GameSingleTLV::GAME_MSG_TALK_CONTENT:
		{
			//聊天消息
			auto pbMsg = dynamic_cast<pb::Talk*>(single->m_poGameMsg);
			this->ProcTalkContent(pbMsg->content());
			break;
		}
		case GameSingleTLV::GAME_MSG_CHANGE_WORLD:
		{
			//客户端切换场景的请求
			auto pbMsg = dynamic_cast<pb::ChangeWorldRequest*>(single->m_poGameMsg);
			this->ProcChangeWorld(pbMsg->srcid(), pbMsg->targetid());
			break;
		}
		case GameSingleTLV::GAME_MSG_SKILL_TRIGGER:
		{
			//客户端触发了技能
			auto pbMsg = dynamic_cast<pb::SkillTrigger*>(single->m_poGameMsg);
			this->ProcSkillTrigger(pbMsg);
			break;
		}
		case GameSingleTLV::GAME_MSG_SKILL_CONTACT:
		{
			//玩家技能命中
			auto pbMsg = dynamic_cast<pb::SkillContact*>(single->m_poGameMsg);
			this->ProcSkillContact(pbMsg);
			break;
		}
		default:
			break;
		}
	}
	return nullptr;
}

void GameRole::Fini()
{
	//当玩家下线的时候,这个函数就会被调用,在这里实现下线的逻辑即可
	//下线的时候不要忘记从AOI中删除当前玩家
	mCurrentWorld->DelPlayer(this);
	auto players = mCurrentWorld->GetSurPlayers(this);
	for (auto &r : players)
	{
		if (r == this)
			continue;
		auto role = dynamic_cast<GameRole*>(r);
		auto msg = MakeLogoffSyncPid();
		ZinxKernel::Zinx_SendOut(*msg, *role->mProtocol);
	}
}

void GameRole::ProcNewPosition(float _x, float _y, float _z, float _v)
{
	//如果当前AOI坐标变化了,这时候就要切换格子
	if (mCurrentWorld->GridChanged(this, _x, _z))
	{
		mCurrentWorld->DelPlayer(this);
		//获取旧的周边玩家集合
		auto oldList = mCurrentWorld->GetSurPlayers(this);
		//变换坐标
		this->x = _x;
		this->y = _y;
		this->z = _z;
		this->v = _v;

		//获取新的周边玩家集合
		auto newList = mCurrentWorld->GetSurPlayers(this);

		//视野消失的处理
		this->ViewDisappear(oldList, newList);
		this->ViewAppear(oldList, newList);

		mCurrentWorld->AddPlayer(this);
	}
	//当玩家移动的时候会收到移动消息,就会调用这个函数
	this->x = _x;
	this->y = _y;
	this->z = _z;
	this->v = _v;

	auto players = mCurrentWorld->GetSurPlayers(this);
	for (auto &r : players)
	{
		if (r == this)
			continue;
		auto role = dynamic_cast<GameRole*>(r);
		auto msg = MakeNewPosBroadcast();
		ZinxKernel::Zinx_SendOut(*msg, *role->mProtocol);
	}
}

void GameRole::ProcTalkContent(std::string content)
{
	//获取当前服务器所有的玩家列表,然后广播消息
	auto players = ZinxKernel::Zinx_GetAllRole();
	for (auto &r : players)
	{
		auto role = dynamic_cast<GameRole*>(r);
		auto msg = MakeTalkBroadcast(content);
		ZinxKernel::Zinx_SendOut(*msg, *role->mProtocol);
	}
}

void GameRole::ProcChangeWorld(int srcId, int targetWorldId)
{
	if (srcId == targetWorldId)
		return;
	//实现切换场景的所有业务
	//当前AOI世界也要进行切换
	//下线当前AOI世界
	mCurrentWorld->DelPlayer(this);
	//告诉周边玩家我下线
	auto players = mCurrentWorld->GetSurPlayers(this);
	for (auto &r : players)
	{
		if (r == this)
			continue;
		auto role = dynamic_cast<GameRole*>(r);
		auto msg = MakeLogoffSyncPid();
		ZinxKernel::Zinx_SendOut(*msg, *role->mProtocol);
	}

	//产生一个随机出生点位置
	if (targetWorldId == 1)
	{
		x = 100 + (g_rand_engine() % 20);
		y = 0;
		z = 100 + (g_rand_engine() % 20);
		v = 0;
		hp = 1000;
	}
	if (targetWorldId == 2)
	{
		//战斗场景的区间是 x [0,140]  z [0,140]
		//最后控制出生点在	10-130之间即可
		x = 10 + g_rand_engine()%120;
		y = 0;
		z = 10 + g_rand_engine()%120;
	}

	//上线新的AOI世界
	mCurrentWorld = WorldManager::GetInstance().GetWorld(targetWorldId);
	mCurrentWorld->AddPlayer(this);
	players = mCurrentWorld->GetSurPlayers(this);
	for (auto &r : players)
	{
		if (r == this)
			continue;
		auto role = dynamic_cast<GameRole*>(r);
		auto msg = MakeInitPosBroadcast();
		ZinxKernel::Zinx_SendOut(*msg, *role->mProtocol);
	}
	//注意顺序不能变
	//1 先发送场景切换的响应
	//2 再发送场景里边的周边玩家

	//产生切换场景的响应消息
	auto msg = MakeChangeWorldResponse(srcId, targetWorldId);
	ZinxKernel::Zinx_SendOut(*msg, *mProtocol);

	//告诉自己新的场景里边周边有什么玩家
	msg = MakeSurPlays();
	ZinxKernel::Zinx_SendOut(*msg, *mProtocol);
}

void GameRole::ProcSkillTrigger(pb::SkillTrigger * trigger)
{
	//收到客户端技能触发的消息之后,就在这里进行广播,广播给周边玩家

	if (trigger->pid ()!= this->mPlayerID)
	{
		//报错
	}
	//验证子弹...
	//记录子弹的ID...

	auto players = mCurrentWorld->GetSurPlayers(this);
	for (auto &r : players)
	{
		auto role = dynamic_cast<GameRole*>(r);
		auto msg = MakeSkillTrigger(trigger);
		ZinxKernel::Zinx_SendOut(*msg, *role->mProtocol);
	}
}

void GameRole::ProcSkillContact(pb::SkillContact * contact)
{
	cout << "Get new contact src:" << contact->srcpid() << " target:" << contact->targetpid() << endl;;
	//在这里触发技能命中,逻辑
	//做保护
	if (contact->srcpid() != this->mPlayerID)
	{
		//不用管,只管当前玩家自己的命中消息
		return;
	}
	int targetPlayerId = contact->targetpid();

	//防外挂验证
	//.....

	auto tmpContactMsg = new SkillContact(*contact);

	//确定到底命中了哪个玩家
	GameRole *targetRole = nullptr;
	auto players = mCurrentWorld->GetSurPlayers(this);
	for (auto p : players)
	{
		auto r = dynamic_cast<GameRole*>(p);
		if (r->mPlayerID == tmpContactMsg->targetpid())
		{
			if (r->mPlayerID != tmpContactMsg->targetpid())
			{
				cout << "error: r->mPlayerId"<<r->mPlayerID<<" targetpid"<<tmpContactMsg->targetpid() << endl;
				continue;
			}
			targetRole = r;
			break;
		}
	}

	if (targetRole == nullptr)
	{
		//没有找到受害者,这里肯定有问题
		return;
	}

	if (targetRole->mPlayerID != contact->targetpid())
	{
		cout << "error" << endl;
	}

	//计算受到的伤害
	int attackValue = 300 + g_rand_engine() % 300;

	targetRole->hp -= attackValue;


	//继续广播给所有玩家命中
	//修改一下消息中的hp值
	auto pos = tmpContactMsg->mutable_contactpos();
	pos->set_bloodvalue(targetRole->hp);
	pos->set_x(pos->x());
	pos->set_y(pos->y());
	pos->set_z(pos->z());
	pos->set_v(pos->v());

	//广播给所有玩家当前命中了,并且扣了血量
	for (auto &r : players)
	{
		auto role = dynamic_cast<GameRole*>(r);
		auto msg = MakeSkillContact(tmpContactMsg);
		ZinxKernel::Zinx_SendOut(*msg, *role->mProtocol);
	}

	delete tmpContactMsg;


	//判断受害者 hp值<=0 ,就切换场景,回到第一个场景里边
	if (targetRole->hp <= 0)
	{
		if (targetRole->mPlayerID != contact->targetpid())
		{
			cout << "error" << endl;
		}
		cout << "Player id : " << targetRole->mPlayerID << " name : " << targetRole->mPlayerName
			<< " die in hp :" << targetRole->hp << endl;
		targetRole->ProcChangeWorld(targetRole->mCurrentWorld->mWorldId, 1);
	}


}

void GameRole::ViewDisappear(std::list<AOI_Player*>& oldList, std::list<AOI_Player*>& newList)
{
	//集合运算 旧的集合 - 新的集合 = 要做视野消失的玩家
	//要对集合先排序,排序前还要确保集合是能够随机访问
	vector<AOI_Player*> oldVec(oldList.begin(), oldList.end());
	vector<AOI_Player*> newVec(newList.begin(), newList.end());

	sort(oldVec.begin(), oldVec.end());
	sort(newVec.begin(), newVec.end());

	vector<AOI_Player*> diff;
	//集合运算,差集
	std::set_difference(oldVec.begin(), oldVec.end(), newVec.begin(), newVec.end(),
		std::inserter(diff, diff.begin()));

	for (auto &r : diff)
	{
		auto role = dynamic_cast<GameRole*>(r);

		auto msg = this->MakeLogoffSyncPid();
		ZinxKernel::Zinx_SendOut(*msg, *role->mProtocol);

		msg = role->MakeLogoffSyncPid();
		ZinxKernel::Zinx_SendOut(*msg, *this->mProtocol);
	}

}

void GameRole::ViewAppear(std::list<AOI_Player*>& oldList, std::list<AOI_Player*>& newList)
{
	//集合运算 新的集合 - 旧的集合 = 要做视野出现的玩家
	//要对集合先排序,排序前还要确保集合是能够随机访问
	vector<AOI_Player*> oldVec(oldList.begin(), oldList.end());
	vector<AOI_Player*> newVec(newList.begin(), newList.end());

	sort(oldVec.begin(), oldVec.end());
	sort(newVec.begin(), newVec.end());

	vector<AOI_Player*> diff;
	//集合运算,差集
	std::set_difference(newVec.begin(), newVec.end(), oldVec.begin(), oldVec.end(),
		std::inserter(diff, diff.begin()));

	for (auto &r : diff)
	{
		auto role = dynamic_cast<GameRole*>(r);

		auto msg = this->MakeInitPosBroadcast();
		ZinxKernel::Zinx_SendOut(*msg, *role->mProtocol);

		msg = role->MakeInitPosBroadcast();
		ZinxKernel::Zinx_SendOut(*msg, *this->mProtocol);
	}

}

//构造上线同步玩家id和姓名的消息
GameMsg * GameRole::MakeLogonSyncPid()
{
	auto pbMsg = new SyncPid;
	pbMsg->set_pid(this->mPlayerID);
	pbMsg->set_username(this->mPlayerName);

	auto single = new GameSingleTLV(GameSingleTLV::GAME_MSG_LOGON_SYNCPID, pbMsg);
	auto retMsg = new GameMsg;
	retMsg->m_MsgList.push_back(single);
	return retMsg;
}

GameMsg * GameRole::MakeTalkBroadcast(std::string _talkContent)
{
	auto pbMsg = new BroadCast;
	pbMsg->set_pid(this->mPlayerID);
	pbMsg->set_username(this->mPlayerName);
	/*根据Tp不同，BroadCast消息会包含：
      1 聊天内容（Content）
      2 初始位置(P)
      4 新位置P
    */
	pbMsg->set_tp(1);
	pbMsg->set_content(_talkContent);

	auto single = new GameSingleTLV(GameSingleTLV::GAME_MSG_BROADCAST, pbMsg);
	auto retMsg = new GameMsg;
	retMsg->m_MsgList.push_back(single);
	return retMsg;
}

GameMsg * GameRole::MakeInitPosBroadcast()
{
	auto pbMsg = new BroadCast;
	pbMsg->set_pid(this->mPlayerID);
	pbMsg->set_username(this->mPlayerName);
	/*根据Tp不同，BroadCast消息会包含：
      1 聊天内容（Content）
      2 初始位置(P)
      4 新位置P
    */
	pbMsg->set_tp(2);
	//自动创建一个新的Position对象并且返回指针
	auto pos = pbMsg->mutable_p();
	pos->set_bloodvalue(hp);
	pos->set_x(x);
	pos->set_y(y);
	pos->set_z(z);
	pos->set_v(v);

	auto single = new GameSingleTLV(GameSingleTLV::GAME_MSG_BROADCAST, pbMsg);
	auto retMsg = new GameMsg;
	retMsg->m_MsgList.push_back(single);
	return retMsg;
}

GameMsg * GameRole::MakeNewPosBroadcast()
{
	auto pbMsg = new BroadCast;
	pbMsg->set_pid(this->mPlayerID);
	pbMsg->set_username(this->mPlayerName);
	/*根据Tp不同，BroadCast消息会包含：
      1 聊天内容（Content）
      2 初始位置(P)
      4 新位置P
    */
	pbMsg->set_tp(4);
	//自动创建一个新的Position对象并且返回指针
	auto pos = pbMsg->mutable_p();
	pos->set_bloodvalue(hp);
	pos->set_x(x);
	pos->set_y(y);
	pos->set_z(z);
	pos->set_v(v);

	auto single = new GameSingleTLV(GameSingleTLV::GAME_MSG_BROADCAST, pbMsg);
	auto retMsg = new GameMsg;
	retMsg->m_MsgList.push_back(single);
	return retMsg;
}

GameMsg * GameRole::MakeLogoffSyncPid()
{
	auto pbMsg = new SyncPid;
	pbMsg->set_pid(this->mPlayerID);
	pbMsg->set_username(this->mPlayerName);

	auto single = new GameSingleTLV(GameSingleTLV::GAME_MSG_LOGOFF_SYNCPID, pbMsg);
	auto retMsg = new GameMsg;
	retMsg->m_MsgList.push_back(single);
	return retMsg;
}

GameMsg * GameRole::MakeSurPlays()
{
	auto pbMsg = new SyncPlayers;
	//获取周边玩家
	auto players = mCurrentWorld->GetSurPlayers(this);
	for (auto &r : players)
	{
		if (r == this)
			continue;
		auto role = dynamic_cast<GameRole*>(r);
		auto p = pbMsg->add_ps();
		p->set_pid(role->mPlayerID);
		p->set_username(role->mPlayerName);
		auto pos = p->mutable_p();
		pos->set_x(role->x);
		pos->set_y(role->y);
		pos->set_z(role->z);
		pos->set_v(role->v);
		pos->set_bloodvalue(role->hp);
	}
	auto single = new GameSingleTLV(GameSingleTLV::GAME_MSG_SUR_PLAYER, pbMsg);
	auto retMsg = new GameMsg;
	retMsg->m_MsgList.push_back(single);
	return retMsg;
}

GameMsg * GameRole::MakeChangeWorldResponse(int srcId, int targetId)
{
	auto pbMsg = new ChangeWorldResponse;
	pbMsg->set_pid(this->mPlayerID);
	//切换成功返回1
	pbMsg->set_changeres(1);
	pbMsg->set_srcid(srcId);
	pbMsg->set_targetid(targetId);
	//玩家初始出生点
	auto pos = pbMsg->mutable_p();
	pos->set_x(x);
	pos->set_y(y);
	pos->set_z(z);
	pos->set_v(v);
	pos->set_bloodvalue(hp);
	auto single = new GameSingleTLV(GameSingleTLV::GAME_MSG_CHANGE_WORLD_RESPONSE, pbMsg);
	auto retMsg = new GameMsg;
	retMsg->m_MsgList.push_back(single);
	return retMsg;
}

GameMsg * GameRole::MakeSkillTrigger(pb::SkillTrigger * trigger)
{
	//拿到消息之后,复制这条消息,然后返回
	auto pbMsg = new SkillTrigger(*trigger);
	auto single = new GameSingleTLV(GameSingleTLV::GAME_MSG_SKILL_BROAD, pbMsg);
	auto retMsg = new GameMsg;
	retMsg->m_MsgList.push_back(single);
	return retMsg;
}

GameMsg * GameRole::MakeSkillContact(pb::SkillContact * contact)
{
	auto pbMsg = new SkillContact(*contact);
	auto single = new GameSingleTLV(GameSingleTLV::GAME_MSG_SKILL_CONTACT_BROAD, pbMsg);
	auto retMsg = new GameMsg;
	retMsg->m_MsgList.push_back(single);
	return retMsg;
}

