// Server
#include "Server.h"

int main()
{
	Server MyServer(80, false, true);
	while (true)
	{
		MyServer.ListenForNewConnection();
	}
	return 0;
}