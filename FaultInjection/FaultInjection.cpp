// FaultInjection.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "globals.h"

#include "Operators.h"
#include "Operator.h"
#include "Library.h"

using namespace std;

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
	printf("%-6s %-*s\n", "PID", dwidth-7, "Process");
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

				_tprintf(TEXT("%6u %-*s\n"), aProcesses[i], dwidth-7, szProcessName);
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

	// Inject omfc_1 into first instance of pattern_3
	library->inject();
	// Inject omfc_3 into first instance of pattern_4

	return 0;
}
