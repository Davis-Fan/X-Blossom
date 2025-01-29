#include "blossom.h"
#include "graph.h"
#include "stopwatch.h"
extern int nodes;
extern int num_of_threads;
extern int count;
extern std::chrono::microseconds duration_blossom;
extern std::chrono::microseconds duration_augmenting_path;
extern std::chrono::microseconds duration_expand;
extern std::chrono::microseconds duration_edge;
extern std::chrono::microseconds duration_prepare;
extern std::chrono::microseconds duration_update;
extern std::chrono::microseconds duration_total;
bool stop_immediately = false;


void testParBlossom_200(Graph& G, std::vector<int>& M, int& threshold){

    int iteration = 20;
    count = 0;
    int valid_iteration = iteration;

    for(int i=0;i<iteration;i++) {

        bool valid_M = true;

        auto start_total_f = std::chrono::high_resolution_clock::now();
        auto pre_1 = duration_prepare;
        auto pre_2 = duration_blossom;
        auto pre_3 = duration_augmenting_path;
        auto pre_4 = duration_expand;

        parFindMaximumMatchingNoRecursionUpdatePathTable_200(G, M, valid_M, threshold);

        if(!valid_M){
            duration_prepare = pre_1;
            duration_blossom = pre_2;
            duration_augmenting_path = pre_3;
            duration_expand = pre_4;
            valid_iteration--;
            continue;
        }

        auto end_total_f = std::chrono::high_resolution_clock::now();
        auto period_f = std::chrono::duration_cast<std::chrono::microseconds>(end_total_f - start_total_f);
        duration_total = period_f + duration_total;

//        std::cout << "This code's augmenting path taken: " << (duration_augmenting_path-pre_3).count()/(1000.0) << " milliseconds" << std::endl;
//        std::cout << "This code's expand taken: " << (duration_expand-pre_4).count()/(1000.0) << " milliseconds" << std::endl;
//        std::cout << "This code's blossom taken: " << (duration_blossom-pre_2).count()/(1000.0) << " milliseconds" << std::endl;

        std::cout << "The computation time: " << (duration_blossom-pre_2+duration_expand-pre_4+duration_augmenting_path-pre_3).count()/(1000.0) << " milliseconds" << std::endl;
        std::cout << "*************************************"<< std::endl;

        if(i == iteration-1 || stop_immediately){
            break;
        }
        M = std::vector<int>(nodes, -1);
    }

    std::cout << std::endl;
    std::cout << "This is a PARALLEL method by using Path Table" <<std::endl;
    std::cout << "# of threads: " << num_of_threads << std::endl;
//    std::cout << "The average augmenting path taken: " << duration_augmenting_path.count()/(1000.0*valid_iteration) << " milliseconds" << std::endl;
//    std::cout << "The average expand taken: " << duration_expand.count()/(1000.0*valid_iteration) << " milliseconds" << std::endl;
//    std::cout << "The average blossom taken: " << duration_blossom.count()/(1000.0*valid_iteration) << " milliseconds" << std::endl;
    std::cout << "The average computation taken: " << (duration_blossom.count()+duration_expand.count()+duration_augmenting_path.count())/(1000.0*valid_iteration) << " milliseconds" << std::endl;
    testMatching(M);
}
