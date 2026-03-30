from __future__ import annotations

import argparse
import os
from pathlib import Path

os.environ.setdefault("MPLCONFIGDIR", "/tmp/matplotlib")
os.environ.setdefault("MPLBACKEND", "Agg")
os.environ.setdefault("XDG_CACHE_HOME", "/tmp")

Path(os.environ["MPLCONFIGDIR"]).mkdir(parents=True, exist_ok=True)

import matplotlib.pyplot as plt
import numpy as np
import pandas as pd

INPUT_PATH = Path("./edgeScore")
OUTPUT_PATH = Path("./plots/edge_score_analysis")
REQUIRED_COLUMNS = ["edge_id", "edge_score", "d_value"]


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(
        description="Grafico simple para ver si edge_score se relaciona con el corte."
    )
    parser.add_argument(
        "--input-dir",
        type=Path,
        default=INPUT_PATH,
        help="Directorio con CSVs edge_id, edge_score, d_value.",
    )
    parser.add_argument(
        "--output-dir",
        type=Path,
        default=OUTPUT_PATH,
        help="Directorio donde guardar el grafico y el resumen.",
    )
    parser.add_argument(
        "--bins",
        type=int,
        default=None,
        help="Cantidad de bins para agrupar edge_score flotantes.",
    )
    parser.add_argument(
        "--bin-width",
        type=float,
        default=None,
        help="Ancho fijo de bin para agrupar edge_score flotantes.",
    )
    args = parser.parse_args()

    if args.bins is not None and args.bins < 1:
        raise ValueError("--bins debe ser un entero positivo")

    if args.bin_width is not None and args.bin_width <= 0:
        raise ValueError("--bin-width debe ser positivo")

    if args.bins is not None and args.bin_width is not None:
        raise ValueError("Usa solo una de --bins o --bin-width")

    return args


def load_data(input_dir: Path) -> pd.DataFrame:
    frames = []

    for csv_path in sorted(input_dir.glob("*.csv")):
        df = pd.read_csv(csv_path)
        missing = [column for column in REQUIRED_COLUMNS if column not in df.columns]

        if missing:
            missing_text = ", ".join(missing)
            raise ValueError(f"{csv_path} no tiene las columnas requeridas: {missing_text}")

        df = df[REQUIRED_COLUMNS].copy()
        df["instance"] = csv_path.stem
        frames.append(df)

    if not frames:
        raise FileNotFoundError(f"No se encontraron CSVs en {input_dir}")

    data = pd.concat(frames, ignore_index=True)
    data["edge_score"] = pd.to_numeric(data["edge_score"], errors="raise")
    data["d_value"] = pd.to_numeric(data["d_value"], errors="raise").astype(int)

    if not data["d_value"].isin([0, 1]).all():
        raise ValueError("La columna d_value debe contener solo 0 o 1")

    return data


def safe_corr(x: pd.Series, y: pd.Series, method: str) -> float:
    if x.nunique() < 2 or y.nunique() < 2:
        return float("nan")
    return float(x.corr(y, method=method))


def weighted_trend_coeffs(summary: pd.DataFrame) -> tuple[float, float] | None:
    if len(summary) < 2:
        return None

    x = summary["edge_score"].to_numpy(dtype=float)
    y = summary["cut_rate"].to_numpy(dtype=float)
    weights = summary["total_edges"].to_numpy(dtype=float)

    if np.allclose(x, x[0]) or np.all(weights <= 0):
        return None

    weight_sum = weights.sum()
    x_mean = np.dot(weights, x) / weight_sum
    y_mean = np.dot(weights, y) / weight_sum
    covariance = np.dot(weights, (x - x_mean) * (y - y_mean))
    variance = np.dot(weights, (x - x_mean) ** 2)

    if np.isclose(variance, 0.0):
        return None

    slope = covariance / variance
    intercept = y_mean - slope * x_mean
    return float(slope), float(intercept)


def build_bin_edges(scores: pd.Series, bins: int | None, bin_width: float | None) -> np.ndarray:
    score_min = float(scores.min())
    score_max = float(scores.max())

    if np.isclose(score_min, score_max):
        delta = bin_width if bin_width is not None else 0.5
        return np.array([score_min - delta, score_max + delta], dtype=float)

    if bin_width is not None:
        start = np.floor(score_min / bin_width) * bin_width
        end = np.ceil(score_max / bin_width) * bin_width
        edges = np.arange(start, end + bin_width, bin_width, dtype=float)
        if edges[-1] < score_max:
            edges = np.append(edges, edges[-1] + bin_width)
        return edges

    if bins is None:
        q1, q3 = scores.quantile([0.25, 0.75])
        iqr = float(q3 - q1)
        if iqr > 0:
            fd_width = 2 * iqr / (len(scores) ** (1 / 3))
            bins = int(np.ceil((score_max - score_min) / fd_width)) if fd_width > 0 else 30
        else:
            bins = int(np.sqrt(len(scores)))
        bins = max(10, min(60, bins))

    return np.linspace(score_min, score_max, bins + 1, dtype=float)


def summarize_by_score(data: pd.DataFrame, bins: int | None, bin_width: float | None) -> pd.DataFrame:
    edges = build_bin_edges(data["edge_score"], bins=bins, bin_width=bin_width)
    score_bins = pd.cut(
        data["edge_score"],
        bins=edges,
        include_lowest=True,
        duplicates="drop",
    )

    return (
        data.assign(score_bin=score_bins)
        .groupby("score_bin", observed=True, as_index=False)
        .agg(
            score_min=("edge_score", "min"),
            score_max=("edge_score", "max"),
            edge_score=("edge_score", "mean"),
            score_median=("edge_score", "median"),
            total_edges=("d_value", "size"),
            cut_edges=("d_value", "sum"),
            cut_rate=("d_value", "mean"),
        )
        .sort_values("score_min")
        .assign(
            bin_left=lambda df: df["score_bin"].map(lambda interval: float(interval.left)),
            bin_right=lambda df: df["score_bin"].map(lambda interval: float(interval.right)),
        )
        .drop(columns="score_bin")
    )


def plot_cut_probability(summary: pd.DataFrame, output_dir: Path, pearson_corr: float) -> Path:
    output_dir.mkdir(parents=True, exist_ok=True)
    output_path = output_dir / "edge_score_vs_cut_probability.png"

    fig, ax = plt.subplots(figsize=(10, 6))

    max_edges = max(int(summary["total_edges"].max()), 1)
    sizes = 60 + 220 * summary["total_edges"] / max_edges
    ax.scatter(
        summary["edge_score"],
        summary["cut_rate"],
        s=sizes,
        color="steelblue",
        alpha=0.8,
        edgecolors="black",
        linewidths=0.5,
    )

    coeffs = weighted_trend_coeffs(summary)
    if coeffs is not None:
        x_values = np.linspace(summary["edge_score"].min(), summary["edge_score"].max(), 200)
        y_values = coeffs[0] * x_values + coeffs[1]
        ax.plot(x_values, y_values, color="firebrick", linewidth=2)

    ax.set_title("Relacion entre edge_score y probabilidad de corte")
    ax.set_xlabel("edge_score promedio del bin")
    ax.set_ylabel("proporcion de veces que la arista fue cortada")
    ax.set_ylim(-0.02, 1.02)
    ax.grid(True, alpha=0.25)

    ax.text(
        0.02,
        0.98,
        f"Pearson score/corte = {pearson_corr:.3f}",
        transform=ax.transAxes,
        ha="left",
        va="top",
        bbox={"boxstyle": "round", "facecolor": "white", "alpha": 0.9, "edgecolor": "0.8"},
    )
    ax.text(
        0.02,
        0.90,
        "Tamano del punto = cantidad de aristas en el bin",
        transform=ax.transAxes,
        ha="left",
        va="top",
        bbox={"boxstyle": "round", "facecolor": "white", "alpha": 0.9, "edgecolor": "0.8"},
    )

    fig.tight_layout()
    fig.savefig(output_path, dpi=300)
    plt.close(fig)
    return output_path


def print_summary(
    data: pd.DataFrame,
    summary: pd.DataFrame,
    pearson_corr: float,
    spearman_corr: float,
    plot_path: Path,
) -> None:
    print("Resumen")
    print(f"- archivos analizados: {data['instance'].nunique()}")
    print(f"- filas analizadas: {len(data)}")
    print(f"- bins usados en el grafico: {len(summary)}")
    print(f"- correlacion de Pearson score/corte: {pearson_corr:.4f}")
    print(f"- correlacion de Spearman score/corte: {spearman_corr:.4f}")
    print(f"- grafico: {plot_path}")
    print()
    print("Lectura rapida")
    if pd.isna(pearson_corr):
        print("- no se pudo calcular la correlacion")
    elif abs(pearson_corr) < 0.1:
        print("- no se ve una correlacion lineal clara entre edge_score y corte")
    elif pearson_corr > 0:
        print("- hay una tendencia positiva: a mayor edge_score, mas corte")
    else:
        print("- hay una tendencia negativa: a mayor edge_score, menos corte")
    


def main() -> None:
    args = parse_args()
    data = load_data(args.input_dir)
    summary = summarize_by_score(data, bins=args.bins, bin_width=args.bin_width)

    pearson_corr = safe_corr(data["edge_score"], data["d_value"], method="pearson")
    spearman_corr = safe_corr(data["edge_score"], data["d_value"], method="spearman")

    args.output_dir.mkdir(parents=True, exist_ok=True)
    summary.to_csv(args.output_dir / "cut_probability_by_score.csv", index=False)
    plot_path = plot_cut_probability(summary, args.output_dir, pearson_corr)
    print_summary(data, summary, pearson_corr, spearman_corr, plot_path)


if __name__ == "__main__":
    main()
