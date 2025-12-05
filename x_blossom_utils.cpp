#include "blossom.h"
#include "graph.h"


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


void parNewMatchingVector(std::vector<int>& M, std::vector<std::vector<int>>& path_collection, int num_of_threads){
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


void parExposedNode(std::vector<int>& exposed, std::vector<int>& M, int num_of_threads) {
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
        if(test >= path_table.size()){
            std::cout << test << std::endl;
            std::cout << "infinite loop New V = " <<new_v << std::endl;
            printNodesVector(path_table[new_v]);
            return {};
        }
    }

    return path;
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


void copy_vector_to_vector(std::vector<int>& nodes_vector, const std::vector<int>& vector_1, const std::vector<int>& vector_2) {
    nodes_vector.clear();
    nodes_vector.insert(nodes_vector.end(), vector_1.begin(), vector_1.end());
    nodes_vector.insert(nodes_vector.end(), vector_2.begin(), vector_2.end());
}


void find_blossom_vector_debug(std::vector<int>& path_v, std::vector<int>& path_w, std::vector<int>& blossom, std::vector<std::vector<int>>& path_table_vector, bool& valid_flag){

    int s = path_v.size() - 1;
    int t = path_w.size() - 1;

    if(path_v.back() != path_w.back()){
        valid_flag = false;
        std::cout << "problem+++" << std::endl;
        std::cout << "v = " << path_v.front() << std::endl;
        std::cout << "w = " << path_w.front() << std::endl;
        std::cout << "+++++++++++++++++++++++++++++++++++" << std::endl;
        printNodesVector(path_v);
        printNodesVector(path_w);
        std::cout << "+++++++++++++++++++++++++++++++++++" << std::endl;
        std::cout << "-----------------------------------" << std::endl;
        printNodesVector(path_table_vector[path_v.front()]);
        printNodesVector(path_table_vector[path_w.front()]);
        std::cout << "-----------------------------------" << std::endl;
        return;
    }

    for(; s>=0 && t>=0; s--, t--){
        if(path_v[s] != path_w[t]){
            break;
        }
    }

    for(int i = s+1; i>=0; i--){
        blossom.push_back(path_v[i]);
    }

    for(int j = 0; j<=t+1; j++){
        blossom.push_back(path_w[j]);
    }
}

void printNodesVector(const std::vector<int>& nodes_vector) {
    std::cout << "////////////////////////" << std::endl;
    for (const int& node : nodes_vector) {
        std::cout << node << " ";
    }
    std::cout << std::endl;
    std::cout << "////////////////////////" << std::endl;
    std::cout << std::endl;
}