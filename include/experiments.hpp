#pragma once

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

/**
 * @brief Declaraciones para ejecutar los experimentos de la Tarea 2.
 *
 * Este archivo define:
 * - Constantes globales de experimentación.
 * - Tipos de escenarios.
 * - Estructuras para guardar resultados.
 * - Firmas de funciones para ejecutar experimentos y exportar CSV.
 */
namespace experiments {

using Key = std::uint32_t;

/**
 * @brief Valor c usado para calcular M = 10 * c * N.
 *
 * Elegimos c = 1 para reducir el costo experimental, manteniéndonos dentro
 * del rango permitido por el enunciado.
 */
constexpr int C = 1;

/**
 * @brief Parámetro lambda para la distribución sesgada exponencial.
 *
 * El enunciado recomienda valores en [0.001, 0.05].
 */
constexpr double LAMBDA = 0.01;

/**
 * @brief Cantidad de búsquedas medidas por bloque.
 *
 * Medir búsqueda por búsqueda puede introducir demasiado overhead del reloj.
 * Por eso medimos bloques de búsquedas.
 */
constexpr std::size_t SEARCH_BLOCK_SIZE = 1000;

/**
 * @brief Semilla base para reproducibilidad.
 */
constexpr std::uint64_t SEED = 4102;

/**
 * @brief Tamaño del dataset grande usado en los experimentos de teoremas.
 */
constexpr std::size_t BIG_N = std::size_t{1} << 25;

/**
 * @brief Tipo de árbol evaluado.
 */
enum class TreeType {
    AVL,
    Splay
};

/**
 * @brief Escenarios base de inserción y búsqueda.
 */
enum class BaseScenario {
    RandomInsertUniformSearch,
    RandomInsertBiasedSearch,
    SortedInsertUniformSearch,
    SortedInsertBiasedSearch
};

/**
 * @brief Resultado de un experimento base para un árbol y escenario específico.
 */
struct BaseResult {
    BaseScenario scenario;
    TreeType tree;

    std::size_t n;
    std::size_t m;

    int c;
    double lambda;

    std::size_t inserted_unique;
    std::size_t found_count;

    std::uint64_t insert_time_ns;
    std::uint64_t search_time_ns;
};

/**
 * @brief Resultado de un bloque de búsquedas dentro de un experimento base.
 *
 * Sirve para observar la evolución amortizada durante una secuencia de búsquedas.
 */
struct BaseSearchBlockResult {
    BaseScenario scenario;
    TreeType tree;

    std::size_t n;
    std::size_t m;

    std::size_t block_index;
    std::size_t block_start;
    std::size_t block_size;

    std::size_t found_count;

    std::uint64_t block_time_ns;
};

/**
 * @brief Contenedor de resultados base.
 *
 * Incluye resultados agregados y resultados por bloque de búsqueda.
 */
struct BaseExperimentOutput {
    std::vector<BaseResult> summary_results;
    std::vector<BaseSearchBlockResult> block_results;
};

/**
 * @brief Ejecuta todos los escenarios base para AVL y Splay.
 *
 * Retorna tanto resultados agregados como tiempos por bloque de búsqueda.
 */
BaseExperimentOutput run_base_experiments_with_blocks();

/**
 * @brief Escribe resultados por bloque de búsqueda en un archivo CSV.
 *
 * @param results Resultados por bloque.
 * @param output_path Ruta del archivo CSV de salida.
 */
void write_base_search_block_results_csv(
    const std::vector<BaseSearchBlockResult>& results,
    const std::string& output_path
);
/**
 * @brief Resultado del experimento Sequential Access Theorem.
 */
struct SequentialResult {
    TreeType tree;

    std::size_t n;
    std::size_t m;

    std::size_t inserted_unique;
    std::size_t found_count;

    std::uint64_t build_time_ns;
    std::uint64_t search_time_ns;
};

/**
 * @brief Resultado del experimento Working Set Theorem.
 */
struct WorkingSetResult {
    TreeType tree;

    std::size_t n;
    std::size_t w;
    std::size_t m;

    int c;

    std::size_t inserted_unique;
    std::size_t found_count;

    std::uint64_t build_time_ns;
    std::uint64_t search_time_ns;
};

/**
 * @brief Convierte un tipo de árbol a string para CSV o logs.
 */
std::string to_string(TreeType tree);

/**
 * @brief Convierte un escenario base a string para CSV o logs.
 */
std::string to_string(BaseScenario scenario);

/**
 * @brief Retorna los valores de N para los escenarios base.
 *
 * N ∈ {2^10, 2^11, 2^12, 2^13, 2^14}.
 */
std::vector<std::size_t> base_n_values();

/**
 * @brief Retorna los valores de m para Sequential Access dado N.
 *
 * m ∈ {N/100, 2N/100, ..., 9N/100, N/10}.
 */
std::vector<std::size_t> sequential_m_values(std::size_t n);

/**
 * @brief Retorna los valores de W para Working Set.
 *
 * W ∈ {10, 10^2, 10^3, ..., 10^6}.
 */
std::vector<std::size_t> working_set_values();

/**
 * @brief Ejecuta todos los escenarios base para AVL y Splay.
 *
 * @return Vector con todos los resultados base.
 */
std::vector<BaseResult> run_base_experiments();

/**
 * @brief Ejecuta el experimento Sequential Access Theorem.
 *
 * @return Vector con resultados para AVL y Splay.
 */
std::vector<SequentialResult> run_sequential_access_experiment();

/**
 * @brief Ejecuta el experimento Working Set Theorem.
 *
 * @return Vector con resultados para AVL y Splay.
 */
std::vector<WorkingSetResult> run_working_set_experiment();

/**
 * @brief Escribe resultados base en un archivo CSV.
 *
 * @param results Resultados a escribir.
 * @param output_path Ruta del archivo CSV de salida.
 */
void write_base_results_csv(
    const std::vector<BaseResult>& results,
    const std::string& output_path
);

/**
 * @brief Escribe resultados Sequential Access en un archivo CSV.
 *
 * @param results Resultados a escribir.
 * @param output_path Ruta del archivo CSV de salida.
 */
void write_sequential_results_csv(
    const std::vector<SequentialResult>& results,
    const std::string& output_path
);

/**
 * @brief Escribe resultados Working Set en un archivo CSV.
 *
 * @param results Resultados a escribir.
 * @param output_path Ruta del archivo CSV de salida.
 */
void write_working_set_results_csv(
    const std::vector<WorkingSetResult>& results,
    const std::string& output_path
);

/**
 * @brief Ejecuta toda la batería experimental y guarda CSVs en results_dir.
 *
 * @param results_dir Carpeta donde se guardarán los archivos CSV.
 */
void run_all_experiments(const std::string& results_dir);

} // namespace experiments