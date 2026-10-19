#include "Config.hpp"
#include "ConfigError.hpp"
#include "Server.hpp"
#include <csignal>
#include <iostream>

static void printUsage(const char* prog)
{
	std::cerr << "Usage: " << prog << " [config_file]" << std::endl;
}

int main(int argc, char* argv[])
{
	signal(SIGPIPE, SIG_IGN);

	if (argc > 2)
	{
		printUsage(argv[0]);
		return (1);
	}

	std::string configPath = "configs/default.conf";
	if (argc == 2)
		configPath = argv[1];

	try
	{
		Config config(configPath);
		std::cout << "Loaded " << config.getServers().size()
			<< " server(s) from " << configPath << std::endl;
		for (std::size_t i = 0; i < config.getServers().size(); ++i)
		{
			const std::vector<ServerConfig::Listen>& ls =
				config.getServers()[i].getListens();
			for (std::size_t j = 0; j < ls.size(); ++j)
				std::cout << "  server[" << i << "] listen "
					<< ls[j].host << ":" << ls[j].port
					<< (ls[j].is_default ? " (default)" : "") << std::endl;
		}

		Server server(config.getServers());
		server.run();
	}
	catch (const ConfigError& e)
	{
		std::cerr << "Config error: " << e.what() << std::endl;
		return (1);
	}
	catch (const std::exception& e)
	{
		std::cerr << "Fatal error: " << e.what() << std::endl;
		return (1);
	}

	return (0);
}
