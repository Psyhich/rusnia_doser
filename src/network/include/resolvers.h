#ifndef RESOLVERS_H
#define RESOLVERS_H

#include <memory>
#include <optional>
#include <string>

#include "uri.h"

namespace NetUtil
{

using PossibleAddress = std::optional<std::string>;

class AddressResolver
{
public:
	AddressResolver() = default;
	virtual ~AddressResolver(){}

	virtual PossibleAddress ResolveHostAddress(const URI &hostAddress) = 0;
};

using PAddressResolver = std::shared_ptr<AddressResolver>;

}

#endif // RESOLVERS_H
