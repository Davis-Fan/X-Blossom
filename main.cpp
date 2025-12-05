#include "blossom.h"
#include "graph.h"
#include "stopwatch.h"
std::chrono::microseconds duration_blossom;
std::chrono::microseconds duration_augmenting_path;
std::chrono::microseconds duration_expand;
std::chrono::microseconds duration_prepare;
std::chrono::microseconds duration_total(0);


/// \brief Test if a maximum matching valid or not.
void testMatching(std::vector<int>& M){
    bool is_valid = true;
    for(int i=0; i<M.size();i++){
        int k = M[i];
        if(k != -1 && M[k] != i){
            is_valid = false;
        }
    }
    std::set<std::pair<int, int>> M_set;
    for(int i=0; i<M.size(); i++){
        if(M[i] != -1){
            M_set.insert({std::min(i,M[i]), std::max(i, M[i])});
        }
    }

    if(is_valid){
        std::cout << "The matching is valid and the matching size is " << M_set.size() << std::endl;
    }else{
        std::cout << "The matching is NOT valid !!!" << std::endl;
    }
}



void readFileIntoVector(const std::string& filename, std::vector<int>& vec) {
    std::ifstream file(filename);
    if (!file) {
        std::cerr << "Error opening file: " << filename << std::endl;
        return;
    }
    int value;
    while (file >> value) {
        vec.push_back(value);
    }
    file.close();
}



int main(int argc, char* argv[]) {

    if (argc < 4) {
        std::cerr << "Usage: " << argv[0] << " <rowOffsets file path> <columnIndices file path> <number of threads>" << std::endl;
        return 1;
    }

    std::string rowOffsetsFilePath = argv[1];
    std::string columnIndicesFilePath = argv[2];
    std::vector<int> rowOffsets;
    std::vector<int> columnIndices;
    readFileIntoVector(rowOffsetsFilePath, rowOffsets);
    readFileIntoVector(columnIndicesFilePath, columnIndices);


    Graph G(rowOffsets, columnIndices); // Load rowOffset and columnIndices to build a graph
    int nodes = static_cast<int>(G.rowOffsets.size()) - 1;
    int edge_count = static_cast<int>(G.columnIndices.size());
    std::cout << "The graph has " << edge_count/2 << " edges " << std::endl;
    std::vector<int> M(nodes,-1);    // Build a vector to store matching

    int num_of_threads = 1;
    if(argc == 4){
        num_of_threads = std::stoi(argv[3]);
    }

    std::cout << "Test X-Blossom" << std::endl;
    test_x_blossom_maximum_matching(G, M, num_of_threads);
    testMatching(M);

    return 0;
}