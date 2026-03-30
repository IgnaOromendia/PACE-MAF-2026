import pandas as pd
import os
import math
import matplotlib.pyplot as plt

INPUT_PATH = "./edgeScore"
OUTPUT_PATH = "./plots"

scores = {}
edge_ids = {}
d_values = {}
instance = []

def read_input():
    for filename in os.listdir(INPUT_PATH):
        file_path = os.path.join(INPUT_PATH, filename)
        if os.path.isfile(file_path):
            name = filename.replace(".csv", "")
            df = pd.read_csv(file_path)
            scores[name] = df["edge_score"]
            edge_ids[name] = df["edge_id"]
            d_values[name] = df["d_value"]
            instance.append(name)

def plot_instances():
    if not instance:
        return

    os.makedirs(OUTPUT_PATH, exist_ok=True)

    sorted_instances = sorted(instance)
    cols = 2
    rows = math.ceil(len(sorted_instances) / cols)

    fig, axes = plt.subplots(rows, cols, figsize=(16, 4 * rows), squeeze=False)
    axes_flat = axes.flatten()

    for ax, name in zip(axes_flat, sorted_instances):
        colors = ["red" if d_value == 1 else "steelblue" for d_value in d_values[name]]
        ax.bar(edge_ids[name], scores[name], width=0.8, color=colors)
        ax.set_title(name)
        ax.set_xlabel("edgeId")
        ax.set_ylabel("edge score")
        ax.grid(True, alpha=0.3)

    for ax in axes_flat[len(sorted_instances):]:
        ax.axis("off")

    fig.suptitle("Edge scores por instancia")
    fig.tight_layout()
    fig.savefig(os.path.join(OUTPUT_PATH, "all_instances_edge_scores.png"), dpi=300)
    plt.close(fig)



if __name__ == "__main__":
    read_input()
    plot_instances()
