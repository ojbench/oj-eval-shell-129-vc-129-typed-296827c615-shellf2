#include <bits/stdc++.h>
using namespace std;

struct FastInput {
    static constexpr size_t kBufSize = 1 << 20;
    char buf[kBufSize];
    size_t idx = 0, size = 0;

    inline char get() {
        if (idx >= size) {
            size = fread(buf, 1, kBufSize, stdin);
            idx = 0;
            if (size == 0) return 0;
        }
        return buf[idx++];
    }

    inline void skip_spaces() {
        char c;
        while ((c = get())) {
            if (!isspace(static_cast<unsigned char>(c))) {
                --idx;
                return;
            }
        }
    }

    bool read_int(int &out) {
        skip_spaces();
        char c = get();
        if (!c) return false;
        bool neg = false;
        if (c == '-') {
            neg = true;
            c = get();
        }
        long long v = 0;
        while (c && !isspace(static_cast<unsigned char>(c))) {
            if (c < '0' || c > '9') return false;
            v = v * 10 + (c - '0');
            c = get();
        }
        out = neg ? -static_cast<int>(v) : static_cast<int>(v);
        return true;
    }

    bool read_token(string_view &out) {
        skip_spaces();
        char c = get();
        if (!c) return false;
        if (c == '"') {
            const char *start = buf + idx;
            while ((c = get())) {
                if (c == '"') {
                    out = string_view(start, (buf + idx - 1) - start);
                    return true;
                }
            }
            return false;
        }
        const char *start = buf + idx - 1;
        while ((c = get()) && !isspace(static_cast<unsigned char>(c))) {}
        out = string_view(start, (buf + idx - (c ? 1 : 0)) - start);
        if (c) --idx;
        return true;
    }
};

struct FastOutput {
    static constexpr size_t kBufSize = 1 << 20;
    char buf[kBufSize];
    size_t idx = 0;

    ~FastOutput() { flush(); }

    inline void flush() {
        if (idx) fwrite(buf, 1, idx, stdout);
        idx = 0;
    }

    inline void put(char c) {
        if (idx >= kBufSize) flush();
        buf[idx++] = c;
    }

    inline void write(string_view s) {
        for (char c : s) put(c);
    }

    inline void write_ll(long long x) {
        if (x == 0) {
            put('0');
            return;
        }
        if (x < 0) {
            put('-');
            x = -x;
        }
        char tmp[32];
        int n = 0;
        while (x > 0) {
            tmp[n++] = char('0' + x % 10);
            x /= 10;
        }
        while (n--) put(tmp[n]);
    }
};

struct Hash {
    using is_transparent = void;
    size_t operator()(string_view s) const noexcept { return std::hash<string_view>{}(s); }
    size_t operator()(const string &s) const noexcept { return std::hash<string_view>{}(s); }
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

static inline bool parse_int_sv(string_view sv, long long &out) {
    if (sv.empty()) return false;
    size_t i = 0;
    bool neg = false;
    if (sv[0] == '-') {
        neg = true;
        i = 1;
        if (i == sv.size()) return false;
    }
    long long v = 0;
    for (; i < sv.size(); ++i) {
        char c = sv[i];
        if (c < '0' || c > '9') return false;
        v = v * 10 + (c - '0');
    }
    out = neg ? -v : v;
    return true;
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    FastInput in;
    FastOutput out;

    int n;
    if (!in.read_int(n)) return 0;

    unordered_map<string, vector<Value>, Hash, Eq> vars;
    vars.reserve(1 << 16);

    vector<vector<string>> scope_names(1);
    vector<unordered_set<string, Hash, Eq>> scope_used(1);
    scope_names[0].reserve(128);
    scope_used[0].reserve(128);
    int depth = 0;

    auto find_var = [&](string_view name) -> Value* {
        auto it = vars.find(name);
        if (it == vars.end() || it->second.empty()) return nullptr;
        return &it->second.back();
    };

    for (int i = 0; i < n; ++i) {
        string_view cmd;
        if (!in.read_token(cmd)) break;
        bool ok = true;

        if (cmd == "Indent") {
            ++depth;
            if ((int)scope_names.size() <= depth) {
                scope_names.emplace_back();
                scope_used.emplace_back();
                scope_names.back().reserve(128);
                scope_used.back().reserve(128);
            } else {
                scope_names[depth].clear();
                scope_used[depth].clear();
            }
        } else if (cmd == "Dedent") {
            if (depth == 0) ok = false;
            else {
                for (const string &name : scope_names[depth]) {
                    auto it = vars.find(name);
                    if (it != vars.end() && !it->second.empty()) {
                        it->second.pop_back();
                        if (it->second.empty()) vars.erase(it);
                    }
                }
                scope_names[depth].clear();
                scope_used[depth].clear();
                --depth;
            }
        } else if (cmd == "Declare") {
            string_view type, name, value;
            if (!in.read_token(type) || !in.read_token(name) || !in.read_token(value)) ok = false;
            else {
                auto &used = scope_used[depth];
                if (used.find(name) != used.end()) ok = false;
                else {
                    auto key = string(name);
                    used.emplace(key);
                    scope_names[depth].emplace_back(key);
                    auto &stk = vars[key];
                    Value v;
                    if (type == "int") {
                        long long x;
                        if (!parse_int_sv(value, x)) ok = false;
                        else {
                            v.is_int = true;
                            v.iv = x;
                            stk.push_back(std::move(v));
                        }
                    } else if (type == "string") {
                        v.is_int = false;
                        v.sv = string(value);
                        stk.push_back(std::move(v));
                    } else ok = false;
                    if (!ok) {
                        used.erase(key);
                        scope_names[depth].pop_back();
                        if (!stk.empty()) {
                            stk.pop_back();
                            if (stk.empty()) vars.erase(key);
                        }
                    }
                }
            }
        } else if (cmd == "Add") {
            string_view res, a, b;
            if (!in.read_token(res) || !in.read_token(a) || !in.read_token(b)) ok = false;
            else {
                Value *rv = find_var(res);
                Value *av = find_var(a);
                Value *bv = find_var(b);
                if (!rv || !av || !bv) ok = false;
                else if (rv->is_int != av->is_int || rv->is_int != bv->is_int) ok = false;
                else if (rv->is_int) rv->iv = av->iv + bv->iv;
                else rv->sv = av->sv + bv->sv;
            }
        } else if (cmd == "SelfAdd") {
            string_view name, value;
            if (!in.read_token(name) || !in.read_token(value)) ok = false;
            else {
                Value *v = find_var(name);
                if (!v) ok = false;
                else if (v->is_int) {
                    long long x;
                    if (!parse_int_sv(value, x)) ok = false;
                    else v->iv += x;
                } else {
                    v->sv.append(value.data(), value.size());
                }
            }
        } else if (cmd == "Print") {
            string_view name;
            if (!in.read_token(name)) ok = false;
            else {
                Value *v = find_var(name);
                if (!v) ok = false;
                else {
                    out.write(name);
                    out.put(':');
                    if (v->is_int) out.write_ll(v->iv);
                    else out.write(v->sv);
                    out.put('\n');
                }
            }
        } else ok = false;

        if (!ok) out.write("Invalid operation\n");
    }

    return 0;
}
