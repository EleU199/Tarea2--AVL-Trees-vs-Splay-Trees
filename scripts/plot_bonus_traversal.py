import argparse
import shutil
from pathlib import Path
from typing import Optional

import matplotlib.pyplot as plt
import pandas as pd


def ensure_output_dir(path: Path) -> None:
    path.mkdir(parents=True, exist_ok=True)


def clean_dir(path: Path) -> None:
    if path.exists():
        shutil.rmtree(path)
    path.mkdir(parents=True, exist_ok=True)


def load_csv_if_exists(path: Path) -> Optional[pd.DataFrame]:
    if not path.exists():
        print(f"[bonus-plot] No existe: {path}")
        return None

    return pd.read_csv(path)


def validate_columns(df: pd.DataFrame, required_columns: set, csv_name: str) -> None:
    missing = required_columns - set(df.columns)

    if missing:
        raise ValueError(f"Faltan columnas en {csv_name}: {sorted(missing)}")


def save_plot(path: Path) -> None:
    plt.tight_layout()
    plt.savefig(path, dpi=200)
    plt.close()


def plot_bonus_blocks(blocks_csv: Path, output_dir: Path, rolling_window: int) -> None:
    df = load_csv_if_exists(blocks_csv)

    if df is None:
        return

    required_columns = {
        "block_index",
        "block_start",
        "block_size",
        "found_count",
        "block_time_ns",
        "block_time_ms",
        "avg_search_ns",
    }

    validate_columns(df, required_columns, str(blocks_csv))

    # =========================
    # 1. Tiempo promedio por búsqueda en cada bloque
    # =========================

    plt.figure(figsize=(12, 5))

    plt.plot(
        df["block_index"],
        df["avg_search_ns"],
        linewidth=0.8,
        label="Promedio por bloque",
    )

    plt.title(
        "Bonus — Traversal Conjecture\n"
        "Tiempos de búsqueda — promedio por búsqueda en cada bloque"
    )
    plt.xlabel("Bloque de búsqueda")
    plt.ylabel("Tiempo promedio por búsqueda en el bloque (ns)")
    plt.grid(True, alpha=0.3)
    plt.legend()

    save_plot(output_dir / "bonus_busqueda_promedio_por_bloque.png")

    # =========================
    # 2. Promedio móvil
    # =========================

    window = max(1, rolling_window)

    df = df.copy()
    df["rolling_avg_search_ns"] = (
        df["avg_search_ns"]
        .rolling(window=window, min_periods=1)
        .mean()
    )

    plt.figure(figsize=(12, 5))

    plt.plot(
        df["block_index"],
        df["rolling_avg_search_ns"],
        linewidth=1.2,
        label=f"Promedio móvil ({window} bloques)",
    )

    plt.title(
        "Bonus — Traversal Conjecture\n"
        "Tiempos de búsqueda — promedio móvil por bloque"
    )
    plt.xlabel("Bloque de búsqueda")
    plt.ylabel("Tiempo promedio por búsqueda (ns)")
    plt.grid(True, alpha=0.3)
    plt.legend()

    save_plot(output_dir / "bonus_busqueda_promedio_movil.png")

    # =========================
    # 3. Tiempo total por bloque
    # =========================

    plt.figure(figsize=(12, 5))

    plt.plot(
        df["block_index"],
        df["block_time_ms"],
        linewidth=0.8,
        label="Tiempo por bloque",
    )

    plt.title(
        "Bonus — Traversal Conjecture\n"
        "Tiempos de búsqueda — tiempo total por bloque"
    )
    plt.xlabel("Bloque de búsqueda")
    plt.ylabel("Tiempo total del bloque (ms)")
    plt.grid(True, alpha=0.3)
    plt.legend()

    save_plot(output_dir / "bonus_busqueda_total_por_bloque.png")

    # =========================
    # 4. Costo acumulado
    # =========================

    df["cumulative_time_ms"] = df["block_time_ms"].cumsum()

    plt.figure(figsize=(12, 5))

    plt.plot(
        df["block_start"],
        df["cumulative_time_ms"],
        linewidth=1.2,
        label="Tiempo acumulado",
    )

    plt.title(
        "Bonus — Traversal Conjecture\n"
        "Tiempos de búsqueda — costo acumulado"
    )
    plt.xlabel("Búsquedas realizadas")
    plt.ylabel("Tiempo acumulado (ms)")
    plt.grid(True, alpha=0.3)
    plt.legend()

    save_plot(output_dir / "bonus_busqueda_tiempo_acumulado.png")

    print(f"[bonus-plot] Gráficos por bloque guardados en: {output_dir}")


def plot_bonus_summary(summary_csv: Path, output_dir: Path) -> None:
    df = load_csv_if_exists(summary_csv)

    if df is None:
        return

    required_columns = {
        "n",
        "block_size",
        "build_t1_time_ms",
        "build_t2_time_ms",
        "traversal_time_ms",
        "search_time_ms",
        "avg_search_ns",
    }

    validate_columns(df, required_columns, str(summary_csv))

    row = df.iloc[0]

    # =========================
    # 1. Resumen de tiempos principales
    # =========================

    labels = [
        "Construcción T1",
        "Construcción T2",
        "Recorrido T1",
        "Búsqueda en T2",
    ]

    values = [
        row["build_t1_time_ms"],
        row["build_t2_time_ms"],
        row["traversal_time_ms"],
        row["search_time_ms"],
    ]

    plt.figure(figsize=(9, 5))

    plt.bar(labels, values)

    plt.title(
        "Bonus — Traversal Conjecture\n"
        "Resumen de tiempos principales"
    )
    plt.xlabel("Etapa")
    plt.ylabel("Tiempo (ms)")
    plt.grid(True, axis="y", alpha=0.3)

    save_plot(output_dir / "bonus_resumen_tiempos_principales.png")

    # =========================
    # 2. Texto resumen en consola
    # =========================

    print("[bonus-plot] Resumen:")
    print(f"  N: {int(row['n'])}")
    print(f"  Tamaño de bloque: {int(row['block_size'])}")
    print(f"  Tiempo construcción T1 (ms): {row['build_t1_time_ms']:.3f}")
    print(f"  Tiempo construcción T2 (ms): {row['build_t2_time_ms']:.3f}")
    print(f"  Tiempo recorrido T1 (ms): {row['traversal_time_ms']:.3f}")
    print(f"  Tiempo búsqueda en T2 (ms): {row['search_time_ms']:.3f}")
    print(f"  Tiempo promedio por búsqueda (ns): {row['avg_search_ns']:.3f}")


def main() -> None:
    parser = argparse.ArgumentParser(
        description="Genera gráficos separados para el bonus Traversal Conjecture."
    )

    parser.add_argument(
        "--summary-csv",
        default="results/bonus_traversal_summary.csv",
        help="Ruta al CSV resumen del bonus.",
    )

    parser.add_argument(
        "--blocks-csv",
        default="results/bonus_traversal_blocks.csv",
        help="Ruta al CSV de bloques del bonus.",
    )

    parser.add_argument(
        "--out",
        default="results/plots/bonus_traversal",
        help="Carpeta donde se guardarán los gráficos del bonus.",
    )

    parser.add_argument(
        "--rolling",
        type=int,
        default=100,
        help="Ventana del promedio móvil en cantidad de bloques.",
    )

    parser.add_argument(
        "--clean",
        action="store_true",
        help="Borra gráficos anteriores del bonus antes de generar los nuevos.",
    )

    args = parser.parse_args()

    output_dir = Path(args.out)

    if args.clean:
        clean_dir(output_dir)
    else:
        ensure_output_dir(output_dir)

    plot_bonus_summary(
        summary_csv=Path(args.summary_csv),
        output_dir=output_dir,
    )

    plot_bonus_blocks(
        blocks_csv=Path(args.blocks_csv),
        output_dir=output_dir,
        rolling_window=args.rolling,
    )

    print("[bonus-plot] Proceso terminado.")
    print(f"[bonus-plot] Gráficos guardados en: {output_dir}")


if __name__ == "__main__":
    main()