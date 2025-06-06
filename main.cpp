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
reads json file and creates json output
*/
void read_json(string circuitpath, json& out, int num_ciphertext){
    ifstream f(circuitpath);
    json data = json::parse(f);

    int input_length=0;
    input_length =  data["modules"]["topEntity"]["ports"]["c$arg"]["bits"].size();     //length of the first input 
    for (int i = 0; i<num_ciphertext-1; i++) {
        input_length +=  data["modules"]["topEntity"]["ports"]["c$arg_"+to_string(i)]["bits"].size();  // length of whole input
    }



    int result_length =  data["modules"]["topEntity"]["ports"]["result"]["bits"].size();  // length of result

    cout << "input_length: " << input_length << " result_length: " << result_length << "\n";
    
    out["data"]["inputlength"] = input_length;
    out["data"]["resultlength"] = result_length;


    int counter =0;
    

    for (auto el : data["modules"]["topEntity"]["cells"].items())
    {
        string key = el.key();
        json conn = el.value()["connections"];
        string type = el.value()["type"];


        json info = {{"type", el.value()["type"]},{"inputA", el.value()["connections"]["A"][0]}, {"inputB", el.value()["connections"]["B"][0]}, {"output", el.value()["connections"]["Y"][0]}};   //from here, we can calculate largest value of working resigter op
        out["gates"].push_back({{"cells"+to_string(++counter), info}});         
    }
}

/*
reads json file and parses into CircuitGraph 
*/
void read_json_to_Circuit(string circuitpath, CircuitGraph &CG,int num_ciphertext){
    ifstream f(circuitpath);
    if (!f.is_open()) {
        throw std::runtime_error("Failed to open circuit file: " + circuitpath);
    }
    json data = json::parse(f);

    int input_length=0;
    input_length =  data["modules"]["topEntity"]["ports"]["c$arg"]["bits"].size();     //length of the first input 
    for (int i = 0; i<num_ciphertext-1; i++) {
        input_length +=  data["modules"]["topEntity"]["ports"]["c$arg_"+to_string(i)]["bits"].size();  // length of whole input
    }

    int temp_result_length = data["modules"]["topEntity"]["ports"]["result"]["bits"].size(); 
    int result_length =  0;
    for (int i = 0; i < temp_result_length; i++){
        try
        {
            if ((int)data["modules"]["topEntity"]["ports"]["result"]["bits"][i] != 0){
                result_length++;
            }
        }catch(const std::exception& e){
            continue;
        }
    }

    cout << "input_length: " << input_length << " result_length: " << result_length << "\n";
    
    CG.input_length = input_length;
    CG.output_length = result_length;

    // add all the input registers and output registers
    for (int i = 0; i < input_length; i++) {
        CG.push_back_Gate(i, GATES::INPUT, {}, input_length);
    }
    for (int i = 0; i < result_length; i++) {
        CG.push_back_Gate(input_length + i, GATES::OUTPUT, {}, input_length+i); 
    }


    int counter = input_length+result_length; // it should start at the first value after input registers
    int offset = 2; // why yosys
    for (auto el : data["modules"]["topEntity"]["cells"].items())
    {
        string key = el.key();
        json conn = el.value()["connections"];
        string type = el.value()["type"];
   
        GATES gate_type = convert(type);
      //  cout << "gate type: " << to_string(gate_type) << "counter: " << counter << endl;

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
          //  cout << "output: " << out << endl;
        }  

        CG.push_back_Gate(counter, gate_type, parents, out-offset);
       // cout << "pushed gate" << endl;;

        if (conn.contains("A")) {
          //  cout << "add A" << endl;
            CG.addChild(in_1-offset, counter);
         //   cout << "added child A" << endl;
        }
        if( conn.contains("B")) {
            CG.addChild(in_2-offset, counter);
          //  cout << "added child B" << endl;
        }       

        counter++;
    }

}

void printerror(){
    std::cerr << "Usage: ./clashtoFHE -c jsoncircuitfile -n n a0 a1 .. a7 b1 b2 .. b7 -b bitlength [-t threads] -cloud cloudkey -out outfile " << std::endl;
    // optinall add -print for printing
    // -test for testing only
}

int main(int argc, char** argv) {

    auto begin = std::chrono::high_resolution_clock::now();


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

    for (int i = 1; i < argc; ++i)
    {
        if (string("-c") == argv[i])
        {
            if (argc <= i + 1)
            {
                printerror();
                return -1;
            }

            circuitpath = string(argv[++i]);
        }
        

        else if (string("-n")==argv[i])
        {
            if (argc <= i + 1)
            {
                printerror();
                return -1;
            }

        num_ciphertext=std::stoi(argv[++i]);

         for (int j = 0; j < num_ciphertext; ++j)
            {
                input_files.push_back(argv[++i]); // save the names of the input files
            }

           
        }
        else if (string("-out") == argv[i] ){
            if (argc <= i + 1)
            {
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
        } else {
            std::cerr << "Unknown argument: " << argv[i] << std::endl;
            printerror();
            return -1;
        }
    }



    // Read the cloud key from file
    FILE *cloud_key_file = fopen(cloud_key_filename, "rb");
    if (cloud_key_file == NULL) {
        perror("No cloud is given");
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
   


    //process jsonfile
    json out;
    //read_json(circuitpath, out, num_ciphertext);
    CircuitGraph CG;
    read_json_to_Circuit(circuitpath, CG, num_ciphertext);
    cout << "read circuit" << endl;
    
    if (k > 1){
        CG.computeDepths();
        CG.defineSubgraphs(k);
        for (int i = 0; i < k; i++){
            cout << "subgraph " << i << " has " << CG.subgraphs[i].gates.size() << " gates" << endl;
        }

        CG.collect_remaining();
        cout << "remaining gates: " << CG.subgraphs[k].gates.size() << endl;
        if(print == true){
            CG.write_subgraphs("splitPIR3ways"); 
        }

        cout << "Splitting done" << endl;
    }



    Evaluator evaluator;

    evaluator.init(&CG, cloud_key, params, input_registers);
    evaluator.parallel_evaluate(k); // TODO

    cout << "evaluated" << endl;


    FILE *ciphertext_file = fopen(out_file, "wb");
    if (ciphertext_file == NULL) {
        perror("Failed to open ciphertext file");
        return 1;
    }
    int resultlength = CG.output_length;


    for (int j = 0; j < resultlength; j++) { 
        export_gate_bootstrapping_ciphertext_toFile(ciphertext_file, &evaluator.output_registers[j], params);
    }
    fclose(ciphertext_file);

    cout << "wrote output" << endl;

     auto finish = std::chrono::high_resolution_clock::now();

     auto interval = std::chrono::duration_cast<std::chrono::microseconds>(finish - begin).count();

    double seconds = interval / 1000000;
    double milliseconds = interval / 1000;


    std::cout << "Total execution time: "
              << std::fixed << std::setprecision(6) << seconds << " seconds ("
              << std::fixed << std::setprecision(3) << milliseconds << " milliseconds)"
              << std::endl;
    return 0;
    
}
