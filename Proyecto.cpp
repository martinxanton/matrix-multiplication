#include <iostream>
#include <vector>
#include <thread>
#include <mutex>
#include <chrono>

using namespace std;

// Variable para proteger el acceso a C
std::mutex mtx;

// Función para la multiplicación secuencial de matrices
vector<vector<int>> multiplicacionSecuencial(vector<vector<int>> &A, vector<vector<int>> &B)
{
    int n = A.size();
    vector<vector<int>> C(n, vector<int>(n));
    for (int i = 0; i < n; i++)
    {
        for (int j = 0; j < n; j++)
        {
            for (int k = 0; k < n; k++)
            {
                C[i][j] += A[i][k] * B[k][j];
            }
        }
    }
    return C;
}

// Función para la multiplicación paralela de matrices
vector<vector<int>> multiplicacionParalela(vector<vector<int>> &A, vector<vector<int>> &B)
{
    int n = A.size();
    vector<vector<int>> C(n, vector<int>(n));
    // Número de threads
    int num_threads = 4;
    // Vector de threads
    vector<thread> threads(num_threads);

    // División del trabajo entre threads
    for (int i = 0; i < num_threads; i++)
    {
        threads[i] = thread([&, i, n]()
                            {
      for (int j = i * n / num_threads; j < (i + 1) * n / num_threads; j++) {
        for (int k = 0; k < n; k++) {
          for (int l = 0; l < n; l++) {
            // Bloquea el acceso a C
            std::lock_guard<std::mutex> lock(mtx);
            C[j][l] += A[j][k] * B[k][l];
            // Desbloquea el acceso a C
          }
        }
      } });
    }

    // Sincronización de threads
    for (auto &t : threads)
    {
        t.join();
    }

    return C;
}

// Función para comparar y mostrar resultados
void compararResultados(vector<vector<int>> &C1, vector<vector<int>> &C2)
{
    bool iguales = true;
    for (int i = 0; i < C1.size(); i++)
    {
        for (int j = 0; j < C1[i].size(); j++)
        {
            if (C1[i][j] != C2[i][j])
            {
                iguales = false;
                cout << "Matrices diferentes en elemento (" << i << ", " << j << "): " << C1[i][j] << " vs " << C2[i][j] << endl;
                break;
            }
        }
    }
    if (iguales)
    {
        cout << "El resultado de la multiplicacion de matrices son iguales." << endl;
    }
}

int main()
{
    // Tamaño de la matriz
    int n = 500;

    /*
      // Generación de matrices aleatorias

      vector<vector<int>> A(n, vector<int>(n));
      vector<vector<int>> B(n, vector<int>(n));
      for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
          A[i][j] = rand() % 10;
          B[i][j] = rand() % 10;
        }
      }
    */
    // Define las matrices fijas
    vector<vector<int>> A(n, vector<int>(n, 0));
    for (int i = 0; i < n; i++)
    {
        for (int j = i; j < n; j++)
        {
            A[i][j] = rand() % n;
        }
    }

    vector<vector<int>> B(n, vector<int>(n, 50));
    for (int i = 0; i < n; i++)
    {
        B[i][i] = 1;
    }

    // Medición del tiempo para la multiplicación secuencial
    auto start = chrono::high_resolution_clock::now();
    vector<vector<int>> C_secuencial = multiplicacionSecuencial(A, B);
    auto end = chrono::high_resolution_clock::now();
    double tiempo_secuencial = chrono::duration_cast<chrono::milliseconds>(end - start).count();

    // Medición del tiempo para la multiplicación paralela
    start = chrono::high_resolution_clock::now();
    vector<vector<int>> C_paralela = multiplicacionParalela(A, B);
    end = chrono::high_resolution_clock::now();
    double tiempo_paralela = chrono::duration_cast<chrono::milliseconds>(end - start).count();

    // Impresión de los tiempos de ejecución
    cout << "Tiempo secuencial: " << tiempo_secuencial << " ms" << endl;
    cout << "Tiempo paralelo: " << tiempo_paralela << " ms" << endl;
    // Verificación de la correctitud del resultado
    bool correcto = true;
    for (int i = 0; i < n; i++)
    {
        for (int j = 0; j < n; j++)
        {
            if (C_secuencial[i][j] != C_paralela[i][j])
            {
                correcto = false;
                cout << "**ERROR:** Resultados incorrectos en elemento (" << i << ", " << j << "):" << endl;
                cout << "  Secuencial: " << C_secuencial[i][j] << endl;
                cout << "  Paralelo: " << C_paralela[i][j] << endl;
                break;
            }
        }
        if (!correcto)
        {
            break;
        }
    }

    if (correcto)
    {
        cout << "Los resultados de la multiplicación son correctos" << endl;
    }

    return 0;
}