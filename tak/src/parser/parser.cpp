//
// Created by Diago on 2024-07-04.
//

#include <parser.hpp>


ast_node*
parse_keyword(parser &parser, lexer &lxr) {
     return nullptr;
}

ast_node*
parse_parenthesized_expression(parser& parser, lexer& lxr) {

    PARSER_ASSERT(lxr.current() == LPAREN, "Expected beginning of parenthesized expression.");

    ++parser.inside_parenthesized_expression;
    lxr.advance(1);

    auto* expr = parse_expression(parser, lxr);
    if(expr == nullptr || (expr->type != NODE_SINGLETON_LITERAL
        && expr->type != NODE_BINEXPR
        && expr->type != NODE_CALL
        && expr->type != NODE_IDENT
        && expr->type != NODE_ASSIGN
        && expr->type != NODE_UNARYEXPR
    )) {
        delete expr;
        return nullptr;
    }

    return expr;
}

ast_node*
parse_singleton_literal(parser& parser, lexer& lxr) {

    PARSER_ASSERT(lxr.current().kind == LITERAL, "Expected literal.");

    auto* node         = new ast_singleton_literal();
    node->value        = std::string(lxr.current().value);
    node->literal_type = lxr.current().type;

    return node;
}

ast_node*
parse_braced_expression(parser& parser, lexer& lxr) {

    PARSER_ASSERT(lxr.current() == LBRACE, "Expected left-brace.");
    return nullptr;
}

ast_node*
parse_unary_expression(parser& parser, lexer& lxr) {

    PARSER_ASSERT(lxr.current().kind == UNARY_EXPR_OPERATOR, "Expected unary operator.");


    auto* node        = new ast_unaryexpr();
    node->_operator   = lxr.current().type;

    const size_t   src_pos  = lxr.current().src_pos;
    const uint32_t line     = lxr.current().line;


    lxr.advance(1);
    node->operand = parse_expression(parser, lxr);
    if(node->operand == nullptr) {
        delete node;
        return nullptr;
    }


    const auto right_t = node->operand->type;
    if(right_t != NODE_SINGLETON_LITERAL
        && right_t != NODE_BINEXPR
        && right_t != NODE_ASSIGN
        && right_t != NODE_UNARYEXPR
        && right_t != NODE_IDENT
        && right_t != NODE_CALL
        && right_t != NODE_BRACED_EXPRESSION
    ) {
        lxr.raise_error("Unexpected expression following unary operator.", src_pos, line);
        delete node;
        return nullptr;
    }

    node->operand->parent = node;
    return node;
}


ast_node*
parse_expression(parser& parser, lexer& lxr) {

    const auto& curr = lxr.current();
    ast_node*   expr = nullptr;

    bool state       = false;
    auto _           = defer([&]{ if(!state){delete expr;} });


    switch(curr.type) {
        case END_OF_FILE:
            return nullptr;

        case IDENTIFIER:
            expr = parse_identifier(parser, lxr);
            break;

        case LPAREN:
            expr = parse_parenthesized_expression(parser, lxr);
            break;

        case LBRACE:
            expr = parse_braced_expression(parser, lxr);
            break;

        case INTEGER_LITERAL:
        case FLOAT_LITERAL:
        case STRING_LITERAL:
        case CHARACTER_LITERAL:
        case BOOLEAN_LITERAL:
            expr = parse_singleton_literal(parser, lxr);
            break;

        case BITWISE_XOR_OR_PTR:
        case BITWISE_NOT:
        case CONDITIONAL_NOT:
        case PLUS:
        case SUB:
            expr = parse_unary_expression(parser, lxr);
            break;

        case KW_RET:
        case KW_BRK:
        case KW_CONT:
        case KW_FOR:
        case KW_WHILE:
        case KW_DO:
        case KW_IF:
        case KW_ELSE:
        case KW_STRUCT:
        case KW_ENUM:
        case KW_SWITCH:
        case KW_CASE:
        case KW_ELIF:
            expr = parse_keyword(parser, lxr);
            break;

        default:
            lxr.raise_error("Invalid token at the beginning of an expression.");
            return nullptr;
    }



    if(lxr.current() == RPAREN) {
        if(!parser.inside_parenthesized_expression) {
            lxr.raise_error("Unexpected token.");
            return nullptr;
        }

        --parser.inside_parenthesized_expression;
        lxr.advance(1);
    }


    if(lxr.current().kind == BINARY_EXPR_OPERATOR &&
        (expr->type == NODE_ASSIGN
        || expr->type == NODE_CALL
        || expr->type == NODE_IDENT
        || expr->type == NODE_BINEXPR
        || expr->type == NODE_SINGLETON_LITERAL
        || expr->type == NODE_UNARYEXPR)
        ) {

        state = true;
        return parse_binary_expression(expr, parser, lxr);
    }


    if(lxr.current() == SEMICOLON) {
        if(parser.inside_parenthesized_expression) {
            lxr.raise_error("Unexpected token inside of parenthesized expression.");
            return nullptr;
        }

        lxr.advance(1);
        state = true;
        return expr;
    }


    lxr.raise_error("Unexpected token following expression.");
    return nullptr;
}


ast_vardecl*
parse_parameterized_vardecl(parser& parser, lexer& lxr) {

    PARSER_ASSERT(lxr.current() == IDENTIFIER, "Expected variable identifier.");

    const auto     name      = std::string(lxr.current().value);
    const size_t   src_pos   = lxr.current().src_pos;
    const uint32_t line      = lxr.current().line;
    uint16_t       flags     = SYM_VAR_IS_PROCARG;
    uint16_t       ptr_depth = 0;
    var_t          var_type  = VAR_NONE;


    lxr.advance(1);
    if(lxr.current() == CONST_TYPE_ASSIGNMENT) {
        flags |= SYM_VAR_IS_CONSTANT;
    }

    else if(lxr.current() != TYPE_ASSIGNMENT) {
        lxr.raise_error("Expected type assignment here. Got this instead.");
        return nullptr;
    }


    lxr.advance(1);
    if(lxr.current().kind != TYPE_IDENTIFIER) {
        lxr.raise_error("Expected type identifier. Got this instead.");
        return nullptr;
    }

    if(lxr.current() == TOKEN_KW_VOID) {
        lxr.raise_error("Void can only be used as a procedure return type.");
        return nullptr;
    }

    if(lxr.current() == TOKEN_KW_PROC) {
        lxr.raise_error("Procedures cannot be used as procedure parameters.");
        return nullptr;
    }

    var_type = token_to_var_t(lxr.current().type);
    if(var_type == VAR_NONE) {
        lxr.raise_error("Unrecognized type identifier.");
        return nullptr;
    }


    lxr.advance(1);
    if(lxr.current() == BITWISE_XOR_OR_PTR) {
        flags |= SYM_IS_POINTER;
        while(lxr.current() == BITWISE_XOR_OR_PTR) {
            ptr_depth++;
            lxr.advance(1);
        }
    }


    if(lxr.current() == LSQUARE_BRACKET) {
        lxr.raise_error("Static arrays cannot be used as parameters. Pass an array as a pointer instead.");
        return nullptr;
    }

    if(lxr.current() != COMMA && lxr.current() != RPAREN) {
        lxr.raise_error("Expected comma at the end of parameterized variable declaration.");
        return nullptr;
    }

    if(parser.scoped_symbol_exists_at_current_scope(name)) {   // a new scope should be pushed by parse_procdecl
        lxr.raise_error("Symbol already exists within this scope.");
        return nullptr;
    }


    auto* var_ptr = dynamic_cast<variable*>(parser.create_symbol(name, src_pos, line, SYM_VARIABLE, flags));
    if(var_ptr == nullptr) {
        return nullptr;
    }


    var_ptr->array_length  = 0;
    var_ptr->pointer_depth = ptr_depth;
    var_ptr->variable_type = var_type;

    auto* vardecl        = new ast_vardecl();
    vardecl->identifier  = new ast_identifier();

    vardecl->identifier->parent       = vardecl;
    vardecl->identifier->symbol_index = var_ptr->symbol_index;

    return vardecl;
}


ast_node*
parse_procdecl(procedure* proc, parser& parser, lexer& lxr) {

    PARSER_ASSERT(lxr.current() == TOKEN_KW_PROC, "Expected proc type identifier.");


    if(!(proc->flags & SYM_IS_GLOBAL)) {
        lxr.raise_error("Declaration of procedure at non-global scope.");
        return nullptr;
    }

    lxr.advance(1);
    if(lxr.current() != LPAREN) {
        lxr.raise_error("Expected parameter list here.");
        return nullptr;
    }


    ast_procdecl* node = new ast_procdecl();
    node->identifier   = new ast_identifier();
    node->type         = NODE_PROCDECL;

    node->identifier->symbol_index = proc->symbol_index;
    node->identifier->parent       = node;
    node->identifier->type         = NODE_IDENT;

    bool state = false;
    auto _     = defer([&] {
        if(!state) { delete node; }
        parser.pop_scope();
    });


    //
    // Get procedure parameters (if any)
    //

    lxr.advance(1);
    while(lxr.current() != RPAREN) {

        if(lxr.current() != IDENTIFIER) {
            lxr.raise_error("Expected procedure parameter.");
            return nullptr;
        }

        auto* param = parse_parameterized_vardecl(parser, lxr);
        if(param == nullptr) {
            return nullptr;
        }

        param->parent = node;
        node->parameters.emplace_back(param);

        if(lxr.current() == COMMA && lxr.peek(1) != IDENTIFIER) {
            lxr.raise_error("Expected identifier following comma.");
            return nullptr;
        }

        lxr.advance(1);
    }


    //
    // Get return type (remember we allow "void" here).
    //

    lxr.advance(1);
    if(lxr.current() != ARROW || lxr.peek(1).kind != TYPE_IDENTIFIER) {
        lxr.raise_error("Expected procedure return type here.");
        return nullptr;
    }

    lxr.advance(1);
    proc->return_type = token_to_var_t(lxr.current().type);
    if(proc->return_type == VAR_NONE && lxr.current() != TOKEN_KW_VOID) {
        lxr.raise_error("Unrecognized return type.");
        return nullptr;
    }


    //
    // Now we should fill out the parameter types in the symbol table for easy lookup later.
    //

    for(const auto& param : node->parameters) {
        const auto* symbol_ptr = dynamic_cast<variable*>(parser.lookup_unique_symbol(param->identifier->symbol_index));
        if(symbol_ptr == nullptr) {
            return nullptr;
        }

        proc->parameter_list.emplace_back(symbol_ptr->variable_type);
    }


    //
    // In the future we should just be leaving this as a definition.
    //

    lxr.advance(1);
    if(lxr.current() != LBRACE) {
        lxr.raise_error("Expected start of procedure body here.");
        return nullptr;
    }

    //
    // Now we parse the rest of the procedure body. Just keep calling parse_expression
    // and checking if the returned AST node can be represented inside of a procedure body.
    //

    lxr.advance(1);
    while(lxr.current() != RBRACE) {

        size_t   curr_pos = lxr.current().src_pos;
        uint32_t line     = lxr.current().line;

        auto* expr = parse_expression(parser, lxr);
        if(expr == nullptr) {
            return nullptr;
        }

        if(expr->type == NODE_STRUCT_DEFINITION || expr->type == NODE_PROCDECL || expr->type == NODE_ENUM_DEFINITION) {
            lxr.raise_error("Illegal expression inside of procedure body.", curr_pos, line);
            delete expr;
            return nullptr;
        }

        expr->parent = node;
        node->body.emplace_back(expr);
    }


    lxr.advance(1);
    state = true;
    return node;
}


ast_node*
parse_vardecl(variable* var, parser& parser, lexer& lxr) {

    PARSER_ASSERT(lxr.current().kind == TYPE_IDENTIFIER, "Expected type identifier.");

    var->variable_type = token_to_var_t(lxr.current().type);
    if(var->variable_type == VAR_NONE) {
        lxr.raise_error("Unrecognized type identifier.");
        return nullptr;
    }


    //
    // Check for pointer type.
    //

    lxr.advance(1);
    if(lxr.current() == BITWISE_XOR_OR_PTR) {

        var->flags |= SYM_IS_POINTER;
        while(lxr.current() == BITWISE_XOR_OR_PTR) {
            var->pointer_depth++;
            lxr.advance(1);
        }
    }


    //
    // Check for array type.
    //

    if(lxr.current() == LSQUARE_BRACKET) {

        var->flags |= SYM_VAR_IS_ARRAY;
        lxr.advance(1);

        if(lxr.current() == INTEGER_LITERAL) {

            auto istr = std::string(lxr.current().value);

            try {
                var->array_length = std::stoul(istr);
            } catch(const std::out_of_range& e) {
                lxr.raise_error("Array size is too large.");
                return nullptr;
            } catch(...) {
                lxr.raise_error("Array size must be a valid non-negative integer literal.");
                return nullptr;
            }

            lxr.advance(1);
        }

        if(lxr.current() != RSQUARE_BRACKET) {
            lxr.raise_error("Expected closing bracket here.");
            return nullptr;
        }

        lxr.advance(1);
    }


    //
    // Generate AST node
    //

    bool  state       = false;
    auto* node        = new ast_vardecl();
    node->identifier  = new ast_identifier();

    node->identifier->symbol_index = var->symbol_index;
    node->identifier->parent       = node;

    auto _ = defer([&]{ if(!state) { delete node; } });


    if(lxr.current() == VALUE_ASSIGNMENT) {

        lxr.advance(1);
        node->init_value = parse_expression(parser, lxr);
        if(*(node->init_value) == nullptr) {
            return nullptr;
        }


        const auto subexpr_type = (*node->init_value)->type;
        if(subexpr_type     != NODE_BINEXPR
            && subexpr_type != NODE_UNARYEXPR
            && subexpr_type != NODE_IDENT
            && subexpr_type != NODE_CALL
            && subexpr_type != NODE_SINGLETON_LITERAL
            && subexpr_type != NODE_BRACED_EXPRESSION
            && subexpr_type != NODE_ASSIGN
            ) {

            lxr.raise_error("Invalid expression being assigned to variable.", var->src_pos, var->line_number);
            return nullptr;
        }

        state = true;
        return node;
    }

    var->flags |= SYM_VAR_DEFAULT_INITIALIZED;
    state = true;
    return node;
}


ast_node*
parse_decl(parser& parser, lexer& lxr) {

    PARSER_ASSERT(lxr.current() == IDENTIFIER, "Expected identifier.");

    auto     name         = std::string(lxr.current().value);
    size_t   src_pos      = lxr.current().src_pos;
    uint32_t line         = lxr.current().line;
    uint16_t flags        = SYM_FLAGS_NONE;
    sym_t    type         = SYM_VARIABLE;


    lxr.advance(1);
    if(lxr.current() == CONST_TYPE_ASSIGNMENT) {
        flags |= SYM_VAR_IS_CONSTANT;
    }

    else if(lxr.current() != TYPE_ASSIGNMENT) {
        lxr.raise_error("Expected type assignment.");
        return nullptr;
    }

    if(parser.scope_stack.size() == 1) {
        flags |= SYM_IS_GLOBAL;
    }

    if(parser.scoped_symbol_exists_at_current_scope(name)) {
        lxr.raise_error("Symbol redeclaration, this already exists at the current scope.", src_pos, line);
        return nullptr;
    }


    lxr.advance(1);
    if(lxr.current().kind != TYPE_IDENTIFIER) {
        lxr.raise_error("Expected type identifier here.");
        return nullptr;
    }

    if(lxr.current() == TOKEN_KW_VOID) { // This is technically a type ident, but only used for returns.
        lxr.raise_error("Type identifier \"void\" can only be used as a return type.");
        return nullptr;
    }


    //
    // Parse if procedure
    //

    if(lxr.current() == TOKEN_KW_PROC) {

        type           = SYM_PROCEDURE;
        auto* proc_ptr = dynamic_cast<procedure*>(parser.create_symbol(name, src_pos, line, type, flags));
        if(proc_ptr == nullptr) {
            return nullptr;
        }

        return parse_procdecl(proc_ptr, parser, lxr);
    }


    //
    // Parse if variable
    //

    auto* var_ptr = dynamic_cast<variable*>(parser.create_symbol(name, src_pos, line, type, flags));
    if(var_ptr == nullptr) {
        return nullptr;
    }

    return parse_vardecl(var_ptr, parser, lxr);
}


ast_node*
parse_assign(ast_identifier* ident, parser& parser, lexer& lxr) {

    PARSER_ASSERT(lxr.current() == VALUE_ASSIGNMENT, "Expected '='.");

    uint32_t line      = lxr.current().line;
    size_t   src_pos   = lxr.current().src_pos;

    auto*    node      = new ast_assign();
    bool     state     = false;
    auto     _         = defer([&]{ if(!state){delete node;} });


    lxr.advance(1);
    node->identifier         = ident;
    node->identifier->parent = node;
    node->expression         = parse_expression(parser, lxr);

    if(node->expression == nullptr) {
        return nullptr;
    }


    const auto subexpr_type = node->expression->type;
    if(    subexpr_type != NODE_BINEXPR
        && subexpr_type != NODE_UNARYEXPR
        && subexpr_type != NODE_IDENT
        && subexpr_type != NODE_CALL
        && subexpr_type != NODE_SINGLETON_LITERAL
        && subexpr_type != NODE_BRACED_EXPRESSION
        && subexpr_type != NODE_ASSIGN
    ) {
        lxr.raise_error("Invalid expression being assigned to variable", src_pos, line);
        return nullptr;
    }


    state = true;
    return node;
}


ast_node*
parse_call(parser& parser, lexer& lxr) {

    PARSER_ASSERT(lxr.current() == IDENTIFIER, "Expected called identifier.");
    return nullptr;
}


ast_node*
parse_binary_expression(ast_node* left_operand, parser& parser, lexer& lxr) {

    PARSER_ASSERT(lxr.current().kind == BINARY_EXPR_OPERATOR, "Expected binary operator.");
    PARSER_ASSERT(left_operand != nullptr, "Null left operand passed.");


    if(lxr.current() == VALUE_ASSIGNMENT) {
        auto* ident = dynamic_cast<ast_identifier*>(left_operand);
        if(ident == nullptr) {
            lxr.raise_error("Assignment to constant.");
            return nullptr;
        }

        return parse_assign(ident, parser, lxr);
    }


    bool  state              = false;
    auto* binexpr            = new ast_binexpr();
    auto  _                  = defer([&]{ if(!state){ delete binexpr; } });

    binexpr->_operator       = lxr.current().type;
    binexpr->left_op         = left_operand;
    binexpr->left_op->parent = binexpr;


    size_t   src_pos = lxr.current().src_pos;
    uint32_t line    = lxr.current().line;

    lxr.advance(1);
    binexpr->right_op = parse_expression(parser, lxr);
    if(binexpr->right_op == nullptr) {
        return nullptr;
    }


    binexpr->right_op->parent = binexpr;
    const auto right_t = binexpr->right_op->type;
    if(right_t != NODE_SINGLETON_LITERAL
        && right_t != NODE_BINEXPR
        && right_t != NODE_ASSIGN
        && right_t != NODE_UNARYEXPR
        && right_t != NODE_IDENT
        && right_t != NODE_CALL
        && right_t != NODE_BRACED_EXPRESSION
    ) {
        lxr.raise_error("Unexpected expression following binary operator.", src_pos, line);
        return nullptr;
    }


    state = true;
    return binexpr;
}


ast_node*
parse_identifier(parser& parser, lexer& lxr) {

    PARSER_ASSERT(lxr.current() == IDENTIFIER, "Expected identifier.");
    const auto next_type = lxr.peek(1).type;


    if(next_type == TYPE_ASSIGNMENT || next_type == CONST_TYPE_ASSIGNMENT) {
        return parse_decl(parser, lxr);
    }

    if(next_type == LPAREN) {
        return parse_call(parser, lxr);
    }


    uint32_t sym_index = 0;
    if(sym_index = parser.lookup_scoped_symbol(std::string(lxr.current().value)); sym_index == INVALID_SYMBOL_INDEX) {
        lxr.raise_error("Symbol does not exist in this scope.");
        return nullptr;
    }

    auto* ident         = new ast_identifier();
    ident->symbol_index = sym_index;

    lxr.advance(1);
    if(next_type == VALUE_ASSIGNMENT) {
        return parse_assign(ident, parser, lxr);
    }

    return ident;
}


bool
generate_ast_from_source(parser& parser, const std::string& source_file_name) {

    lexer     lxr;
    ast_node* toplevel_decl = nullptr;


    if(!lxr.init(source_file_name)) {
        return false;
    }

    if(parser.scope_stack.empty()) {
        parser.push_scope(); // push global scope
    }


    do {
        toplevel_decl = parse_expression(parser, lxr);
        if(toplevel_decl == nullptr) {
            break;
        }

        if(toplevel_decl->type != NODE_PROCDECL && toplevel_decl->type != NODE_VARDECL) {
            lxr.raise_error("This is not allowed at global scope.");
            return false;
        }

        parser.toplevel_decls.emplace_back(toplevel_decl);
    } while(true);


    parser.pop_scope();
    return lxr.current() == END_OF_FILE;
}
