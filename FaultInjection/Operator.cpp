#include "stdafx.h"
#include "Operator.h"

Operator::Operator(const byte *pattern, DWORD64 size) 
{
	_size = size;
	try
	{
		_pattern = static_cast<byte *>(malloc(_size));
	}
	catch (bad_alloc&)
	{
		cout << "Failed to allocate memory: " << GetLastError() << endl;
		return;
	}
	
	memcpy(_pattern, pattern, size);
}

Operator::~Operator() 
{
	free(_pattern);
}