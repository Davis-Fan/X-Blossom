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

/// \class Graph
/// \brief Lightweight CSR-based representation of an undirected graph.
///
/// The Graph class stores an undirected graph in compressed sparse row (CSR)
/// format and optionally provides an adjacency-list view for algorithms
/// that need it. Vertices are assumed to be labeled `0, 1, ..., n-1`.

class Graph {

public:

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    /// \brief Construct a graph from CSR arrays.
    ///
    /// \param csrRowOffsets
    ///     Row offset array of length `n + 1`. For each vertex `u`,
    ///     its neighbors are stored in `csrColumnIndices` in the range
    ///     `[csrRowOffsets[u], csrRowOffsets[u+1])`.
    ///
    /// \param csrColumnIndices
    ///     Column index array storing the adjacency lists of all vertices
    ///     concatenated together.
    ///
    /// The constructor copies the input arrays and sets `num_of_nodes` to
    /// `csrRowOffsets.size() - 1`.

    std::vector<int> columnIndices;
    std::vector<int> rowOffsets;

    Graph(const std::vector<int>& csrRowOffsets, const std::vector<int>& csrColumnIndices) {
        rowOffsets = csrRowOffsets;
        columnIndices = csrColumnIndices;
        num_of_nodes = static_cast<int>(rowOffsets.size()) - 1;
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


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    /// \brief Optional adjacency-list utilities (not required by X-Blossom API).

    std::unordered_map<int, std::list<int>> adjLists; // Adjacency lists with label as key
    std::vector<std::vector<int>> adjMatrix;
    int num_of_nodes = 0;

    Graph(int num_nodes){
        num_of_nodes = num_nodes;
    }
    Graph()= default;


    void addNode(int label) {
        adjLists[label];
    }

    // Add an edge
    void addEdge(int srcLabel, int destLabel) {
        addNode(srcLabel);
        addNode(destLabel);
        adjLists[srcLabel].push_back(destLabel);
        adjLists[destLabel].push_back(srcLabel);
        if(!adjMatrix.empty()){
            adjMatrix[srcLabel][destLabel] = 1;
            adjMatrix[destLabel][srcLabel] = 1;
        }
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
};


struct pair_hash {
    std::size_t operator()(const std::pair<int, int>& p) const {
        return std::hash<int>()(p.first) ^ std::hash<int>()(p.second);
    }
};

#endif //BLOSSOM_GRAPH_H
