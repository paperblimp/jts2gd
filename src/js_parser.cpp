
// built-in
#include <memory>
#include <memory_resource>
#include <unordered_set>
#include <typeinfo>
#include <cstdint>

// local
#include "globals.hpp"
#include "js_parser.hpp"
#include "tree.hpp"
#include "tree_releaser.hpp"



//
//  Javascript ES5 Parser
//  
//
//  Some of the parser's functions keep objects allocated 
//  in the heap inside 'unique_ptr's to ensure that in case 
//  of an exception when calling another parser function, resources are not lost.
//




// specialization of 'unique_ptr' which uses a custom destructor
template <typename T>
using unique_ptr = std::unique_ptr<T, UniqueReleaser<T>>;



JSParser::JSParser(const std::vector<Token>& s, EventHandler& eh)
: source(s), eh(&eh), source_size(source.size())
{
    // initialize counter
    this->idx = 0;
}


Program* JSParser::operator()()
{
    auto prog = new Program{};

    while (!this->at_end())
    {
        try
        {
            prog->stmts.push_back(this->parse_stmt());
        }
        catch (const JSParser::SyntaxError&)
        {
            this->parser_rewind();
        }
    }

    prog->function_expressions = std::move(this->function_expressions);
    return prog;
}


// ####################################################
// #                                                  #
// #                   Statements                     #
// #                                                  #
// ####################################################



Statement* JSParser::parse_stmt()
{
    auto& tk = this->current_tok();

    // statement
    const auto func_ptr = this->statement_first.find(tk.type);
    if (func_ptr != this->statement_first.end())
        return (this->*func_ptr->second)();

    // label
    if (this->match(TokenType::TWO_DOTS, 1))
        return this->parse_labeled_stmt();

    // expression
    if (this->expr_first.find(tk.type) != this->expr_first.end())
        return this->parse_expression_stmt();


    this->eh->add_error("unexpected token '" + std::string(tk.lexeme) + "'", tk.location);
    throw JSParser::SyntaxError{};
}



Statement* JSParser::parse_var_decl_stmt()
{
    auto expr = unique_ptr<VarDeclStmt>(new VarDeclStmt{});
    
    switch (this->current_tok().type)
    {
        case (TokenType::VAR):
        case (TokenType::LET):
        {
            expr->type = VarDeclStmtType::VAR;
            break;
        }
        case (TokenType::CONST):
        {
            expr->type = VarDeclStmtType::CONST;
            break;
        }

        default:
        {
            panic("compiler internal error");
        }
    }
    this->advance();

    // consumes at least one variable
    do expr->decls.push_back(this->parse_var_decl());
    while (this->consume(TokenType::COMMA, false));
    this->optional_semicolon();

    return expr.release();
}

Statement* JSParser::parse_empty_stmt()
{
    this->advance();
    return new EmptyStmt{};
}

Statement* JSParser::parse_labeled_stmt()
{
    this->eh->add_error("GDscript does not support labels", this->current_tok().location);
    throw JSParser::SyntaxError{};
}

Statement* JSParser::parse_if_stmt()
{
    auto expr = unique_ptr<IfStmt>(new IfStmt);
    
    this->consume(TokenType::IF);
    this->consume(TokenType::LEFT_PAREM);
    expr->cond = this->parse_expression();
    this->consume(TokenType::RIGHT_PAREM);
    expr->body = this->parse_stmt();

    // optional else part
    if (this->consume(TokenType::ELSE, false))
        expr->else_block = this->parse_stmt();

    return expr.release();
}

Statement* JSParser::parse_for_stmt()
{
    auto expr = unique_ptr<ForStmt>(new ForStmt{});
    
    this->consume(TokenType::FOR);
    this->consume(TokenType::LEFT_PAREM);

    // for normal

    if (!this->consume(TokenType::SEMICOLON, false))
    {

        if (this->match(TokenType::VAR) || this->match(TokenType::LET) || this->match(TokenType::CONST))
        {
            auto vdecl_stmt = unique_ptr<VarDeclStmt>(new VarDeclStmt{});
    
            switch (this->current_tok().type)
            {
                case (TokenType::VAR):
                case (TokenType::LET):
                {
                    vdecl_stmt->type = VarDeclStmtType::VAR;
                    break;
                }
                case (TokenType::CONST):
                {
                    vdecl_stmt->type = VarDeclStmtType::CONST;
                    break;
                }

                default:
                {
                    panic("compiler internal error");
                }
            }
            this->advance();

            // consumes at least one variable
            do vdecl_stmt->decls.push_back(this->parse_var_decl());
            while (this->consume(TokenType::COMMA, false));

            expr->init_expr = vdecl_stmt.release();




            if (this->match(TokenType::OF))
            {
                if (static_cast<VarDeclStmt*>(expr->init_expr)->decls.size() > 1)
                {
                    this->eh->add_error("GDscript only allows a single variable to be declared in a for of loop", this->current_tok().location);
                    throw JSParser::SyntaxError{};
                }

                if (static_cast<VarDeclStmt*>(expr->init_expr)->type == VarDeclStmtType::CONST)
                    this->eh->add_warning("constancy cannot be ensured in for loops", this->current_tok().location);

                for (auto vdecl: static_cast<VarDeclStmt*>(expr->init_expr)->decls)
                {
                    
                    if (vdecl->type != nullptr)
                    {
                        this->eh->add_error("GDscript does not support static typing on variables declared in for of loops", vdecl->type->location);
                        throw JSParser::SyntaxError{};
                    }
                    if (vdecl->init_value != nullptr)
                    {
                        this->eh->add_error("GDscript does not support initialization of variables in for of loops", vdecl->var->location);
                        throw JSParser::SyntaxError{};
                    }
                }

                expr->init_var_decl = static_cast<VarDeclStmt*>(expr->init_expr)->decls[0]->var;
                Releaser{}.visit(expr->init_expr);
                
                goto FOR_OF;
            }
        }
        else
        {
            expr->init_expr = this->parse_expression_stmt();
            this->consume(TokenType::OF, false);
        }

        if (!this->match(TokenType::SEMICOLON, -1))
            this->consume(TokenType::SEMICOLON);
    }

    if (!this->consume(TokenType::SEMICOLON, false))
    {
        expr->cond = this->parse_expression();
        this->consume(TokenType::SEMICOLON);
    }

    if (!this->consume(TokenType::RIGHT_PAREM, false))
    {
        expr->post = this->parse_expression();
        this->consume(TokenType::RIGHT_PAREM);
    }
    
    goto BLOCK;

    // for in

    FOR_OF:;

    expr->for_of = true;
    this->consume(TokenType::OF);
    expr->of_expr = this->parse_expression();
    this->consume(TokenType::RIGHT_PAREM);

    BLOCK:;

    // loop body
    expr->block = this->parse_stmt();

    return expr.release();
}

Statement* JSParser::parse_while_stmt()
{
    auto expr = unique_ptr<WhileStmt>(new WhileStmt);
    
    this->consume(TokenType::WHILE);
    this->consume(TokenType::LEFT_PAREM);
    expr->cond = this->parse_expression();
    this->consume(TokenType::RIGHT_PAREM);

    // body
    expr->body = this->parse_stmt();

    return expr.release();
}

Statement* JSParser::parse_continue_stmt()
{
    this->consume(TokenType::CONTINUE);

    // if there is no semicolon after the keyword
    if (!this->consume(TokenType::SEMICOLON, false))
    {
        // "insert" the semicolon if the next token is a '}' or EOF
        if (this->match(TokenType::RIGHT_BRACE) || this->match(TokenType::lEOF))
            return new ContinueStmt{};
        
        // error if the next token is on the same line as the keyword
        else if (!this->separated_by_newline(this->current_tok(-1), this->current_tok()))
        {
            this->eh->add_error("GDscript does not support labels", this->current_tok().location);
            throw JSParser::SyntaxError{};
        }
    }

    return new ContinueStmt{};
}

Statement* JSParser::parse_break_stmt()
{
    this->consume(TokenType::BREAK);

    // if there is no semicolon after the keyword
    if (!this->consume(TokenType::SEMICOLON, false))
    {
        // "insert" the semicolon if the next token is a '}' or EOF
        if (this->match(TokenType::RIGHT_BRACE) || this->match(TokenType::lEOF))
            return new BreakStmt{};
        
        // error if the next token is on the same line as the keyword
        else if (!this->separated_by_newline(this->current_tok(-1), this->current_tok()))
        {
            this->eh->add_error("GDscript does not support labels", this->current_tok().location);
            throw JSParser::SyntaxError{};
        }
    }

    return new BreakStmt{};
}

Statement* JSParser::parse_import_stmt()
{
    this->eh->add_error("GDscript does not support import statement", this->current_tok().location);
    throw JSParser::SyntaxError{};
}

Statement* JSParser::parse_return_stmt()
{
    this->consume(TokenType::RETURN);

    // if there is no semicolon after the keyword
    if (!this->consume(TokenType::SEMICOLON, false))
    {
        // "insert" the semicolon if the next token is a '}' or EOF
        if (this->match(TokenType::RIGHT_BRACE) || this->match(TokenType::lEOF))
            return new ReturnStmt{};
        
        // parse the expression to be returned
        else if (!this->separated_by_newline(this->current_tok(-1), this->current_tok()))
        {
            auto expr = unique_ptr<ReturnStmt>(new ReturnStmt{});
            expr->value = this->parse_expression();

            // tries to consume the optional semicolon
            this->consume(TokenType::SEMICOLON, false);

            return expr.release();
        }
    }

    return new ReturnStmt{};
}

Statement* JSParser::parse_with_stmt()
{
    this->eh->add_error("GDscript does not support with statement", this->current_tok().location);
    throw JSParser::SyntaxError{};
}


Case* JSParser::parse_case()
{
    if (this->match(TokenType::CASE))
    {
        
        auto _case = unique_ptr<Case>(new Case{});

        // consumes sequence of cases clauses
        do
        {
            this->consume(TokenType::CASE);
            auto comp_val_location = this->current_tok().location; // save expression start location
            auto comp_val = unique_ptr<Expression>(this->parse_expression());
            
            //
            // expressions of switch clauses can be composed only 
            // of identifiers or access of identifiers with periods
            //

            bool valid_expr = true;
            
            // guarantees that it is a primary expression
            if (typeid(*comp_val) != typeid(PrimaryExpr))
            {
                valid_expr = false;
            }
            else
            {
                // ensures that the primary expression is composed only of members access
                PrimaryExpr* pexpr = static_cast<PrimaryExpr*>(comp_val.get());
                for (auto part: pexpr->parts)
                {
                    if (typeid(*part) != typeid(MemberAccessPart))
                    {
                        valid_expr = false;
                        break;
                    }
                }
            }

            if (!valid_expr)
            {
                this->eh->add_error("invalid case expression, only member access (\"A.B\") is allowed in GDScript", comp_val_location);
                throw JSParser::SyntaxError{};
            }

            _case->comp_values.push_back(comp_val.release());
            this->consume(TokenType::TWO_DOTS);

        }
        while(this->match(TokenType::CASE));

        // parse clause body
        do _case->stmts.push_back(this->parse_stmt());
        while (!this->match(TokenType::CASE) && !this->match(TokenType::DEFAULT) && !this->match(TokenType::RIGHT_BRACE));

        return _case.release();
    }

    // consume "default" clause
    else if (this->match(TokenType::DEFAULT))
    {
        this->consume(TokenType::DEFAULT);
        this->consume(TokenType::TWO_DOTS);
        
        auto _case = unique_ptr<Case>(new Case{});

        // parse clause body
        do _case->stmts.push_back(this->parse_stmt());
        while (!this->match(TokenType::CASE) && !this->match(TokenType::RIGHT_BRACE));

        return _case.release();
    }
    else
    {
        auto& tk = this->current_tok();
        this->eh->add_error("unexpected token '" + std::string(tk.lexeme) + "'", tk.location);
        throw JSParser::SyntaxError{};
    }
}


Statement* JSParser::parse_switch_case_stmt()
{
    auto expr = unique_ptr<SwitchCaseStmt>(new SwitchCaseStmt);
    
    this->consume(TokenType::SWITH);
    this->consume(TokenType::LEFT_PAREM);
    expr->match_value = this->parse_expression();
    this->consume(TokenType::RIGHT_PAREM);
    
    // body
    this->consume(TokenType::LEFT_BRACE);
    while (!this->consume(TokenType::RIGHT_BRACE, false))
        expr->case_clauses.push_back(this->parse_case());

    return expr.release();
}

Statement* JSParser::parse_throw_stmt()
{
    this->eh->add_error("GDscript does not support exceptions", this->current_tok().location);
    throw JSParser::SyntaxError{};
}

Statement* JSParser::parse_try_stmt()
{
    this->eh->add_error("GDscript does not support exceptions", this->current_tok().location);
    throw JSParser::SyntaxError{};
}

Statement* JSParser::parse_function()
{

    auto expr = unique_ptr<FunctionStmt>(new FunctionStmt{});

    this->consume(TokenType::FUNCTION);
    this->consume(TokenType::IDENTIFIER);
    expr->name = &this->current_tok(-1);

    this->consume(TokenType::LEFT_PAREM);
    if (!this->match(TokenType::RIGHT_PAREM))
    {
        // parses function parameters
        do expr->params.push_back(this->parse_var_decl());
        while (this->consume(TokenType::COMMA, false));
    }
    this->consume(TokenType::RIGHT_PAREM);

    // parses the type if it exists
    if (this->consume(TokenType::TWO_DOTS, false))
    {
        this->expect(TokenType::IDENTIFIER);

        // null type if "any" or "Any"
        if (this->current_tok().lexeme != "any" && this->current_tok().lexeme != "Any")
            expr->type = &this->current_tok();
        
        this->advance();
    }

    // parse function body
    this->consume(TokenType::LEFT_BRACE);
    while (!this->match(TokenType::RIGHT_BRACE))
        expr->func_body.push_back(this->parse_stmt());
    this->consume(TokenType::RIGHT_BRACE);

    return expr.release();
}

VarDecl* JSParser::parse_var_decl()
{
    auto decl = unique_ptr<VarDecl>(new VarDecl{});

    this->consume(TokenType::IDENTIFIER);
    decl->var = &this->current_tok(-1);

    // parses the type if it exists
    if (this->consume(TokenType::TWO_DOTS, false))
    {
        this->expect(TokenType::IDENTIFIER);

        // null type if "any" or "Any"
        if (this->current_tok().lexeme != "any" && this->current_tok().lexeme != "Any")
            decl->type = &this->current_tok();

        this->advance();
    }

    // parse initialization value if it exists
    if (this->consume(TokenType::EQUAL, false))
        decl->init_value = this->parse_assignment();

    return decl.release();
}


Statement* JSParser::parse_extends()
{
    auto expr = unique_ptr<ExtendsStmt>(new ExtendsStmt{});

    this->consume(TokenType::EXTENDS);

    this->consume(TokenType::IDENTIFIER);
    expr->name = &this->current_tok(-1);
    this->optional_semicolon();

    return expr.release();
}

Statement* JSParser::parse_class_extends()
{
    auto cext = unique_ptr<ClassExtendsStmt>(new ClassExtendsStmt{});

    this->consume(TokenType::CLASS);

    this->consume(TokenType::IDENTIFIER);
    cext->class_name = &this->current_tok(-1);

    this->consume(TokenType::EXTENDS);

    this->consume(TokenType::IDENTIFIER);
    cext->extended = &this->current_tok(-1);

    // parse class body
    this->consume(TokenType::LEFT_BRACE);
    while (!this->match(TokenType::RIGHT_BRACE))
        cext->body.push_back(this->parse_stmt());
    this->consume(TokenType::RIGHT_BRACE);

    return cext.release();

}


Statement* JSParser::parse_expression_stmt()
{   
    auto ptr = unique_ptr<ExpressionStmt>(new ExpressionStmt{});
    ptr->expr = this->parse_expression();
    this->optional_semicolon();

    return ptr.release();
}


Statement* JSParser::parse_block()
{
    auto block = unique_ptr<Block>(new Block{});

    this->consume(TokenType::LEFT_BRACE);

    // consume statements until you find the end of the block
    while (!this->consume(TokenType::RIGHT_BRACE, false))
        block->stmts.push_back(this->parse_stmt());

    return block.release();
}


// ####################################################
// #                                                  #
// #                  Expressions                     #
// #                                                  #
// ####################################################




Expression* JSParser::parse_expression()
{
    auto expr = unique_ptr<Expression>(this->parse_assignment());

    if (this->match(TokenType::COMMA))
    {
        this->eh->add_error("comma operator does not exist in GDscript", this->current_tok().location);
        throw JSParser::SyntaxError{};
    }

    return expr.release();
}

Expression* JSParser::parse_assignment()
{
    auto expr = unique_ptr<Expression>(this->parse_conditional_expr());

    const Token* tk = &this->current_tok();
    if (this->assignment_operators.find(tk->type) != this->assignment_operators.end())
    {
        this->advance();
        BinaryExpr* new_expr = new BinaryExpr{};
        new_expr->oprt = tk;
        new_expr->left = expr.release();
        expr.reset(new_expr);
        new_expr->right = this->parse_conditional_expr();

        tk = &this->current_tok();
        if (this->assignment_operators.find(tk->type) != this->assignment_operators.end())
        {
            this->eh->add_error("assignment returns nothing in GDScript", tk->location);
            throw JSParser::SyntaxError{};
        }
    }

    if (this->match(TokenType::ZF_RIGHT_SHIFT_EQ))
    {
        this->eh->add_error("Operator zero fill right shift equal(>>>=) does not exist in GDScript", this->current_tok().location);
        throw JSParser::SyntaxError{};
    }

    return expr.release();
}

Expression* JSParser::parse_conditional_expr()
{
    auto expr = unique_ptr<Expression>(this->parse_logical_or());

    if (this->match(TokenType::TERNARY))
    {

        this->advance();
        auto new_expr = new ConditionalExpr{};
        new_expr->cond = expr.release();
        expr.reset(new_expr);

        new_expr->expr1 = this->parse_expression();

        this->consume(TokenType::TWO_DOTS);

        new_expr->expr2 = this->parse_expression();
    }
    
    return expr.release();
}

Expression* JSParser::parse_logical_or()
{
    auto expr = unique_ptr<Expression>(this->parse_logical_and());

    while (this->match(TokenType::LOGICAL_OR))
    {
        this->advance();
        BinaryExpr* new_expr = new BinaryExpr{};
        new_expr->oprt = &this->current_tok(-1);
        new_expr->left = expr.release();
        expr.reset(new_expr);
        new_expr->right = this->parse_logical_and();
    }

    return expr.release();
}

Expression* JSParser::parse_logical_and()
{
    auto expr = unique_ptr<Expression>(this->parse_or());

    while (this->match(TokenType::LOGICAL_AND))
    {
        this->advance();
        BinaryExpr* new_expr = new BinaryExpr{};
        new_expr->oprt = &this->current_tok(-1);
        new_expr->left = expr.release();
        expr.reset(new_expr);
        new_expr->right = this->parse_or();
    }

    return expr.release();
}

Expression* JSParser::parse_or()
{
    auto expr = unique_ptr<Expression>(this->parse_xor());

    while (this->match(TokenType::OR))
    {
        this->advance();
        BinaryExpr* new_expr = new BinaryExpr{};
        new_expr->oprt = &this->current_tok(-1);
        new_expr->left = expr.release();
        expr.reset(new_expr);
        new_expr->right = this->parse_xor();
    }

    return expr.release();
}

Expression* JSParser::parse_xor()
{
    auto expr = unique_ptr<Expression>(this->parse_and());

    while (this->match(TokenType::XOR))
    {
        this->advance();
        BinaryExpr* new_expr = new BinaryExpr{};
        new_expr->oprt = &this->current_tok(-1);
        new_expr->left = expr.release();
        expr.reset(new_expr);
        new_expr->right = this->parse_and();
    }

    return expr.release();
}

Expression* JSParser::parse_and()
{
    auto expr = unique_ptr<Expression>(this->parse_equality());

    while (this->match(TokenType::AND))
    {
        this->advance();
        BinaryExpr* new_expr = new BinaryExpr{};
        new_expr->oprt = &this->current_tok(-1);
        new_expr->left = expr.release();
        expr.reset(new_expr);
        new_expr->right = this->parse_equality();
    }

    return expr.release();
}

Expression* JSParser::parse_equality()
{
    auto expr = unique_ptr<Expression>(this->parse_relational());

    const Token* tk = &this->current_tok();
    while (this->equality_operators.find(tk->type) != this->equality_operators.end())
    {
        this->advance();
        BinaryExpr* new_expr = new BinaryExpr{};
        new_expr->oprt = tk;
        new_expr->left = expr.release();
        expr.reset(new_expr);
        new_expr->right = this->parse_relational();
        tk = &this->current_tok();
    }

    return expr.release();
}

Expression* JSParser::parse_relational()
{
    auto expr = unique_ptr<Expression>(this->parse_shift());

    const Token* tk = &this->current_tok();
    while (this->relational_operators.find(tk->type) != this->relational_operators.end())
    {
        this->advance();
        BinaryExpr* new_expr = new BinaryExpr{};
        new_expr->oprt = tk;
        new_expr->left = expr.release();
        expr.reset(new_expr);
        new_expr->right = this->parse_shift();

        tk = &this->current_tok();
    }
    return expr.release();
}

Expression* JSParser::parse_shift()
{
    auto expr = unique_ptr<Expression>(this->parse_additive());

    while (this->match(TokenType::LEFT_SHIFT) || this->match(TokenType::RIGHT_SHIFT))
    {
        this->advance();
        BinaryExpr* new_expr = new BinaryExpr{};
        new_expr->oprt = &this->current_tok(-1);
        new_expr->left = expr.release();
        expr.reset(new_expr);
        new_expr->right = this->parse_additive();
    }

    if (this->match(TokenType::ZF_RIGHT_SHIFT))
    {
        this->eh->add_error("Operator zero fill right shift(>>>) does not exist in GDScript", this->current_tok().location);
        throw JSParser::SyntaxError{};
    }

    return expr.release();
}

Expression* JSParser::parse_additive()
{
    auto expr = unique_ptr<Expression>(this->parse_multiplicative());

    while (this->match(TokenType::PLUS) || this->match(TokenType::MINUS))
    {
        this->advance();
        BinaryExpr* new_expr = new BinaryExpr{};
        new_expr->oprt = &this->current_tok(-1);
        new_expr->left = expr.release();
        expr.reset(new_expr);
        new_expr->right = this->parse_multiplicative();
    }

    return expr.release();
}

Expression* JSParser::parse_multiplicative()
{
    auto expr = unique_ptr<Expression>(this->parse_unary());

    while (this->match(TokenType::MUL) || this->match(TokenType::MOD) || this->match(TokenType::DIV))
    {
        this->advance();
        BinaryExpr* new_expr = new BinaryExpr{};
        new_expr->oprt = &this->current_tok(-1);
        new_expr->left = expr.release();
        expr.reset(new_expr);
        new_expr->right = this->parse_unary();
    }

    return expr.release();
}

Expression* JSParser::parse_unary()
{
    auto& tk = this->current_tok();
    
    if (this->unary_operators.find(tk.type) == this->unary_operators.end())
    {
        return this->parse_postfix();
    }
    else
    {
        if (this->unsuported_unary_operators.find(tk.type) != this->unsuported_unary_operators.end())
        {
            this->eh->add_error("it is not possible to translate the '" + std::string(tk.lexeme) + "' operator to gdscript", tk.location);
            throw JSParser::SyntaxError{};
        }

        auto expr = unique_ptr<UnaryExpr>(new UnaryExpr);
        expr->oprt = &tk;
        this->advance();

        expr->value = this->parse_unary();
        return expr.release();   
    }
}

Expression* JSParser::parse_postfix()
{
    auto member_expr = unique_ptr<Expression>(this->parse_member_expr());

    if (this->match(TokenType::PLUS_PLUS) || this->match(TokenType::MINUS_MINUS))
    {
        this->eh->add_error("gdscript doesn't have posfix operators", this->current_tok().location);
        throw JSParser::SyntaxError{};
    }

    return member_expr.release();
}



Expression* JSParser::parse_member_expr()
{
    auto expr = unique_ptr<PrimaryExpr>(new PrimaryExpr{});
    auto& tk = this->current_tok();

    if (tk.type == TokenType::IDENTIFIER)
    {
        expr->type = PrimaryExprType::IDENTIFIER;

        if (this->match(TokenType::ARROW, 1) || this->match(TokenType::TWO_DOTS, 1))
            expr->identifier = this->parse_function_expression();
        else
        {
            expr->identifier = &tk;
            this->advance();
        }
    }
    else if (this->literal_member_first.find(tk.type) != this->literal_member_first.end())
    {
        expr->literal = &tk;
        expr->type = PrimaryExprType::LITERAL;
        this->advance();
    }

    // array literal
    else if (tk.type == TokenType::LEFT_BRACKET)
    {
        expr->array_members = new std::vector<Expression*>();
        expr->type = PrimaryExprType::ARRAY_LITERAL;

        this->advance();

        // if not empty...
        if (!this->consume(TokenType::RIGHT_BRACKET, false))
        {
            expr->array_members->push_back(this->parse_assignment());

            while (this->match(TokenType::COMMA))
            {
                this->advance();

                if (this->match(TokenType::COMMA))
                {
                    this->eh->add_error("elision of items in literal lists does not exist in GDscript", this->current_tok().location);
                    throw JSParser::SyntaxError();
                }
                expr->array_members->push_back(this->parse_assignment());
            }

            this->consume(TokenType::RIGHT_BRACKET);
        }
    }
    else if (tk.type == TokenType::LEFT_PAREM)
    {
        if (auto fname = this->parse_function_expression(true); fname != nullptr)
        {
            expr->identifier = fname;
            expr->type = PrimaryExprType::LITERAL;
        }
        else
        {
            this->consume(TokenType::LEFT_PAREM);
            expr->expr = this->parse_expression();
            expr->type = PrimaryExprType::EXPRESSION;
            this->consume(TokenType::RIGHT_PAREM);
        }
    }
    else if (tk.type == TokenType::FUNCTION)
    {
        this->eh->add_error("function expressions does not exist in GDscript", tk.location);
        throw JSParser::SyntaxError();
    }
    else
    {
        this->eh->add_error("unexpected token '" + std::string(tk.lexeme) + "'", tk.location);
        throw JSParser::SyntaxError();
    }

    while (true)
    {
        // member access
        if (this->match(TokenType::DOT))
        {
            this->advance();

            this->expect(TokenType::IDENTIFIER, "");

            auto ptr = unique_ptr<MemberAccessPart>(new MemberAccessPart{});
            ptr->member = &this->current_tok();
            expr->parts.push_back(ptr.release());
            this->advance();
        }
        
        // index
        else if (this->match(TokenType::LEFT_BRACKET))
        {
            this->advance();
            auto ptr = unique_ptr<ArrayIndexPart>(new ArrayIndexPart{});
            ptr->index = this->parse_expression();
            expr->parts.push_back(ptr.release());
            this->advance();
        }

        // call
        else if (this->match(TokenType::LEFT_PAREM))
        {
            this->advance();

            auto call_ptr = unique_ptr<FunctionCallPart>(new FunctionCallPart{});

            // stops if empty array literal
            if (this->consume(TokenType::RIGHT_PAREM, false))
            {
                expr->parts.push_back(call_ptr.release());
                continue;
            }

            do call_ptr->args.push_back(this->parse_assignment());
            while (this->consume(TokenType::COMMA, false));
            
            expr->parts.push_back(call_ptr.release());
            this->consume(TokenType::RIGHT_PAREM);
        }
        else
            break;
    }

    return expr.release();
}


const Token* JSParser::parse_function_expression(bool backtrack)
{
    auto original_idx = this->idx;
    auto original_eh = this->eh;
    EventHandler temp_handler;

    if (backtrack)
        this->eh = &temp_handler;

    try
    {
        auto fexpr = unique_ptr<FunctionExpression>(new FunctionExpression{});

        fexpr->name.type = TokenType::IDENTIFIER;
        fexpr->name.location = this->current_tok().location;
        fexpr->name_value = std::string("__function_expression_") + std::to_string(this->fexpr_id++);
        fexpr->name.lexeme = fexpr->name_value;
        
        fexpr->literal.type = TokenType::STRING;
        fexpr->literal.location = this->current_tok().location;
        fexpr->literal_value = std::string("\"__function_expression_") + std::to_string(this->fexpr_id - 1) + '"';
        fexpr->literal.lexeme = fexpr->literal_value;

        if (this->consume(TokenType::LEFT_PAREM, false))
        {
            if (!this->match(TokenType::RIGHT_PAREM))
            {
                // parses function parameters
                do fexpr->params.push_back(this->parse_var_decl());
                while (this->consume(TokenType::COMMA, false));
            }
            this->consume(TokenType::RIGHT_PAREM);
        }
        else
        {
            fexpr->params.push_back(this->parse_var_decl());
        }

        this->consume(TokenType::ARROW);

        
        // parse function body
        if (this->consume(TokenType::LEFT_BRACE, false))
        {
            while (!this->match(TokenType::RIGHT_BRACE))
                fexpr->func_body.push_back(this->parse_stmt());
            this->consume(TokenType::RIGHT_BRACE);
        }
        else
        {
            fexpr->expression = this->parse_expression();
            fexpr->expression_body = true;
        }

        auto name = &fexpr->literal;
        this->function_expressions.push_back(fexpr.release());
        return name;
    }
    catch (const std::exception& error)
    {
        if (backtrack)
        {
            this->idx = original_idx;
            this->eh = original_eh;
            return nullptr;
        }
        else
            throw error;
    }
}






bool JSParser::at_end(int offset) const
{
    return this->idx + offset >= this->source_size || this->match(TokenType::lEOF);
}

const Token& JSParser::current_tok(int offset) const
{
    return this->source[this->idx + offset];
}

bool JSParser::match(TokenType type, int offset) const
{
    return this->current_tok(offset).type == type;
}

void JSParser::advance(int offset)
{
    this->idx += offset;
}


void JSParser::optional_semicolon()
{
    if ((!this->consume(TokenType::SEMICOLON, false) && !this->match(TokenType::RIGHT_BRACE)) && !this->match(TokenType::lEOF))
        if (!this->separated_by_newline(this->current_tok(-1), this->current_tok()))
            this->unexpected(this->current_tok());
}

void JSParser::unexpected(const Token& tk, const std::string& message_end)
{
    if (tk.type == TokenType::OF && message_end.empty())
        this->eh->add_error("'for of' loops must contain a variable declaration, unexpected token '" + std::string(tk.lexeme) + "' " + message_end, tk.location);
    else
        this->eh->add_error("unexpected token '" + std::string(tk.lexeme) + "' " + message_end, tk.location);
    throw JSParser::SyntaxError{};
}

bool JSParser::expect(TokenType type, bool error, const std::string& message_end)
{
    if (!this->match(type))
    {
        if (error)
        {
            this->unexpected(this->current_tok(), message_end);
        }
        else
            return false;
    }

    return true;
}

bool JSParser::consume(TokenType type, bool error, const std::string& message_end)
{
    auto ret = this->expect(type, error, message_end);
    
    if (ret)
        this->advance();

    return ret;
}


void JSParser::parser_rewind()
{
    while (!this->at_end())
    {
        this->advance();

        auto& tk = this->current_tok();
        const auto ptr = this->statement_first.find(tk.type);
        if (ptr != this->statement_first.end()) // && ptr->first != TokenType::VAR && ptr->first != TokenType::SEMICOLON)
            break;
    }
}

bool JSParser::separated_by_newline(const Token& tk1, const Token& tk2) const
{
    const char* start = tk1.lexeme.data() + tk1.lexeme.size();
    const char* end = tk2.lexeme.data();

    for (; start != end; ++start)
        if (*start == '\n')
            return true;
    return false;
}