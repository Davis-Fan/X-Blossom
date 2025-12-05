#include "blossom.h"
#include "graph.h"
#include "stopwatch.h"

/// \brief Benchmark wrapper for x_blossom_maximum_matching.
///
/// This helper runs `x_blossom_maximum_matching` on the given graph,
/// measures the time spent in the main phases of the algorithm, and
/// prints timing statistics to `std::cout`.
///
/// This function is intended for **experiments and performance evaluation**,
/// not as the primary API for integrating X-Blossom into other applications.
/// For library-style use, call `x_blossom_maximum_matching` directly.
///
/// \param G
///     Input graph, stored in CSR format. Vertices are labeled
///     `0, 1, ..., n-1`, where `n = G.rowOffsets.size() - 1`.
///
/// \param M
///     Matching vector. Inside this function, `M` is resized to `n` and
///     initialized to `-1` before calling `x_blossom_maximum_matching`.
///     On return, `M[v]` is the mate of vertex `v`, or `-1` if unmatched.
///
/// \param num_of_threads
///     Number of threads to use in `x_blossom_maximum_matching`.
///
/// \note
///     - The number of iterations is currently fixed to 1 by the local
///       variable `iteration`.
///     - This function uses global timing variables
///       (`duration_prepare`, `duration_blossom`,
///        `duration_augmenting_path`, `duration_expand`, `duration_total`)
///     - Timing information is printed to `std::cout`.
void test_x_blossom_maximum_matching(Graph& G, std::vector<int>& M, int num_of_threads){

    int iteration = 1;  // set the running iteration
    int nodes = static_cast<int>(G.rowOffsets.size()) - 1;

    for(int i=0;i<iteration;i++) {

        M = std::vector<int>(nodes, -1);
        auto start_total_f = std::chrono::high_resolution_clock::now();
        auto pre_1 = duration_prepare;
        auto pre_2 = duration_blossom;
        auto pre_3 = duration_augmenting_path;
        auto pre_4 = duration_expand;

        x_blossom_maximum_matching(G, M, num_of_threads);

        auto end_total_f = std::chrono::high_resolution_clock::now();
        auto period_f = std::chrono::duration_cast<std::chrono::microseconds>(end_total_f - start_total_f);
        duration_total = period_f + duration_total;
        std::cout << "No." << i << " iteration; " <<"The computation time: " << (duration_blossom-pre_2+duration_expand-pre_4+duration_augmenting_path-pre_3).count()/(1000.0) << " milliseconds" << std::endl;
        std::cout << "*************************************"<< std::endl;
    }

    std::cout << "# of threads: " << num_of_threads << std::endl;
    std::cout << "The average computation taken: " << (duration_blossom.count()+duration_expand.count()+duration_augmenting_path.count())/(1000.0*iteration) << " milliseconds" << std::endl;
}
