#ifndef PARSER_H
#define PARSER_H

#include <iostream>
#include <fstream>
#include <sstream>
#include <unordered_map>
#include <unordered_set>
#include <stack>
#include <queue>
#include <algorithm>
#include <locale>

using namespace std;

class Parser {
private:
    // Data structures (no vectors used)
    unordered_map<string, unordered_set<string>> productions;
    unordered_map<string, unordered_set<string>> firstSets;
    unordered_map<string, unordered_set<string>> followSets;
    unordered_map<string, unordered_map<string, string>> parseTable;
    unordered_set<string> nonTerminals;
    unordered_set<string> terminals = {
        "identifier", "number", "int", "float", "for", "loop",
        "agar", "magar", "<", ">", ",", ";", "{", "}", "==",
        "<", ">", "<=", ">=", "!=", "<>", "+", "-", "*", "/"
    };

    // Output files with UTF-8 BOM
    ofstream parseTree;
    ofstream parseTableFile;
    ofstream cfgFile;
    ofstream firstFile;
    ofstream followFile;
    ofstream errorFile;

    // Parsing state
    queue<string> inputTokens;
    size_t currentTokenIndex = 0;
    bool parseError = false;

    // UTF-8 BOM writer
    void writeBOM(ofstream& file) {
        const unsigned char bom[] = { 0xEF, 0xBB, 0xBF };
        file.write(reinterpret_cast<const char*>(bom), 3);
    }

public:
    void ParserCore() {
        // Open files with UTF-8 encoding
        parseTree.open("parseTree.txt", ios::binary);
        parseTableFile.open("parseTable.txt", ios::binary);
        cfgFile.open("noAmbigutyCFG.txt", ios::binary);
        firstFile.open("First.txt", ios::binary);
        followFile.open("Follow.txt", ios::binary);
        errorFile.open("errors.txt", ios::app | ios::binary);

        writeBOM(parseTree);
        writeBOM(parseTableFile);
        writeBOM(cfgFile);
        writeBOM(firstFile);
        writeBOM(followFile);

        readCFG("cfg_rules.txt");
        removeLeftRecursion();
        computeFirst();
        computeFollow();
        buildParseTable();
        parseTokens();
    }

    void readCFG(const string& filename) {
        ifstream file(filename);
        string line;
        while (getline(file, line)) {
            size_t arrowPos = line.find("->");
            if (arrowPos == string::npos) continue;

            // Extract LHS
            string lhs = line.substr(0, arrowPos);
            lhs.erase(remove_if(lhs.begin(), lhs.end(), ::isspace), lhs.end());

            // Extract RHS productions
            string rhs = line.substr(arrowPos + 2);
            istringstream iss(rhs);
            string production;
            while (getline(iss, production, '|')) {
                production.erase(production.find_last_not_of(" \t") + 1);
                production.erase(0, production.find_first_not_of(" \t"));
                if (production == "?") production = "";
                productions[lhs].insert(production);
                nonTerminals.insert(lhs);
            }
        }
        file.close();
    }

    void removeLeftRecursion() {
        unordered_map<string, unordered_set<string>> newProductions;
        for (auto& prod : productions) {
            string A = prod.first;
            unordered_set<string> alpha, beta;

            for (const string& rule : prod.second) {
                istringstream iss(rule);
                string firstSymbol;
                iss >> firstSymbol;

                if (firstSymbol == A) {
                    string rest;
                    getline(iss, rest);
                    if (rest.empty()) alpha.insert("?");
                    else alpha.insert(rest.substr(1));
                }
                else {
                    beta.insert(rule);
                }
            }

            if (!alpha.empty()) {
                string A_prime = A + "'";
                nonTerminals.insert(A_prime);

                // Update A's productions
                unordered_set<string> newBeta;
                for (const string& b : beta) {
                    newBeta.insert(b + " " + A_prime);
                }
                newProductions[A] = newBeta;

                // A' productions
                unordered_set<string> newAlpha;
                for (const string& a : alpha) {
                    newAlpha.insert(a + " " + A_prime);
                }
                newAlpha.insert("?");
                newProductions[A_prime] = newAlpha;
            }
            else {
                newProductions[A] = prod.second;
            }
        }
        productions = newProductions;

        // Write modified CFG
        for (const auto& prod : productions) {
            cfgFile << prod.first << " -> ";
            cout << prod.first << " -> ";
            for (const string& rule : prod.second) {
                cout << (rule.empty() ? "?" : rule) << " | ";
                cfgFile << (rule.empty() ? "?" : rule) << " | ";
            }
            cfgFile << "\n";
            cout << "\n";
        }
    }

    void computeFirst() {
        bool changed;
        do {
            changed = false;
            for (const auto& prod : productions) {
                string A = prod.first;
                for (const string& rule : prod.second) {
                    istringstream iss(rule);
                    string symbol;
                    bool canDeriveEpsilon = true;

                    while (iss >> symbol && canDeriveEpsilon) {
                        if (terminals.count(symbol)) {
                            if (firstSets[A].insert(symbol).second) changed = true;
                            canDeriveEpsilon = false;
                        }
                        else {
                            size_t before = firstSets[A].size();
                            firstSets[A].insert(firstSets[symbol].begin(), firstSets[symbol].end());
                            if (firstSets[A].size() != before) changed = true;

                            if (!firstSets[symbol].count("?")) {
                                canDeriveEpsilon = false;
                            }
                        }
                    }
                    if (canDeriveEpsilon) {
                        if (firstSets[A].insert("?").second) changed = true;
                    }
                }
            }
        } while (changed);

        // Write FIRST sets
        for (const auto& entry : firstSets) {
            firstFile << "FIRST(" << entry.first << ") = { ";
            cout << "FIRST(" << entry.first << ") = { ";
            for (const string& s : entry.second) {
                cout << (s.empty() ? "?" : s) << " ";
                firstFile << (s.empty() ? "?" : s) << " ";
            }
            cout << "}\n";
            firstFile << "}\n";
        }
    }

    void computeFollow() {
        followSets["program"].insert("$");
        bool changed;
        do {
            changed = false;
            for (const auto& prod : productions) {
                string A = prod.first;
                for (const string& rule : prod.second) {
                    vector<string> symbols;
                    istringstream iss(rule);
                    string symbol;
                    while (iss >> symbol) symbols.push_back(symbol);

                    for (size_t i = 0; i < symbols.size(); i++) {
                        if (terminals.count(symbols[i])) continue;

                        string B = symbols[i];
                        for (size_t j = i + 1; j < symbols.size(); j++) {
                            string C = symbols[j];
                            if (terminals.count(C)) {
                                if (followSets[B].insert(C).second) changed = true;
                                break;
                            }

                            followSets[B].insert(firstSets[C].begin(), firstSets[C].end());
                            followSets[B].erase("?");

                            if (!firstSets[C].count("?")) break;
                        }

                        if (i == symbols.size() - 1 ||
                            (i < symbols.size() - 1 && firstSets[symbols[i + 1]].count("?")))
                        {
                            size_t before = followSets[B].size();
                            followSets[B].insert(followSets[A].begin(), followSets[A].end());
                            if (followSets[B].size() != before) changed = true;
                        }
                    }
                }
            }
        } while (changed);

        // Write FOLLOW sets
        for (const auto& entry : followSets) {
            cout << "FOLLOW(" << entry.first << ") = { ";
            followFile << "FOLLOW(" << entry.first << ") = { ";
            for (const string& s : entry.second) {
                cout << s << " ";
                followFile << s << " ";
            }
            cout << "}\n";
            followFile << "}\n";
        }
    }

    void buildParseTable() {
        for (const auto& prod : productions) {
            string A = prod.first;
            for (const string& rule : prod.second) {
                unordered_set<string> firstAlpha;
                istringstream iss(rule);
                string symbol;
                bool canDeriveEpsilon = true;

                while (iss >> symbol && canDeriveEpsilon) {
                    if (terminals.count(symbol)) {
                        firstAlpha.insert(symbol);
                        canDeriveEpsilon = false;
                    }
                    else {
                        firstAlpha.insert(firstSets[symbol].begin(), firstSets[symbol].end());
                        if (!firstSets[symbol].count("?")) {
                            canDeriveEpsilon = false;
                        }
                    }
                }

                if (canDeriveEpsilon) firstAlpha.insert("?");

                for (const string& term : firstAlpha) {
                    if (term != "?") {
                        parseTable[A][term] = rule;
                    }
                    else {
                        for (const string& followTerm : followSets[A]) {
                            parseTable[A][followTerm] = rule;
                        }
                    }
                }
            }
        }

        // Write parse table
        for (const auto& row : parseTable) {
            parseTableFile << row.first << ":\n";
            cout << row.first << ":\n";
            for (const auto& entry : row.second) {
                cout << "\t" << entry.first << " -> " << (entry.second.empty() ? "?" : entry.second) << "\n";
                parseTableFile << "\t" << entry.first << " -> " << (entry.second.empty() ? "?" : entry.second) << "\n";
            }
        }
    }

    void parseTokens() {
        ifstream tokenFile("token.txt", ios::binary);
        string line;
        while (getline(tokenFile, line)) {
            istringstream iss(line);
            string type, token, lineNum;
            getline(iss, type, ':');
            getline(iss, token, ':');
            inputTokens.push(token);
        }
        inputTokens.push("$");

        stack<string> s;
        s.push("$");
        s.push("program");

        while (!s.empty() && !inputTokens.empty()) {
            string top = s.top();
            string currentToken = inputTokens.front();

            if (top == "$") {
                if (currentToken == "$") {
                    cout << "Valid program structure\n";
                    parseTree << "Valid program structure\n";
                    break;
                }
                else {
                    cout << "SYNTAX_ERROR: Extra tokens\n";
                    errorFile << "SYNTAX_ERROR: Extra tokens\n";
                    break;
                }
            }

            if (top == currentToken) {
                s.pop();
                inputTokens.pop();
            }
            else if (terminals.count(top)) {
                cout << "SYNTAX_ERROR: Expected '" << top << "' found '" << currentToken << "'\n";
                errorFile << "SYNTAX_ERROR: Expected '" << top << "' found '" << currentToken << "'\n";
                panicMode(top, s, currentToken);
            }
            else {
                if (parseTable[top].count(currentToken)) {
                    string production = parseTable[top][currentToken];
                    s.pop();

                    if (!production.empty()) {
                        istringstream iss(production);
                        stack<string> temp;
                        string symbol;
                        while (iss >> symbol) temp.push(symbol);
                        while (!temp.empty()) {
                            if (temp.top() != "?") s.push(temp.top());
                            temp.pop();
                        }
                    }
                    printParseTree(top + " ? " + production, 0);
                }
                else {
                    cout << "SYNTAX_ERROR: No rule for " << top << " with " << currentToken << "\n";
                    errorFile << "SYNTAX_ERROR: No rule for " << top << " with " << currentToken << "\n";
                    panicMode(top, s, currentToken);
                }
            }
        }
    }

    void panicMode(const string& nonTerminal, stack<string>& s, const string& token) {
        while (!s.empty()) {
            string top = s.top();
            s.pop();
            if (!isTerminal(top) && followSets[top].count(token)) {
                return;
            }
        }
        s.push("$");
        s.push("program");
        inputTokens.pop();
    }

    bool isTerminal(const string& symbol) {
        return terminals.count(symbol) || symbol == "$";
    }

    void printParseTree(const string& node, int depth) {
        string indent(depth * 2, ' ');
        cout << indent << node << "\n";
        parseTree << indent << node << "\n";
    }
};

#endif // PARSER_H