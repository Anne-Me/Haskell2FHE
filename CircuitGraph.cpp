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

void CircuitGraph::computeDepths(){
    int depth = 0;
    int max_depth = 0;
    for(int i = 0; i < input_length; i++){
        for(int c = 0; c < gates[i].children.size(); c++){
            max_depth = propagateDepth(gates[i].children[c], depth);
        }
    }
    for (int i = 0; i < gates.size(); i++){
        if(gates[i].depth == 0){
            bottom_layer++;
        }
    }
   // << "Bottom layer: " << bottom_layer << endl;
}

void CircuitGraph::recomputeDepths(){
    max_depth = 0;
    int remaining = subgraphs[subgraphs.size()-1].gates.size();
    for (int i = 0; i < remaining; i++){
          int id = subgraphs[subgraphs.size()-1].gates[i];
          gates[id].depth = 0; // reset depth
          gates[id].collected = -1; // reset collected
     }
     int max_depth = 0;
     for (int i = 0; i < remaining; i++){
          int id = subgraphs[subgraphs.size()-1].gates[i]; // only go over gates in the last subgraph
          for (int c = 0; c < gates[id].children.size(); c++){
                max_depth = propagateDepth(gates[id].children[c], 1);
          }
     }
     bottom_layer = 0;
     for (int i = 0; i < remaining; i++){
        int id = subgraphs[subgraphs.size()-1].gates[i];
        if(gates[id].depth == 0){
            bottom_layer++;
        }
    }
}

void CircuitGraph::reset_depths_from_layer(int d){
    int rem_max_depth = 0;
    for(int i = 0; i < gates.size(); i++){
        if (gates[i].depth > d){ // exclude the layer itself
            gates[i].depth = gates[i].depth - (d+1); // reset depth
            gates[i].collected = -1; // reset collected
            if( gates[i].depth > rem_max_depth){
                rem_max_depth = gates[i].depth;
            }
        }
    }
    bottom_layer = 0;
    int remaining = subgraphs[subgraphs.size()-1].gates.size();
     for (int i = 0; i < remaining; i++){
        int id = subgraphs[subgraphs.size()-1].gates[i];
        if(gates[id].depth == 0){
            bottom_layer++;
        }
    }
}

bool CircuitGraph::identify_output(int id){
    if (gates[id].children.size() == 0){ // not good, output gate could still be used for other gate
        return true;
    }
    return false;
}

int CircuitGraph::propagateDepth(int id, int depth){
    if (max_depth < depth){
        max_depth = depth;
    }
    if (gates[id].depth < depth){
        gates[id].depth = depth;
        for (int i = 0; i < gates[id].children.size(); i++){
            max_depth = propagateDepth(gates[id].children[i], depth+1);
        }
    }
    return max_depth;
}

bool CircuitGraph::isInput(int id){
    if (gates[id].type == GATES::INPUT){ // used to be collected == -2
        return true;
    }
    return false;
}

void CircuitGraph::defineSubgraphs(int num_Threads, int previous){
    init_subgraphs(num_Threads,previous);

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
                success = collect_parents(parent_nodes, latest_node_child_node, t,0); 
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

void CircuitGraph::init_subgraphs(int num_Threads, int previous){
    float gap = (float)bottom_layer / num_Threads;

    if (gap < 1){
        cout << "more threads than bottom layer gates" << endl;
        while(gap < 1){
            num_Threads--;
            gap = (float)bottom_layer / num_Threads;
        }
    }

    // select from the bottomlayer by picking every gapth gate
    //std::vector<SubGraph> subgraphs;
    int t = 0;
    float nextfloat = (t+1)*gap;
    int next = floor((t+1)*gap); // alternative splitting: skip = gap
    int skip = 1;
    if(previous == 0){
        for(int i = 0; i < gates.size(); i++){
            if(gates[i].depth == 0){

            //cout << "skip: " << skip << "next: " << next << endl;
                if (skip < next){
                    skip++;
                } else {
                    SubGraph sg = {t,vector<int>{i},vector<int>{},vector<int>{},vector<int>{}, false, 0,0};
                    subgraphs.push_back(sg);
                    gates[i].collected = t;
                    t++;
                    skip++;
                    float nextfloat = (t+1)*gap;

                    next = floor((t+1)*gap);
                    //cout << "nextfloat " << nextfloat << "next " << next << endl;
                    //skip = 0;
                }
            }
        }
    } else {
        SubGraph last =  subgraphs[previous];
        subgraphs.pop_back();
        for (int i = 0; i < last.gates.size(); i++)
        {
           if(gates[last.gates[i]].depth == 0){
                if (skip < next){
                    skip++;
                } else {
                    SubGraph sg = {t+previous,vector<int>{last.gates[i]},vector<int>{},vector<int>{},vector<int>{}, false, 0,0};
                    subgraphs.push_back(sg);
                    gates[last.gates[i]].collected = t+previous;
                    t++;
                    skip++;
                    float nextfloat = (t+1)*gap;

                    next = floor((t+1)*gap);
                }
            }
        }
    }
}

void CircuitGraph::defineSubgraphs_test(int num_Threads, int previous){
    SubGraph previous_remain;
    if(previous > 0){
        previous_remain = subgraphs[previous];
    }
    init_subgraphs(num_Threads,previous);
    
    // go around the starting points and collect connected gates, taking turns to make the subgraphs evenly sized
    std::vector<int> closed_subgraphs;
    bool all_done = false;
    while(!all_done){
        for(int t = previous; t < subgraphs.size(); t++){
            if (subgraphs[t].closed){continue;}
            bool added_nodes = false;
            
            while(!added_nodes){ // keep trying to expand subgraph t until something was found
                int latest_node = subgraphs[t].gates[subgraphs[t].head];
                int latest_node_child_counter = subgraphs[t].head_child; 
                int latest_node_child_node = 0;
                bool success;
                if (gates[latest_node].children.size() != 0){ // else it is a output node (no children)
                    latest_node_child_node = gates[latest_node].children[latest_node_child_counter];
                    stack<int> parent_nodes;
                    success = collect_parents(parent_nodes, latest_node_child_node, t,0); 
            //      cout << "success for node " << latest_node_child_node << "+" << success << endl;
                    if(success){
                        // add all nodes to subgraph
                        added_nodes = true;
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
                        added_nodes = true; // terminate loop
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
bool CircuitGraph::collect_parents(stack<int> &parent_nodes, int current_node, int t, int p){
  //  cout << "current node: " << current_node << endl; 
    if(isInput(current_node) ){
        return true; 
    }
    if (gates[current_node].collected != -1){
        if (gates[current_node].collected == t){ // node is already part of the current subgraph, then all parent nodes need to be included already
            return true;
        }
        if (gates[current_node].collected != t){ // the parent node is part of a different subgraph, need to cut here
            if(gates[current_node].collected < p){
                return true; // parent is from previous subgraph spliting
            }
            return false;
        }
    }

    if(gates[current_node].collected == -1){ // not included anywhere, go to collect all parents
        parent_nodes.push(current_node); // push child onto stack before its parent since it appears later in the circuit
        gates[current_node].collected = t;
        bool success = true;
        for (int p = 0; p < gates[current_node].parents.size(); p++){ // skips for nodes of depth == 0
            int parent = gates[current_node].parents[p];
            success = success && collect_parents(parent_nodes, parent, t,0);
        }
        return success;
    } 

    return false;
}


/*
after calling defineSubgraphs, this function collects all remaining nodes that are not part of a subgraph
*/
void CircuitGraph::collect_remaining(){
    // iterate over all gates and collect those that are not part of a subgraph
    int idsg = subgraphs.size();
    SubGraph sg = {idsg,vector<int>{},vector<int>{},vector<int>{},vector<int>{}, false, 0,0};
    int num_bottom = 0;
    int reached_depth = 0;
    for (int i = 0; i < gates.size(); i++) {
        if (gates[i].collected == -1) { // not part of a subgraph
        if(gates[i].depth == 0){
            num_bottom++;
        }
            sg.gates.push_back(i);
            gates[i].collected = idsg; // mark as collected in the last subgraph
            // find out what the dependencies of the subgraph are
            for (int j = 0; j < gates[i].parents.size(); j++) {
                //cout << "parent: " << gates[i].parents[j] << endl;
                int parent = gates[i].parents[j];
                if (gates[parent].collected != -1 && !isInput(gates[parent].id) && gates[parent].collected != idsg) { // parent is part of a different subgraph
                    if (std::find(sg.dependencies.begin(), sg.dependencies.end(), gates[parent].collected) == sg.dependencies.end()) {
                        sg.dependencies.push_back(gates[parent].collected);
                       // cout << "added dependency: " << gates[parent].collected << endl;
                    }
                }
            }            
        } else {
            if(reached_depth < gates[i].depth){
                reached_depth = gates[i].depth;
            }
            
        }
    }

    cout << "gates left in bottom layer: " << num_bottom << " reached depth " << reached_depth << endl;
    sg.closed = true;
    subgraphs.push_back(sg);
}

void CircuitGraph::executable_order(){
    // brings gates into executable order by using their depth
    // input gates are not collected
    for (int i = 0; i <= max_depth; i++){
        for (int j = 0; j < gates.size(); j++){
            if (gates[j].depth == i){
                executable.push_back(j);
            }
        }   
    }
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

int CircuitGraph::depth_statistics_subgraphs(string file, std::vector<int> subgraphs_ids){
    // count how many gates per depth there are per subgraph
    bool write = true;
    int max_depth_sg = 0;
    std::ofstream f(file);
    if (!f.is_open()) {
        write = false;
    }

    for(int i = 0; i < subgraphs_ids.size();i++){
        std::vector<int> depth_count(max_depth + 1, 0);

        for (const auto& gate_id : subgraphs[subgraphs_ids[i]].gates) {
            Node gate = gates[gate_id];
            if (gate.depth >= 0 && gate.depth < depth_count.size()) {
                depth_count[gate.depth]++;
                if(gate.depth > max_depth_sg){
                    max_depth_sg = gate.depth; // find the maximum depth in all subgraphs
                }
            }
            
        }
        if(write){ f << "Subgraph " << subgraphs_ids[i] << ":\n"; }
        for (int i = 0; i < depth_count.size(); i++) {
            if(depth_count[i] == 0){
                continue; // skip empty depths
            }
            if(write){f << "Depth " << i << ": " << depth_count[i] << "\n";}
        }
    }
    if(write){f.close();}
    return max_depth_sg;
}