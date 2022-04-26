#ifndef JTS2GD_JS_PARSER
#define JTS2GD_JS_PARSER


// built-in
#include <cstddef>
#include <unordered_map>
#include <unordered_set>
#include <exception>
#include <cstdint>
#include <memory_resource>
#include <stack>

// local
#include "globals.hpp"
#include "event.hpp"
#include "tree.hpp"






class JSParser
{

    // table to choose function from the current token
    const std::unordered_map<TokenType, Statement*(JSParser::*)(void)> statement_first
    {
        {TokenType::VAR,        &JSParser::parse_var_decl_stmt},
        {TokenType::LET,        &JSParser::parse_var_decl_stmt},
        {TokenType::CONST,      &JSParser::parse_var_decl_stmt},
        {TokenType::SEMICOLON,  &JSParser::parse_empty_stmt},
        {TokenType::IF,         &JSParser::parse_if_stmt},
        {TokenType::FOR,        &JSParser::parse_for_stmt},
        {TokenType::WHILE,      &JSParser::parse_while_stmt},
        {TokenType::CONTINUE,   &JSParser::parse_continue_stmt},
        {TokenType::BREAK,      &JSParser::parse_break_stmt},
        {TokenType::IMPORT,     &JSParser::parse_import_stmt},
        {TokenType::RETURN,     &JSParser::parse_return_stmt},
        {TokenType::WITH,       &JSParser::parse_with_stmt},
        {TokenType::SWITH,      &JSParser::parse_switch_case_stmt},
        {TokenType::THROW,      &JSParser::parse_throw_stmt},
        {TokenType::TRY,        &JSParser::parse_try_stmt},
        {TokenType::FUNCTION,   &JSParser::parse_function},
        {TokenType::LEFT_BRACE, &JSParser::parse_block},
        {TokenType::EXTENDS,    &JSParser::parse_extends},
        {TokenType::CLASS,    &JSParser::parse_class_extends}
    };

    const std::unordered_set<TokenType> expr_first
    {
        TokenType::IDENTIFIER,
        TokenType::LEFT_PAREM,
        TokenType::STRING,
        TokenType::INTEGER,
        TokenType::FLOAT,
        TokenType::HEXA,
        TokenType::OCTAL,
        TokenType::DELETE,
        TokenType::VOID,
        TokenType::TYPEOF,
        TokenType::PLUS_PLUS,
        TokenType::MINUS_MINUS,
        TokenType::PLUS,
        TokenType::MINUS,
        TokenType::NOT,
        TokenType::LOGICAL_NOT,
        TokenType::LEFT_BRACKET
    };

    const std::unordered_set<TokenType> literal_member_first
    {
        TokenType::INTEGER,
        TokenType::HEXA,
        TokenType::FLOAT,
        TokenType::OCTAL,
        TokenType::STRING,
        TokenType::TRUE,
        TokenType::FALSE,
        TokenType::lNULL
    };

    const std::unordered_set<TokenType> unary_operators
    {
        TokenType::DELETE,
        TokenType::VOID,
        TokenType::TYPEOF,
        TokenType::PLUS_PLUS,
        TokenType::MINUS_MINUS,
        TokenType::PLUS,
        TokenType::MINUS,
        TokenType::NOT,
        TokenType::LOGICAL_NOT
    };

    const std::unordered_set<TokenType> unsuported_unary_operators
    {
        TokenType::DELETE,
        TokenType::VOID,
        TokenType::TYPEOF,
        TokenType::PLUS_PLUS,
        TokenType::MINUS_MINUS,
    };

    const std::unordered_set<TokenType> binary_operators
    {
        TokenType::MUL,
        TokenType::DIV,
        TokenType::MOD,
        TokenType::PLUS,
        TokenType::MINUS,
        TokenType::LEFT_SHIFT,
        TokenType::RIGHT_SHIFT,
        TokenType::ZF_RIGHT_SHIFT,
        TokenType::LESS_THAN,
        TokenType::GREATER_THAN,
        TokenType::LESS_THAN_EQ,
        TokenType::GREATER_THAN_EQ,
        TokenType::INSTANCEOF,
        TokenType::IN,
        TokenType::EQ_EQ,
        TokenType::NOT_EQ,
        TokenType::EQ_EQ_EQ,
        TokenType::NOT_EQ_EQ,
        TokenType::AND,
        TokenType::XOR,
        TokenType::OR,
        TokenType::LOGICAL_AND,
        TokenType::LOGICAL_OR,
    };

    const std::unordered_set<TokenType> unsuported_binary_operators
    {
        TokenType::ZF_RIGHT_SHIFT,
        TokenType::ZF_RIGHT_SHIFT_EQ,
    };

    const std::unordered_set<TokenType> relational_operators
    {
        TokenType::LESS_THAN,
        TokenType::GREATER_THAN,
        TokenType::LESS_THAN_EQ,
        TokenType::GREATER_THAN_EQ,
        TokenType::INSTANCEOF,
        TokenType::IN
    };
    
    const std::unordered_set<TokenType> equality_operators
    {
        TokenType::EQ_EQ,
        TokenType::NOT_EQ,
        TokenType::EQ_EQ_EQ,
        TokenType::NOT_EQ_EQ
    };

    const std::unordered_set<TokenType> assignment_operators
    {
        TokenType::EQUAL,
        TokenType::MUL_EQ,
        TokenType::DIV_EQ,
        TokenType::MOD_EQ,
        TokenType::PLUS_EQ,
        TokenType::MINUS_EQ,
        TokenType::LEFT_SHIFT_EQ,
        TokenType::RIGHT_SHIFT_EQ,
        TokenType::AND_EQ,
        TokenType::XOR_EQ,
        TokenType::OR_EQ
    };

    struct SyntaxError: public std::exception
    {
        const char* what() const noexcept override
        {
            return "parser syntax error";
        }
    };


    private:

        const std::vector<Token>& source;
        EventHandler* eh;

        uint32_t idx;
        const size_t source_size;

        uint32_t fexpr_id = 0;
        std::vector<FunctionExpression*> function_expressions;

    public:

        JSParser(const std::vector<Token>&, EventHandler&);
        Program* operator()();

    private:


        Statement* parse_stmt();

        Statement* parse_var_decl_stmt();
        Statement* parse_empty_stmt();
        Statement* parse_labeled_stmt();
        Statement* parse_if_stmt();
        Statement* parse_for_stmt();
        Statement* parse_while_stmt();
        Statement* parse_continue_stmt();
        Statement* parse_break_stmt();
        Statement* parse_import_stmt();
        Statement* parse_return_stmt();
        Statement* parse_with_stmt();
        Statement* parse_switch_case_stmt();
        Statement* parse_block();
        Statement* parse_throw_stmt();
        Statement* parse_try_stmt();
        Statement* parse_function();
        Statement* parse_expression_stmt();
        Statement* parse_extends();
        Statement* parse_class_extends();
        
        Expression* parse_expression();
        Expression* parse_assignment();
        Expression* parse_conditional_expr();
        Expression* parse_logical_or();
        Expression* parse_logical_and();
        Expression* parse_or();
        Expression* parse_xor();
        Expression* parse_and();
        Expression* parse_equality();
        Expression* parse_relational();
        Expression* parse_shift();
        Expression* parse_additive();
        Expression* parse_multiplicative();
        Expression* parse_unary();
        Expression* parse_postfix();
        Expression* parse_member_expr();

        VarDecl* parse_var_decl();
        Case* parse_case();
        const Token* parse_function_expression(bool = false);

        // ####################################################
        // #                                                  #
        // #                     UTILS                        #
        // #                                                  #
        // ####################################################

        [[nodiscard]]
        inline bool at_end(int = 0) const;

        [[nodiscard]]
        inline const Token& current_tok(int = 0) const;

        [[nodiscard]]
        inline bool match(TokenType, int = 0) const;

        inline void advance(int = 1);

        void parser_rewind();

        bool expect(TokenType, bool = true, const std::string& = "");
        bool consume(TokenType, bool = true, const std::string& = "");
        void unexpected(const Token&, const std::string& = "");
        void optional_semicolon();

        [[nodiscard]]
        bool separated_by_newline(const Token&, const Token&) const;

};


#endif