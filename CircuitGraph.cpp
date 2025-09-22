#include <iostream>
#include "nlohmann/json.hpp"
#include "stdexcept"
#include "CircuitGraph.h"
#include <cmath>
#include "nlohmann/json.hpp"
#include <fstream>


using json = nlohmann::json;
using namespace std;

// while reading the json parse the circuit in here
// list of gates and each gate should have its incoming and outcoming nodes for easy access
// data in json file does not have that only the number of the edge -> need intermediate data structure that collects all children and parent on the way?
// gates need to be numbered
// each gate needs parents (a most two, since at most two incoming edges) and children (arbitrary) and number indicating depth
// assuming that by not disturbing the order of gates in the json file, we do not disturb the evaluation order

CircuitGraph::CircuitGraph(int num_gates){
    gates.resize(num_gates);
    bottom_layer=0;
    max_depth = 0;
}

CircuitGraph::CircuitGraph(){
    gates.reserve(1);
    bottom_layer=0;
    max_depth = 0;
}

void CircuitGraph::resize(int num){
    gates.resize(num);
}


void CircuitGraph::push_back_Gate(int id, GATES type, std::vector<int> parents, int out){
    Node node;
    node.id = id;
    node.type = type;
    node.parents = parents;
    node.depth = -1;
    node.collected = -1;
    node.out = out;
    gates.push_back(node);
}    

void CircuitGraph::set_gate(int id, GATES type, std::vector<int> parents, int out){
    if (id >= gates.size()) {
        throw std::out_of_range("Gate ID is out of range.");
    }
    Node node;
    node.id = id;
    node.type = type;
    node.parents = parents;
    node.depth = -1;
    node.collected = -1;
    if(type == GATES::INPUT){
        node.collected = -2; // input gates are never collected
    }
    node.out = out;

    if (gates[id].children.size()>0){ // children could have been added before the gate itself is added
        node.children = gates[id].children;
    }
    gates[id] = node;
}

void CircuitGraph::addChild(int id, int cildId){
    gates[id].children.push_back(cildId);
}


void CircuitGraph::topologicalSorting(){
    std::vector<int> sorting;
    std::unordered_map<int, bool> visited;
    std::stack<int> sorted;
    for(int i = input_length; i < gates.size(); i++){ // skip inputs, input depth is already set to -1
        if(visited[i] == true) continue;
        recurseTopologicalSorting(visited, sorted,i);
    }
    
    while(sorted.empty() == false){
        int next_gate = sorted.top();
        sorted.pop();
        executable.push_back(next_gate);
        int d = 0;
        for (int j = 0; j < gates[next_gate].parents.size(); j++){
            int p = gates[next_gate].parents[j];
            if(gates[p].depth >= d){
                d = gates[p].depth+1;
            }
        }
        gates[next_gate].depth = d;
        if(max_depth < d) max_depth = d;
    }
}

void CircuitGraph::recurseTopologicalSorting(std::unordered_map<int,bool> &visited, std::stack<int> &sorted, int id){
    if(visited[id] == true) return;
    for(int i = 0; i < gates[id].children.size(); i++){
        recurseTopologicalSorting(visited, sorted, gates[id].children[i]);
    }
    visited[id] = true;
    sorted.push(id);
}


bool CircuitGraph::identify_output(int id){
    if (gates[id].children.size() == 0){ // not good, output gate could still be used for other gate
        return true;
    }
    return false;
}


void CircuitGraph::split_level(int num_Threads, int d){
    subgraphs.clear();
    for (int i = 0; i < num_Threads; i++){
        subgraphs.push_back(SubGraph{i, vector<int>{}, vector<int>{}, vector<int>{}, vector<int>{}, false, 0, 0});
    }
    int num_gates = 0;
    for(int i = 0; i < executable.size();i++){
        if(gates[executable[i]].depth == d){
            num_gates++;
        }
    }
    if(num_gates == 0){
        cerr << "no gates at depth " << d << endl;
        return;
    }
    float gates_per_thread = (float)num_gates / num_Threads;

    int k = 0;
    for(int i = 0; i < gates.size(); i++){
        if(gates[i].depth == d){
            subgraphs[k].gates.push_back(i);
            k = (k+1) % num_Threads; 
        }
    }
}


// write the circuit to a file in json format
void CircuitGraph::write(string filename) {
    json out;
    out["data"]["inputlength"] = input_length;
    out["data"]["resultlength"] = output_length;
    out["gates"] = json::array();

    for (auto& gate : gates) {
        json info = {{"type", gate.type}, {"parents", gate.parents}, {"children", gate.children}};
        out["gates"].push_back({{"gate" + to_string(gate.id), info}});
    }

    std::ofstream file(filename);

    if (file.is_open()) {
        file << out.dump(4); // pretty print with 4 spaces
        file.close();
    } else {
        throw std::runtime_error("Could not open file for writing: " + filename);
    }
}



void CircuitGraph::gate_statistics(){
    std::vector<int> gates_stats(11,0);

    for (int gateid : executable) {
        Node gate = gates[gateid];
        switch (gate.type) {
        case GATES::AND:
            gates_stats[0] = gates_stats[0]+1;
            break;
        case GATES::OR:
            gates_stats[1] = gates_stats[1]+1;
            break;
        case GATES::NAND:
            gates_stats[2] = gates_stats[2]+1;
            break;
        case GATES::XOR:
            gates_stats[3] = gates_stats[3]+1;
            break;
        case GATES::NOR:
            gates_stats[4] = gates_stats[4]+1;
            break;
        case GATES::XNOR:
            gates_stats[5] = gates_stats[5]+1;
            break;
        case GATES::ANDYN:
            gates_stats[6] = gates_stats[6]+1;
            break;
        case GATES::NOT:
            gates_stats[7] = gates_stats[7]+1;
            break;
        case GATES::ORYN:
            gates_stats[8] = gates_stats[8]+1;
            break;
        case GATES::MUX:
            gates_stats[9] = gates_stats[9]+1;
            break;
        case GATES::INPUT:
            gates_stats[10] = gates_stats[10]+1;
            break;
        default:
            std::cout << "unknown gate" << to_string(gate.type) << std::endl;
            break;
        }
    }

    for(int i = 0; i < gates_stats.size(); i++){
        if(i == 0){
            cout << "AND: ";
        } else if (i == 1){
            cout << "OR: ";
        } else if (i == 2){
            cout << "NAND: ";
        } else if (i == 3){
            cout << "XOR: ";
        } else if (i == 4){
            cout << "NOR: ";
        } else if (i == 5){
            cout << "XNOR: ";
        } else if (i == 6){
            cout << "ANDYN: ";
        } else if (i == 7){
            cout << "NOT: ";
        } else if (i == 8){
            cout << "ORYN: ";
        } else if (i == 9){
            cout << "MUX: ";
        } else if (i == 10){
            cout << "INPUT: ";
        }
        std::cout << gates_stats[i] << std::endl;
    }
}

void CircuitGraph::depth_statistics(string file){
    // count how many gates per depth there are
    std::vector<int> depth_count(max_depth + 1, 0);
    int ins = 0;
    for (const auto& gate : gates) {
        if (gate.depth >= 0 && gate.depth < depth_count.size()) {
            depth_count[gate.depth]++;
        } else if (gate.depth < 0) {
            ins++;
        }
    }
    // write to file
    std::ofstream f(file);
    if (!f.is_open()) {
        throw std::runtime_error("Could not open file for writing: " + file);
    }
    f << "Depth,Count\n";
    f << "Input:" << ins << "\n"; // write number of inputs
    for (int i = 0; i < depth_count.size(); i++) {
        f << "Depth " << i << ": " << depth_count[i] << "\n";
    }
    f.close();
}
