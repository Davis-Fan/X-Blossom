#include "blossom.h"
#include "graph.h"
extern int num_of_threads;


void parCreateNewMatching(std::vector<int>& M, std::list<int>& path){
    int edgeIndex = 0;
    for (auto it = path.begin(); std::next(it) != path.end(); ++it) {
        int node1 = *it;
        int node2 = *std::next(it);

        if (edgeIndex % 2 == 0) {
            M[node1] = node2;
            M[node2] = node1;
        }

        ++edgeIndex;
    }
}

void parCreateNewMatchingNoRecursion(std::vector<int>& M, std::vector<std::list<int>>& paths){
    for (auto& path: paths){
        parCreateNewMatching(M, path);
    }
}


void parCreateNewMatchingNo(std::vector<int>& M, std::vector<std::list<int>>& paths, int index, int num_threads){
    for(int i = index; i<paths.size(); i+=num_threads){
        parCreateNewMatching(M, paths[i]);
    }
}

void parNewMatching(std::vector<int>& M, std::vector<std::list<int>>& paths){
    std::vector<std::thread> threads;
    threads.reserve(num_of_threads);
    for(int begin=0; begin < num_of_threads; begin++){
        threads.emplace_back(parCreateNewMatchingNo, std::ref(M), std::ref(paths), begin, num_of_threads);
    }
    for (auto& thread : threads) {
        thread.join();
    }
}


std::unordered_set<int> parGetNodesNotInMatching(Graph& G, std::vector<int>& M) {

    std::unordered_set<int> nodesNotInM = G.parGetAllNodes();
    std::unordered_set<int> nodesToErase;

    for(auto& i:nodesNotInM){
        if(M[i] != -1){
            nodesToErase.insert(i);
        }
    }

    for (auto& node : nodesToErase) {
        nodesNotInM.erase(node);
    }
    return nodesNotInM;
}


std::unordered_set<int> parGetNodesNotInMatchingCSR(Graph& G, std::vector<int>& M) {
    std::unordered_set<int> nodesNotInM = G.parGetAllNodesCSR();
    for (auto it = nodesNotInM.begin(); it != nodesNotInM.end(); ) {
        if (M[*it] != -1) {
            it = nodesNotInM.erase(it); // Erase and get the next iterator
        } else {
            ++it; // Move to the next element
        }
    }
    return nodesNotInM;
}


int parGetMatchingNode(int& inputNode, std::vector<int>& M) {
    int matching_node = M[inputNode];
    if(matching_node != -1){
        return matching_node;
    }
    return -1;
}