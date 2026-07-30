#include <bits/stdc++.h>
using namespace std;

struct Value {
    bool is_int; // true if int, false if string
    long long iv = 0;
    string sv;
};

struct Var { // variable definition
    bool is_int;
    Value val;
};

static inline bool is_int_literal(const string &s, long long &out) {
    if (s.empty()) return false;
    size_t i = 0;
    bool neg = false;
    if (s[0] == '-') { neg = true; i = 1; if (i == s.size()) return false; }
    long long v = 0;
    for (; i < s.size(); ++i) {
        char c = s[i];
        if (c < '0' || c > '9') return false;
        int d = c - '0';
        v = v * 10 + d;
    }
    out = neg ? -v : v;
    return true;
}

// parse a line into tokens, respecting quoted strings (double quotes),
// where quoted strings remain as a single token including quotes.
static vector<string> tokenize(const string &line) {
    vector<string> tokens;
    const char *p = line.c_str();
    size_t n = line.size();
    size_t i = 0;
    while (i < n) {
        while (i < n && isspace((unsigned char)p[i])) i++;
        if (i >= n) break;
        if (p[i] == '"') {
            size_t j = i + 1;
            // find next unescaped quote (there are no escapes per spec)
            while (j < n && p[j] != '"') j++;
            if (j >= n) {
                // unmatched quote, take rest as token
                tokens.emplace_back(line.substr(i));
                break;
            } else {
                tokens.emplace_back(line.substr(i, j - i + 1));
                i = j + 1;
            }
        } else {
            size_t j = i;
            while (j < n && !isspace((unsigned char)p[j])) j++;
            tokens.emplace_back(line.substr(i, j - i));
            i = j;
        }
    }
    return tokens;
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int n;
    if (!(cin >> n)) return 0;
    string dummy;
    getline(cin, dummy); // consume rest of line

    // scopes: vector of unordered_map name -> Var, from outer to inner
    vector<unordered_map<string, Var>> scopes;
    scopes.emplace_back(); // global scope

    auto lookup = [&](const string &name) -> pair<int, Var*> {
        for (int i = (int)scopes.size() - 1; i >= 0; --i) {
            auto it = scopes[i].find(name);
            if (it != scopes[i].end()) return {i, &it->second};
        }
        return {-1, nullptr};
    };

    string line;
    for (int t = 0; t < n; ++t) {
        if (!std::getline(cin, line)) break;
        if (!line.empty() && line.back() == '\r') line.pop_back();
        auto tokens = tokenize(line);
        if (tokens.empty()) { cout << "Invalid operation\n"; continue; }
        const string &cmd = tokens[0];
        bool ok = true;

        if (cmd == "Indent") {
            if (tokens.size() != 1) { ok = false; }
            else scopes.emplace_back();
        } else if (cmd == "Dedent") {
            if (tokens.size() != 1) ok = false;
            else {
                if (scopes.size() <= 1) ok = false; // cannot pop global
                else scopes.pop_back();
            }
        } else if (cmd == "Declare") {
            if (tokens.size() != 4) { ok = false; }
            else {
                const string &type = tokens[1];
                const string &name = tokens[2];
                const string &val = tokens[3];
                // disallow redeclare in same scope
                if (scopes.back().count(name)) {
                    ok = false;
                } else if (type == "int") {
                    long long v;
                    if (!is_int_literal(val, v)) ok = false;
                    else {
                        Var var; var.is_int = true; var.val.is_int = true; var.val.iv = v; var.val.sv.clear();
                        scopes.back().emplace(name, std::move(var));
                    }
                } else if (type == "string") {
                    if (val.size() >= 2 && val.front() == '"' && val.back() == '"') {
                        Var var; var.is_int = false; var.val.is_int = false; var.val.iv = 0; var.val.sv = val.substr(1, val.size()-2);
                        scopes.back().emplace(name, std::move(var));
                    } else ok = false;
                } else {
                    ok = false;
                }
            }
        } else if (cmd == "Add") {
            if (tokens.size() != 4) ok = false;
            else {
                const string &res = tokens[1];
                const string &v1n = tokens[2];
                const string &v2n = tokens[3];
                auto r = lookup(res);
                auto a = lookup(v1n);
                auto b = lookup(v2n);
                if (!r.second || !a.second || !b.second) ok = false;
                else {
                    if (r.second->is_int != a.second->is_int || r.second->is_int != b.second->is_int) ok = false;
                    else if (r.second->is_int) {
                        // int addition
                        long long sum = a.second->val.iv + b.second->val.iv;
                        r.second->val.iv = sum;
                    } else {
                        // string concat
                        r.second->val.sv = a.second->val.sv + b.second->val.sv;
                    }
                }
            }
        } else if (cmd == "SelfAdd") {
            if (tokens.size() != 3) ok = false;
            else {
                const string &name = tokens[1];
                const string &val = tokens[2];
                auto it = lookup(name);
                if (!it.second) ok = false;
                else if (it.second->is_int) {
                    long long v;
                    if (!is_int_literal(val, v)) ok = false;
                    else it.second->val.iv += v;
                } else {
                    if (val.size() >= 2 && val.front() == '"' && val.back() == '"') {
                        it.second->val.sv += val.substr(1, val.size()-2);
                    } else ok = false;
                }
            }
        } else if (cmd == "Print") {
            if (tokens.size() != 2) ok = false;
            else {
                const string &name = tokens[1];
                auto it = lookup(name);
                if (!it.second) ok = false;
                else {
                    if (it.second->is_int) cout << name << ":" << it.second->val.iv << '\n';
                    else cout << name << ":" << it.second->val.sv << '\n';
                }
            }
        } else {
            ok = false;
        }

        if (!ok) cout << "Invalid operation\n";
    }
    return 0;
}
