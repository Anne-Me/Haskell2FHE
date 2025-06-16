#ifndef CIRCUITGRAPH_H
#define CIRCUITGRAPH_H

#include <vector>
#include <stack>
#include <string>
#include <stdexcept>

enum GATES{
    AND,
    OR,
    NAND,
    XOR,
    NOR,
    XNOR,
    ANDYN,
    NOT,
    ORYN,
    MUX,
    INPUT,
    OUTPUT,
    CONST,
    CONSTW
};

inline GATES convert(std::string gate_str){
    if (gate_str == "$_NAND_") return NAND;
    else  if(gate_str == "$_ANDNOT_") return ANDYN; // A & ~B
    else if(gate_str == "$_OR_") return OR;
    else if(gate_str == "$_AND_") return AND;
    else if(gate_str == "$_XOR_") return XOR;
    else if(gate_str == "$_XNOR_") return XNOR;
    else if(gate_str == "$_NOR_") return NOR;
    else if(gate_str == "$_NOT_") return NOT;
    else if(gate_str == "$_ORNOT_") return ORYN;
    else if(gate_str == "$_MUX_") return MUX;
    else if(gate_str == "AND") return AND;
    else if(gate_str == "XOR") return XOR;
    else if(gate_str == "INV") return NOT;
    else if(gate_str == "EQ") return CONST;
    else if(gate_str == "EQW") return CONSTW;
    else throw std::invalid_argument("unknown gate " + gate_str);
}

inline std::string to_string(GATES gate){
    switch (gate){
        case NAND: return "$_NAND_";
        case ANDYN: return "$_ANDNOT_";
        case OR: return "$_OR_";
        case AND: return "$_AND_";
        case XOR: return "$_XOR_";
        case XNOR: return "$_XNOR_";
        case NOR: return "$_NOR_";
        case NOT: return "$_NOT_";
        case ORYN: return "$_ORNOT_";
        case INPUT: return "INPUT";
        case OUTPUT: return "OUTPUT";
        case MUX: return "$_MUX_";
        default: throw std::invalid_argument("unknown gate");
    }
}

inline std::string to_string_bristol(GATES gate){
    switch (gate){
        case AND: return "AND";
        case XOR: return "XOR";
        case NOT: return "INV";
        case INPUT: return "INPUT";
        case CONST: return "EQ";
        case CONSTW: return "EQW";
        default: throw std::invalid_argument("unknown gate");
    }
}


// struct that implements a gate
struct Node{
    int id; // uniqure ide, must match position in gates vector
    GATES type;
    std::vector<int> parents;
    std::vector<int> children;
    int depth;  
    int collected; // -1 if not yet assigned to a subgraph, the index of the subgraph otherwise
    int out; // output register 
};  


struct SubGraph{
    int id;
    std::vector<int> gates; // contains gate ids in executable order
    std::vector<int> inputs;
    std::vector<int> outputs;
    std::vector<int> dependencies;

    // these three should probably not be here, only needed in one function
    bool closed = false; // is the subgraph done being constructed
    int head = 0; // lowest node in subgraph that has not had all it's children explored
    int head_child = 0; // child of the node that needs exploring next
};


class CircuitGraph
    {

    public: 
    std::vector<Node> gates; 
    int input_length;
    int output_length;
    std::vector<SubGraph> subgraphs;
    std::vector<int> executable; // vector conatins gates in executable order, contains gate id


    int max_depth;

    private:
    int bottom_layer;

    public:

    CircuitGraph(int num_gates);

    CircuitGraph();

    void resize(int num);

    void addChild(int id, int cildId);

    void computeDepths();

    void defineSubgraphs(int);

    void defineSubgraphs_test(int num_Threads);

    void collect_remaining();

    void executable_order();

    void write(std::string filename);

    void write_subgraphs(std::string file_prefix);

    void push_back_Gate(int id, GATES type, std::vector<int> parents, int out);
    void set_gate(int id, GATES type, std::vector<int> parents, int out);
    bool identify_output(int id);

    private:

    int propagateDepth(int id, int depth);

    bool isInOut(int id);

    bool collect_parents(std::stack<int> &parent_nodes, int current_node, int t);

    };

#endif //CIRCUITGRAPH_H
