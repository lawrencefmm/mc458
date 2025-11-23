import subprocess
import time
import pandas as pd
import seaborn as sns
import matplotlib.pyplot as plt
import numpy as np
import os
from math import log10
from concurrent.futures import ThreadPoolExecutor, as_completed

# --- IMPORTANTE: Certifique-se de que gerador_testes.py está na mesma pasta ---
try:
    from gerador_testes import ensure_tests_for_pairs
except ImportError:
    print("ERRO: 'gerador_testes.py' não encontrado. O script não pode gerar os casos de teste.")
    exit(1)

# --- Configuração dos Experimentos ---

N_values = [100, 1000, 10000, 100000, 1000000] 
# N=100 será o único onde testaremos a matriz densa/linear
num_runs = 6

# --- Funções Auxiliares ---

def compile_cpp_programs():
    """Compila os programas C++ necessários, incluindo o denso."""
    print("Compilando programas C++...")
    cpp_files = {
        "gerador": "src/gerador.cpp",
        "algoritmo1": "src/algoritmo1.cpp",
        "algoritmo2": "src/algoritmo2.cpp",
        "algoritmo_denso": "src/algoritmo_denso.cpp"
    }
    
    # Cria pasta src caso não exista e move arquivos se necessário (opcional, apenas organização)
    # Aqui assumimos que o usuário já tem os arquivos ou na raiz ou em src/
    
    for executable, source in cpp_files.items():
        # Fallback: se não estiver em src/, tenta na raiz
        if not os.path.exists(source) and os.path.exists(os.path.basename(source)):
            source = os.path.basename(source)

        if not os.path.exists(source):
            print(f"Aviso: Arquivo de origem não encontrado: {source}. Pulando compilação.")
            continue
            
        print(f"Compilando {source} -> {executable}...")
        try:
            subprocess.run(
                ["g++", "-o", executable, source, "-O3", "-std=c++17"],
                check=True,
                capture_output=True,
                text=True
            )
        except subprocess.CalledProcessError as e:
            print(f"Erro ao compilar {source}:")
            print(e.stderr)
            return False
            
    print("Compilação concluída com sucesso.")
    return True

def run_experiment(N, k, runs):
    """Executa experimentos. Denso roda apenas se N=100."""
    # print(f"Executando para N={N}, k={k}...") # Comentei para limpar o output
    base_dir = "tests"
    pair_dir = os.path.join(base_dir, f"N_{N}_K_{k}")
    file_types = [
        ("insercao", "teste_insercao.txt"),
        ("consulta", "teste_consulta.txt"),
        ("set", "teste_set.txt"),
        ("transpose", "teste_transpose.txt"),
        ("soma", "teste_soma.txt"),
        ("multiplicacao", "teste_multiplicacao.txt"),
    ]
    results = []
    
    run_dense = (N == 100)

    for tipo, fname in file_types:
        path = os.path.join(pair_dir, fname)
        if not os.path.exists(path):
            continue
        with open(path, "r") as f:
            test_input = f.read()
        
        times_algo1 = []
        times_algo2 = []
        times_dense = []

        for i in range(runs):
            # Algoritmo 1
            try:
                start = time.perf_counter()
                subprocess.run(["./algoritmo1"], input=test_input, text=True, capture_output=True, check=True)
                times_algo1.append(time.perf_counter() - start)
            except Exception:
                times_algo1.append(np.nan)
            
            # Algoritmo 2
            try:
                start = time.perf_counter()
                subprocess.run(["./algoritmo2"], input=test_input, text=True, capture_output=True, check=True)
                times_algo2.append(time.perf_counter() - start)
            except Exception:
                times_algo2.append(np.nan)

            # Algoritmo Denso
            if run_dense:
                try:
                    start = time.perf_counter()
                    subprocess.run(["./algoritmo_denso"], input=test_input, text=True, capture_output=True, check=True, timeout=60)
                    times_dense.append(time.perf_counter() - start)
                except subprocess.TimeoutExpired:
                    # print(f"  [Denso] Timeout para N={N}, tipo={tipo}")
                    times_dense.append(np.nan)
                except Exception as e:
                    # print(f"  [Denso] Erro: {e}")
                    times_dense.append(np.nan)
            else:
                times_dense.append(np.nan)

        results.append({
            "N": N,
            "k": k,
            "sparsity": k / (N*N),
            "test_type": tipo,
            "time_algo1": np.mean(times_algo1),
            "time_algo2": np.mean(times_algo2),
            "time_dense": np.mean(times_dense) if run_dense and times_dense else np.nan
        })
    return results

def plot_results(df):
    """Gera gráficos com estilo científico e alta resolução."""
    print("\nGerando gráficos...")
    
    if not os.path.exists("graficos"):
        os.makedirs("graficos")

    # --- Configuração de Estilo Científico ---
    sns.set_theme(style="whitegrid", context="talk", font_scale=1.0)
    
    plt.rcParams['font.family'] = 'sans-serif'
    plt.rcParams['axes.edgecolor'] = '#333333'
    plt.rcParams['grid.linestyle'] = '--'
    plt.rcParams['grid.alpha'] = 0.6
    plt.rcParams['legend.frameon'] = True
    plt.rcParams['legend.framealpha'] = 0.9
    plt.rcParams['legend.facecolor'] = 'white'

    df_melted = df.melt(
        id_vars=['N', 'k', 'sparsity', 'test_type'], 
        value_vars=['time_algo1', 'time_algo2', 'time_dense'], 
        var_name='Algoritmo', 
        value_name='Tempo (s)'
    )
    
    nome_map = {
        'time_algo1': 'Algoritmo 1 (Map)',
        'time_algo2': 'Algoritmo 2 (Vector/Map)',
        'time_dense': 'Denso (Ref)'
    }
    df_melted['Algoritmo'] = df_melted['Algoritmo'].map(nome_map)
    
    df_plot = df_melted.dropna(subset=['Tempo (s)'])
    palette = sns.color_palette("deep")

    for tipo in df['test_type'].unique():
        out_dir = os.path.join("graficos", tipo)
        os.makedirs(out_dir, exist_ok=True)

        subset = df_plot[df_plot['test_type'] == tipo]
        if subset.empty:
            continue

        plt.figure(figsize=(10, 7))
        
        sns.lineplot(
            data=subset, 
            x='N', 
            y='Tempo (s)', 
            hue='Algoritmo', 
            style='Algoritmo', 
            palette=palette,
            markers=True, 
            dashes=False, 
            linewidth=2.5,
            markersize=9,
            alpha=0.9
        )

        plt.title(f'Desempenho: {tipo.capitalize()}', fontsize=16, pad=15, weight='bold')
        plt.xlabel('Dimensão da Matriz (N)', fontsize=14, labelpad=10)
        plt.ylabel('Tempo Médio de Execução (s)', fontsize=14, labelpad=10)
        
        plt.xscale('log')
        plt.yscale('log')
        
        plt.grid(True, which="major", color='#dddddd', linestyle='-', linewidth=1.0)
        plt.grid(True, which="minor", color='#eeeeee', linestyle=':', linewidth=0.7)

        plt.legend(title="Implementação", loc='best', fontsize=11, title_fontsize=12)
        plt.tight_layout()
        
        filename = os.path.join(out_dir, f"comparativo_cientifico_{tipo}.png")
        plt.savefig(filename, dpi=300, bbox_inches='tight')
        plt.close()
        print(f"  Gráfico salvo: {filename}")

# --- Função Principal ---

def main():
    if not compile_cpp_programs():
        return
        
    all_results = []
    pares = []
    
    # --- 1. Definição dos Pares (N, k) ---
    for n in N_values:
        if n < 10000:
            # N < 10^4: 1%, 5%, 10%, 20%
            ratios = [0.01, 0.05, 0.1, 0.2]
            for r in ratios:
                k = int(r * n * n)
                if k > 0: pares.append((n, k))
        else:
            # N >= 10^4: Regras do PDF baseadas no expoente i
            i = int(log10(n))
            k1 = int(10**(i - 4))
            k2 = int(10**(i - 3))
            k3 = int(10**(i - 2))
            
            for k in [k1, k2, k3]:
                if k > 0: pares.append((n, k))

    print(f"Total de configurações (N, k) planejadas: {len(pares)}")
    
    # --- 2. Geração dos Arquivos de Teste ---
    # Chamada explícita ao gerador_testes.py
    print("\nIniciando geração de casos de teste...")
    ensure_tests_for_pairs(pares, base_dir="tests")
    print("Geração de testes concluída.\n")
    
    # --- 3. Execução dos Experimentos ---
    max_workers = min(len(pares), os.cpu_count()-2 or 1)
    print(f"Executando benchmarks em paralelo com {max_workers} threads...")
    
    futures = {}
    with ThreadPoolExecutor(max_workers=max_workers) as ex:
        for (n, k) in pares:
            futures[ex.submit(run_experiment, n, k, num_runs)] = (n, k)
            
        # Barra de progresso simples
        completed = 0
        total = len(pares)
        for fut in as_completed(futures):
            n, k = futures[fut]
            completed += 1
            # print(f"[{completed}/{total}] Concluído N={n}, k={k}")
            try:
                res_list = fut.result()
                all_results.extend(res_list)
            except Exception as e:
                print(f"Erro em N={n}: {e}")

    if not all_results:
        print("Nenhum resultado gerado.")
        return

    # --- 4. Salvamento e Gráficos ---
    results_df = pd.DataFrame(all_results)
    results_df.to_csv("resultados_com_denso_corrigido.csv", index=False)
    print(f"\nDados salvos em 'resultados_com_denso_corrigido.csv'.")
    
    plot_results(results_df)
    print("\nExperimentos finalizados com sucesso.")

if __name__ == "__main__":
    main()