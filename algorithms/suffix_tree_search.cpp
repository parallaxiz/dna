#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <algorithm>
#include <chrono>

using namespace std;

struct SuffixTree {
    struct Node {
        int start, end, suffixLink;
        int children[5];
        Node(int s = -1, int e = -1) : start(s), end(e), suffixLink(0) {
            for(int i=0; i<5; i++) children[i] = -1;
        }
        int edgeLen(int leafEnd) const { return (end == -1 ? leafEnd : end) - start; }
    };

    string text;
    vector<Node> nodes;
    int root, leafEnd, activeNode, activeEdge, activeLen, remaining, lastNewNode;

    static int charToIndex(char c) {
        if (c == 'A') return 0; if (c == 'T') return 1;
        if (c == 'G') return 2; if (c == 'C') return 3; return 4;
    }

    SuffixTree() : leafEnd(0), activeNode(0), activeEdge(0), activeLen(0), remaining(0), lastNewNode(-1) {}

    SuffixTree(const string& s) : text(s), leafEnd(0), activeLen(0), remaining(0), lastNewNode(-1) {
        if (s.size() < 10000000) nodes.reserve(s.size() * 2 + 10);
        else nodes.reserve(s.size() * 1.5);
        root = newNode(-1, -1);
        activeNode = root; activeEdge = 0;
        for (int i = 0; i < (int)s.size(); i++) extend(i);
    }

    int newNode(int start, int end) {
        nodes.emplace_back(start, end);
        return nodes.size() - 1;
    }

    void extend(int pos) {
        leafEnd = pos + 1; remaining++; lastNewNode = -1;
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
                    activeEdge += eLen; activeLen -= eLen; activeNode = nxt; continue;
                }
                if (text[nodes[nxt].start + activeLen] == text[pos]) {
                    if (lastNewNode != -1 && activeNode != root) nodes[lastNewNode].suffixLink = activeNode;
                    activeLen++; break;
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
            if (activeNode == root && activeLen > 0) { activeLen--; activeEdge = pos - remaining + 1; }
            else activeNode = nodes[activeNode].suffixLink ? nodes[activeNode].suffixLink : root;
        }
    }

    void collectLeaves(int node, int depth, vector<int>& res) const {
        struct Frame { int n, d; };
        vector<Frame> st = {{node, depth}};
        while(!st.empty()) {
            Frame f = st.back(); st.pop_back();
            bool isLeaf = true;
            for(int i=0; i<5; i++) if(nodes[f.n].children[i] != -1) { isLeaf = false; break; }
            if (isLeaf) res.push_back(text.size() - f.d);
            else {
                for(int i=0; i<5; i++) {
                    int c = nodes[f.n].children[i];
                    if (c != -1) st.push_back({c, f.d + nodes[c].edgeLen(leafEnd)});
                }
            }
        }
    }

    vector<int> search(const string& pat) const {
        int node = root, m = pat.size();
        for (int pi = 0; pi < m; ) {
            int idx = charToIndex(pat[pi]);
            int child = nodes[node].children[idx];
            if (child == -1) return {};
            int eStart = nodes[child].start, eLen = nodes[child].edgeLen(leafEnd);
            int j = 0;
            while (j < eLen && pi < m) {
                if (text[eStart + j] != pat[pi]) return {};
                j++; pi++;
            }
            if (pi == m) {
                vector<int> matches; collectLeaves(child, pi, matches);
                sort(matches.begin(), matches.end());
                return matches;
            }
            node = child;
        }
        return {};
    }

    void save(const string& path) {
        ofstream ofs(path, ios::binary);
        size_t sz = nodes.size();
        ofs.write((char*)&sz, sizeof(sz));
        ofs.write((char*)nodes.data(), sz * sizeof(Node));
        ofs.write((char*)&root, sizeof(root));
        ofs.write((char*)&leafEnd, sizeof(leafEnd));
    }

    bool load(const string& path) {
        ifstream ifs(path, ios::binary);
        if (!ifs.good()) return false;
        size_t sz; ifs.read((char*)&sz, sizeof(sz));
        nodes.resize(sz);
        ifs.read((char*)nodes.data(), sz * sizeof(Node));
        ifs.read((char*)&root, sizeof(root));
        ifs.read((char*)&leafEnd, sizeof(leafEnd));
        return true;
    }
};

int main(int argc, char* argv[]) {
    string seq, pat, savePath, loadPath; bool jsonOut = false;
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
    if (seq.empty() || seq.back() != '$') seq += "$";

    SuffixTree st;
    st.text = seq;
    double buildUs = 0;
    auto t0 = chrono::high_resolution_clock::now();
    bool loaded = false;
    if (!loadPath.empty()) loaded = st.load(loadPath);
    
    if (!loaded) {
        st = SuffixTree(seq);
        auto t1 = chrono::high_resolution_clock::now();
        buildUs = chrono::duration<double, micro>(t1 - t0).count();
        if (!savePath.empty()) st.save(savePath);
    } else {
        buildUs = 0;
    }

    auto t2 = chrono::high_resolution_clock::now();
    auto matches = st.search(pat);
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
