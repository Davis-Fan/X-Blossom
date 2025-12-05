#include "blossom.h"
#include "graph.h"
#include "stopwatch.h"

/// \brief Compute a maximum matching on an undirected graph using X-Blossom.
///
/// This routine repeatedly finds augmenting paths in G and updates the
/// matching until no augmenting path exists. On output, M encodes a
/// maximum matching: M[v] is the vertex matched with v, or -1 if v is
/// unmatched.
///
/// \param G
///     Undirected input graph. Vertices are assumed to be labeled
///     0, 1, ..., n-1. G is stored in CSR format (two std::vectors).
///
/// \param M
///     Output matching vector. On entry, M may be empty or any size.
///     On return, M is resized to the number of vertices in G, and
///     M[v] contains the matched partner of v, or -1 if v is unmatched.
///     (one std::vector)
///
/// \param num_of_threads
///     Number of worker threads used by the parallel X-Blossom algorithm.
///     Must be >= 1. The implementation divides the search for augmenting
///     paths and the matching updates across these threads.
///
/// \note
///     This function does not perform any I/O. It is intended as a
///     library-style entry point that can be called from larger C++
///     applications.
void x_blossom_maximum_matching(Graph& G, std::vector<int>& M, int num_of_threads){

    std::vector<std::vector<int>> path_collection;
    path_collection.reserve(num_of_threads);
    std::vector<std::vector<int>> path_table_vector;
    int nodes = static_cast<int>(G.rowOffsets.size()) - 1;
    path_table_vector.resize(nodes);
    for (auto& sub_vector : path_table_vector) {
        sub_vector.reserve(100);
        sub_vector.clear();
    }

    while(true){
        for (auto& sub_vector : path_table_vector) {
            sub_vector.clear();
        }

        parFindAugmentingPathNoRecursionUpdatePathTable(G,M, path_collection, path_table_vector, num_of_threads);
        if(path_collection.empty()){
            break;
        }
        parNewMatchingVector(M, path_collection, num_of_threads);
        path_collection.clear();
    }

}

//////////////////////////////////////////////////////////////////////////////////////////////////////////
/// \brief All followings are internal helper functions used by x_blossom_maximum_matching.
///
/// These functions are implementation details of the X-Blossom algorithm.
/// They are not part of the public API
//////////////////////////////////////////////////////////////////////////////////////////////////////////
std::mutex path_mutex_lock;
void parAugmentingPath(const std::vector<int>& rowOffsets,
                           const std::vector<int>& columnIndices,
                           const std::vector<int>& nodes_vector,
                           int index, int num_threads, std::vector<int>& is_even, std::vector<int>& belongs,
                           std::vector<std::vector<int>>& path_table_vector, std::vector<std::atomic<int>>& select_tree, std::vector<std::vector<int>>& path_collection){

    std::vector<int> local_path;
    int estimated_size = static_cast<int>((static_cast<double>(is_even.size()) / num_threads) * 1.25);
    local_path.reserve(estimated_size);


    for (int i = index; i < nodes_vector.size(); i += num_threads) {

        int v = nodes_vector[i];
        int start_index = rowOffsets[v];
        int end_index = rowOffsets[v + 1];

        for(int j = start_index; j<end_index; j++){

            int w = columnIndices[j];
            int expected = 0;
            int tree_v = belongs[v];
            int tree_w = belongs[w];

            if(is_even[w] && tree_v != tree_w && tree_v != -1 && tree_w != -1){

                int min_tree = (tree_v < tree_w) ? tree_v : tree_w;
                int max_tree = (tree_v < tree_w) ? tree_w : tree_v;

                if(select_tree[min_tree].compare_exchange_strong(expected, 1)){
                    if(select_tree[max_tree].compare_exchange_strong(expected, 1)){

                        std::vector<int> path_v_vector = find_path_vector(path_table_vector, v);
                        std::vector<int> path_w_vector = find_path_vector(path_table_vector, w);


                        for(int s=path_v_vector.size()-1;s>=0; s--){
                            local_path.push_back(path_v_vector[s]);
                        }

                        for(int t=0; t<path_w_vector.size(); t++){
                            local_path.push_back(path_w_vector[t]);
                        }

                    }else{
                        int expected_to = 1;
                        select_tree[min_tree].compare_exchange_strong(expected_to, 0);
                    }
                } else {
                    break;
                }
            }

        }
    }

    {std::lock_guard<std::mutex> guard(path_mutex_lock);
        if(!local_path.empty()){
            path_collection.push_back(local_path);
        }
    }
}


std::mutex expand_mutex_lock;
void parExpand(const std::vector<int>& rowOffsets,
                   const std::vector<int>& columnIndices,
                   const std::vector<int>& nodes_vector,
                   int index, int num_threads, std::vector<int>& is_even, std::vector<int>& belongs,
                   std::vector<std::vector<int>>& path_table_vector, std::vector<int>& vector_1, std::vector<std::atomic<int>>& select_match, std::vector<int>& M){

    std::vector<int> local_vector;
    int estimated_size = static_cast<int>((static_cast<double>(is_even.size()) / num_threads) * 1.25);
    local_vector.reserve(estimated_size);

    for (int i = index; i < nodes_vector.size(); i += num_threads) {
        int v = nodes_vector[i];
        int start_index = rowOffsets[v];
        int end_index = rowOffsets[v + 1];

        for(int j = start_index; j<end_index; j++){
            int w = columnIndices[j];
            int x = M[w];

            if(belongs[w] == -1){
                int expected = 0;
                int min_w_x = std::min(w,x);

                if(select_match[min_w_x].compare_exchange_strong(expected, 1)){

                    path_table_vector[x].push_back(w);
                    path_table_vector[x].push_back(v);

                    is_even[w] = 0;
                    is_even[x] = 1;

                    belongs[w] = belongs[v];
                    belongs[x] = belongs[v];

                    local_vector.push_back(x);
                }
            }
        }
    }

    {std::lock_guard<std::mutex> guard(expand_mutex_lock);
        vector_1.insert(vector_1.end(),local_vector.begin(), local_vector.end());
    }


}


std::mutex blossom_mutex_lock;
void parBlossom(const std::vector<int>& rowOffsets,
                    const std::vector<int>& columnIndices,
                    const std::vector<int>& nodes_vector,
                    int index, int num_threads, std::vector<int>& is_even, std::vector<int>& belongs,
                    std::vector<std::vector<int>>& path_table_vector, std::vector<int>& vector_2, std::vector<std::atomic<int>>& select_blossom,
                    std::vector<int>& blossom_to_base, std::vector<int>& M){

    std::vector<int> local_vector;
    int estimated_size = static_cast<int>((static_cast<double>(is_even.size()) / num_threads));
    local_vector.reserve(estimated_size);


    for (int i = index; i < nodes_vector.size(); i += num_threads) {
        int v = nodes_vector[i];
        int start_index = rowOffsets[v];
        int end_index = rowOffsets[v + 1];

        for(int j = start_index; j<end_index; j++){
            int w = columnIndices[j];

            if(is_even[w] && belongs[w] == belongs[v] && w != M[v] && belongs[w] != -1 && !(blossom_to_base[w] == blossom_to_base[v] && blossom_to_base[v] != -1)){

                bool consistent_flag = true;
                bool valid_flag = true;

                std::vector<int> path_v_vector = find_path_vector_blossom(path_table_vector, v);
                std::vector<int> path_w_vector = find_path_vector_blossom_w(path_table_vector, w, belongs, consistent_flag);

                if(!consistent_flag){
                    continue;
                }

                std::vector<int> blossom;
                blossom.reserve(path_v_vector.size()+path_w_vector.size());
                find_blossom_vector_debug(path_v_vector, path_w_vector, blossom, path_table_vector, valid_flag);
                if(!valid_flag){
                    continue;
                }
                blossom_to_base[blossom[0]] = blossom[0];

                for(int k=blossom.size()-3; k>=0; k=k-2){
                    int current = blossom[k];

                    if(blossom_to_base[current] == -1){
                        blossom_to_base[current] = blossom[0];
                    }

                    // The update of is_even may be missing
                    if(!is_even[current] && path_table_vector[current].empty()){
                        int expected = 0;

                        if(select_blossom[current].compare_exchange_strong(expected, 1)){
                            bool test = true;

                            for(int n=k+1; n<blossom.size(); n++){
                                path_table_vector[current].push_back(blossom[n]);
                            }

                            for(int c = 0;c<path_table_vector[current].size(); c++){
                                int check = path_table_vector[current][c];
                                for(int d = c+1; d<path_table_vector[current].size(); d++){
                                    if(check == path_table_vector[current][d]) {
                                        test = false;
                                        break;
                                    }
                                }
                            }

                            if(!test){
                                path_table_vector[current].clear();
                                continue;
                            }else{
                                local_vector.push_back(current);
                                is_even[current] = 1;
                            }

                        }
                    }
                }

                for(int k=2; k<blossom.size()-1; k=k+2){
                    int current = blossom[k];

                    if(blossom_to_base[current] == -1){
                        blossom_to_base[current] = blossom[0];
                    }

                    if(!is_even[current] && path_table_vector[current].empty()){
                        int expected = 0;

                        if(select_blossom[current].compare_exchange_strong(expected, 1)){
                            bool test = true;

                            for(int m=k-1; m>=0; m--){
                                path_table_vector[current].push_back(blossom[m]);
                            }


                            for(int c = 0;c<path_table_vector[current].size(); c++){
                                int check = path_table_vector[current][c];
                                for(int d = c+1; d<path_table_vector[current].size(); d++){
                                    if(check == path_table_vector[current][d]) {
                                        test = false;
                                        break;
                                    }
                                }
                            }

                            if(!test){
                                path_table_vector[current].clear();
                                continue;
                            }else{
                                local_vector.push_back(current);
                                is_even[current] = 1;
                            }

                        }
                    }
                }

            }
        }
    }

    {std::lock_guard<std::mutex> guard(blossom_mutex_lock);
        vector_2.insert(vector_2.end(),local_vector.begin(), local_vector.end());
    }

}


void parFindAugmentingPathNoRecursionUpdatePathTable(Graph& G, std::vector<int>& M, std::vector<std::vector<int>>& path_collection, std::vector<std::vector<int>>& path_table_vector, int num_of_threads){


    // Find all exposed nodes
    int nodes = static_cast<int>(G.rowOffsets.size()) - 1;
    std::vector<int> exposed;
    exposed.reserve(nodes);
    parExposedNode(exposed,M, num_of_threads);

    std::vector<int> is_even(nodes,0);                                  // Whether a node is even or not
    std::vector<int> belongs(nodes,-1);                                 // Which tree the node belongs to

    std::vector<std::atomic<int>> select_tree(nodes);
    std::vector<std::atomic<int>> select_match(nodes);
    std::vector<std::atomic<int>> select_blossom(nodes);

    parInitializeAtomicPathTable(select_tree, select_match, select_blossom, path_table_vector, nodes, num_of_threads);
    parInitializeExposed(exposed, is_even, belongs, num_of_threads);

    std::vector<int> nodes_vector;
    nodes_vector.reserve(nodes);
    nodes_vector = exposed;

    std::vector<int> vector_1;
    vector_1.reserve(nodes);

    std::vector<int> vector_2;
    vector_2.reserve(nodes);

    std::vector<std::thread> threads;
    threads.reserve(num_of_threads);
    std::vector<int> blossom_to_base(nodes,-1);


    while (true){

        auto start_augmenting_path = std::chrono::high_resolution_clock::now();
        for(int begin = 0; begin < num_of_threads; begin++){
            threads.emplace_back(parAugmentingPath, std::ref(G.rowOffsets), std::ref(G.columnIndices), std::ref(nodes_vector), begin, num_of_threads, std::ref(is_even),
                                 std::ref(belongs), std::ref(path_table_vector), std::ref(select_tree), std::ref(path_collection));
        }

        for (auto& thread : threads) {
            thread.join();
        }
        auto end_augmenting_path = std::chrono::high_resolution_clock::now();
        duration_augmenting_path = std::chrono::duration_cast<std::chrono::microseconds>(end_augmenting_path - start_augmenting_path) + duration_augmenting_path;

        if(!path_collection.empty()){
            return;
        }

        threads.clear();
        vector_1.clear();

        auto start_expand = std::chrono::high_resolution_clock::now();
        for(int begin = 0; begin < num_of_threads; begin++){
            threads.emplace_back(parExpand, std::ref(G.rowOffsets), std::ref(G.columnIndices), std::ref(nodes_vector), begin, num_of_threads, std::ref(is_even), std::ref(belongs),
                                 std::ref(path_table_vector), std::ref(vector_1), std::ref(select_match), std::ref(M));
        }

        for (auto& thread : threads) {
            thread.join();
        }
        auto end_expand = std::chrono::high_resolution_clock::now();
        duration_expand = std::chrono::duration_cast<std::chrono::microseconds>(end_expand - start_expand) + duration_expand;

        threads.clear();
        copy_vector_to_vector(nodes_vector, vector_1, vector_2);
        vector_2.clear();

        auto start_blossom = std::chrono::high_resolution_clock::now();
        for(int begin = 0; begin < num_of_threads; begin++){
            threads.emplace_back(parBlossom, std::ref(G.rowOffsets), std::ref(G.columnIndices), std::ref(nodes_vector), begin, num_of_threads, std::ref(is_even), std::ref(belongs),
                                 std::ref(path_table_vector), std::ref(vector_2), std::ref(select_blossom), std::ref(blossom_to_base), std::ref(M));
        }

        for (auto& thread : threads) {
            thread.join();
        }
        auto end_blossom = std::chrono::high_resolution_clock::now();
        duration_blossom = std::chrono::duration_cast<std::chrono::microseconds>(end_blossom - start_blossom) + duration_blossom;

        threads.clear();
        copy_vector_to_vector(nodes_vector, vector_1, vector_2);

        if(nodes_vector.empty()){
            break;
        }

    }

}
