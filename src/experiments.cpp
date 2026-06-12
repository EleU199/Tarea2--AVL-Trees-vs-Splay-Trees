#include "experiments.hpp"

#include "avl_tree.hpp"
#include "dataset.hpp"
#include "splay_tree.hpp"
#include "timer.hpp"

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
    return {};
}

std::vector<WorkingSetResult> run_working_set_experiment() {
    return {};
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
}

} // namespace experiments