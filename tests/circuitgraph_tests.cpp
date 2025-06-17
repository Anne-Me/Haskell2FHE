#include <iostream>
#include "stdexcept"
#include "../CircuitGraph.h"
#include <vector>
#include <stack>
#include <string>

#include "circuitgraph_tests.h"

void createSimpleCircuit(){
    CircuitGraph graph1 = CircuitGraph(7);
    std::vector<int> no_parents;
    graph1.push_back_Gate(0,GATES::AND, no_parents,0);
    graph1.push_back_Gate(1,GATES::AND, no_parents,1);
    graph1.push_back_Gate(2,GATES::NAND, no_parents,2);
    graph1.push_back_Gate(3,GATES::OR, no_parents,3);
    graph1.push_back_Gate(4,GATES::AND, std::vector<int>{0,1},4);
    graph1.push_back_Gate(5,GATES::AND, std::vector<int>{2,3},5);
    graph1.push_back_Gate(6,GATES::AND, std::vector<int>{4,5},6);

    graph1.addChild(0,4);
    graph1.addChild(1,4);
    graph1.addChild(2,5);
    graph1.addChild(3,5);
    graph1.addChild(4,6);
    graph1.addChild(5,6);

    /*
    TEST computeDepths
    */
    graph1.computeDepths();

    std::cout << "depth of gate 0: " << graph1.gates[0].depth << std::endl;
    std::cout << "depth of gate 1: " << graph1.gates[1].depth << std::endl;
    std::cout << "depth of gate 2: " << graph1.gates[2].depth << std::endl;
    std::cout << "depth of gate 3: " << graph1.gates[3].depth << std::endl;
    std::cout << "depth of gate 4: " << graph1.gates[4].depth << std::endl;
    std::cout << "depth of gate 5: " << graph1.gates[5].depth << std::endl;
    std::cout << "depth of gate 6: " << graph1.gates[6].depth << std::endl;


    graph1.defineSubgraphs(2,0);

    std::cout << "number of subgraphs " << graph1.subgraphs.size() << std::endl;
    std::cout << "number of gates in first subgraph " << graph1.subgraphs[0].gates.size() << std::endl;
    std::cout << "number of gates in second subgraph " << graph1.subgraphs[1].gates.size() << std::endl;
}

void createSimpleCircuitPlus2(){  // same circuit as in previous task but make 3 subgraphs
    CircuitGraph graph1 = CircuitGraph(7);
    std::vector<int> no_parents;
    graph1.push_back_Gate(0,GATES::AND, no_parents,0);
    graph1.push_back_Gate(1,GATES::AND, no_parents,1);
    graph1.push_back_Gate(2,GATES::NAND, no_parents,2);
    graph1.push_back_Gate(3,GATES::OR, no_parents,3);
    graph1.push_back_Gate(4,GATES::AND, std::vector<int>{0,1},4);
    graph1.push_back_Gate(5,GATES::AND, std::vector<int>{2,3},5);
    graph1.push_back_Gate(6,GATES::AND, std::vector<int>{4,5},6);
    graph1.push_back_Gate(7,GATES::XOR, no_parents,7);
    graph1.push_back_Gate(8, GATES::XOR, std::vector<int>{0,7},8);

    graph1.addChild(0,4);
    graph1.addChild(0,8);
    graph1.addChild(1,4);
    graph1.addChild(2,5);
    graph1.addChild(3,5);
    graph1.addChild(4,6);
    graph1.addChild(5,6);
    graph1.addChild(7,8);

    /*
    TEST computeDepths
    */
    graph1.computeDepths();

    std::cout << "depth of gate 0: " << graph1.gates[0].depth << std::endl;
    std::cout << "depth of gate 4: " << graph1.gates[4].depth << std::endl;
    std::cout << "depth of gate 6: " << graph1.gates[6].depth << std::endl;
    std::cout << "depth of gate 8: " << graph1.gates[8].depth << std::endl;


    graph1.defineSubgraphs(2,0);

    std::cout << "number of subgraphs " << graph1.subgraphs.size() << std::endl;
    std::cout << "number of gates in first subgraph " << graph1.subgraphs[0].gates.size() << std::endl;
    std::cout << "number of gates in second subgraph " << graph1.subgraphs[1].gates.size() << std::endl;
}


void createSimpleCircuitPlus2split3(){  // same circuit as in previous task but make 3 subgraphs
    std::cout << "graphing " << std::endl;
    CircuitGraph graph1 = CircuitGraph(13);
    graph1.input_length = 3;
    std::vector<int> no_parents;
    graph1.set_gate(0, GATES::INPUT, no_parents,0);
    graph1.set_gate(1, GATES::INPUT, no_parents,1);
    graph1.set_gate(2, GATES::INPUT, no_parents,2);
    graph1.set_gate(3,GATES::AND,std::vector<int>{0,2},3);
    graph1.set_gate(4,GATES::AND, std::vector<int>{1,2},4);
    graph1.set_gate(5,GATES::NAND, std::vector<int>{0,1},5);
    graph1.set_gate(6,GATES::OR, std::vector<int>{0,1},6);
    graph1.set_gate(9,GATES::AND, std::vector<int>{3,4},9);
    graph1.set_gate(10,GATES::AND, std::vector<int>{5,6},10);
    graph1.set_gate(11,GATES::AND, std::vector<int>{7,8},11);
    graph1.set_gate(7,GATES::XOR, std::vector<int>{0,2},7);
    graph1.set_gate(8, GATES::XOR, std::vector<int>{3,7},8);
    graph1.set_gate(12, GATES::XOR, std::vector<int>{8,11},12);
    std::cout << "all gates set " << std::endl;

    graph1.addChild(0,3);
    graph1.addChild(0,5);
    graph1.addChild(0,6);
    graph1.addChild(0,7);
    graph1.addChild(1,4);
    graph1.addChild(1,6);
    graph1.addChild(1,5);
    graph1.addChild(2,4);
    graph1.addChild(2,7);
    graph1.addChild(2,3);

    graph1.addChild(7,8);
    graph1.addChild(3,8);
    graph1.addChild(4,9);
    graph1.addChild(3,9);
    graph1.addChild(5,10);
    graph1.addChild(6,10);
    graph1.addChild(9,11);
    graph1.addChild(10,11);
    graph1.addChild(11,12);
    graph1.addChild(8,12);

    /*
    TEST computeDepths
    */
    graph1.computeDepths();

    std::cout << "depth of gate 0: " << graph1.gates[0].depth << " (correct: -1)" << std::endl;
    std::cout << "depth of gate 1: " << graph1.gates[1].depth << " (correct: -1)" << std::endl;
    std::cout << "depth of gate 3: " << graph1.gates[3].depth << " (correct: 0)" << std::endl;
    std::cout << "depth of gate 4: " << graph1.gates[4].depth << " (correct: 0)" << std::endl;
    std::cout << "depth of gate 5: " << graph1.gates[5].depth << " (correct: 0)" << std::endl;
    std::cout << "depth of gate 6: " << graph1.gates[6].depth << " (correct: 0)" << std::endl;
    std::cout << "depth of gate 7: " << graph1.gates[7].depth << " (correct: 0)" << std::endl;
    std::cout << "depth of gate 8: " << graph1.gates[8].depth << " (correct: 1)" << std::endl;
    std::cout << "depth of gate 9: " << graph1.gates[9].depth << " (correct: 1)" << std::endl;
    std::cout << "depth of gate 10: " << graph1.gates[10].depth << " (correct: 1)" << std::endl;
    std::cout << "depth of gate 12: " << graph1.gates[12].depth << " (correct: 3)"<< std::endl;
    std::cout << "depth of gate 11: " << graph1.gates[11].depth << " (correct: 2)"<< std::endl;

    graph1.defineSubgraphs_test(3,0);

    std::cout << "number of subgraphs " << graph1.subgraphs.size() << std::endl;
    std::cout << "number of gates in first subgraph " << graph1.subgraphs[0].gates.size() << std::endl;
    std::cout << "number of gates in second subgraph " << graph1.subgraphs[1].gates.size() << std::endl;
    std::cout << "number of gates in third subgraph " << graph1.subgraphs[2].gates.size() << std::endl;

    graph1.collect_remaining();
    std::cout << " remaining " << graph1.subgraphs[3].gates.size() << std::endl;

    //graph1.write_subgraphs("test_circuit");
}




