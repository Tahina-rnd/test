#ifndef CGIHANDLER_HPP
#define CGIHANDLER_HPP

#include <string>
#include <vector>
#include <map>
#include <ctime>
#include <sys/types.h>
#include "Request.hpp"
#include "Response.hpp"
#include "ServerConfig.hpp"
#include "LocationConfig.hpp"

// Gestionnaire d'exécution CGI/1.1 non bloquant.
// - Ouvre deux pipes (stdin / stdout) passés en O_NONBLOCK.
// - Lance le binaire CGI via fork() + chdir() + dup2() + execve().
// - Toutes les lectures/écritures sur les pipes sont pilotées par
//   l'unique poll() de Server::run() (aucune E/S hors poll, aucun errno).
// - Surveille la fin du processus avec waitpid(WNOHANG) et applique
//   un timeout strict de 10 secondes (SIGKILL + waitpid, zéro zombie).
class CgiHandler
{
	public:
		static const std::time_t CGI_TIMEOUT_SEC = 10;

		struct Target
		{
			std::string binary;
			std::string diskPath;
			std::string scriptDir;
			std::string scriptFile;
			std::string scriptName;
			std::string pathInfo;
		};

		CgiHandler(const Target& target, const Request& request,
			const ServerConfig* server);
		~CgiHandler();

		bool start();

		int getInFd() const;
		int getOutFd() const;
		pid_t getPid() const;
		std::time_t getStartTime() const;

		bool wantsWrite() const;
		bool wantsRead() const;
		bool isFinished() const;
		bool hasFailed() const;
		int getErrorCode() const;

		// Appelées uniquement lorsque poll() signale POLLOUT / POLLIN
		void handleWritable();
		void handleReadable();
		void closeInPipe();
		void closeOutPipe();

		// Vérifie l'état du processus enfant (waitpid WNOHANG) et le timeout
		void checkState();
		void killOnTimeout();

		Response buildResponse(const ServerConfig* server, bool keepAlive) const;

		// Détecte si normPath correspond à un script CGI dans loc
		static bool resolveTarget(const std::string& normPath,
			const ServerConfig* server, const LocationConfig* loc,
			Target& outTarget, int& outErrorCode);

	private:
		CgiHandler(const CgiHandler& other);
		CgiHandler& operator=(const CgiHandler& other);

		void buildEnv(std::vector<std::string>& envStrings) const;
		void execChild(int inReadFd, int outWriteFd,
			const std::vector<std::string>& envStrings);
		void reapChild(bool blocking);
		void cleanupPipes();

		Target              _target;
		const Request&      _request;
		const ServerConfig* _server;

		int                 _inFd;
		int                 _outFd;
		pid_t               _pid;
		std::time_t         _startTime;

		std::string         _inputBody;
		std::size_t         _inputOffset;
		std::string         _outputBuffer;

		bool                _childExited;
		int                 _exitStatus;
		bool                _failed;
		int                 _errorCode;
};

#endif
