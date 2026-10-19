#ifndef SERVERCONFIG_HPP
#define SERVERCONFIG_HPP

#include <map>
#include <string>
#include <vector>
#include "LocationConfig.hpp"

class ServerConfig
{
	public:
		struct Listen
		{
			std::string host;
			int port;
			// Explicite (`listen 80 default_server ;`) vs calculé
			// (gagnant du port : flag explicite sinon premier déclarant).
			bool default_server;
			bool is_default;
		};

		ServerConfig();
		explicit ServerConfig(int port);

		// listen
		void addListen(const std::string& host, int port, bool isDefaultServer);
		const std::vector<Listen>& getListens() const;
		std::vector<Listen>& getListens();
		int getPort() const; // compat: premier listen

		// server-level
		void setServerNames(const std::vector<std::string>& names);
		const std::string& getServerName() const;
		const std::vector<std::string>& getServerNames() const;
		void setRoot(const std::string& root);
		const std::string& getRoot() const;
		void setIndexes(const std::vector<std::string>& indexes);
		const std::string& getIndex() const;
		const std::vector<std::string>& getIndexes() const;
		void setClientMaxBodySize(size_t size);
		size_t getClientMaxBodySize() const;
		void addErrorPage(int code, const std::string& path);
		const std::map<int, std::string>& getErrorPages() const;

		void addLocation(const LocationConfig& loc);
		const std::vector<LocationConfig>& getLocations() const;
		const LocationConfig* matchLocation(const std::string& uri) const;

		// Héritage (fallback) : toute location sans root/index/maxBody
		// reçoit la valeur du server ; défauts si définis nulle part
		// (root ".", index "index.html", maxBody 1 Mo déjà en place).
		// Appelé une fois le bloc server entièrement parsé.
		void resolveInheritance();

		// Serveur par défaut : vrai si ce server est le gagnant d'au
		// moins un de ses ports (premier déclarant, sauf flag explicite
		// `default_server` gagnant). Positionné par markDefaultServers().
		void setDefault(bool v);
		bool isDefault() const;

	private:
		std::vector<Listen> _listens;
		std::vector<std::string> _serverNames;
		std::string _root;
		std::vector<std::string> _indexes;
		size_t _clientMaxBodySize;
		std::map<int, std::string> _errorPages;
		std::vector<LocationConfig> _locations;
		bool _isDefault;
};

#endif
