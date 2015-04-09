#include "include.h"
#include <unistd.h>

struct test_case {
    int n;
    ll ans;

    test_case() {
    }

    void input(istream &cin) {
        cin >> n;
    }

    void solve() {
        ans = 1;
        for (int i = 2; i <= n; i++) {
            ans *= i;
        }
        // sleep(1);
        for (int i = 0; i < static_cast<ll>(n) * 1000000000 % 10000000000LL; i++) {
            ans += i;
        }
    }

    void output(ostream &cout) {
        cout << ans << endl;
    }
};

#include "gcj.h"
