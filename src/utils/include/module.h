#ifndef MODULE_H
#define MODULE_H

#include <memory>
class IModule
{
public:
	IModule() = default;
	virtual ~IModule() {}

	virtual bool Initialize() = 0;
};

using Module = std::unique_ptr<IModule>;

#endif // MODULE_H
