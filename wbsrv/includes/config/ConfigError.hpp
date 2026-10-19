#ifndef CONFIGERROR_HPP
#define CONFIGERROR_HPP

#include <stdexcept>
#include <string>

class ConfigError : public std::runtime_error
{
	public:
		explicit ConfigError(const std::string& msg);
};

#endif
