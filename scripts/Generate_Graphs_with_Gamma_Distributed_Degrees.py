import numpy as np
from scipy.sparse import csr_matrix
from scipy.io import mmwrite
import os
import matplotlib.pyplot as plt
from scipy.stats import gamma

def generate_gamma_degree_graph(num_of_nodes, shape, scale):
    # Sample degrees from Gamma distribution
    degrees = np.random.gamma(shape, scale, num_of_nodes)
    degrees = np.round(degrees).astype(int)

    # Ensure all degrees are at least 1 (optional)
    degrees[degrees < 1] = 1

    # Make sum of degrees even
    if degrees.sum() % 2 != 0:
        degrees[0] += 1

    # Create stubs
    stubs = np.repeat(np.arange(num_of_nodes), degrees)
    np.random.shuffle(stubs)

    # Pair stubs into edges
    row, col = [], []
    existing_edges = set()
    i = 0
    while i < len(stubs) - 1:
        u, v = stubs[i], stubs[i + 1]
        if u != v and (u, v) not in existing_edges and (v, u) not in existing_edges:
            row.extend([u, v])
            col.extend([v, u])
            existing_edges.add((u, v))
            i += 2
        else:
            # Try to find a better partner by swapping with a later stub
            j = i + 2
            while j < len(stubs):
                w = stubs[j]
                if u != w and (u, w) not in existing_edges and (w, u) not in existing_edges:
                    stubs[i + 1], stubs[j] = stubs[j], stubs[i + 1]
                    break
                j += 1
            i += 2

    # Build CSR matrix
    data = np.ones(len(row), dtype=int)
    csr_graph = csr_matrix((data, (row, col)), shape=(num_of_nodes, num_of_nodes))

    return csr_graph


# Generate graphs, If you prefer using degree:
# num_of_nodes = 10000
# shape = 2.0      # Gamma shape (k)
# scale = 2.0      # Gamma scale (θ)
# csr_graph = generate_gamma_degree_graph(num_of_nodes, shape, scale)


# Generate graphs, If you prefer using density:
num_of_nodes = 10000
density = 0.001                                                    # desired edge density
target_avg_deg = density * (num_of_nodes - 1)
shape = 2.0                                                       # controls variance of degrees
scale = target_avg_deg / shape
csr_graph = generate_gamma_degree_graph(num_of_nodes, shape, scale)


# File path
base_dir = "Graphs_with_Gamma_Distributed_Degrees"
folder_name = f"gamma_shape_{shape:.1f}_scale_{scale:.1f}_{num_of_nodes}"
folder_path = os.path.join(base_dir, folder_name)
os.makedirs(folder_path, exist_ok=True)
row_offsets_path = os.path.join(folder_path, "gamma_rowOffsets.txt")
col_indices_path = os.path.join(folder_path, "gamma_columnIndices.txt")
mm_path = os.path.join(folder_path, "gamma_graph.mtx")
pdf_path = os.path.join(folder_path, "degree_histogram.pdf")


# Save graph data
np.savetxt(row_offsets_path, csr_graph.indptr, fmt='%d')
np.savetxt(col_indices_path, csr_graph.indices, fmt='%d')
mmwrite(mm_path, csr_graph)
print(f"Gamma-distributed graph saved to '{folder_path}'")


# Degree histogram
degrees = csr_graph.getnnz(axis=1)
plt.figure(figsize=(8, 5))
plt.hist(degrees, bins=100, density=True, alpha=0.7, edgecolor='black')
plt.title("Histogram of Node Degrees")
plt.xlabel("Degree")
plt.ylabel("Probability Density")
plt.grid(True)
plt.savefig(pdf_path, dpi=400, bbox_inches='tight')
plt.show()

# Fit Gamma to degrees
filtered_degrees = degrees[degrees > 0]  # remove zeros
fitted_shape, loc, fitted_scale = gamma.fit(filtered_degrees, floc=0)
print(f"Fitted shape: {fitted_shape:.3f}, scale: {fitted_scale:.3f}")