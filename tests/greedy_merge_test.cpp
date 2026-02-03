#include <gtest/gtest.h>
#include <mags/greedy_merge.h>

// Include the header where get_priority_queue, CandidateSet, PriorityQueue, minPair are declared.

namespace mags::gm::test {
    using namespace detail;

    TEST(GreedyMergeTest, PQ_DescendingOrder) {
        CandidateSet candidate_set;
        candidate_set.resize(3);

        candidate_set[0][1] = 0.4;
        candidate_set[0][2] = 0.9;
        candidate_set[1][0] = 0.4;
        candidate_set[1][2] = 0.5;
        candidate_set[2][0] = 0.9;
        candidate_set[2][1] = 0.5;

        PriorityQueue pq = get_priority_queue(candidate_set);

        ASSERT_EQ(pq.size(), 3u);

        auto it = pq.begin(); // largest element first because comparator is std::greater<

        auto check_entry = [&](double saving, int a, int b) {
            ASSERT_NE(it, pq.end());
            EXPECT_DOUBLE_EQ(it->first, saving);
            EXPECT_EQ(it->second.second, b);
            EXPECT_EQ(it->second.first,  a);
            ++it;
        };

        check_entry(0.9, 0, 2);
        check_entry(0.5, 1, 2);
        check_entry(0.4, 0, 1);

        EXPECT_EQ(it, pq.end());
    }

    TEST(GreedyMergeTest, PQ_SymmetricCandidates_NoDuplicates) {
        const Graph g = { {}, {} };

        CandidateSet candidate_set;
        candidate_set.resize(2);

        // symmetric suggestions with the same score
        candidate_set[0][1] = 0.7;
        candidate_set[1][0] = 0.7;

        PriorityQueue pq = get_priority_queue(candidate_set);

        // Only one undirected pair should be present
        ASSERT_EQ(pq.size(), 1u);

        auto it = pq.begin();
        ASSERT_NE(it, pq.end());
        EXPECT_DOUBLE_EQ(it->first, 0.7);
        EXPECT_EQ(it->second.first,  0);
        EXPECT_EQ(it->second.second, 1);
    }

    TEST(GreedyMergeTest, PQ_TieBreaksByNodePair) {
        const Graph g = { {}, {}, {} };

        CandidateSet candidate_set;
        candidate_set.resize(3);

        candidate_set[1][0] = 0.6; 
        candidate_set[2][0] = 0.6; 
        candidate_set[2][1] = 0.6; 

        PriorityQueue pq = get_priority_queue(candidate_set);
        ASSERT_EQ(pq.size(), 3u);

        auto it = pq.begin();

        auto check_entry = [&](double saving, int a, int b) {
            ASSERT_NE(it, pq.end());
            EXPECT_DOUBLE_EQ(it->first, saving);
            EXPECT_EQ(it->second.second, b);
            EXPECT_EQ(it->second.first,  a);
            ++it;
        };

        check_entry(0.6, 1, 2);
        check_entry(0.6, 0, 2);
        check_entry(0.6, 0, 1);

        EXPECT_EQ(it, pq.end());
    }
    
    TEST(GreedyMergeTest, Replace_RemovesInvalidPairsAndDoesNotCreatePlaceholders) {
        /*
            The test inludes a pair in the PQ and CS to ensure that the check
            !candidate_set[u_super].contains(candidate_node)
            evaluates to false, and no placeholder is created in the PQ and CS because an entry already exists
        */


        // Graph: 0 -- 1 -- 2
        const Graph g = {
            {1},      
            {0, 2},   
            {1}       
        };
        SuperNodeSet sns(g);

        // Merge 0 and 1 so that super(1) is u_super = 0
        sns.merge(0, 1);

        // v = 1 has two candidates: {0:0.8, 2:0.4}
        CandidateSet cs(3);
        cs[1][0] = 0.8;  // candidate_node == u_super 
        cs[1][2] = 0.4;  // candidate_node != u_super

        // Symmetric entries
        cs[0][1] = 0.8;
        cs[2][1] = 0.4; 

        // Pair to ensure false on the contains check 
        // This entry should remain the CS instead of a placeholder
        cs[2][0] = 0.3;
        cs[0][2] = 0.3;

        PriorityQueue pq;
        // Entries that must be removed by replace
        pq.emplace(0.8, minPair(1, 0));  // (0.8, (0,1))
        pq.emplace(0.4, minPair(1, 2));  // (0.4, (1,2))

        // Entry that should remain in PQ instead of a placeholder
        pq.emplace(0.3, minPair(2, 0));  // (0.3, (0,2))

        // Sanity preconditions
        ASSERT_EQ(pq.size(), 3u);
        ASSERT_EQ(pq.count({0.8, minPair(1,0)}), 1u);
        ASSERT_EQ(pq.count({0.4, minPair(1,2)}), 1u);
        ASSERT_EQ(pq.count({0.3, minPair(2,0)}), 1u);

        // Act
        replace(/*v=*/1, sns, cs, pq);

        // Check that u_super (0) is the super node for v
        EXPECT_EQ(0, sns.get_super_node(1));
        
        // Check that candidate nodes does not have the invalid node v
        EXPECT_FALSE(cs[0].contains(1));  
        EXPECT_FALSE(cs[2].contains(1));  
        
        // Check that v is cleared from pq
        EXPECT_TRUE(cs[1].empty());      
        
        // v's PQ entries removed
        EXPECT_EQ(pq.count({0.8, minPair(1,0)}), 0u);  
        EXPECT_EQ(pq.count({0.4, minPair(1,2)}), 0u);  
        
        // For candidate_node != u_super: create placeholder in PQ and CS to initiate update
        EXPECT_TRUE(cs[0].contains(2));
        EXPECT_TRUE(cs[2].contains(0));
        EXPECT_DOUBLE_EQ(cs[0][2], 0.3);               
        EXPECT_DOUBLE_EQ(cs[2][0], 0.3);              
        // No placeholder since the entry exists
        EXPECT_EQ(pq.count({-1, minPair(0,2)}), 0u); 
        // The entry exists
        EXPECT_EQ(pq.count({0.3, minPair(0,2)}), 1u); 
    }
    
    TEST(GreedyMergeTest, Replace_InsertsPlaceholdersWhenMissing) {
        // identical to the test above, but here we want to create a placeholder
        // thus not keeping a pair to make the contains check evaluate to true

        // Graph: 0 -- 1 -- 2
        const Graph g = {
            {1},     
            {0, 2},   
            {1}       
        };
        SuperNodeSet sns(g);
        sns.merge(0, 1); 
        
        CandidateSet cs(3);
        // v has two candidates again
        cs[1][0] = 0.8;  // candidate_node == u_super
        cs[1][2] = 0.4;  // candidate_node != u_super
        
        // Symmetric entries
        cs[0][1] = 0.8;
        cs[2][1] = 0.4;
        
        PriorityQueue pq;
        pq.emplace(0.8, minPair(1, 0));
        pq.emplace(0.4, minPair(1, 2));
        
        ASSERT_EQ(pq.size(), 2u);
        
        replace(/*v=*/1, sns, cs, pq);
        
        // Check that u_super (0) is the super node for v
        EXPECT_EQ(0, sns.get_super_node(1));

        // Check that candidate nodes does not have the invalid node v
        EXPECT_FALSE(cs[0].contains(1));  
        EXPECT_FALSE(cs[2].contains(1));  
        
        // Check that v is cleared from pq
        EXPECT_TRUE(cs[1].empty());      
        
        // v's PQ entries removed
        EXPECT_EQ(pq.count({0.8, minPair(1,0)}), 0u);  
        EXPECT_EQ(pq.count({0.4, minPair(1,2)}), 0u);  
        
        // For candidate_node != u_super: create placeholder in PQ and CS to initiate update
        EXPECT_TRUE(cs[0].contains(2));
        EXPECT_TRUE(cs[2].contains(0));
        EXPECT_DOUBLE_EQ(cs[0][2], -1.0);               
        EXPECT_DOUBLE_EQ(cs[2][0], -1.0);              
    }

    TEST(GreedyMergeTest, Evaluate_NewSavingStoredAndOldPushed) {
        // Graph: 0 -- 2 -- 1
        const Graph g = {
            {2},      
            {2},      
            {0, 1}    
        };
        SuperNodeSet sns(g);

        CandidateSet candidate_set(3);
        // Old saving (stale) value; evaluate should replace this with the new saving (0.5)
        candidate_set[0][1] = 0.1;

        std::vector<NodeID> to_remove;
        std::vector<std::pair<int, double>> to_update;

        // Act: default threshold = -0.03; new saving(0,1)=0.5 >= threshold -> update
        evaluate(/*u=*/0, /*v=*/1, sns, candidate_set, to_remove, to_update /*, threshold = -0.03*/);

        // Assert: v moved to update with its *old* saving, and entry is updated to new saving
        ASSERT_TRUE(to_remove.empty());
        ASSERT_EQ(to_update.size(), 1u);
        EXPECT_EQ(to_update[0].first, 1);
        EXPECT_DOUBLE_EQ(to_update[0].second, 0.1);

        // New saving stored in candidate_set[0][1] (expected 0.5 for this shape)
        EXPECT_DOUBLE_EQ(candidate_set[0][1], 0.5);
    }

    TEST(GreedyMergeTest, Evaluate_WhenNewSavingBelowOrEqualThreshold) {
        // Graph: 0 -- 2 -- 1
        const Graph g = {
            {2},      
            {2},      
            {0, 1}    
        };
        SuperNodeSet sns(g);

        CandidateSet candidate_set(3);
        candidate_set[0][1] = 0.1; // old saving retained if removal triggered

        std::vector<NodeID> to_remove;
        std::vector<std::pair<int, double>> to_update;

        // Choose threshold higher than new saving (0.5) to force removal
        const double threshold = 0.6;
        evaluate(/*u=*/0, /*v=*/1, sns, candidate_set, to_remove, to_update, threshold);

        // Assert: v is marked for removal, no update entry, candidate_set unchanged
        ASSERT_EQ(to_remove.size(), 1u);
        EXPECT_EQ(to_remove[0], 1);

        EXPECT_TRUE(to_update.empty());
        EXPECT_DOUBLE_EQ(candidate_set[0][1], 0.1); // unchanged
    }

    TEST(GreedyMergeTest, Evaluate_WhenSavingUnchanged) {
        // Graph: 0 -- 2 -- 1
        const Graph g = {
            {2},      // 0
            {2},      // 1
            {0, 1}    // 2
        };
        SuperNodeSet sns(g);

        CandidateSet candidate_set(3);

        // Sets saving equal to new saving to trigger early return
        candidate_set[0][1] = 0.5;

        // to verify not modified
        std::vector<NodeID> to_remove = {42}; 
        std::vector<std::pair<int, double>> to_update = {{99, 3.14}};

        evaluate(/*u=*/0, /*v=*/1, sns, candidate_set, to_remove, to_update /*, default threshold*/);

        // Assert: no changes
        ASSERT_EQ(to_remove.size(), 1u);
        EXPECT_EQ(to_remove[0], 42);

        ASSERT_EQ(to_update.size(), 1u);
        EXPECT_EQ(to_update[0].first, 99);
        EXPECT_DOUBLE_EQ(to_update[0].second, 3.14);

        EXPECT_DOUBLE_EQ(candidate_set[0][1], 0.5);
    }

    TEST(GreedyMergeTest, RemovesCandidateVFromPQAndCandidateSet) {
        // Build candidate_set with symmetric entry (u=1, v=0) and a different pair to ensure only the target is removed.
        CandidateSet candidate_set;
        candidate_set.resize(3);
        candidate_set[1][0] = 0.7;           // target to remove
        candidate_set[0][1] = 0.7;           // symmetric partner (will be erased too)
        candidate_set[2][1] = 0.5;           // should remain
        candidate_set[1][2] = 0.5;           // symmetric for (2,1), should remain

        // Build PQ
        PriorityQueue pq;
        pq.emplace(0.7, minPair(1, 0));      // target entry (0.7, (0,1))
        pq.emplace(0.5, minPair(2, 1));      // other entry (0.5, (1,2)) that must remain

        // Sanity: both present initially
        ASSERT_EQ(pq.size(), 2u);
        ASSERT_TRUE(candidate_set[1].contains(0));
        ASSERT_TRUE(candidate_set[0].contains(1));
        ASSERT_TRUE(candidate_set[2].contains(1));
        ASSERT_TRUE(candidate_set[1].contains(2));

        remove_candidate_v(/*u=*/1, /*v=*/0, candidate_set, pq);

        EXPECT_EQ(pq.size(), 1u); // the pq should now contain one element
        EXPECT_EQ(pq.count({0.7, minPair(1,0)}), 0u); // the element should not exist in pq
        EXPECT_EQ(pq.count({0.5, minPair(2,1)}), 1u); // the lelement should exist in pq

        // Assert: candidate_set[1][0] and candidate_set[0][1] removed (symmetric erase)
        EXPECT_FALSE(candidate_set[1].contains(0));
        EXPECT_FALSE(candidate_set[0].contains(1));

        // Assert: unrelated entries remain intact
        EXPECT_TRUE(candidate_set[2].contains(1));
        EXPECT_TRUE(candidate_set[1].contains(2));
    }

    TEST(GreedyMergeTest, UpdateCandidateV_ReplacesOldEntryAndSyncsSymmetry) {
        CandidateSet candidate_set(3);
        PriorityQueue pq;
        const NodeID u = 1;
        const NodeID v = 0;

        // New saving
        candidate_set[u][v] = 0.85;
        // Old saving
        candidate_set[v][u] = -1;

        pq.emplace(-1, minPair(u, v));

        // Also add an unrelated entry to make sure it's not touched
        candidate_set[2][1] = 0.50;
        candidate_set[1][2] = 0.50;
        pq.emplace(0.50, minPair(2, 1));

        // Sanity preconditions
        ASSERT_EQ(pq.size(), 2u);
        ASSERT_EQ(pq.count({-1, minPair(u, v)}), 1u);
        ASSERT_EQ(pq.count({0.50, minPair(2, 1)}), 1u);

        // Act: tell the function the 'old' saving_score it must remove from PQ
        update_candidate_v(u, v, /*saving_score=*/-1, candidate_set, pq);

        // Assert: symmetry updated 
        EXPECT_DOUBLE_EQ(candidate_set[u][v], 0.85);
        EXPECT_DOUBLE_EQ(candidate_set[v][u], 0.85);

        // Assert: old PQ entry removed, new entry inserted
        EXPECT_EQ(pq.count({-1, minPair(u, v)}), 0u);
        EXPECT_EQ(pq.count({0.85, minPair(u, v)}), 1u);

        // Assert: unrelated entries are untouched
        EXPECT_EQ(pq.count({0.50, minPair(2, 1)}), 1u);
    }
}