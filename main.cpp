#include <bits/stdc++.h>
using namespace std;

struct Hash {
    using is_transparent = void;
    size_t operator()(string_view s) const noexcept { return hash<string_view>{}(s); }
    size_t operator()(const string &s) const noexcept { return hash<string_view>{}(s); }
};

struct Eq {
    using is_transparent = void;
    bool operator()(string_view a, string_view b) const noexcept { return a == b; }
    bool operator()(const string &a, const string &b) const noexcept { return a == b; }
    bool operator()(const string &a, string_view b) const noexcept { return string_view(a) == b; }
    bool operator()(string_view a, const string &b) const noexcept { return a == string_view(b); }
};

struct Value {
    bool is_int = true;
    long long iv = 0;
    string sv;
};

static inline void skip_spaces(const string &line, size_t &i) {
    while (i < line.size() && line[i] == ' ') ++i;
}

static inline bool read_word(const string &line, size_t &i, string_view &out) {
    skip_spaces(line, i);
    if (i >= line.size()) return false;
    size_t j = i;
    while (j < line.size() && line[j] != ' ') ++j;
    out = string_view(line.data() + i, j - i);
    i = j;
    return true;
}

static inline bool read_quoted(const string &line, size_t &i, string &out) {
    skip_spaces(line, i);
    if (i >= line.size() || line[i] != '"') return false;
    size_t j = i + 1;
    while (j < line.size() && line[j] != '"') ++j;
    if (j >= line.size()) return false;
    out.assign(line.data() + i + 1, j - i - 1);
    i = j + 1;
    return true;
}

static inline bool parse_int(string_view s, long long &out) {
    if (s.empty()) return false;
    size_t i = 0;
    bool neg = false;
    if (s[0] == '-') {
        neg = true;
        i = 1;
        if (i == s.size()) return false;
    }
    long long v = 0;
    for (; i < s.size(); ++i) {
        char c = s[i];
        if (c < '0' || c > '9') return false;
        v = v * 10 + (c - '0');
    }
    out = neg ? -v : v;
    return true;
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int n;
    if (!(cin >> n)) return 0;
    string line;
    getline(cin, line);

    unordered_map<string, vector<Value>, Hash, Eq> vars;
    vars.reserve(1 << 16);
    vector<vector<string>> scope_names(1);
    vector<unordered_set<string, Hash, Eq>> scope_used(1);
    int depth = 0;

    auto lookup = [&](string_view name) -> Value* {
        auto it = vars.find(name);
        if (it == vars.end() || it->second.empty()) return nullptr;
        return &it->second.back();
    };

    for (int tc = 0; tc < n; ++tc) {
        getline(cin, line);
        if (!line.empty() && line.back() == '\r') line.pop_back();
        size_t i = 0;
        string_view cmd;
        bool ok = true;
        if (!read_word(line, i, cmd)) {
            cout << "Invalid operation\n";
            continue;
        }

        if (cmd == "Indent") {
            skip_spaces(line, i);
            if (i != line.size()) ok = false;
            else {
                ++depth;
                if ((int)scope_names.size() <= depth) {
                    scope_names.emplace_back();
                    scope_used.emplace_back();
                }
            }
        } else if (cmd == "Dedent") {
            skip_spaces(line, i);
            if (i != line.size() || depth == 0) ok = false;
            else {
                for (const string &name : scope_names[depth]) {
                    auto it = vars.find(name);
                    if (it != vars.end()) {
                        it->second.pop_back();
                        if (it->second.empty()) vars.erase(it);
                    }
                }
                scope_names[depth].clear();
                scope_used[depth].clear();
                --depth;
            }
        } else if (cmd == "Declare") {
            string_view type, name;
            if (!read_word(line, i, type) || !read_word(line, i, name)) ok = false;
            else if (scope_used[depth].find(name) != scope_used[depth].end()) ok = false;
            else {
                Value v;
                if (type == "int") {
                    string_view tok;
                    if (!read_word(line, i, tok)) ok = false;
                    else {
                        long long x;
                        if (!parse_int(tok, x)) ok = false;
                        else {
                            v.is_int = true;
                            v.iv = x;
                        }
                    }
                } else if (type == "string") {
                    if (!read_quoted(line, i, v.sv)) ok = false;
                    else v.is_int = false;
                } else ok = false;

                skip_spaces(line, i);
                if (ok && i != line.size()) ok = false;
                if (ok) {
                    string key(name);
                    scope_used[depth].insert(key);
                    scope_names[depth].push_back(key);
                    vars[key].push_back(std::move(v));
                }
            }
        } else if (cmd == "Add") {
            string_view res, a, b;
            if (!read_word(line, i, res) || !read_word(line, i, a) || !read_word(line, i, b)) ok = false;
            else {
                skip_spaces(line, i);
                if (i != line.size()) ok = false;
                else {
                    Value *rv = lookup(res);
                    Value *av = lookup(a);
                    Value *bv = lookup(b);
                    if (!rv || !av || !bv) ok = false;
                    else if (rv->is_int != av->is_int || rv->is_int != bv->is_int) ok = false;
                    else if (rv->is_int) rv->iv = av->iv + bv->iv;
                    else rv->sv = av->sv + bv->sv;
                }
            }
        } else if (cmd == "SelfAdd") {
            string_view name;
            if (!read_word(line, i, name)) ok = false;
            else {
                Value *v = lookup(name);
                if (!v) ok = false;
                else if (v->is_int) {
                    string_view tok;
                    if (!read_word(line, i, tok)) ok = false;
                    else {
                        long long x;
                        if (!parse_int(tok, x)) ok = false;
                        else v->iv += x;
                    }
                    skip_spaces(line, i);
                    if (ok && i != line.size()) ok = false;
                } else {
                    if (!read_quoted(line, i, v->sv)) ok = false;
                    skip_spaces(line, i);
                    if (ok && i != line.size()) ok = false;
                }
            }
        } else if (cmd == "Print") {
            string_view name;
            if (!read_word(line, i, name)) ok = false;
            else {
                skip_spaces(line, i);
                if (i != line.size()) ok = false;
                else {
                    Value *v = lookup(name);
                    if (!v) ok = false;
                    else if (v->is_int) cout << name << ':' << v->iv << '\n';
                    else cout << name << ':' << v->sv << '\n';
                }
            }
        } else {
            ok = false;
        }

        if (!ok) cout << "Invalid operation\n";
    }
    return 0;
}
