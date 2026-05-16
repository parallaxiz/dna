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
        auto cmp = [&](int i, int j) {
            if (rank[i] != rank[j]) return rank[i] < rank[j];
            int ri = i + k < n ? rank[i + k] : -1;
            int rj = j + k < n ? rank[j + k] : -1;
            return ri < rj;
        };
        sort(sa.begin(), sa.end(), cmp);
        tmp[sa[0]] = 0;
        for (int i = 1; i < n; i++) {
            tmp[sa[i]] = tmp[sa[i-1]] + (cmp(sa[i-1], sa[i]) ? 1 : 0);
        }
        rank = tmp;
        if (rank[sa[n-1]] == n-1) break;
    }
    return sa;
}

vector<int> searchSA(const string& s, const string& pat, const vector<int>& sa) {
    int n = s.size(), m = pat.size();
    if (m == 0) return {};
    int l = 0, r = n - 1, first = -1;
    while (l <= r) {
        int mid = l + (r - l) / 2;
        if (s.substr(sa[mid], m) >= pat) { first = mid; r = mid - 1; }
        else l = mid + 1;
    }
    if (first == -1 || s.substr(sa[first], m) != pat) return {};
    
    l = first, r = n - 1;
    int last = first;
    while (l <= r) {
        int mid = l + (r - l) / 2;
        if (s.substr(sa[mid], m) == pat) { last = mid; l = mid + 1; }
        else r = mid - 1;
    }
    
    vector<int> res;
    for (int i = first; i <= last; i++) res.push_back(sa[i]);
    sort(res.begin(), res.end());
    return res;
}

int main(int argc, char* argv[]) {
    string seq, pat, savePath, loadPath; 
    bool jsonOut = false;
    for(int i=1; i<argc; i++) {
        string arg = argv[i];
        if (arg == "--json") jsonOut = true;
        else if (arg == "--save" && i + 1 < argc) savePath = argv[++i];
        else if (arg == "--load" && i + 1 < argc) loadPath = argv[++i];
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

    vector<int> sa;
    double buildUs = 0;
    auto t0 = chrono::high_resolution_clock::now();
    
    bool loaded = false;
    if (!loadPath.empty()) {
        ifstream ifs(loadPath, ios::binary);
        if (ifs.good()) {
            size_t sz; ifs.read((char*)&sz, sizeof(sz));
            sa.resize(sz);
            ifs.read((char*)sa.data(), sz * sizeof(int));
            loaded = true;
        }
    }
    
    if (!loaded) {
        sa = buildSA(seq);
        auto t1 = chrono::high_resolution_clock::now();
        buildUs = chrono::duration<double, micro>(t1 - t0).count();
        
        if (!savePath.empty()) {
            ofstream ofs(savePath, ios::binary);
            size_t sz = sa.size();
            ofs.write((char*)&sz, sizeof(sz));
            ofs.write((char*)sa.data(), sz * sizeof(int));
        }
    } else {
        auto t1 = chrono::high_resolution_clock::now();
        buildUs = 0; // Indicate build was reused
    }

    auto t2 = chrono::high_resolution_clock::now();
    auto matches = searchSA(seq, pat, sa);
    auto t3 = chrono::high_resolution_clock::now();
    double searchUs = chrono::duration<double, micro>(t3 - t2).count();

    if (jsonOut) {
        cout << "{\"matches\":[";
        for (size_t k = 0; k < matches.size(); k++) { if (k) cout << ","; cout << matches[k]; }
        cout << "], \"buildUs\":" << buildUs << ", \"searchUs\":" << searchUs << "}\n";
    } else {
        cout << "Matches: " << matches.size() << "\nBuild: " << buildUs << " us\nSearch: " << searchUs << " us\n";
    }
    return 0;
}
