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
    gates.reserve(num_gates);
    bottom_layer=0;
}

CircuitGraph::CircuitGraph(){
    gates.reserve(1);
    bottom_layer=0;
}


void CircuitGraph::push_back_Gate(int id, GATES type, std::vector<int> parents, int out){
    Node node;
    node.id = id;
    node.type = type;
    node.parents = parents;
    node.depth = 0;
    node.collected = -1;
    node.out = out;
    gates.push_back(node);
}    

void CircuitGraph::addChild(int id, int cildId){
    gates[id].children.push_back(cildId);
}

void CircuitGraph::computeDepths(){
    for(int i = 0; i < gates.size(); i++){
        if(gates[i].type == GATES::INPUT || gates[i].type == GATES::OUTPUT){
            gates[i].depth = -1; // not part of circuit 
            gates[i].collected = -2; // don't collect ever
            continue;
        }
        int depth = 0;
        for(int j =  0; j < gates[i].parents.size(); j++){
            int parentdepth = gates[gates[i].parents[j]].depth;
            if ( parentdepth>= depth){
                depth = parentdepth + 1;
            }
        }
        gates[i].depth = depth;
        if (depth == 0){
            bottom_layer++;
        }
    }
}

bool CircuitGraph::isInOut(int id){
    if (gates[id].collected == -2){
        return true;
    }
    return false;
}

void CircuitGraph::defineSubgraphs(int num_Threads){
    int gap = floor(bottom_layer / num_Threads);
    cout << "gap: " << gap << endl;
    if (gap < 1){
        cerr << "more threads than bottom layer gates??" << endl;
        return;
    }

    // select from the bottomlayer by picking every gapth gate
    //std::vector<SubGraph> subgraphs;
    int skip = gap;
    int t = 0;

    for(int i = 0; i < gates.size(); i++){
        if(gates[i].depth == 0){
            if (skip < gap){
                skip++;
            } else {
                SubGraph sg = {t,vector<int>{i},vector<int>{},vector<int>{},vector<int>{}, false, 0,0};
                subgraphs.push_back(sg);
                gates[i].collected = t;
                t++;
                skip = 0;
            }
        }
    }

    cout << "t: " << t  << "subgraphs.size " << subgraphs.size() << endl;
    cout << "initial gates: ";
    for (int t = 0; t < subgraphs.size(); t++){
        cout << subgraphs[t].gates[0] << ", ";
    }
    cout << endl;


    // go around the starting points and collect connected gates, taking turns to make the subgraphs evenly sized

    std::vector<int> closed_subgraphs;
    bool all_done = false;
    while(!all_done){
        for(int t = 0; t < subgraphs.size(); t++){
            if (subgraphs[t].closed){continue;}
            int latest_node = subgraphs[t].gates[subgraphs[t].head];
            int latest_node_child_counter = subgraphs[t].head_child; 
            int latest_node_child_node = 0;
            bool success;
            if (gates[latest_node].children.size() != 0){ // else it is a output node (no children)
                latest_node_child_node = gates[latest_node].children[latest_node_child_counter];
                stack<int> parent_nodes;
                success = collect_parents(parent_nodes, latest_node_child_node, t); 
          //      cout << "success for node " << latest_node_child_node << "+" << success << endl;
                if(success){
                    // add all nodes to subgraph
                    while(parent_nodes.size() > 0){
                        int p = parent_nodes.top();
                        parent_nodes.pop();
                        subgraphs[t].gates.push_back(p);
                    }
                } else {
                    // purge all from the stack set explored to -1 only if it was this subgraph
                    while(parent_nodes.size() > 0){
                        int p = parent_nodes.top();
                        parent_nodes.pop();
                        if(gates[p].collected == t){ // only uncollect if it is not claimed by another subgraph
                            gates[p].collected = -1;
                        }
                    }
                }
            } else {
                success = false;
            }
            
            
            if (gates[latest_node].children.size() > 0 && latest_node_child_counter < (gates[latest_node].children.size()-1)){
                // advance child counter for current node
                subgraphs[t].head_child++;
            } else { // all children from this node are explored
                // advance head since this node is completed
                if (!success && subgraphs[t].head == (subgraphs[t].gates.size()-1)){
                    subgraphs[t].closed = true;
                   // cout << "subgraph" << t << " closed," << subgraphs[t].gates.size() << " gates" << endl;
                } else if (success && subgraphs[t].head == (subgraphs[t].gates.size()-1)){
                    cout << "ERROR: sucessful finding of children but also no furhter gates? " << endl;
                    return;
                } else { // can sucess be true even if no new gates were added?
                    subgraphs[t].head++;
                    subgraphs[t].head_child = 0;
                } 
            } 
        }
        // check if all subgraphs are closed
        all_done = true;
        for(int t = 0; t < subgraphs.size(); t ++ ){
            if(subgraphs[t].closed == false){
                all_done = false;
            }
        }
    }
}


// whenever a node is at depth 0 it gets pushed on the stack & whever a node is not collected yet it gets pushed on the stack
// child nodes always need to get pushed on the stack before their parents, the top thing on the stack needs to get computed first
bool CircuitGraph::collect_parents(stack<int> &parent_nodes, int current_node, int t){
  //  cout << "current node: " << current_node << endl; 
    if(isInOut(current_node)){
        return true; // skip inputs and outputs
    }
    if (gates[current_node].collected != -1){
        if (gates[current_node].collected == t){ // node is already part of the current subgraph, then all parent nodes need to be included already
            return true;
        }
        if (gates[current_node].collected != t) { // the parent node is part of a different subgraph, need to cut here
            return false;
        }
    }
    /*
    if(gates[current_node].depth == 0){
        parent_nodes.push(current_node);
        gates[current_node].collected = t;
        return true;
    }
    */

    if(gates[current_node].collected == -1){ // not included anywhere, go to collect all parents
        parent_nodes.push(current_node); // push child onto stack before its parent since it appears later in the circuit
        gates[current_node].collected = t;
        bool success = true;
        for (int p = 0; p < gates[current_node].parents.size(); p++){ // skips for nodes of depth == 0
            int parent = gates[current_node].parents[p];
            success = success && collect_parents(parent_nodes, parent, t);
        }
        return success;
    } 

    return false;
}


/*
after calling defineSubgraphs, this function collects all remaining nodes that are not part of a subgraph
*/
void CircuitGraph::collect_remaining(){
    if (subgraphs.size() == 0) {
        throw std::runtime_error("No subgraphs defined. Please call defineSubgraphs() first.");
    }
    // iterate over all gates and collect those that are not part of a subgraph
    int idsg = subgraphs.size();
    SubGraph sg = {idsg,vector<int>{},vector<int>{},vector<int>{},vector<int>{}, false, 0,0};
    for (int i = 0; i < gates.size(); i++) {
        if (gates[i].collected == -1) { // not part of a subgraph
            sg.gates.push_back(i);
            gates[i].collected = idsg; // mark as collected in the last subgraph
            // find out what the dependencies of the subgraph are
            for (int j = 0; j < gates[i].parents.size(); j++) {
                //cout << "parent: " << gates[i].parents[j] << endl;
                int parent = gates[i].parents[j];
                if (gates[parent].collected != -1 && gates[parent].collected != idsg) { // parent is part of a different subgraph
                    if (std::find(sg.dependencies.begin(), sg.dependencies.end(), gates[parent].collected) == sg.dependencies.end()) {
                        sg.dependencies.push_back(gates[parent].collected);
                        cout << "added dependency: " << gates[parent].collected << endl;
                    }
                }
            }            
        }
    }
    sg.closed = true;
    subgraphs.push_back(sg);
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


// write subgraphs to files and create filenames with the prefix and numbers
void CircuitGraph::write_subgraphs(std::string file_prefix)
{
    if(subgraphs.size() == 0) {
        throw std::runtime_error("No subgraphs defined. Please call defineSubgraphs() first.");
    }
    for (int i = 0; i < subgraphs.size(); i++) {
        json out;
        out["id"] = subgraphs[i].id;
        out["gates"] = json::array();
        out["inputs"] = subgraphs[i].inputs;
        out["outputs"] = subgraphs[i].outputs;
        out["dependencies"] = subgraphs[i].dependencies;

        for (auto& gate : subgraphs[i].gates) {
            json info = {{"type", to_string(gates[gate].type)}, {"parents", gates[gate].parents}, {"children", gates[gate].children}};
            out["gates"].push_back({{"gate" + to_string(gates[gate].id), info}});
        }


        std::ofstream file(file_prefix + std::to_string(i) + ".json");
        if (file.is_open()) {
            file << out.dump(4); // pretty print with 4 spaces
            file.close();
        } else {
            throw std::runtime_error("Could not open file for writing: " + file_prefix + std::to_string(i) + ".json");
        }
    }
}
