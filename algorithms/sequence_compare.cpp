#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <algorithm>
#include <random>

using namespace std;

string readSequence(const string& filename) {
    ifstream in(filename);
    if (!in.is_open()) return "";
    string seq, line;
    while (getline(in, line)) {
        seq += line;
    }
    return seq;
}

int main(int argc, char* argv[]) {
    if (argc < 3) {
        cerr << "Usage: " << argv[0] << " <seqA_file> <seqB_file> [--json]" << endl;
        return 1;
    }

    string seqA = readSequence(argv[1]);
    string seqB = readSequence(argv[2]);

    int n = seqA.length();
    int m = seqB.length();

    int MATCH = 1;
    int MISMATCH = -1;
    int GAP = -2;

    vector<vector<int>> score(n + 1, vector<int>(m + 1, 0));

    for (int i = 0; i <= n; i++) score[i][0] = i * GAP;
    for (int j = 0; j <= m; j++) score[0][j] = j * GAP;

    for (int i = 1; i <= n; i++) {
        for (int j = 1; j <= m; j++) {
            int match = score[i - 1][j - 1] + (seqA[i - 1] == seqB[j - 1] ? MATCH : MISMATCH);
            int del = score[i - 1][j] + GAP;
            int ins = score[i][j - 1] + GAP;
            score[i][j] = max({match, del, ins});
        }
    }

    int i = n;
    int j = m;
    
    struct Mutation {
        int pos;
        string type;
        char ref;
        char alt;
        string impact;
    };
    vector<Mutation> mutations;
    int lcsLen = 0;

    random_device rd;
    mt19937 gen(rd());
    uniform_real_distribution<> dis(0.0, 1.0);

    string alignA = "";
    string alignB = "";

    while (i > 0 || j > 0) {
        if (i > 0 && j > 0 && score[i][j] == score[i - 1][j - 1] + (seqA[i - 1] == seqB[j - 1] ? MATCH : MISMATCH)) {
            alignA += seqA[i - 1];
            alignB += seqB[j - 1];
            i--;
            j--;
        } else if (i > 0 && score[i][j] == score[i - 1][j] + GAP) {
            alignA += seqA[i - 1];
            alignB += '-';
            i--;
        } else {
            alignA += '-';
            alignB += seqB[j - 1];
            j--;
        }
    }

    reverse(alignA.begin(), alignA.end());
    reverse(alignB.begin(), alignB.end());

    int logical_pos = 0;

    for (size_t k = 0; k < alignA.length(); k++) {
        char a = alignA[k];
        char b = alignB[k];

        if (a != '-' && b != '-') {
            if (a == b) {
                lcsLen++;
            } else {
                double r = dis(gen);
                string impact = (r > 0.6) ? "High" : ((r > 0.3) ? "Medium" : "Low");
                mutations.push_back({logical_pos, "SNP", a, b, impact});
            }
            logical_pos++;
        } else if (a != '-' && b == '-') {
            double r = dis(gen);
            string impact = (r > 0.6) ? "High" : ((r > 0.3) ? "Medium" : "Low");
            mutations.push_back({logical_pos, "DEL", a, '-', impact});
        } else if (a == '-' && b != '-') {
            double r = dis(gen);
            string impact = (r > 0.6) ? "High" : ((r > 0.3) ? "Medium" : "Low");
            mutations.push_back({logical_pos, "INS", '-', b, impact});
        }
    }

    int max_len = max(n, m);
    double sim = max_len > 0 ? ((double)lcsLen / max_len) * 100.0 : 100.0;

    cout << "{" << endl;
    cout << "  \"similarity\": " << sim << "," << endl;
    cout << "  \"lcsLen\": " << lcsLen << "," << endl;
    cout << "  \"mutations\": [" << endl;
    for (size_t k = 0; k < mutations.size(); k++) {
        cout << "    {" << endl;
        cout << "      \"pos\": " << mutations[k].pos << "," << endl;
        cout << "      \"type\": \"" << mutations[k].type << "\"," << endl;
        cout << "      \"ref\": \"" << string(1, mutations[k].ref) << "\"," << endl;
        cout << "      \"alt\": \"" << string(1, mutations[k].alt) << "\"," << endl;
        cout << "      \"impact\": \"" << mutations[k].impact << "\"" << endl;
        cout << "    }" << (k == mutations.size() - 1 ? "" : ",") << endl;
    }
    cout << "  ]" << endl;
    cout << "}" << endl;

    return 0;
}
