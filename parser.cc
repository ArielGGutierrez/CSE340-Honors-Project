#include <cstdlib>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <ctype.h>
#include <string.h>

#include "compiler.h"
#include "lexer.h"
#include "parser.h"

std::string tokenString[] =
{
    "END_OF_FILE",
    "VAR", "FOR", "IF", "WHILE", "SWITCH", "CASE", "DEFAULT", "print", "ARRAY",
    "PLUS", "MINUS", "DIV", "MULT",
    "EQUAL", "COLON", "COMMA", "SEMICOLON",
    "LBRAC", "RBRAC", "LPAREN", "RPAREN", "LBRACE", "RBRACE",
    "NOTEQUAL", "GREATER", "LESS",
    "NUM", "ID", "ERROR"
};

void Parser::syntax_error(TokenType expected, Token actual)
{
    debug("SYNTAX ERROR !!!\nExpected (%d), Got (%d), on Line %d\n",
        expected, actual.token_type, actual.line_no);
    exit(EXIT_FAILURE);
}

void Parser::expect(TokenType token)
{
    Token t = lexer.peek(1);
    if (token == t.token_type)
    {
        t = lexer.GetToken();
    }
    else
    {
        syntax_error(token, t);
    }
}

void Parser::addToMem(Token t, int value)
{
    /* Dealt with variable section */
    if (location(t.lexeme) == -1)
    {
        localvarNames.push_back(t.lexeme);
        localMem.push_back(value);
    }
}

void Parser::addToGlobalMem()
{
    for (int i = 0; i < localMem.size(); i++)
    {
        int location = stack_pointer + frame_pointer;
        if (location < 1000)
        {
            varNames[location] = localvarNames[i];
            mem[location] = localMem[i];
            stack_pointer++;
        }
        else
        {
            debug("MEMORY ERROR !!!\nRan out of memory\n");
            exit(EXIT_FAILURE);
        }
    }
}

void Parser::getGlobalMem()
{
    clearLocalMem();
    for (int i = frame_pointer; i < stack_pointer; i++)
    {
        localMem.push_back(mem[frame_pointer + i]);
        localvarNames.push_back(varNames[frame_pointer + i]);
    }
}

void Parser::clearLocalMem()
{
    localMem.clear();
    localvarNames.clear();
}

int Parser::location(string name)
{
    for (int i = 0; i < localvarNames.size(); i++)
    {
        if (!localvarNames[i].empty())
        {
            if (localvarNames[i] == name)
            {
                return i;
            }
        }
    }

    return -1;
}

/* Parser */
//-------------------------------------------------------------------------------------------------
struct InstructionNode* Parser::parse_program()
{
    struct InstructionNode* head;
    parse_var_section();
    isMain = false;
    parse_func_decl_list();
    isMain = true;
    getGlobalMem();
    head = parse_body();
    return head;
}

void Parser::parse_var_section()
{
    parse_id_list(true);
    Token t = lexer.peek(1);
    if (t.token_type == SEMICOLON)
    {
        expect(SEMICOLON);
        addToGlobalMem();
        clearLocalMem();
    }
    else syntax_error(SEMICOLON, t);
}

vector<string> Parser::parse_id_list(bool write)
{
    string id;
    vector<string> ids;

    Token t = lexer.peek(1);
    if (t.token_type == ID)
    {
        expect(ID);

        if (write) addToMem(t, 0);
        else id = t.lexeme;

        t = lexer.peek(1);
        if (t.token_type == COMMA)
        {
            expect(COMMA);
            if (write) parse_id_list(true);
            else ids = parse_id_list(false);
        }
    }
    else syntax_error(ID, t);
    ids.insert(ids.begin(), id);
    return ids;
}

void Parser::parse_func_decl_list()
{
    parse_func_decl();
    clearLocalMem();

    Token t = lexer.peek(1);
    if (t.token_type == ID)
    {
        parse_func_decl_list();
    }
}

struct Function* Parser::parse_func_decl()
{
    Function* function = new Function;
    functions.push_back(function);

    Token t = lexer.peek(1);
    if (t.token_type == ID)
    {
        expect(ID);

        function->name = t.lexeme;
        addToMem(t, 0);

        t = lexer.peek(1);
        if (t.token_type == LPAREN)
        {
            expect(LPAREN);

            t = lexer.peek(1);
            if (t.token_type == ID)
            {
                parse_id_list(true);

                t = lexer.peek(1);
                if (t.token_type == RPAREN)
                {
                    expect(RPAREN);
                    
                    function->body = parse_function_body();
                    function->localvarNames = localvarNames;
                    function->localMem = localMem;
                }
                else syntax_error(RPAREN, t);
            }
            else syntax_error(ID, t);
        }
        else syntax_error(LPAREN, t);
    }
    else syntax_error(ID, t);
    return function;
}

struct InstructionNode* Parser::parse_function_body()
{
    struct InstructionNode* node;

    Token t = lexer.peek(1);
    if (t.token_type == LBRACE)
    {
        expect(LBRACE);

        parse_id_list(true);

        t = lexer.peek(1);
        if (t.token_type == SEMICOLON)
        {
            expect(SEMICOLON);

            node = parse_stmt_list();

            t = lexer.peek(1);
            if (t.token_type == RBRACE)
            {
                expect(RBRACE);
            }
            else syntax_error(RBRACE, t);
        }
        else syntax_error(SEMICOLON, t);
    }
    else syntax_error(LBRACE, t);
    return node;
}

struct InstructionNode* Parser::parse_body()
{
    struct InstructionNode* node;

    Token t = lexer.peek(1);
    if (t.token_type == LBRACE)
    {
        expect(LBRACE);

        node = parse_stmt_list();
        if (isMain)
        {
            stack_pointer = 0;
            addToGlobalMem();
        }
        t = lexer.peek(1);
        if (t.token_type == RBRACE)
        {
            expect(RBRACE);
        }

        else syntax_error(RBRACE, t);
    }

    else syntax_error(LBRACE, t);

    return node;
}

struct InstructionNode* Parser::parse_stmt_list()
{
    struct InstructionNode* node = parse_stmt();

    Token t = lexer.peek(1);
    if (t.token_type == ID     ||
        t.token_type == PRINT ||
        t.token_type == WHILE  ||
        t.token_type == IF     ||
        t.token_type == SWITCH ||
        t.token_type == FOR)
    {
        struct InstructionNode* iterator = node;
        while (iterator->next != nullptr)
        {
            iterator = iterator->next;
        }
        iterator->next = parse_stmt_list();
    }
    return node;
}

struct InstructionNode* Parser::parse_stmt()
{
    struct InstructionNode* node;
    Token t = lexer.peek(1);
    if (t.token_type == ID)          node = parse_assign_stmt();
    else if (t.token_type == PRINT)  node = parse_print_stmt();
    else if (t.token_type == WHILE)  node = parse_while_stmt();
    else if (t.token_type == IF)     node = parse_if_stmt();
    else if (t.token_type == SWITCH) node = parse_switch_stmt();
    else if (t.token_type == FOR)    node = parse_for_stmt();
    else
    {
        debug("SYNTAX ERROR !!!\nInvalid statement on Line %d\n", t.line_no);
        exit(EXIT_FAILURE);
    }
    return node;
}

struct InstructionNode* Parser::parse_print_stmt()
{
    struct InstructionNode* node = new InstructionNode;
    node->type = PRINTIN;
    node->next = nullptr;

    Token t = lexer.peek(1);
    if (t.token_type == PRINT)
    {
        expect(PRINT);

        t = lexer.peek(1);
        if (t.token_type == ID)
        {
            expect(ID);
            node->print_inst.var_index = location(t.lexeme);

            t = lexer.peek(1);
            if (t.token_type == SEMICOLON)
            {
                expect(SEMICOLON);
            }
            else syntax_error(SEMICOLON, t);
        }
        else syntax_error(ID, t);
    }
    else syntax_error(PRINT, t);
    return node;
}

struct InstructionNode* Parser::parse_assign_stmt()
{
    struct InstructionNode* node = new InstructionNode;
    node->type = ASSIGN;
    node->next = nullptr;

    InstructionNode* funCall = nullptr;

    Token t = lexer.peek(1);
    if (t.token_type == ID)
    {
        expect(ID);

        node->assign_inst.left_hand_side_index = location(t.lexeme);

        t = lexer.peek(1);
        if (t.token_type == EQUAL)
        {
            expect(EQUAL);

            t = lexer.peek(1);
            Token t2 = lexer.peek(2);
            if (t2.token_type == SEMICOLON)
            {
                node->assign_inst.op = OPERATOR_NONE;
                node->assign_inst.operand1_index = parse_primary();
            }
            else if (t2.token_type == LPAREN)
            {
                funCall = parse_function_call();
                node->assign_inst.op = OPERATOR_NONE;
                node->assign_inst.operand1_index = localMem.size();
                funCall->next = node;
            }
            else
            {
                parse_expr(node);
            }

            t = lexer.peek(1);
            if (t.token_type == SEMICOLON)
            {
                expect(SEMICOLON);
            }
            else syntax_error(SEMICOLON, t);
        }
        else syntax_error(EQUAL, t);
    }
    else syntax_error(ID, t);

    if (funCall != nullptr) return funCall;
    else return node;
}

int Parser::parse_primary()
{
    Token t = lexer.peek(1);
    if (t.token_type == ID)
    {
        expect(ID);
        return location(t.lexeme);
    }
    else if (t.token_type == NUM)
    {
        expect(NUM);
        addToMem(t, stoi(t.lexeme));
        return location(t.lexeme);
    }
    else
    {
        debug("SYNTAX ERROR !!!\nInvalid primary on Line %d\n", t.line_no);
        exit(EXIT_FAILURE);
    }
}

void Parser::parse_expr(struct InstructionNode* node)
{
    node->assign_inst.operand1_index = parse_primary();
    node->assign_inst.op = parse_op();
    node->assign_inst.operand2_index = parse_primary();
}

struct InstructionNode* Parser::parse_function_call()
{
    struct InstructionNode* node = new InstructionNode;
    node->type = FUNCTION;
    node->next = nullptr;

    Token t = lexer.peek(1);
    if (t.token_type == ID)
    {
        expect(ID);

        bool success = false;
        for (int i = 0; i < functions.size(); i++)
        {
            if (functions[i]->name == t.lexeme)
            {
                node->function_inst.function = functions[i];
                success = true;
                break;
            }
        }

        if (!success)
        {
            debug("Function Doesn't Exist on Line %d", t.line_no);
            exit(EXIT_FAILURE);
        }

        t = lexer.peek(1);
        if (t.token_type == LPAREN)
        {
            expect(LPAREN);

            vector<string> parameters = parse_id_list(false);

            t = lexer.peek(1);
            if (t.token_type == RPAREN)
            {
                expect(RPAREN);
                vector<int>* values = new vector<int>();
                for (int i = 0; i < parameters.size(); i++)
                {
                    values->push_back(location(parameters[i]));
                }
                node->function_inst.operators = values;
            }
            else syntax_error(RPAREN, t);
        }
        else syntax_error(LPAREN, t);
    }
    else syntax_error(ID, t);
    return node;
}

struct InstructionNode* Parser::parse_if_stmt()
{
    struct InstructionNode* node = new InstructionNode;
    node->type = CJMP;
    node->next = nullptr;

    Token t = lexer.peek(1);
    if (t.token_type == IF)
    {
        expect(IF);
        parse_condition(node);
        node->next = parse_body();

        struct InstructionNode* noop = new InstructionNode;
        noop->type = NOOP;
        noop->next = nullptr;

        struct InstructionNode* iterator = node;
        while (iterator->next != nullptr)
        {
            iterator = iterator->next;
        }
        iterator->next = noop;

        node->cjmp_inst.target = noop;
    }
    else syntax_error(IF, t);
    return node;
}

struct InstructionNode* Parser::parse_while_stmt()
{
    struct InstructionNode* node = new InstructionNode;
    node->type = CJMP;
    node->next = nullptr;

    Token t = lexer.peek(1);
    if (t.token_type == WHILE)
    {
        expect(WHILE);
        parse_condition(node);
        node->next = parse_body();

        struct InstructionNode* jmp = new InstructionNode;
        jmp->type = JMP;
        jmp->next = nullptr;
        jmp->jmp_inst.target = node;

        struct InstructionNode* iterator = node;
        while (iterator->next != nullptr)
        {
            iterator = iterator->next;
        }
        iterator->next = jmp;

        struct InstructionNode* noop = new InstructionNode;
        noop->type = NOOP;
        noop->next = nullptr;
        iterator->next->next = noop;

        node->cjmp_inst.target = noop;
    }
    else syntax_error(WHILE, t);
    return node;
}

struct InstructionNode* Parser::parse_for_stmt()
{
    struct InstructionNode* node;

    Token t = lexer.peek(1);
    if (t.token_type == FOR)
    {
        expect(FOR);

        t = lexer.peek(1);
        if (t.token_type == LPAREN)
        {
            expect(LPAREN);
            node = parse_assign_stmt();

            struct InstructionNode* condition = new InstructionNode;
            condition->type = CJMP;
            condition->next = nullptr;
            parse_condition(condition);
            node->next = condition;

            t = lexer.peek(1);
            if (t.token_type == SEMICOLON)
            {
                expect(SEMICOLON);

                struct InstructionNode* assign2 = parse_assign_stmt();

                t = lexer.peek(1);
                if (t.token_type == RPAREN)
                {
                    expect(RPAREN);

                    condition->next = parse_body();

                    struct InstructionNode* jmp = new InstructionNode;
                    jmp->type = JMP;
                    jmp->next = nullptr;
                    jmp->jmp_inst.target = condition;

                    struct InstructionNode* iterator = condition;
                    while (iterator->next != nullptr)
                    {
                        iterator = iterator->next;
                    }
                    iterator->next = assign2;
                    iterator->next->next = jmp;

                    struct InstructionNode* noop = new InstructionNode;
                    noop->type = NOOP;
                    noop->next = nullptr;

                    condition->cjmp_inst.target = noop;
                    iterator->next->next->next = noop;
                }
                else syntax_error(RPAREN, t);
            }
            else syntax_error(SEMICOLON, t);
        }
        else syntax_error(LPAREN, t);
    }
    else syntax_error(FOR, t);
    return node;
}

struct InstructionNode* Parser::parse_switch_stmt()
{
    struct InstructionNode* node;

    Token t = lexer.peek(1);
    if (t.token_type == SWITCH)
    {
        expect(SWITCH);

        t = lexer.peek(1);
        if (t.token_type == ID)
        {
            expect(ID);
            int operand1_index = location(t.lexeme);

            t = lexer.peek(1);
            if (t.token_type == LBRACE)
            {
                expect(LBRACE);

                struct InstructionNode* label = new InstructionNode;
                label->type = NOOP;
                label->next = nullptr;

                node = parse_case_list(operand1_index, label);

                struct InstructionNode* iterator = node;
                while (iterator->next != nullptr)
                {
                    iterator = iterator->next;
                }

                t = lexer.peek(1);
                if (t.token_type == DEFAULT)
                {
                    struct InstructionNode* defaultCase = parse_default_case();

                    iterator->next = defaultCase;

                    while (iterator->next != 0)
                    {
                        iterator = iterator->next;
                    }
                    iterator->next = label;

                    t = lexer.peek(1);
                    if (t.token_type == RBRACE)
                    {
                        expect(RBRACE);
                    }
                    else syntax_error(RBRACE, t);
                }
                else if (t.token_type == RBRACE)
                {
                    expect(RBRACE);
                    iterator->next = label;
                }
                else syntax_error(RBRACE, t);
            }
            else syntax_error(LBRACE, t);
        }
        else syntax_error(ID, t);
    }
    else syntax_error(SWITCH, t);

    return node;
}

struct InstructionNode* Parser::parse_case_list(int operand1_index, struct InstructionNode* label)
{
    struct InstructionNode* node = parse_case(operand1_index);

    struct InstructionNode* iterator = node->cjmp_inst.target;
    while (iterator->next != nullptr)
    {
        iterator = iterator->next;
    }

    struct InstructionNode* jmp = new InstructionNode;
    jmp->type = JMP;
    jmp->next = nullptr;
    jmp->jmp_inst.target = label;
    iterator->next = jmp;

    Token t = lexer.peek(1);
    if (t.token_type == CASE)
    {
        node->next = parse_case_list(operand1_index, label);
    }
    return node;
}

struct InstructionNode* Parser::parse_case(int operand1_index)
{
    struct InstructionNode* node = new InstructionNode;
    node->type = CJMP;
    node->next = nullptr;
    node->cjmp_inst.operand1_index = operand1_index;
    node->cjmp_inst.condition_op = CONDITION_NOTEQUAL;

    Token t = lexer.peek(1);
    if (t.token_type == CASE)
    {
        expect(CASE);

        t = lexer.peek(1);
        if (t.token_type == NUM)
        {
            expect(NUM);
            addToMem(t, stoi(t.lexeme));
            int operand2_index = location(t.lexeme);
            node->cjmp_inst.operand2_index = operand2_index;

            t = lexer.peek(1);
            if (t.token_type == COLON)
            {
                expect(COLON);
                node->cjmp_inst.target = parse_body();
            }
            else syntax_error(COLON, t);
        }
        else syntax_error(NUM, t);
    }
    else syntax_error(CASE, t);
    return node;
}

struct InstructionNode* Parser::parse_default_case()
{
    struct InstructionNode* node;

    Token t = lexer.peek(1);
    if (t.token_type == DEFAULT)
    {
        expect(DEFAULT);

        t = lexer.peek(1);
        if (t.token_type == COLON)
        {
            expect(COLON);
            node = parse_body();
        }
        else syntax_error(COLON, t);
    }
    else syntax_error(DEFAULT, t);
    return node;
}

ArithmeticOperatorType Parser::parse_op()
{
    Token t = lexer.peek(1);
    if (t.token_type == PLUS)
    {
        expect(PLUS);
        return OPERATOR_PLUS;
    }
    else if (t.token_type == MINUS)
    {
        expect(MINUS);
        return OPERATOR_MINUS;
    }
    else if (t.token_type == MULT)
    {
        expect(MULT);
        return OPERATOR_MULT;
    }
    else if (t.token_type == DIV)
    {
        expect(DIV);
        return OPERATOR_DIV;
    }
    else
    {
        debug("SYNTAX ERROR !!!\nInvalid operator on Line %d", t.line_no);
        exit(EXIT_FAILURE);
    }
}

void Parser::parse_condition(struct InstructionNode* node)
{
    node->cjmp_inst.operand1_index = parse_primary();
    node->cjmp_inst.condition_op = parse_relop();
    node->cjmp_inst.operand2_index = parse_primary();
}

ConditionalOperatorType Parser::parse_relop()
{
    Token t = lexer.peek(1);
    if (t.token_type == GREATER)
    {
        expect(GREATER);
        return CONDITION_GREATER;
    }
    else if (t.token_type == LESS)
    {
        expect(LESS);
        return CONDITION_LESS;
    }
    else if (t.token_type == NOTEQUAL)
    {
        expect(NOTEQUAL);
        return CONDITION_NOTEQUAL;
    }
    else
    {
        debug("SYNTAX ERROR !!!\nInvalid relop on Line %d", t.line_no);
        exit(EXIT_FAILURE);
    }
}
//-------------------------------------------------------------------------------------------------

struct InstructionNode * parse_generate_intermediate_representation()
{
    Parser parser;
    return parser.parse_program();
}