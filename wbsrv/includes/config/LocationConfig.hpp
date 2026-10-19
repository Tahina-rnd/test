#ifndef LOCATIONCONFIG_HPP
#define LOCATIONCONFIG_HPP

#include <cstddef>
#include <map>
#include <string>
#include <vector>

// Configuration d'un bloc `location`.
// Champs avec flag `has*` : distingue "non précisé" (hérite du server
// parent via ServerConfig::resolveInheritance) de "précisé".
// Après résolution, root/indexes/maxBody sont toujours renseignés :
// héritage parent, sinon défauts (root ".", index "index.html",
// maxBody 1 Mo). Zéro allocation manuelle (RAII pur).
class LocationConfig
{
	public:
		LocationConfig();
		explicit LocationConfig(const std::string& path);

		const std::string& getPath() const;
		const std::vector<std::string>& getMethods() const;
		const std::string& getRoot() const;
		bool hasRoot() const;
		bool getAutoindex() const;
		bool hasAutoindex() const;
		const std::string& getIndex() const;
		const std::vector<std::string>& getIndexes() const;
		bool hasIndex() const;
		size_t getClientMaxBodySize() const;
		bool hasMaxBodySize() const;
		const std::string& getUploadStore() const;
		bool hasUploadStore() const;
		const std::map<std::string, std::string>& getCgi() const;
		bool hasRedirect() const;
		int getRedirectCode() const;
		const std::string& getRedirectUrl() const;

		void setMethods(const std::vector<std::string>& methods);
		void setRoot(const std::string& root);
		void setAutoindex(bool on);
		void setIndexes(const std::vector<std::string>& indexes);
		void setClientMaxBodySize(size_t size);
		void setUploadStore(const std::string& path);
		void addCgi(const std::string& ext, const std::string& binary);
		void setRedirect(int code, const std::string& url);

		bool methodAllowed(const std::string& method) const;

	private:
		std::string _path;
		std::vector<std::string> _methods;
		std::string _root;
		bool _hasRoot;
		bool _autoindex;
		bool _hasAutoindex;
		std::vector<std::string> _indexes;
		bool _hasIndex;
		size_t _clientMaxBodySize;
		bool _hasMaxBodySize;
		std::string _uploadStore;
		bool _hasUploadStore;
		std::map<std::string, std::string> _cgi;
		bool _hasRedirect;
		int _redirectCode;
		std::string _redirectUrl;
};

#endif
