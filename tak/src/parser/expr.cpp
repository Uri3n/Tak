//
// Created by Diago on 2024-07-09.
//

#include <parser.hpp>


ast_node*
parse_expression(parser& parser, lexer& lxr, const bool subexpression, const bool parse_single) {

    const auto  curr  = lxr.current();
    ast_node*   expr  = nullptr;
    bool        state = false;

    defer_if(!state, [&] {
        delete expr;
    });


    if(curr == TOKEN_END_OF_FILE)       return nullptr;
    if(curr == TOKEN_IDENTIFIER)        expr = parse_identifier(parser, lxr);
    else if(curr == TOKEN_LPAREN)       expr = parse_parenthesized_expression(parser, lxr);
    else if(curr == TOKEN_LBRACE)       expr = parse_braced_expression(parser, lxr);
    else if(curr.kind == KIND_LITERAL)  expr = parse_singleton_literal(parser, lxr);
    else if(curr.kind == KIND_KEYWORD)  expr = parse_keyword(parser, lxr);
    else if(VALID_UNARY_OPERATOR(curr)) expr = parse_unary_expression(parser, lxr);


    else {
        lxr.raise_error("Invalid token at the beginning of an expression.");
        return nullptr;
    }

    if(expr == nullptr)
        return nullptr;


    //
    // Check if we're leaving a parenthesized expression
    //

    if(lxr.current() == TOKEN_RPAREN) {
        if(!parser.inside_parenthesized_expression_) {
            lxr.raise_error("Unexpected token.");
            return nullptr;
        }

        --parser.inside_parenthesized_expression_;
        lxr.advance(1);
        state = true;
        return expr;
    }


    //
    // For these expressions we should not check the terminal no matter what.
    //

    if(EXPR_NEVER_NEEDS_TERMINAL(expr->type)) {
        state = true;
        return expr;
    }


    //
    // If the next token is an operator we can recurse until the subexpression is parsed.
    //

    while(lxr.current() == TOKEN_LSQUARE_BRACKET && !parse_single)
        expr = parse_subscript(expr, parser, lxr);

    if(lxr.current().kind == KIND_BINARY_EXPR_OPERATOR && !parse_single)
        expr = parse_binary_expression(expr, parser, lxr);

    if(subexpression) {
        state = true;
        return expr;
    }


    if(lxr.current() == TOKEN_SEMICOLON || lxr.current() == TOKEN_COMMA) {
        if(parser.inside_parenthesized_expression_) {
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


ast_node*
parse_keyword(parser &parser, lexer &lxr) {

    parser_assert(lxr.current().kind == KIND_KEYWORD, "Expected keyword.");

    if(lxr.current() == TOKEN_KW_RET)       return parse_ret(parser, lxr);
    if(lxr.current() == TOKEN_KW_IF)        return parse_branch(parser, lxr);
    if(lxr.current() == TOKEN_KW_SWITCH)    return parse_switch(parser, lxr);
    if(lxr.current() == TOKEN_KW_WHILE)     return parse_while(parser, lxr);
    if(lxr.current() == TOKEN_KW_FOR)       return parse_for(parser, lxr);
    if(lxr.current() == TOKEN_KW_STRUCT)    return parse_structdef(parser, lxr);
    if(lxr.current() == TOKEN_KW_NAMESPACE) return parse_namespace(parser, lxr);


    lxr.raise_error("This keyword is not allowed here.");
    return nullptr;
}


ast_node*
parse_parenthesized_expression(parser& parser, lexer& lxr) {

    parser_assert(lxr.current() == TOKEN_LPAREN, "Expected beginning of parenthesized expression.");

    ++parser.inside_parenthesized_expression_;
    lxr.advance(1);

    const size_t   curr_pos = lxr.current().src_pos;
    const uint32_t line     = lxr.current().line;
    auto* expr              = parse_expression(parser, lxr, true);

    if(expr == nullptr) return nullptr;
    if(!VALID_SUBEXPRESSION(expr->type)) {
        lxr.raise_error("This expression cannot be used within parentheses.", curr_pos, line);
        delete expr;
        return nullptr;
    }

    return expr;
}


ast_node*
parse_singleton_literal(parser& parser, lexer& lxr) {

    parser_assert(lxr.current().kind == KIND_LITERAL, "Expected literal.");

    auto* node         = new ast_singleton_literal();
    node->literal_type = lxr.current().type;


    if(node->literal_type == TOKEN_STRING_LITERAL || node->literal_type == TOKEN_CHARACTER_LITERAL) {
        const auto real = remove_escaped_chars(lxr.current().value);
        if(!real) {
            lxr.raise_error("String contains one or more invalid escaped characters.");
            delete node;
            return nullptr;
        }

        node->value = *real;
    }

    else {
        node->value = std::string(lxr.current().value);
    }

    lxr.advance(1);
    return node;
}


ast_node*
parse_braced_expression(parser& parser, lexer& lxr) {

    parser_assert(lxr.current() == TOKEN_LBRACE, "Expected left-brace.");

    bool  state = false;
    auto* node  = new ast_braced_expression();

    defer_if(!state, [&] {
        delete node;
    });


    lxr.advance(1);
    while(lxr.current() != TOKEN_RBRACE) {

        const size_t   curr_pos = lxr.current().src_pos;
        const uint32_t line     = lxr.current().line;

        node->members.emplace_back(parse_expression(parser, lxr, true));
        if(node->members.back() == nullptr) {
            return nullptr;
        }

        if(!VALID_SUBEXPRESSION(node->members.back()->type)) {
            lxr.raise_error("Invalid subexpression within braced expression.", curr_pos, line);
            return nullptr;
        }

        if(lxr.current() == TOKEN_COMMA) {
            lxr.advance(1);
        }
    }


    lxr.advance(1);
    state = true;
    return node;
}


ast_node*
parse_unary_expression(parser& parser, lexer& lxr) {

    parser_assert(VALID_UNARY_OPERATOR(lxr.current()), "Expected unary operator.");

    auto* node        = new ast_unaryexpr();
    node->_operator   = lxr.current().type;

    const size_t   src_pos  = lxr.current().src_pos;
    const uint32_t line     = lxr.current().line;


    lxr.advance(1);
    node->operand = parse_expression(parser, lxr, true, true);
    if(node->operand == nullptr) {
        delete node;
        return nullptr;
    }

    const auto right_t = node->operand->type;
    if(!VALID_SUBEXPRESSION(right_t)) {
        lxr.raise_error("Unexpected expression following unary operator.", src_pos, line);
        delete node;
        return nullptr;
    }

    node->operand->parent = node;
    return node;
}


ast_node*
parse_assign(ast_node* assigned, parser& parser, lexer& lxr) {

    parser_assert(lxr.current() == TOKEN_VALUE_ASSIGNMENT, "Expected '='.");

    const uint32_t line     = lxr.current().line;
    const size_t   src_pos  = lxr.current().src_pos;

    auto* node   = new ast_assign();
    bool  state  = false;

    defer_if(!state, [&] {
        delete node;
    });


    lxr.advance(1);
    node->assigned         = assigned;
    node->assigned->parent = node;
    node->expression       = parse_expression(parser, lxr, true);

    if(node->expression == nullptr) {
        return nullptr;
    }


    const auto subexpr_type = node->expression->type;
    if(!VALID_SUBEXPRESSION(subexpr_type)) {
        lxr.raise_error("Invalid expression being assigned to variable", src_pos, line);
        return nullptr;
    }


    state = true;
    return node;
}


ast_node*
parse_call(const uint32_t sym_index, parser& parser, lexer& lxr) {

    parser_assert(lxr.current() == TOKEN_LPAREN, "Expected '('.");

    //
    // First we need to validate that the called symbol is a procedure.
    // I COULD also just do this in the checker?? hmm....
    //

    const auto* sym = parser.lookup_unique_symbol(sym_index);
    if(sym == nullptr) {
        return nullptr;
    }

    if(sym->type.sym_type != SYM_PROCEDURE) {
        lxr.raise_error("Attempt to call symbol that is not a procedure.");
        return nullptr;
    }


    //
    // Generate AST node.
    //

    bool  state      = false;
    auto* node       = new ast_call();
    node->identifier = new ast_identifier();

    node->identifier->parent       = node;
    node->identifier->symbol_index = sym_index;

    defer_if(!state, [&] {
        delete node;
    });


    //
    // Parse the parameter list.
    //

    lxr.advance(1);
    if(lxr.current() == TOKEN_RPAREN) {
        lxr.advance(1);
        state = true;
        return node;
    }


    const uint16_t old_paren_index = parser.inside_parenthesized_expression_++;
    while(old_paren_index < parser.inside_parenthesized_expression_) {

        const size_t   curr_pos = lxr.current().src_pos;
        const uint32_t line     = lxr.current().line;


        auto* expr = parse_expression(parser, lxr, true);
        if(expr == nullptr) {
            return nullptr;
        }

        const auto _type = expr->type;
        if(!VALID_SUBEXPRESSION(_type)) {
            lxr.raise_error("Invalid subexpression within call.", curr_pos, line);
            return nullptr;
        }

        node->arguments.emplace_back(expr);

        if(old_paren_index >= parser.inside_parenthesized_expression_)
            break;

        if(lxr.current() == TOKEN_COMMA || lxr.current() == TOKEN_SEMICOLON) {
            lxr.advance(1);
            if(lxr.current() == TOKEN_RPAREN) {
                --parser.inside_parenthesized_expression_;
                lxr.advance(1);
            }
        }
    }


    state = true;
    return node;
}


ast_node*
parse_binary_expression(ast_node* left_operand, parser& parser, lexer& lxr) {

    parser_assert(lxr.current().kind == KIND_BINARY_EXPR_OPERATOR, "Expected binary operator.");
    parser_assert(left_operand != nullptr, "Null left operand passed.");


    if(lxr.current() == TOKEN_VALUE_ASSIGNMENT) {
        return parse_assign(left_operand, parser, lxr);
    }


    bool  state    = false;
    auto* binexpr  = new ast_binexpr();

    defer_if(!state, [&] {
        delete binexpr;
    });

    binexpr->_operator       = lxr.current().type;
    binexpr->left_op         = left_operand;
    binexpr->left_op->parent = binexpr;


    const size_t   src_pos = lxr.current().src_pos;
    const uint32_t line    = lxr.current().line;

    lxr.advance(1);
    binexpr->right_op = parse_expression(parser, lxr, true);
    if(binexpr->right_op == nullptr) {
        return nullptr;
    }


    binexpr->right_op->parent = binexpr;
    const auto right_t        = binexpr->right_op->type;

    if(!VALID_SUBEXPRESSION(right_t)) {
        lxr.raise_error("Unexpected expression following binary operator.", src_pos, line);
        return nullptr;
    }

    state = true;
    return binexpr;
}


ast_node*
parse_subscript(ast_node* operand, parser& parser, lexer& lxr) {

    parser_assert(lxr.current() == TOKEN_LSQUARE_BRACKET, "Expected '['.");
    parser_assert(operand != nullptr, "Null operand.");
    lxr.advance(1);


    const size_t   curr_pos = lxr.current().src_pos;
    const uint32_t line     = lxr.current().line;

    auto* node            = new ast_subscript();
    node->operand         = operand;
    node->operand->parent = node;
    node->value           = parse_expression(parser, lxr, true);

    bool state = false;
    defer_if(!state, [&] {
        delete node;
    });


    if(node->value == nullptr)
        return nullptr;

    if(!VALID_SUBEXPRESSION(node->value->type) || lxr.current() != TOKEN_RSQUARE_BRACKET) {
        lxr.raise_error("Invalid expression within subscript operator.", curr_pos, line);
        return nullptr;
    }


    lxr.advance(1);
    state = true;
    return node;
}