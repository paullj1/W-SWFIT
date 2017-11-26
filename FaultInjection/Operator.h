// Class definition for the Operator object 
// This object contains a byte array and a size
#pragma once

#ifndef OPERATOR_H
#define OPERATOR_H

#include "stdafx.h"
#include "globals.h"

using namespace std;

class Operator 
{
	public:
		Operator(const byte *pattern, DWORD64 size);
		~Operator();

		DWORD64 size() { return _size; }
		const byte *pattern() { return (const byte *)_pattern; }

	private:
		byte *_pattern;
		DWORD64 _size;
};

#endif
