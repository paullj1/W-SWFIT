#include "stdafx.h"
#include "Process.h"

Library::Library(HANDLE _target, DWORD64 _start, DWORD _size, string _path) {
	hTarget = _target;
	start_addr = _start;
	image_size = _size;
	name = _path;
	buf = (byte *)malloc(image_size);
	function_patterns = map < Operator *, Operator * >();
	functions = vector < Function *>();

	if (!buf) {
		cerr << "Failed to allocate space for memory contents: " << GetLastError() << endl;
		CloseHandle(hTarget);
		return;
	}
	read_memory_into_bufer();
	build_operator_map();
	find_functions();
}


Library::~Library() {
	free(buf);
	CloseHandle(hTarget);
}

// PUBLIC FUNCTIONS
bool Library::write_library_to_disk(string path) {

	cout << "Writing static copy of memory contents for analysis to " << path << endl;
	FILE *fp;
	fopen_s(&fp, path.c_str(), "w");
	SIZE_T bytes_written = 0;
	while (bytes_written < image_size) {
		bytes_written += fwrite(buf, 1, image_size, fp);
	}
	fclose(fp);
	cout << "Wrote " << bytes_written << " bytes." << endl << endl;
	return true;
}

bool Library::inject() {
	// For each function in the module, call public inject funciton
	for (vector< Function *>::iterator it = functions.begin(); it != functions.end(); ++it) {
		if ((*it)->inject())
			return true;
	}
	return true;
}

// PRIVATE FUNCTIONS
bool Library::read_memory_into_bufer() {
	SIZE_T num_bytes_read = 0;
	int count = 0;

	if (ReadProcessMemory(hTarget, (DWORD64 *)start_addr, buf, image_size, &num_bytes_read) != 0) {
		cout << "Buffered memory contents. Got " << num_bytes_read << " bytes." << endl << endl;
		return true;
	}
	else {
		cout << "Failed to read memory: " << GetLastError() << endl;
		return false;
	}
	return false;
}

bool Library::build_operator_map() {
	function_patterns[new Operator(start_pattern_1, sizeof(start_pattern_1))] =
		new Operator(end_pattern_1, sizeof(end_pattern_1));
	function_patterns[new Operator(start_pattern_2, sizeof(start_pattern_2))] =
		new Operator(end_pattern_2, sizeof(end_pattern_2));
	function_patterns[new Operator(start_pattern_3, sizeof(start_pattern_3))] =
		new Operator(end_pattern_3, sizeof(end_pattern_3));
	function_patterns[new Operator(start_pattern_4, sizeof(start_pattern_4))] =
		new Operator(end_pattern_4, sizeof(end_pattern_4));
	function_patterns[new Operator(start_pattern_5, sizeof(start_pattern_5))] =
		new Operator(end_pattern_5, sizeof(end_pattern_5));
	function_patterns[new Operator(start_pattern_6, sizeof(start_pattern_6))] =
		new Operator(end_pattern_6, sizeof(end_pattern_6));
	function_patterns[new Operator(start_pattern_7, sizeof(start_pattern_7))] =
		new Operator(end_pattern_7, sizeof(end_pattern_7));
	function_patterns[new Operator(start_pattern_8, sizeof(start_pattern_8))] =
		new Operator(end_pattern_8, sizeof(end_pattern_8));
	function_patterns[new Operator(start_pattern_9, sizeof(start_pattern_9))] =
		new Operator(end_pattern_9, sizeof(end_pattern_9));
	function_patterns[new Operator(start_pattern_10, sizeof(start_pattern_10))] =
		new Operator(end_pattern_10, sizeof(end_pattern_10));
	function_patterns[new Operator(start_pattern_11, sizeof(start_pattern_11))] =
		new Operator(end_pattern_11, sizeof(end_pattern_11));
	function_patterns[new Operator(start_pattern_12, sizeof(start_pattern_12))] =
		new Operator(end_pattern_12, sizeof(end_pattern_12));
	return true;
}

bool Library::find_functions() {
	for (map < Operator *, Operator * >::iterator it = function_patterns.begin();
			it != function_patterns.end(); ++it ) {

		DWORD64 begin = 0;
		while (find_pattern(it->first, begin, image_size, &begin)) {

			DWORD64 end = 0;
			if (find_pattern(it->second, begin, image_size, &end)) {
				functions.push_back(new Function(hTarget, start_addr + begin + (it->first)->size(), 
												 start_addr + end - (it->second)->size(), &buf[begin]));
			}
			begin++;
		}
	}
	return true;
}

// Search 'buf' for 'pattern' at 'start'.  If found, sets 'offset', and returns true.
bool Library::find_pattern(Operator *op, DWORD64 start, DWORD64 stop, DWORD64 *location) {

	const byte *pattern = op->pattern();
	for (DWORD64 i = start; i < stop; i++) {
		if (buf[i] == pattern[0]) {
			for (int j = 1; j < op->size(); j++) {
				if (buf[i + j] != pattern[j])
					break;
				if (j < op->size() - 1)
					continue;

				*location = i;
				return true;
			}
		}
	}
	return false;
}

