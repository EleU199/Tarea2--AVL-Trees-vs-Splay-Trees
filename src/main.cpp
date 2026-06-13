#include "experiments.hpp"

#include <exception>
#include <iostream>
#include <string>

int main(int argc, char* argv[]) {
    std::string results_dir = "results";
    if (argc >= 2) {
        results_dir = argv[1];
    }
    try {
        experiments::run_all_experiments(results_dir);
    } catch (const std::exception& error) {
        std::cerr << "Error: " << error.what() << '\n';
        return 1;
    }

    return 0;
}