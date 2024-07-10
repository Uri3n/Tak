//
// Created by Diago on 2024-07-09.
//

#include <parser.hpp>


ast_node*
parse_proc_ptr(procedure* proc, parser& parser, lexer& lxr) {

    PARSER_ASSERT(lxr.current() == TOKEN_KW_PROC, "Expected \"proc\" keyword.");
    PARSER_ASSERT(lxr.peek(1) == BITWISE_XOR_OR_PTR, "Expected next token to be pointy fella (^)");


    //
    // Get correct pointer depth
    //

    lxr.advance(1);
    while(lxr.current() == BITWISE_XOR_OR_PTR) {
        proc->pointer_depth++;
        lxr.advance(1);
    }


    //
    // Check for array type.
    //

    if(lxr.current() == LSQUARE_BRACKET) {

        proc->flags |= SYM_IS_ARRAY;
        lxr.advance(1);

        if(lxr.current() == INTEGER_LITERAL) {

            const auto istr = std::string(lxr.current().value);

            try {
                proc->array_length = std::stoul(istr);
            } catch(const std::out_of_range& e) {
                lxr.raise_error("Array size is too large.");
                return nullptr;
            } catch(...) {
                lxr.raise_error("Array size must be a valid non-negative integer literal.");
                return nullptr;
            }

            if(proc->array_length == 0) {
                lxr.raise_error("Array length cannot be 0.");
                return nullptr;
            }

            lxr.advance(1);
        }

        if(lxr.current() != RSQUARE_BRACKET) {
            lxr.raise_error("Expected closing square bracket.");
            return nullptr;
        }

        lxr.advance(1);
    }


    //
    // Get param types
    //

    if(lxr.current() != LPAREN || (lxr.peek(1).kind != TYPE_IDENTIFIER && lxr.peek(1) != RPAREN)) {
        lxr.raise_error("Expected list of accepted parameter types. Example: \"(i32, i8, bool)\".");
        return nullptr;
    }


    lxr.advance(1);
    while(lxr.current() != RPAREN) {
        if(lxr.current().kind != TYPE_IDENTIFIER) {
            lxr.raise_error("Expected type identifier.");
            return nullptr;
        }

        proc->parameter_list.emplace_back(token_to_var_t(lxr.current().type));
        lxr.advance(1);

        if(lxr.current() == COMMA) {
            lxr.advance(1);
        }
    }


    //
    // Get return type
    //

    if(lxr.peek(1) != ARROW || lxr.peek(2).kind != TYPE_IDENTIFIER) {
        lxr.raise_error("Expected procedure return type after parameter list. Example: -> i32");
        return nullptr;
    }

    lxr.advance(2);
    proc->return_type = token_to_var_t(lxr.current().type);
    if(proc->return_type == VAR_NONE && lxr.current() != TOKEN_KW_VOID) {
        lxr.raise_error("Invalid procedure return type.");
        return nullptr;
    }


    //
    // Create AST node, check for additional assignment
    //

    auto* node       = new ast_vardecl();
    node->identifier = new ast_identifier();
    bool state       = false;
    auto _           = defer([&]{ if(!state){ delete node; } });

    node->identifier->parent       = node;
    node->identifier->symbol_index = proc->symbol_index;


    lxr.advance(1);
    if(lxr.current() == VALUE_ASSIGNMENT) {

        const size_t   curr_pos  = lxr.current().src_pos;
        const uint32_t curr_line = lxr.current().line;


        lxr.advance(1);
        node->init_value = parse_expression(parser, lxr, true);
        if(*(node->init_value) == nullptr) {
            return nullptr;
        }


        const auto subexpr_type = (*node->init_value)->type;
        if(!VALID_SUBEXPRESSION(subexpr_type)) {
            lxr.raise_error("Invalid expression being assigned to variable.", curr_pos, curr_line);
            return nullptr;
        }

        state = true;
        return node;

    }


    proc->flags |= SYM_DEFAULT_INITIALIZED;
    state = true;
    return node;
}


ast_vardecl*
parse_parameterized_vardecl(parser& parser, lexer& lxr) {

    PARSER_ASSERT(lxr.current() == IDENTIFIER, "Expected variable identifier.");

    const auto     name      = std::string(lxr.current().value);
    const size_t   src_pos   = lxr.current().src_pos;
    const uint32_t line      = lxr.current().line;
    uint16_t       flags     = SYM_IS_PROCARG;
    uint16_t       ptr_depth = 0;
    var_t          var_type  = VAR_NONE;


    lxr.advance(1);
    if(lxr.current() == CONST_TYPE_ASSIGNMENT) {
        flags |= SYM_IS_CONSTANT;
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

    if(!(proc->flags & SYM_IS_CONSTANT)) {
        lxr.raise_error("Procedures must be declared as constant. This one was declared using ':'.");
        return nullptr;
    }


    lxr.advance(1);
    if(lxr.current() != LPAREN) {
        lxr.raise_error("Expected parameter list here.");
        return nullptr;
    }


    parser.push_scope();

    auto* node         = new ast_procdecl();
    node->identifier   = new ast_identifier();

    node->identifier->symbol_index = proc->symbol_index;
    node->identifier->parent       = node;

    bool state = false;
    auto _     = defer([&] {
        if(!state) {
            delete node;
        }
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

        if(lxr.current() == COMMA) {
            lxr.advance(1);
        }

    }


    //
    // Get return type (remember we allow "void" here).
    //

    lxr.advance(1);
    if(lxr.current() != ARROW || lxr.peek(1).kind != TYPE_IDENTIFIER) {
        lxr.raise_error("Expected procedure return type after parameter list. Example: -> i32");
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

        auto* expr = parse_expression(parser, lxr, false);
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

        var->flags |= SYM_IS_ARRAY;
        lxr.advance(1);

        if(lxr.current() == INTEGER_LITERAL) {

            const auto istr = std::string(lxr.current().value);

            try {
                var->array_length = std::stoul(istr);
            } catch(const std::out_of_range& e) {
                lxr.raise_error("Array size is too large.");
                return nullptr;
            } catch(...) {
                lxr.raise_error("Array size must be a valid non-negative integer literal.");
                return nullptr;
            }

            if(var->array_length == 0) {
                lxr.raise_error("Array length cannot be 0.");
                return nullptr;
            }

            lxr.advance(1);
        }

        if(lxr.current() != RSQUARE_BRACKET) {
            lxr.raise_error("Expected closing square bracket.");
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

        const size_t   curr_pos  = lxr.current().src_pos;
        const uint32_t curr_line = lxr.current().line;


        lxr.advance(1);
        node->init_value = parse_expression(parser, lxr, true);
        if(*(node->init_value) == nullptr) {
            return nullptr;
        }


        const auto subexpr_type = (*node->init_value)->type;
        if(!VALID_SUBEXPRESSION(subexpr_type)) {
            lxr.raise_error("Invalid expression being assigned to variable.", curr_pos, curr_line);
            return nullptr;
        }

        state = true;
        return node;
    }

    var->flags |= SYM_DEFAULT_INITIALIZED;
    state = true;
    return node;
}


ast_node*
parse_decl(parser& parser, lexer& lxr) {

    PARSER_ASSERT(lxr.current() == IDENTIFIER, "Expected identifier.");

    const auto     name     = std::string(lxr.current().value);
    const size_t   src_pos  = lxr.current().src_pos;
    const uint32_t line     = lxr.current().line;
    uint16_t       flags    = SYM_FLAGS_NONE;
    sym_t          type     = SYM_VARIABLE;


    lxr.advance(1);
    if(lxr.current() == CONST_TYPE_ASSIGNMENT) {
        flags |= SYM_IS_CONSTANT;
    }

    else if(lxr.current() != TYPE_ASSIGNMENT) {
        lxr.raise_error("Expected type assignment.");
        return nullptr;
    }

    if(parser.scope_stack.size() <= 1) {
        flags |= SYM_IS_GLOBAL;
    }


    //
    // Check for redeclarations.
    //

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

        if(lxr.peek(1) == BITWISE_XOR_OR_PTR) { // function pointer
            proc_ptr->flags |= SYM_IS_POINTER;
            return parse_proc_ptr(proc_ptr, parser, lxr);
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