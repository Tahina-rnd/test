#ifndef CONFIG_HPP
#define CONFIG_HPP

#include <string>
#include <vector>
#include "ServerConfig.hpp"

class Config
{
	public:
		explicit Config(const std::string& path);

		const std::vector<ServerConfig>& getServers() const;

	private:
		std::vector<ServerConfig> _servers;

		void parseFile(const std::string& path);
};

#endif
