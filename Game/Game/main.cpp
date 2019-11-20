#include <iostream>
#include <unistd.h>
#include "zinx.h"
#include "ZinxTimer.h"
#include "ZinxTCP.h"
#include "GameChannel.h"
#include <fcntl.h>
using namespace std;

class ShutdownServer : public TimerOutProc
{
public:
	virtual void Proc()                //超时的时候会去调用的接口
	{
		//每30秒检查一下当前服务器如果没有玩家,就自动关闭
		auto allPlayers = ZinxKernel::Zinx_GetAllRole();
		if (allPlayers.size() == 0)
		{
			ZinxKernel::Zinx_Exit();
		}
	}
	virtual int GetTimerSec()          //获取当前任务的超时间隔，一秒触发一次就返回1
	{
		return 30;
	}
};


//守护进程+进程监控
void Daemon()
{
	//打开日志文件
	int logFd = open("./game.log", O_WRONLY | O_CREAT , 0664);
	if (logFd < 0)
	{
		perror("open");
		exit(EXIT_FAILURE);
	}
	//将标准输出重定向到日志文件
	if (dup2(logFd, STDOUT_FILENO) < 0)
	{
		perror("dup2");
		exit(EXIT_FAILURE);
	}
	if (dup2(logFd, STDERR_FILENO) < 0)
	{
		perror("dup2");
		exit(EXIT_FAILURE);
	}
	else
	{
		close(logFd);
	}

	//2 fork  实现守护进程
	int pid = fork();
	if (pid > 0)
	{
		//    父进程： 退出
		exit(EXIT_SUCCESS);
	}
	else if (pid == 0)
	{
		//    子进程:
		if (setsid() < 0)
		{
			perror("setsid");
			exit(EXIT_FAILURE);
		}
		while (true)
		{
			//3 fork 实现进程监控
			pid = fork();
			if (pid > 0)
			{
				//父进程：
				//wait
				int status;
				wait(&status);
				if (status == 0)
				{
					exit(EXIT_SUCCESS);
				}
				else
				{
					//如果拿到结果值不为0
					//继续启动fork
					continue;
				}
			}
			else if (pid == 0)
			{
				//子进程:
				//执行游戏业务逻辑
				break;
			}
			else
			{
				perror("fork");
				exit(EXIT_FAILURE);
			}
		}
	}
	else
	{
		perror("fork");
		exit(EXIT_FAILURE);
	}

}

int main(int argc, char **argv)
{
	//1 判断main 参数 有 daemon
	if (argc == 2)
	{
		if (string("daemon") == argv[1])
		{
			Daemon();
		}
	}

	//注册定时任务
	ShutdownServer *poTimeout = new ShutdownServer;
	ZinxTimerDeliver::GetInstance().RegisterProcObject(*poTimeout);

	//创建TCP listen 通道,用来监听端口
	ZinxTCPListen *chListener = new ZinxTCPListen(8888, new GameChannelFactory);

	//1 初始化，调用了 epoll_create
	ZinxKernel::ZinxKernelInit();

	//创建定时器通道
	ZinxTimer *poTimer = new ZinxTimer;
	ZinxKernel::Zinx_Add_Channel(*poTimer);

	ZinxKernel::Zinx_Add_Channel(*chListener);


	//生命循环          调用了 epoll_wait
	ZinxKernel::Zinx_Run();


	//3 垃圾回收        调用了close
	ZinxKernel::ZinxKernelFini();



	return 0;
}
