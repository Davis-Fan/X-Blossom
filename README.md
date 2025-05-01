# X-Blossom

This is the code repository for X-Blossom


---

## Download the code repository for X-Blossom

Firstly, use the command line below to download this repository.

```
$ git clone https://github.com/Davis-Fan/X-Blossom.git
$ cd X-Blossom
```

---

## Dataset

We use both real-world and synthetic datasets to test **X-Blossom**. Due to storage limitations, we provide only a few example real-world graphs in the **Example_Dataset** folder, which includes two graphs: Google and StackOverflow. The full datasets can be downloaded from the following links: [real-world datasets](https://drive.google.com/drive/folders/1lzK8XVxkpKgmH_p8ylyXZw_cmoE_zQXm?usp=drive_link), [Erdős–Rényi random graphs](https://drive.google.com/drive/folders/1-K0nVgKUf7_w4E6YkQyRZ0Tx0hzJLq7V?usp=drive_link), [𝑑-regular random graphs](https://drive.google.com/drive/folders/1ehKgkS6jnYNm4KeubG9_U5SaHADrwlac?usp=drive_link), [random graphs with gamma-distributed degrees](https://drive.google.com/drive/folders/17FUNo-aCNM0GJTlzalqYrOqz349lNovD?usp=drive_link), and [scalability test graphs](https://drive.google.com/drive/folders/1PtXNz4Hl6cK-coSwPRNQjmDXQhaNBPfZ?usp=drive_link).

All graphs are undirected and stored in the Compressed Sparse Row (CSR) format. The representation consists of two arrays: `row_offsets` and `column_indices`. The `row_offsets` array marks the start index of each node’s adjacency list within `column_indices`, while the `column_indices` array stores the adjacent neighbors of each node. Since the graphs are undirected, each edge `(u, v)` appears twice: once in `column_indices` for node `u` and once for node `v`.

For example, consider a graph:

    0 — 1 — 2
     \  |
      \ |
        3

The adjacency list representation is:

    0 → {1, 3}
    1 → {0, 2, 3}
    2 → {1}
    3 → {0, 1}

The CSR format is:

    row_offsets    = [0, 2, 5, 6, 8]
    column_indices = [1, 3, 0, 2, 3, 1, 0, 1]



In the test, we need to input two paths that contain the information of a graph. For example, if we want to test the Google social graph in the example dataset, the first parameter is the "Example_Dataset/Example_Realworld_Datasets/Google/gplus_rowOffsets.txt"  which is the **row offsets** array of the Google social graph in the real-world datasets. The second parameter is the "Example_Dataset/Example_Realworld_Datasets/Google/gplus_columnIndices.txt", which is the **column indices** array of the Google social graph.

 

---

## Run the X-Blossom

1) Combine and unzip the example graphs

```
$ cat Example_Dataset.part_* > Example_Dataset.zip
$ unzip Example_Dataset.zip
$ rm Example_Dataset.zip Example_Dataset.part_*
```

2) Create a new folder to contain the complied file

```
$ mkdir test
$ cd test
```

3) Complie code

```
$ cmake ..
$ make
```

4. Run the program. We need to input two paths that contain the information of the graph. For example, if we want to test the StackOverflow graph, please use:

```
$ ./Blossom ../Example_Dataset/Example_Realworld_Datasets/StackOverflow/stackOverflow_rowOffsets.txt ../Example_Dataset/Example_Realworld_Datasets/StackOverflow/stackOverflow_columnIndices.txt 1 8
```

The program will execute the *same* test for 20 runs. The third parameter is 1, indicating it is a parallel program. The forth parameter is 8, which is the number of threads used in the parallel program.

