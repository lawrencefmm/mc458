/*
 * gerador.cpp
 * Compilar com: g++ -o gerador gerador.cpp -O3 -std=c++17
 * * Uso: ./gerador [N] [k]
 * [N]: dimensão da matriz (N x N)
 * [k]: número de elementos não nulos
 *
 * Saída: Imprime k linhas no formato "linha coluna valor" para stdout
 */

#include <iostream>
#include <string>
#include <vector>
#include <random>       // Para geração de números aleatórios
#include <unordered_set> // Para garantir posições (i, j) únicas
#include <functional>   // Para std::hash

// Estrutura para representar uma posição (linha, coluna)
struct Position {
    int row;
    int col;

    // Operador '==' para o unordered_set
    bool operator==(const Position& other) const {
        return row == other.row && col == other.col;
    }
};

// Hash customizado para que Position possa ser usada no unordered_set
namespace std {
    template <>
    struct hash<Position> {
        size_t operator()(const Position& p) const {
            // Combina o hash da linha e da coluna
            size_t h1 = hash<int>{}(p.row);
            size_t h2 = hash<int>{}(p.col);
            return h1 ^ (h2 << 1); // Combinação simples de hash
        }
    };
}

int main(int argc, char* argv[]) {
    if (argc != 3) {
        std::cerr << "Uso: ./gerador [N] [k]" << std::endl;
        return 1;
    }

    std::ios_base::sync_with_stdio(false);
    std::cin.tie(NULL);

    long long N = std::stoll(argv[1]);
    long long k = std::stoll(argv[2]);

    printf("%lld %lld\n", N, k);

    if (k > N * N) {
        std::cerr << "Erro: k não pode ser maior que N*N." << std::endl;
        k = N * N;
    }

    // Geradores de números aleatórios
    std::mt19937 gen(std::random_device{}()); // Motor de geração
    std::uniform_int_distribution<long long> dist_pos(0, N - 1);
    std::uniform_int_distribution<int> dist_val(1, 100);

    std::unordered_set<Position> generated_positions;
    generated_positions.reserve(k); 

    for (long long i = 0; i < k; ++i) {
        Position pos;
        do {
            pos = { (int)dist_pos(gen), (int)dist_pos(gen) };
        } while (!generated_positions.insert(pos).second); 

        std::cout << pos.row << " " << pos.col << " " << dist_val(gen) << "\n";
    }

    return 0;
}