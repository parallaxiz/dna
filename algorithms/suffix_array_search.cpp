#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <algorithm>
#include <chrono>

using namespace std;

vector<int> buildSA(const string& s) {
    int n = s.size();
    vector<int> sa(n), rank(n), tmp(n);
    for (int i = 0; i < n; i++) { sa[i] = i; rank[i] = (unsigned char)s[i]; }

    for (int k = 1; k < n; k <<= 1) {
        auto getRank = [&](int i) { return i < n ? rank[i] : -1; };
        vector<int> nsa(n);
        int j = 0;
        for (int i = n - k; i < n; i++) nsa[j++] = i;
        for (int i = 0; i < n; i++) if (sa[i] >= k) nsa[j++] = sa[i] - k;

        int maxRank = *max_element(rank.begin(), rank.end());
        vector<int> cnt(maxRank + 2, 0);
        for (int i = 0; i < n; i++) cnt[rank[i] + 1]++;
        for (int i = 1; i <= maxRank + 1; i++) cnt[i] += cnt[i - 1];
        for (int i = n - 1; i >= 0; i--) sa[--cnt[rank[nsa[i]] + 1]] = nsa[i];

        tmp[sa[0]] = 0;
        for (int i = 1; i < n; i++) {
            bool same = (rank[sa[i]] == rank[sa[i - 1]]) && (getRank(sa[i] + k) == getRank(sa[i - 1] + k));
            tmp[sa[i]] = tmp[sa[i - 1]] + (same ? 0 : 1);
        }
        rank = tmp;
        if (rank[sa[n - 1]] == n - 1) break;
    }
    return sa;
}

vector<int> searchSA(const string& s, const string& pat, const vector<int>& sa) {
    int n = s.size(), m = pat.size();
    int l = 0, r = n - 1, first = -1;
    while (l <= r) {
        int mid = l + (r - l) / 2;
        if (s.substr(sa[mid], m) >= pat) { first = mid; r = mid - 1; }
        else l = mid + 1;
    }
    if (first == -1 || s.substr(sa[first], m) != pat) return {};
    l = 0, r = n - 1;
    int last = -1;
    while (l <= r) {
        int mid = l + (r - l) / 2;
        if (s.substr(sa[mid], m) <= pat) { last = mid; l = mid + 1; }
        else r = mid - 1;
    }
    vector<int> res;
    for (int i = first; i <= last; i++) res.push_back(sa[i]);
    sort(res.begin(), res.end());
    return res;
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
    auto sa = buildSA(seq);
    auto t1 = chrono::high_resolution_clock::now();
    auto matches = searchSA(seq, pat, sa);
    auto t2 = chrono::high_resolution_clock::now();

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
