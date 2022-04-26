
// built-in
#include <cctype>
#include <cstddef>
#include <cstdlib>
#include <string>
#include <string_view>
#include <bitset>
#include <cassert>
#include <variant>
#include <cstdint>

// local
#include "lexer.hpp"



Lexer::Lexer(const std::string& source, EventHandler& eh, const std::string& source_name)
: source(source), source_name(source_name), eh(eh), source_size(source.size())
{
    // initialize counters
    this->idx = 0;
    this->line = 1;
    this->collum = 1; // most text editors count columns from 1
}

std::vector<Token> Lexer::operator()()
{
    // ensure that the same lexer is not used more than once
    assert(this->output.size() == 0);

    while (!this->at_end())
    {
        int32_t ch = this->current_char();
        this->read_token(ch);
    }

    // inserting the EOF token
    auto tk = Token
    {
        TokenType::lEOF,
        {this->source.data() + this->idx, 0}, // empty lexeme
        {&this->source_name, this->line, this->collum}
    };
    this->output.push_back(std::move(tk));

    return std::move(this->output);
}


void Lexer::read_token(int32_t ch)
{

    // backup of the counters in case the first analysis fails
    uint32_t start_idx = this->idx;
    uint32_t start_line = this->line;
    uint32_t start_collum = this->collum;

    {
        std::optional<Token> tk;
    
        if (ch == '"' || ch == '\'')
            tk = this->lex_string(ch);
    
        else if (isdigit(ch))
            tk = this->lex_number(ch);
    
        else if (isalpha(ch) || ch == '_')
            tk = this->lex_identifier(ch);

        // multi-char punctuation
        else if (ispunct(ch) && this->l_ispunct(this->current_char(1)))
            tk = this->lex_punctuation(ch);
        
        if (tk.has_value())
        {
            this->output.push_back(std::move(tk.value()));
            return;
        }
    }
    
    // backup of the counters in case the first analysis fails
    this->idx = start_idx;
    this->line = start_line;
    this->collum = start_collum;

    switch (ch)
    {
        case ('\t'):
        case (' '):
        case ('\v'):
        case ('\f'):
        {
            this->advance(ch);
            break;
        }

        case ('\n'):
        {
            this->advance(ch);
            break;
        }

        case ('/'):
        {
            this->advance(ch);

            // comment
            if (!this->at_end() && this->match('/'))
            {
                this->advance('/');

                if (this->match('g') && this->match('d', 1))
                {
                    this->advance('g');
                    this->advance('d');
                }
                else
                {
                    while (ch = this->current_char(), !this->at_end() && ch != '\n')
                        this->advance(ch);
                }
            }
            else if (!this->at_end() && this->match('*'))
            {
                this->advance('*');

                while (ch = this->current_char(), !this->at_end() && !(ch == '*' && this->match('/', 1)))
                {
                    this->advance(ch);
                }
                
                if (!this->at_end())
                {
                    this->advance('*');
                    this->advance('/');
                }
            }

            // div eq '/='
            else if (!this->at_end() && this->match('='))
            {
                this->advance('=');
                auto tk = Token
                {
                    TokenType::DIV_EQ,
                    {this->source.data() + this->idx - 2, 2},
                    {&this->source_name, this->line, this->collum - 2}
                };
                this->output.push_back(std::move(tk));
            }

            // div
            else
            {
                auto tk = Token
                {
                    TokenType::DIV,
                    {this->source.data() + this->idx - 1, 1},
                    {&this->source_name, this->line, this->collum - 1}
                };
                this->output.push_back(std::move(tk));
            }

            break;
        }


        // single-char punctuation
        case ('-'):
        case ('+'):
        case ('('):
        case (')'):
        case ('['):
        case (']'):
        case ('='):
        case ('*'):
        case ('%'):
        case ('>'):
        case ('<'):
        case (','):
        case ('.'):
        case (':'):
        case (';'):
        case ('?'):
        case ('!'):
        case ('{'):
        case ('}'):
        case ('~'):
        case ('&'):
        case ('^'):
        case ('|'):
        {
            auto tk = Token
            {
                single_char_tokens.at(ch),
                {this->source.data() + this->idx, 1},
                {&this->source_name, this->line, this->collum}
            };
            this->output.push_back(std::move(tk));

            this->advance(ch);
            break;
        }

        default:
        {
            this->eh.add_error("invalid char", {&this->source_name, this->line, this->collum});
            this->advance(ch);
        }
    }
}

// checks if the end of the file has already been reached
bool Lexer::at_end() const
{
    return this->idx >= this->source_size;
}

// identifies and returns utf-8 catacter
int32_t Lexer::current_char(int offset) const
{   
    const char* char_ptr = this->source.data() + this->idx + offset;
    char head = *char_ptr;
    uint8_t char_size = this->utf8_char_size(head);

    if (char_size == 1)
        return head;
    
    int32_t out_char = *(int32_t*)char_ptr;
    out_char >>= (4 - char_size) * 8;
    
    return out_char;
}


bool Lexer::match(int ch, int offset) const
{
    return this->current_char(offset) == ch;
}

void Lexer::advance(int ch)
{
    if (ch == '\n')
    {
        ++this->line;
        ++this->idx;
        this->collum = 1;
    }
    else
    {
        this->idx += this->utf8_char_size(ch);
        ++this->collum;
    }
}

// parse utf-8 char size
uint8_t Lexer::utf8_char_size(uint8_t ch) const
{
    if (ch < 0b10000000)
        return 1;

    else if (ch < 0b11100000)
        return 2;
    
    else if (ch < 0b11110000)
        return 3;

    else
        return 4;
}


std::optional<Token> Lexer::lex_string(int32_t ch)
{
    char string_start = ch;
    SourceLocation location {&this->source_name, this->line, this->collum};
    uint32_t start_idx = this->idx;

    while (true)
    {
        this->advance(ch);
        ch = this->current_char();

        // stop if it is at the end of the file or if it finds an unescaped finisher
        if (this->at_end() || (ch == string_start && !this->match('\\', -1)))
            break;
    }
    this->advance(ch);



    char* start = (char*)this->source.data() + start_idx;
    uint32_t size = this->idx - start_idx;

    auto tk = Token
    {
        TokenType::STRING,
        {start, size},
        location,
    };
    return {std::move(tk)};
}


std::optional<Token> Lexer::lex_number(int32_t ch)
{

    SourceLocation location {&this->source_name, this->line, this->collum};
    uint32_t start_idx = this->idx;
    char* start = (char*)this->source.data() + start_idx;
    char* end = nullptr;
    
    TokenType type;
    size_t size;

    if (ch != '0') // decimal, integer & float
    {
        strtoll(start, &end, 10);
        size = end - start;
        
        this->idx += size;
        this->collum += size;
        
        if (!this->at_end() && this->match('.'))
        {
            strtod(start + size, &end);
            size = end - (start + size);
            
            this->idx += size;
            this->collum += size;
            
            type = TokenType::FLOAT;
        }
        else
            type = TokenType::INTEGER;
    }
    else if (this->match('x', 1) || this->match('X', 1)) // hexa
    {
        strtoll(start, &end, 16);
        size = end - start;
        
        this->idx += size;
        this->collum += size;
        
        type = TokenType::HEXA;
    }
    else // octal
    {
        strtoll(start, &end, 8);
        size = end - start;
        
        this->idx += size;
        this->collum += size;
        
        type = TokenType::OCTAL;
    }

    return 
    {
        Token
        {
            type,
            {start, this->idx - start_idx},
            location
        }
    };
}


std::optional<Token> Lexer::lex_identifier(int32_t ch)
{
    uint32_t start_idx = this->idx;
    SourceLocation location {&this->source_name, this->line, this->collum};

    this->advance(ch);
    while (ch = this->current_char(), !this->at_end() && (isalnum(ch) || ch == '_'))
        this->advance(ch);


    const std::string_view lexeme {this->source.data() + start_idx, this->idx - start_idx};
    TokenType type = TokenType::IDENTIFIER;

    // check if it is a keyword
    const auto type_ptr = multi_char_tokens.find(lexeme);
    if (type_ptr != multi_char_tokens.end())
        type = type_ptr->second;

    return Token
    {
        type,
        lexeme,
        location
    };
}


// custom punct function for multi-char punctuation
bool Lexer::l_ispunct(int ch)
{
    switch (ch)
    {
        case ('|'):
        case ('&'):
        case ('+'):
        case ('-'):
        case ('='):
        case ('/'):
        case ('<'):
        case ('>'):
        {
            return true;
        }
    }

    return false;
}

std::optional<Token> Lexer::lex_punctuation(int32_t ch)
{
    uint32_t start_idx = this->idx;
    SourceLocation location {&this->source_name, this->line, this->collum};

    std::optional<Token> last_tk {};
    uint32_t last_idx = this->idx;
    uint32_t last_line = this->line;
    uint32_t last_collum = this->collum;
    
    while (!this->at_end())
    {

        // checks if the current string forms a valid token
        const std::string_view lexeme {this->source.data() + start_idx, this->idx - start_idx};
        const auto type_ptr = multi_char_tokens.find(lexeme);
        if (type_ptr != multi_char_tokens.end())
        {
            last_tk = Token
            {
                type_ptr->second,
                lexeme,
                location
            };
            last_idx = this->idx;
            last_line = this->line;
            last_collum = this->collum;
        }

        // stop if the size exceeds the size of the largest punctuation token
        if (lexeme.size() >= 4)
            break;

        this->advance(ch);
        ch = this->current_char();
    }

    this->idx = last_idx;
    this->line = last_line;
    this->collum = last_collum;
    return last_tk;
}