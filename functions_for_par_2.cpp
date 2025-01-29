#include "blossom.h"
#include "graph.h"
extern int num_of_threads;
extern bool stop_immediately;

std::mutex even_mutex_lock;
void parFindEvenNode(std::vector<int>& is_even, std::vector<int>& node_vector, int start, int end) {
    std::vector<int> local_even;
    local_even.reserve(end - start);

    for(int i = start; i < end; ++i) {
        if (is_even[i]) {
            local_even.push_back(i);
        }
    }

    std::lock_guard<std::mutex> guard(even_mutex_lock);
    node_vector.insert(node_vector.end(), local_even.begin(), local_even.end());
}


void parEvenNode(std::vector<int>& is_even, std::vector<int>& node_vector) {
    std::vector<std::thread> threads;
    threads.reserve(num_of_threads);

    int node_size = is_even.size();
    int chunk_size = (node_size + num_of_threads - 1) / num_of_threads;

    for (int t = 0; t < num_of_threads; ++t) {
        int start = t * chunk_size;
        int end = std::min(start + chunk_size, node_size);
        threads.emplace_back(parFindEvenNode, std::ref(is_even), std::ref(node_vector), start, end);
    }

    for (auto& thread : threads) {
        thread.join();
    }
}



void find_blossom_vector(std::vector<int>& path_v, std::vector<int>& path_w, std::vector<int>& blossom){

    int s = path_v.size() - 1;
    int t = path_w.size() - 1;

    if(path_v.back() != path_w.back()){
        std::cout << "problem" << std::endl;
        stop_immediately = true;
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




std::list<int> compute_path_to_base_vector(std::list<int>& blossom, int index, std::list<int>::const_iterator& index_t){
    std::list<int> res;

    if(index % 2 == 0){
        for(auto it = blossom.begin(); it != index_t; it++){
            res.push_back(*it);
        }
    } else {
        for(auto it = std::prev(blossom.end()); it != index_t; it--){
            res.push_back(*it);
        }
    }
    return res;
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