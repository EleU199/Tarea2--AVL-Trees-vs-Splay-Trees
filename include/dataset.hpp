#pragma once

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <cmath>
#include <limits>
#include <random>
#include <stdexcept>
#include <vector>

/**
 * @brief Utilidades para generar datasets y secuencias de búsqueda.
 *
 * Este archivo no depende de AVLTree ni de SplayTree.
 * Solo genera claves y secuencias para los experimentos.
 */
namespace dataset {

using Key = std::uint32_t;
using RNG = std::mt19937_64;

/**
 * @brief Semilla por defecto para reproducibilidad.
 */
constexpr std::uint64_t DEFAULT_SEED = 4102;

/**
 * @brief Crea un generador pseudoaleatorio reproducible.
 *
 * @param seed Semilla inicial.
 * @return Generador mt19937_64.
 */
RNG make_rng(std::uint64_t seed = DEFAULT_SEED);

/**
 * @brief Genera n claves uniformemente distribuidas en [0, 2^32 - 1].
 *
 * @param n Cantidad de claves.
 * @param rng Generador pseudoaleatorio.
 * @return Vector con n claves.
 */
std::vector<Key> generate_uniform_keys(std::size_t n, RNG& rng);

/**
 * @brief Retorna una copia ordenada de un vector de claves.
 *
 * @param keys Vector original.
 * @return Copia ordenada de menor a mayor.
 */
std::vector<Key> sorted_copy(std::vector<Key> keys);

/**
 * @brief Muestrea un índice uniforme en [0, n - 1].
 *
 * @param n Tamaño del universo.
 * @param rng Generador pseudoaleatorio.
 * @return Índice escogido.
 */
std::size_t sample_uniform_index(std::size_t n, RNG& rng);

/**
 * @brief Muestrea un índice usando distribución exponencial discreta truncada.
 *
 * Usa una distribución proporcional a:
 *
 * P(i) = exp(-lambda * i), para i en {0, ..., n - 1}
 *
 * normalizada sobre n elementos.
 *
 * @param n Tamaño del universo.
 * @param lambda Parámetro de sesgo. Recomendado: [0.001, 0.05].
 * @param rng Generador pseudoaleatorio.
 * @return Índice escogido.
 */
std::size_t sample_biased_index(std::size_t n, double lambda, RNG& rng);

/**
 * @brief Genera una secuencia de búsquedas uniformes desde un dataset.
 *
 * Cada elemento de la secuencia se obtiene escogiendo un índice uniforme
 * del vector keys.
 *
 * @param keys Dataset base.
 * @param m Largo de la secuencia.
 * @param rng Generador pseudoaleatorio.
 * @return Secuencia de claves a buscar.
 */
std::vector<Key> make_uniform_search_sequence(
    const std::vector<Key>& keys,
    std::size_t m,
    RNG& rng
);

/**
 * @brief Genera una secuencia de búsquedas sesgadas desde un dataset.
 *
 * Cada elemento de la secuencia se obtiene escogiendo un índice según
 * una distribución exponencial discreta truncada.
 *
 * @param keys Dataset base.
 * @param m Largo de la secuencia.
 * @param lambda Parámetro de sesgo.
 * @param rng Generador pseudoaleatorio.
 * @return Secuencia de claves a buscar.
 */
std::vector<Key> make_biased_search_sequence(
    const std::vector<Key>& keys,
    std::size_t m,
    double lambda,
    RNG& rng
);

} // namespace dataset

// =====================
// Implementación inline
// =====================

namespace dataset {

inline RNG make_rng(std::uint64_t seed) {
    return RNG(seed);
}

inline std::vector<Key> generate_uniform_keys(std::size_t n, RNG& rng) {
    std::vector<Key> keys;
    keys.reserve(n);

    std::uniform_int_distribution<Key> dist(
        std::numeric_limits<Key>::min(),
        std::numeric_limits<Key>::max()
    );

    for (std::size_t i = 0; i < n; ++i) {
        keys.push_back(dist(rng));
    }

    return keys;
}

inline std::vector<Key> sorted_copy(std::vector<Key> keys) {
    std::sort(keys.begin(), keys.end());
    return keys;
}

inline std::size_t sample_uniform_index(std::size_t n, RNG& rng) {
    if (n == 0) {
        throw std::invalid_argument("sample_uniform_index: n no puede ser 0");
    }

    std::uniform_int_distribution<std::size_t> dist(0, n - 1);
    return dist(rng);
}

inline std::size_t sample_biased_index(std::size_t n, double lambda, RNG& rng) {
    if (n == 0) {
        throw std::invalid_argument("sample_biased_index: n no puede ser 0");
    }

    if (lambda <= 0.0) {
        return sample_uniform_index(n, rng);
    }

    std::uniform_real_distribution<double> dist(0.0, 1.0);
    double u = dist(rng);

    // Distribución discreta truncada sobre i = 0, ..., n - 1:
    //
    // P(i) = exp(-lambda * i) * (1 - exp(-lambda)) / (1 - exp(-lambda * n))
    //
    // Se muestrea por inversa acumulada.
    double denominator = 1.0 - std::exp(-lambda * static_cast<double>(n));
    double value = 1.0 - u * denominator;

    std::size_t idx = static_cast<std::size_t>(
        std::floor(-std::log(value) / lambda)
    );

    if (idx >= n) {
        idx = n - 1;
    }

    return idx;
}

inline std::vector<Key> make_uniform_search_sequence(
    const std::vector<Key>& keys,
    std::size_t m,
    RNG& rng
) {
    if (keys.empty()) {
        throw std::invalid_argument("make_uniform_search_sequence: keys está vacío");
    }

    std::vector<Key> sequence;
    sequence.reserve(m);

    for (std::size_t i = 0; i < m; ++i) {
        std::size_t idx = sample_uniform_index(keys.size(), rng);
        sequence.push_back(keys[idx]);
    }

    return sequence;
}

inline std::vector<Key> make_biased_search_sequence(
    const std::vector<Key>& keys,
    std::size_t m,
    double lambda,
    RNG& rng
) {
    if (keys.empty()) {
        throw std::invalid_argument("make_biased_search_sequence: keys está vacío");
    }

    std::vector<Key> sequence;
    sequence.reserve(m);

    for (std::size_t i = 0; i < m; ++i) {
        std::size_t idx = sample_biased_index(keys.size(), lambda, rng);
        sequence.push_back(keys[idx]);
    }

    return sequence;
}

} // namespace dataset