#include "Evaluator.h"
#include <iostream>
#include "stdexcept"
#include <thread>

using json = nlohmann::json;
using namespace std;



void Evaluator::init(CircuitGraph* CG, const TFheGateBootstrappingCloudKeySet* key, const TFheGateBootstrappingParameterSet* params, LweSample* input_registers) {
   
    this->CG = CG;
    bk = key;
    this->length_input = CG->input_length;
    this->length_output = CG->output_length;
   
    this->length_working = CG->gates.size() - CG->input_length - CG->output_length; // all gates that are not input or output are working registers

    this->working_registers = new_gate_bootstrapping_ciphertext_array(length_working, params);
    this->input_registers = input_registers;
    this->output_registers = new_gate_bootstrapping_ciphertext_array(length_output, params);
    cout << "input length: " << length_input << " output length: " << length_output << " working length: " << length_working << endl;


}

void Evaluator::parallel_evaluate(int num_threads){
    if (CG->subgraphs.size() == 0 || num_threads == 1) {
    // do single threaded stuff
    /*
        int first_gate = length_input + length_output; // first gate is the first working register

        for (int i = first_gate; i < CG->gates.size(); i++) {
            evaluate_gate(i);
            //cout << "evaluated gate: " << i << " of " << CG->gates.size() << " type " << to_string(CG->gates[i].type) << std::endl;
        }
        */
        for(int i = 0; i < CG->executable.size(); i++){
            int gate_id = CG->executable[i];
            evaluate_gate(gate_id);
            cout << "evaluated gate: " << gate_id << " of " << CG->executable.size() << " type " << to_string(CG->gates[gate_id].type) << std::endl;
        }
        cout << "done evaluating whole circuit." << std::endl;
        return;
    }
    if (num_threads >= CG->subgraphs.size()) {
        cout << " not using optimally many threads, using " << CG->subgraphs.size() << " threads instead of " << num_threads << std::endl;
        num_threads = CG->subgraphs.size();
    } 
    // do multi-thread stuff
    vector<std::thread> threads;
    threads.reserve(num_threads);

    for (int i = 0; i < num_threads; ++i) {
        threads.emplace_back(&Evaluator::evaluate_subgraph,this, i);
    }
    cout << "started " << num_threads << " threads to evaluate subgraphs" << std::endl;

    // wait for threads
    for (auto& t : threads) {
        if (t.joinable()) {
            t.join();
        }
    }
    cout << "joined all threads" << std::endl;
    // execute remaining gates
    evaluate_subgraph(num_threads);
}

void Evaluator::evaluate_subgraph(int t){
    for (auto gate_id : CG->subgraphs[t].gates) {  
        evaluate_gate(gate_id);
    }
    cout << "done evaluating subgraph: " << t << std::endl;
}


void Evaluator::evaluate_gate(int gate_id) {
    if (gate_id < 0 || gate_id >= CG->gates.size()) {
        throw std::invalid_argument("Gate ID out of range");
    }     
        Node node = CG->gates[gate_id];
        GATES gate = node.type;
        switch (gate){
             case NAND: {
                int ct_ipA = node.parents[0]; // TODO is the order of parents kept when creatig node?
                int ct_ipB = node.parents[1];
                int ct_out = node.out;
                bootsNAND(find_register(ct_out), find_register(ct_ipA), find_register(ct_ipB), bk);
                break;
            }
             case OR: {
                int ct_ipA = node.parents[0]; 
                int ct_ipB = node.parents[1];
                int ct_out = node.out;
                bootsOR(find_register(ct_out), find_register(ct_ipA), find_register(ct_ipB), bk);
                break;
            }
             case AND: {
                int ct_ipA = node.parents[0]; 
                int ct_ipB = node.parents[1];
                int ct_out = node.out;
                bootsAND(find_register(ct_out), find_register(ct_ipA), find_register(ct_ipB), bk);
                break;
            }
             case XOR: {
                int ct_ipA = node.parents[0]; 
                int ct_ipB = node.parents[1];
                int ct_out = node.out;
                bootsXOR(find_register(ct_out), find_register(ct_ipA), find_register(ct_ipB), bk);
                break;
            }
             case XNOR: {
                int ct_ipA = node.parents[0]; 
                int ct_ipB = node.parents[1];
                int ct_out = node.out;
                bootsXNOR(find_register(ct_out), find_register(ct_ipA), find_register(ct_ipB), bk);
                break;
            }
             case NOR: {
                int ct_ipA = node.parents[0]; 
                int ct_ipB = node.parents[1];
                int ct_out = node.out;
                bootsNOR(find_register(ct_out), find_register(ct_ipA), find_register(ct_ipB), bk);
                break;  
            } 
            case ANDYN : {
                int ct_ipA = node.parents[0]; 
                int ct_ipB = node.parents[1];
                int ct_out = node.out;
                bootsANDYN(find_register(ct_out), find_register(ct_ipA), find_register(ct_ipB), bk);
                break;
            }
            case NOT : {
                int ct_ipA = node.parents[0]; 
                int ct_out = node.out;
                bootsNOT(find_register(ct_out), find_register(ct_ipA), bk);
                break;
            }
            case ORYN: {
                int ct_ipA = node.parents[0]; 
                int ct_ipB = node.parents[1];
                int ct_out = node.out;
                bootsORYN(find_register(ct_out), find_register(ct_ipA), find_register(ct_ipB), bk);
                break;
            }
            default: {
                cout << "gate type: " << to_string(gate) << endl;
            }

        }
}



// the circuit imagines registers to be attached like this [inputregisters][outputregister][workingregisters]

LweSample * Evaluator::find_register(int id) {
    if (id < length_input) {
        return &input_registers[id]; 
    } else if (id < length_input + length_output){
        return &output_registers[id - length_input];
    } else if (id < length_input+length_output+length_working){
        return &working_registers[id - length_input - length_output];
    } else {
        throw std::invalid_argument("index outside of registers");
    }
}