from pathlib import Path

import pandas as pd

INPUT_PATH = Path("./edgeScore")
REQUIRED_COLUMNS = ["edge_id", "number", "edge_score", "triples", "paths", "damage", "d_value", "leaf"]


def load_data(input_dir: Path) -> pd.DataFrame:
    frames = []

    for csv_path in sorted(input_dir.glob("*.csv")):
        df = pd.read_csv(csv_path)
        missing = [column for column in REQUIRED_COLUMNS if column not in df.columns]

        if missing:
            missing_text = ", ".join(missing)
            raise ValueError(f"{csv_path} no tiene las columnas requeridas: {missing_text}")

        if df.empty: continue

        df = df[REQUIRED_COLUMNS].copy()
        df["instance"] = csv_path.stem
        frames.append(df)

    if not frames:
        raise FileNotFoundError(f"No se encontraron CSVs con datos en {input_dir}")

    data = pd.concat(frames, ignore_index=True)
    data["number"] = pd.to_numeric(data["number"], errors="raise").round().astype(int)
    data["edge_score"] = pd.to_numeric(data["edge_score"], errors="raise")

    for column in ["triples", "paths", "damage"]:
        data[column] = pd.to_numeric(data[column], errors="raise")

    data["d_value"] = pd.to_numeric(data["d_value"], errors="raise").round().astype(int)
    data["leaf"] = pd.to_numeric(data["leaf"], errors="raise").round().astype(int)

    return data


def leaf_prob(df: pd.DataFrame) -> float:
    leaf_rows = df[df["leaf"] == 1]
    if leaf_rows.empty:
        return 0.0

    return float((leaf_rows["d_value"] == 1).mean())


def cut_precision(df: pd.DataFrame) -> float:
    return float((df["d_value"] == 1).mean() * 100)

def cut_precision_by_number(df: pd.DataFrame) -> pd.Series:
    return (
        df.groupby("number", sort=True)["d_value"]
        .apply(lambda values: (values == 1).mean() * 100)
    )


def unused_edges(df:pd.DataFrame) -> float:
    return float((df["d_value"] == -1).mean() * 100)


def count_instances_with_all_d_value(df: pd.DataFrame, target: int) -> int:
    return int(
        df.groupby("instance", sort=True)["d_value"]
        .apply(lambda values: (values == target).all())
        .sum()
    )


def count_instances_with_all_d_value_except_one(df: pd.DataFrame) -> int:
    return int(
        df.groupby("instance", sort=True)["d_value"]
        .apply(lambda values: ((values == 0).sum() == 1) and ((values == 1).sum() == len(values) - 1))
        .sum()
    )


def average_metric_by_instance(df: pd.DataFrame, metric_fn) -> float:
    metric_by_instance = [
        metric_fn(instance_df)
        for _, instance_df in df.groupby("instance", sort=True)
    ]

    return float(sum(metric_by_instance) / len(metric_by_instance))

if __name__ == "__main__":
    data = load_data(INPUT_PATH)
    total_instances     = data["instance"].nunique()
    avg_leaf_cut_prob   = average_metric_by_instance(data, leaf_prob)

    greedy_precision = cut_precision(data)
    greedy_precision_by_number = cut_precision_by_number(data)

    avg_unused_edges    = average_metric_by_instance(data, unused_edges)

    instances_all_one   = count_instances_with_all_d_value(data, 1)
    instances_all_zero  = count_instances_with_all_d_value(data, 0)
    instances_one_error = count_instances_with_all_d_value_except_one(data)

    print(f"Archivos cargados: {total_instances}")
    print(f"Filas cargadas: {len(data)}")
    print(f"Probabilidad de cortar una hoja: {avg_leaf_cut_prob:.6f}")
    print(f"Precisión de selección greedy: {greedy_precision:.2f}%")
    print("Precisión de selección greedy por corte:")
    for number, precision in greedy_precision_by_number.items():
        print(f"  corte {number}: {precision:.2f}%")
    print(f"Promedio de aristas no usadas para promedios por instancia: {avg_unused_edges:.2f}%")
    print(
        f"Instancias de perfecta precisión: {instances_all_one} "
        f"({instances_all_one / total_instances * 100:.2f}%)"
    )
    print(
        f"Instancias de nula precisión: {instances_all_zero} "
        f"({instances_all_zero / total_instances * 100:.2f}%)"
    )
    print(
        f"Instancias con un único error: {instances_one_error} "
        f"({instances_one_error / total_instances * 100:.2f}%)"
    )
