import os
import random

# Operações incluídas (códigos: 1,2,3,4,6)
OP_TYPES = [
    ("consulta", 1),
    ("set", 2),
    ("transpose", 3),
    ("soma", 4),
    ("multiplicacao", 6),
]

def _gera_matriz(N, k, rng):
    # Gera 'k' posições únicas com valores aleatórios
    # Se k for muito próximo de N*N, a abordagem de set é lenta, mas para matriz esparsa ok.
    k = min(k, N * N)
    usados = set()
    elems = []
    
    # Proteção contra loop infinito se k ~ N*N
    if k > (N * N) // 2:
        # Se for densa, gera tudo e remove (não deve acontecer no projeto de matriz esparsa)
        pass 
    
    while len(elems) < k:
        i = rng.randrange(N)
        j = rng.randrange(N)
        if (i, j) in usados:
            continue
        usados.add((i, j))
        v = rng.randint(1, 100)
        elems.append((i, j, v))
    return elems

def _linha_matriz(elems, N):
    # Formato esperado pelos algoritmos: k N
    out = [f"{len(elems)} {N}"]
    out += [f"{i} {j} {v}" for (i, j, v) in elems]
    return out

def _gera_queries(tipo_nome, tipo_cod, N, rng, qtd=1000):
    qs = []
    for _ in range(qtd):
        if tipo_cod == 1:  # acesso
            m = rng.choice([1, 2])
            i = rng.randrange(N)
            j = rng.randrange(N)
            qs.append(f"1 {m} {i} {j}")
        elif tipo_cod == 2:  # set
            m = rng.choice([1, 2])
            i = rng.randrange(N)
            j = rng.randrange(N)
            v = rng.randint(0, 100)
            qs.append(f"2 {m} {i} {j} {v}")
        elif tipo_cod == 3:  # transpose
            m = rng.choice([1, 2])
            qs.append(f"3 {m}")
        elif tipo_cod == 4:  # soma
            qs.append("4")
        elif tipo_cod == 6:  # multiplicacao
            qs.append("6")
    return qs

def write_test_files(N, K, base_dir):
    # Seed fixa para reprodutibilidade baseada nos parâmetros
    rng = random.Random(hash((N, K)) & 0xFFFFFFFF)
    dir_pair = os.path.join(base_dir, f"N_{N}_K_{K}")
    os.makedirs(dir_pair, exist_ok=True)

    # Gera matrizes base A e B
    # Se N for gigante (10^6) e K pequeno, isso é rápido.
    A = _gera_matriz(N, K, rng)
    B = _gera_matriz(N, K, rng)

    # Arquivo 1: apenas inserção/carregamento (Q=0)
    insercao_path = os.path.join(dir_pair, "teste_insercao.txt")
    if not os.path.exists(insercao_path):
        linhas = _linha_matriz(A, N) + _linha_matriz(B, N) + ["0"]
        with open(insercao_path, "w") as f:
            f.write("\n".join(linhas))
    
    # Demais arquivos de teste
    for nome, cod in OP_TYPES:
        path = os.path.join(dir_pair, f"teste_{nome}.txt")
        if os.path.exists(path):
            continue
            
        # Define quantidade de queries.
        # Multiplicação é O(K*d) ou O(N^3), então fazemos poucas.
        # Soma é O(K), fazemos moderado.
        # Set/Get é O(1)/O(log), fazemos muitas.
        if cod == 6: 
            qtd = 5 
        elif cod == 4 or cod == 3:
            qtd = 20
        else:
            qtd = 10000 # Testar throughput de consultas
            
        queries = _gera_queries(nome, cod, N, rng, qtd=qtd)
        linhas = _linha_matriz(A, N) + _linha_matriz(B, N) + [str(len(queries))] + queries
        with open(path, "w") as f:
            f.write("\n".join(linhas))

def ensure_tests_for_pairs(pairs, base_dir="tests"):
    """Garante que os arquivos de teste existam para todos os pares (N, K)."""
    print(f"Verificando/Gerando arquivos de teste em '{base_dir}'...")
    count = 0
    for (N, K) in pairs:
        write_test_files(N, K, base_dir)
        count += 1
        if count % 5 == 0:
            print(f"Processados {count}/{len(pairs)} pares...")
    print("Geração de testes finalizada.")