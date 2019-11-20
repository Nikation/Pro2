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

//�������������
default_random_engine g_rand_engine(time(nullptr));

GameRole::GameRole()
{
	//��ʼ��ID�Լ�����
	this->mPlayerID = smRoleCount++;
	//this->mPlayerName = string("Player_") + to_string(mPlayerID);
	//��������������л�ȡһ���������
	this->mPlayerName = RandomName::GetInstance().GetName();

	//��ʼλ��
	x = 100+(g_rand_engine()%20);
	y = 0;
	z = 100+(g_rand_engine()%20);
	v = 0;

	//hp ��Ѫ��1000
	hp = 1000;

}

GameRole::~GameRole()
{
	//�黹����
	RandomName::GetInstance().ReleaseName(this->mPlayerName);
}

bool GameRole::Init()
{
	//��������ߵ�ʱ��ͻᴥ���������
	//������ʵ��һЩ���ߵ�ҵ���߼�
	//���͵�ǰ���ID�Լ����������
	auto msg = MakeLogonSyncPid();
	//����Ϣ���ͳ�ȥ
	ZinxKernel::Zinx_SendOut(*msg, *mProtocol);

	//������ǰ��ҳ�ʼ������λ����Ϣ
	msg = MakeInitPosBroadcast();
	ZinxKernel::Zinx_SendOut(*msg, *mProtocol);

	//���ߵ�ʱ��Ҫ����ǰ�����ӵ�AOI�������
	this->mCurrentWorld = WorldManager::GetInstance().GetWorld(1);
	mCurrentWorld->AddPlayer(this);
	
	//�����ܱ���ҵ���Ϣ
	msg = MakeSurPlays();
	ZinxKernel::Zinx_SendOut(*msg, *mProtocol);

	//��ȡ�����������,����������Ϣ���Է�,ҲҪ����һ���Է����ߵ���Ϣ���Լ�
	auto players = mCurrentWorld->GetSurPlayers(this);

	for (auto &r : players)
	{
		if (r == this)
			continue;
		auto role = dynamic_cast<GameRole*>(r);
		cout << "Init sur players : " << role->mPlayerName << endl;

		//����������Ϣ
		msg = MakeInitPosBroadcast();
		//ע�ⷢ�͸��Է���protocol��
		ZinxKernel::Zinx_SendOut(*msg, *role->mProtocol);
	}


	return true;
}

//���������Ҫ���Ǵ�����protocol ���ݹ����� UserData��Ϣ
UserData * GameRole::ProcMsg(UserData & _poUserData)
{
	GameMsg &msg = dynamic_cast<GameMsg&>(_poUserData);
	for(auto single : msg.m_MsgList)
	{
		//�����ͻ��˴��ݹ�����ÿһ����Ϣ
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
			//������Ϣ
			auto pbMsg = dynamic_cast<pb::Talk*>(single->m_poGameMsg);
			this->ProcTalkContent(pbMsg->content());
			break;
		}
		case GameSingleTLV::GAME_MSG_CHANGE_WORLD:
		{
			//�ͻ����л�����������
			auto pbMsg = dynamic_cast<pb::ChangeWorldRequest*>(single->m_poGameMsg);
			this->ProcChangeWorld(pbMsg->srcid(), pbMsg->targetid());
			break;
		}
		case GameSingleTLV::GAME_MSG_SKILL_TRIGGER:
		{
			//�ͻ��˴����˼���
			auto pbMsg = dynamic_cast<pb::SkillTrigger*>(single->m_poGameMsg);
			this->ProcSkillTrigger(pbMsg);
			break;
		}
		case GameSingleTLV::GAME_MSG_SKILL_CONTACT:
		{
			//��Ҽ�������
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
	//��������ߵ�ʱ��,��������ͻᱻ����,������ʵ�����ߵ��߼�����
	//���ߵ�ʱ��Ҫ���Ǵ�AOI��ɾ����ǰ���
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
	//�����ǰAOI����仯��,��ʱ���Ҫ�л�����
	if (mCurrentWorld->GridChanged(this, _x, _z))
	{
		mCurrentWorld->DelPlayer(this);
		//��ȡ�ɵ��ܱ���Ҽ���
		auto oldList = mCurrentWorld->GetSurPlayers(this);
		//�任����
		this->x = _x;
		this->y = _y;
		this->z = _z;
		this->v = _v;

		//��ȡ�µ��ܱ���Ҽ���
		auto newList = mCurrentWorld->GetSurPlayers(this);

		//��Ұ��ʧ�Ĵ���
		this->ViewDisappear(oldList, newList);
		this->ViewAppear(oldList, newList);

		mCurrentWorld->AddPlayer(this);
	}
	//������ƶ���ʱ����յ��ƶ���Ϣ,�ͻ�����������
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
	//��ȡ��ǰ���������е�����б�,Ȼ��㲥��Ϣ
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
	//ʵ���л�����������ҵ��
	//��ǰAOI����ҲҪ�����л�
	//���ߵ�ǰAOI����
	mCurrentWorld->DelPlayer(this);
	//�����ܱ����������
	auto players = mCurrentWorld->GetSurPlayers(this);
	for (auto &r : players)
	{
		if (r == this)
			continue;
		auto role = dynamic_cast<GameRole*>(r);
		auto msg = MakeLogoffSyncPid();
		ZinxKernel::Zinx_SendOut(*msg, *role->mProtocol);
	}

	//����һ�����������λ��
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
		//ս�������������� x [0,140]  z [0,140]
		//�����Ƴ�������	10-130֮�伴��
		x = 10 + g_rand_engine()%120;
		y = 0;
		z = 10 + g_rand_engine()%120;
	}

	//�����µ�AOI����
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
	//ע��˳���ܱ�
	//1 �ȷ��ͳ����л�����Ӧ
	//2 �ٷ��ͳ�����ߵ��ܱ����

	//�����л���������Ӧ��Ϣ
	auto msg = MakeChangeWorldResponse(srcId, targetWorldId);
	ZinxKernel::Zinx_SendOut(*msg, *mProtocol);

	//�����Լ��µĳ�������ܱ���ʲô���
	msg = MakeSurPlays();
	ZinxKernel::Zinx_SendOut(*msg, *mProtocol);
}

void GameRole::ProcSkillTrigger(pb::SkillTrigger * trigger)
{
	//�յ��ͻ��˼��ܴ�������Ϣ֮��,����������й㲥,�㲥���ܱ����

	if (trigger->pid ()!= this->mPlayerID)
	{
		//����
	}
	//��֤�ӵ�...
	//��¼�ӵ���ID...

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
	//�����ﴥ����������,�߼�
	//������
	if (contact->srcpid() != this->mPlayerID)
	{
		//���ù�,ֻ�ܵ�ǰ����Լ���������Ϣ
		return;
	}
	int targetPlayerId = contact->targetpid();

	//�������֤
	//.....

	auto tmpContactMsg = new SkillContact(*contact);

	//ȷ�������������ĸ����
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
		//û���ҵ��ܺ���,����϶�������
		return;
	}

	if (targetRole->mPlayerID != contact->targetpid())
	{
		cout << "error" << endl;
	}

	//�����ܵ����˺�
	int attackValue = 300 + g_rand_engine() % 300;

	targetRole->hp -= attackValue;


	//�����㲥�������������
	//�޸�һ����Ϣ�е�hpֵ
	auto pos = tmpContactMsg->mutable_contactpos();
	pos->set_bloodvalue(targetRole->hp);
	pos->set_x(pos->x());
	pos->set_y(pos->y());
	pos->set_z(pos->z());
	pos->set_v(pos->v());

	//�㲥��������ҵ�ǰ������,���ҿ���Ѫ��
	for (auto &r : players)
	{
		auto role = dynamic_cast<GameRole*>(r);
		auto msg = MakeSkillContact(tmpContactMsg);
		ZinxKernel::Zinx_SendOut(*msg, *role->mProtocol);
	}

	delete tmpContactMsg;


	//�ж��ܺ��� hpֵ<=0 ,���л�����,�ص���һ���������
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
	//�������� �ɵļ��� - �µļ��� = Ҫ����Ұ��ʧ�����
	//Ҫ�Լ���������,����ǰ��Ҫȷ���������ܹ��������
	vector<AOI_Player*> oldVec(oldList.begin(), oldList.end());
	vector<AOI_Player*> newVec(newList.begin(), newList.end());

	sort(oldVec.begin(), oldVec.end());
	sort(newVec.begin(), newVec.end());

	vector<AOI_Player*> diff;
	//��������,�
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
	//�������� �µļ��� - �ɵļ��� = Ҫ����Ұ���ֵ����
	//Ҫ�Լ���������,����ǰ��Ҫȷ���������ܹ��������
	vector<AOI_Player*> oldVec(oldList.begin(), oldList.end());
	vector<AOI_Player*> newVec(newList.begin(), newList.end());

	sort(oldVec.begin(), oldVec.end());
	sort(newVec.begin(), newVec.end());

	vector<AOI_Player*> diff;
	//��������,�
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

//��������ͬ�����id����������Ϣ
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
	/*����Tp��ͬ��BroadCast��Ϣ�������
      1 �������ݣ�Content��
      2 ��ʼλ��(P)
      4 ��λ��P
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
	/*����Tp��ͬ��BroadCast��Ϣ�������
      1 �������ݣ�Content��
      2 ��ʼλ��(P)
      4 ��λ��P
    */
	pbMsg->set_tp(2);
	//�Զ�����һ���µ�Position�����ҷ���ָ��
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
	/*����Tp��ͬ��BroadCast��Ϣ�������
      1 �������ݣ�Content��
      2 ��ʼλ��(P)
      4 ��λ��P
    */
	pbMsg->set_tp(4);
	//�Զ�����һ���µ�Position�����ҷ���ָ��
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
	//��ȡ�ܱ����
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
	//�л��ɹ�����1
	pbMsg->set_changeres(1);
	pbMsg->set_srcid(srcId);
	pbMsg->set_targetid(targetId);
	//��ҳ�ʼ������
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
	//�õ���Ϣ֮��,����������Ϣ,Ȼ�󷵻�
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

