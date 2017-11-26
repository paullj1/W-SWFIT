// Class definition for the Funciton object
#pragma once

#ifndef FUNCTION_H
#define FUNCTION_H

#include "stdafx.h"
#include "globals.h"

#include "Operator.h"

#include <map>

using namespace std;

class Function 
{
	public:
		Function(HANDLE _target, DWORD64 _start, DWORD64 _end, byte *_code);
		~Function();

		bool inject();

	private:
		map < DWORD64, Operator *> local_injection_points; // Address -> NOP Sequence
		DWORD64 start_addr = 0;
		DWORD64 end_addr = 0;
		byte *buf;
		DWORD64 size = 0;
		HANDLE hTarget;  // Managed by Library (don't close it here)

		// Capstone Buffer
		cs_insn *code_buf;
		size_t cs_count = 0;
		csh cs_handle;

		bool build_injection_points();
		bool perform_injection(DWORD64 addr);
		bool inject(Operator *op, DWORD64 addr);

		// Build map of injectable points
		bool find_operators_mfc();
		bool find_operators_ompla();
};

#endif