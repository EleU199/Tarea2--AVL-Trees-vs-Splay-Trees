import argparse
import shutil
from pathlib import Path
from typing import Optional

import matplotlib.pyplot as plt
import pandas as pd


SCENARIO_NAMES = {
    "random_insert_uniform_search": "Inserción aleatoria, búsqueda uniforme",
    "random_insert_biased_search": "Inserción aleatoria, búsqueda sesgada",
    "sorted_insert_uniform_search": "Inserción ordenada, búsqueda uniforme",
    "sorted_insert_biased_search": "Inserción ordenada, búsqueda sesgada",
}

SCENARIO_ORDER = [
    "random_insert_uniform_search",
    "random_insert_biased_search",
    "sorted_insert_uniform_search",
    "sorted_insert_biased_search",
]


def ensure_output_dir(path: Path) -> None:
    path.mkdir(parents=True, exist_ok=True)


def clean_dir(path: Path) -> None:
    if path.exists():
        shutil.rmtree(path)
    path.mkdir(parents=True, exist_ok=True)


def load_csv_if_exists(path: Path) -> Optional[pd.DataFrame]:
    if not path.exists():
        print(f"[plot] No existe: {path}")
        return None
    return pd.read_csv(path)


def validate_columns(df: pd.DataFrame, required_columns: set[str], csv_name: str) -> None:
    missing = required_columns - set(df.columns)
    if missing:
        raise ValueError(f"Faltan columnas en {csv_name}: {sorted(missing)}")


def scenario_title(scenario: str) -> str:
    return SCENARIO_NAMES.get(scenario, scenario)


def sorted_scenarios(df: pd.DataFrame) -> list[str]:
    present = set(df["scenario"].unique())
    ordered = [s for s in SCENARIO_ORDER if s in present]
    extra = sorted(present - set(ordered))
    return ordered + extra


def save_plot(path: Path) -> None:
    plt.tight_layout()
    plt.savefig(path, dpi=200)
    plt.close()


def plot_metric_by_scenario(
    df: pd.DataFrame,
    output_dir: Path,
    metric: str,
    ylabel: str,
    filename_prefix: str,
    title_prefix: str,
) -> None:
    for scenario in sorted_scenarios(df):
        group = df[df["scenario"] == scenario]

        plt.figure(figsize=(9, 5))

        for tree, tree_group in group.groupby("tree"):
            tree_group = tree_group.sort_values("n")
            plt.plot(
                tree_group["n"],
                tree_group[metric],
                marker="o",
                label=tree,
            )

        plt.title(f"{title_prefix}\n{scenario_title(scenario)}")
        plt.xlabel("N")
        plt.ylabel(ylabel)
        plt.xscale("log", base=2)
        plt.grid(True, alpha=0.3)
        plt.legend()

        output_path = output_dir / f"{filename_prefix}_por_escenario__{scenario}.png"
        save_plot(output_path)


def plot_all_scenarios_for_each_tree(
    df: pd.DataFrame,
    output_dir: Path,
    metric: str,
    ylabel: str,
    filename_prefix: str,
    title_prefix: str,
) -> None:
    for tree, tree_group in df.groupby("tree"):
        plt.figure(figsize=(10, 6))

        for scenario in sorted_scenarios(tree_group):
            scenario_group = tree_group[tree_group["scenario"] == scenario].sort_values("n")
            plt.plot(
                scenario_group["n"],
                scenario_group[metric],
                marker="o",
                label=scenario_title(scenario),
            )

        plt.title(f"{title_prefix} — {tree}")
        plt.xlabel("N")
        plt.ylabel(ylabel)
        plt.xscale("log", base=2)
        plt.grid(True, alpha=0.3)
        plt.legend(fontsize=8)

        output_path = output_dir / f"{filename_prefix}_todos_escenarios__{tree.lower()}.png"
        save_plot(output_path)


def plot_base_results(base_csv: Path, search_dir: Path, insert_dir: Path) -> None:
    df = load_csv_if_exists(base_csv)
    if df is None:
        return

    required_columns = {
        "scenario",
        "tree",
        "n",
        "m",
        "insert_time_ms",
        "search_time_ms",
        "avg_insert_ns",
        "avg_search_ns",
    }
    validate_columns(df, required_columns, str(base_csv))

    # Búsqueda
    plot_metric_by_scenario(
        df=df,
        output_dir=search_dir,
        metric="avg_search_ns",
        ylabel="Tiempo promedio por búsqueda (ns)",
        filename_prefix="busqueda_promedio_base",
        title_prefix="Tiempos de búsqueda — promedio por búsqueda",
    )

    plot_metric_by_scenario(
        df=df,
        output_dir=search_dir,
        metric="search_time_ms",
        ylabel="Tiempo total de búsqueda (ms)",
        filename_prefix="busqueda_total_base",
        title_prefix="Tiempos de búsqueda — tiempo total",
    )

    plot_all_scenarios_for_each_tree(
        df=df,
        output_dir=search_dir,
        metric="avg_search_ns",
        ylabel="Tiempo promedio por búsqueda (ns)",
        filename_prefix="busqueda_promedio_base",
        title_prefix="Tiempos de búsqueda — comparación de escenarios",
    )

    # Inserción
    plot_metric_by_scenario(
        df=df,
        output_dir=insert_dir,
        metric="avg_insert_ns",
        ylabel="Tiempo promedio por inserción (ns)",
        filename_prefix="insercion_promedio_base",
        title_prefix="Tiempos de inserción — promedio por inserción",
    )

    plot_metric_by_scenario(
        df=df,
        output_dir=insert_dir,
        metric="insert_time_ms",
        ylabel="Tiempo total de inserción (ms)",
        filename_prefix="insercion_total_base",
        title_prefix="Tiempos de inserción — tiempo total",
    )

    plot_all_scenarios_for_each_tree(
        df=df,
        output_dir=insert_dir,
        metric="avg_insert_ns",
        ylabel="Tiempo promedio por inserción (ns)",
        filename_prefix="insercion_promedio_base",
        title_prefix="Tiempos de inserción — comparación de escenarios",
    )

    print(f"[plot] Gráficos base listos.")


def plot_search_blocks(blocks_csv: Path, search_dir: Path) -> None:
    df = load_csv_if_exists(blocks_csv)
    if df is None:
        return

    required_columns = {
        "scenario",
        "tree",
        "n",
        "m",
        "block_index",
        "block_start",
        "block_size",
        "avg_search_ns",
    }
    validate_columns(df, required_columns, str(blocks_csv))

    for scenario in sorted_scenarios(df):
        scenario_df = df[df["scenario"] == scenario]

        for n in sorted(scenario_df["n"].unique()):
            group = scenario_df[scenario_df["n"] == n]

            plt.figure(figsize=(10, 5))

            for tree, tree_group in group.groupby("tree"):
                tree_group = tree_group.sort_values("block_index")
                plt.plot(
                    tree_group["block_index"],
                    tree_group["avg_search_ns"],
                    label=tree,
                )

            plt.title(
                f"Tiempos de búsqueda — evolución por bloques\n"
                f"{scenario_title(scenario)}, N={n}"
            )
            plt.xlabel("Bloque de búsqueda")
            plt.ylabel("Tiempo promedio por búsqueda en el bloque (ns)")
            plt.grid(True, alpha=0.3)
            plt.legend()

            output_path = search_dir / f"busqueda_bloques__{scenario}__n_{n}.png"
            save_plot(output_path)

    # resumen para N máximo
    largest_n = int(df["n"].max())
    df_largest = df[df["n"] == largest_n]

    for scenario in sorted_scenarios(df_largest):
        group = df_largest[df_largest["scenario"] == scenario]

        plt.figure(figsize=(11, 5))

        for tree, tree_group in group.groupby("tree"):
            tree_group = tree_group.sort_values("block_index")
            plt.plot(
                tree_group["block_index"],
                tree_group["avg_search_ns"],
                label=tree,
            )

        plt.title(
            f"Tiempos de búsqueda — evolución amortizada por bloques\n"
            f"{scenario_title(scenario)}, N máximo={largest_n}"
        )
        plt.xlabel("Bloque de búsqueda")
        plt.ylabel("Tiempo promedio por búsqueda en el bloque (ns)")
        plt.grid(True, alpha=0.3)
        plt.legend()

        output_path = search_dir / f"busqueda_bloques_resumen_n_max__{scenario}.png"
        save_plot(output_path)

    print(f"[plot] Gráficos por bloques listos.")


def plot_sequential_results(sequential_csv: Path, search_dir: Path, insert_dir: Path) -> None:
    df = load_csv_if_exists(sequential_csv)
    if df is None:
        return

    required_columns = {
        "tree",
        "n",
        "m",
        "build_time_ns",
        "search_time_ns",
    }
    validate_columns(df, required_columns, str(sequential_csv))

    df = df.copy()
    df["search_time_ms"] = df["search_time_ns"] / 1_000_000.0
    df["avg_search_ns"] = df["search_time_ns"] / df["m"]
    df["build_time_ms"] = df["build_time_ns"] / 1_000_000.0

    # búsqueda total
    plt.figure(figsize=(9, 5))
    for tree, tree_group in df.groupby("tree"):
        tree_group = tree_group.sort_values("m")
        plt.plot(tree_group["m"], tree_group["search_time_ms"], marker="o", label=tree)

    plt.title("Tiempos de búsqueda — Sequential Access Theorem\nTiempo total de búsqueda")
    plt.xlabel("m")
    plt.ylabel("Tiempo total de búsqueda (ms)")
    plt.grid(True, alpha=0.3)
    plt.legend()
    save_plot(search_dir / "busqueda_sequential_total.png")

    # búsqueda promedio
    plt.figure(figsize=(9, 5))
    for tree, tree_group in df.groupby("tree"):
        tree_group = tree_group.sort_values("m")
        plt.plot(tree_group["m"], tree_group["avg_search_ns"], marker="o", label=tree)

    plt.title("Tiempos de búsqueda — Sequential Access Theorem\nPromedio por búsqueda")
    plt.xlabel("m")
    plt.ylabel("Tiempo promedio por búsqueda (ns)")
    plt.grid(True, alpha=0.3)
    plt.legend()
    save_plot(search_dir / "busqueda_sequential_promedio.png")

    # construcción
    plt.figure(figsize=(9, 5))
    for tree, tree_group in df.groupby("tree"):
        tree_group = tree_group.sort_values("m")
        plt.plot(tree_group["m"], tree_group["build_time_ms"], marker="o", label=tree)

    plt.title("Tiempos de inserción — Sequential Access Theorem\nTiempo de construcción del árbol")
    plt.xlabel("m")
    plt.ylabel("Tiempo de construcción (ms)")
    plt.grid(True, alpha=0.3)
    plt.legend()
    save_plot(insert_dir / "insercion_sequential_construccion.png")

    print("[plot] Gráficos Sequential listos.")


def plot_working_set_results(working_csv: Path, search_dir: Path, insert_dir: Path) -> None:
    df = load_csv_if_exists(working_csv)
    if df is None:
        return

    required_columns = {
        "tree",
        "n",
        "w",
        "m",
        "build_time_ns",
        "search_time_ns",
    }
    validate_columns(df, required_columns, str(working_csv))

    df = df.copy()
    df["search_time_ms"] = df["search_time_ns"] / 1_000_000.0
    df["avg_search_ns"] = df["search_time_ns"] / df["m"]
    df["build_time_ms"] = df["build_time_ns"] / 1_000_000.0

    # búsqueda total
    plt.figure(figsize=(9, 5))
    for tree, tree_group in df.groupby("tree"):
        tree_group = tree_group.sort_values("w")
        plt.plot(tree_group["w"], tree_group["search_time_ms"], marker="o", label=tree)

    plt.title("Tiempos de búsqueda — Working Set Theorem\nTiempo total de búsqueda")
    plt.xlabel("W")
    plt.ylabel("Tiempo total de búsqueda (ms)")
    plt.xscale("log", base=10)
    plt.grid(True, alpha=0.3)
    plt.legend()
    save_plot(search_dir / "busqueda_working_set_total.png")

    # búsqueda promedio
    plt.figure(figsize=(9, 5))
    for tree, tree_group in df.groupby("tree"):
        tree_group = tree_group.sort_values("w")
        plt.plot(tree_group["w"], tree_group["avg_search_ns"], marker="o", label=tree)

    plt.title("Tiempos de búsqueda — Working Set Theorem\nPromedio por búsqueda")
    plt.xlabel("W")
    plt.ylabel("Tiempo promedio por búsqueda (ns)")
    plt.xscale("log", base=10)
    plt.grid(True, alpha=0.3)
    plt.legend()
    save_plot(search_dir / "busqueda_working_set_promedio.png")

    # construcción
    plt.figure(figsize=(9, 5))
    for tree, tree_group in df.groupby("tree"):
        tree_group = tree_group.sort_values("w")
        plt.plot(tree_group["w"], tree_group["build_time_ms"], marker="o", label=tree)

    plt.title("Tiempos de inserción — Working Set Theorem\nTiempo de construcción del árbol")
    plt.xlabel("W")
    plt.ylabel("Tiempo de construcción (ms)")
    plt.xscale("log", base=10)
    plt.grid(True, alpha=0.3)
    plt.legend()
    save_plot(insert_dir / "insercion_working_set_construccion.png")

    print("[plot] Gráficos Working Set listos.")


def main() -> None:
    parser = argparse.ArgumentParser(
        description="Genera gráficos separados de búsqueda e inserción para AVL vs Splay Tree."
    )

    parser.add_argument("--base-csv", default="results/base_results.csv")
    parser.add_argument("--blocks-csv", default="results/base_search_blocks.csv")
    parser.add_argument("--sequential-csv", default="results/sequential_results.csv")
    parser.add_argument("--working-csv", default="results/working_set_results.csv")
    parser.add_argument("--out", default="results/plots")
    parser.add_argument(
        "--clean",
        action="store_true",
        help="Borra los gráficos anteriores antes de generar los nuevos.",
    )

    args = parser.parse_args()

    output_dir = Path(args.out)
    search_dir = output_dir / "tiempos_busqueda"
    insert_dir = output_dir / "tiempos_insercion"

    if args.clean:
        clean_dir(search_dir)
        clean_dir(insert_dir)
    else:
        ensure_output_dir(search_dir)
        ensure_output_dir(insert_dir)

    plot_base_results(Path(args.base_csv), search_dir, insert_dir)
    plot_search_blocks(Path(args.blocks_csv), search_dir)
    plot_sequential_results(Path(args.sequential_csv), search_dir, insert_dir)
    plot_working_set_results(Path(args.working_csv), search_dir, insert_dir)

    print("[plot] Proceso terminado.")
    print(f"[plot] Gráficos de búsqueda: {search_dir}")
    print(f"[plot] Gráficos de inserción: {insert_dir}")


if __name__ == "__main__":
    main()
