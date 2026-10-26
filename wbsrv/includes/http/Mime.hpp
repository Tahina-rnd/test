#ifndef MIME_HPP
#define MIME_HPP

#include <string>
#include <map>

class Mime
{
	public:
		static std::string getType(const std::string& path);

	private:
		Mime();
		~Mime();
		Mime(const Mime& other);
		Mime& operator=(const Mime& other);

		static void init();
		static std::map<std::string, std::string> _types;
		static bool _initialized;
};

#endif
