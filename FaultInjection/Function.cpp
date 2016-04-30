
#include "stdafx.h"
#include "Function.h"
#include "Operators.h"

Function::Function(HANDLE _target, DWORD64 _start, DWORD64 _end, byte *_code) {
	hTarget = _target;
	start_addr = _start;
	end_addr = _end;
	size = end_addr - start_addr;
	buf = _code;
	local_injection_points = map < DWORD64, Operator *>();
	build_injection_points();
}

Function::~Function() { }

// Public Functions

bool Function::inject() {
	// For each injection point in the funciton, ask the user if they would like to inject
	for (map<DWORD64, Operator *>::iterator it = local_injection_points.begin();
		it != local_injection_points.end(); ++it) {

		// If the user injects, return true;
		if (inject(it->second, it->first))
			return true;
	}
	return false;
}

// Private Functions
bool Function::build_injection_points() {
	find_operators_mfc();

	// TODO: Other operators

	return true;
}

// Returns address of injection point for "Operator for Missing Function Call (OMFC)"
bool Function::find_operators_mfc() {
	// Patterns found
	const byte omfc_1[] = { 0xff, 0x15, 0xd4, 0xee, 0x00, 0x00 };
	const byte omfc_2[] = { 0xff, 0xd0 };
	const byte omfc_3[] = { 0xff, 0x15, 0xbd, 0xa4, 0x1c, 0x00 };

	vector < Operator *> list_of_omfc = vector < Operator *>();
	list_of_omfc.push_back(new Operator(omfc_1, sizeof(omfc_1)));
	list_of_omfc.push_back(new Operator(omfc_2, sizeof(omfc_2)));
	list_of_omfc.push_back(new Operator(omfc_3, sizeof(omfc_3)));

	// Check for each function type
	for (vector < Operator *>::iterator it = list_of_omfc.begin();
			it != list_of_omfc.end(); ++it) {

		// Iterate through entire peice of code
		DWORD64 current_offset = 0;
		DWORD64 start = 0;
		while (find_pattern(*it, start, size, &current_offset)) {
			// TODO:  Check constraints
			local_injection_points[start_addr + current_offset] = *it;
			start = current_offset + 1;
		}
	}
	return true;
}

// Search 'buf' for 'pattern' at 'start'.  If found, sets 'offset', and returns true.
bool Function::find_pattern(Operator *op, DWORD64 start, DWORD64 stop, DWORD64 *offset) {

	const byte *pattern = op->pattern();
	for (DWORD64 i = start; i < stop; i++) {
		if (buf[i] == pattern[0]) {
			for (int j = 1; j < op->size(); j++) {
				if (buf[i + j] != pattern[j])
					break;
				if (j < op->size() - 1)
					continue;

				*offset = i;
				return true;
			}
		}
	}
	return false;
}

bool Function::inject(Operator *op, DWORD64 addr) {

	// Ready to continue?
	string cont = "";
	printf("Ready to inject %d bytes at: 0x%X\n\n", op->size(), addr);
	cout << "Continue? [Y|n]: ";
	getline(cin, cont);

	if (cont.find("n") != string::npos || cont.find("N") != string::npos) {
		cout << "Aborting" << endl;
		return false;
	}

	byte *nop_array = (byte *)malloc(op->size());
	fill_n(nop_array, op->size(), 0x90);

	SIZE_T mem_bytes_written = 0;
	if (WriteProcessMemory(hTarget, (LPVOID)addr, nop_array, op->size(), &mem_bytes_written) != 0) {
		cout << "Bytes written: " << mem_bytes_written << endl;
		cout << "Successful injection." << endl;
		return true;
	} else {
		cerr << "Failed to inject fault into memory: " << GetLastError() << endl;
		return false;
	}
	return false;
}
