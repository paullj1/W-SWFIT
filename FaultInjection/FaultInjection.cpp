// FaultInjection.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "Operators.h"

#include <windows.h>
#include <psapi.h>

#include <string>
#include <iostream>
#include <fstream>
#include <stdio.h>
#include <io.h>
#include <tchar.h>

bool find_pattern(const byte pattern[], size_t size_of_pattern, byte *buf, size_t size_of_buf, int start, int *offset);

using namespace std;

int _tmain(int argc, _TCHAR* argv[]) {

	// Declarations
	__int64* start_addr;
	DWORD size_of_ntdsa;
	DWORD aProcesses[1024], cbNeeded, cProcesses;
	TCHAR *sProcesses[1024];
	TCHAR szProcessName[MAX_PATH] = TEXT("<unknown>");
	unsigned int i;

	// Get All pids
	if (!EnumProcesses(aProcesses, sizeof(aProcesses), &cbNeeded)){
		cout << "Failed to get all PIDs: " << GetLastError() << endl;
		return -1;
	}

	// Get screen width
	CONSOLE_SCREEN_BUFFER_INFO csbi;
	GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi);
	int dwidth = csbi.srWindow.Right - csbi.srWindow.Left;

	cout << "Running Processes" << endl;
	printf("%-6s %-*s\n", "PID", dwidth-7, "Process");
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
				sProcesses[i] = szProcessName;
				CloseHandle(hProc);
			}
		}
	}

	// Which process?
	int pid = 0;
	cout << "Into which process would you like to inject faults? [PID]: ";
	cin >> pid;

	HANDLE hTarget = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pid);
	if (!hTarget) {
		cout << "Failed to open process (check your privilege): " << GetLastError() << endl;
		return -1;
	}

	// Get Process Image File Name
	TCHAR *filename = NULL;
	for (i = 0; i < sizeof(aProcesses); i++) {
		if (aProcesses[i] == pid) {
			filename = sProcesses[i];
		}
	}

	if (filename == NULL) {
		cout << "Failed to locate selected PID's name: " << GetLastError() << endl;
		CloseHandle(hTarget);
		return -1;
	}

	// Enumerate modules within process
	HMODULE hmods[1024];
	cout << "DLLs currently loaded in " << filename << ":" << endl;
	printf("%-4s %-*s\n", "ID", dwidth-5, "Module Name:");
	if (EnumProcessModules(hTarget, hmods, sizeof(hmods), &cbNeeded)) {
		for (i = 0; i < (cbNeeded / sizeof(HMODULE)); i++) {
			TCHAR szModName[MAX_PATH];
			if (GetModuleFileNameEx(hTarget, hmods[i], szModName, sizeof(szModName) / sizeof(TCHAR))) {
				_tprintf(TEXT("%4d %-*s\n"), i, dwidth-5, szModName);
			} else {
				cout << "Failed to Print enumerated list of modules: " << GetLastError() << endl;
			}
		}
	} else {
		cout << "Failed to enum the modules: " << GetLastError() << endl;
	}

	// Which Module?
	int mod_id = 0;
	cout << "Into which module would you like to inject faults? [ID]: ";
	cin >> mod_id;

	MODULEINFO lModInfo = { 0 };
	cout << "Dll Information:" << endl;
	if (GetModuleInformation(hTarget, hmods[mod_id], &lModInfo, sizeof(lModInfo))){
		cout << "\t Base Addr: " << lModInfo.lpBaseOfDll << endl;
		cout << "\t Size of image: " << lModInfo.SizeOfImage << endl << endl;

		start_addr = (__int64*)lModInfo.lpBaseOfDll;
		size_of_ntdsa = lModInfo.SizeOfImage;
	}

	byte *buf = (byte *)malloc(size_of_ntdsa);
	if (!buf) {
		cout << "Failed to allocate space for memory contents: " << GetLastError() << endl;
		CloseHandle(hTarget);
		return -1;
	}

	SIZE_T num_bytes_read = 0;
	int count = 0;

	if (ReadProcessMemory(hTarget, start_addr, buf, size_of_ntdsa, &num_bytes_read) != 0) {
		cout << "Buffered memory contents. Got " << num_bytes_read << " bytes." << endl << endl;
	} else {
		int error_code = GetLastError();
		if (error_code == 299) {
			cout << "Partial read. Got " << num_bytes_read << " bytes: " << endl;
		} else  {
			cout << "Failed to read memory: " << GetLastError() << endl;
			CloseHandle(hTarget);
			free(buf);
			return -1;
		}
	}

	// Write DLL to disk for static ref
	cout << "Writing static copy of memory contents for analysis to 'C:\\memdump.dll'" << endl;
	if (num_bytes_read > 0) {
		FILE *fp;
		fopen_s(&fp, "C:\\memdump.dll", "w");
		SIZE_T bytes_written = 0;
		while (bytes_written < num_bytes_read) {
			bytes_written += fwrite(buf, 1, num_bytes_read, fp);
		}
		fclose(fp);
		cout << "Wrote " << bytes_written << " bytes." << endl << endl;
	}

	// Ready to continue?
	string cont = "";
	cout << "Ready to begin fault load.  Continue? [Y|n]: ";
	cin >> cont;
	if (cont.find("n") != string::npos || cont.find("N") != string::npos) {
		CloseHandle(hTarget);
		return 0;
	}



	int offset_of_function = 0;

	// Skip first function
	find_pattern(start_pattern_6, sizeof(start_pattern_6), buf, size_of_ntdsa, 0, &offset_of_function);

	if (find_pattern(start_pattern_6, sizeof(start_pattern_6), buf, size_of_ntdsa, offset_of_function, &offset_of_function)) {
		int offset_of_exit = 0;
		if (find_pattern(end_pattern_6, sizeof(end_pattern_6), buf, size_of_ntdsa, offset_of_function, &offset_of_exit)){

			int size_of_function = offset_of_exit - offset_of_function;
			int call_offset = 0;

			if (find_pattern(omva_1, sizeof(omva_1), buf, size_of_ntdsa, offset_of_function, &call_offset)) {
				byte nop_array[] = { 0x90, 0x90, 0x90, 0x90, 0x90 };
				memcpy(buf + call_offset, nop_array, sizeof(nop_array));

				SIZE_T mem_bytes_written = 0;
				if (WriteProcessMemory(hTarget, start_addr, buf, size_of_ntdsa, &mem_bytes_written) != 0)
					cout << "Failed to write to memory: " << GetLastError() << endl;
				else {
					cout << "Successful injection. Prepare for blue screen." << endl;
					CloseHandle(hTarget);
					free(buf);
					return 0;
				}
			}
		}
	}

	cout << "Failed to find injection point" << endl;
	CloseHandle(hTarget);
	free(buf);
	return -1;
}

// Search 'buf' for 'pattern' at 'start'.  If found, sets 'offset', and returns true.
bool find_pattern(const byte pattern[], size_t size_of_pattern, byte *buf, size_t size_of_buf, int start, int *offset) {

	for (unsigned int i = start; i < size_of_buf; i++) {
		if (buf[i] == pattern[0]) {
			for (int j = 1; j < size_of_pattern; j++) {
				if (buf[i + j] != pattern[j])
					break;
				if (j < size_of_pattern - 1)
					continue;

				*offset = i;
				return true;
			}
		}
	}
	return false;
}
