#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <algorithm>
#include <chrono>

using namespace std;

struct DAWG {
    struct State {
        int len, link, cnt;
        int next[5];
        State() : len(0), link(-1), cnt(0) { for(int i=0; i<5; i++) next[i] = -1; }
    };
    vector<State> st;
    int last;

    static int charToIndex(char c) {
        if (c == 'A') return 0; if (c == 'T') return 1;
        if (c == 'G') return 2; if (c == 'C') return 3; return 4;
    }

    DAWG() { st.emplace_back(); st[0].link = -1; st[0].len = 0; last = 0; }

    void extend(char c) {
        int idx = charToIndex(c);
        if (st[last].next[idx] != -1) {
            int q = st[last].next[idx];
            if (st[q].len == st[last].len + 1) { last = q; st[last].cnt++; return; }
            int clone = st.size(); st.push_back(st[q]);
            st[clone].len = st[last].len + 1; st[clone].cnt = 0;
            while (last != -1 && st[last].next[idx] == q) { st[last].next[idx] = clone; last = st[last].link; }
            st[q].link = clone; last = clone; st[last].cnt++; return;
        }
        int cur = st.size(); st.emplace_back();
        st[cur].len = st[last].len + 1; st[cur].cnt = 1;
        int p = last;
        while (p != -1 && st[p].next[idx] == -1) { st[p].next[idx] = cur; p = st[p].link; }
        if (p == -1) st[cur].link = 0;
        else {
            int q = st[p].next[idx];
            if (st[q].len == st[p].len + 1) st[cur].link = q;
            else {
                int clone = st.size(); st.push_back(st[q]);
                st[clone].len = st[p].len + 1; st[clone].cnt = 0;
                while (p != -1 && st[p].next[idx] == q) { st[p].next[idx] = clone; p = st[p].link; }
                st[q].link = st[cur].link = clone;
            }
        }
        last = cur;
    }

    void computeCounts() {
        int sz = st.size(), maxLen = 0;
        for (int i = 0; i < sz; i++) if (st[i].len > maxLen) maxLen = st[i].len;
        vector<int> count(maxLen + 1, 0);
        for (int i = 0; i < sz; i++) count[st[i].len]++;
        for (int i = 1; i <= maxLen; i++) count[i] += count[i-1];
        vector<int> order(sz);
        for (int i = 0; i < sz; i++) order[--count[st[i].len]] = i;
        for (int i = sz - 1; i >= 0; i--) {
            int v = order[i];
            if (st[v].link != -1) st[st[v].link].cnt += st[v].cnt;
        }
    }

    int search(const string& pat) {
        int cur = 0;
        for (char c : pat) {
            int idx = charToIndex(c);
            if (st[cur].next[idx] == -1) return 0;
            cur = st[cur].next[idx];
        }
        return st[cur].cnt;
    }
};

vector<int> findPositions(const string& seq, const string& pat) {
    vector<int> pos; size_t start = 0;
    while ((start = seq.find(pat, start)) != string::npos) { pos.push_back(start); start++; }
    return pos;
}

int main(int argc, char* argv[]) {
    string seq, pat; bool jsonOut = false;
    for(int i=1; i<argc; i++) {
        string arg = argv[i];
        if (arg == "--json") jsonOut = true;
        else if (seq.empty()) seq = arg;
        else if (pat.empty()) pat = arg;
    }
    if (!seq.empty()) {
        ifstream fs(seq);
        if (fs.good()) { stringstream buffer; buffer << fs.rdbuf(); seq = buffer.str(); }
    }
    if (!pat.empty()) {
        ifstream fp(pat);
        if (fp.good()) { stringstream buffer; buffer << fp.rdbuf(); pat = buffer.str(); }
    }
    for (auto& c : seq) if (c != '$') c = toupper(c);
    for (auto& c : pat) if (c != '$') c = toupper(c);

    auto t0 = chrono::high_resolution_clock::now();
    DAWG dawg; for (char c : seq) dawg.extend(c); dawg.computeCounts();
    auto t1 = chrono::high_resolution_clock::now();
    int count = dawg.search(pat);
    auto t2 = chrono::high_resolution_clock::now();
    auto matches = findPositions(seq, pat);
    auto t3 = chrono::high_resolution_clock::now();

    double buildUs = chrono::duration<double, micro>(t1 - t0).count();
    double searchUs = chrono::duration<double, micro>(t2 - t1).count();

    if (jsonOut) {
        cout << "{\"matches\":[";
        for (size_t k = 0; k < matches.size(); k++) { if (k) cout << ","; cout << matches[k]; }
        cout << "], \"buildUs\":" << buildUs << ", \"searchUs\":" << searchUs << "}\n";
    } else {
        cout << "Matches: " << matches.size() << "\nBuild: " << buildUs << " us\nSearch: " << searchUs << " us\n";
    }
    return 0;
}
