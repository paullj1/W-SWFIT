#include "stdafx.h"
#include "Operator.h"

Operator::Operator(const byte *pattern, DWORD64 size) {
	_size = size;
	_pattern = (byte *)malloc(_size);
	memcpy(_pattern, pattern, size);
}

Operator::~Operator() {
	free(_pattern);
}