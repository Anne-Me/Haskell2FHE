#include <gtest/gtest.h>
#include "CircuitGraph.h"
#include <vector>

TEST(CircuitGraphTest, Simple_depths) {
    CircuitGraph graph1(7);
        std::cout << "created graph" << std::endl;


    std::vector<int> no_parents;
    graph1.set_gate(0, GATES::AND,  no_parents, 0);
    graph1.set_gate(1, GATES::AND,  no_parents, 1);
    graph1.set_gate(2, GATES::NAND, no_parents, 2);
    graph1.set_gate(3, GATES::OR,   no_parents, 3);
    graph1.set_gate(4, GATES::AND,  std::vector<int>{0,1}, 4);
    graph1.set_gate(5, GATES::AND,  std::vector<int>{2,3}, 5);
    graph1.set_gate(6, GATES::AND,  std::vector<int>{4,5}, 6);
    std::cout << "set gates" << std::endl;

    // add child edges 
    graph1.addChild(0,4);
    graph1.addChild(1,4);
    graph1.addChild(2,5);
    graph1.addChild(3,5);
    graph1.addChild(4,6);
    graph1.addChild(5,6);

        std::cout << "add children" << std::endl;


    // ----- computeDepths -----
    graph1.computeDepths();

        std::cout << "computed depths" << std::endl;


    EXPECT_EQ(graph1.gates[0].depth, 0);
    EXPECT_EQ(graph1.gates[1].depth, 0);
    EXPECT_EQ(graph1.gates[2].depth, 0);
    EXPECT_EQ(graph1.gates[3].depth, 0);

    EXPECT_EQ(graph1.gates[4].depth, 1);
    EXPECT_EQ(graph1.gates[5].depth, 1);

    EXPECT_EQ(graph1.gates[6].depth, 2);

}