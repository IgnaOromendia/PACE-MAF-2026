from __future__ import annotations

import argparse
from pathlib import Path

import pandas as pd

INPUT_PATH = Path("./edgeScore/bestEdgeScore.csv")
REQUIRED_COLUMNS = ["edge_id", "edge_score", "d_value"]


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(
        description=(
            "Procesa bestEdgeScore.csv y calcula la probabilidad de que "
            "la arista con mayor edge_score sea eliminada."
        )
    )
    parser.add_argument(
        "--input",
        type=Path,
        default=INPUT_PATH,
        help="Ruta al CSV bestEdgeScore.",
    )
    return parser.parse_args()


def load_data(input_path: Path) -> pd.DataFrame:
    if not input_path.exists():
        raise FileNotFoundError(f"No existe el archivo {input_path}")

    data = pd.read_csv(input_path)
    missing = [column for column in REQUIRED_COLUMNS if column not in data.columns]

    if missing:
        missing_text = ", ".join(missing)
        raise ValueError(f"{input_path} no tiene las columnas requeridas: {missing_text}")

    data = data[REQUIRED_COLUMNS].copy()
    data["edge_id"] = pd.to_numeric(data["edge_id"], errors="raise").astype(int)
    data["edge_score"] = pd.to_numeric(data["edge_score"], errors="raise")
    data["d_value"] = pd.to_numeric(data["d_value"], errors="raise").astype(int)

    if data.empty:
        raise ValueError(f"{input_path} no tiene filas para procesar")

    if not data["d_value"].isin([0, 1]).all():
        raise ValueError("La columna d_value debe contener solo 0 o 1")

    return data


def summarize_best_edges(data: pd.DataFrame) -> dict[str, float | int]:
    total_edges = int(len(data))
    cut_edges = int(data["d_value"].sum())
    cut_probability = cut_edges / total_edges

    return {
        "total_edges": total_edges,
        "cut_edges": cut_edges,
        "cut_probability": cut_probability,
        "avg_best_edge_score": float(data["edge_score"].mean()),
    }


def print_summary(summary: dict[str, float | int], input_path: Path) -> None:
    print("Resumen best edges")
    print(f"- archivo analizado: {input_path}")
    print(f"- cantidad de best edges: {summary['total_edges']}")
    print(f"- suma de d_value: {summary['cut_edges']}")
    print(
        "- probabilidad de eliminar la arista con mayor edge score: "
        f"{summary['cut_probability']:.6f}"
    )
    print(f"- edge_score promedio de esas aristas: {summary['avg_best_edge_score']:.6f}")


def main() -> None:
    args = parse_args()
    data = load_data(args.input)
    summary = summarize_best_edges(data)
    print_summary(summary, args.input)


if __name__ == "__main__":
    main()
