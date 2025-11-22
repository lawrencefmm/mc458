# Sparse Matrix Utilities

Código em C++ que implementa uma estrutura de matriz esparsa com operações:
- leitura de duas matrizes esparsas,
- acesso e modificação de elementos,
- transposição (view O(1)),
- soma, multiplicação e escala,
- impressão densa N x N.

Compilação
```
g++ -std=c++17 -O2 sparse_table.cpp -o sparse_table
```

Execução
```
./sparse_table < entrada.txt
```

Formato de entrada
- Primeira matriz: `k1 N` seguido de `k1` linhas `i j v`
- Segunda matriz: `k2 N` seguido de `k2` linhas `i j v`
- Em seguida: um inteiro `Q` com número de operações, seguido de Q comandos:
  - `1 m i j` -> imprimir A(i,j) onde m = 1 para primeira matriz, 2 para segunda
  - `2 m i j v` -> definir A(i,j) = v
  - `3 m` -> alternar view transposta de m e imprimir denso
  - `4` -> imprimir A + B (denso)
  - `5 m alpha` -> imprimir alpha * (matriz m) (denso)
  - `6` -> imprimir A * B (denso)

Observações
- Índices e valores seguem o formato usado no código (inteiros).
- O programa imprime matrizes densas N x N, separando elementos por espaço.
