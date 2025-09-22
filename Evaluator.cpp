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
    //cout << "input length: " << length_input << " output length: " << length_output << " working length: " << length_working << endl;
}

/*
* takes as input the number of threads and iterates over each alyer of the circuit until max depth is reached
* per layer splits the gates in that layer into num_threads many subcircuits and starts concurrent threads
*/
void Evaluator::per_level_parallel(int num_threads){
    for (int i = 0; i <= CG->max_depth; i++) {
        CG->split_level(num_threads, i);
        vector<std::thread> threads;
        threads.reserve(num_threads);
        //cout << "evaluating level: " << i << " with " << CG->subgraphs.size() << " subgraphs" << std::endl;
        if(num_threads> 1){ 
            for (int j = 0; j < num_threads; ++j) {
                threads.emplace_back(&Evaluator::evaluate_subgraph,this, j);
            }
            // wait for threads
            for (auto& t : threads) {
                if (t.joinable()) {
                    t.join();
                }
            }
        } else {
            evaluate_subgraph(0);
        }
    }

}



void Evaluator::evaluate_subgraph(int t){
    for (auto gate_id : CG->subgraphs[t].gates) {  
        evaluate_gate(gate_id);
    }
    //cout << "done evaluating subgraph: " << t << std::endl;
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
            case MUX: {
                int ct_ipA = node.parents[0]; 
                int ct_ipB = node.parents[1];
                int ct_ipC = node.parents[2];
                int ct_out = node.out;
                //cout << "MUX " << ct_ipA << ct_ipB << ct_ipC << ct_out << endl;
                /*
                LweSample* temp_registers  = new_gate_bootstrapping_ciphertext_array(4, bk->params);

                // (c AND a) OR (NOT(c) AND b)
                bootsAND(&temp_registers[0], find_register(ct_ipC), find_register(ct_ipA), bk);
                bootsNOT(&temp_registers[1], find_register(ct_ipC), bk);
                bootsAND(&temp_registers[2], &temp_registers[1], find_register(ct_ipB), bk);
                bootsOR(&temp_registers[3], &temp_registers[0], &temp_registers[2], bk);

                bootsCOPY(find_register(ct_out), &temp_registers[3], bk);
                
                
                delete_gate_bootstrapping_ciphertext_array(4,temp_registers);
                */
                bootsMUX(find_register(ct_out), find_register(ct_ipC), find_register(ct_ipA),find_register(ct_ipB), bk);
                break;
            }
            default: {
                cout << "unknwon gate type: " << to_string(gate) << endl;
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

void Evaluator::move_outputs(){
    int counter = 0;
    for(int i = 0; i < CG->gates.size(); i++){
        if(CG->identify_output(i)){
            if (counter > length_output) {
                throw std::out_of_range("More output registers than expected");
            }
            bootsCOPY(find_register(counter+length_input),find_register(i), bk);
            counter++;
        }
    }
}