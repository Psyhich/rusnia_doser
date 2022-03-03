#ifndef RESOURCE_MANAGER_H
#define RESOURCE_MANAGER_H

#include <list>
#include <string>
#include <mutex>

class TargetManager
{
public:
	TargetManager(const std::string &targetsServer);
	
	std::string GetTarget();

private:
	bool CheckTarget(const std::string &targetToCheck);

private:
	std::string m_targetsServer;

	std::mutex m_allowedListMutex;
	std::list<std::string> m_allowedList;

	std::mutex m_downListMutex;
	std::list<std::string> m_downList;

	std::mutex m_blockingHostsMutex;
	std::list<std::string> m_blockingHosts;
};

#endif // RESOURCE_MANAGER_H 
