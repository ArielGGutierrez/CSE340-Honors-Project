/*
 * Copyright (C) Rida Bazzi, 2017
 *
 * Do not share this file with anyone
 */
#include <cstdio>
#include <cstdlib>
#include <cstdarg>
#include <cctype>
#include <cstring>
#include <string>

#include "compiler.h"

using namespace std;

#define DEBUG 1     // 1 => Turn ON debugging, 0 => Turn OFF debugging

string varNames[1000];
int mem[1000];

int stack_pointer = 0;
int frame_pointer = 0;
vector<InstructionNode*> return_addresses;

std::vector<int> inputs;
int next_input = 0;

void debug(const char* format, ...)
{
    va_list args;
    if (DEBUG)
    {
        va_start (args, format);
        vfprintf (stdout, format, args);
        va_end (args);
    }
}

void execute_program(struct InstructionNode * program)
{
    struct InstructionNode * pc = program;
    int op1, op2, result, i;
    struct Function* func;
    do
    {
        switch(pc->type)
        {
            case FUNCTION:
                func = pc->function_inst.function;
                for (i = 0; i < pc->function_inst.operators->size(); i++)
                {
                    func->localMem[i+1] = mem[frame_pointer + pc->function_inst.operators->at(i)];
                }
                mem[stack_pointer + frame_pointer] = frame_pointer;
                return_addresses.push_back(pc->next);
                frame_pointer += stack_pointer+1;
                stack_pointer = 0;
                for (int i = 0; i < func->localMem.size(); i++)
                {
                    varNames[frame_pointer + stack_pointer] = func->localvarNames[i];
                    mem[frame_pointer + stack_pointer] = func->localMem[i];
                    stack_pointer++;
                }
                pc = func->body;
                break;
            case NOOP:
                pc = pc->next;
                break;
            case PRINTIN:
                printf("%d ", mem[frame_pointer + pc->print_inst.var_index]);
                pc = pc->next;
                break;
            case ASSIGN:
                switch(pc->assign_inst.op)
                {
                    case OPERATOR_PLUS:
                        op1 = mem[frame_pointer + pc->assign_inst.operand1_index];
                        op2 = mem[frame_pointer + pc->assign_inst.operand2_index];
                        result = op1 + op2;
                        break;
                    case OPERATOR_MINUS:
                        op1 = mem[frame_pointer + pc->assign_inst.operand1_index];
                        op2 = mem[frame_pointer + pc->assign_inst.operand2_index];
                        result = op1 - op2;
                        break;
                    case OPERATOR_MULT:
                        op1 = mem[frame_pointer + pc->assign_inst.operand1_index];
                        op2 = mem[frame_pointer + pc->assign_inst.operand2_index];
                        result = op1 * op2;
                        break;
                    case OPERATOR_DIV:
                        op1 = mem[frame_pointer + pc->assign_inst.operand1_index];
                        op2 = mem[frame_pointer + pc->assign_inst.operand2_index];
                        result = op1 / op2;
                        break;
                    case OPERATOR_NONE:
                        op1 = mem[frame_pointer + pc->assign_inst.operand1_index];
                        result = op1;
                        break;
                }
                mem[frame_pointer + pc->assign_inst.left_hand_side_index] = result;
                pc = pc->next;
                break;
            case CJMP:
                if (pc->cjmp_inst.target == NULL)
                {
                    debug("Error: pc->cjmp_inst->target is null.\n");
                    exit(EXIT_FAILURE);
                }
                op1 = mem[frame_pointer + pc->cjmp_inst.operand1_index];
                op2 = mem[frame_pointer + pc->cjmp_inst.operand2_index];
                switch(pc->cjmp_inst.condition_op)
                {
                    case CONDITION_GREATER:
                        if(op1 > op2)
                            pc = pc->next;
                        else
                            pc = pc->cjmp_inst.target;
                        break;
                    case CONDITION_LESS:
                        if(op1 < op2)
                            pc = pc->next;
                        else
                            pc = pc->cjmp_inst.target;
                        break;
                    case CONDITION_NOTEQUAL:
                        if(op1 != op2)
                            pc = pc->next;
                        else
                            pc = pc->cjmp_inst.target;
                        break;
                }
                break;
            case JMP:
                if (pc->jmp_inst.target == NULL)
                {
                    debug("Error: pc->jmp_inst->target is null.\n");
                    exit(EXIT_FAILURE);
                }
                pc = pc->jmp_inst.target;
                break;
            default:
                debug("Error: invalid value for pc->type (%d).\n", pc->type);
                exit(EXIT_FAILURE);
                break;
        }

        if (pc == NULL && !return_addresses.empty()) // Return from function
        {
            int frame = mem[frame_pointer-1];
            mem[frame_pointer-1] = mem[frame_pointer];
            varNames[frame_pointer-1] = varNames[frame_pointer];
            //debug("Function returned %d\n", mem[frame_pointer]);
            for (int i = frame_pointer; i < stack_pointer; i++)
            {
                mem[frame_pointer + i] = 0;
                varNames[frame_pointer + i].clear();
            }
            stack_pointer = frame_pointer - frame - 1;
            frame_pointer = frame;
            pc = return_addresses[return_addresses.size()-1];
            return_addresses.pop_back();
        }
    } while(pc != NULL);
}

int main()
{
    struct InstructionNode * program;
    program = parse_generate_intermediate_representation();
    execute_program(program);
    return 0;
}
