#include "blossom.h"
#include "graph.h"
extern int num_of_threads;
extern int nodes;


void parCreateNewMatchingVector(std::vector<int>& M, std::vector<std::vector<int>>& path_collection, int index, int num_threads){
    for(int i = index; i<path_collection.size(); i+=num_threads){
        const std::vector<int>& path = path_collection[i];
        int path_size = path.size();

        for(int j=0;j<path_size; j+=2){
            int node1 = path[j];
            int node2 = path[j+1];
            M[node1] = node2;
            M[node2] = node1;
        }
    }
}


void parNewMatchingVector(std::vector<int>& M, std::vector<std::vector<int>>& path_collection){
    std::vector<std::thread> threads;
    threads.reserve(num_of_threads);
    for(int begin=0; begin < num_of_threads; begin++){
        threads.emplace_back(parCreateNewMatchingVector, std::ref(M), std::ref(path_collection), begin, num_of_threads);
    }
    for (auto& thread : threads) {
        thread.join();
    }
}


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
std::mutex exposed_mutex_lock;
void parFindExposedNode(std::vector<int>& exposed, std::vector<int>& M, int start, int end) {
    std::vector<int> local_exposed;
    local_exposed.reserve(end - start);

    for(int i = start; i < end; ++i) {
        if (M[i] == -1) {
            local_exposed.push_back(i);
        }
    }

    std::lock_guard<std::mutex> guard(exposed_mutex_lock);
    exposed.insert(exposed.end(), local_exposed.begin(), local_exposed.end());
}


void parExposedNode(std::vector<int>& exposed, std::vector<int>& M) {
    std::vector<std::thread> threads;
    threads.reserve(num_of_threads);

    int node_size = M.size();
    int chunk_size = (node_size + num_of_threads - 1) / num_of_threads;

    for (int t = 0; t < num_of_threads; ++t) {
        int start = t * chunk_size;
        int end = std::min(start + chunk_size, node_size);
        threads.emplace_back(parFindExposedNode, std::ref(exposed), std::ref(M), start, end);
    }

    for (auto& thread : threads) {
        thread.join();
    }
}


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


void initialize_range(std::vector<std::atomic<int>>& vec1,
                      std::vector<std::atomic<int>>& vec2,
                      std::vector<std::atomic<int>>& vec3,
                      int start, int end) {
    for (int i = start; i < end; ++i) {
        vec1[i] = 0;
        vec2[i] = 0;
        vec3[i] = 0;
    }
}


void parInitializeAtomic(std::vector<std::atomic<int>>& select_tree,
                         std::vector<std::atomic<int>>& select_match,
                         std::vector<std::atomic<int>>& select_blossom,
                         int nodes, int num_threads) {
    std::vector<std::thread> threads;
    int chunk_size = (nodes + num_threads - 1) / num_threads;

    for (int t = 0; t < num_threads; ++t) {
        int start = t * chunk_size;
        int end = std::min(start + chunk_size, nodes);
        threads.emplace_back(initialize_range, std::ref(select_tree), std::ref(select_match), std::ref(select_blossom), start, end);
    }

    for (auto& thread : threads) {
        thread.join();
    }
}


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void initialize_range_2(std::vector<std::atomic<int>>& vec1,
                      std::vector<std::atomic<int>>& vec2,
                      std::vector<std::atomic<int>>& vec3,
                        std::vector<std::vector<int>>& path_table_vector,
                      int start, int end) {
    for (int i = start; i < end; ++i) {
        vec1[i] = 0;
        vec2[i] = 0;
        vec3[i] = 0;
        path_table_vector[i].clear();
    }
}

void parInitializeAtomicPathTable(std::vector<std::atomic<int>>& select_tree,
                         std::vector<std::atomic<int>>& select_match,
                         std::vector<std::atomic<int>>& select_blossom,
                         std::vector<std::vector<int>>& path_table_vector,
                         int nodes, int num_threads) {
    std::vector<std::thread> threads;
    int chunk_size = (nodes + num_threads - 1) / num_threads;

    for (int t = 0; t < num_threads; ++t) {
        int start = t * chunk_size;
        int end = std::min(start + chunk_size, nodes);
        threads.emplace_back(initialize_range_2, std::ref(select_tree), std::ref(select_match), std::ref(select_blossom), std::ref(path_table_vector), start, end);
    }

    for (auto& thread : threads) {
        thread.join();
    }
}


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void parProcessExposed(const std::vector<int>& exposed, std::vector<int>& is_even, std::vector<int>& belongs, int start, int end){
    for (int j = start; j < end; ++j) {
        int current_node = exposed[j];
        is_even[current_node] = 1;
        belongs[current_node] = current_node;
    }
}

void parInitializeExposed(const std::vector<int>& exposed, std::vector<int>& is_even, std::vector<int>& belongs, int num_threads){
    std::vector<std::thread> threads;
    int chunk_size = (static_cast<int>(exposed.size())+ num_threads - 1) / num_threads;

    for (int t = 0; t < num_threads; ++t) {
        int start = t * chunk_size;
        int end = std::min(start + chunk_size, static_cast<int>(exposed.size()));
        threads.emplace_back(parProcessExposed, std::ref(exposed), std::ref(is_even), std::ref(belongs), start, end);
    }

    for (auto& thread : threads) {
        thread.join();
    }

}


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int count_shared_elements_2(const std::vector<std::vector<int>>& path_collection) {
    std::unordered_map<int, int> element_counts;
    for (const auto& path : path_collection) {
        for (int node : path) {
            element_counts[node]++;
        }
    }

    int shared_count = 0;
    for (const auto& entry : element_counts) {
        if (entry.second > 1) {
            shared_count++;
        }
    }

    return shared_count;
}

void print_shared_paths(const std::vector<std::vector<int>>& paths) {
    std::unordered_map<int, int> element_to_path_index;

    for (size_t i = 0; i < paths.size(); ++i) {
        const auto& path = paths[i];
        for (int node : path) {
            if (element_to_path_index.find(node) != element_to_path_index.end()) {
                // Element already seen, print both paths
                int first_path_index = element_to_path_index[node];

                std::cout << "Shared element: " << node << "\n";
                std::cout << "First path: ";
                for (int val : paths[first_path_index]) {
                    std::cout << val << " ";
                }
                std::cout << "\n";

                std::cout << "Second path: ";
                for (int val : path) {
                    std::cout << val << " ";
                }
                std::cout << "\n";

                return;  // Stop after finding the first shared element
            } else {
                // Record the path index where this element is first seen
                element_to_path_index[node] = i;
            }
        }
    }

    std::cout << "No shared elements found between any two paths.\n";
}




//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

std::vector<int> find_path_vector(const std::vector<std::vector<int>>& path_table, int v) {
    std::vector<int> path;
    path.push_back(v);
    int test = 0;
    if (path_table[v].empty()) {
        return path;
    }

    int new_v = v;
    while (!path_table[new_v].empty()) {
        path.insert(path.end(), path_table[new_v].begin(), path_table[new_v].end());
        new_v = path.back();
    }

    return path;
}


std::vector<int> find_path_vector_blossom(const std::vector<std::vector<int>>& path_table, int v) {
    std::vector<int> path;
    path.push_back(v);
    int test = 0;
    if (path_table[v].empty()) {
        return path;
    }

    int new_v = v;
    while (!path_table[new_v].empty()) {
        path.insert(path.end(), path_table[new_v].begin(), path_table[new_v].end());
        new_v = path.back();
        test++;
        if(test >= nodes){
            std::cout << test << std::endl;
            std::cout << "infinite loop New V = " <<new_v << std::endl;
            printNodesVector(path_table[new_v]);
            return {};
        }
    }

    return path;
}


void print_path_vector_blossom(const std::vector<std::vector<int>>& path_table, int v, bool& valid) {
    int test = 0;
    if (path_table[v].empty()) {
        std::cout << "Nothing in Path-Table" << std::endl;
    }

    int new_v = v;
    while (!path_table[new_v].empty()) {
        std::cout << "new_v = " << new_v << "-";
        std::cout << std::endl;
        new_v = path_table[new_v].back();
        test++;
        if(test >= nodes){
            valid = false;
            std::cout << test << std::endl;
            std::cout << "Blossom bool valid = " <<valid << std::endl;
            std::cout << "infinite loop New V = " <<new_v << std::endl;
            printNodesVector(path_table[new_v]);
            break;
        }
    }
}


std::vector<int> find_path_vector_blossom_w(const std::vector<std::vector<int>>& path_table, int v, std::vector<int>& belongs, bool& consistent_flag) {
    std::vector<int> path;
    path.push_back(v);

    if (path_table[v].empty()) {
        if(v == belongs[v]){
            return path;
        } else {
            consistent_flag = false;
            return {};
        }
    }

    int new_v = v;
    int root = belongs[v];

    while (!path_table[new_v].empty()) {
        path.insert(path.end(), path_table[new_v].begin(), path_table[new_v].end());
        new_v = path.back();

        if(belongs[new_v] != root){
            consistent_flag = false;
            return {};
        }

    }

    return path;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

std::vector<int> find_path_vector_on_tree(const std::vector<std::vector<int>>& path_table, int v, Graph& v_tree, int root_of_v) {

    std::vector<int> path;
    path.push_back(v);

    if(path.back() == root_of_v){
        return path;
    }

    int new_v = v;
    while (!path_table[new_v].empty()) {
        path.insert(path.end(), path_table[new_v].begin(), path_table[new_v].end());
        new_v = path.back();
    }

    if(new_v != root_of_v){
        auto path_v = v_tree.findShortestPath(new_v, root_of_v);
        for (auto it = path_v.begin(); it != path_v.end(); ++it) {
            if (*it == new_v) {
                continue;
            }
            path.push_back(*it);
        }
    }

    return path;

}