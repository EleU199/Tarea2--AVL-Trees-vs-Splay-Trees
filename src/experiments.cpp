#include "experiments.hpp"

#include "avl_tree.hpp"
#include "dataset.hpp"
#include "splay_tree.hpp"
#include "timer.hpp"

#include <random>
#include <unordered_set>
#include <algorithm>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <stdexcept>

namespace experiments {

namespace {
double ns_to_ms(std::uint64_t ns);
double average_ns(std::uint64_t total_ns, std::size_t count);
template <typename Tree>
BaseResult run_base_for_tree(
    TreeType tree_type,
    BaseScenario scenario,
    const std::vector<Key>& insertion_keys,
    const std::vector<Key>& search_keys,
    std::size_t n,
    std::size_t m,
    std::vector<BaseSearchBlockResult>& block_results
) {
    Tree tree(insertion_keys.size());

    std::size_t inserted_unique = 0;

    timer::Timer insert_timer;

    for (Key key : insertion_keys) {
        if (tree.insert(key)) {
            ++inserted_unique;
        }
    }

    std::uint64_t insert_time_ns = insert_timer.elapsed_ns();

    std::size_t total_found_count = 0;
    std::uint64_t total_search_time_ns = 0;

    std::size_t block_index = 0;
    std::size_t block_start = 0;

    while (block_start < search_keys.size()) {
        std::size_t remaining = search_keys.size() - block_start;
        std::size_t current_block_size = std::min(SEARCH_BLOCK_SIZE, remaining);

        std::size_t block_found_count = 0;

        timer::Timer block_timer;

        for (std::size_t i = 0; i < current_block_size; ++i) {
            Key key = search_keys[block_start + i];

            if (tree.search(key)) {
                ++block_found_count;
            }
        }

        std::uint64_t block_time_ns = block_timer.elapsed_ns();

        total_found_count += block_found_count;
        total_search_time_ns += block_time_ns;

        block_results.push_back(
            BaseSearchBlockResult{
                scenario,
                tree_type,
                n,
                m,
                block_index,
                block_start,
                current_block_size,
                block_found_count,
                block_time_ns
            }
        );

        block_start += current_block_size;
        ++block_index;
    }

    return BaseResult{
        scenario,
        tree_type,
        n,
        m,
        C,
        LAMBDA,
        inserted_unique,
        total_found_count,
        insert_time_ns,
        total_search_time_ns
    };
}
void append_base_scenario_results(
    std::vector<BaseResult>& results,
    std::vector<BaseSearchBlockResult>& block_results,
    BaseScenario scenario,
    const std::vector<Key>& insertion_keys,
    const std::vector<Key>& search_keys,
    std::size_t n,
    std::size_t m
) {
    results.push_back(
        run_base_for_tree<AVLTree>(
            TreeType::AVL,
            scenario,
            insertion_keys,
            search_keys,
            n,
            m,
            block_results
        )
    );

    results.push_back(
        run_base_for_tree<SplayTree>(
            TreeType::Splay,
            scenario,
            insertion_keys,
            search_keys,
            n,
            m,
            block_results
        )
    );
}

std::vector<Key> make_random_working_set(
    const std::vector<Key>& keys,
    std::size_t w,
    std::uint64_t seed
) {
    if (w == 0) {
        throw std::runtime_error("Working set W no puede ser 0");
    }

    if (w > keys.size()) {
        throw std::runtime_error("Working set W no puede ser mayor que N");
    }

    dataset::RNG rng = dataset::make_rng(seed);
    std::uniform_int_distribution<std::size_t> dist(0, keys.size() - 1);

    std::unordered_set<std::size_t> selected_indices;
    selected_indices.reserve(w * 2);

    std::vector<Key> working_set;
    working_set.reserve(w);

    while (working_set.size() < w) {
        std::size_t idx = dist(rng);

        if (selected_indices.insert(idx).second) {
            working_set.push_back(keys[idx]);
        }
    }

    return working_set;
}

template <typename Tree>
WorkingSetResult run_working_set_for_tree(
    TreeType tree_type,
    const std::vector<Key>& insertion_keys,
    const std::vector<Key>& working_set,
    std::size_t n,
    std::size_t w,
    std::size_t m
) {
    Tree tree(insertion_keys.size());

    std::size_t inserted_unique = 0;

    std::cout << "[working_set] Construyendo "
              << to_string(tree_type)
              << " con N = "
              << n
              << ", W = "
              << w
              << '\n';

    timer::Timer build_timer;

    for (Key key : insertion_keys) {
        if (tree.insert(key)) {
            ++inserted_unique;
        }
    }

    std::uint64_t build_time_ns = build_timer.elapsed_ns();

    std::cout << "[working_set] Buscando en "
              << to_string(tree_type)
              << " con W = "
              << w
              << ", M = "
              << m
              << '\n';

    dataset::RNG search_rng =
        dataset::make_rng(SEED + 7000 + static_cast<std::uint64_t>(w));

    std::uniform_int_distribution<std::size_t> dist(0, working_set.size() - 1);

    std::size_t found_count = 0;

    timer::Timer search_timer;

    for (std::size_t i = 0; i < m; ++i) {
        Key key = working_set[dist(search_rng)];

        if (tree.search(key)) {
            ++found_count;
        }
    }

    std::uint64_t search_time_ns = search_timer.elapsed_ns();

    return WorkingSetResult{
        tree_type,
        n,
        w,
        m,
        C,
        inserted_unique,
        found_count,
        build_time_ns,
        search_time_ns
    };
}
std::vector<Key> make_strictly_increasing_sequence(
    const std::vector<Key>& keys,
    std::size_t length
) {
    std::vector<Key> sorted_keys = keys;

    std::sort(sorted_keys.begin(), sorted_keys.end());

    sorted_keys.erase(
        std::unique(sorted_keys.begin(), sorted_keys.end()),
        sorted_keys.end()
    );

    if (sorted_keys.size() < length) {
        throw std::runtime_error(
            "No hay suficientes claves unicas para generar una secuencia estrictamente creciente"
        );
    }

    std::vector<Key> sequence;
    sequence.reserve(length);

    std::size_t step = sorted_keys.size() / length;

    if (step == 0) {
        step = 1;
    }

    for (std::size_t i = 0; i < length; ++i) {
        std::size_t idx = i * step;

        if (idx >= sorted_keys.size()) {
            idx = sorted_keys.size() - 1;
        }

        sequence.push_back(sorted_keys[idx]);
    }

    return sequence;
}

template <typename Tree>
std::vector<SequentialResult> run_sequential_for_tree(
    TreeType tree_type,
    const std::vector<Key>& insertion_keys,
    const std::vector<Key>& search_sequence,
    const std::vector<std::size_t>& checkpoints,
    std::size_t n
) {
    std::vector<SequentialResult> results;
    results.reserve(checkpoints.size());

    Tree tree(insertion_keys.size());

    std::size_t inserted_unique = 0;

    std::cout << "[sequential] Construyendo "
              << to_string(tree_type)
              << " con N = "
              << n
              << '\n';

    timer::Timer build_timer;

    for (Key key : insertion_keys) {
        if (tree.insert(key)) {
            ++inserted_unique;
        }
    }

    std::uint64_t build_time_ns = build_timer.elapsed_ns();

    std::cout << "[sequential] Buscando secuencia creciente en "
              << to_string(tree_type)
              << '\n';

    std::size_t found_count = 0;
    std::size_t checkpoint_index = 0;

    timer::Timer search_timer;

    for (std::size_t i = 0; i < search_sequence.size(); ++i) {
        if (tree.search(search_sequence[i])) {
            ++found_count;
        }

        std::size_t current_m = i + 1;

        if (
            checkpoint_index < checkpoints.size() &&
            current_m == checkpoints[checkpoint_index]
        ) {
            std::uint64_t search_time_ns = search_timer.elapsed_ns();

            results.push_back(
                SequentialResult{
                    tree_type,
                    n,
                    current_m,
                    inserted_unique,
                    found_count,
                    build_time_ns,
                    search_time_ns
                }
            );

            std::cout << "[sequential] "
                      << to_string(tree_type)
                      << " m = "
                      << current_m
                      << " listo\n";

            ++checkpoint_index;
        }
    }

    return results;
}

double average_ns(std::uint64_t total_ns, std::size_t count) {
    if (count == 0) {
        return 0.0;
    }

    return static_cast<double>(total_ns) / static_cast<double>(count);
}

double ns_to_ms(std::uint64_t ns) {
    return static_cast<double>(ns) / 1'000'000.0;
}

} // namespace

std::string to_string(TreeType tree) {
    switch (tree) {
        case TreeType::AVL:
            return "AVL";
        case TreeType::Splay:
            return "Splay";
    }

    return "Unknown";
}

std::string to_string(BaseScenario scenario) {
    switch (scenario) {
        case BaseScenario::RandomInsertUniformSearch:
            return "random_insert_uniform_search";
        case BaseScenario::RandomInsertBiasedSearch:
            return "random_insert_biased_search";
        case BaseScenario::SortedInsertUniformSearch:
            return "sorted_insert_uniform_search";
        case BaseScenario::SortedInsertBiasedSearch:
            return "sorted_insert_biased_search";
    }

    return "unknown_scenario";
}

std::vector<std::size_t> base_n_values() {
    return {
        std::size_t{1} << 10,
        std::size_t{1} << 11,
        std::size_t{1} << 12,
        std::size_t{1} << 13,
        std::size_t{1} << 14
    };
}

std::vector<std::size_t> sequential_m_values(std::size_t n) {
    std::vector<std::size_t> values;
    values.reserve(10);

    for (std::size_t k = 1; k <= 10; ++k) {
        values.push_back((k * n) / 100);
    }

    return values;
}

std::vector<std::size_t> working_set_values() {
    return {
        10,
        100,
        1'000,
        10'000,
        100'000,
        1'000'000
    };
}

std::vector<BaseResult> run_base_experiments() {
    return run_base_experiments_with_blocks().summary_results;
}

std::vector<SequentialResult> run_sequential_access_experiment() {
    const std::size_t n = BIG_N;

    std::cout << "[sequential] Generando dataset grande N = "
              << n
              << '\n';

    dataset::RNG rng = dataset::make_rng(SEED + 1000);

    std::vector<Key> keys = dataset::generate_uniform_keys(n, rng);

    std::vector<std::size_t> checkpoints = sequential_m_values(n);

    if (checkpoints.empty()) {
        throw std::runtime_error("No hay valores de m para Sequential Access");
    }

    std::size_t max_m = checkpoints.back();

    std::cout << "[sequential] Generando secuencia estrictamente creciente max_m = "
              << max_m
              << '\n';

    std::vector<Key> increasing_sequence =
        make_strictly_increasing_sequence(keys, max_m);

    std::vector<SequentialResult> results;
    results.reserve(checkpoints.size() * 2);

    std::vector<SequentialResult> avl_results =
        run_sequential_for_tree<AVLTree>(
            TreeType::AVL,
            keys,
            increasing_sequence,
            checkpoints,
            n
        );

    results.insert(
        results.end(),
        avl_results.begin(),
        avl_results.end()
    );

    std::vector<SequentialResult> splay_results =
        run_sequential_for_tree<SplayTree>(
            TreeType::Splay,
            keys,
            increasing_sequence,
            checkpoints,
            n
        );

    results.insert(
        results.end(),
        splay_results.begin(),
        splay_results.end()
    );

    return results;
}

std::vector<WorkingSetResult> run_working_set_experiment() {
    const std::size_t n = BIG_N;
    const std::size_t m = 10 * static_cast<std::size_t>(C) * n;

    std::cout << "[working_set] Generando dataset grande N = "
              << n
              << '\n';

    dataset::RNG rng = dataset::make_rng(SEED + 2000);

    std::vector<Key> keys = dataset::generate_uniform_keys(n, rng);

    std::vector<std::size_t> w_values = working_set_values();

    std::vector<WorkingSetResult> results;
    results.reserve(w_values.size() * 2);

    for (std::size_t w : w_values) {
        std::cout << "[working_set] Generando working set W = "
                  << w
                  << '\n';

        std::vector<Key> working_set =
            make_random_working_set(
                keys,
                w,
                SEED + 3000 + static_cast<std::uint64_t>(w)
            );

        WorkingSetResult avl_result =
            run_working_set_for_tree<AVLTree>(
                TreeType::AVL,
                keys,
                working_set,
                n,
                w,
                m
            );

        results.push_back(avl_result);

        WorkingSetResult splay_result =
            run_working_set_for_tree<SplayTree>(
                TreeType::Splay,
                keys,
                working_set,
                n,
                w,
                m
            );

        results.push_back(splay_result);
    }

    return results;
}

void write_base_results_csv(
    const std::vector<BaseResult>& results,
    const std::string& output_path
) {
    std::ofstream out(output_path);

    if (!out) {
        throw std::runtime_error("No se pudo abrir archivo CSV: " + output_path);
    }

    out << "scenario,"
        << "tree,"
        << "n,"
        << "m,"
        << "c,"
        << "lambda,"
        << "inserted_unique,"
        << "found_count,"
        << "insert_time_ns,"
        << "search_time_ns,"
        << "insert_time_ms,"
        << "search_time_ms,"
        << "avg_insert_ns,"
        << "avg_search_ns\n";

    out << std::fixed << std::setprecision(6);

    for (const BaseResult& result : results) {
        out << to_string(result.scenario) << ','
            << to_string(result.tree) << ','
            << result.n << ','
            << result.m << ','
            << result.c << ','
            << result.lambda << ','
            << result.inserted_unique << ','
            << result.found_count << ','
            << result.insert_time_ns << ','
            << result.search_time_ns << ','
            << ns_to_ms(result.insert_time_ns) << ','
            << ns_to_ms(result.search_time_ns) << ','
            << average_ns(result.insert_time_ns, result.n) << ','
            << average_ns(result.search_time_ns, result.m) << '\n';
    }
}

void write_sequential_results_csv(
    const std::vector<SequentialResult>& results,
    const std::string& output_path
) {
    std::ofstream out(output_path);

    if (!out) {
        throw std::runtime_error("No se pudo abrir archivo CSV: " + output_path);
    }

    out << "tree,n,m,inserted_unique,found_count,build_time_ns,search_time_ns\n";

    for (const SequentialResult& result : results) {
        out << to_string(result.tree) << ','
            << result.n << ','
            << result.m << ','
            << result.inserted_unique << ','
            << result.found_count << ','
            << result.build_time_ns << ','
            << result.search_time_ns << '\n';
    }
}


void write_working_set_results_csv(
    const std::vector<WorkingSetResult>& results,
    const std::string& output_path
) {
    std::ofstream out(output_path);

    if (!out) {
        throw std::runtime_error("No se pudo abrir archivo CSV: " + output_path);
    }

    out << "tree,n,w,m,c,inserted_unique,found_count,build_time_ns,search_time_ns\n";

    for (const WorkingSetResult& result : results) {
        out << to_string(result.tree) << ','
            << result.n << ','
            << result.w << ','
            << result.m << ','
            << result.c << ','
            << result.inserted_unique << ','
            << result.found_count << ','
            << result.build_time_ns << ','
            << result.search_time_ns << '\n';
    }
}

BaseExperimentOutput run_base_experiments_with_blocks() {
    BaseExperimentOutput output;

    auto n_values = base_n_values();
    output.summary_results.reserve(n_values.size() * 4 * 2);

    dataset::RNG dataset_rng = dataset::make_rng(SEED);

    for (std::size_t n : n_values) {
        std::cout << "[base] Generando dataset N = " << n << '\n';

        std::vector<Key> random_keys = dataset::generate_uniform_keys(n, dataset_rng);
        std::vector<Key> sorted_keys = dataset::sorted_copy(random_keys);

        const std::size_t m = 10 * static_cast<std::size_t>(C) * n;

        dataset::RNG uniform_rng = dataset::make_rng(SEED + static_cast<std::uint64_t>(n) + 1);
        dataset::RNG biased_rng = dataset::make_rng(SEED + static_cast<std::uint64_t>(n) + 2);

        std::vector<Key> uniform_searches =
            dataset::make_uniform_search_sequence(random_keys, m, uniform_rng);

        std::vector<Key> biased_searches =
            dataset::make_biased_search_sequence(sorted_keys, m, LAMBDA, biased_rng);

        std::cout << "[base] Escenario a: insercion aleatoria, busqueda uniforme\n";
        append_base_scenario_results(
            output.summary_results,
            output.block_results,
            BaseScenario::RandomInsertUniformSearch,
            random_keys,
            uniform_searches,
            n,
            m
        );

        std::cout << "[base] Escenario b: insercion aleatoria, busqueda sesgada\n";
        append_base_scenario_results(
            output.summary_results,
            output.block_results,
            BaseScenario::RandomInsertBiasedSearch,
            random_keys,
            biased_searches,
            n,
            m
        );

        std::cout << "[base] Escenario c: insercion ordenada, busqueda uniforme\n";
        append_base_scenario_results(
            output.summary_results,
            output.block_results,
            BaseScenario::SortedInsertUniformSearch,
            sorted_keys,
            uniform_searches,
            n,
            m
        );

        std::cout << "[base] Escenario d: insercion ordenada, busqueda sesgada\n";
        append_base_scenario_results(
            output.summary_results,
            output.block_results,
            BaseScenario::SortedInsertBiasedSearch,
            sorted_keys,
            biased_searches,
            n,
            m
        );
    }

    return output;
}

void write_base_search_block_results_csv(
    const std::vector<BaseSearchBlockResult>& results,
    const std::string& output_path
) {
    std::ofstream out(output_path);

    if (!out) {
        throw std::runtime_error("No se pudo abrir archivo CSV: " + output_path);
    }

    out << "scenario,"
        << "tree,"
        << "n,"
        << "m,"
        << "block_index,"
        << "block_start,"
        << "block_size,"
        << "found_count,"
        << "block_time_ns,"
        << "block_time_ms,"
        << "avg_search_ns\n";

    out << std::fixed << std::setprecision(6);

    for (const BaseSearchBlockResult& result : results) {
        out << to_string(result.scenario) << ','
            << to_string(result.tree) << ','
            << result.n << ','
            << result.m << ','
            << result.block_index << ','
            << result.block_start << ','
            << result.block_size << ','
            << result.found_count << ','
            << result.block_time_ns << ','
            << ns_to_ms(result.block_time_ns) << ','
            << average_ns(result.block_time_ns, result.block_size) << '\n';
    }
}

void run_all_experiments(const std::string& results_dir) {
    std::filesystem::create_directories(results_dir);

    std::filesystem::path base_output =
        std::filesystem::path(results_dir) / "base_results.csv";

    std::filesystem::path block_output =
        std::filesystem::path(results_dir) / "base_search_blocks.csv";

    std::filesystem::path sequential_output =
        std::filesystem::path(results_dir) / "sequential_results.csv";

    std::filesystem::path working_output =
        std::filesystem::path(results_dir) / "working_set_results.csv";

    std::cout << "[base] Ejecutando experimentos base...\n";

    BaseExperimentOutput base_output_data = run_base_experiments_with_blocks();

    write_base_results_csv(base_output_data.summary_results, base_output.string());
    write_base_search_block_results_csv(base_output_data.block_results, block_output.string());

    std::cout << "[base] Resultados agregados guardados en: "
              << base_output.string()
              << '\n';

    std::cout << "[base] Resultados por bloque guardados en: "
              << block_output.string()
              << '\n';

    std::cout << "[sequential] Ejecutando experimento Sequential Access...\n";

    std::vector<SequentialResult> sequential_results =
        run_sequential_access_experiment();

    write_sequential_results_csv(
        sequential_results,
        sequential_output.string()
    );

    std::cout << "[sequential] Resultados guardados en: "
              << sequential_output.string()
              << '\n';

    std::cout << "[working_set] Ejecutando experimento Working Set...\n";

    std::vector<WorkingSetResult> working_results =
        run_working_set_experiment();

    write_working_set_results_csv(
        working_results,
        working_output.string()
    );

    std::cout << "[working_set] Resultados guardados en: "
              << working_output.string()
              << '\n';
}

} // namespace experiments