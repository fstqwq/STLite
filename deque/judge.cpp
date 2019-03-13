#include <bits/stdc++.h>
using namespace std;
typedef pair<int, int> pii;
typedef long long LL;
typedef long double LD;
#define x first
#define y second
#define mp(a, b) make_pair(a, b)
#define read(a) scanf("%d", &a)


string name[] = {"one", "two", "three", "four", "five", "six", "seven", "eight"};

int main(int argc, char ** argv)  {
	for (auto i : name) {
		string add = "./data/" + i + "/";
		ifstream t((add + "code.cpp").c_str());
		if (t.is_open()) {
			t.close();
		}
		else break;
		cout << i << " : \n";
		cout << "Copy\n";
		system(("cp " + add + "code.cpp ./code.cpp").c_str());
		cout << "Compiling\n";
		system("g++ code.cpp -o aa -std=c++14 -w");
		cout << "Running\n";
		system("./aa > ans.out");
		if (system(("diff -q -w ans.out " + add + "answer.txt").c_str())) {
			break;
		}
		cout << "Accept\n";
	}
	return 0;
}
