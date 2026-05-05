#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <chrono>

using namespace std;

vector<int> naiveSearch(const string& seq, const string& pat) {
    vector<int> matches;
    int n = seq.size();
    int m = pat.size();
    for (int i = 0; i <= n - m; i++) {
        bool match = true;
        for (int j = 0; j < m; j++) {
            if (seq[i + j] != pat[j]) {
                match = false;
                break;
            }
        }
        if (match) matches.push_back(i);
    }
    return matches;
}

int main(int argc, char* argv[]) {
    string seq, pat;
    bool jsonOut = false;
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
    auto matches = naiveSearch(seq, pat);
    auto t1 = chrono::high_resolution_clock::now();
    double searchUs = chrono::duration<double, micro>(t1 - t0).count();

    if (jsonOut) {
        cout << "{\"matches\":[";
        for (size_t k = 0; k < matches.size(); k++) {
            if (k) cout << ",";
            cout << matches[k];
        }
        cout << "], \"buildUs\":0, \"searchUs\":" << searchUs << "}\n";
    } else {
        cout << "Matches: " << matches.size() << "\nTime: " << searchUs << " us\n";
    }
    return 0;
}
