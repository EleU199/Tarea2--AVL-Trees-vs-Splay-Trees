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


def load_csv(path: Path) -> pd.DataFrame:
    if not path.exists():
        raise FileNotFoundError(f"No existe el archivo: {path}")
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


def plot_search_index_for_selected_n(
    df: pd.DataFrame,
    output_dir: Path,
    n_values: list[int],
    use_block_end: bool,
) -> None:
    """
    Genera un gráfico por cada par (N, escenario), agrupando AVL y Splay.

    Nota:
    El CSV base_search_blocks.csv no contiene el tiempo de cada búsqueda individual,
    sino el tiempo promedio por bloque de búsquedas. Por eso el eje X representa
    el índice aproximado de búsqueda del bloque.
    """

    available_n = sorted(int(n) for n in df["n"].unique())

    for n in n_values:
        if n not in available_n:
            print(f"[plot] Advertencia: N={n} no está en el CSV. Valores disponibles: {available_n}")
            continue

        df_n = df[df["n"] == n]

        for scenario in sorted_scenarios(df_n):
            group = df_n[df_n["scenario"] == scenario]

            plt.figure(figsize=(11, 5))

            for tree, tree_group in group.groupby("tree"):
                tree_group = tree_group.sort_values("block_index").copy()

                if use_block_end:
                    tree_group["search_index"] = (
                        tree_group["block_start"] + tree_group["block_size"]
                    )
                    xlabel = "Índice de búsqueda al final del bloque"
                else:
                    tree_group["search_index"] = (
                        tree_group["block_start"] + (tree_group["block_size"] / 2.0)
                    )
                    xlabel = "Índice aproximado de búsqueda, centro del bloque"

                plt.plot(
                    tree_group["search_index"],
                    tree_group["avg_search_ns"],
                    linewidth=1.2,
                    label=tree,
                )

            plt.title(
                "Tiempos de búsqueda — índice de búsqueda vs tiempo de ejecución\n"
                f"{scenario_title(scenario)}, N={n}"
            )
            plt.xlabel(xlabel)
            plt.ylabel("Tiempo promedio por búsqueda en el bloque (ns)")
            plt.grid(True, alpha=0.3)
            plt.legend()

            output_path = output_dir / f"busqueda_indice_vs_tiempo__n_{n}__{scenario}.png"
            save_plot(output_path)


def plot_search_index_combined_by_scenario(
    df: pd.DataFrame,
    output_dir: Path,
    n_values: list[int],
    use_block_end: bool,
) -> None:
    """
    Genera un gráfico por escenario, incluyendo ambas estructuras para los N pedidos.
    Es un complemento opcional a los 8 gráficos principales.
    """

    for scenario in sorted_scenarios(df):
        group_scenario = df[df["scenario"] == scenario]

        plt.figure(figsize=(12, 6))

        for n in n_values:
            group_n = group_scenario[group_scenario["n"] == n]

            if group_n.empty:
                continue

            for tree, tree_group in group_n.groupby("tree"):
                tree_group = tree_group.sort_values("block_index").copy()

                if use_block_end:
                    tree_group["search_index"] = (
                        tree_group["block_start"] + tree_group["block_size"]
                    )
                    xlabel = "Índice de búsqueda al final del bloque"
                else:
                    tree_group["search_index"] = (
                        tree_group["block_start"] + (tree_group["block_size"] / 2.0)
                    )
                    xlabel = "Índice aproximado de búsqueda, centro del bloque"

                plt.plot(
                    tree_group["search_index"],
                    tree_group["avg_search_ns"],
                    linewidth=1.1,
                    label=f"{tree}, N={n}",
                )

        plt.title(
            "Tiempos de búsqueda — comparación por índice de búsqueda\n"
            f"{scenario_title(scenario)}"
        )
        plt.xlabel(xlabel)
        plt.ylabel("Tiempo promedio por búsqueda en el bloque (ns)")
        plt.grid(True, alpha=0.3)
        plt.legend(fontsize=8)

        output_path = output_dir / f"busqueda_indice_vs_tiempo__comparado__{scenario}.png"
        save_plot(output_path)


def main() -> None:
    parser = argparse.ArgumentParser(
        description=(
            "Genera gráficos de índice de búsqueda vs tiempo para los escenarios base, "
            "usando base_search_blocks.csv."
        )
    )

    parser.add_argument(
        "--csv",
        default="results/base_search_blocks.csv",
        help="Ruta al CSV base_search_blocks.csv.",
    )

    parser.add_argument(
        "--out",
        default="results/plots/busqueda_por_indice",
        help="Carpeta de salida para los gráficos.",
    )

    parser.add_argument(
        "--n-values",
        nargs="+",
        type=int,
        default=[2**12, 2**14],
        help="Valores de N a graficar. Por defecto: 4096 16384.",
    )

    parser.add_argument(
        "--block-end",
        action="store_true",
        help=(
            "Usa el índice final del bloque como eje X. "
            "Por defecto se usa el centro del bloque."
        ),
    )

    parser.add_argument(
        "--combined",
        action="store_true",
        help=(
            "Además de los 8 gráficos principales, genera 4 gráficos "
            "comparando ambos N dentro de cada escenario."
        ),
    )

    parser.add_argument(
        "--clean",
        action="store_true",
        help="Borra gráficos anteriores antes de generar los nuevos.",
    )

    args = parser.parse_args()

    csv_path = Path(args.csv)
    output_dir = Path(args.out)

    if args.clean:
        clean_dir(output_dir)
    else:
        ensure_output_dir(output_dir)

    df = load_csv(csv_path)

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

    validate_columns(df, required_columns, str(csv_path))

    plot_search_index_for_selected_n(
        df=df,
        output_dir=output_dir,
        n_values=args.n_values,
        use_block_end=args.block_end,
    )

    if args.combined:
        plot_search_index_combined_by_scenario(
            df=df,
            output_dir=output_dir,
            n_values=args.n_values,
            use_block_end=args.block_end,
        )

    print("[plot] Proceso terminado.")
    print(f"[plot] Gráficos guardados en: {output_dir}")
    print("[plot] Nota: el CSV contiene tiempos promedio por bloque, no tiempos individuales por búsqueda.")


if __name__ == "__main__":
    main()
