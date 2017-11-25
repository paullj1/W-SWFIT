#include "stdafx.h"
#include "globals.h"

#include "Operators.h"
#include "Operator.h"
#include "Library.h"

using namespace std;

bool SendSyslog();
char* GetAddressOfData(HANDLE process, const char *data, size_t len);

int _tmain(int argc, _TCHAR* argv[]) {

	// Declarations
	DWORD aProcesses[1024], cbNeeded, cProcesses;
	TCHAR szProcessName[MAX_PATH] = TEXT("<unknown>");
	unsigned int i;

	// Get All pids
	if (!EnumProcesses(aProcesses, sizeof(aProcesses), &cbNeeded)){
		cerr << "Failed to get all PIDs: " << GetLastError() << endl;
		return -1;
	}

	// Get screen width
	CONSOLE_SCREEN_BUFFER_INFO csbi;
	GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi);
	int dwidth = csbi.srWindow.Right - csbi.srWindow.Left;

	cout << "Running Processes" << endl;
	printf("%-6s %-*s\n", "PID", dwidth - 7, "Process");
	cout << string(3, '-') << " " << string(dwidth - 7, '-') << endl;
	cProcesses = cbNeeded / sizeof(DWORD);
	for (i = 0; i < cProcesses; i++) {
		if (aProcesses[i] != 0) {
			HANDLE hProc = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, aProcesses[i]);
			if (hProc != NULL) {
				HMODULE hMod;
				DWORD cbNeededMod;
				if (EnumProcessModules(hProc, &hMod, sizeof(hMod), &cbNeededMod)) {
					GetModuleBaseName(hProc, hMod, szProcessName, sizeof(szProcessName) / sizeof(TCHAR));
				}

				_tprintf(TEXT("%6u %-*s\n"), aProcesses[i], dwidth - 7, szProcessName);
				CloseHandle(hProc);
			}
		}
	}

	// Which process?
	string s_pid = "";
	cout << endl << "Into which process would you like to inject faults? [PID]: ";
	getline(cin, s_pid);
	int pid = stoi(s_pid);

	HANDLE hTarget = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pid);
	if (!hTarget) {
		cerr << "Failed to open process (check your privilege): " << GetLastError() << endl;
		return -1;
	}

	// Fault Injection or Memory Corruptions?
	string s_fivsmc = "";
	cout << endl << "Would you like to inject Faults, or corrupt Memory? [f|m]: ";
	getline(cin, s_fivsmc);
	if (s_fivsmc.find("m") != string::npos){

		string s_query = "";
		cout << endl << "What are you looking for in process memory?: ";
		getline(cin, s_query);

		char* ret = GetAddressOfData(hTarget, s_query.c_str(), s_query.length());
		if (ret) {
			cout << "Found at addr: " << (void*)ret << endl;
			size_t bytesRead;
			size_t sizeToRead = s_query.size();
			char *buf = (char *)malloc(sizeToRead + 1);
			ReadProcessMemory(hTarget, ret, buf, sizeToRead, (SIZE_T*)&bytesRead);
			buf[sizeToRead] = '\0';

			cout << "Num bytes read: " << bytesRead << endl;
			cout << "Contents: " << string(buf) << endl;

			// Overwrite it
			byte *null_array = (byte *)malloc(bytesRead);
			fill_n(null_array, bytesRead, 0x00);

			SIZE_T mem_bytes_written = 0;
			if (WriteProcessMemory(hTarget, (LPVOID)ret, null_array, bytesRead, &mem_bytes_written) != 0) {
				cout << "Bytes written: " << mem_bytes_written << endl;
				cout << "Successful corruption." << endl;
				return 0;
			} else {
				cerr << "Failed to corrupt memory: " << GetLastError() << endl;
				return -1;
			}
		} else {
			cout << "Not found" << endl;
		}
		return 0;
	}

	// Enumerate modules within process
	HMODULE hmods[1024];
	cout << "DLLs currently loaded in target process:" << endl;
	printf("%-4s %-*s\n", "ID", dwidth-5, "Module Name:");
	cout << string(4, '-') << " " << string(dwidth - 5, '-') << endl;
	if (EnumProcessModules(hTarget, hmods, sizeof(hmods), &cbNeeded)) {
		for (i = 0; i < (cbNeeded / sizeof(HMODULE)); i++) {
			TCHAR szModName[MAX_PATH];
			if (GetModuleFileNameEx(hTarget, hmods[i], szModName, sizeof(szModName) / sizeof(TCHAR))) {
				_tprintf(TEXT("%4d %-*s\n"), i, dwidth-5, szModName);
			} else {
				cerr << "Failed to Print enumerated list of modules: " << GetLastError() << endl;
			}
		}
	} else {
		cerr << "Failed to enum the modules: " << GetLastError() << endl;
	}

	// Which Module?
	string s_mod_id = "";
	cout << "Into which module would you like to inject faults? [ID]: ";
	getline(cin, s_mod_id);
	int mod_id = stoi(s_mod_id);

	MODULEINFO lModInfo = { 0 };
	cout << "Dll Information:" << endl;
	if (GetModuleInformation(hTarget, hmods[mod_id], &lModInfo, sizeof(lModInfo))){

		cout << "\t Base Addr: " << lModInfo.lpBaseOfDll << endl;
		cout << "\t Entry Point: " << lModInfo.EntryPoint << endl;
		cout << "\t Size of image: " << lModInfo.SizeOfImage << endl << endl;

	} else {
		cerr << "Failed to get module information: " << GetLastError() << endl;
		return -1;
	}

	// Get module name
	TCHAR szModName[MAX_PATH] = TEXT("<unknown>");
	GetModuleFileNameEx(hTarget, hmods[mod_id], szModName, sizeof(szModName) / sizeof(TCHAR));

	// Build library object
	Library *library = new Library(hTarget, (DWORD64)lModInfo.lpBaseOfDll, 
									lModInfo.SizeOfImage, string((char *)&szModName));

	// Save library for future static analysis
	library->write_library_to_disk("C:\\memdump.dll");

	library->inject();

	// Send syslog message
	SendSyslog();

	return 0;
}

bool SendSyslog() {
	WSADATA wsaData;
	int iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult != NO_ERROR) {
		cerr << "Couldn't send syslog message" << endl;
		return false;
	}

	SOCKET ConnectSocket;
	ConnectSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (ConnectSocket == INVALID_SOCKET) {
		cerr << "Couldn't send syslog message" << endl;
		WSACleanup();
		return false;
	}

	sockaddr_in clientService;
	clientService.sin_family = AF_INET;
	clientService.sin_addr.s_addr = inet_addr("192.168.224.7");
	clientService.sin_port = htons(514);
	
	iResult = connect(ConnectSocket, (SOCKADDR *)&clientService, sizeof(clientService));
	if (iResult == SOCKET_ERROR) {
		cerr << "Couldn't send syslog message" << endl;
		closesocket(ConnectSocket);
		WSACleanup();
		return false;
	}

	char *sendbuf = "FAULT_INJECTED_SUCCESSFULLY";
	iResult = send(ConnectSocket, sendbuf, (int)strlen(sendbuf), 0);
	if (iResult == SOCKET_ERROR) {
		cerr << "Couldn't send syslog message" << endl;
		closesocket(ConnectSocket);
		WSACleanup();
		return false;
	}
	cout << "Successfully sent syslog message" << endl;

	closesocket(ConnectSocket);
	WSACleanup();
	return true;
}

char* GetAddressOfData(HANDLE process, const char *data, size_t len) {

	SYSTEM_INFO si;
	GetSystemInfo(&si);

	MEMORY_BASIC_INFORMATION info;
	vector<char> chunk;
	char* p = 0;
	while(p < si.lpMaximumApplicationAddress) {

	  if(VirtualQueryEx(process, p, &info, sizeof(info)) == sizeof(info)) {

		p = (char*)info.BaseAddress;
		chunk.resize(info.RegionSize);
		SIZE_T bytesRead;

		if(ReadProcessMemory(process, p, &chunk[0], info.RegionSize, &bytesRead)) 
		  for(size_t i = 0; i < (bytesRead - len); ++i) 
			if(memcmp(data, &chunk[i], len) == 0)
			  return (char*)p + i;
			 
		p += info.RegionSize;
		
	  }
	}
	return 0;
}

