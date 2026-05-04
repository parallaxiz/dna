/*
 * ============================================================
 *  SUFFIX TREE PATTERN SEARCH — O(n) build, O(m + k) search
 * ============================================================
 *  Builds a compressed trie of all suffixes using Ukkonen's
 *  algorithm, then walks the tree to find the pattern.
 *  k = number of occurrences found.
 *
 *  Compile : g++ -std=c++17 -o suffix_tree_search suffix_tree_search.cpp
 *  Run     : ./suffix_tree_search
 *            ./suffix_tree_search <sequence> <pattern>
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

// ── Suffix Tree (Ukkonen's) ───────────────────────────────
struct SuffixTree {
    static int charToIndex(char c) {
        if (c == 'A') return 0;
        if (c == 'T') return 1;
        if (c == 'G') return 2;
        if (c == 'C') return 3;
        return 4; // $
    }

    struct Node {
        int start, end;           // edge label = seq[start..end)
        int suffixLink;
        int children[5];
        Node(int s = -1, int e = -1) : start(s), end(e), suffixLink(0) {
            for(int i=0; i<5; i++) children[i] = -1;
        }
        int edgeLen(int leafEnd) const { return (end == -1 ? leafEnd : end) - start; }
    };

    string text;
    vector<Node> nodes;
    int root, leafEnd;
    int activeNode, activeEdge, activeLen;
    int remaining;
    int lastNewNode;

    SuffixTree(const string& s) : text(s), leafEnd(0), activeLen(0), remaining(0), lastNewNode(-1) {
        // Reserve a more conservative amount to avoid OOM on 32-bit
        // 1.5 * size is usually enough for DNA sequences
        if (s.size() < 10000000) nodes.reserve(s.size() * 2 + 10);
        else nodes.reserve(s.size() * 1.5); 
        root = newNode(-1, -1);
        activeNode = root;
        activeEdge = 0;
        for (int i = 0; i < (int)s.size(); i++) extend(i);
    }

    int newNode(int start, int end) {
        nodes.emplace_back(start, end);
        return nodes.size() - 1;
    }

    void extend(int pos) {
        leafEnd = pos + 1;
        remaining++;
        lastNewNode = -1;

        while (remaining > 0) {
            if (activeLen == 0) activeEdge = pos;

            int idx = charToIndex(text[activeEdge]);
            int nxt = nodes[activeNode].children[idx];

            if (nxt == -1) {
                nodes[activeNode].children[idx] = newNode(pos, -1);
                if (lastNewNode != -1) nodes[lastNewNode].suffixLink = activeNode;
                lastNewNode = activeNode;
            } else {
                int eLen = nodes[nxt].edgeLen(leafEnd);
                if (activeLen >= eLen) {
                    activeEdge += eLen;
                    activeLen -= eLen;
                    activeNode = nxt;
                    continue;
                }

                if (text[nodes[nxt].start + activeLen] == text[pos]) {
                    if (lastNewNode != -1 && activeNode != root) nodes[lastNewNode].suffixLink = activeNode;
                    activeLen++;
                    break;
                }

                int split = newNode(nodes[nxt].start, nodes[nxt].start + activeLen);
                nodes[activeNode].children[idx] = split;
                nodes[split].children[charToIndex(text[pos])] = newNode(pos, -1);
                nodes[nxt].start += activeLen;
                nodes[split].children[charToIndex(text[nodes[nxt].start])] = nxt;

                if (lastNewNode != -1) nodes[lastNewNode].suffixLink = split;
                lastNewNode = split;
            }
            remaining--;
            if (activeNode == root && activeLen > 0) {
                activeLen--;
                activeEdge = pos - remaining + 1;
            } else {
                activeNode = nodes[activeNode].suffixLink ? nodes[activeNode].suffixLink : root;
            }
        }
    }

    // ── collect all leaf positions under a node (iterative) ────────
    void collectLeaves(int rootNode, int rootDepth, vector<int>& result) const {
        struct Frame { int node; int depth; };
        vector<Frame> stack;
        stack.push_back({rootNode, rootDepth});

        while(!stack.empty()) {
            Frame f = stack.back();
            stack.pop_back();

            const Node& nd = nodes[f.node];
            bool isLeaf = true;
            for(int i=0; i<5; i++) if(nd.children[i] != -1) { isLeaf = false; break; }
            
            if (isLeaf) {
                result.push_back(text.size() - f.depth);
            } else {
                for(int i=0; i<5; i++) {
                    int child = nd.children[i];
                    if (child == -1) continue;
                    int eLen = nodes[child].edgeLen(leafEnd);
                    stack.push_back({child, f.depth + eLen});
                }
            }
        }
    }

    // ── search ─────────────────────────────────────────────
    vector<int> search(const string& pat, bool verbose) const {
        vector<int> matches;
        int node = root;
        int m = pat.size();
        int step = 0;

        if (verbose) {
            cout << DIM << "──────── Tree Traversal for Pattern ────────" << RESET << "\n\n";
            cout << "  Looking for: " << colourSeq(pat) << " (" << m << " bp)\n\n";
        }

        for (int pi = 0; pi < m; ) {
            int idx = charToIndex(pat[pi]);
            int child = nodes[node].children[idx];
            if (child == -1) {
                if (verbose) cout << "    step " << step << " │ no edge for '" << colourBase(pat[pi]) << "' → " << RED << "NOT FOUND" << RESET << "\n";
                return {};
            }
            
            int eStart = nodes[child].start;
            int eLen = nodes[child].edgeLen(leafEnd);
            int j = 0;
            while (j < eLen && pi < m) {
                if (text[eStart + j] != pat[pi]) return {};
                j++; pi++;
            }

            if (pi == m) {
                collectLeaves(child, pi, matches);
            } else {
                node = child;
            }
        }

        sort(matches.begin(), matches.end());
        if (verbose) cout << "\n  Traversal steps: " << BOLD << step << RESET << "\n";
        return matches;
    }

    // ── print tree structure (abbreviated) ─────────────────
    void printTree(int node, int depth, int maxDepth) const {
        if (depth > maxDepth) return;
        for (int i=0; i<5; i++) {
            int child = nodes[node].children[i];
            if (child == -1) continue;
            int eLen = nodes[child].edgeLen(leafEnd);
            string label = text.substr(nodes[child].start, min(eLen, 25));

            for (int i = 0; i < depth; i++) cout << "  ";
            cout << DIM << "├─ " << RESET << colourSeq(label);
            if (eLen > 25) cout << DIM << "..." << RESET;
            cout << "\n";
            printTree(child, depth + 1, maxDepth);
        }
    }
};

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

    string seqS = seq;
    if (seqS.empty() || seqS.back() != '$') seqS += "$";
    if (!jsonOut) cout << "Suffix Tree Pattern Search\n";
    auto t0 = chrono::high_resolution_clock::now();
    SuffixTree st(seqS);
    auto t1 = chrono::high_resolution_clock::now();
    buildUs = chrono::duration<double, micro>(t1 - t0).count();

    auto t2 = chrono::high_resolution_clock::now();
    auto matches = st.search(pat, !jsonOut);
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
