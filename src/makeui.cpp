#include <iostream>

using namespace std;

int main(int argc, char **argv)
{
	for (int i = 0; i < 16; ++i) {
		for (int j = 0; j < 16; ++j) {
			cout << "<item row=\"" << i << "\" column=\"" << j << "\">\n";
			cout << "<widget class=\"tile\" name=\"tile_" << i * 16 + j << "\" native=\"true\"/>\n";
			cout << "</item>\n";
		}
	}
}

