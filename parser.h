#ifndef __PARSER__H__
#define __PARSER__H__

#include <cstdlib>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <ctype.h>
#include <string.h>

#include "compiler.h"
#include "lexer.h"

using namespace std;

class Parser
{
    private:
        
    public:
        vector<string> localvarNames;
        vector<int> localMem;

        LexicalAnalyzer lexer;
        vector<Function*> functions;

        bool isMain;

        void syntax_error(TokenType expected, Token actual);
		void expect(TokenType token);
        void addToMem(Token t, int value);
        void addToGlobalMem();
        void clearLocalMem();
        void getGlobalMem();
        int location(string varName);

        struct InstructionNode* parse_program();
        void parse_var_section();
        vector<string> parse_id_list(bool write);
        void parse_func_decl_list();
        struct Function* parse_func_decl();
        struct InstructionNode* parse_function_body();
        struct InstructionNode* parse_body();
        struct InstructionNode* parse_stmt_list();
        struct InstructionNode* parse_stmt();
        struct InstructionNode* parse_assign_stmt();
        int parse_primary();
        void parse_expr(struct InstructionNode* node);
        struct InstructionNode* parse_function_call();
        ArithmeticOperatorType parse_op();
        struct InstructionNode* parse_print_stmt();
        struct InstructionNode* parse_while_stmt();
        struct InstructionNode* parse_if_stmt();
        void parse_condition(struct InstructionNode* node);
        ConditionalOperatorType parse_relop();
        struct InstructionNode* parse_switch_stmt();
        struct InstructionNode* parse_for_stmt();
        struct InstructionNode* parse_case_list(int operand1_index, struct InstructionNode* label);
        struct InstructionNode* parse_default_case();
        struct InstructionNode* parse_case(int operand1_index);
};

#endif  //__PARSER__H__