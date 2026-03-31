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
    struct Node {
        int start, end;           // edge label = seq[start..end)
        int suffixLink;
        unordered_map<char, int> children;
        Node(int s = -1, int e = -1) : start(s), end(e), suffixLink(0) {}
        int edgeLen(int leafEnd) const { return (end == -1 ? leafEnd : end) - start; }
    };

    string text;
    vector<Node> nodes;
    int root, leafEnd;
    int activeNode, activeEdge, activeLen;
    int remaining;
    int lastNewNode;

    SuffixTree(const string& s) : text(s), leafEnd(0), activeLen(0), remaining(0), lastNewNode(-1) {
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

            char c = text[activeEdge];
            if (nodes[activeNode].children.count(c) == 0) {
                nodes[activeNode].children[c] = newNode(pos, -1);
                if (lastNewNode != -1) {
                    nodes[lastNewNode].suffixLink = activeNode;
                    lastNewNode = -1;
                }
            } else {
                int nxt = nodes[activeNode].children[c];
                int eLen = nodes[nxt].edgeLen(leafEnd);
                if (activeLen >= eLen) {
                    activeEdge += eLen;
                    activeLen -= eLen;
                    activeNode = nxt;
                    continue;
                }
                if (text[nodes[nxt].start + activeLen] == text[pos]) {
                    activeLen++;
                    if (lastNewNode != -1) {
                        nodes[lastNewNode].suffixLink = activeNode;
                        lastNewNode = -1;
                    }
                    break;
                }
                // split
                int split = newNode(nodes[nxt].start, nodes[nxt].start + activeLen);
                nodes[activeNode].children[c] = split;
                nodes[split].children[text[pos]] = newNode(pos, -1);
                nodes[nxt].start += activeLen;
                nodes[split].children[text[nodes[nxt].start]] = nxt;
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

    // ── collect all leaf positions under a node ────────────
    void collectLeaves(int node, int depth, vector<int>& result) const {
        const Node& nd = nodes[node];
        if (nd.children.empty()) {
            // leaf: suffix starts at (text.size() - depth)
            result.push_back(text.size() - depth);
            return;
        }
        for (auto& [ch, child] : nd.children) {
            int eLen = nodes[child].edgeLen(leafEnd);
            collectLeaves(child, depth + eLen, result);
        }
    }

    // ── search ─────────────────────────────────────────────
    vector<int> search(const string& pat, bool verbose) const {
        vector<int> matches;
        int node = root;
        int pi = 0;  // index into pattern
        int m = pat.size();
        int step = 0;

        if (verbose) {
            cout << DIM << "──────── Tree Traversal for Pattern ────────" << RESET << "\n\n";
            cout << "  Looking for: " << colourSeq(pat) << " (" << m << " bp)\n\n";
        }

        while (pi < m) {
            char c = pat[pi];
            step++;

            if (nodes[node].children.count(c) == 0) {
                if (verbose)
                    cout << "    step " << setw(2) << step
                         << " │ no edge for '" << colourBase(c) << "' at node " << node
                         << "  → " << RED << "NOT FOUND" << RESET << "\n";
                return {};
            }

            int child = nodes[node].children.at(c);
            int eStart = nodes[child].start;
            int eLen = nodes[child].edgeLen(leafEnd);
            string edgeLabel = text.substr(eStart, eLen);

            if (verbose) {
                cout << "    step " << setw(2) << step
                     << " │ follow edge '" << colourSeq(edgeLabel.substr(0, min(20, (int)edgeLabel.size()))) << "'";
                if ((int)edgeLabel.size() > 20) cout << DIM << "..." << RESET;
                cout << "  (node " << node << " → " << child << ")\n";
            }

            // match along the edge
            int j = 0;
            while (j < eLen && pi < m) {
                if (text[eStart + j] != pat[pi]) {
                    if (verbose)
                        cout << "           mismatch at edge offset " << j
                             << ": '" << colourBase(text[eStart + j]) << "' vs '"
                             << colourBase(pat[pi]) << "'  → " << RED << "NOT FOUND" << RESET << "\n";
                    return {};
                }
                j++;
                pi++;
            }

            if (pi < m) {
                node = child;  // continue from child
            } else {
                // pattern fully matched — collect leaves
                if (j < eLen) {
                    // pattern ends in the middle of an edge
                    if (verbose)
                        cout << "           pattern matched at edge offset " << j
                             << " (mid-edge) → collecting leaves\n";
                }
                int depth = pi;  // characters consumed from root
                // but we need depth from this child node
                collectLeaves(child, depth, matches);
            }
        }

        sort(matches.begin(), matches.end());

        if (verbose) {
            cout << "\n  Traversal steps: " << BOLD << step << RESET << "\n";
        }

        return matches;
    }

    // ── print tree structure (abbreviated) ─────────────────
    void printTree(int node, int depth, int maxDepth) const {
        if (depth > maxDepth) return;
        for (auto& [ch, child] : nodes[node].children) {
            int eLen = nodes[child].edgeLen(leafEnd);
            string label = text.substr(nodes[child].start, min(eLen, 25));

            for (int i = 0; i < depth; i++) cout << "  ";
            cout << DIM << "├─ " << RESET << colourSeq(label);
            if (eLen > 25) cout << DIM << "..." << RESET;
            cout << DIM << "  (node " << child << ")" << RESET;

            if (nodes[child].children.empty())
                cout << "  " << GREEN << "●" << RESET;                // leaf
            cout << "\n";

            printTree(child, depth + 1, maxDepth);
        }
    }
};

// ── main ───────────────────────────────────────────────────
int main(int argc, char* argv[]) {
    string seq = "ATGAAATCGATCGATCGATCGTAGCTAGCTAGCTATGAAAGCTAGCTATGAAATCGATCGTAGCTATGAAAGCTAGCTATGAAA";
    string pat = "ATGAAA";

    if (argc >= 3) {
        seq = argv[1];
        pat = argv[2];
    }

    for (auto& c : seq) c = toupper(c);
    for (auto& c : pat) c = toupper(c);

    // Append sentinel
    string seqS = seq + "$";

    cout << "\n" << BOLD << GREEN
         << "╔══════════════════════════════════════════════════╗\n"
         << "║          SUFFIX TREE PATTERN SEARCH             ║\n"
         << "╚══════════════════════════════════════════════════╝"
         << RESET << "\n\n";

    cout << DIM << "Algorithm : " << RESET << "Ukkonen's online construction + tree walk\n";
    cout << DIM << "Build     : " << RESET << "O(n)\n";
    cout << DIM << "Search    : " << RESET << "O(m + k)   (k = number of matches)\n";
    cout << DIM << "Sequence  : " << RESET << seq.size() << " bp\n";
    cout << DIM << "Pattern   : " << RESET << colourSeq(pat) << " (" << pat.size() << " bp)\n\n";

    // Build
    auto t0 = chrono::high_resolution_clock::now();
    SuffixTree st(seqS);
    auto t1 = chrono::high_resolution_clock::now();
    double buildUs = chrono::duration<double, micro>(t1 - t0).count();

    cout << DIM << "──────── Tree Structure (depth ≤ 3) ────────" << RESET << "\n\n";
    cout << DIM << "  root" << RESET << "\n";
    st.printTree(st.root, 1, 3);
    cout << "\n  " << DIM << "Total nodes: " << RESET << BOLD << st.nodes.size() << RESET << "\n\n";

    // Search
    auto t2 = chrono::high_resolution_clock::now();
    auto matches = st.search(pat, true);
    auto t3 = chrono::high_resolution_clock::now();
    double searchUs = chrono::duration<double, micro>(t3 - t2).count();

    cout << "\n" << DIM << "──────── Results ────────" << RESET << "\n\n";
    cout << "  Matches found   : " << BOLD << GREEN << matches.size() << RESET << "\n";
    cout << "  Match positions : ";
    for (size_t k = 0; k < matches.size(); k++) {
        if (k) cout << ", ";
        cout << CYAN << matches[k] << RESET;
    }
    if (matches.empty()) cout << DIM << "(none)" << RESET;
    cout << "\n";

    cout << "\n" << DIM << "──────── Timing ────────" << RESET << "\n\n";
    auto fmtTime = [](double us) -> string {
        char buf[64];
        if (us < 1000) snprintf(buf, sizeof(buf), "%.2f µs", us);
        else            snprintf(buf, sizeof(buf), "%.3f ms", us / 1000.0);
        return string(buf);
    };
    cout << "  Build time  : " << BOLD << YELLOW << fmtTime(buildUs) << RESET << "\n";
    cout << "  Search time : " << BOLD << YELLOW << fmtTime(searchUs) << RESET << "\n";
    cout << "  Total       : " << BOLD << YELLOW << fmtTime(buildUs + searchUs) << RESET << "\n\n";

    return 0;
}
