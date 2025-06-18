#ifndef CLASHTOFHE_EVALUATOR_H
#define CLASHTOFHE_EVALUATOR_H

#include <tfhe/tfhe.h>
#include <tfhe/tfhe_io.h>
#include <iostream>
#include "nlohmann/json.hpp"
#include"CircuitGraph.h"
#include <string>


using json = nlohmann::json;

class Evaluator {

public:

    LweSample* working_registers;
    LweSample* input_registers;
    LweSample* output_registers;

    int length_input;
    int length_output;
    int length_working;

    CircuitGraph* CG;
    const TFheGateBootstrappingCloudKeySet* bk;

    void init(CircuitGraph* CG, const TFheGateBootstrappingCloudKeySet* key, const TFheGateBootstrappingParameterSet* params, LweSample* input_registers);
    void per_level_parallel(int num_threads);
    void parallel_evaluate(int num_threads);
    void evaluate_subgraph(int t); // t is subgraph id
    void evaluate_gate(int gate_id);
    LweSample* find_register(int id);

    void move_outputs();
};


#endif //CLASHTOFHE_EVALUATOR_H
