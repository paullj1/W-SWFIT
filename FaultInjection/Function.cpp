
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

	// Build Capstone (CS) Array of code
	if (cs_open(CS_ARCH_X86, CS_MODE_64, &cs_handle) != CS_ERR_OK)
		cerr << "Error disassembling code." << endl;

	// Enable op details
	cs_option(cs_handle, CS_OPT_DETAIL, CS_OPT_ON);
	//cs_option(cs_handle, CS_OP_DETAIL, CS_OPT_ON);

	cs_count = cs_disasm(cs_handle, buf, size, start_addr, 0, &code_buf);
	if (cs_count == 0)
		cerr << "Error disassembling code." << endl;


	// Build Injection points based on disassembled code
	build_injection_points();
}

Function::~Function() { 
	cs_close(&cs_handle);
	cs_free(code_buf, size);
}

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

	// Constraint 2 (C02):  Call must not be only statement in the block (size > size of entry + size of exit + 16)
	if (cs_count < 6) return false;

	for (size_t j = 0; j < cs_count; j++) {
		if (string(code_buf[j].mnemonic).find("call") != string::npos){
			// Constraint 1 (C01):  Return value of the function (EAX) must not be used.
			bool constraint01 = false;
			for (size_t i = j + 1; i < cs_count; i++) {
				cs_detail *details = code_buf[i].detail;
				if (code_buf[i].detail) {
					for (size_t k = 0; k < details->regs_read_count; k++) {
						string modreg = string(cs_reg_name(cs_handle, details->regs_read[k]));
						if (modreg.find("eax") != string::npos || modreg.find("rax") != string::npos)
							constraint01 = true;
					}
				}
			}

			// Doesn't violate any of the OMFC constraints, add it
			if (!constraint01) {
				Operator *op = new Operator(code_buf[j].bytes, code_buf[j].size);
				local_injection_points[code_buf[j].address] = op;
			}
		}
	}
		//printf("0x%"PRIx64":\t%s\t\t%s\n", code_buf[j].address, code_buf[j].mnemonic, code_buf[j].op_str);
	return true;
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
