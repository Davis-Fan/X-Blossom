#ifndef BLOSSOM_BLOSSOM_H
#define BLOSSOM_BLOSSOM_H
#include "graph.h"
#include <iostream>
#include <list>
#include <unordered_map>
#include <queue>
#include <random>
#include <fstream>
#include <set>
#include <stack>
#include <chrono>
#include <thread>
#include <mutex>
#include <unordered_set>
#include <shared_mutex>
#include <climits>
#include <algorithm>
#include <atomic>


void createNewMatching(std::set<std::pair<int, int>>& M, std::list<int>& path);
std::set<int> getNodesNotInMatching(Graph& G, std::set<std::pair<int, int>>& M);
bool isEdgeInMatching(int node1, int node2, std::set<std::pair<int, int>>& M);
bool isEdgeVisited(std::pair<int, int>& edge, std::set<std::pair<int, int>>& visitedEdges);
void addEdgeVisited(std::pair<int, int>& edge, std::set<std::pair<int, int>>& visitedEdges);
int getMatchingNode(int& inputNode, const std::set<std::pair<int, int>>& M);
bool findAndSplit(std::list<int>& augmentingPath, int element, std::list<int>& L_stem, std::list<int>& R_stem);
std::set<std::pair<int, int>> contractMatching(std::set<std::pair<int, int>>& M, std::list<int>& blossom, int w);
std::list<int> findBaseAndRearrange(std::list<int>& blossom, std::set<std::pair<int, int>>& M);
bool isValidMatching(Graph& G, std::set<std::pair<int, int>>& M);
void createNewMatchingNoRecursion(std::set<std::pair<int, int>>& M, std::list<std::list<int>>& paths);
void addBlossomEdgesToVisited(std::list<int>& based_blossom, std::set<std::pair<int, int>>& visitedEdges);
void printNodesToCheck(const std::vector<std::list<int>>& nodes_to_check);
void printNodesVector(const std::vector<int>& nodes_vector);


////////////////////////////////////////////////////////////////////////////////////////////////
// Some functions in parallel non-recursion solution

std::unordered_set<int> parGetNodesNotInMatchingCSR(Graph& G, std::vector<int>& M);
void parNewMatchingVector(std::vector<int>& M, std::vector<std::vector<int>>& path_collection);
void parExposedNode(std::vector<int>& exposed, std::vector<int>& M);
void parInitializeExposed(const std::vector<int>& exposed, std::vector<int>& is_even, std::vector<int>& belongs, int num_threads);
std::vector<int> find_path_vector(const std::vector<std::vector<int>>& path_table, int v);
void testMatching(std::vector<int>& M);
void parInitializeAtomicPathTable(std::vector<std::atomic<int>>& select_tree,
                           std::vector<std::atomic<int>>& select_match,
                           std::vector<std::atomic<int>>& select_blossom,
                           std::vector<std::vector<int>>& path_table_vector,
                           int nodes, int num_threads);

////////////////////////////////////////////////////////////////////////////////////////////////

void copy_vector_to_vector(std::vector<int>& nodes_vector, const std::vector<int>& vector_1, const std::vector<int>& vector_2);
std::vector<int> find_path_vector_blossom(const std::vector<std::vector<int>>& path_table, int v);
void print_path_vector_blossom(const std::vector<std::vector<int>>& path_table, int v, bool& valid);
std::vector<int> find_path_vector_blossom_w(const std::vector<std::vector<int>>& path_table, int v, std::vector<int>& belongs, bool& consistent_flag);
void find_blossom_vector_debug(std::vector<int>& path_v, std::vector<int>& path_w, std::vector<int>& blossom, std::vector<std::vector<int>>& path_table_vector, bool& valid_flag);
void testParBlossom_200(Graph& G, std::vector<int>& M, int& threshold);
void parFindMaximumMatchingNoRecursionUpdatePathTable_200(Graph& G, std::vector<int>& M, bool& valid_M, int& threshold);
void parFindAugmentingPathNoRecursionUpdatePathTable_200(Graph& G, std::vector<int>& M, std::vector<std::vector<int>>& path_collection, std::vector<std::vector<int>>& path_table_vector);

////////////////////////////////////////////////////////////////////////////////////////////////

#endif
