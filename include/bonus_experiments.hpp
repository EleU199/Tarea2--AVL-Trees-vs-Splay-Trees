#pragma once

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

/**
 * @brief Experimentos bonus para Traversal Conjecture.
 *
 * Se separa del flujo principal para no mezclar los experimentos base,
 * Sequential Access y Working Set con el experimento bonus.
 */
namespace bonus {

using Key = std::uint32_t;

/**
 * @brief Tamaño por defecto para el bonus.
 *
 * El enunciado usa N = 2^25.
 */
constexpr std::size_t DEFAULT_BONUS_N = std::size_t{1} << 25;

/**
 * @brief Semilla base para reproducibilidad.
 */
constexpr std::uint64_t BONUS_SEED = 4102;

/**
 * @brief Cantidad de búsquedas medidas por bloque.
 *
 * Igual que en los experimentos base, medimos por bloques para evitar
 * que el overhead del reloj domine al medir búsqueda por búsqueda.
 */
constexpr std::size_t BONUS_BLOCK_SIZE = 1000;

/**
 * @brief Resumen global del experimento bonus.
 */
struct TraversalBonusSummary {
    std::size_t n;
    std::size_t block_size;

    std::uint64_t build_t1_time_ns;
    std::uint64_t build_t2_time_ns;
    std::uint64_t traversal_time_ns;
    std::uint64_t search_time_ns;

    std::size_t sequence_size;
    std::size_t found_count;
};

/**
 * @brief Resultado de un bloque de búsquedas del bonus.
 */
struct TraversalBonusBlock {
    std::size_t block_index;
    std::size_t block_start;
    std::size_t block_size;

    std::size_t found_count;

    std::uint64_t block_time_ns;
};

/**
 * @brief Resultado completo del bonus.
 */
struct TraversalBonusResult {
    TraversalBonusSummary summary;
    std::vector<TraversalBonusBlock> blocks;
};

/**
 * @brief Ejecuta el experimento bonus Traversal Conjecture.
 *
 * Construye dos Splay Trees con los mismos elementos, pero con distinto
 * orden de inserción. Luego obtiene el recorrido preorden de T1 y busca
 * esa secuencia en T2.
 *
 * @param n Tamaño del dataset.
 * @return Resultados agregados y por bloques.
 */
TraversalBonusResult run_traversal_bonus(std::size_t n = DEFAULT_BONUS_N);

/**
 * @brief Escribe el resumen del bonus en CSV.
 *
 * @param summary Resumen a escribir.
 * @param output_path Ruta del archivo CSV.
 */
void write_traversal_bonus_summary_csv(
    const TraversalBonusSummary& summary,
    const std::string& output_path
);

/**
 * @brief Escribe los resultados por bloque del bonus en CSV.
 *
 * @param blocks Bloques a escribir.
 * @param output_path Ruta del archivo CSV.
 */
void write_traversal_bonus_blocks_csv(
    const std::vector<TraversalBonusBlock>& blocks,
    const std::string& output_path
);

/**
 * @brief Ejecuta el bonus y escribe sus CSV en results_dir.
 *
 * Genera:
 * - bonus_traversal_summary.csv
 * - bonus_traversal_blocks.csv
 *
 * @param results_dir Carpeta de resultados.
 * @param n Tamaño del dataset.
 */
void run_and_write_traversal_bonus(
    const std::string& results_dir,
    std::size_t n = DEFAULT_BONUS_N
);

} // namespace bonus