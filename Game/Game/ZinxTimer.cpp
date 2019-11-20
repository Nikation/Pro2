#include "ZinxTimer.h"
#include <sys/timerfd.h>
#include <iostream>
using namespace std;
ZinxTimer::ZinxTimer()
	:m_fd(-1)
{
}
ZinxTimer::~ZinxTimer()
{
	if (m_fd >= 0)
	{
		close(m_fd);
		m_fd = -1;
	}
}

bool ZinxTimer::Init()
{
	//创建定时器,设定超时为1秒
	int fd = timerfd_create(CLOCK_MONOTONIC,0);
	if (fd < 0)
	{
		return false;
	}
    struct itimerspec spec = {{1,0},{1,0}};
    int ret = timerfd_settime(fd,0,&spec,NULL);
	if (ret < 0)
	{
		close(fd);
		return false;
	}
	m_fd = fd;
	return true;
}

bool ZinxTimer::ReadFd(std::string & _input)
{
	//EPOLLIN触发的时候,其实就是超时的时候
	//将超时次数读取出来传递给下一个处理者
	uint64_t over_times = 0;
	int ret = read(m_fd, &over_times, sizeof(over_times));
	if (ret != sizeof(over_times))
	{
		return false;
	}
	_input.append((char*)&over_times, sizeof(over_times));
	return true;
}

bool ZinxTimer::WriteFd(std::string & _output)
{
	//这里不需要实现逻辑
	return false;
}

void ZinxTimer::Fini()
{
	//销毁当前通道的定时器
	if (m_fd >= 0)
	{
		close(m_fd);
		m_fd = -1;
	}
}

int ZinxTimer::GetFd()
{
	return m_fd;
}

std::string ZinxTimer::GetChannelInfo()
{
	return "ZinxTimerChannel";
}

AZinxHandler * ZinxTimer::GetInputNextStage(BytesMsg & _oInput)
{
	return &ZinxTimerDeliver::GetInstance();
}

#define ZINX_TIMER_WHEEL_SIZE 8
ZinxTimerDeliver ZinxTimerDeliver::m_single;
ZinxTimerDeliver::ZinxTimerDeliver()
	:m_cur_index(0)
{
	//构造轮子,指定轮子的格子数
	m_TimerWheel.resize(ZINX_TIMER_WHEEL_SIZE);
}

ZinxTimerDeliver::~ZinxTimerDeliver()
{
	//释放所有的定时任务
	for (auto &l : m_TimerWheel)
	{
		for (auto p : l)
		{
			delete p.pProc;
		}
	}
}

bool ZinxTimerDeliver::RegisterProcObject(TimerOutProc & _proc)
{
	//注册一个定时任务到相应的轮子的格子里边
	//获取任务的时间间隔
	int timespec = _proc.GetTimerSec();
	//做保护,时间间隔最少为1
	if (timespec < 1)
	{
		return false;
	}
	//计算格子数
	//注册到哪个格子的计算 = （当前刻度 + 任务时间间隔) % 格子数
	int idx = (m_cur_index + timespec) % m_TimerWheel.size();
	
	//计算圈数
	//圈数 = 任务时间间隔 / 格子数
	int round = timespec / m_TimerWheel.size();

	//构造节点
	WheelNode node;
	node.LastCount = round;
	node.pProc = &_proc;

	m_TimerWheel[idx].push_back(node);

	return true;
}

void ZinxTimerDeliver::UnRegisterProcObject(TimerOutProc & _proc)
{
	//遍历所有格子,将任务清理掉
	for (std::list<WheelNode> &l : m_TimerWheel)		//C++11遍历容器的玩法
	{
		for (auto it = l.begin(); it != l.end(); )
		{
			//边删除边遍历
			if (it->pProc == &_proc)
			{
				it = l.erase(it);
			}
			else
			{
				++it;
			}
		}
	}
}

IZinxMsg * ZinxTimerDeliver::InternalHandle(IZinxMsg & _oInput)
{
	BytesMsg &bytes = dynamic_cast<BytesMsg&>(_oInput);
	uint64_t over_times;
	bytes.szData.copy((char*)&over_times, sizeof(over_times));

	//每秒钟都会被触发

	//如果一次触发的过程中发现,超时次数不止一次,这时候要遍历的格子就不止一个,构造循环
	for (uint64_t i = 0; i < over_times; ++i)
	{
		//每次触发的时候刻度都自增
		m_cur_index = (m_cur_index + 1) % m_TimerWheel.size();
		std::list<TimerOutProc*> registerList;

		//遍历当前格子的任务
		for (auto it = m_TimerWheel[m_cur_index].begin(); it != m_TimerWheel[m_cur_index].end(); )
		{
			//任务的圈数要自减
			it->LastCount--;
			if (it->LastCount < 0)
			{
				//触发任务	
				it->pProc->Proc();
				//触发完成之后,将当前任务剔除掉
				registerList.push_back(it->pProc);
				it = m_TimerWheel[m_cur_index].erase(it);
			}
			else
			{
				++it;
			}
		}

		//重新注册刚才剔除掉的任务
		for (auto p : registerList)
		{
			this->RegisterProcObject(*p);
		}
	}
	return nullptr;
}

AZinxHandler * ZinxTimerDeliver::GetNextHandler(IZinxMsg & _oNextMsg)
{
	return nullptr;
}
