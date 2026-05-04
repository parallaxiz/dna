import re
import glob
import os

def patch_file(filepath):
    with open(filepath, 'r', encoding='utf-8') as f:
        content = f.read()
    
    # Add <fstream> and <sstream> if not present
    if '<fstream>' not in content:
        content = content.replace('#include <iostream>', '#include <iostream>\n#include <fstream>\n#include <sstream>')
        
    # Replace main function
    main_regex = re.compile(r'int main\(int argc, char\* argv\[\]\) \{.*', re.DOTALL)
    
    # Check which algorithm it is to preserve its specific build/search logic
    is_sa = 'buildSuffixArray' in content
    is_st = 'SuffixTree' in content
    is_dawg = 'DAWG' in content
    
    main_body = """int main(int argc, char* argv[]) {
    string seq = "";
    string pat = "";
    bool jsonOutput = false;
    
    for (int i = 1; i < argc; i++) {
        string arg = argv[i];
        if (arg == "--json") {
            jsonOutput = true;
        } else if (arg == "--file" && i + 2 < argc) {
            ifstream fseq(argv[++i]);
            if (fseq) { stringstream buffer; buffer << fseq.rdbuf(); seq = buffer.str(); }
            ifstream fpat(argv[++i]);
            if (fpat) { stringstream buffer; buffer << fpat.rdbuf(); pat = buffer.str(); }
        } else if (seq.empty()) {
            seq = arg;
        } else if (pat.empty()) {
            pat = arg;
        }
    }

    if (seq.empty()) seq = "ATGAAATCGATCGATCGATCGTAGCTAGCTAGCTATGAAAGCTAGCTATGAAATCGATCGTAGCTATGAAAGCTAGCTATGAAA";
    if (pat.empty()) pat = "ATGAAA";

    for (auto& c : seq) { if (c != '$') c = toupper(c); }
    for (auto& c : pat) { if (c != '$') c = toupper(c); }

    double buildUs = 0.0;
    double searchUs = 0.0;
"""

    if is_sa:
        main_body += """
    if (!jsonOutput) {
        cout << "\\n" << BOLD << YELLOW
             << "╔══════════════════════════════════════════════════╗\\n"
             << "║         SUFFIX ARRAY PATTERN SEARCH             ║\\n"
             << "╚══════════════════════════════════════════════════╝"
             << RESET << "\\n\\n";
        cout << DIM << "Algorithm : " << RESET << "Sort all suffixes, then binary search\\n";
        cout << DIM << "Build     : " << RESET << "O(n log n)\\n";
        cout << DIM << "Search    : " << RESET << "O(m log n)\\n";
        cout << DIM << "Sequence  : " << RESET << seq.size() << " bp\\n";
        cout << DIM << "Pattern   : " << RESET << colourSeq(pat) << " (" << pat.size() << " bp)\\n\\n";
    }

    auto t0 = chrono::high_resolution_clock::now();
    auto sa = buildSuffixArray(seq, !jsonOutput);
    auto t1 = chrono::high_resolution_clock::now();
    buildUs = chrono::duration<double, micro>(t1 - t0).count();

    auto t2 = chrono::high_resolution_clock::now();
    auto matches = suffixArraySearch(seq, sa, pat, !jsonOutput);
    auto t3 = chrono::high_resolution_clock::now();
    searchUs = chrono::duration<double, micro>(t3 - t2).count();
"""
    elif is_st:
        main_body += """
    string seqS = seq;
    if (seqS.empty() || seqS.back() != '$') seqS += "$";
    
    if (!jsonOutput) {
        cout << "\\n" << BOLD << GREEN
             << "╔══════════════════════════════════════════════════╗\\n"
             << "║          SUFFIX TREE PATTERN SEARCH             ║\\n"
             << "╚══════════════════════════════════════════════════╝"
             << RESET << "\\n\\n";
        cout << DIM << "Algorithm : " << RESET << "Ukkonen's online construction + tree walk\\n";
        cout << DIM << "Build     : " << RESET << "O(n)\\n";
        cout << DIM << "Search    : " << RESET << "O(m + k)\\n";
        cout << DIM << "Sequence  : " << RESET << seq.size() << " bp\\n";
        cout << DIM << "Pattern   : " << RESET << colourSeq(pat) << " (" << pat.size() << " bp)\\n\\n";
    }

    auto t0 = chrono::high_resolution_clock::now();
    SuffixTree st(seqS);
    auto t1 = chrono::high_resolution_clock::now();
    buildUs = chrono::duration<double, micro>(t1 - t0).count();
    
    if (!jsonOutput) {
        cout << DIM << "──────── Tree Structure (depth ≤ 3) ────────" << RESET << "\\n\\n";
        cout << DIM << "  root" << RESET << "\\n";
        st.printTree(st.root, 1, 3);
        cout << "\\n  " << DIM << "Total nodes: " << RESET << BOLD << st.nodes.size() << RESET << "\\n\\n";
    }

    auto t2 = chrono::high_resolution_clock::now();
    auto matches = st.search(pat, !jsonOutput);
    auto t3 = chrono::high_resolution_clock::now();
    searchUs = chrono::duration<double, micro>(t3 - t2).count();
"""
    elif is_dawg:
        main_body += """
    if (!jsonOutput) {
        cout << "\\n" << BOLD << MAGENTA
             << "╔══════════════════════════════════════════════════╗\\n"
             << "║             DAWG (SUFFIX AUTOMATON)             ║\\n"
             << "║              PATTERN SEARCH                     ║\\n"
             << "╚══════════════════════════════════════════════════╝"
             << RESET << "\\n\\n";
    }

    auto t0 = chrono::high_resolution_clock::now();
    DAWG dawg;
    for (int i = 0; i < (int)seq.size(); i++) dawg.extend(seq[i], !jsonOutput, i);
    dawg.computeCounts();
    auto t1 = chrono::high_resolution_clock::now();
    buildUs = chrono::duration<double, micro>(t1 - t0).count();

    if (!jsonOutput) {
        cout << "\\n  Total states: " << BOLD << dawg.st.size() << RESET << "\\n\\n";
        cout << DIM << "──────── Automaton Structure (first 20 states) ────────" << RESET << "\\n\\n";
        dawg.printAutomaton(20);
        cout << "\\n";
    }

    auto t2 = chrono::high_resolution_clock::now();
    auto result = dawg.search(pat, !jsonOutput);
    auto t3 = chrono::high_resolution_clock::now();
    searchUs = chrono::duration<double, micro>(t3 - t2).count();
    
    auto matches = findPositions(seq, pat);
"""
    else: # naive
        main_body += """
    auto t0 = chrono::high_resolution_clock::now();
    auto matches = naiveSearch(seq, pat, !jsonOutput);
    auto t1 = chrono::high_resolution_clock::now();
    searchUs = chrono::duration<double, micro>(t1 - t0).count();
"""

    main_body += """
    if (jsonOutput) {
        cout << "{";
        cout << "\\"matches\\": [";
        for (size_t k = 0; k < matches.size(); k++) {
            if (k) cout << ", ";
            cout << matches[k];
        }
        cout << "], ";
        cout << "\\"buildUs\\": " << buildUs << ", ";
        cout << "\\"searchUs\\": " << searchUs;
        cout << "}\\n";
        return 0;
    }

    cout << "\\n" << DIM << "──────── Results ────────" << RESET << "\\n\\n";
    cout << "  Matches found   : " << BOLD << GREEN << matches.size() << RESET << "\\n";
    cout << "  Match positions : ";
    for (size_t k = 0; k < matches.size(); k++) {
        if (k) cout << ", ";
        cout << CYAN << matches[k] << RESET;
    }
    if (matches.empty()) cout << DIM << "(none)" << RESET;
    cout << "\\n";

    cout << "\\n" << DIM << "──────── Timing ────────" << RESET << "\\n\\n";
    auto fmtTime = [](double us) -> string {
        char buf[64];
        if (us < 1000) snprintf(buf, sizeof(buf), "%.2f µs", us);
        else            snprintf(buf, sizeof(buf), "%.3f ms", us / 1000.0);
        return string(buf);
    };
    if (buildUs > 0) cout << "  Build time  : " << BOLD << YELLOW << fmtTime(buildUs) << RESET << "\\n";
    cout << "  Search time : " << BOLD << YELLOW << fmtTime(searchUs) << RESET << "\\n";
    if (buildUs > 0) cout << "  Total       : " << BOLD << YELLOW << fmtTime(buildUs + searchUs) << RESET << "\\n\\n";

    return 0;
}
"""
    new_content = main_regex.sub(main_body, content)
    
    with open(filepath, 'w', encoding='utf-8') as f:
        f.write(new_content)

for f in glob.glob('algorithms/*.cpp'):
    patch_file(f)
