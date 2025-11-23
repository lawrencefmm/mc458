#include <iostream>
#include <map>
#include <vector>
#include <tuple>
#include <memory>
#include <cmath>
#include <algorithm>

const long long MOD = 1000000;

class SparseMatrix {
public:
    struct Core {
        std::map<std::pair<int,int>, long long> data;
    };

    int n;
    std::shared_ptr<Core> core;
    bool transposed;

    explicit SparseMatrix(int n_ = 0)
        : n(n_), core(std::make_shared<Core>()), transposed(false) {}

    SparseMatrix(int n_, const std::vector<std::tuple<int,int,long long>>& elems)
        : n(n_), core(std::make_shared<Core>()), transposed(false) {
        for (auto &t : elems) {
            int i, j; long long v;
            std::tie(i,j,v) = t;
            set(i,j,v);
        }
    }

    long long get(int i, int j) const {
        int bi = transposed ? j : i;
        int bj = transposed ? i : j;
        auto it = core->data.find({bi,bj});
        if (it == core->data.end()) return 0LL;
        return it->second;
    }

    void set(int i, int j, long long v) {
        int bi = transposed ? j : i;
        int bj = transposed ? i : j;
        auto key = std::make_pair(bi,bj);
        if (v == 0LL) {
            core->data.erase(key);
        } else {
            core->data[key] = v;
        }
    }

    void addValue(int i, int j, long long delta) {
        int bi = transposed ? j : i;
        int bj = transposed ? i : j;
        auto key = std::make_pair(bi,bj);
        auto it = core->data.find(key);
        if (it == core->data.end()) {
            if (delta != 0LL)
                core->data[key] = delta;
        } else {
            long long nv = it->second + delta;
            if (nv == 0LL) core->data.erase(it);
            else it->second = nv;
        }
    }

    void toggleTranspose() {
        transposed = !transposed;
    }

    SparseMatrix materialize() const {
        if (!transposed) return *this;
        SparseMatrix M(n);
        for (const auto &kv : core->data) {
            int bi = kv.first.first;
            int bj = kv.first.second;
            long long v = kv.second;
            M.set(bj, bi, v);
        }
        return M;
    }

    template<class Func>
    void forEachNonZero(Func f) const {
        if (!transposed) {
            for (const auto &kv : core->data) {
                f(kv.first.first, kv.first.second, kv.second);
            }
        } else {
            for (const auto &kv : core->data) {
                int bi = kv.first.first;
                int bj = kv.first.second;
                f(bj, bi, kv.second);
            }
        }
    }

    SparseMatrix add(const SparseMatrix &B) const {
        if (n != B.n) throw std::runtime_error("Dimension mismatch in add");
        SparseMatrix C(n);
        this->forEachNonZero([&](int i, int j, long long v){
            C.addValue(i,j,v % MOD);
        });
        B.forEachNonZero([&](int i, int j, long long v){
            C.addValue(i,j,v % MOD);
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
        C.transposed = false;
        if (alpha == 0LL) return C;
        
        this->forEachNonZero([&](int i, int j, long long v){
            long long nv = (v % MOD) * (alpha % MOD);
            nv %= MOD;
            if (nv < 0) nv += MOD;
            if (nv != 0LL) C.core->data[{i,j}] = nv;
        });
        return C;
    }

    SparseMatrix multiply(const SparseMatrix &B) const {
        if (n != B.n) throw std::runtime_error("Dimension mismatch in multiply");
        
        SparseMatrix A_mat = this->materialize();
        SparseMatrix B_mat = B.materialize();
        
        SparseMatrix C(n);

        for (const auto &akv : A_mat.core->data) {
            int i = akv.first.first;
            int k = akv.first.second;
            long long a_val = akv.second;

            auto it = B_mat.core->data.lower_bound({k, -1});

            while (it != B_mat.core->data.end() && it->first.first == k) {
                int j = it->first.second;
                long long b_val = it->second;
                
                long long prod = (a_val % MOD) * (b_val % MOD);
                prod %= MOD;
                if (prod < 0) prod += MOD;
                C.addValue(i, j, prod);
                it++;
            }
        }
        
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
            volatile long long res;
            if (m == 1) res = A.get(i,j);
            else        res = B.get(i,j);
            (void)res;
        }
        else if (op == 2) { // set
            int m, i, j;
            long long v;
            std::cin >> m >> i >> j >> v;
            if (m == 1) A.set(i,j,v);
            else        B.set(i,j,v);
        }
        else if (op == 3) { // transpose
            int m;
            std::cin >> m;
            if (m == 1) A.toggleTranspose();
            else        B.toggleTranspose();
        }
        else if (op == 4) { // soma
            SparseMatrix C = A.add(B);
        }
        else if (op == 5) { // scale
            int m; long long alpha;
            std::cin >> m >> alpha;
            if (m == 1) { SparseMatrix C = A.scale(alpha); }
            else        { SparseMatrix C = B.scale(alpha); }
        }
        else if (op == 6) { // mult
            SparseMatrix C = A.multiply(B);
        }
    }
    return 0;
}