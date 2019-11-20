#pragma once
#include <deque>
#include <string>
class RandomName{
public:
	static RandomName &GetInstance();
    RandomName();
    ~RandomName();
    void LoadFile();
    std::string GetName();
    void ReleaseName(std::string szName);
private:
	static RandomName smInstance;
    std::deque<std::string> m_names;
};

