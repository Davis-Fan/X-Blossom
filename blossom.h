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


extern std::chrono::microseconds duration_blossom;
extern std::chrono::microseconds duration_augmenting_path;
extern std::chrono::microseconds duration_expand;
extern std::chrono::microseconds duration_prepare;
extern std::chrono::microseconds duration_total;



void parNewMatchingVector(std::vector<int>& M, std::vector<std::vector<int>>& path_collection, int num_of_threads);
void parExposedNode(std::vector<int>& exposed, std::vector<int>& M, int num_of_threads);
void parInitializeExposed(const std::vector<int>& exposed, std::vector<int>& is_even, std::vector<int>& belongs, int num_threads);
std::vector<int> find_path_vector(const std::vector<std::vector<int>>& path_table, int v);
void testMatching(std::vector<int>& M);
void parInitializeAtomicPathTable(std::vector<std::atomic<int>>& select_tree, std::vector<std::atomic<int>>& select_match, std::vector<std::atomic<int>>& select_blossom, std::vector<std::vector<int>>& path_table_vector, int nodes, int num_threads);
void printNodesVector(const std::vector<int>& nodes_vector);

void copy_vector_to_vector(std::vector<int>& nodes_vector, const std::vector<int>& vector_1, const std::vector<int>& vector_2);
std::vector<int> find_path_vector_blossom(const std::vector<std::vector<int>>& path_table, int v);
std::vector<int> find_path_vector_blossom_w(const std::vector<std::vector<int>>& path_table, int v, std::vector<int>& belongs, bool& consistent_flag);
void find_blossom_vector_debug(std::vector<int>& path_v, std::vector<int>& path_w, std::vector<int>& blossom, std::vector<std::vector<int>>& path_table_vector, bool& valid_flag);
void test_x_blossom_maximum_matching(Graph& G, std::vector<int>& M, int num_threads);
void x_blossom_maximum_matching(Graph& G, std::vector<int>& M, int num_threads);    /// Main API function
void parFindAugmentingPathNoRecursionUpdatePathTable(Graph& G, std::vector<int>& M, std::vector<std::vector<int>>& path_collection, std::vector<std::vector<int>>& path_table_vector, int num_of_threads);

#endif
