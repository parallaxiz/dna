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
    string seq = "ATGAAATCGATCGATCGATCGTAGCTAGCTAGCTATGAAAGCTAGCTATGAAATCGATCGTAGCTATGAAAGCTAGCTATGAAA";
    string pat = "ATGAAA";

    if (argc >= 3) {
        seq = argv[1];
        pat = argv[2];
    }

    // Uppercase
    for (auto& c : seq) c = toupper(c);
    for (auto& c : pat) c = toupper(c);

    // Timed run
    auto t0 = chrono::high_resolution_clock::now();
    auto matches = naiveSearch(seq, pat, true);
    auto t1 = chrono::high_resolution_clock::now();

    double us = chrono::duration<double, micro>(t1 - t0).count();

    cout << "\n" << DIM << "──────── Timing ────────" << RESET << "\n\n";
    if (us < 1000)
        cout << "  Elapsed: " << BOLD << YELLOW << fixed << setprecision(2) << us << " µs" << RESET << "\n";
    else
        cout << "  Elapsed: " << BOLD << YELLOW << fixed << setprecision(3) << us / 1000.0 << " ms" << RESET << "\n";

    cout << "\n";
    return 0;
}
