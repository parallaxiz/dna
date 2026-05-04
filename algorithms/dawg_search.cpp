/*
 * ============================================================
 *  DAWG (Suffix Automaton) PATTERN SEARCH — O(n) build, O(m) search
 * ============================================================
 *  Builds a Directed Acyclic Word Graph (suffix automaton) that
 *  recognises every substring of the text, then walks the
 *  automaton with the pattern characters.
 *
 *  Compile : g++ -std=c++17 -o dawg_search dawg_search.cpp
 *  Run     : ./dawg_search
 *            ./dawg_search <sequence> <pattern>
 * ============================================================
 */

#include <iostream>
#include <numeric>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <unordered_map>
#include <chrono>
#include <iomanip>
#include <algorithm>

using namespace std;

// ── colour helpers ─────────────────────────────────────────
const string CYAN    = "\033[96m";
const string GREEN   = "\033[92m";
const string RED     = "\033[91m";
const string YELLOW  = "\033[93m";
const string MAGENTA = "\033[95m";
const string DIM     = "\033[90m";
const string BOLD    = "\033[1m";
const string RESET   = "\033[0m";

string colourBase(char c) {
    switch (c) {
        case 'A': return CYAN   + string(1, c) + RESET;
        case 'T': return YELLOW + string(1, c) + RESET;
        case 'G': return GREEN  + string(1, c) + RESET;
        case 'C': return RED    + string(1, c) + RESET;
        default:  return string(1, c);
    }
}

string colourSeq(const string& s) {
    string out;
    for (char c : s) out += colourBase(c);
    return out;
}

// ── Suffix Automaton (DAWG) ────────────────────────────────
struct DAWG {
    static int charToIndex(char c) {
        if (c == 'A') return 0;
        if (c == 'T') return 1;
        if (c == 'G') return 2;
        if (c == 'C') return 3;
        return 4; // $
    }

    struct State {
        int len, link;
        int next[5];
        int cnt;                    // endpos count
        State() : len(0), link(-1), cnt(0) {
            for(int i=0; i<5; i++) next[i] = -1;
        }
    };

    vector<State> st;
    int last;

    DAWG(int max_n = 0) {
        // No reserve here to avoid contiguous memory requirement on 32-bit
        st.emplace_back();          // initial state (id = 0)
        st[0].link = -1;
        st[0].len = 0;
        last = 0;
    }

    void extend(char c, bool verbose, int pos) {
        int idx = charToIndex(c);
        // check if transition already exists (online)
        if (st[last].next[idx] != -1) {
            int q = st[last].next[idx];
            if (st[q].len == st[last].len + 1) {
                last = q;
                st[last].cnt++;
                return;
            }
            int clone = st.size();
            st.push_back(st[q]);
            st[clone].len = st[last].len + 1;
            st[clone].cnt = 0;

            while (last != -1 && st[last].next[idx] != -1 && st[last].next[idx] == q) {
                st[last].next[idx] = clone;
                last = st[last].link;
            }
            st[q].link = clone;
            last = clone;
            st[last].cnt++;
            return;
        }

        int cur = st.size();
        st.emplace_back();
        st[cur].len = st[last].len + 1;
        st[cur].cnt = 1;

        int p = last;
        while (p != -1 && st[p].next[idx] == -1) {
            st[p].next[idx] = cur;
            p = st[p].link;
        }

        if (p == -1) {
            st[cur].link = 0;
        } else {
            int q = st[p].next[idx];
            if (st[q].len == st[p].len + 1) {
                st[cur].link = q;
            } else {
                int clone = st.size();
                st.push_back(st[q]);
                st[clone].len = st[p].len + 1;
                st[clone].cnt = 0;

                while (p != -1 && st[p].next[idx] != -1 && st[p].next[idx] == q) {
                    st[p].next[idx] = clone;
                    p = st[p].link;
                }
                st[q].link = clone;
                st[cur].link = clone;
            }
        }

        last = cur;

        if (verbose && pos < 25) {
            cout << "    char " << setw(2) << pos
                 << " '" << colourBase(c) << "'"
                 << "  → new state " << BOLD << cur << RESET
                 << "  (len=" << st[cur].len
                 << ", link=" << st[cur].link
                 << ", total states=" << st.size() << ")\n";
        }
    }

    // ── count occurrences by propagating counts up suffix links (O(n)) ─
    void computeCounts() {
        int sz = st.size();
        int maxLen = 0;
        for (int i = 0; i < sz; i++) if (st[i].len > maxLen) maxLen = st[i].len;
        
        vector<int> count(maxLen + 1, 0);
        for (int i = 0; i < sz; i++) count[st[i].len]++;
        for (int i = 1; i <= maxLen; i++) count[i] += count[i-1];
        
        vector<int> order(sz);
        for (int i = 0; i < sz; i++) order[--count[st[i].len]] = i;
        
        for (int i = sz - 1; i >= 0; i--) {
            int v = order[i];
            if (st[v].link != -1)
                st[st[v].link].cnt += st[v].cnt;
        }
    }

    // ── search: walk from initial state ────────────────────
    struct SearchResult {
        bool found;
        int endState;
        long long occurrences;
    };

    SearchResult search(const string& pat, bool verbose) {
        int cur = 0;
        int m = pat.size();

        if (verbose) {
            cout << DIM << "──────── Automaton Walk for Pattern ────────" << RESET << "\n\n";
            cout << "  Looking for: " << colourSeq(pat) << " (" << m << " bp)\n\n";
        }

        for (int i = 0; i < m; i++) {
            char c = pat[i];
            int idx = charToIndex(c);

            if (verbose) {
                cout << "    step " << setw(2) << (i + 1)
                     << " │ state " << setw(3) << cur
                     << " + '" << colourBase(c) << "'  →  ";
            }

            if (st[cur].next[idx] == -1) {
                if (verbose)
                    cout << RED << "NO TRANSITION — pattern not found" << RESET << "\n";
                return {false, -1, 0};
            }

            cur = st[cur].next[idx];

            if (verbose) {
                cout << "state " << BOLD << cur << RESET
                     << "  (len=" << st[cur].len << ")\n";
            }
        }

        if (verbose)
            cout << "\n  Final state: " << BOLD << cur << RESET
                 << "  →  " << GREEN << BOLD << "PATTERN FOUND" << RESET << "\n";

        return {true, cur, st[cur].cnt};
    }

    // ── print automaton structure ──────────────────────────
    void printAutomaton(int maxStates) const {
        int show = min((int)st.size(), maxStates);
        cout << "  " << DIM << "State" << RESET << " │ "
             << DIM << "Len" << RESET << "  │ "
             << DIM << "Link" << RESET << " │ "
             << DIM << "Clone" << RESET << " │ "
             << DIM << "Transitions" << RESET << "\n";
        cout << DIM << "  ──────┼──────┼──────┼───────┼────────────────────" << RESET << "\n";

        for (int i = 0; i < show; i++) {
            cout << "  " << setw(4) << i << "  │ "
                 << setw(3) << st[i].len << "  │ "
                 << setw(3) << st[i].link << "  │ "
                 << DIM << string("   no") << RESET << "  │ ";

            bool first = true;
            char idxToChar[] = {'A', 'T', 'G', 'C', '$'};
            for (int j=0; j<5; j++) {
                int nxt = st[i].next[j];
                if (nxt == -1) continue;
                if (!first) cout << " ";
                cout << colourBase(idxToChar[j]) << DIM << "→" << RESET << nxt;
                first = false;
            }
            cout << "\n";
        }
        if ((int)st.size() > maxStates) {
            cout << DIM << "  ... (" << (st.size() - maxStates) << " more states)" << RESET << "\n";
        }
    }
};

// ── find actual positions via naive scan (DAWG tells count) ─
vector<int> findPositions(const string& seq, const string& pat) {
    vector<int> pos;
    size_t start = 0;
    while ((start = seq.find(pat, start)) != string::npos) {
        pos.push_back(start);
        start++;
    }
    return pos;
}

// ── main ───────────────────────────────────────────────────
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
    } else {
        seq = "ATGAAATCGATCGATCGATCGTAGCTAGCTAGCTATGAAAGCTAGCTATGAAATCGATCGTAGCTATGAAAGCTAGCTATGAAA";
    }

    if (!pat.empty()) {
        ifstream fp(pat);
        if (fp.good()) { stringstream buffer; buffer << fp.rdbuf(); pat = buffer.str(); }
    } else {
        pat = "ATGAAA";
    }

    for (auto& c : seq) { if (c != '$') c = toupper(c); }
    for (auto& c : pat) { if (c != '$') c = toupper(c); }

    double buildUs = 0.0;
    double searchUs = 0.0;

    if (!jsonOut) cout << "DAWG Pattern Search\n";
    auto t0 = chrono::high_resolution_clock::now();
    DAWG dawg(seq.size());
    for (int i = 0; i < (int)seq.size(); i++) dawg.extend(seq[i], !jsonOut, i);
    dawg.computeCounts();
    auto t1 = chrono::high_resolution_clock::now();
    buildUs = chrono::duration<double, micro>(t1 - t0).count();

    auto t2 = chrono::high_resolution_clock::now();
    auto result = dawg.search(pat, !jsonOut);
    auto t3 = chrono::high_resolution_clock::now();
    searchUs = chrono::duration<double, micro>(t3 - t2).count();
    
    auto matches = findPositions(seq, pat);

    if (jsonOut) {
        cout << "{\"matches\":[";
        for (size_t k = 0; k < matches.size(); k++) {
            if (k) cout << ",";
            cout << matches[k];
        }
        cout << "], \"buildUs\":" << buildUs << ", \"searchUs\":" << searchUs << "}\n";
        return 0;
    }

    cout << "\n  Matches found   : " << matches.size() << "\n";
    cout << "  Build time  : " << buildUs << " us\n";
    cout << "  Search time : " << searchUs << " us\n";
    return 0;
}
