#include "LocationConfig.hpp"

LocationConfig::LocationConfig()
	: _path("/"), _hasRoot(false), _autoindex(false), _hasAutoindex(false),
	  _hasIndex(false), _clientMaxBodySize(0), _hasMaxBodySize(false),
	  _hasUploadStore(false),
	  _hasRedirect(false), _redirectCode(0)
{
}

LocationConfig::LocationConfig(const std::string &path)
	: _path(path), _hasRoot(false), _autoindex(false), _hasAutoindex(false),
	  _hasIndex(false), _clientMaxBodySize(0), _hasMaxBodySize(false),
	  _hasUploadStore(false),
	  _hasRedirect(false), _redirectCode(0)
{
}

const std::string &LocationConfig::getPath() const
{
	return (_path);
}

const std::vector<std::string> &LocationConfig::getMethods() const
{
	return (_methods);
}

const std::string &LocationConfig::getRoot() const
{
	return (_root);
}

bool LocationConfig::hasRoot() const
{
	return (_hasRoot);
}

bool LocationConfig::getAutoindex() const
{
	return (_autoindex);
}

bool LocationConfig::hasAutoindex() const
{
	return (_hasAutoindex);
}

const std::string &LocationConfig::getIndex() const
{
	static const std::string empty;
	if (_indexes.empty())
		return (empty);
	return (_indexes[0]);
}
const std::vector<std::string> &LocationConfig::getIndexes() const
{
	return (_indexes);
}

bool LocationConfig::hasIndex() const
{
	return (_hasIndex);
}

size_t LocationConfig::getClientMaxBodySize() const
{
	return (_clientMaxBodySize);
}

bool LocationConfig::hasMaxBodySize() const
{
	return (_hasMaxBodySize);
}

const std::string &LocationConfig::getUploadStore() const
{
	return (_uploadStore);
}

bool LocationConfig::hasUploadStore() const
{
	return (_hasUploadStore);
}

const std::map<std::string, std::string> &LocationConfig::getCgi() const
{
	return (_cgi);
}

bool LocationConfig::hasRedirect() const
{
	return (_hasRedirect);
}

int LocationConfig::getRedirectCode() const
{
	return (_redirectCode);
}

const std::string &LocationConfig::getRedirectUrl() const
{
	return (_redirectUrl);
}

void LocationConfig::setMethods(const std::vector<std::string> &methods)
{
	_methods = methods;
}
void LocationConfig::setRoot(const std::string &root)
{
	_root = root;
	_hasRoot = true;
}
void LocationConfig::setAutoindex(bool on)
{
	_autoindex = on;
	_hasAutoindex = true;
}
void LocationConfig::setIndexes(const std::vector<std::string> &indexes)
{
	_indexes = indexes;
	_hasIndex = true;
}
void LocationConfig::setClientMaxBodySize(size_t size)
{
	_clientMaxBodySize = size;
	_hasMaxBodySize = true;
}
void LocationConfig::setUploadStore(const std::string &path)
{
	_uploadStore = path;
	_hasUploadStore = true;
}
void LocationConfig::addCgi(const std::string &ext, const std::string &binary) { _cgi[ext] = binary; }
void LocationConfig::setRedirect(int code, const std::string &url)
{
	_hasRedirect = true;
	_redirectCode = code;
	_redirectUrl = url;
}

bool LocationConfig::methodAllowed(const std::string &method) const
{
	if (_methods.empty())
		return (true);
	for (std::size_t i = 0; i < _methods.size(); ++i)
	{
		if (_methods[i] == method)
			return (true);
	}
	return (false);
}
