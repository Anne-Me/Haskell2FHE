#include <iostream>
#include <chrono>

#include <fstream>
#include "nlohmann/json.hpp"
#include <stdexcept>
#include <cstdio>

#include <tfhe/tfhe.h>
#include <tfhe/tfhe_io.h>

#include "tests/circuitgraph_tests.h"

#include "Evaluator.h"
#include "CircuitGraph.h"

using namespace std;

using json = nlohmann::json;


/*
reads json file and parses into CircuitGraph 
*/
void read_json_to_Circuit(string circuitpath, CircuitGraph &CG){
    ifstream f(circuitpath);
    if (!f.is_open()) {
        throw std::runtime_error("Failed to open circuit file: " + circuitpath);
    }
    json data = json::parse(f);

    int input_length=0;
    int result_length = 0;

    for (auto el : data["modules"]["topEntity"]["ports"].items()){
        if(el.value()["direction"] == "input"){
            input_length +=  el.value()["bits"].size();
        } else if(el.value()["direction"] == "output"){
            result_length +=  el.value()["bits"].size();
        } else {
            cout << "Unknoen port type: " << el.value()["direction"] << endl;
        }
    }

    int num_gates = data["modules"]["topEntity"]["cells"].size();

    cout << "input_length: " << input_length << " result_length: " << result_length << " gates: " << num_gates << endl;
    
    CG.input_length = input_length;
    CG.output_length = result_length;
    CG.resize(input_length + num_gates);

    // add all the input registers 
    for (int i = 0; i < input_length; i++) {
        CG.set_gate(i, GATES::INPUT, {}, i);
    }

    int counter = input_length+result_length; 
    int offset = 2; // why yosys
    for (auto el : data["modules"]["topEntity"]["cells"].items())
    {
        string key = el.key();
        json conn = el.value()["connections"];
        string type = el.value()["type"];
   
        GATES gate_type = convert(type);
        //cout << "gate type: " << to_string(gate_type) << "counter: " << counter << endl;

        vector<int> parents;
        int in_1;
        int in_2;       
        int out;
        if (conn.contains("A")) {
            in_1 = conn["A"][0];
            parents.push_back(in_1-offset);
          //  cout << "input A: " << in_1 << endl;
        } else {
            std::cerr << "Error: Gate " << counter+offset << " has no input A." << std::endl;
        }
        if (conn.contains("B")) {
            in_2 = conn["B"][0];
            parents.push_back(in_2-offset);
          //  cout << "input B: " << in_2 << endl;
        }else if (gate_type != GATES::NOT){
            std::cerr << "Error: Gate " << counter+offset << " has no input B." << std::endl;
        }     
         // Y is not giving us the child info but register to place otput
        if (conn.contains("Y")) {
            out = (int)conn["Y"][0];
           // cout << "output: " << out << endl;
        }  
        try
        {  
        CG.set_gate(out-offset, gate_type, parents, out-offset);
        }
        catch(const std::exception& e)
        {
            cout << "Error while setting gate: " << out-offset << endl;
            cout << "A" << in_1-offset << " B: " << in_2-offset << " out: " << out-offset << endl;
            std::cerr << e.what() << '\n';
        }
        
       // cout << "pushed gate" << endl;;

        if (conn.contains("A")) {
          //  cout << "add A" << endl;
            CG.addChild(in_1-offset, out - offset);
         //   cout << "added child A" << endl;
        }
        if( conn.contains("B")) {
            CG.addChild(in_2-offset, out - offset);
          //  cout << "added child B" << endl;
        }       

        counter++;
    }

}

void read_bristol_to_Circuit(string circuitpath, CircuitGraph &CG){
    ifstream f(circuitpath);
    if (!f.is_open()) {
        throw std::runtime_error("Failed to open circuit file: " + circuitpath);
    }
    int num_gates, num_wires;
    f >> num_gates >> num_wires;

    int input_length = 0;
    int num_inputs = 0;
    int output_length = 0;
    int num_outputs = 0;
    int temp;
    f >> num_inputs;
    for (int i = 0; i < num_inputs; i++){
        f >> temp;
        input_length += temp;
    }
    f >> num_outputs;
    for (int i = 0; i < num_outputs; i++){
        f >> temp;
        output_length += temp;
    }

    cout << "input_length: " << input_length << " result_length: " << output_length << " gates: " << num_gates << endl;

    CG.input_length = input_length;
    CG.output_length = output_length;
    CG.resize(input_length + num_gates);

    for (int i = 0; i < input_length; i++) {
        CG.set_gate(i, GATES::INPUT, {}, i);
    }
    
    for(int i = 0; i < num_gates; i++){
        vector<int> parents;
        int num_in,num_out,in_1,in_2,out;
        f >> num_in;
        f >> num_out;
        if(num_in == 1){
            f >> in_1;
            parents.push_back(in_1);
        } else if (num_in == 2){
            f >> in_1;
            f >> in_2;
            parents.push_back(in_1);
            parents.push_back(in_2);
        } else {
            throw std::runtime_error("Invalid number of inwires. For MAND gates please transform to AND gates first.");
        }
        if(num_out == 1){
            f >> out;
            CG.addChild(in_1, out); 
            if(num_in == 2){
                CG.addChild(in_2, out);
            }
        } else {
            throw std::runtime_error("Invalid number of outwires.");
        }

        string type;
        f >> type;
        GATES gate_type = convert(type);

        try
            {  
                CG.set_gate(out, gate_type, parents, out);
                /*
                if (out % 20 == 0){
                    cout << "set gate: " << out << " type: " << to_string(gate_type) << " parents " << CG.gates[out].parents[0];
                    if(CG.gates[out].parents.size() > 1){
                        cout  <<" " << CG.gates[out].parents[1] << endl;
                    } else {cout << endl;}
                    
                }*/
            }
            catch(const std::exception& e)
            {                    
                cout << "Error while setting gate: " << out << endl;
                cout << "A" << in_1 << " B: " << in_2 << " out: " << out << endl;
                std::cerr << e.what() << '\n';
            }
        }
}

void printerror(){
    std::cerr << "Usage: ./clashtoFHE -c jsoncircuitfile -n n a0 a1 .. a7 b1 b2 .. b7 -b bitlength [-t threads] -cloud cloudkey -out outfile " << std::endl;
    // optinall add -print for printing
    // -test for testing only
}

int main(int argc, char** argv) {



    if (argc == 1) {
        printerror();
        return 1;
    }

    string circuitpath;

    int num_ciphertext;
    int bitlength; 
    int k = 1; // number of threads
   
    std::vector<char *> input_files;
    char* out_file;
    char* cloud_key_filename;

    bool print = false;
    bool eval = true;
    string format = "json"; 

    for (int i = 1; i < argc; ++i)
    {
        if (string("-c") == argv[i]){
            if (argc <= i + 1){
                printerror();
                return -1;
            }
            format = "json";
            circuitpath = string(argv[++i]);
            cout << "circuit path: " << circuitpath << endl;
        }

        else if (string("-cbristol") == argv[i]){
            if (argc <= i + 1){
                printerror();
                return -1;
            }
            format = "bristol";
            circuitpath = string(argv[++i]);
        }
        

        else if (string("-n")==argv[i]){
            if (argc <= i + 1)
            {
                printerror();
                return -1;
            }

        num_ciphertext=std::stoi(argv[++i]);

         for (int j = 0; j < num_ciphertext; ++j){
                input_files.push_back(argv[++i]); // save the names of the input files
            }
        }
        else if (string("-out") == argv[i] ){
            if (argc <= i + 1){
                printerror();
                return -1;
            }
            out_file = argv[++i]; 
        }
       
        else if (string("-cloud") == argv[i] ){
            if (argc <= i + 1)
            {
                printerror();
                return -1;
            }

            cloud_key_filename = argv[++i];
        }

        else if (string("-b") == argv[i] ){
            if (argc <= i + 1)
            {
                printerror();
                return -1;
            }

            bitlength = atoi(argv[++i]);
        } else if (string("-t") == argv[i]) {
            if (argc <= i + 1)
            {
                printerror();
                return -1;
            }

            k = atoi(argv[++i]);
        }
        
         else if (string("-test") == argv[i]){
            createSimpleCircuitPlus2split3();
            return 0;
        } else if (string("-print") == argv[i]){
            print = true;
        } else if (string("-noeval") == argv[i]){
            eval = false;
        }      
        else {
            std::cerr << "Unknown argument: " << argv[i] << std::endl;
            printerror();
            return -1;
        }
    }
   
    //process circuit

    CircuitGraph CG;
    if (format == "json"){
        read_json_to_Circuit(circuitpath, CG);
        cout << "read circuit" << endl;
    } else if (format == "bristol"){
        read_bristol_to_Circuit(circuitpath, CG);
        cout << "read circuit" << endl;
    } else {
        cerr << "Unknown format" << endl;
        return -1;
    }


    CG.computeDepths();
    CG.executable_order();
    cout << "max depth " << CG.max_depth << " executable gates: " << CG.executable.size() << endl;
    cout << endl;
    
    
    if (k > 1){
        CG.defineSubgraphs_test(k,0);
        for (int i = 0; i < k; i++){
            cout << "subgraph " << i << " has " << CG.subgraphs[i].gates.size() << " gates" << endl;
        }

        CG.collect_remaining();
        cout << "remaining gates: " << CG.subgraphs[k].gates.size() << endl;
        if(print == true){
            //CG.write_subgraphs("splitPIR3ways"); 
            cout << "no writing" << endl;
        }

        cout << "Splitting done" << endl;
    }
    if (!eval) {
        cout << "Not evaluating, exit" << endl;
        return 0;
    }

    // Read the cloud key from file
    FILE *cloud_key_file = fopen(cloud_key_filename, "rb");
    if (cloud_key_file == NULL) {
        perror("No bootstrapping key is given");
        return 1;
    }

    TFheGateBootstrappingCloudKeySet *cloud_key = new_tfheGateBootstrappingCloudKeySet_fromFile(cloud_key_file);

    const TFheGateBootstrappingParameterSet *params = cloud_key->params;  //retrieving the TFHE parameters from the secret key set.
    fclose(cloud_key_file);

    LweSample* input_registers = new_gate_bootstrapping_ciphertext_array(num_ciphertext * bitlength, params);

    LweSample* temp;

         // read the ciphertexts
    for (int i = 0; i<num_ciphertext; i++)
    {
        FILE* in = fopen(input_files[i], "rb");
        if (in == NULL) {throw std::invalid_argument("failed to open ciphertext file");} 

        for (int j = 0; j<bitlength; j++) { 
            import_gate_bootstrapping_ciphertext_fromFile(in, &input_registers[j + bitlength * i], params);
          
        }
        fclose(in);
    }
    cout << "read inputs" << endl;
   

    
    Evaluator evaluator;

    evaluator.init(&CG, cloud_key, params, input_registers);
    auto begin = std::chrono::high_resolution_clock::now();

    evaluator.parallel_evaluate(k); 
    auto finish = std::chrono::high_resolution_clock::now();

    auto interval = std::chrono::duration_cast<std::chrono::microseconds>(finish - begin).count();

    double seconds = interval / 1000000;
    double milliseconds = interval / 1000;

    cout << "evaluated" << endl;


    FILE *ciphertext_file = fopen(out_file, "wb");
    if (ciphertext_file == NULL) {
        perror("Failed to open ciphertext file");
        return 1;
    }
    int resultlength = CG.output_length;

    /*if(format == "bristol"){
        evaluator.move_outputs();
    }*/

    cout << CG.output_length << " output length " << CG.gates.size() << "gates.size" << endl;
    if (format == "bristol") { // output gates are the last registers
        int first_gate = CG.gates.size() - CG.output_length; 
        int eval_first_gate = evaluator.length_working - evaluator.length_output; // first gate is the first working register
        for (int j = 0; j < resultlength; j++) {             
            export_gate_bootstrapping_ciphertext_toFile(ciphertext_file, &evaluator.working_registers[eval_first_gate + j], params);
        }
    } else if (format == "json"){ // output gates are the first registers after input
        for (int j = 0; j < resultlength; j++) { 
            export_gate_bootstrapping_ciphertext_toFile(ciphertext_file, &evaluator.output_registers[j], params);
        }
    }
    fclose(ciphertext_file);

    cout << "wrote output" << endl;

   


    std::cout << "Evaluation execution time: "
              << std::fixed << std::setprecision(6) << seconds << " seconds ("
              << std::fixed << std::setprecision(3) << milliseconds << " milliseconds)"
              << std::endl;
    return 0;
    
}
