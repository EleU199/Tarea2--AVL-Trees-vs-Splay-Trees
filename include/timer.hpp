#pragma once

#include <chrono>
#include <cstdint>

/**
 * @brief Utilidades simples para medir tiempos de ejecución.
 *
 * Usa std::chrono::steady_clock, que es apropiado para medir intervalos
 * porque es monotónico.
 */
namespace timer {

using Clock = std::chrono::steady_clock;
using TimePoint = Clock::time_point;

/**
 * @brief Temporizador manual.
 *
 * Uso:
 *
 * Timer t;
 * // código a medir
 * auto ns = t.elapsed_ns();
 */
class Timer {
public:
    /**
     * @brief Crea un temporizador y comienza a medir inmediatamente.
     */
    Timer();

    /**
     * @brief Reinicia el temporizador al tiempo actual.
     */
    void reset();

    /**
     * @brief Retorna el tiempo transcurrido en nanosegundos.
     */
    std::uint64_t elapsed_ns() const;

    /**
     * @brief Retorna el tiempo transcurrido en microsegundos.
     */
    double elapsed_us() const;

    /**
     * @brief Retorna el tiempo transcurrido en milisegundos.
     */
    double elapsed_ms() const;

    /**
     * @brief Retorna el tiempo transcurrido en segundos.
     */
    double elapsed_s() const;

private:
    TimePoint start_;
};

/**
 * @brief Mide el tiempo que demora una función sin argumentos.
 *
 * @param function Función, lambda o callable a medir.
 * @return Tiempo en nanosegundos.
 */
template <typename Function>
std::uint64_t measure_ns(Function&& function);

} // namespace timer
// =====================
// Implementación inline
// =====================

namespace timer {

inline Timer::Timer()
    : start_(Clock::now()) {}

inline void Timer::reset() {
    start_ = Clock::now();
}

inline std::uint64_t Timer::elapsed_ns() const {
    auto end = Clock::now();

    return static_cast<std::uint64_t>(
        std::chrono::duration_cast<std::chrono::nanoseconds>(
            end - start_
        ).count()
    );
}

inline double Timer::elapsed_us() const {
    return static_cast<double>(elapsed_ns()) / 1'000.0;
}

inline double Timer::elapsed_ms() const {
    return static_cast<double>(elapsed_ns()) / 1'000'000.0;
}

inline double Timer::elapsed_s() const {
    return static_cast<double>(elapsed_ns()) / 1'000'000'000.0;
}

template <typename Function>
inline std::uint64_t measure_ns(Function&& function) {
    Timer t;
    function();
    return t.elapsed_ns();
}

} // namespace timer