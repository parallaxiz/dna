/*
 * ============================================================
 *  NAIVE (BRUTE FORCE) PATTERN SEARCH — O(n × m)
 * ============================================================
 *  Slides a window of size m across the DNA sequence of length n.
 *  At every position it compares the pattern character-by-character.
 *
 *  Compile : g++ -std=c++17 -o naive_search naive_search.cpp
 *  Run     : ./naive_search
 *            ./naive_search <sequence> <pattern>
 * ============================================================
 */

#include <iostream>
#include <numeric>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <chrono>
#include <iomanip>

using namespace std;

// ── colour helpers (ANSI) ──────────────────────────────────
const string CYAN    = "\033[96m";
const string GREEN   = "\033[92m";
const string RED     = "\033[91m";
const string YELLOW  = "\033[93m";
const string DIM     = "\033[90m";
const string BOLD    = "\033[1m";
const string RESET   = "\033[0m";

// ── pretty-print a base with DNA colour ────────────────────
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

// ── NAIVE SEARCH with step-by-step output ──────────────────
vector<int> naiveSearch(const string& seq, const string& pat, bool verbose) {
    vector<int> matches;
    int n = seq.size();
    int m = pat.size();
    int comparisons = 0;

    if (verbose) {
        cout << "\n" << BOLD << CYAN
             << "╔══════════════════════════════════════════════════╗\n"
             << "║        NAIVE (BRUTE FORCE) PATTERN SEARCH       ║\n"
             << "╚══════════════════════════════════════════════════╝"
             << RESET << "\n\n";

        cout << DIM << "Algorithm : " << RESET << "Slide window of size m across the text\n";
        cout << DIM << "Complexity: " << RESET << "O(n × m)  worst case\n";
        cout << DIM << "Sequence  : " << RESET << n << " bp\n";
        cout << DIM << "Pattern   : " << RESET << colourSeq(pat) << " (" << m << " bp)\n";
        cout << "\n" << DIM << "──────── Step-by-Step Working ────────" << RESET << "\n\n";
    }

    for (int i = 0; i <= n - m; i++) {
        bool match = true;
        int j;

        for (j = 0; j < m; j++) {
            comparisons++;
            if (seq[i + j] != pat[j]) {
                match = false;
                break;
            }
        }

        // ── verbose output (limit to first 30 positions) ──
        if (verbose && i < 30) {
            string window = seq.substr(i, m);

            cout << DIM << "  pos " << RESET
                 << setw(4) << i << "  │  ";

            // show alignment
            for (int k = 0; k < m; k++) {
                if (k < j || (k == j && match)) {
                    // matched so far
                    cout << GREEN << pat[k] << RESET;
                } else if (k == j) {
                    // mismatch position
                    cout << RED << pat[k] << RESET;
                } else {
                    cout << DIM << pat[k] << RESET;
                }
            }

            cout << " vs ";
            for (int k = 0; k < m; k++) {
                if (k < j || (k == j && match))
                    cout << GREEN << window[k] << RESET;
                else if (k == j)
                    cout << RED << window[k] << RESET;
                else
                    cout << DIM << window[k] << RESET;
            }

            if (match) {
                cout << "  " << GREEN << BOLD << "✓ MATCH" << RESET;
            } else {
                cout << "  " << RED << "✗ mismatch at offset " << j << RESET;
            }
            cout << "\n";
        }

        if (match) {
            matches.push_back(i);
        }
    }

    if (verbose && n - m + 1 > 30) {
        cout << DIM << "  ... (" << (n - m + 1 - 30)
             << " more positions checked) ..." << RESET << "\n";
    }

    if (verbose) {
        cout << "\n" << DIM << "──────── Results ────────" << RESET << "\n\n";
        cout << "  Total comparisons : " << BOLD << comparisons << RESET << "\n";
        cout << "  Matches found     : " << BOLD << GREEN << matches.size() << RESET << "\n";
        cout << "  Match positions   : ";
        for (size_t k = 0; k < matches.size(); k++) {
            if (k) cout << ", ";
            cout << CYAN << matches[k] << RESET;
        }
        if (matches.empty()) cout << DIM << "(none)" << RESET;
        cout << "\n";
    }

    return matches;
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

    if (!jsonOut) cout << "Naive Pattern Search\n";
    auto t0 = chrono::high_resolution_clock::now();
    auto matches = naiveSearch(seq, pat, !jsonOut);
    auto t1 = chrono::high_resolution_clock::now();
    searchUs = chrono::duration<double, micro>(t1 - t0).count();

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
