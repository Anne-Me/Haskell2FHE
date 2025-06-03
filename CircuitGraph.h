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
    INPUT,
    OUTPUT
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
        default: throw std::invalid_argument("unknown gate");
    }
}


// struct that implements a gate
struct Node{
    int id; // must match position in gates vector
    GATES type;
    std::vector<int> parents;
    std::vector<int> children;
    int depth;  
    int collected; // -1 if not yet assigned to a subgraph, the index of the subgraph otherwise
    int out; // for all inner nodes out = id but for nodes connected to output registers it is the output register id
};  


struct SubGraph{
    int id;
    std::vector<int> gates;
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
    std::vector<Node> gates; // gates must always be in an order such that they can be executed
    int input_length;
    int output_length;
    std::vector<SubGraph> subgraphs;



    private:
    int bottom_layer;

    public:

    CircuitGraph(int num_gates);

    CircuitGraph();

    void addChild(int id, int cildId);

    void computeDepths();

    bool isInOut(int id);

    void defineSubgraphs(int);

    bool collect_parents(std::stack<int> &parent_nodes, int current_node, int t);

    void collect_remaining();

    void write(std::string filename);

    void write_subgraphs(std::string file_prefix);

    void push_back_Gate(int id, GATES type, std::vector<int> parents, int out);
    };



#endif //CIRCUITGRAPH_H
