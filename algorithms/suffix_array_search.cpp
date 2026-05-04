/*
 * ============================================================
 *  SUFFIX ARRAY PATTERN SEARCH — O(n log n) build, O(m log n) search
 * ============================================================
 *  1. Build a suffix array by sorting all suffixes of the sequence.
 *  2. Use binary search on the sorted array to find the pattern.
 *
 *  Compile : g++ -std=c++17 -o suffix_array_search suffix_array_search.cpp
 *  Run     : ./suffix_array_search
 *            ./suffix_array_search <sequence> <pattern>
 * ============================================================
 */

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <algorithm>
#include <chrono>
#include <iomanip>
#include <numeric>

using namespace std;

// ── colour helpers (ANSI) ──────────────────────────────────
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

// ── build suffix array ─────────────────────────────────────
vector<int> buildSuffixArray(const string& s, bool verbose) {
    int n = s.size();
    vector<int> sa(n);
    iota(sa.begin(), sa.end(), 0);   // sa = {0, 1, 2, ..., n-1}

    sort(sa.begin(), sa.end(), [&](int a, int b) {
        return s.compare(a, string::npos, s, b, string::npos) < 0;
    });

    if (verbose) {
        cout << DIM << "──────── Suffix Array (first 20 entries) ────────" << RESET << "\n\n";
        cout << "  " << DIM << "Rank" << RESET << "  │ "
             << DIM << " Idx" << RESET << "  │  "
             << DIM << "Suffix (truncated)" << RESET << "\n";
        cout << DIM << "  ──────┼───────┼─────────────────────────────" << RESET << "\n";

        int show = min(n, 20);
        for (int i = 0; i < show; i++) {
            string suffix = s.substr(sa[i], min(40, n - sa[i]));
            cout << "  " << setw(4) << i << "  │ "
                 << setw(4) << sa[i] << "  │  "
                 << colourSeq(suffix);
            if (n - sa[i] > 40) cout << DIM << "..." << RESET;
            cout << "\n";
        }
        if (n > 20) {
            cout << DIM << "  ... (" << (n - 20) << " more suffixes)" << RESET << "\n";
        }
        cout << "\n";
    }

    return sa;
}

// ── binary search on suffix array ──────────────────────────
vector<int> suffixArraySearch(const string& seq, const vector<int>& sa,
                               const string& pat, bool verbose) {
    int n = seq.size();
    int m = pat.size();
    vector<int> matches;
    int step = 0;

    if (verbose) {
        cout << DIM << "──────── Binary Search for Pattern ────────" << RESET << "\n\n";
        cout << "  Looking for: " << colourSeq(pat) << " (" << m << " bp)\n\n";
    }

    // ── find lower bound ───────────────────────────────────
    int lo = 0, hi = n - 1;
    int lower = n;  // first position where pattern is a prefix

    if (verbose) cout << "  " << MAGENTA << "Finding LOWER bound:" << RESET << "\n";

    while (lo <= hi) {
        int mid = lo + (hi - lo) / 2;
        step++;
        int cmp = seq.compare(sa[mid], min(m, n - sa[mid]), pat);

        if (verbose && step <= 15) {
            string suffix = seq.substr(sa[mid], min(m + 4, n - sa[mid]));
            cout << "    step " << setw(2) << step
                 << " │ lo=" << setw(4) << lo
                 << " hi=" << setw(4) << hi
                 << " mid=" << setw(4) << mid
                 << " │ SA[" << mid << "]=" << setw(4) << sa[mid]
                 << " │ \"" << colourSeq(suffix) << "\""
                 << " cmp=" << (cmp < 0 ? RED + string("< 0") : cmp > 0 ? GREEN + string("> 0") : CYAN + string("= 0"))
                 << RESET << "\n";
        }

        if (cmp < 0) {
            lo = mid + 1;
        } else {
            lower = mid;
            hi = mid - 1;
        }
    }

    // ── find upper bound ───────────────────────────────────
    lo = lower;
    hi = n - 1;
    int upper = n;

    if (verbose) cout << "\n  " << MAGENTA << "Finding UPPER bound:" << RESET << "\n";

    while (lo <= hi) {
        int mid = lo + (hi - lo) / 2;
        step++;
        int cmp = seq.compare(sa[mid], min(m, n - sa[mid]), pat);

        if (verbose && step <= 30) {
            string suffix = seq.substr(sa[mid], min(m + 4, n - sa[mid]));
            cout << "    step " << setw(2) << step
                 << " │ lo=" << setw(4) << lo
                 << " hi=" << setw(4) << hi
                 << " mid=" << setw(4) << mid
                 << " │ SA[" << mid << "]=" << setw(4) << sa[mid]
                 << " │ \"" << colourSeq(suffix) << "\""
                 << " cmp=" << (cmp < 0 ? RED + string("< 0") : cmp > 0 ? GREEN + string("> 0") : CYAN + string("= 0"))
                 << RESET << "\n";
        }

        if (cmp <= 0) {
            lo = mid + 1;
        } else {
            upper = mid;
            hi = mid - 1;
        }
    }

    // ── collect matches ────────────────────────────────────
    for (int i = lower; i < upper; i++) {
        // verify full match
        if (sa[i] + m <= n && seq.compare(sa[i], m, pat) == 0) {
            matches.push_back(sa[i]);
        }
    }
    sort(matches.begin(), matches.end());

    if (verbose) {
        cout << "\n  Binary search range: [" << CYAN << lower << RESET
             << ", " << CYAN << upper << RESET << ")\n";
        cout << "  Total binary search steps: " << BOLD << step << RESET << "\n";
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

    if (!jsonOut) cout << "Suffix Array Pattern Search\n";
    auto t0 = chrono::high_resolution_clock::now();
    auto sa = buildSuffixArray(seq, !jsonOut);
    auto t1 = chrono::high_resolution_clock::now();
    buildUs = chrono::duration<double, micro>(t1 - t0).count();

    auto t2 = chrono::high_resolution_clock::now();
    auto matches = suffixArraySearch(seq, sa, pat, !jsonOut);
    auto t3 = chrono::high_resolution_clock::now();
    searchUs = chrono::duration<double, micro>(t3 - t2).count();

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
