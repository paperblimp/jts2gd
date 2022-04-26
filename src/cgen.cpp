
// built-in
#include <typeinfo>
#include <cstdint>

// local
#include "cgen.hpp"





void GDScriptCGen::visit(VarDecl& vdecl)
{
    this->output.append(vdecl.var->lexeme);

    if (vdecl.type != nullptr)
    {
        this->output.append(": ");
        this->output.append(this->translate_type(vdecl.type->lexeme));
    }

    if (vdecl.init_value != nullptr)
    {
        this->output.append(" = ");
        this->visit(vdecl.init_value);
    }
}

void GDScriptCGen::visit(Program& prog)
{
    this->scope.push_level();

    for (auto fexpr: prog.function_expressions)
    {
        this->visit(fexpr);
        this->line_feed();
    }

    for (auto stmt: prog.stmts)
    {
        this->visit(stmt);
        this->line_feed();
    }
    
    this->scope.pop_level();
}

void GDScriptCGen::visit(FunctionCallPart& fcall)
{
    this->output.push_back('(');
    
    if (!fcall.args.empty())
    {
        uint32_t size = fcall.args.size();

        this->visit(fcall.args[0]);
        for (uint32_t idx = 1; idx < size; ++idx)
        {
            this->output.append(", ");
            this->visit(fcall.args[idx]);
        }
    }

    this->output.push_back(')');
}

void GDScriptCGen::visit(ArrayIndexPart& arridx)
{
    this->output.push_back('[');
    this->visit(arridx.index);
    this->output.push_back(']');
}

void GDScriptCGen::visit(MemberAccessPart& mexpr)
{
    this->output.push_back('.');

    if (this->func_id)
    {
        this->output.append(this->translate_function(mexpr.member->lexeme));
    
        if (mexpr.member->lexeme == "log")
        {   
            auto idx = this->output.find("console.log", this->output.size() - 11);
            if (idx != this->output.npos)
                this->output.replace(idx, 11, "print");
        }
    }
    else
       this->output.append(mexpr.member->lexeme);
}

void GDScriptCGen::visit(ConditionalExpr& cexpr)
{
    this->visit(cexpr.expr1);
    this->output.append(" if ");
    this->visit(cexpr.cond);
    this->output.append(" else ");
    this->visit(cexpr.expr2);
}

void GDScriptCGen::visit(BinaryExpr& bexpr)
{
    auto oprt_lexeme = bexpr.oprt->lexeme;

    if (bexpr.oprt->type == TokenType::INSTANCEOF)
        oprt_lexeme = "is";

    this->visit(bexpr.left);
    this->output.push_back(' ');
    this->output.append(oprt_lexeme);
    this->output.push_back(' ');
    this->visit(bexpr.right);

}

void GDScriptCGen::visit(UnaryExpr& uexpr)
{
    this->output.append(uexpr.oprt->lexeme);
    this->visit(uexpr.value);
}


void GDScriptCGen::render_primary_expression(PrimaryExpr& pexpr, bool render_init, uint32_t render_start, uint32_t render_end)
{
    if (render_init)
    {
        switch (pexpr.type)
        {
            case (PrimaryExprType::IDENTIFIER):
            {
                if (!pexpr.parts.empty() && typeid(*pexpr.parts.at(0)) == typeid(FunctionCallPart))
                    this->output.append(this->translate_function(pexpr.identifier->lexeme));
                else
                    this->output.append(pexpr.identifier->lexeme);

                break;
            }

            case (PrimaryExprType::EXPRESSION):
            {
                this->output.push_back('(');
                this->visit(pexpr.expr);
                this->output.push_back(')');
                
                break;
            }

            case (PrimaryExprType::LITERAL):
            {
                this->output.append(pexpr.literal->lexeme);
                
                break;
            }

            case (PrimaryExprType::ARRAY_LITERAL):
            {
                this->output.push_back('[');

                if (!pexpr.array_members->empty())
                {
                    uint32_t size = pexpr.array_members->size();

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
    }


    uint32_t size = render_end;

    for (uint32_t idx = render_start; idx < size; ++idx)
    {
        this->func_id = false;
        if (idx + 1 < size && typeid(*pexpr.parts.at(idx + 1)) == typeid(FunctionCallPart))
            this->func_id = true;

        this->visit(pexpr.parts.at(idx));
    }
}


void GDScriptCGen::visit(PrimaryExpr& pexpr)
{

    auto render_args = [this](const std::vector<Expression*>& args) -> void
    {
        uint32_t size = args.size();

        if (size == 0)
            return;

        this->visit(args[0]);
        for (uint32_t idx = 1; idx < size; ++idx)
        {
            this->output.append(", ");
            this->visit(args[idx]);
        }
    };

    auto fold_fexpr_call = [&pexpr, &render_args, this](uint32_t end, const std::vector<Expression*>& args) -> void
    {
        this->output.append("call(");
        std::vector<MemberExprPart*> tmp {pexpr.parts.begin() + end, pexpr.parts.end()};
        pexpr.parts.resize(end);
        this->visit(pexpr);
        pexpr.parts.insert(pexpr.parts.end(), tmp.begin(), tmp.end());

        if (!args.empty())
        {
            this->output.append(", ");
            render_args(args);
        }
        this->output.push_back(')');
    };

    auto check_part = [&pexpr, this](int64_t idx) -> bool
    {
        if (idx == -1 && (pexpr.type != PrimaryExprType::IDENTIFIER || this->scope.has_var(pexpr.identifier->lexeme)))
            return true;
        else
            if (idx != -1 && typeid(*pexpr.parts.at(idx)) != typeid(MemberAccessPart))
                return true;
        
        return false;
    };

    int64_t end = -1;
    if (pexpr.parts.size() > 0)
        for (int64_t idx = pexpr.parts.size() - 1; idx >= 0; --idx)
            if (typeid(*pexpr.parts.at((size_t)idx)) == typeid(FunctionCallPart) && check_part(idx - 1))
            {
                fold_fexpr_call((uint32_t)idx, static_cast<FunctionCallPart*>(pexpr.parts.at((size_t)idx))->args);
                end = idx;
                break;
            }
    this->render_primary_expression(pexpr, end == -1, end + 1, pexpr.parts.size());
}

void GDScriptCGen::visit(Block& blk)
{
    if (blk.stmts.empty())
    {
        this->indent();
        this->output.append("pass");
        return;
    }

    for (auto stmt: blk.stmts)
    {
        this->visit(stmt);
        this->line_feed();
    }
    this->output.pop_back();
}

void GDScriptCGen::visit(VarDeclStmt& vdecl)
{
    for (auto decl: vdecl.decls)
    {
        this->indent();
        this->output.append(VarDeclStmtTypeRepr[(int)vdecl.type]);
        this->output.push_back(' ');
        this->scope.push_var_definition(decl->var->lexeme);
        this->visit(decl);
        this->line_feed();
    }
    this->output.pop_back();
}

void GDScriptCGen::visit(IfStmt& istmt)
{

    this->indent();

    this->output.append("if ");
    this->visit(istmt.cond);
    this->output.push_back(':');

    this->line_feed();
    this->indentation += 1;
    this->scope.push_level();
    this->visit(istmt.body);
    this->scope.pop_level();
    this->indentation -= 1;

    if (istmt.else_block)
    {
        this->line_feed();
        this->indent();
        this->output.append("else:");
        this->line_feed();
        this->indentation += 1;
        this->scope.push_level();
        this->visit(istmt.else_block);
        this->scope.pop_level();
        this->indentation -= 1;
    }
}

void GDScriptCGen::visit(WhileStmt& wstmt)
{
    this->indent();

    this->output.append("while ");
    this->visit(wstmt.cond);
    this->output.push_back(':');

    this->line_feed();
    this->indentation += 1;
    this->scope.push_level();
    this->visit(wstmt.body);
    this->scope.pop_level();
    this->indentation -= 1;
}

void GDScriptCGen::visit(ForStmt& fstmt)
{
    this->indent();

    this->scope.push_level();
    if (fstmt.for_of)
    {
        this->output.append("for ");
        this->output.append(fstmt.init_var_decl->lexeme);
        this->scope.push_var_definition(fstmt.init_var_decl->lexeme);
        this->output.append(" in ");
        this->visit(fstmt.of_expr);
        this->output.push_back(':');
        this->line_feed();
        this->indentation += 1;
        this->visit(fstmt.block);
        this->indentation -= 1;
    }
    else
    {
        this->output.append("if 1:");
        this->line_feed();
        this->indentation += 1;

        this->visit(fstmt.init_expr);
        this->line_feed();

        this->indent();
        this->output.append("while ");
        if (fstmt.cond)
            this->visit(fstmt.cond);
        else
            this->output.append("true");
        this->output.push_back(':');

        this->line_feed();
        this->indentation += 1;
        this->visit(fstmt.block);

        this->line_feed();
        this->indent();
        this->visit(fstmt.post);

        this->indentation -= 2;
    }
    this->scope.pop_level();
}

void GDScriptCGen::visit(ContinueStmt&)
{
    this->indent();
    this->output.append("continue");
}

void GDScriptCGen::visit(BreakStmt&)
{
    this->indent();
    this->output.append("break");
}

void GDScriptCGen::visit(ReturnStmt& rexpr)
{
    this->indent();
    this->output.append("return ");
    this->visit(rexpr.value);
}

void GDScriptCGen::visit(Case& cclause)
{
    this->indent();

    if (cclause.comp_values.empty())
    {
        this->output.append("_:");
    }
    else
    {
        uint32_t size = cclause.comp_values.size();

        this->visit(cclause.comp_values[0]);
        for (uint32_t idx = 1; idx < size; ++idx)
        {
            this->output.append(", ");
            this->visit(cclause.comp_values[idx]);
        }

        this->output.push_back(':');
    }
    this->line_feed();

    this->indentation += 1;
    this->scope.push_level();
    for (auto stmt: cclause.stmts)
    {
        this->visit(stmt);
        this->line_feed();
    }
    this->scope.pop_level();
    this->indent();
    this->output.append("continue");
    
    this->indentation -= 1;
}

void GDScriptCGen::visit(SwitchCaseStmt& sstmt)
{
    this->indent();

    this->output.append("match ");
    this->visit(sstmt.match_value);
    this->output.push_back(':');

    this->indentation += 1;
    for (auto case_clause: sstmt.case_clauses)
    {
        this->line_feed();
        this->visit(case_clause);
    }
    this->indentation -= 1;
}

void GDScriptCGen::visit(FunctionStmt& fdecl)
{
    this->indent();
    this->output.append("func ");
    this->output.append(fdecl.name->lexeme);
    this->output.push_back('(');

    if (!fdecl.params.empty())
    {
        const uint32_t size = fdecl.params.size();
        this->visit(fdecl.params[0]);
        for (uint32_t idx = 1; idx < size; ++idx)
        {
            this->output.append(", ");
            this->visit(fdecl.params[idx]);
        }
    }

    this->output.push_back(')');

    if (fdecl.type != nullptr)
    {
        this->output.append(" -> ");
        this->output.append(this->translate_type(fdecl.type->lexeme));
    }

    this->output.push_back(':');
    this->line_feed();


    this->indentation += 1;
    
    if (fdecl.func_body.empty())
    {
        this->indent();
        this->output.append("pass");
    }
    else
    {   this->scope.push_level();

        for (auto arg: fdecl.params)
            this->scope.push_var_definition(arg->var->lexeme);

        for (auto stmt: fdecl.func_body)
        {
            this->visit(stmt);
            this->line_feed();
        }
        this->scope.pop_level();
    }
    this->indentation -= 1;
}

void GDScriptCGen::visit(ExpressionStmt& expr)
{
    this->indent();
    this->visit(expr.expr);
}

void GDScriptCGen::visit(EmptyStmt&)
{
    this->indent();
    this->output.append("pass");
}

void GDScriptCGen::visit(ExtendsStmt& estmt)
{
    this->indent();
    this->output.append("extends ");
    this->output.append(estmt.name->lexeme);
}

void GDScriptCGen::visit(ClassExtendsStmt& cestmt)
{
    this->indent();
    this->output.append("extends ");
    this->output.append(cestmt.extended->lexeme);
    
    this->line_feed();

    this->indentation += 1;
    
    if (cestmt.body.empty())
    {
        this->indent();
        this->output.append("pass");
    }
    else
    {
        this->scope.push_level();
        for (auto stmt: cestmt.body)
        {
            this->visit(stmt);
            this->line_feed();
        }
        this->scope.pop_level();
    }
    this->indentation -= 1;
}


void GDScriptCGen::visit(FunctionExpression& fexpr)
{
    this->indent();
    this->output.append("func ");
    this->output.append(fexpr.name.lexeme);
    this->output.push_back('(');

    if (!fexpr.params.empty())
    {
        const uint32_t size = fexpr.params.size();
        this->visit(fexpr.params[0]);
        for (uint32_t idx = 1; idx < size; ++idx)
        {
            this->output.append(", ");
            this->visit(fexpr.params[idx]);
        }
    }

    this->output.push_back(')');
    this->output.push_back(':');
    this->line_feed();


    this->indentation += 1;
    
    if (fexpr.expression_body)
    {
        this->indent();
        this->output.append("return ");
        this->visit(fexpr.expression);
    }
    else
    {
        if (fexpr.func_body.empty())
        {
            this->indent();
            this->output.append("pass");
        }
        else
        {

            this->scope.push_level();

            for (auto arg: fexpr.params)
                this->scope.push_var_definition(arg->var->lexeme);

            for (auto stmt: fexpr.func_body)
            {
                this->visit(stmt);
                this->line_feed();
            }
            this->scope.pop_level();
        }
    }

    this->indentation -= 1;

}




std::string_view GDScriptCGen::translate_function(const std::string_view& name)
{
    auto ptr = function_table.find(name);
    if (ptr != function_table.end())
        return ptr->second;
    else
        return name;
}

std::string_view GDScriptCGen::translate_type(const std::string_view& type_name)
{
    auto ptr = type_table.find(type_name);
    if (ptr != type_table.end())
        return ptr->second;
    else
        return type_name;
}
