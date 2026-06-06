#include <iostream>
#include <vector>
#include <string>
#include <cmath>
#include <algorithm>
#include <iomanip>
#include <chrono> // Biblioteca mantida para o cronômetro

using namespace std;

// Estrutura para armazenar as coordenadas bidimensionais de uma cidade
struct Cidade {
    int id;
    double x;
    double y;
};

// Função de cálculo de distância conforme especificado no enunciado do trabalho (EUC_2D)
int calcular_distancia(const Cidade& c1, const Cidade& c2) {
    double dx = c1.x - c2.x;
    double dy = c1.y - c2.y;
    // d_ij = floor(0.5 + sqrt((xi - xj)^2 + (yi - yj)^2))
    return static_cast<int>(std::floor(0.5 + std::sqrt(dx * dx + dy * dy)));
}

// Algoritmo de Prim para gerar a Lista de Adjacência da MST
void gerar_mst_prim(int n, const vector<Cidade>& cidades, vector<vector<int>>& adj_mst) {
    vector<int> chave(n, 2e9);
    vector<int> pai(n, -1);
    vector<bool> na_mst(n, false);

    chave[0] = 0;

    for (int cont = 0; cont < n - 1; ++cont) {
        int u = -1;
        int min_chave = 2e9;
        
        // Seleciona o vértice de menor chave que ainda não está na MST
        for (int v = 0; v < n; ++v) {
            if (!na_mst[v] && chave[v] < min_chave) {
                min_chave = chave[v];
                u = v;
            }
        }

        if (u == -1) break;
        na_mst[u] = true;

        // Atualiza os vizinhos do vértice selecionado
        for (int v = 0; v < n; ++v) {
            if (u != v && !na_mst[v]) {
                int dist = calcular_distancia(cidades[u], cidades[v]);
                if (dist < chave[v]) {
                    pai[v] = u;
                    chave[v] = dist;
                }
            }
        }
    }

    // Reconstrói a árvore como lista de adjacência
    for (int i = 1; i < n; ++i) {
        if (pai[i] != -1) {
            adj_mst[pai[i]].push_back(i);
            adj_mst[i].push_back(pai[i]);
        }
    }
}

// DFS para gerar o tour inicial (Twice-Around-the-Tree) coletando a ordem de visita
void dfs_mst(int u, const vector<vector<int>>& adj_mst, vector<bool>& visitado, vector<int>& tour) {
    visitado[u] = true;
    tour.push_back(u);
    for (int v : adj_mst[u]) {
        if (!visitado[v]) {
            dfs_mst(v, adj_mst, visitado, tour);
        }
    }
}

// Função auxiliar para calcular o custo total de um determinado tour
long long calcular_custo_tour(const vector<int>& tour, const vector<Cidade>& cidades) {
    long long custo = 0;
    int n = tour.size();
    for (int i = 0; i < n; ++i) {
        custo += calcular_distancia(cidades[tour[i]], cidades[tour[(i + 1) % n]]);
    }
    return custo;
}

// Otimização local utilizando a estratégia 2-Opt
void otimizar_2opt(vector<int>& tour, const vector<Cidade>& cidades) {
    int n = tour.size();
    bool melhorou = true;

    while (melhorou) {
        melhorou = false;
        for (int i = 1; i < n - 1; ++i) {
            for (int j = i + 1; j < n; ++j) {
                int a = tour[i - 1];
                int b = tour[i];
                int c = tour[j];
                int d = tour[(j + 1) % n];

                // Distâncias atuais vs Novas distâncias se trocarmos as arestas
                int dist_atual = calcular_distancia(cidades[a], cidades[b]) + calcular_distancia(cidades[c], cidades[d]);
                int dist_nova = calcular_distancia(cidades[a], cidades[c]) + calcular_distancia(cidades[b], cidades[d]);

                if (dist_nova < dist_atual) {
                    // Inverte o sub-intervalo de 'b' até 'c' (tour de i até j)
                    std::reverse(tour.begin() + i, tour.begin() + j + 1);
                    melhorou = true;
                }
            }
        }
    }
}

int main() {
    // Configuração para acelerar a leitura de dados via stdin
    ios_base::sync_with_stdio(false);
    cin.tie(NULL);

    string linha, nome_instancia = "desconhecido";
    int dimensao = 0;

    // 1. Parsing do cabeçalho da especificação da TSPLIB
    while (cin >> linha) {
        if (linha == "NAME:") {
            cin >> nome_instancia;
        } else if (linha == "DIMENSION:") {
            cin >> dimensao;
        } else if (linha == "NODE_COORD_SECTION") {
            break;
        }
    }

    if (dimensao == 0) return 0;

    vector<Cidade> cidades(dimensao);
    for (int i = 0; i < dimensao; ++i) {
        cin >> cidades[i].id >> cidades[i].x >> cidades[i].y;
    }

    // ==========================================
    // CRONÔMETRO INICIA AQUI (Após ler o arquivo)
    // ==========================================
    auto inicio = chrono::high_resolution_clock::now();

    // 2. Execução do Twice-Around-the-Tree
    vector<vector<int>> adj_mst(dimensao);
    gerar_mst_prim(dimensao, cidades, adj_mst);

    vector<bool> visitado(dimensao, false);
    vector<int> tour;
    tour.reserve(dimensao);
    dfs_mst(0, adj_mst, visitado, tour);

    // 3. Aplicação do Refinamento Local com 2-Opt
    otimizar_2opt(tour, cidades);

    // 4. Cálculo do custo final do circuito fechado
    long long peso_total = calcular_custo_tour(tour, cidades);

    // ==========================================
    // CRONÔMETRO FINALIZA AQUI
    // ==========================================
    auto fim = chrono::high_resolution_clock::now();
    chrono::duration<double, milli> tempo_gasto = fim - inicio;
    
    // Exibe o tempo gasto no fluxo de erro (mostra na tela, mas não salva no arquivo .tour)
    cerr << "Tempo de execucao (" << nome_instancia << "): " << fixed << setprecision(3) << tempo_gasto.count() << " ms\n";

    // 5. Impressão estritamente formatada exigida pelo enunciado
    cout << "NAME: " << nome_instancia << "\n";
    cout << "COMMENT: Gabriela, Higor, Vitor - Twice-Around-the-Tree com 2-Opt\n";
    cout << "TYPE: TOUR\n";
    cout << "DIMENSION: " << dimensao << "\n";
    cout << "TOTAL WEIGHT: " << peso_total << "\n";
    cout << "TOUR_SECTION\n";
    for (int i = 0; i < dimensao; ++i) {
        cout << cidades[tour[i]].id << "\n"; // Exibe o identificador real da cidade
    }
    cout << "EOF\n";

    return 0;
}