#ifndef JTS2GD_GLOBALS
#define JTS2GD_GLOBALS


// built-in
#include <sstream>
#include <string_view>
#include <variant>
#include <cstdint>

//local
#include "utils.hpp"


enum class TokenType
{
    
    // ES5 keywords

    THIS,
    NEW,
    DELETE,
    VOID,
    TYPEOF,
    INSTANCEOF,
    IN,
    VAR,
    LET,
    CONST,
    IF,
    ELSE,
    DO,
    WHILE,
    FOR,
    OF,
    CONTINUE,
    BREAK,
    RETURN,
    WITH,
    SWITH,
    CASE,
    DEFAULT,
    THROW,
    TRY,
    CATCH,
    FINALLY,
    FUNCTION,
    IMPORT,
    CLASS,
    lNULL,

    // ES5 literals

    IDENTIFIER,
    STRING,
    INTEGER,
    FLOAT,
    HEXA,
    OCTAL,
    TRUE,
    FALSE,
    REGEXPS,

    // ES5 multi-char symbols

    LOGICAL_OR,
    LOGICAL_AND,
    PLUS_PLUS,
    MINUS_MINUS,
    EQ_EQ,
    NOT_EQ,
    GREATER_THAN_EQ,
    LESS_THAN_EQ,
    PLUS_EQ,
    MINUS_EQ,
    MUL_EQ,
    DIV_EQ,
    MOD_EQ,
    OR_EQ,
    AND_EQ,
    XOR_EQ,
    LEFT_SHIFT,
    RIGHT_SHIFT,
    LEFT_SHIFT_EQ,
    RIGHT_SHIFT_EQ,
    ZF_RIGHT_SHIFT,
    EQ_EQ_EQ,
    NOT_EQ_EQ,
    ZF_RIGHT_SHIFT_EQ,
    ARROW,

    // ES5 single-char symbols

    MINUS,
    PLUS,
    LEFT_PAREM,
    RIGHT_PAREM,
    LEFT_BRACKET,
    RIGHT_BRACKET,
    EQUAL,
    MUL,
    DIV,
    MOD,
    GREATER_THAN,
    LESS_THAN,
    COMMA,
    DOT,
    TWO_DOTS,
    SEMICOLON,
    TERNARY,
    LOGICAL_NOT,
    LEFT_BRACE,
    RIGHT_BRACE,
    NOT,
    AND,
    XOR,
    OR,

    // Custom
    EXTENDS,
    lEOF
};


//  Array with textual representations of possible 
//  types of tokens to facilitate debugging
// 
//  The order and number of textual representations
//  must be exactly the same as the token type enumerator

const std::string TokenTypeRepr[]
{
        
    // ES5 keywords

    "THIS",
    "NEW",
    "DELETE",
    "VOID",
    "TYPEOF",
    "INSTANCEOF",
    "IN",
    "VAR",
    "LET",
    "CONST",
    "IF",
    "ELSE",
    "DO",
    "WHILE",
    "FOR",
    "OF",
    "CONTINUE",
    "BREAK",
    "RETURN",
    "WITH",
    "SWITH",
    "CASE",
    "DEFAULT",
    "THROW",
    "TRY",
    "CATCH",
    "FINALLY",
    "FUNCTION",
    "IMPORT",
    "CLASS",
    "lNULL",

    // ES5 literals

    "IDENTIFIER",
    "STRING",
    "INTEGER",
    "FLOAT",
    "HEXA",
    "OCTAL",
    "TRUE",
    "FALSE",
    "REGEXPS",

    // ES5 multi-char symbols

    "LOGICAL_OR",
    "LOGICAL_AND",
    "PLUS_PLUS",
    "MINUS_MINUS",
    "EQ_EQ",
    "NOT_EQ",
    "GREATER_THAN_EQ",
    "LESS_THAN_EQ",
    "PLUS_EQ",
    "MINUS_EQ",
    "MUL_EQ",
    "DIV_EQ",
    "MOD_EQ",
    "OR_EQ",
    "AND_EQ",
    "XOR_EQ",
    "LEFT_SHIFT",
    "RIGHT_SHIFT",
    "LEFT_SHIFT_EQ",
    "RIGHT_SHIFT_EQ",
    "ZF_RIGHT_SHIFT",
    "EQ_EQ_EQ",
    "NOT_EQ_EQ",
    "ZF_RIGHT_SHIFT_EQ",
    "ARROW",
    // ES5 single-char symbols

    "MINUS",
    "PLUS",
    "LEFT_PAREM",
    "RIGHT_PAREM",
    "LEFT_BRACKET",
    "RIGHT_BRACKET",
    "EQUAL",
    "MUL",
    "DIV",
    "MOV",
    "GREATER_THAN",
    "LESS_THAN",
    "COMMA",
    "DOT",
    "TWO_DOTS",
    "SEMICOLON",
    "TERNARY",
    "LOGICAL_NOT",
    "LEFT_BRACE",
    "RIGHT_BRACE",
    "NOT",
    "AND",
    "XOR",
    "OR",
    
    // Custom
    "EXTENDS",
    "EOF"
};



struct Token
{

    TokenType type;
    std::string_view lexeme;
    SourceLocation location;

    std::string repr() const
    {
        std::ostringstream token_repr;

        token_repr << this->location << ' ';
        token_repr << TokenTypeRepr[(int)this->type] << ' ';
        token_repr << '"' << this->lexeme << '"';

        return token_repr.str();
    }
};


#endif