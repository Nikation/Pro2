#include "RandomName.h"
#include <iostream>
#include <fstream>
#include <algorithm>
using namespace std;
#define RANDOM_NAME_FIRST_FILE "random_first.txt"
#define RANDOM_NAME_LAST_FILE "random_last.txt"

RandomName RandomName::smInstance;

RandomName & RandomName::GetInstance()
{
	return smInstance;
}

RandomName::RandomName()
{
	//加载姓氏文件以及名字文件
	LoadFile();
}


RandomName::~RandomName()
{
}

void RandomName::LoadFile()
{
	ifstream ifirst, ilast;
	ifirst.open(RANDOM_NAME_FIRST_FILE);
	ilast.open(RANDOM_NAME_LAST_FILE);
	if (ifirst.is_open() && ilast.is_open())
	{
		string firstName;
		while (getline(ifirst, firstName))
		{
			string lastName;
			while (getline(ilast, lastName))
			{
				string finalName = firstName + " " + lastName;
				m_names.push_back(finalName);

			}
			ilast.clear(ios::goodbit);
			ilast.seekg(ios::beg);
		}
		ifirst.close();
		ilast.close();

		//设置随机数种子
		srand(time(nullptr));
		//随机洗牌,打乱顺序
		random_shuffle(m_names.begin(), m_names.end());

	}
	else
	{
		cerr << "Error in open file name file" << endl;
	}

}

//获取一个名字
std::string RandomName::GetName()
{
	//出队
	string retName = m_names.front();
	m_names.pop_front();
	return retName;
}

void RandomName::ReleaseName(std::string szName)
{
	m_names.push_back(szName);
}
