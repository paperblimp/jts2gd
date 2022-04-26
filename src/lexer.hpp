#ifndef JTS2GD_LEXER
#define JTS2GD_LEXER


// built-in
#include <cctype>
#include <cstdint>
#include <string>
#include <vector>
#include <unordered_map>
#include <optional>
#include <cstdint>

// local
#include "globals.hpp"
#include "event.hpp"



// Table to identify a keyword and get its type
const std::unordered_map<std::string_view, TokenType> multi_char_tokens
{
    {"this", TokenType::THIS},
    {"new", TokenType::NEW},
    {"delete", TokenType::DELETE},
    {"void", TokenType::VOID},
    {"typeof", TokenType::VOID},
    {"instanceof", TokenType::INSTANCEOF},
    {"in", TokenType::IN},
    {"var", TokenType::VAR},
    {"let", TokenType::LET},
    {"const", TokenType::CONST},
    {"if", TokenType::IF},
    {"else", TokenType::ELSE},
    {"do", TokenType::DO},
    {"while", TokenType::WHILE},
    {"for", TokenType::FOR},
    {"of", TokenType::OF},
    {"continue", TokenType::CONTINUE},
    {"break", TokenType::BREAK},
    {"return", TokenType::RETURN},
    {"with", TokenType::WITH},
    {"switch", TokenType::SWITH},
    {"case", TokenType::CASE},
    {"default", TokenType::DEFAULT},
    {"throw", TokenType::THROW},
    {"try", TokenType::TRY},
    {"catch", TokenType::CATCH},
    {"finally", TokenType::FINALLY},
    {"function", TokenType::FUNCTION},
    {"import", TokenType::IMPORT},
    {"class", TokenType::CLASS},
    {"null", TokenType::lNULL},
    {"extends", TokenType::EXTENDS},

    {"true", TokenType::TRUE},
    {"false", TokenType::FALSE},

    {"||", TokenType::LOGICAL_OR},
    {"&&", TokenType::LOGICAL_AND},
    {"++", TokenType::PLUS_PLUS},
    {"--", TokenType::MINUS_MINUS},
    {"==", TokenType::EQ_EQ},
    {"!=", TokenType::NOT_EQ},
    {">=", TokenType::GREATER_THAN_EQ},
    {"<=", TokenType::LESS_THAN_EQ},
    {"+=", TokenType::PLUS_EQ},
    {"-=", TokenType::MINUS_EQ},
    {"*=", TokenType::MUL},
    {"/=", TokenType::DIV_EQ},
    {"%/", TokenType::MOD_EQ},
    {"|=", TokenType::OR_EQ},
    {"&=", TokenType::AND_EQ},
    {"^=", TokenType::XOR_EQ},
    {"<<", TokenType::LEFT_SHIFT},
    {">>", TokenType::RIGHT_SHIFT},
    {"<<=", TokenType::LEFT_SHIFT_EQ},
    {">>=", TokenType::RIGHT_SHIFT_EQ},
    {">>>", TokenType::ZF_RIGHT_SHIFT},
    {"===", TokenType::EQ_EQ_EQ},
    {"!==", TokenType::NOT_EQ_EQ},
    {">>>=", TokenType::ZF_RIGHT_SHIFT_EQ},
    {"=>", TokenType::ARROW}
};

// single-char tokens
const std::unordered_map<int32_t, TokenType> single_char_tokens
{
    {'-', TokenType::MINUS},
    {'+', TokenType::PLUS},
    {'(', TokenType::LEFT_PAREM},
    {')', TokenType::RIGHT_PAREM},
    {'[', TokenType::LEFT_BRACKET},
    {']', TokenType::RIGHT_BRACKET},
    {'=', TokenType::EQUAL},
    {'*', TokenType::MUL},
    {'/', TokenType::DIV},
    {'%', TokenType::MOD},
    {'>', TokenType::GREATER_THAN},
    {'<', TokenType::LESS_THAN},
    {',', TokenType::COMMA},
    {'.', TokenType::DOT},
    {':', TokenType::TWO_DOTS},
    {';', TokenType::SEMICOLON},
    {'?', TokenType::TERNARY},
    {'!', TokenType::LOGICAL_NOT},
    {'{', TokenType::LEFT_BRACE},
    {'}', TokenType::RIGHT_BRACE},
    {'~', TokenType::NOT},
    {'&', TokenType::AND},
    {'^', TokenType::XOR},
    {'|', TokenType::OR}
};



class Lexer
{

    private:

        const std::string& source;
        const std::string& source_name; // eg, foobar.js
        EventHandler& eh;
        uint32_t idx;
        uint32_t line;
        uint32_t collum;
        const uint32_t source_size;

        std::vector<Token> output;

    public:

        Lexer(const std::string&, EventHandler&, const std::string&);
        std::vector<Token> operator()();

    private:

        void read_token(int32_t);

        [[nodiscard]]
        inline bool at_end() const;

        [[nodiscard]]
        inline int32_t current_char(int = 0) const;

        [[nodiscard]]
        inline bool match(int, int = 0) const;

        inline void advance(int);

        [[nodiscard]]
        inline uint8_t utf8_char_size(uint8_t) const;

        std::optional<Token> lex_string(int32_t);
        std::optional<Token> lex_number(int32_t);
        std::optional<Token> lex_identifier(int32_t);
        std::optional<Token> lex_punctuation(int32_t);

        bool l_ispunct(int);
};


void lex(const std::string&);


#endif