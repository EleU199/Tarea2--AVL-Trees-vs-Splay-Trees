# Tarea 2: Árboles AVL vs Splay Trees

Proyecto para la **Tarea 2 de CC4102 - Diseño y Análisis de Algoritmos**, Facultad de Ciencias Físicas y Matemáticas, Universidad de Chile.

**Integrantes:**
- Lucciano Aguilar
- Jeremy Cárcamo
- Anita Espinoza

El objetivo del proyecto es implementar y comparar experimentalmente dos estructuras de árboles binarios de búsqueda autoajustables o autobalanceados:

- **AVL Tree**
- **Splay Tree**

Se estudian distintos escenarios de inserción y búsqueda, además de experimentos asociados a propiedades teóricas de los Splay Trees:

- **Sequential Access Theorem**
- **Working Set Theorem**
- **Traversal Conjecture** como bonus

El proyecto mide tiempos de inserción, búsqueda, construcción y evolución amortizada mediante bloques de búsqueda. Los resultados se exportan a archivos CSV y luego se grafican con scripts de Python.

---

## 1. Estructura del proyecto

Una estructura general del repositorio es:

```text
t2cpp/
├── include/
│   ├── avl_tree.hpp
│   ├── splay_tree.hpp
│   ├── dataset.hpp
│   ├── timer.hpp
│   ├── experiments.hpp
│   └── bonus_experiments.hpp
├── src/
│   ├── main.cpp
│   ├── experiments.cpp
│   └── bonus_experiments.cpp
├── scripts/
│   ├── plot_results.py
│   └── plot_bonus_traversal.py
├── results/
│   └── .gitkeep
├── .gitignore
├── README.md
└── Makefile
```

La carpeta `results/` se usa para guardar los CSV y gráficos generados localmente. Estos archivos no son subidos al repositorio, salvo el archivo `.gitkeep`, cuyo único propósito es conservar la carpeta vacía en Git.

---

## 2. Descripción general

El proyecto implementa un árbol AVL y un Splay Tree para claves de tipo:

```cpp
std::uint32_t
```

El rango de generación de claves es:

```text
[0, 2^32 - 1]
```

Ambas estructuras usan un pool de nodos basado en `std::vector`, donde los hijos se representan mediante índices enteros en lugar de punteros directos. Esto permite reducir el overhead de memoria y mantener una mejor localidad de datos durante los experimentos.

---

## 3. Implementaciones principales

### 3.1. AVL Tree

Archivo principal:

```text
include/avl_tree.hpp
```

El AVL implementa:

- inserción;
- búsqueda;
- rotación simple a la derecha;
- rotación simple a la izquierda;
- balanceo LL, RR, LR y RL;
- verificación de propiedad BST;
- verificación del invariante AVL.

Cada nodo almacena:

```cpp
struct Node {
    Key key;
    Index left;
    Index right;
    int height;
    std::uint32_t count;
};
```

El campo `count` se utiliza para manejar claves repetidas sin crear nodos duplicados. Si una clave ya existe, se incrementa su contador interno.

---

### 3.2. Splay Tree

Archivo principal:

```text
include/splay_tree.hpp
```

El Splay Tree implementa:

- inserción;
- búsqueda;
- rotación simple a la derecha;
- rotación simple a la izquierda;
- operación `splay`;
- verificación de propiedad BST;
- verificación de enlaces `parent`;
- recorrido preorden para el bonus.

Cada nodo almacena:

```cpp
struct Node {
    Key key;
    Index left;
    Index right;
    Index parent;
    std::uint32_t count;
};
```

A diferencia del AVL, el Splay Tree no almacena altura ni factor de balance. Cada inserción y búsqueda lleva el nodo accedido a la raíz mediante rotaciones.

---

### 3.3. Generación de datos

Archivo principal:

```text
include/dataset.hpp
```

Este archivo contiene funciones para:

- generar claves uniformemente distribuidas;
- ordenar copias de datasets;
- generar secuencias de búsqueda uniforme;
- generar secuencias de búsqueda sesgada mediante una distribución exponencial discreta truncada.

La semilla por defecto es:

```cpp
constexpr std::uint64_t DEFAULT_SEED = 4102;
```

---

### 3.4. Medición de tiempo

Archivo principal:

```text
include/timer.hpp
```

El proyecto usa `std::chrono::steady_clock` para medir intervalos de tiempo. Los tiempos se almacenan principalmente en nanosegundos y luego se exportan también en milisegundos para facilitar la graficación.

---

## 4. Parámetros experimentales

Archivo principal:

```text
include/experiments.hpp
```

Constantes principales:

```cpp
constexpr int C = 1;
constexpr double LAMBDA = 0.01;
constexpr std::uint64_t SEED = 4102;
constexpr std::size_t BIG_N = std::size_t{1} << 25;
constexpr std::size_t SEARCH_BLOCK_SIZE = 1000;
```

Interpretación:

- `C = 1`: valor usado para calcular `M = 10 * C * N`.
- `LAMBDA = 0.01`: parámetro de la distribución sesgada exponencial.
- `BIG_N = 2^25`: tamaño del dataset grande para Sequential Access y Working Set.
- `SEARCH_BLOCK_SIZE = 1000`: tamaño de bloque usado para medir la evolución amortizada de búsquedas.

---

## 5. Experimentos implementados

### 5.1. Escenarios base

Para cada:

```text
N ∈ {2^10, 2^11, 2^12, 2^13, 2^14}
```

se generan datasets de claves aleatorias uniformes y se ejecutan:

```text
M = 10 * C * N
```

búsquedas.

Los escenarios base son:

```text
a) Inserción aleatoria, búsqueda uniforme
b) Inserción aleatoria, búsqueda sesgada
c) Inserción ordenada, búsqueda uniforme
d) Inserción ordenada, búsqueda sesgada
```

Para cada escenario se mide:

- tiempo total de inserción;
- tiempo total de búsqueda;
- tiempo promedio por inserción;
- tiempo promedio por búsqueda;
- cantidad de claves únicas insertadas;
- cantidad de búsquedas exitosas.

Además, se guardan mediciones por bloques de búsqueda para observar la evolución amortizada del Splay Tree durante la secuencia de accesos.

Archivos generados:

```text
results/base_results.csv
results/base_search_blocks.csv
```

---

### 5.2. Sequential Access Theorem

Se construye un dataset grande con:

```text
N = 2^25
```

Luego se genera una secuencia de búsqueda estrictamente creciente y se mide el tiempo acumulado para:

```text
m ∈ {N/100, 2N/100, ..., 9N/100, N/10}
```

El objetivo es comparar el comportamiento experimental de AVL y Splay en secuencias crecientes.

Archivo generado:

```text
results/sequential_results.csv
```

---

### 5.3. Working Set Theorem

Se construye un dataset grande con:

```text
N = 2^25
```

Para cada:

```text
W ∈ {10, 10^2, 10^3, 10^4, 10^5, 10^6}
```

se genera un subconjunto activo de tamaño `W`. Luego se ejecutan:

```text
M = 10 * C * N
```

búsquedas escogidas uniformemente dentro de dicho subconjunto.

El objetivo es analizar cómo cambia el costo promedio de búsqueda al variar el tamaño del working set.

Archivo generado:

```text
results/working_set_results.csv
```

---

### 5.4. Bonus: Traversal Conjecture

Archivos principales:

```text
include/bonus_experiments.hpp
src/bonus_experiments.cpp
```

El experimento bonus construye dos Splay Trees, `T1` y `T2`, con las mismas claves, pero insertadas en distinto orden. Luego:

1. se obtiene una secuencia recorriendo `T1` en preorden;
2. se busca esa secuencia completa en `T2`;
3. se mide el costo total y el costo por bloques de búsqueda.

Archivos generados:

```text
results/bonus_traversal_summary.csv
results/bonus_traversal_blocks.csv
```

---

## 6. Requisitos

### 6.1. Sistema usado para los resultados finales

Los resultados finales fueron obtenidos en el siguiente equipo:

```text
Sistema operativo: Windows 11 Pro
Procesador: AMD Ryzen 5 3600
Memoria RAM: 32,0 GB
Caché L1: 384 KB
Caché L2: 3 MB
Caché L3: 32 MB
Compilador: g++
```

### 6.2. Compilador

Se requiere un compilador con soporte para C++17.

Verificar instalación:

```bash
g++ --version
```

En Windows se puede usar MinGW-w64, MSYS2 o una instalación equivalente de GCC.

### 6.3. Python

Para graficar los resultados se requiere Python 3 con:

```bash
pip install pandas matplotlib
```

Si se usa un entorno virtual, activarlo antes de ejecutar los scripts.

---

## 7. Compilación

Todos los comandos deben ejecutarse desde la raíz del proyecto:

```text
t2cpp/
```

### 7.1. Compilación optimizada recomendada

En Windows PowerShell:

```powershell
g++ -std=c++17 -O3 -march=native -DNDEBUG -Wall -Wextra -Iinclude src/main.cpp src/experiments.cpp src/bonus_experiments.cpp -o t2cpp
```

Esto genera:

```text
t2cpp.exe
```

### 7.2. Compilación alternativa más conservadora

Si `-march=native` causa problemas en otro equipo, usar:

```powershell
g++ -std=c++17 -O3 -DNDEBUG -Wall -Wextra -Iinclude src/main.cpp src/experiments.cpp src/bonus_experiments.cpp -o t2cpp
```

### 7.3. Compilación sin bonus

Si se desea compilar solo los experimentos principales sin el bonus:

```powershell
g++ -std=c++17 -O3 -march=native -DNDEBUG -Wall -Wextra -Iinclude src/main.cpp src/experiments.cpp -o t2cpp
```

En ese caso, `main.cpp` no debe llamar a funciones del namespace `bonus`.

---

## 8. Ejecución

### 8.1. Ejecutar flujo completo

```powershell
.\t2cpp.exe
```

o equivalentemente:

```powershell
.\t2cpp.exe results
```

La carpeta `results` es la salida por defecto.

### 8.2. Ejecutar con prioridad alta en Windows

En PowerShell:

```powershell
$p = Start-Process ".\t2cpp.exe" -ArgumentList "results" -PassThru; $p.PriorityClass = "High"; $p.WaitForExit()
```

### 8.3. Control manual desde `main.cpp`

El archivo `src/main.cpp` contiene las llamadas principales:

```cpp
experiments::run_all_experiments(results_dir);
bonus::run_and_write_traversal_bonus(results_dir);
```

Para ejecutar solo los experimentos principales, comentar la línea del bonus:

```cpp
experiments::run_all_experiments(results_dir);
// bonus::run_and_write_traversal_bonus(results_dir);
```

Para ejecutar solo el bonus, comentar la línea de los experimentos principales:

```cpp
// experiments::run_all_experiments(results_dir);
bonus::run_and_write_traversal_bonus(results_dir);
```

Después de modificar `main.cpp`, recompilar antes de ejecutar.

---

## 9. Archivos de salida

Al ejecutar los experimentos principales se generan:

```text
results/base_results.csv
results/base_search_blocks.csv
results/sequential_results.csv
results/working_set_results.csv
```

Al ejecutar el bonus se generan:

```text
results/bonus_traversal_summary.csv
results/bonus_traversal_blocks.csv
```

---

## 10. Gráficos

Los scripts de graficación se encuentran en:

```text
scripts/plot_results.py
scripts/plot_bonus_traversal.py
```

### 10.1. Graficar experimentos principales

```powershell
python scripts/plot_results.py --clean
```

Si `python` no está asociado correctamente en Windows:

```powershell
py scripts/plot_results.py --clean
```

Los gráficos principales se guardan en:

```text
results/plots/tiempos_busqueda/
results/plots/tiempos_insercion/
```

### 10.2. Graficar bonus

```powershell
python scripts/plot_bonus_traversal.py --clean
```

También se puede modificar la ventana del promedio móvil:

```powershell
python scripts/plot_bonus_traversal.py --clean --rolling 200
```

Los gráficos del bonus se guardan en:

```text
results/plots/bonus_traversal/
```

---

## 11. Gráficos principales generados

### 11.1. Tiempos de búsqueda

Carpeta:

```text
results/plots/tiempos_busqueda/
```

Incluye, entre otros:

```text
busqueda_total_base_por_escenario__random_insert_uniform_search.png
busqueda_total_base_por_escenario__random_insert_biased_search.png
busqueda_total_base_por_escenario__sorted_insert_uniform_search.png
busqueda_total_base_por_escenario__sorted_insert_biased_search.png
busqueda_bloques_resumen_n_max__random_insert_biased_search.png
busqueda_bloques_resumen_n_max__sorted_insert_biased_search.png
busqueda_sequential_total.png
busqueda_sequential_promedio.png
busqueda_working_set_total.png
busqueda_working_set_promedio.png
```

### 11.2. Tiempos de inserción

Carpeta:

```text
results/plots/tiempos_insercion/
```

Incluye, entre otros:

```text
insercion_total_base_por_escenario__random_insert_uniform_search.png
insercion_total_base_por_escenario__random_insert_biased_search.png
insercion_total_base_por_escenario__sorted_insert_uniform_search.png
insercion_total_base_por_escenario__sorted_insert_biased_search.png
insercion_sequential_construccion.png
insercion_working_set_construccion.png
```

### 11.3. Bonus

Carpeta:

```text
results/plots/bonus_traversal/
```

Incluye:

```text
bonus_resumen_tiempos_principales.png
bonus_busqueda_promedio_por_bloque.png
bonus_busqueda_promedio_movil.png
bonus_busqueda_total_por_bloque.png
bonus_busqueda_tiempo_acumulado.png
```

---

## 12. Formato de CSV

### 12.1. `base_results.csv`

Columnas principales:

```csv
scenario,tree,n,m,c,lambda,inserted_unique,found_count,insert_time_ns,search_time_ns,insert_time_ms,search_time_ms,avg_insert_ns,avg_search_ns
```

### 12.2. `base_search_blocks.csv`

Columnas principales:

```csv
scenario,tree,n,m,block_index,block_start,block_size,found_count,block_time_ns,block_time_ms,avg_search_ns
```

### 12.3. `sequential_results.csv`

Columnas principales:

```csv
tree,n,m,inserted_unique,found_count,build_time_ns,search_time_ns
```

### 12.4. `working_set_results.csv`

Columnas principales:

```csv
tree,n,w,m,c,inserted_unique,found_count,build_time_ns,search_time_ns
```

### 12.5. `bonus_traversal_summary.csv`

Columnas principales:

```csv
n,block_size,build_t1_time_ns,build_t2_time_ns,traversal_time_ns,search_time_ns,sequence_size,found_count,build_t1_time_ms,build_t2_time_ms,traversal_time_ms,search_time_ms,avg_search_ns
```

### 12.6. `bonus_traversal_blocks.csv`

Columnas principales:

```csv
block_index,block_start,block_size,found_count,block_time_ns,block_time_ms,avg_search_ns
```

---

## 13. Flujo completo para replicar la tarea

Desde cero, el flujo recomendado es:

### Paso 1: compilar

```powershell
g++ -std=c++17 -O3 -march=native -DNDEBUG -Wall -Wextra -Iinclude src/main.cpp src/experiments.cpp src/bonus_experiments.cpp -o t2cpp
```

### Paso 2: ejecutar experimentos

```powershell
.\t2cpp.exe 
```

### Paso 3: generar gráficos principales

```powershell
python scripts/plot_results.py --clean
```

### Paso 4: generar gráficos del bonus

```powershell
python scripts/plot_bonus_traversal.py --clean
```

---

## 14. Notas sobre reproducibilidad

- Los datasets se generan pseudoaleatoriamente con semilla fija.
- Los resultados de tiempo pueden variar según CPU, RAM, sistema operativo, compilador, temperatura, procesos en segundo plano y estado de caché.
- Las mediciones se hacen en nanosegundos usando `std::chrono::steady_clock`.
- Para evitar que el overhead del reloj afecte la medición de cada búsqueda individual, se miden bloques de búsquedas y se reporta el promedio por bloque.
- En los experimentos grandes, especialmente Working Set y Traversal Conjecture, el tiempo de ejecución puede ser considerable. En el equipo usado para los resultados finales, la batería grande puede tardar un par de horas.(o más en máquinas antiguas)
- Las funciones de debugging como verificación de invariantes no deben ejecutarse dentro de los loops de medición grandes.
- Los gráficos se generan desde los CSV ya producidos por los ejecutables, por lo que no es necesario repetir los experimentos para volver a graficar.Esto permite modificar los gráficos desde los archivos `.py`.
-### Uso del Makefile

El proyecto incluye un `Makefile` para simplificar la compilación y ejecución.
Desde la raíz del proyecto, ejecutar `make` o `mingw32-make` compila el ejecutable principal.
El comando `make run` o `mingw32-make run` compila y ejecuta los experimentos.
El comando `make clean` o `mingw32-make clean` elimina los ejecutables generados.
En Windows con MinGW normalmente se usa `mingw32-make` en lugar de `make`.