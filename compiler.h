/*
 * Copyright (C) Rida Bazzi, 2017
 *
 * Do not share this file with anyone
 */
#ifndef _COMPILER_H_
#define _COMPILER_H_

#include <string.h>
#include <string>
#include <vector>

using namespace std;

extern string varNames[1000];
extern int mem[1000];

extern int stack_pointer;
extern int frame_pointer;

extern std::vector<int> inputs;
extern int next_input;

enum ArithmeticOperatorType {
    OPERATOR_NONE = 123,
    OPERATOR_PLUS,
    OPERATOR_MINUS,
    OPERATOR_MULT,
    OPERATOR_DIV
};

enum ConditionalOperatorType {
    CONDITION_GREATER = 345,
    CONDITION_LESS,
    CONDITION_NOTEQUAL
};

enum InstructionType
{
    NOOP = 1000,
    PRINTIN,
    ASSIGN,
    CJMP,
    JMP,
    FUNCTION
};

struct Function
{
    string name;
    vector<string> localvarNames;
    vector<int> localMem;
    struct InstructionNode* body;
};

struct InstructionNode
{
    InstructionType type;

    union
    {
        struct
        {
            int left_hand_side_index;
            int operand1_index;
            int operand2_index;
            
            /*
             * If op == OPERATOR_NONE then only operand1 is meaningful.
             * Otherwise both operands are meaningful
             */
            ArithmeticOperatorType op;
        } assign_inst;
        
        struct
        {
            struct Function* function;
            vector<int>* operators;
        } function_inst;
        
        struct
        {
            int var_index;
        } print_inst;
        
        struct {
            ConditionalOperatorType condition_op;
            int operand1_index;
            int operand2_index;
            struct InstructionNode * target;
        } cjmp_inst;
        
        struct {
            struct InstructionNode * target;
        } jmp_inst;
  
    };

    struct InstructionNode * next; // next statement in the list or NULL
};

void debug(const char* format, ...);

struct InstructionNode * parse_generate_intermediate_representation();

#endif /* _COMPILER_H_ */
