#include "experiments.hpp"
#include "bonus_experiments.hpp"

#include <exception>
#include <iostream>
#include <string>

int main(int argc, char* argv[]) {
    std::string results_dir = "results";

    if (argc >= 2) {
        results_dir = argv[1];
    }

    try {
        /*
         * Experimentos principales:
         * - Escenarios base
         * - Sequential Access Theorem
         * - Working Set Theorem
         */
        experiments::run_all_experiments(results_dir);

        /*
         * Experimento bonus:
         * - Traversal Conjecture
         *
         * Si quieres ejecutar solo los experimentos principales,
         * comenta esta línea.
         *
         * Si quieres ejecutar solo el bonus,
         * comenta la llamada a experiments::run_all_experiments(...).
         */
        bonus::run_and_write_traversal_bonus(results_dir);

    } catch (const std::exception& error) {
        std::cerr << "Error: " << error.what() << '\n';
        return 1;
    }

    return 0;
}