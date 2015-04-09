
// TODO: detect if the solution crashes, and report which test case it was on
// TODO: make it easy to debug such crashes

string get_executable_path() {
    char buff[PATH_MAX];
    ssize_t len = ::readlink("/proc/self/exe", buff, sizeof(buff)-1);
    if (len != -1) {
        buff[len] = '\0';
        return string(buff);
    } else {
        assert(false);
    }
}

#define BUFSIZE 4096
char buf[BUFSIZE];

struct machine {
    string host, tmpdir;
    int jobs;
    vector<FILE*> fp;
    vector<stringstream*> ans;
    vector<int> done;
    vector<int> got;
    vector<bool> dispatched;
    bool initialized;

    machine(string _host, int _jobs) {
        host = _host;
        jobs = _jobs;
        fp = vector<FILE*>(jobs, NULL);
        done = vector<int>(jobs);
        got = vector<int>(jobs);
        dispatched = vector<bool>(jobs, false);
        initialized = false;

        for (int i = 0; i < jobs; i++) {
            ans.push_back(new stringstream());
        }
    }

    string exec(string cmd) {
        FILE *fp = popen(cmd.c_str(), "r");
        assert(fp != NULL);
        stringstream ss;
        while (fgets(buf, BUFSIZE, fp) != NULL) {
            ss << buf;
        }

        assert(pclose(fp) == 0);

        return ss.str();
    }

    void init() {
        stringstream ss;
        ss << "ssh " << host << " \"mktemp -d\"";
        stringstream res(exec(ss.str()));
        res >> tmpdir;
    }

    void copy(string local, string remote) {
        stringstream ss;
        ss << "scp " << local << " " << host << ":" << tmpdir << "/" << remote;
        exec(ss.str());
    }

    void cleanup() {
        stringstream ss;
        ss << "ssh " << host << " \"rm -rf " << tmpdir << "\"";
        exec(ss.str());
    }

    FILE* spawn(int no, vector<int> handle) {
        got[no] = size(handle);
        done[no] = 0;

        stringstream ss;
        ss << "ssh " << host << " \"cd " << tmpdir << " && ./run test.in -r -o ";
        for (int i = 0; i < size(handle); i++) {
            if (i != 0) {
                ss << ",";
            }
            ss << handle[i];
        }
        ss << "\"";

        dispatched[no] = true;
        return fp[no] = popen(ss.str().c_str(), "r");
    }
};

void clear_progress() {
    cerr << "\033[1F\033[2K";
}

void progress(int done, int total, bool clear=true) {
    if (done > 0 && clear) {
        clear_progress();
    }

    int bars = 60;
    int done_bars = static_cast<int>(round(static_cast<double>(done) / total * bars) + 1e-9);
    cerr << "[";
    for (int i = 0; i < done_bars; i++)
        cerr << "#";
    for (int i = done_bars; i < bars; i++)
        cerr << "-";
    cerr << "] (" << done << "/" << total << ")" << endl;
}

void full_clear_progress(vector<machine> &machines) {
    for (int i = 0; i < size(machines); i++) {
        for (int j = 0; j < machines[i].jobs; j++) {
            clear_progress();
        }
    }
}

void full_progress(vector<machine> &machines, bool clear=true) {
    int max_width = 0;

    int cnt = 0;
    for (int i = 0; i < size(machines); i++) {
        cnt += machines[i].jobs;
        stringstream ss;
        ss << machines[i].host << "_" << (machines[i].jobs-1);
        max_width = max(max_width, size(ss.str()));
    }

    if (clear) {
        for (int i = 0; i < cnt; i++) {
            cerr << "\033[1F";
        }
    }

    for (int i = 0; i < size(machines); i++) {
        for (int j = 0; j < machines[i].jobs; j++) {
            stringstream ss;
            ss << machines[i].host << "_" << j;
            int add = max_width - size(ss.str());

            if (clear) {
                cerr << "\033[2K";
            }

            for (int k = 0; k < add; k++) {
                cerr << " ";
            }

            cerr << ss.str() << " ";
            if (!machines[i].initialized) {
                cerr << "INITIALIZING" << endl;
            } else if (!machines[i].dispatched[j]) {
                cerr << "DISPATCHING" << endl;
            } else {
                progress(machines[i].done[j], machines[i].got[j], false);
            }
        }
    }
}

int do_distribute(string distconf, string input_file, vector<int> handle, ostream &ofs) {
    ifstream conf(distconf.c_str());
    string host;
    int jobs;

    int total_jobs = 0;
    vector<machine> machines;
    while (conf >> host >> jobs) {
        machines.push_back(machine(host, jobs));
        total_jobs += jobs;
    }

    full_progress(machines, false);

    for (int i = 0; i < size(machines); i++) {
        machines[i].init();
        machines[i].copy(input_file, "test.in");
        machines[i].copy(get_executable_path(), "run"); // TODO: compile the program on the target machine
        machines[i].initialized = true;
        full_progress(machines);
    }

    vector<int> cnt(total_jobs);
    int left = size(handle);
    while (left > 0) {
        for (int i = 0; i < total_jobs && left > 0; i++) {
            cnt[i]++;
            left--;
        }
    }

    random_shuffle(cnt.begin(), cnt.end());

    int at = 0, cur = 0;
    for (int i = 0; i < size(machines); i++) {
        for (int j = 0; j < machines[i].jobs; j++) {
            if (cnt[cur] == 0) {
                cur++;
                continue;
            }

            vector<int> curhandle;
            for (int k = 0; k < cnt[cur] && at < size(handle); k++) {
                curhandle.push_back(handle[at++]);
            }

            machines[i].spawn(j, curhandle);
            full_progress(machines);
            cur++;
        }
    }

    while (true) {
        struct timeval tv;
        tv.tv_sec = 5;
        tv.tv_usec = 0;

        fd_set rfds;
        FD_ZERO(&rfds);

        int mx = 0;
        bool any = false;
        for (int i = 0; i < size(machines); i++) {
            for (int j = 0; j < machines[i].jobs; j++) {
                if (machines[i].fp[j] != NULL) {
                    FD_SET(fileno(machines[i].fp[j]), &rfds);
                    mx = max(mx, fileno(machines[i].fp[j]));
                    any = true;
                }
            }
        }

        if (!any) {
            break;
        }

        int retval = select(mx + 1, &rfds, NULL, NULL, &tv);
        assert(retval != -1);
        if (retval) {
            for (int i = 0; i < size(machines); i++) {
                for (int j = 0; j < machines[i].jobs; j++) {
                    if (machines[i].fp[j] != NULL && FD_ISSET(fileno(machines[i].fp[j]), &rfds)) {

                        if (fgets(buf, BUFSIZE, machines[i].fp[j]) != NULL) {
                            // cout << machines[i].host << "_" << j << ": " << buf;
                            int at = 0;
                            while (true) {
                                char *nxt = strstr(buf + at, "Case #");
                                if (nxt == NULL) {
                                    break;
                                }
                                machines[i].done[j]++;
                                at = buf - nxt + 1;
                            }

                            *machines[i].ans[j] << buf;
                        } else {
                            assert(pclose(machines[i].fp[j]) == 0);
                            machines[i].fp[j] = NULL;
                        }

                    }
                }
            }

            full_progress(machines);
        }
    }

    for (int i = 0; i < size(machines); i++) {
        machines[i].cleanup();
    }

    full_clear_progress(machines);

    for (int i = 0; i < size(machines); i++) {
        for (int j = 0; j < machines[i].jobs; j++) {
            cout << machines[i].ans[j]->str();
        }
    }

    return 0;
}

int main(int argc, char *argv[]) {

    bool distribute = false;
    string distconf = "";

    bool input_file_specified = false;
    string input_file = "";

    bool do_only = false;
    set<int> only;

    bool raw = false;

    int at = 1;
    while (at < argc) {
        assert(strcmp(argv[at], "") != 0);
        if (argv[at][0] == '-') {
            if (strcmp(argv[at], "-d") == 0 || strcmp(argv[at], "--distribute") == 0) {
                at++;
                distribute = true;
                assert(at < argc);
                distconf = string(argv[at++]);
            } else if (strcmp(argv[at], "-o") == 0 || strcmp(argv[at], "--only") == 0) {
                at++;
                do_only = true;
                assert(at < argc);
                stringstream ss(argv[at++]);
                int x;
                char c;
                while (ss >> x) {
                    assert(1 <= x);
                    only.insert(x);
                    ss >> c;
                }
            } else if (strcmp(argv[at], "-r") == 0 || strcmp(argv[at], "--raw") == 0) {
                raw = true;
                at++;
            } else {
                cerr << "unrecognized option " << argv[at] << endl;
                return 1;
            }
        } else {
            assert(!input_file_specified);
            input_file_specified = true;
            input_file = string(argv[at++]);
        }
    }

    _global_raw = raw;

    assert(input_file_specified);
    assert(input_file.find(".") != string::npos);

    string output_file = input_file.substr(0, input_file.rfind(".")) + ".out";

    ifstream ifs(input_file.c_str());
    ostream &ofs = cout;

    int tests;
    ifs >> tests;

    vector<int> handle(only.begin(), only.end());
    if (handle.size() > 0) {
        assert(handle[handle.size() - 1] <= tests);
    }

    if (!do_only) {
        for (int i = 1; i <= tests; i++) {
            handle.push_back(i);
        }
    }

    int test_cnt = size(handle);
    int test_done = 0;

    if (distribute) {
        ifs.close();
        return do_distribute(distconf, input_file, handle, ofs);
    }

    if (!raw) {
        progress(0, test_cnt);
    }

    solver solv;
    for (int test = 0; test < tests && test_done < test_cnt; test++) {

        if (test + 1 != handle[test_done]) {
            solv.solve(false, ifs, ofs);
            continue;
        }

        _global_test = test;
        solv.solve(true, ifs, ofs);

        test_done++;
        if (!raw) {
            progress(test_done, test_cnt, false);
        }
    }

    if (!raw) {
        clear_progress();
    }

    return 0;
}

