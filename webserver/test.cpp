/*************************************************************************
    > File Name: test.cpp
    > Author: 
    > Mail: 
    > Created Time: Fri 21 Feb 2025 05:32:37 PM CST
 ************************************************************************/


#include "../common/common.h"
#include <iostream>

using namespace std;

int main() {
	char path[200];
	getcwd(path, 200);
	cout << path << endl;
	cout << strlen(path) << endl;
	for (int i = strlen(path); i >= 0; --i) {
		if (path[i] == '/') {
			path[i] = '\0';
			break;
		}
	}
	cout << path << endl;
	cout << strlen(path) << endl;
	return 0;
}
