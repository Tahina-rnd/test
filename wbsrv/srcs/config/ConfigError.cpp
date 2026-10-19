#include "ConfigError.hpp"

ConfigError::ConfigError(const std::string& msg) : std::runtime_error(msg)
{
}
