#include <iostream>
#include <unordered_map>
#include <vector>
#include <tuple>

const long long MOD = 1000000;

class SparseMatrix {
public:
    using Key = std::pair<int,int>;

    struct KeyHash {
        size_t operator()(const Key& k) const noexcept {
            return std::hash<int>()(k.first * 1315423911u) ^
                   std::hash<int>()(k.second);
        }
    };

    std::unordered_map<Key, long long, KeyHash> data;
    int n;
    bool transposed;

    SparseMatrix(int n_ = 0)
        : n(n_), transposed(false)
    {}

    SparseMatrix(int n_, const std::vector<std::tuple<int,int,long long>>& elems)
        : n(n_), transposed(false)
    {
        for (auto& t : elems) {
            int i, j;
            long long v;
            std::tie(i, j, v) = t;
            set(i, j, v);
        }
    }

    inline Key mapIndex(int i, int j) const {
        return transposed ? Key(j, i) : Key(i, j);
    }

    long long get(int i, int j) const {
        Key k = mapIndex(i, j);
        auto it = data.find(k);
        return (it == data.end() ? 0 : it->second);
    }

    void set(int i, int j, long long v) {
        Key k = mapIndex(i, j);
        if (v == 0) {
            data.erase(k);
        } else {
            data[k] = v;
        }
    }

    void addValue(int i, int j, long long delta) {
        Key k = mapIndex(i, j);
        auto it = data.find(k);

        if (it == data.end()) {
            if (delta != 0) data[k] = delta;
            return;
        }

        long long novo = it->second + delta;
        if (novo == 0) data.erase(it);
        else it->second = novo;
    }

    void transpose() {
        transposed = !transposed;
    }

    SparseMatrix cria_transposta() const {
        if (!transposed) return *this;

        SparseMatrix M(n);
        for (auto& kv : data) {
            int i = kv.first.first;
            int j = kv.first.second;
            long long v = kv.second;
            M.set(j, i, v);
        }
        return M;
    }

    template<typename F>
    void forEachNonZero(F f) const {
        if (!transposed) {
            for (auto& kv : data)
                f(kv.first.first, kv.first.second, kv.second);
        } else {
            for (auto& kv : data)
                f(kv.first.second, kv.first.first, kv.second);
        }
    }

    SparseMatrix add(const SparseMatrix& B) const {
        if (n != B.n) throw std::runtime_error("Dimension mismatch");

        SparseMatrix C(n);

        this->forEachNonZero([&](int i, int j, long long v){
            C.addValue(i, j, v % MOD);
        });

        B.forEachNonZero([&](int i, int j, long long v){
            C.addValue(i, j, v % MOD);
        });

        C.forEachNonZero([&](int i, int j, long long v){
            long long val = v % MOD;
            if (val < 0) val += MOD;
            C.set(i, j, val);
        });

        return C;
    }

    SparseMatrix scale(long long alpha) const {
        SparseMatrix C(n);
        if (alpha == 0) return C;

        this->forEachNonZero([&](int i, int j, long long v){
            long long nv = (v % MOD) * (alpha % MOD);
            nv %= MOD;
            if (nv < 0) nv += MOD;
            C.set(i, j, nv);
        });

        return C;
    }

    SparseMatrix multiply(const SparseMatrix& B) const {
        if (n != B.n) throw std::runtime_error("Dimension mismatch");

        SparseMatrix A0 = this->cria_transposta();
        std::vector<std::vector<std::pair<int,long long>>> rowB(n);
        
        B.forEachNonZero([&](int r, int c, long long val){
            rowB[r].push_back({c, val});
        });

        SparseMatrix C(n);
        
        this->forEachNonZero([&](int i, int k, long long a_val){
             for (auto& par : rowB[k]) {
                int j = par.first;
                long long b_val = par.second;
                long long prod = (a_val % MOD) * (b_val % MOD);
                prod %= MOD;
                if (prod < 0) prod += MOD;
                C.addValue(i, j, prod);
            }
        });

        C.forEachNonZero([&](int i, int j, long long v){
            long long val = v % MOD;
            if (val < 0) val += MOD;
            C.set(i, j, val);
        });

        return C;
    }
};

int main() {
    std::ios::sync_with_stdio(false);
    std::cin.tie(nullptr);

    int k1, N1;
    if (!(std::cin >> k1 >> N1)) return 0;
    std::vector<std::tuple<int,int,long long>> elems1;
    elems1.reserve(k1);
    for (int t = 0; t < k1; ++t) {
        int i, j; long long v;
        std::cin >> i >> j >> v;
        elems1.emplace_back(i, j, v);
    }
    SparseMatrix A(N1, elems1);

    int k2, N2;
    std::cin >> k2 >> N2;
    std::vector<std::tuple<int,int,long long>> elems2;
    elems2.reserve(k2);
    for (int t = 0; t < k2; ++t) {
        int i, j; long long v;
        std::cin >> i >> j >> v;
        elems2.emplace_back(i, j, v);
    }
    SparseMatrix B(N2, elems2);

    if (N1 != N2) return 1;

    int Q;
    if (!(std::cin >> Q)) return 0;

    while (Q--) {
        int op;
        if (!(std::cin >> op)) break;

        if (op == 1) { // consulta
            int m, i, j;
            std::cin >> m >> i >> j;
            // volatile garante que o compilador execute o get mesmo sem printar
            volatile long long res;
            if (m == 1) res = A.get(i,j);
            else        res = B.get(i,j);
            (void)res; // Silencia warning de variável não usada
        }
        else if (op == 2) { // set
            int m, i, j;
            long long v;
            std::cin >> m >> i >> j >> v;
            if (m == 1) A.set(i,j,v);
            else        B.set(i,j,v);
        }
        else if (op == 3) { // transpor
            int m;
            std::cin >> m;
            if (m == 1) A.transpose();
            else        B.transpose();
        }
        else if (op == 4) { // soma
            SparseMatrix C = A.add(B);
            // Apenas realiza a operação, C é destruído ao fim do escopo
        }
        else if (op == 5) { // multiplicar por escalar
            int m; long long alpha;
            std::cin >> m >> alpha;
            if (m == 1) { SparseMatrix C = A.scale(alpha); }
            else        { SparseMatrix C = B.scale(alpha); }
        }
        else if (op == 6) { // multiplicação
            SparseMatrix C = A.multiply(B);
        }
    }
    return 0;
}