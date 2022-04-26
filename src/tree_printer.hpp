#ifndef JTS2GD_JS_TREE_PRINTER
#define JTS2GD_JS_TREE_PRINTER

// built-in
#include <stack>
#include <type_traits>
#include <cstdint>


// local
#include "tree.hpp"


//
//  Javascript Printer
//
//
//  System to represent verbatim the content obtained by the parser.
//  Useful to find errors in the parser or in the tree. 
//
//  All functions must specify whether indentation is 
//  required (placing a true or false on top of the stack)
//  before visiting another 'Statement' node in the tree.
//
//  All 'Statements' must check at the beginning if it is necessary 
//  to perform identification.


struct Printer: public Visitor
{

    uint32_t indentation = 0;
    std::string output;
    std::stack<bool> indent_stack;


    void indent(int offset = 0)
    {
        // checking to avoid negative indentation
        if (int64_t(this->indentation) + offset > 0)
            this->output.append(std::string(4 * (this->indentation + offset), ' '));
    }

    void line_feed()
    {
        this->output.push_back('\n');
    }

    bool should_indent()
    {
        return this->indent_stack.top();
    }


    void visit(VarDecl& vdecl) override
    {
        this->output.append(vdecl.var->lexeme);

        if (vdecl.type)
        {
            this->output.append(": ");
            this->output.append(vdecl.type->lexeme);
        }

        if (vdecl.init_value)
        {
            this->output.append(" = ");
            this->indent_stack.push(false);
            this->visit(vdecl.init_value);
            this->indent_stack.pop();
        }
    }

    void visit(Program& prog) override
    {
        this->indent_stack.push(true);

        for (auto fexpr: prog.function_expressions)
        {
            this->visit(fexpr);
            this->line_feed();
        }
            
        for (auto stmt: prog.stmts)
        {
            this->visit(stmt);
            this->output.push_back(';');
            this->line_feed();
        }
        this->indent_stack.pop();
        this->line_feed();
    }

    void visit(FunctionCallPart& fcall) override
    {
        this->indent_stack.push(false);
        this->output.push_back('(');
        
        
        uint32_t size = fcall.args.size();
        
        if (size > 0)
        {
            this->visit(fcall.args[0]);
            for (uint32_t idx = 1; idx < size; ++idx)
            {
                this->output.append(", ");
                this->visit(fcall.args[idx]);
            }
        }

        this->output.push_back(')');
        this->indent_stack.pop();
    }

    void visit(ArrayIndexPart& arridx) override
    {
        this->indent_stack.push(false);
        this->output.push_back('[');
        this->visit(arridx.index);
        this->output.push_back(']');
        this->indent_stack.pop();
    }

    void visit(MemberAccessPart& maccess) override
    {
        this->output.push_back('.');
        this->output.append(maccess.member->lexeme);
    }

    void visit(ConditionalExpr& cexpr) override
    {
        this->indent_stack.push(false);
        
        this->visit(cexpr.cond);
        this->output.append(" ? ");
        this->visit(cexpr.expr1);
        this->output.append(" : ");
        this->visit(cexpr.expr2);

        this->indent_stack.pop();
    }

    void visit(BinaryExpr& bexpr) override
    {
        this->indent_stack.push(false);
        this->visit(bexpr.left);
        this->output.push_back(' ');
        this->output.append(bexpr.oprt->lexeme);
        this->output.push_back(' ');
        this->visit(bexpr.right);
        this->indent_stack.pop();
    }

    void visit(UnaryExpr& uexpr) override
    {
        this->indent_stack.push(false);
        this->output.append(uexpr.oprt->lexeme);
        this->visit(uexpr.value);
        this->indent_stack.pop();
    }

    void visit(PrimaryExpr& pexpr) override
    {
        this->indent_stack.push(false);
        
        switch (pexpr.type)
        {
            case (PrimaryExprType::IDENTIFIER):
            {
                this->output.append(pexpr.identifier->lexeme);
                break;
            }
            case (PrimaryExprType::LITERAL):
            {
                this->output.append(pexpr.literal->lexeme);
                break;
            }
            case (PrimaryExprType::EXPRESSION):
            {
                this->output.push_back('(');
                this->visit(pexpr.expr);
                this->output.push_back(')');
                break;
            }
            case (PrimaryExprType::ARRAY_LITERAL):
            {
                this->output.push_back('[');

                uint32_t size = (*pexpr.array_members).size();

                if (size > 0)
                {
                    this->visit((*pexpr.array_members)[0]);
                    for (uint32_t idx = 1; idx < size; ++idx)
                    {
                        this->output.append(", ");
                        this->visit((*pexpr.array_members)[idx]);
                    }
                }

                this->output.push_back(']');
                break;
            }
        }
        
        for (auto member: pexpr.parts)
            this->visit(member);

        this->indent_stack.pop();
    }

    void visit(Block& blk) override
    {
        if (this->should_indent())
            this->indent();

        this->output.push_back('{');
        this->line_feed();

        this->indentation += 1;
        this->indent_stack.push(true);
        for (auto stmt: blk.stmts)
        {
            this->visit(stmt);
            this->output.push_back(';');
            this->line_feed();
        }
        this->indent_stack.pop();
        this->indentation -= 1;

        this->indent();
        this->output.push_back('}');
    }

    void visit(VarDeclStmt& vdecl) override
    {
        if (this->should_indent())
            this->indent();

        this->output.append("var ");
        this->indent_stack.push(false);

        this->visit(vdecl.decls[0]);
        for (uint32_t idx = 1; idx < vdecl.decls.size(); ++idx)
        {
            output.append(", ");
            this->visit(vdecl.decls[idx]);
        }

        this->indent_stack.pop();
    }

    void visit(IfStmt& istmt) override
    {
        if (this->should_indent())
            this->indent();

        this->output.append("if (");
        this->indent_stack.push(false);
        this->visit(istmt.cond);
        this->indent_stack.pop();
        this->output.push_back(')');

        this->line_feed();
        this->indentation += 1;
        this->indent_stack.push(true);
        this->visit(istmt.body);

        if (istmt.else_block)
        {
            this->indent(-1);
            this->output.append("else ");
            this->line_feed();
            this->visit(istmt.else_block);
        }
    
        this->indent_stack.pop();
        this->indentation -= 1;
    }

    void visit(WhileStmt& wstmt) override
    {
        if (this->should_indent())
            this->indent();

        this->output.append("while (");
        this->indent_stack.push(false);
        this->visit(wstmt.cond);
        this->indent_stack.pop();
        this->output.push_back(')');

        this->line_feed();
        this->indentation += 1;
        this->indent_stack.push(true);
        this->visit(wstmt.body);
        this->indent_stack.pop();
        this->indentation -= 1;
    }

    void visit(ForStmt& fstmt) override
    {
        if (this->should_indent())
            this->indent();

        this->output.append("for (");
        this->indent_stack.push(false);


        if (fstmt.for_of)
        {
            this->output.append(fstmt.init_var_decl->lexeme);
            this->output.append(" of ");
            this->visit(fstmt.of_expr);
        }
        else
        {
            this->visit(fstmt.init_expr);
            this->output.append("; ");
            this->visit(fstmt.cond);
            this->output.append("; ");
            this->visit(fstmt.post);
        }

        this->indent_stack.pop();
        this->output.push_back(')');
        this->line_feed();

        this->indentation += 1;
        this->indent_stack.push(true);
        this->visit(fstmt.block);
        this->indent_stack.pop();
        this->indentation -= 1;
    }

    void visit(ContinueStmt&) override
    {
        if (this->should_indent())
            this->indent();

        this->output.append("continue");
    }

    void visit(BreakStmt&) override
    {
        if (this->should_indent())
            this->indent();

        this->output.append("break");
    }

    void visit(ReturnStmt& rexpr) override
    {
        if (this->should_indent())
            this->indent();

        this->output.append("return ");
        this->indent_stack.push(false);
        this->visit(rexpr.value);
        this->indent_stack.pop();

    }

    void visit(Case& cs) override
    {
        if (this->should_indent())
            this->indent();

        if (cs.comp_values.empty())
        {
            this->line_feed();
            this->indent();
            this->output.append("default:");
        }
        else
        {
            for (auto clause: cs.comp_values)
            {
                this->line_feed();

                this->indent();
                this->output.append("case ");
                this->indent_stack.push(false);
                this->visit(clause);
                this->indent_stack.pop();
                this->output.push_back(':');
            }
        }

        this->line_feed();
        this->indentation += 1;
        this->indent_stack.push(true);

        for (auto stmt: cs.stmts)
        {
            this->visit(stmt);
            this->output.push_back(';');
            this->line_feed();
        }

        this->indent_stack.pop();
        this->indentation -= 1;
    }

    void visit(SwitchCaseStmt& sstmt) override
    {

        if (this->should_indent())
            this->indent();

        this->output.append("switch (");
        this->indent_stack.push(false);
        this->visit(sstmt.match_value);
        this->indent_stack.pop();
        this->output.push_back(')');
        this->line_feed();

        this->indent();
        this->output.push_back('{');
        this->indentation += 1;
        this->indent_stack.push(false);

        for (auto& c: sstmt.case_clauses)
            this->visit(c);


        this->indent_stack.pop();
        this->indentation -= 1;
        this->indent();
        this->output.push_back('}');

    }

    void visit(FunctionStmt& fdecl) override
    {
        if (this->should_indent())
            this->indent();

        this->output.append("function ");
        this->output.append(fdecl.name->lexeme);
        this->output.push_back('(');

        this->indent_stack.push(false);

        uint32_t size = fdecl.params.size();

        if (size > 0)
        {
            this->visit(fdecl.params[0]);
            for (uint32_t idx = 1; idx < fdecl.params.size(); ++idx)
            {
                output.append(", ");
                this->visit(fdecl.params[idx]);
            }
        }

        this->indent_stack.pop();
        this->output.push_back(')');

        if (fdecl.type != nullptr)
        {
            this->output.append(": ");
            this->output.append(fdecl.type->lexeme);
        }

        this->line_feed();

        this->indent();
        this->output.push_back('{');
        this->line_feed();
        this->indentation += 1;
        this->indent_stack.push(true);
        
        for (auto stmt: fdecl.func_body)
        {
            this->visit(stmt);
            this->output.push_back(';');
            this->line_feed();
        }

        this->indent_stack.pop();
        this->indentation -= 1;
        this->indent();
        this->output.push_back('}');
    }

    void visit(ExpressionStmt& expr) override
    {
        if (this->should_indent())
            this->indent();
        this->visit(expr.expr);
    }

    void visit(EmptyStmt&) override
    {
        if (this->should_indent())
            this->indent();
    }

    void visit(ExtendsStmt& estmt) override
    {
        if (this->should_indent())
            this->indent();
            
        this->output.append("extends ");
        this->output.append(estmt.name->lexeme);
    }

    void visit(ClassExtendsStmt& cestmt) override
    {
        if (this->should_indent())
            this->indent();

        this->output.append("class ");
        this->output.append(cestmt.class_name->lexeme);
        this->output.append(" extends");

        this->line_feed();
        this->indent();
        this->output.push_back('{');
        this->indentation += 1;
        this->indent_stack.push(true);

        for (auto stmt: cestmt.body)
        {
            this->visit(stmt);
            this->output.push_back(';');
            this->line_feed();
        }

        this->indent_stack.pop();
        this->indentation -= 1;
        this->output.push_back('}');
    }

    void visit(FunctionExpression& fexpr) override
    {
        if (this->should_indent())
            this->indent();

        this->output.push_back('(');
        if (fexpr.params.size() > 0)
        {
            this->visit(fexpr.params[0]);
            for (uint32_t idx = 1; idx < fexpr.params.size(); ++idx)
            {
                output.append(", ");
                this->visit(fexpr.params[idx]);
            }
        }
        this->output.push_back(')');
        this->output.append("=>");

        if (fexpr.expression_body)
        {
            this->output.append("return ");
            this->visit(fexpr.expression);
        }
        else
        {
            this->line_feed();
            this->indent();
            this->output.push_back('{');
            this->indentation += 1;
            this->indent_stack.push(true);

            for (auto stmt: fexpr.func_body)
            {
                this->visit(stmt);
                this->output.push_back(';');
                this->line_feed();
            }

            this->indent_stack.pop();
            this->indentation -= 1;
            this->output.push_back('}');
        }
    }



    public:


        // Interface to visit other nodes that checks if the pointer is null.
        template <typename T>
        void visit(T* element)
        {
            static_assert(std::is_base_of_v<Element, T>);
            if (element != nullptr)
                element->accept(*this);  
        }

};


inline std::string print_tree(Program* prog)
{
    Printer p;
    prog->accept(p);
    return p.output;
}

#endif