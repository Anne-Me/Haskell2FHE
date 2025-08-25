#include <gtest/gtest.h>
#include "CircuitGraph.h"
#include <vector>
#include <gmock/gmock.h>

namespace{
    CircuitGraph makeSimpleGraph(){
        CircuitGraph graph1(7);
        graph1.input_length = 0;
        graph1.output_length = 0;

        std::vector<int> no_parents;
        graph1.set_gate(0, GATES::AND,  no_parents, 0);
        graph1.set_gate(1, GATES::AND,  no_parents, 1);
        graph1.set_gate(2, GATES::NAND, no_parents, 2);
        graph1.set_gate(3, GATES::OR,   no_parents, 3);
        graph1.set_gate(4, GATES::AND,  std::vector<int>{0,1}, 4);
        graph1.set_gate(5, GATES::AND,  std::vector<int>{2,3}, 5);
        graph1.set_gate(6, GATES::AND,  std::vector<int>{4,5}, 6);

        // add child edges 
        graph1.addChild(0,4);
        graph1.addChild(1,4);
        graph1.addChild(2,5);
        graph1.addChild(3,5);
        graph1.addChild(4,6);
        graph1.addChild(5,6);

        return graph1;
    }
}

TEST(CircuitGraphTest, Simple_depths) {
    CircuitGraph graph1 = makeSimpleGraph();
    // computeDepths 
    graph1.topologicalSorting();

    EXPECT_EQ(graph1.gates[0].depth, 0);
    EXPECT_EQ(graph1.gates[1].depth, 0);
    EXPECT_EQ(graph1.gates[2].depth, 0);
    EXPECT_EQ(graph1.gates[3].depth, 0);

    EXPECT_EQ(graph1.gates[4].depth, 1);
    EXPECT_EQ(graph1.gates[5].depth, 1);

    EXPECT_EQ(graph1.gates[6].depth, 2);
}

TEST(CircuitGraphTest, split_level) {
    CircuitGraph graph1 = makeSimpleGraph();
    // computeDepths 
    graph1.topologicalSorting();
    // split gates at a certain depth
    graph1.split_level(4,0);

    EXPECT_EQ(graph1.subgraphs.size(),4);
        
    EXPECT_EQ(graph1.subgraphs[0].gates[0],0);
    EXPECT_EQ(graph1.subgraphs[1].gates[0],1);
    EXPECT_EQ(graph1.subgraphs[2].gates[0],2);
    EXPECT_EQ(graph1.subgraphs[3].gates[0],3);

    // split next level
    graph1.split_level(4,1);        
    EXPECT_EQ(graph1.subgraphs[0].gates[0],4);
    EXPECT_EQ(graph1.subgraphs[1].gates[0],5);

    // split depth 2
    graph1.split_level(4,2);        
    EXPECT_EQ(graph1.subgraphs[0].gates[0],6);

    // again with 2 threads

    graph1.split_level(2,0);

    EXPECT_EQ(graph1.subgraphs.size(),2);
        
    EXPECT_EQ(graph1.subgraphs[0].gates.size(),2);
    EXPECT_EQ(graph1.subgraphs[1].gates.size(),2);

    EXPECT_THAT(graph1.subgraphs[0].gates, ::testing::UnorderedElementsAre(0,2));
    EXPECT_THAT(graph1.subgraphs[1].gates, ::testing::UnorderedElementsAre(1,3));
}