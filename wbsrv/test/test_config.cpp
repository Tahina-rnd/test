#include "Config.hpp"
#include "ServerConfig.hpp"
#include "LocationConfig.hpp"

#include <iostream>
#include <string>
#include <vector>

static std::string join(const std::vector<std::string>& v)
{
	std::string out;
	for (std::size_t i = 0; i < v.size(); ++i)
	{
		if (i > 0)
			out += ",";
		out += v[i];
	}
	return (out);
}

int main(int argc, char* argv[])
{
	if (argc != 2)
	{
		std::cerr << "Usage: " << argv[0] << " <config_file>" << std::endl;
		return (2);
	}
	try
	{
		Config cfg(argv[1]);
		const std::vector<ServerConfig>& servers = cfg.getServers();
		for (std::size_t i = 0; i < servers.size(); ++i)
		{
			const ServerConfig& s = servers[i];
			std::cout << "SERVER[" << i << "]"
				<< " default=" << (s.isDefault() ? "Y" : "N")
				<< " root=" << s.getRoot()
				<< " index=" << join(s.getIndexes())
				<< " maxbody=" << s.getClientMaxBodySize()
				<< " names=" << join(s.getServerNames())
				<< std::endl;
			const std::vector<LocationConfig>& locs = s.getLocations();
			for (std::size_t j = 0; j < locs.size(); ++j)
			{
				const LocationConfig& l = locs[j];
				std::cout << "LOC[" << i << "." << j << "]"
					<< " path=" << l.getPath()
					<< " root=" << l.getRoot()
					<< " index=" << join(l.getIndexes())
					<< " maxbody=" << l.getClientMaxBodySize()
					<< " methods=" << join(l.getMethods())
					<< std::endl;
			}
		}
	}
	catch (const std::exception& e)
	{
		std::cerr << "Config error: " << e.what() << std::endl;
		return (1);
	}
	return (0);
}
