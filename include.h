#include <algorithm>
#include <bitset>
#include <cassert>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <list>
#include <map>
#include <queue>
#include <set>
#include <sstream>
#include <stack>
#include <string>
#include <utility>
#include <vector>
#include <unistd.h>
#include <linux/limits.h>
#include <sys/time.h>
#include <sys/types.h>
using namespace std;

#define all(o) (o).begin(), (o).end()
#define allr(o) (o).rbegin(), (o).rend()
const int INF = 2147483647;
typedef long long ll;
typedef pair<int, int> ii;
typedef vector<int> vi;
typedef vector<ii> vii;
typedef vector<vi> vvi;
typedef vector<vii> vvii;
template <class T> int size(const T &x) { return x.size(); }

#define LOGIC if (!process) return;
#define OUTPUT if (!_global_raw) clear_progress();\
               cout << "Case #" << (_global_test + 1) << ": ";

bool _global_raw;
int _global_test;

void clear_progress();

