#include <iostream>
#include <vector>
#include <tuple>
#include <algorithm>

// Algoritmo de referência: Vector de pares (Coordinate List).
// Prints removidos para benchmark.

const long long MOD = 1000000;

class DenseMatrix {
public:
    using Element = std::pair<std::pair<int, int>, long long>;
    
    int n;
    bool is_transposed;
    std::vector<Element> elements;

    DenseMatrix(int n_ = 0) : n(n_), is_transposed(false) {}

    DenseMatrix(int n_, const std::vector<std::tuple<int,int,long long>>& elems)
        : n(n_), is_transposed(false) {
        for (auto &t : elems) {
            int i, j; long long v;
            std::tie(i,j,v) = t;
            if (v != 0) {
                elements.push_back({{i, j}, v});
            }
        }
    }

    long long get(int i, int j) const {
        int target_r = is_transposed ? j : i;
        int target_c = is_transposed ? i : j;

        for (const auto& el : elements) {
            if (el.first.first == target_r && el.first.second == target_c) {
                return el.second;
            }
        }
        return 0;
    }

    void set(int i, int j, long long v) {
        int target_r = is_transposed ? j : i;
        int target_c = is_transposed ? i : j;

        for (auto it = elements.begin(); it != elements.end(); ++it) {
            if (it->first.first == target_r && it->first.second == target_c) {
                if (v == 0) {
                    elements.erase(it);
                } else {
                    it->second = v;
                }
                return;
            }
        }
        if (v != 0) {
            elements.push_back({{target_r, target_c}, v});
        }
    }

    void toggleTranspose() {
        is_transposed = !is_transposed;
    }

    static void addToResult(std::vector<Element>& res, int r, int c, long long val) {
        if (val == 0) return;
        for (auto& el : res) {
            if (el.first.first == r && el.first.second == c) {
                el.second = (el.second + val) % MOD;
                if (el.second < 0) el.second += MOD;
                return;
            }
        }
        res.push_back({{r, c}, val});
    }

    DenseMatrix add(const DenseMatrix &B) const {
        DenseMatrix C(n);
        for (const auto& el : elements) {
            int r = is_transposed ? el.first.second : el.first.first;
            int c = is_transposed ? el.first.first : el.first.second;
            addToResult(C.elements, r, c, el.second);
        }
        for (const auto& el : B.elements) {
            int r = B.is_transposed ? el.first.second : el.first.first;
            int c = B.is_transposed ? el.first.first : el.first.second;
            addToResult(C.elements, r, c, el.second);
        }
        return C;
    }

    DenseMatrix scale(long long alpha) const {
        DenseMatrix C(n);
        if (alpha == 0) return C;
        for (const auto& el : elements) {
            int r = is_transposed ? el.first.second : el.first.first;
            int c = is_transposed ? el.first.first : el.first.second;
            long long val = (el.second % MOD) * (alpha % MOD);
            val %= MOD;
            if (val < 0) val += MOD;
            if (val != 0) {
                C.elements.push_back({{r, c}, val});
            }
        }
        return C;
    }

    DenseMatrix multiply(const DenseMatrix &B) const {
        DenseMatrix C(n);
        for (const auto& elA : elements) {
            int rA = is_transposed ? elA.first.second : elA.first.first;
            int cA = is_transposed ? elA.first.first : elA.first.second;
            long long valA = elA.second;

            for (const auto& elB : B.elements) {
                int rB = B.is_transposed ? elB.first.second : elB.first.first;
                int cB = B.is_transposed ? elB.first.first : elB.first.second;
                long long valB = elB.second;

                if (cA == rB) {
                    long long prod = (valA % MOD) * (valB % MOD);
                    prod %= MOD;
                    addToResult(C.elements, rA, cB, prod);
                }
            }
        }
        for (auto& el : C.elements) {
            if (el.second < 0) el.second += MOD;
        }
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
    DenseMatrix A(N1, elems1);

    int k2, N2;
    std::cin >> k2 >> N2;
    std::vector<std::tuple<int,int,long long>> elems2;
    elems2.reserve(k2);
    for (int t = 0; t < k2; ++t) {
        int i, j; long long v;
        std::cin >> i >> j >> v;
        elems2.emplace_back(i, j, v);
    }
    DenseMatrix B(N2, elems2);

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
            DenseMatrix C = A.add(B);
        }
        else if (op == 5) { // scale
            int m; long long alpha;
            std::cin >> m >> alpha;
            if (m == 1) { DenseMatrix C = A.scale(alpha); }
            else        { DenseMatrix C = B.scale(alpha); }
        }
        else if (op == 6) { // mult
            DenseMatrix C = A.multiply(B);
        }
    }
    return 0;
}