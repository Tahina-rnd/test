#include "Mime.hpp"

namespace
{

struct MimeEntry
{
	const char* ext;
	const char* type;
};

const MimeEntry MIME_TYPES[] = {
	{".html", "text/html"},
	{".htm",  "text/html"},
	{".css",  "text/css"},
	{".js",   "application/javascript"},
	{".json", "application/json"},
	{".xml",  "application/xml"},
	{".txt",  "text/plain"},
	{".png",  "image/png"},
	{".jpg",  "image/jpeg"},
	{".jpeg", "image/jpeg"},
	{".gif",  "image/gif"},
	{".svg",  "image/svg+xml"},
	{".ico",  "image/x-icon"},
	{".webp", "image/webp"},
	{".pdf",  "application/pdf"},
	{".zip",  "application/zip"},
	{".tar",  "application/x-tar"},
	{".gz",   "application/gzip"},
	{".mp3",  "audio/mpeg"},
	{".mp4",  "video/mp4"},
	{".woff", "font/woff"},
	{".woff2", "font/woff2"},
	{".ttf",  "font/ttf"}
};

const std::size_t MIME_TYPES_COUNT = sizeof(MIME_TYPES) / sizeof(MIME_TYPES[0]);

} // namespace

std::map<std::string, std::string> Mime::_types;
bool Mime::_initialized = false;

void Mime::init()
{
	if (_initialized)
		return;
	for (std::size_t i = 0; i < MIME_TYPES_COUNT; ++i)
		_types[MIME_TYPES[i].ext] = MIME_TYPES[i].type;
	_initialized = true;
}

std::string Mime::getType(const std::string& path)
{
	init();

	std::size_t dotPos = path.rfind('.');
	if (dotPos == std::string::npos)
		return "application/octet-stream";

	std::string ext = path.substr(dotPos);
	for (std::size_t i = 0; i < ext.size(); ++i)
	{
		if (ext[i] >= 'A' && ext[i] <= 'Z')
			ext[i] = static_cast<char>(ext[i] + 32);
	}

	std::map<std::string, std::string>::const_iterator it = _types.find(ext);
	if (it != _types.end())
		return it->second;

	return "application/octet-stream";
}
