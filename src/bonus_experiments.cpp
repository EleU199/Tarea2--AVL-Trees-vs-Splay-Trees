#include "bonus_experiments.hpp"

#include "splay_tree.hpp"
#include "timer.hpp"

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <random>
#include <stdexcept>
#include <vector>

namespace bonus {

namespace {

double ns_to_ms(std::uint64_t ns) {
    return static_cast<double>(ns) / 1'000'000.0;
}

double average_ns(std::uint64_t total_ns, std::size_t count) {
    if (count == 0) {
        return 0.0;
    }

    return static_cast<double>(total_ns) / static_cast<double>(count);
}

/**
 * @brief Genera claves únicas en uint32_t sin usar unordered_set.
 *
 * Usa una función lineal módulo 2^32:
 *
 * key_i = A * i + B mod 2^32
 *
 * Como A es impar, la función es una permutación sobre uint32_t.
 */
std::vector<Key> generate_unique_keys(std::size_t n) {
    if (n > (std::size_t{1} << 32)) {
        throw std::runtime_error("No se pueden generar mas de 2^32 claves uint32_t unicas");
    }

    std::vector<Key> keys;
    keys.reserve(n);

    constexpr std::uint64_t A = 2654435761ULL;
    constexpr std::uint64_t B = 1013904223ULL;

    for (std::size_t i = 0; i < n; ++i) {
        std::uint64_t value = A * static_cast<std::uint64_t>(i) + B;
        keys.push_back(static_cast<Key>(value & 0xFFFFFFFFULL));
    }

    return keys;
}

void shuffle_keys(std::vector<Key>& keys, std::uint64_t seed) {
    std::mt19937_64 rng(seed);
    std::shuffle(keys.begin(), keys.end(), rng);
}

std::uint64_t build_tree(
    SplayTree& tree,
    const std::vector<Key>& insertion_order,
    const std::string& tree_name,
    std::size_t& inserted_unique
) {
    std::cout << "[bonus] Construyendo "
              << tree_name
              << " con "
              << insertion_order.size()
              << " claves\n";

    inserted_unique = 0;

    timer::Timer build_timer;

    for (Key key : insertion_order) {
        if (tree.insert(key)) {
            ++inserted_unique;
        }
    }

    std::uint64_t build_time_ns = build_timer.elapsed_ns();

    std::cout << "[bonus] "
              << tree_name
              << " construido. Insertados unicos = "
              << inserted_unique
              << ", tiempo = "
              << ns_to_ms(build_time_ns)
              << " ms\n";

    return build_time_ns;
}

} // namespace

TraversalBonusResult run_traversal_bonus(std::size_t n) {
    std::cout << "[bonus] Generando dataset de "
              << n
              << " claves unicas\n";

    std::vector<Key> keys = generate_unique_keys(n);

    std::vector<Key> permutation_t1 = keys;
    std::vector<Key> permutation_t2 = keys;

    shuffle_keys(permutation_t1, BONUS_SEED + 1);
    shuffle_keys(permutation_t2, BONUS_SEED + 2);

    SplayTree t1(n);
    SplayTree t2(n);

    std::size_t inserted_t1 = 0;
    std::size_t inserted_t2 = 0;

    std::uint64_t build_t1_time_ns =
        build_tree(t1, permutation_t1, "T1", inserted_t1);

    std::uint64_t build_t2_time_ns =
        build_tree(t2, permutation_t2, "T2", inserted_t2);

    if (inserted_t1 != n || inserted_t2 != n) {
        throw std::runtime_error("No se insertaron N claves unicas en ambos Splay Trees");
    }

    std::vector<Key>().swap(keys);
    std::vector<Key>().swap(permutation_t1);
    std::vector<Key>().swap(permutation_t2);

    std::cout << "[bonus] Generando recorrido preorden de T1\n";

    timer::Timer traversal_timer;
    std::vector<Key> sequence = t1.preorder_keys();
    std::uint64_t traversal_time_ns = traversal_timer.elapsed_ns();

    if (sequence.size() != n) {
        throw std::runtime_error("El recorrido preorden no contiene exactamente N claves");
    }

    std::cout << "[bonus] Recorrido generado. Largo = "
              << sequence.size()
              << ", tiempo = "
              << ns_to_ms(traversal_time_ns)
              << " ms\n";

    t1.clear();

    std::cout << "[bonus] Buscando secuencia preorden de T1 en T2\n";

    TraversalBonusResult result;
    result.blocks.reserve((n + BONUS_BLOCK_SIZE - 1) / BONUS_BLOCK_SIZE);

    std::size_t total_found_count = 0;
    std::uint64_t total_search_time_ns = 0;

    std::size_t block_index = 0;
    std::size_t block_start = 0;

    while (block_start < sequence.size()) {
        std::size_t remaining = sequence.size() - block_start;
        std::size_t current_block_size = std::min(BONUS_BLOCK_SIZE, remaining);

        std::size_t block_found_count = 0;

        timer::Timer block_timer;

        for (std::size_t i = 0; i < current_block_size; ++i) {
            Key key = sequence[block_start + i];

            if (t2.search(key)) {
                ++block_found_count;
            }
        }

        std::uint64_t block_time_ns = block_timer.elapsed_ns();

        total_found_count += block_found_count;
        total_search_time_ns += block_time_ns;

        result.blocks.push_back(
            TraversalBonusBlock{
                block_index,
                block_start,
                current_block_size,
                block_found_count,
                block_time_ns
            }
        );

        if (block_index % 1000 == 0) {
            std::cout << "[bonus] Bloque "
                      << block_index
                      << " listo. Progreso = "
                      << block_start + current_block_size
                      << " / "
                      << sequence.size()
                      << '\n';
        }

        block_start += current_block_size;
        ++block_index;
    }

    result.summary = TraversalBonusSummary{
        n,
        BONUS_BLOCK_SIZE,
        build_t1_time_ns,
        build_t2_time_ns,
        traversal_time_ns,
        total_search_time_ns,
        sequence.size(),
        total_found_count
    };

    std::cout << "[bonus] Busqueda terminada. Encontrados = "
              << total_found_count
              << " / "
              << sequence.size()
              << ", tiempo total = "
              << ns_to_ms(total_search_time_ns)
              << " ms\n";

    return result;
}

void write_traversal_bonus_summary_csv(
    const TraversalBonusSummary& summary,
    const std::string& output_path
) {
    std::ofstream out(output_path);

    if (!out) {
        throw std::runtime_error("No se pudo abrir archivo CSV: " + output_path);
    }

    out << "n,"
        << "block_size,"
        << "build_t1_time_ns,"
        << "build_t2_time_ns,"
        << "traversal_time_ns,"
        << "search_time_ns,"
        << "sequence_size,"
        << "found_count,"
        << "build_t1_time_ms,"
        << "build_t2_time_ms,"
        << "traversal_time_ms,"
        << "search_time_ms,"
        << "avg_search_ns\n";

    out << std::fixed << std::setprecision(6);

    out << summary.n << ','
        << summary.block_size << ','
        << summary.build_t1_time_ns << ','
        << summary.build_t2_time_ns << ','
        << summary.traversal_time_ns << ','
        << summary.search_time_ns << ','
        << summary.sequence_size << ','
        << summary.found_count << ','
        << ns_to_ms(summary.build_t1_time_ns) << ','
        << ns_to_ms(summary.build_t2_time_ns) << ','
        << ns_to_ms(summary.traversal_time_ns) << ','
        << ns_to_ms(summary.search_time_ns) << ','
        << average_ns(summary.search_time_ns, summary.sequence_size)
        << '\n';
}

void write_traversal_bonus_blocks_csv(
    const std::vector<TraversalBonusBlock>& blocks,
    const std::string& output_path
) {
    std::ofstream out(output_path);

    if (!out) {
        throw std::runtime_error("No se pudo abrir archivo CSV: " + output_path);
    }

    out << "block_index,"
        << "block_start,"
        << "block_size,"
        << "found_count,"
        << "block_time_ns,"
        << "block_time_ms,"
        << "avg_search_ns\n";

    out << std::fixed << std::setprecision(6);

    for (const TraversalBonusBlock& block : blocks) {
        out << block.block_index << ','
            << block.block_start << ','
            << block.block_size << ','
            << block.found_count << ','
            << block.block_time_ns << ','
            << ns_to_ms(block.block_time_ns) << ','
            << average_ns(block.block_time_ns, block.block_size)
            << '\n';
    }
}

void run_and_write_traversal_bonus(
    const std::string& results_dir,
    std::size_t n
) {
    std::filesystem::create_directories(results_dir);

    std::filesystem::path summary_output =
        std::filesystem::path(results_dir) / "bonus_traversal_summary.csv";

    std::filesystem::path blocks_output =
        std::filesystem::path(results_dir) / "bonus_traversal_blocks.csv";

    std::cout << "[bonus] Ejecutando Traversal Conjecture con N = "
              << n
              << '\n';

    TraversalBonusResult result = run_traversal_bonus(n);

    write_traversal_bonus_summary_csv(
        result.summary,
        summary_output.string()
    );

    write_traversal_bonus_blocks_csv(
        result.blocks,
        blocks_output.string()
    );

    std::cout << "[bonus] Resumen guardado en: "
              << summary_output.string()
              << '\n';

    std::cout << "[bonus] Bloques guardados en: "
              << blocks_output.string()
              << '\n';
}

} // namespace bonus