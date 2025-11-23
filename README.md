# Projeto: Estruturas e Operações em Matrizes Esparsas

Este repositório contém a implementação e avaliação experimental de três abordagens para manipular matrizes esparsas quadradas (dimensão `N x N`) com conjunto de operações definidas (consulta, atualização, transposição, soma, multiplicação escalar e multiplicação matricial). O foco principal é comparar representações e estratégias de acesso/custos, bem como observar escalabilidade em relação a `N` e ao número de elementos não nulos `k`.

## Sumário

1. Visão Geral
2. Arquitetura e Arquivos
3. Formato de Entrada / Protocolo de Operações
4. Algoritmos Implementados
5. Complexidade Assintótica (Resumo)
6. Regras de Modularidade (MOD = 1_000_000)
7. Geração de Casos de Teste
8. Benchmarking e Geração de Gráficos
9. Como Compilar e Executar (Linux / Windows)
10. Reproduzindo os Experimentos
11. Estrutura de Diretórios
12. Próximos Passos / Extensões

---

## 1. Visão Geral

O projeto lê duas matrizes esparsas A e B de dimensão `N x N` (mesmo `N`), executa uma sequência de operações descritas por códigos inteiros e, para fins de benchmark, realiza essas operações sem imprimir resultados (evitando custo de I/O que poluiria as medições). As operações são cuidadosamente implementadas em três variantes para comparar desempenho:

- `algoritmo1.cpp`: usa `std::unordered_map` com hash customizado.
- `algoritmo2.cpp`: usa `std::map` (árvore balanceada) e compartilha estado via `shared_ptr` interno; transposição é uma view lógico‑O(1).
- `algoritmo_denso.cpp`: implementação de referência simples estilo "coordinate list" (vetor de pares) – utilizada apenas para pequenos `N` como baseline.

Um script Python (`main.py`) automatiza:
1. Compilação dos executáveis.
2. Geração (se necessário) dos arquivos de testes (`gerador_testes.py`).
3. Execução repetida dos testes para várias combinações de `(N, k)`.
4. Registro dos tempos médios em CSV e criação de gráficos comparativos.

## 2. Arquitetura e Arquivos

| Arquivo | Propósito |
|---------|-----------|
| `src/algoritmo1.cpp` | Estrutura esparsa baseada em `unordered_map` (hash). |
| `src/algoritmo2.cpp` | Estrutura esparsa baseada em `map` (ordenação + busca logarítmica). |
| `src/algoritmo_denso.cpp` | Implementação simples para referência (lista de coordenadas). |
| `src/gerador.cpp` | (Se presente) utilitário complementar de geração. |
| `gerador_testes.py` | Geração determinística de casos de teste para pares `(N, k)`. |
| `main.py` | Orquestra benchmarks e gera gráficos. |
| `tests/` | Diretório onde são armazenados os arquivos de entrada gerados. |
| `graficos/` | Saída dos gráficos de desempenho (um subdiretório por tipo de operação). |
| `requirements.txt` | Dependências Python para análise/gráficos. |

## 3. Formato de Entrada / Protocolo de Operações

Entrada padrão esperada por cada executável C++:

```
k1 N
i1 j1 v1
... (k1 linhas)
k2 N
i'1 j'1 v'1
... (k2 linhas)
Q
<Q linhas de operações>
```

Restrições:
- Matrizes devem ter a mesma dimensão `N`.
- Índices `i, j` em `[0, N-1]`.
- Valores inteiros; internamente aplica‑se aritmética módulo `1_000_000` nas operações de combinação.

### Códigos de Operação
| Código | Descrição | Sintaxe | Observação |
|--------|-----------|---------|------------|
| 1 | Consulta elemento | `1 m i j` | `m=1` usa A, `m=2` usa B; retorna valor lógico (no benchmark ignorado). |
| 2 | Set / atribuição | `2 m i j v` | Define elemento; remove se `v=0`. |
| 3 | Transpor view | `3 m` | Alterna flag interna de transposição (não realoca). |
| 4 | Soma | `4` | Calcula `A + B (mod MOD)` internamente. |
| 5 | Escala | `5 m alpha` | Cria versão escalada da matriz escolhida. |
| 6 | Multiplicação | `6` | Calcula `A * B (mod MOD)`.

No código atual para benchmarking, os resultados das operações (matrizes resultantes ou valores) não são impressos — apenas construídos em memória. Para uso funcional (ex.: depuração) seria necessário adicionar prints ou funções de exportação.

## 4. Algoritmos Implementados

### 4.1 `algoritmo1` (Hash + View Transposta)
- Estrutura: `unordered_map<pair<int,int>, long long>` com hash custom; transposição representada por booleano `transposed` que troca significado de `(i,j)`.
- Vantagens: acesso e atualização O(1) médio; iteração sobre elementos não zero simples.
- Limitação: ordem de iteração não determinística; custo elevado se hash colidir muito.

### 4.2 `algoritmo2` (Árvore Balanceada + Compartilhamento)
- Estrutura: `std::map<pair<int,int>, long long>` mantendo ordenação por linha/coluna.
- Transposição: flag booleana reinterpretando índices; `materialize()` cria versão física se necessário.
- Busca por intervalo em multiplicação usa `lower_bound` para percorrer todos elementos de uma linha específica.
- Vantagens: iteração ordenada e busca de faixas; pior caso mais previsível.
- Limitação: custo logarítmico em operações pontuais (get/set) vs. hash.

### 4.3 `algoritmo_denso` (Baseline Coordinate List)
- Armazena vetor de pares `( (i,j), v )` sem índice auxiliar.
- `get/set` fazem busca linear (O(k)).
- Usado apenas em tamanhos pequenos (`N=100`) como referência de custo.

## 5. Complexidade Assintótica (Resumo)

| Operação | algoritmo1 (`unordered_map`) | algoritmo2 (`map`) | denso (lista) |
|----------|------------------------------|--------------------|---------------|
| get/set  | O(1) médio / O(k) pior | O(log k) | O(k) |
| transpose (toggle) | O(1) | O(1) | O(1) |
| soma | O(nnz(A)+nnz(B)) | O(nnz(A)+nnz(B)) | O(k_A + k_B) + merges lineares |
| escala | O(nnz) | O(nnz) | O(k) |
| multiplicação | O( Σ_{a(i,k)≠0} deg_B(k) ) | Mesmo, com busca ordenada (menor overhead) | O(k_A * k_B) worst (verificação de todas combinações) |

Onde `nnz` = número de elementos não nulos; `deg_B(k)` = quantidade de elementos de B na linha (ou coluna) que participa do produto.

## 6. Regras de Modularidade

Todas as combinações (soma, escala, produto) aplicam `MOD = 1_000_000`. Valores negativos após redução recebem ajuste `if (val < 0) val += MOD;`. Elementos cujo resultado é `0` são removidos da estrutura para manter esparsidade.

## 7. Geração de Casos de Teste

O script `gerador_testes.py` gera diretórios `tests/N_<N>_K_<k>/` contendo arquivos:
- `teste_insercao.txt` (somente carga inicial; `Q=0`).
- `teste_consulta.txt`, `teste_set.txt`, `teste_transpose.txt`, `teste_soma.txt`, `teste_multiplicacao.txt` com número de queries ajustado conforme custo esperado de cada operação.

Política de geração:
- Usa semente determinística baseada em `(N, K)` para reprodutibilidade.
- Evita colisões de posições; valores iniciais entre 1 e 100.

## 8. Benchmarking e Geração de Gráficos

`main.py`:
1. Compila os executáveis (otimização `-O3`).
2. Monta lista de pares `(N, k)`:
  - Para `N < 10^4`: porcentagens (1%, 5%, 10%, 20%) da matriz densa.
  - Para `N ≥ 10^4`: segue escalas definidas pela potência de 10 (ex.: `N=10^5` => `k` em {10, 100, 1000}).
3. Gera arquivos de teste se não existirem.
4. Executa cada tipo de teste múltiplas vezes (`num_runs`) e calcula média.
5. Salva CSV (`resultados_com_denso_corrigido.csv`).
6. Cria gráficos log‑log por tipo de operação em `graficos/<operacao>/comparativo_cientifico_<operacao>.png`.

Dependências Python (ver `requirements.txt`): `numpy`, `pandas`, `matplotlib`, `seaborn`.

## 9. Como Compilar e Executar

### Linux / WSL
```bash
g++ -O3 -std=c++17 src/algoritmo1.cpp -o algoritmo1
g++ -O3 -std=c++17 src/algoritmo2.cpp -o algoritmo2
g++ -O3 -std=c++17 src/algoritmo_denso.cpp -o algoritmo_denso
```

### Windows (PowerShell, usando g++ do MinGW ou WSL)
```powershell
g++ -O3 -std=c++17 src\algoritmo1.cpp -o algoritmo1.exe
g++ -O3 -std=c++17 src\algoritmo2.cpp -o algoritmo2.exe
g++ -O3 -std=c++17 src\algoritmo_denso.cpp -o algoritmo_denso.exe
```

### Execução Manual
```bash
./algoritmo1 < tests/N_100_K_100/teste_soma.txt
```
Retorno de saída é silencioso (sem prints). Para validar manualmente, adicione temporariamente `std::cout` nos pontos desejados.

## 10. Reproduzindo os Experimentos

1. Instalar dependências Python:
  ```bash
  pip install -r requirements.txt
  ```
2. Executar script principal:
  ```bash
  python main.py
  ```
3. Acompanhar geração de testes e benchmarks no terminal.
4. Inspecionar `resultados_com_denso_corrigido.csv` e os gráficos em `graficos/`.

## 11. Estrutura de Diretórios (Resumo)
```
src/                # Implementações C++
tests/              # Casos gerados (por N e k)
graficos/           # Saída dos gráficos
main.py             # Orquestra experimento
gerador_testes.py   # Geração dos testes
requirements.txt    # Dependências Python
README.md           # Este documento
```
