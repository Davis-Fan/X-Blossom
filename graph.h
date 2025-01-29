#ifndef BLOSSOM_GRAPH_H
#define BLOSSOM_GRAPH_H

#include <iostream>
#include <list>
#include <unordered_map>
#include <queue>
#include <random>
#include <fstream>
#include <set>
#include <unordered_set>

class Graph {

public:

    std::unordered_map<int, std::list<int>> adjLists; // Adjacency lists with label as key

    ////////////////////////////////////////////////

    std::vector<std::vector<int>> adjMatrix;
    int num_of_nodes = 0;
    Graph(int num_nodes){
        num_of_nodes = num_nodes;
//        adjMatrix.resize(num_of_nodes, std::vector<int>(num_of_nodes, -1));
    }

    Graph(){}

    ////////////////////////////////////////////////



    void addNode(int label) {
//        if (adjLists.find(label) != adjLists.end()) {
//            return;
//        }
        adjLists[label];
    }


    // Add an edge
    void addEdge(int srcLabel, int destLabel) {
//        if (hasEdge(srcLabel,destLabel) || srcLabel == destLabel){
//            return;
//        }
        addNode(srcLabel);
        addNode(destLabel);
        adjLists[srcLabel].push_back(destLabel);
        adjLists[destLabel].push_back(srcLabel);


        ////////////////////////////////
        if(!adjMatrix.empty()){
            adjMatrix[srcLabel][destLabel] = 1;
            adjMatrix[destLabel][srcLabel] = 1;
        }
        ////////////////////////////////
    }


    // Remove an edge
    void removeEdge(int srcLabel, int destLabel) {
        if (hasEdge(srcLabel, destLabel)) {
            adjLists[srcLabel].remove(destLabel);
            adjLists[destLabel].remove(srcLabel);
        }
    }


    // Remove a node
    void removeNode(int label) {
        auto it = adjLists.find(label);
        if (it == adjLists.end()) {
            return;
        }

        for (auto& neighbor : it->second) {
            adjLists[neighbor].remove(label);
        }
        adjLists.erase(it);
    }


    // Check if an edge exists
    bool hasEdge(int srcLabel, int destLabel) {
        if (adjLists.find(srcLabel) != adjLists.end()) {
            for (auto& neighbor : adjLists[srcLabel]) {
                if (neighbor == destLabel) {
                    return true;
                }
            }
        }
        return false;
    }


    // Check how many nodes has no edge
    int countNodesWithNoEdges(){
        int count = 0;
        for (auto& pair : adjLists) {
            if (pair.second.empty()) { // If the list of adjacent nodes is empty
                ++count;
            }
        }
        return count;
    }


    // Check the number of nodes the graph has
    int countNodes() {
        return adjLists.size();
    }


    // Print graph
    void printGraph() {
        std::cout << std::endl;
        std::cout << "The Graph is shown as below: " << std::endl;
        for (auto& pair : adjLists) {
            std::cout << "Adjacency list of vertex " << pair.first << ": ";
            for (int& neighbor : pair.second) {
                std::cout << neighbor << " ";
            }
            std::cout << std::endl;
        }
        std::cout << std::endl;
    }


    // Find the shortest path
    std::list<int> findShortestPath(int srcLabel, int destLabel) {
        if (adjLists.find(srcLabel) == adjLists.end() ||
            adjLists.find(destLabel) == adjLists.end()) {
            return {};
        }

        std::queue<int> queue;
        std::unordered_map<int, int> predecessor;
        std::unordered_map<int, bool> visited;

        queue.push(srcLabel);
        visited[srcLabel] = true;
        predecessor[srcLabel] = -1; // -1 denotes no predecessor

        while (!queue.empty()) {
            int current = queue.front();
            queue.pop();

            if (current == destLabel) {
                break;
            }

            for (auto& neighbor : adjLists[current]) {
                if (!visited[neighbor]) {
                    queue.push(neighbor);
                    visited[neighbor] = true;
                    predecessor[neighbor] = current;
                }
            }
        }

        std::list<int> path;
        for (int at = destLabel; at != -1; at = predecessor[at]) {
            path.push_front(at);
        }
        return path;
    }


    // Contract one node to another
    void contractNodes(int nodeToContract, int intoNode) {
        if (adjLists.find(nodeToContract) == adjLists.end() || adjLists.find(intoNode) == adjLists.end()) {
//            std::cout << "One or both nodes do not exist." << std::endl;
            return;
        }

        for (auto neighbor : adjLists[nodeToContract]) {
            if (neighbor != intoNode) { // Avoid self-loop
                adjLists[neighbor].remove(nodeToContract);
                if (!hasEdge(intoNode,neighbor)){
                    adjLists[neighbor].push_back(intoNode);
                    adjLists[intoNode].push_back(neighbor);
                }
            }
        }
        removeNode(nodeToContract);
    }


    // Save a graph
    void saveGraphToFile(std::string& filename) {
        std::ofstream file(filename);
        if (!file.is_open()) {
            std::cout << "Failed to open file for writing." << std::endl;
            return;
        }

        for (auto& pair : adjLists) {
            int src = pair.first;
            for (int dest : pair.second) {
                if (src < dest) {
                    file << src << " " << dest << std::endl;
                }
            }
        }
        file.close();
    }


    // Load a stored graph
    void loadGraphFromFile(std::string& filename) {
        std::ifstream file(filename);
        if (!file.is_open()) {
            std::cout << "Failed to open file for reading." << std::endl;
            return;
        }

        int src, dest;
        while (file >> src >> dest) {
            addEdge(src, dest);
        }
        file.close();
    }


    //Generate a random graph
    void generateRandomGraph(int n, double density) {
        std::random_device rd;  // Obtain a random number from hardware
        std::mt19937 gen(rd()); // Seed the generator
        std::uniform_real_distribution<> dis(0.0, 1.0);

//        for (int i = 0; i < n; i++) {
//            addNode(i);
//        }

        for (int i = 0; i < n; ++i) {
            for (int j = i + 1; j < n; ++j) {
                if (dis(gen) < density) {
                    addEdge(i, j);
                }
            }
        }
    }


    // Visualization G and M
    void exportToDot(const std::string& filename, const std::set<std::pair<int, int>>& M) {
        std::ofstream file(filename);
        if (!file.is_open()) {
            std::cout << "Failed to open file for writing." << std::endl;
            return;
        }

        file << "graph G {" << std::endl;
        file << R"(graph [rankdir=LR, size="6,6", fontname="Arial", dpi=300];)" << std::endl;
        file << R"(node [shape=circle, color="#009ade", fontname="Arial"];)" << std::endl;
        file << R"(edge [color="#009ade"];)" << std::endl;

        for (auto& pair : adjLists) {
            for (int neighbor : pair.second) {
                if (pair.first < neighbor) {
                    if (M.find(std::make_pair(pair.first, neighbor)) != M.end()) {
                        file << "  " << pair.first << " -- " << neighbor << " [color=red];" << std::endl;
                    } else {
                        file << "  " << pair.first << " -- " << neighbor << ";" << std::endl;
                    }
                }
            }
        }

        for (auto& pair : adjLists) {
            if (pair.second.empty()) {
                file << "  " << pair.first << ";" << std::endl;
            }
        }

        file << "}" << std::endl;
    }

    // Get a set of all nodes
    std::set<int> getAllNodes(){
        std::set<int> nodes;
        for (auto& pair : adjLists) {
            nodes.insert(pair.first); // Insert the node itself
        }
        return nodes;
    }

    /////////////////////////////////////////////////////////////

    std::unordered_set<int> parGetAllNodes(){
        std::unordered_set<int> nodes;
        for (auto& pair : adjLists) {
            nodes.insert(pair.first); // Insert the node itself
        }
        return nodes;
    }

    /////////////////////////////////////////////////////////////

    std::unordered_set<int> seqGetAllNodes(){
        std::unordered_set<int> nodes;
        for (auto& pair : adjLists) {
            nodes.insert(pair.first); // Insert the node itself
        }
        return nodes;
    }

    //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // 11-5-2024

    std::vector<int> columnIndices;
    std::vector<int> rowOffsets;

    Graph(const std::vector<int>& csrRowOffsets, const std::vector<int>& csrColumnIndices) {
        rowOffsets = csrRowOffsets;
        columnIndices = csrColumnIndices;
        num_of_nodes = rowOffsets.size() - 1;
    }

    void buildAdjListFromCSR() {
        adjLists.clear();

        for (int i = 0; i < num_of_nodes; ++i) {
            for (int j = rowOffsets[i]; j < rowOffsets[i + 1]; ++j) {
                int neighbor = columnIndices[j];
                adjLists[i].push_back(neighbor);
            }
        }
    }

    void buildCSRFromAdjList() {
        rowOffsets.clear();
        columnIndices.clear();
        rowOffsets.push_back(0);

        int edgeCount = 0;

        if(num_of_nodes == 0){
            std::cout << "ERROR: size = 0" << std::endl;
        }

        for (int i = 0; i < num_of_nodes; ++i) {
            if (adjLists.find(i) != adjLists.end()) {
                for (int neighbor : adjLists[i]) {
                    columnIndices.push_back(neighbor);
                    edgeCount++;
                }
            }
            rowOffsets.push_back(edgeCount);
        }
    }

    void printCSR(){
        std::cout << "rowOffsets: ";
        for (int val : rowOffsets) {
            std::cout << val << " ";
        }
        std::cout << std::endl;

        std::cout << "columnIndices: ";
        for (int val : columnIndices) {
            std::cout << val << " ";
        }
        std::cout << std::endl;
    }


    std::unordered_set<int> parGetAllNodesCSR(){
        std::unordered_set<int> nodes;
        if(num_of_nodes == 0){
            std::cout << "ERROR: No Nodes" << std::endl;
            return nodes;
        }

        for(int i=0; i<num_of_nodes; i++){
            nodes.insert(i);
        }
        return nodes;
    }




    //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


    /////////////////////////////////////////////////////////////

    // Get a vector of all edges to a node
    std::vector<std::pair<int, int>> getEdgesForNode(int i) {
        std::vector<std::pair<int, int>> edges;
        auto it = adjLists.find(i);
        if (it != adjLists.end()) {
            for (int neighbor : it->second) {
                edges.emplace_back(i, neighbor);
            }
        }
        return edges;
    }



};


struct pair_hash {
    std::size_t operator()(const std::pair<int, int>& p) const {
        return std::hash<int>()(p.first) ^ std::hash<int>()(p.second);
    }
};

#endif //BLOSSOM_GRAPH_H
