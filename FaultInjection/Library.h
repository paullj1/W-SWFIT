// Class definition for the Library object (contains single DLL)
#pragma once

#ifndef LIBRARY_H
#define LIBRARY_H

#include "stdafx.h"
#include "globals.h"

#include "Operators.h"
#include "Operator.h"
#include "Function.h"

#include <vector>
#include <map>

using namespace std;

class Library 
{
	public:
		Library(HANDLE _target, DWORD64 _start, DWORD _size, string _path);
		~Library();

		bool write_library_to_disk(string path);
		bool inject();

	private:
		string name; // Name of library
		vector < Function * > functions; // Vector (list) of functions in library
		map < Operator *, Operator * > function_patterns; // Vector of function patterns
		byte *buf; // Buffer for memory contents
		DWORD64 start_addr = 0;
		DWORD image_size = 0;
		HANDLE hTarget;

		bool read_memory_into_bufer();
		bool build_operator_map();
		bool find_functions();
		bool find_pattern(Operator *op, DWORD64 start, DWORD64 stop, DWORD64 *location);
};

#endif
