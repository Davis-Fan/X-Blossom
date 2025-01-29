#include "blossom.h"
#include "graph.h"


// Update matching according to path
void createNewMatching(std::set<std::pair<int, int>>& M, std::list<int>& path){
    int edgeIndex = 0;
    for (auto it = path.begin(); it != path.end() && std::next(it) != path.end(); ++it) {
        int node1 = *it;
        int node2 = *std::next(it);

        if (edgeIndex % 2 == 1) {
            if(node1 < node2){
                M.erase({node1, node2});
            } else {
                M.erase({node2, node1});
            }
        } else {
            if(node1 < node2){
                M.insert({node1, node2});
            } else{
                M.insert({node2, node1});
            }
        }
        ++edgeIndex;
    }
}

void createNewMatchingNoRecursion(std::set<std::pair<int, int>>& M, std::list<std::list<int>>& paths){
    for (auto& path: paths){
        createNewMatching(M, path);
    }
}


// All exposed nodes
std::set<int> getNodesNotInMatching(Graph& G, std::set<std::pair<int, int>>& M) {
    std::set<int> nodesNotInM = G.getAllNodes();
    for (auto& edge : M) {
        nodesNotInM.erase(edge.first);  // Remove the first node of the edge from the set
        nodesNotInM.erase(edge.second); // Remove the second node of the edge from the set
    }
    return nodesNotInM;
}


// Whether an edge in matching or not
bool isEdgeInMatching(int node1, int node2, std::set<std::pair<int, int>>& M) {
    if (node1 > node2) {
        std::swap(node1, node2);
    }
    return M.find(std::make_pair(node1, node2)) != M.end();
}


bool isEdgeVisited(std::pair<int, int>& edge, std::set<std::pair<int, int>>& visitedEdges){
    if (visitedEdges.find({std::min(edge.first, edge.second), std::max(edge.first, edge.second)}) == visitedEdges.end()) {
        return false;
    }
    return true;
}


void addEdgeVisited(std::pair<int, int>& edge, std::set<std::pair<int, int>>& visitedEdges){
    visitedEdges.insert({std::min(edge.first, edge.second), std::max(edge.first, edge.second)});
}


void addBlossomEdgesToVisited(std::list<int>& based_blossom, std::set<std::pair<int, int>>& visitedEdges) {
    for (auto it1 = based_blossom.begin(); std::next(it1) != based_blossom.end(); ++it1) {
        auto it2 = it1;
        ++it2;
        for (; std::next(it2) != based_blossom.end(); ++it2) {
            int node1 = *it1;
            int node2 = *it2;
            std::pair<int, int> edge = (node1 < node2) ? std::make_pair(node1, node2) : std::make_pair(node2, node1);
            addEdgeVisited(edge, visitedEdges);
        }
    }
}


int getMatchingNode(int& inputNode, const std::set<std::pair<int, int>>& M) {
    for (auto& edge : M) {
        if (edge.first == inputNode) {
            return edge.second;
        } else if (edge.second == inputNode) {
            return edge.first;
        }
    }
    return -1;
}


bool findAndSplit(std::list<int>& augmentingPath, int element, std::list<int>& L_stem, std::list<int>& R_stem) {
    auto it = std::find(augmentingPath.begin(), augmentingPath.end(), element);
    if (it == augmentingPath.end()) {
        return false;
    }

    L_stem.assign(augmentingPath.begin(), it);
    R_stem.assign(std::next(it), augmentingPath.end());

    return true;
}


std::set<std::pair<int, int>> contractMatching(std::set<std::pair<int, int>>& M, std::list<int>& blossom, int w) {

    std::set<std::pair<int, int>> contractedM;
    std::set<int> blossomSet(blossom.begin(), blossom.end());

    for (const auto& edge : M) {
        bool firstInBlossom = blossomSet.find(edge.first) != blossomSet.end();
        bool secondInBlossom = blossomSet.find(edge.second) != blossomSet.end();
        if (firstInBlossom && secondInBlossom) {
            continue;
        }
        if (firstInBlossom) {
            contractedM.insert({std::min(w, edge.second), std::max(w, edge.second)});
        } else if (secondInBlossom) {
            contractedM.insert({std::min(w,edge.first), std::max(w, edge.first)});
        } else {
            contractedM.insert(edge);
        }
    }
    return contractedM;
}


std::list<int> findBaseAndRearrange(std::list<int>& blossom, std::set<std::pair<int, int>>& M) {
    std::list<int> basedBlossom;
    int base = -1;

    // Find the base
    for (auto it = blossom.begin(); it != std::prev(std::prev(blossom.end())); ++it) {
        auto nextIt = std::next(it);
        auto nextNextIt = std::next(nextIt);

        bool firstEdgeNotInM = M.find({std::min(*it, *nextIt), std::max(*it, *nextIt)}) == M.end();
        bool secondEdgeNotInM = M.find({std::min(*nextIt, *nextNextIt), std::max(*nextIt, *nextNextIt)}) == M.end();

        if (firstEdgeNotInM && secondEdgeNotInM) {
            base = *nextIt;
            break;
        }
    }

    // Rearrange blossom
    if (base != -1) {
        auto baseIt = std::find(blossom.begin(), blossom.end(), base);
        if (baseIt != blossom.end()) {
            basedBlossom.insert(basedBlossom.end(), baseIt, std::prev(blossom.end()));
            basedBlossom.insert(basedBlossom.end(), blossom.begin(), baseIt);
            basedBlossom.push_back(base);
        }
    } else {
        basedBlossom.insert(basedBlossom.end(),blossom.begin(),blossom.end());
    }

    return basedBlossom;
}



bool isValidMatching(Graph& G, std::set<std::pair<int, int>>& M) {
    std::set<int> matchedVertices;

    for (const auto& edge : M) {
        int u = edge.first;
        int v = edge.second;
        // Check if the edge exists in G
        if (!G.hasEdge(u, v) || !G.hasEdge(v, u)) {
            std::cout << "Edge does not exist in the graph: " << u << " - " << v << std::endl;
            return false;
        }
        // Check if vertices are already matched
        if (matchedVertices.find(u) != matchedVertices.end() || matchedVertices.find(v) != matchedVertices.end()) {
            std::cout << "Vertex already matched: " << u << " or " << v << std::endl;
            return false;
        }

        matchedVertices.insert(u);
        matchedVertices.insert(v);
    }

    return true; // The matching is valid
}

void printNodesToCheck(const std::vector<std::list<int>>& nodes_to_check) {
    std::cout << "////////////////////////" << std::endl;
    for (size_t i = 0; i < nodes_to_check.size(); ++i) {
        std::cout << "List " << i + 1 << ": ";
        // Iterate through each list in the vector
        for (const int& value : nodes_to_check[i]) {
            std::cout << value << " ";
        }
        std::cout << std::endl;
    }
    std::cout << "////////////////////////" << std::endl;
    std::cout << std::endl;
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