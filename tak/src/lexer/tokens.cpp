//
// Created by Diago on 2024-07-03.
//

#include <lexer.hpp>


void
lexer_token_skip(Lexer& lxr) {

    auto &[src, index, curr_line, _current, _] = lxr;

    if(index >= src.size()) {
        _current = Token{TOKEN_END_OF_FILE, KIND_UNSPECIFIC, src.size() - 1, "\\0"};
    } else {
        _current.type = TOKEN_NONE;
        lxr.advance_char(1);
    }
}


void
lexer_token_newline(Lexer& lxr) {

    auto &[src, index, curr_line, _current, _] = lxr;

    if(index >= src.size()) {
        _current = Token{TOKEN_END_OF_FILE, KIND_UNSPECIFIC, src.size() - 1, "\\0"};
    } else {
        _current.type = TOKEN_NONE;
        lxr.advance_line();
        lxr.advance_char(1);
    }
}


void
lexer_token_semicolon(Lexer& lxr) {

    auto &[src, index, curr_line, _current, _] = lxr;

    if(index >= src.size()) {
        _current = Token{TOKEN_END_OF_FILE, KIND_UNSPECIFIC, src.size() - 1, "\\0"};
    } else {
        _current = Token{TOKEN_SEMICOLON, KIND_PUNCTUATOR, index, {&src[index], 1}};
        lxr.advance_char(1);
    }
}


void
lexer_token_lparen(Lexer &lxr) {

    auto &[src, index, curr_line, _current, _] = lxr;

    if(index >= src.size()) {
        _current = Token{TOKEN_END_OF_FILE, KIND_UNSPECIFIC, src.size() - 1, "\\0"};
    } else {
        _current = Token{TOKEN_LPAREN, KIND_PUNCTUATOR, index, {&src[index], 1}};
        lxr.advance_char(1);
    }
}


void
lexer_token_rparen(Lexer &lxr) {

    auto &[src, index, curr_line, _current, _] = lxr;

    if(index >= src.size()) {
        _current = Token{TOKEN_END_OF_FILE, KIND_UNSPECIFIC, src.size() - 1, "\\0"};
    } else {
        _current = Token{TOKEN_RPAREN, KIND_PUNCTUATOR, index, {&src[index], 1}};
        lxr.advance_char(1);
    }
}


void
lexer_token_lbrace(Lexer &lxr) {

    auto &[src, index, curr_line, _current, _] = lxr;

    if(index >= src.size()) {
        _current = Token{TOKEN_END_OF_FILE, KIND_UNSPECIFIC, src.size() - 1, "\\0"};
    } else {
        _current = Token{TOKEN_LBRACE, KIND_PUNCTUATOR, index, {&src[index], 1}};
        lxr.advance_char(1);
    }
}


void
lexer_token_rbrace(Lexer &lxr) {

    auto &[src, index, curr_line, _current, _] = lxr;

    if(index >= src.size()) {
        _current = Token{TOKEN_END_OF_FILE, KIND_UNSPECIFIC, src.size() - 1, "\\0"};
    } else {
        _current = Token{TOKEN_RBRACE, KIND_PUNCTUATOR, index, {&src[index], 1}};
        lxr.advance_char(1);
    }
}


void
lexer_token_comma(Lexer &lxr) {

    auto &[src, index, curr_line, _current, _] = lxr;

    if(index >= src.size()) {
        _current = Token{TOKEN_END_OF_FILE, KIND_UNSPECIFIC, src.size() - 1, "\\0"};
    } else {
        _current = Token{TOKEN_COMMA, KIND_PUNCTUATOR, index, {&src[index], 1}};
        lxr.advance_char(1);
    }
}


void
lexer_token_hyphen(Lexer &lxr) {

    auto &[src, index, curr_line, _current, _] = lxr;

    if(index >= src.size()) {
        _current = Token{TOKEN_END_OF_FILE, KIND_UNSPECIFIC, src.size() - 1, "\\0"};
        return;
    }


    switch(lxr.peek_char()) {
        case '=':
            _current = Token{TOKEN_SUBEQ, KIND_BINARY_EXPR_OPERATOR, index, {&src[index], 2}};
            lxr.advance_char(2);
            break;

        case '>':
            _current = Token{TOKEN_ARROW, KIND_UNSPECIFIC, index, {&src[index], 2}};
            lxr.advance_char(2);
            break;

        case '-':
            _current = Token{TOKEN_DECREMENT, KIND_UNARY_EXPR_OPERATOR, index, {&src[index], 2}};
            lxr.advance_char(2);
            break;

        default:
            _current = Token{TOKEN_SUB, KIND_BINARY_EXPR_OPERATOR, index, {&src[index], 1}};
            lxr.advance_char(1);
            break;
    }
}


void
lexer_token_plus(Lexer &lxr) {

    auto &[src, index, curr_line, _current, _] = lxr;

    if(index >= src.size()) {
        _current = Token{TOKEN_END_OF_FILE, KIND_UNSPECIFIC, src.size() - 1, "\\0"};
    }

    switch(lxr.peek_char()) {
        case '=':
            _current = Token{TOKEN_PLUSEQ, KIND_BINARY_EXPR_OPERATOR, index, {&src[index], 2}};
            lxr.advance_char(2);
            break;

        case '+':
            _current = Token{TOKEN_INCREMENT, KIND_UNARY_EXPR_OPERATOR, index, {&src[index], 2}};
            lxr.advance_char(2);
            break;

        default:
            _current = Token{TOKEN_PLUS, KIND_BINARY_EXPR_OPERATOR, index, {&src[index], 1}};
            lxr.advance_char(1);
            break;
    }
}


void
lexer_token_asterisk(Lexer &lxr) {

    auto &[src, index, curr_line, _current, _] = lxr;

    if(index >= src.size()) {
        _current = Token{TOKEN_END_OF_FILE, KIND_UNSPECIFIC, src.size() - 1, "\\0"};
    }

    else if(lxr.peek_char() == '=') {
        _current = Token{TOKEN_MULEQ, KIND_BINARY_EXPR_OPERATOR, index, {&src[index], 2}};
        lxr.advance_char(2);
    }

    else {
        _current = Token{TOKEN_MUL, KIND_BINARY_EXPR_OPERATOR, index, {&src[index], 1}};
        lxr.advance_char(1);
    }
}


void
lexer_token_fwdslash(Lexer &lxr) {

    auto &[src, index, curr_line, _current, _] = lxr;

    if(index >= src.size()) {
        _current = Token{TOKEN_END_OF_FILE, KIND_UNSPECIFIC, src.size() - 1, "\\0"};
        return;
    }


    switch(lxr.peek_char()) {
        case '/':
            while(lxr.current_char() != '\0' && lxr.current_char() != '\n') {
                lxr.advance_char(1);
            }

            _current.type = TOKEN_NONE;
            break;

        case '*':
            while(true) {
                if(lxr.current_char() == '\0') {
                    break;
                }

                if(lxr.current_char() == '*' && lxr.peek_char() == '/') {
                    lxr.advance_char(2);
                    break;
                }

                if(lxr.current_char() == '\n') {
                    lxr.advance_line();
                }

                lxr.advance_char(1);
            }


            if(lxr.current_char() == '\0') {
                _current = Token{TOKEN_END_OF_FILE, KIND_UNSPECIFIC, src.size() - 1, "\\0"};
            } else {
                _current.type = TOKEN_NONE;
            }

            break;

        case '=':
            _current = Token{TOKEN_DIVEQ, KIND_BINARY_EXPR_OPERATOR, index, {&src[index], 2}};
            lxr.advance_char(2);
            break;

        default:
            _current = Token{TOKEN_DIV, KIND_BINARY_EXPR_OPERATOR, index, {&src[index], 1}};
            lxr.advance_char(1);
            break;
    }
}


void
lexer_token_percent(Lexer &lxr) {

    auto &[src, index, curr_line, _current, _] = lxr;

    if(index >= src.size()) {
        _current = Token{TOKEN_END_OF_FILE, KIND_UNSPECIFIC, src.size() - 1, "\\0"};
    }

    else if(lxr.peek_char() == '=') {
        _current = Token{TOKEN_MODEQ, KIND_BINARY_EXPR_OPERATOR, index, {&src[index], 2}};
        lxr.advance_char(2);
    }

    else {
        _current = Token{TOKEN_MOD, KIND_BINARY_EXPR_OPERATOR, index, {&src[index], 1}};
        lxr.advance_char(1);
    }
}


void
lexer_token_equals(Lexer &lxr) {

    auto &[src, index, curr_line, _current, _] = lxr;

    if(index >= src.size()) {
        _current = Token{TOKEN_END_OF_FILE, KIND_UNSPECIFIC, src.size() - 1, "\\0"};
    }

    else if(lxr.peek_char() == '=') {
        _current = Token{TOKEN_COMP_EQUALS, KIND_BINARY_EXPR_OPERATOR, index, {&src[index], 2}};
        lxr.advance_char(2);
    }

    else {
        _current = Token{TOKEN_VALUE_ASSIGNMENT, KIND_BINARY_EXPR_OPERATOR, index, {&src[index], 1}};
        lxr.advance_char(1);
    }
}


void
lexer_token_lessthan(Lexer &lxr) {

    auto &[src, index, curr_line, _current, _] = lxr;

    if(index >= src.size()) {
        _current = Token{TOKEN_END_OF_FILE, KIND_UNSPECIFIC, src.size() - 1, "\\0"};
        return;
    }

    const size_t start = index;

    switch(lxr.peek_char()) {
        case '=':
            _current = Token{TOKEN_COMP_LTE, KIND_BINARY_EXPR_OPERATOR, index, {&src[index], 2}};
            lxr.advance_char(2);
            break;

        case '<':
            lxr.advance_char(1);
            if(lxr.peek_char() == '=') {
                _current = Token{TOKEN_BITWISE_LSHIFTEQ, KIND_BINARY_EXPR_OPERATOR, start, {&src[start], 3}};
                lxr.advance_char(2);
            } else {
                _current = Token{TOKEN_BITWISE_LSHIFT, KIND_BINARY_EXPR_OPERATOR, start, {&src[start], 2}};
                lxr.advance_char(1);
            }
            break;

        default:
            _current = Token{TOKEN_COMP_LT, KIND_BINARY_EXPR_OPERATOR, index, {&src[index], 1}};
            lxr.advance_char(1);
            break;
    }
}


void
lexer_token_greaterthan(Lexer &lxr) {

    auto &[src, index, curr_line, _current, _] = lxr;

    if(index >= src.size()) {
        _current = Token{TOKEN_END_OF_FILE, KIND_UNSPECIFIC, src.size() - 1, "\\0"};
        return;
    }


    const size_t start = index;

    switch(lxr.peek_char()) {
        case '=':
            _current = Token{TOKEN_COMP_GTE, KIND_BINARY_EXPR_OPERATOR, index, {&src[index], 2}};
            lxr.advance_char(2);
            break;

        case '>':
            lxr.advance_char(1);
            if(lxr.peek_char() == '=') {
                _current = Token{TOKEN_BITWISE_RSHIFTEQ, KIND_BINARY_EXPR_OPERATOR, start, {&src[start], 3}};
                lxr.advance_char(2);
            } else {
                _current = Token{TOKEN_BITWISE_RSHIFT, KIND_BINARY_EXPR_OPERATOR, start, {&src[start], 2}};
                lxr.advance_char(1);
            }
            break;

        default:
            _current = Token{TOKEN_COMP_GT, KIND_BINARY_EXPR_OPERATOR, index, {&src[index], 1}};
            lxr.advance_char(1);
            break;
    }
}


void
lexer_token_ampersand(Lexer &lxr) {

    auto &[src, index, curr_line, _current, _] = lxr;

    if(index >= src.size()) {
        _current = Token{TOKEN_END_OF_FILE, KIND_UNSPECIFIC, src.size() - 1, "\\0"};
        return;
    }


    switch(lxr.peek_char()) {
        case '&':
            _current = Token{TOKEN_CONDITIONAL_AND, KIND_BINARY_EXPR_OPERATOR, index, {&src[index], 2}};
            lxr.advance_char(2);
            break;

        case '=':
            _current = Token{TOKEN_BITWISE_ANDEQ, KIND_BINARY_EXPR_OPERATOR, index, {&src[index], 2}};
            lxr.advance_char(2);
            break;

        default:
            _current = Token{TOKEN_BITWISE_AND, KIND_BINARY_EXPR_OPERATOR, index, {&src[index], 1}}; // can also mean address-of!
            lxr.advance_char(1);
            break;
    }
}


void
lexer_token_verticalline(Lexer &lxr) {

    auto &[src, index, curr_line, _current, _] = lxr;

    if(index >= src.size()) {
        _current = Token{TOKEN_END_OF_FILE, KIND_UNSPECIFIC, src.size() - 1, "\\0"};
        return;
    }


    switch(lxr.peek_char()) {
        case '|':
            _current = Token{TOKEN_CONDITIONAL_OR, KIND_BINARY_EXPR_OPERATOR, index, {&src[index], 2}};
            lxr.advance_char(2);
            break;

        case '=':
            _current = Token{TOKEN_BITWISE_OREQ, KIND_BINARY_EXPR_OPERATOR, index, {&src[index], 2}};
            lxr.advance_char(2);
            break;

        default:
            _current = Token{TOKEN_BITWISE_OR, KIND_BINARY_EXPR_OPERATOR, index, {&src[index], 1}};
            lxr.advance_char(1);
            break;
    }
}


void
lexer_token_exclamation(Lexer &lxr) {

    auto &[src, index, curr_line, _current, _] = lxr;

    if(index >= src.size()) {
        _current = Token{TOKEN_END_OF_FILE, KIND_UNSPECIFIC, src.size() - 1, "\\0"};
    }

    else if(lxr.peek_char() == '=') {
        _current = Token{TOKEN_COMP_NOT_EQUALS, KIND_BINARY_EXPR_OPERATOR, index, {&src[index], 2}};
        lxr.advance_char(2);
    }

    else {
        _current = Token{TOKEN_CONDITIONAL_NOT, KIND_UNARY_EXPR_OPERATOR, index, {&src[index], 1}};
        lxr.advance_char(1);
    }
}


void
lexer_token_tilde(Lexer &lxr) {

    auto &[src, index, curr_line, _current, _] = lxr;

    if(index >= src.size()) {
        _current = Token{TOKEN_END_OF_FILE, KIND_UNSPECIFIC, src.size() - 1, "\\0"};
    } else {
        _current = Token{TOKEN_BITWISE_NOT, KIND_UNARY_EXPR_OPERATOR, index, {&src[index],1}};
        lxr.advance_char(1);
    }
}


void
lexer_token_uparrow(Lexer &lxr) {

    auto &[src, index, curr_line, _current, _] = lxr;

    if(index >= src.size()) {
        _current = Token{TOKEN_END_OF_FILE, KIND_UNSPECIFIC, src.size() - 1, "\\0"};
    }

    else if(lxr.peek_char() == '=') {
        _current = Token{TOKEN_BITWISE_XOREQ, KIND_BINARY_EXPR_OPERATOR, index, {&src[index],2}};
        lxr.advance_char(2);
    }

    else {
        _current = Token{TOKEN_BITWISE_XOR_OR_PTR, KIND_BINARY_EXPR_OPERATOR, index, {&src[index],1}};
        lxr.advance_char(1);
    }
}


void
lexer_token_quote(Lexer &lxr) {

    auto &[src, index, curr_line, _current, _] = lxr;

    if(index >= src.size()) {
        _current = Token{TOKEN_END_OF_FILE, KIND_UNSPECIFIC, src.size() - 1, "\\0"};
        return;
    }


    const char opening_quote = lxr.current_char();
    size_t     string_start  = index;


    lxr.advance_char(1);
    while(true) {

        if(lxr.current_char() == '\0') {
            _current = Token{TOKEN_ILLEGAL, KIND_UNSPECIFIC, src.size() - 1, "\\0"};
            break;
        }

        if(lxr.current_char() == opening_quote) {
            _current = Token{TOKEN_STRING_LITERAL, KIND_LITERAL, index, {&src[string_start], (index - string_start) + 1}};
            lxr.advance_char(1);
            break;
        }

        if(lxr.current_char() == '\\' && lxr.peek_char() == opening_quote) {
            lxr.advance_char(2);
        }

        else {
            lxr.advance_char(1);
        }
    }
}


void
lexer_token_singlequote(Lexer &lxr) {

    auto &[src, index, curr_line, _current, _] = lxr;
    const size_t start = index;

    lxr.advance_char(1);

    if(lxr.current_char() == '\\') {          // check for escaped single-quote
        lxr.advance_char(2);
    } else if(lxr.current_char() == '\'') {   // empty literal
        lxr.advance_char(1);
        _current = Token{TOKEN_CHARACTER_LITERAL, KIND_LITERAL, start, {&src[start], index - start}};
        return;
    } else {
        lxr.advance_char(1);
    }


    switch(lxr.current_char()) {
        case '\0':
            _current = Token{TOKEN_ILLEGAL, KIND_UNSPECIFIC, src.size() - 1, "\\0"};
            break;

        case '\'':
            lxr.advance_char(1);
            _current = Token{TOKEN_CHARACTER_LITERAL, KIND_LITERAL, start, {&src[start], index - start}};
            break;

        default:
            _current = Token{TOKEN_ILLEGAL, KIND_UNSPECIFIC, start, {&src[start], index - start}};
            break;
    }
}


void
lexer_token_lsquarebracket(Lexer &lxr) {

    auto &[src, index, curr_line, _current, _] = lxr;

    if(index >= src.size()) {
        _current = Token{TOKEN_END_OF_FILE, KIND_UNSPECIFIC, src.size() - 1, "\\0"};
    } else {
        _current = Token{TOKEN_LSQUARE_BRACKET, KIND_PUNCTUATOR, index, {&src[index], 1}};
        lxr.advance_char(1);
    }
}


void
lexer_token_rsquarebracket(Lexer &lxr) {

    auto &[src, index, curr_line, _current, _] = lxr;

    if(index >= src.size()) {
        _current = Token{TOKEN_END_OF_FILE, KIND_UNSPECIFIC, src.size() - 1, "\\0"};
    } else {
        _current = Token{TOKEN_RSQUARE_BRACKET, KIND_PUNCTUATOR, index, {&src[index], 1}};
        lxr.advance_char(1);
    }
}


void
lexer_token_questionmark(Lexer &lxr) {

    auto &[src, index, curr_line, _current, _] = lxr;

    if(index >= src.size()) {
        _current = Token{TOKEN_END_OF_FILE, KIND_UNSPECIFIC, src.size() - 1, "\\0"};
    } else {
        _current = Token{TOKEN_QUESTION_MARK, KIND_PUNCTUATOR, index, {&src[index], 1}};
        lxr.advance_char(1);
    }
}


void
lexer_token_colon(Lexer &lxr) {

    auto &[src, index, curr_line, _current, _] = lxr;

    if(index >= src.size()) {
        _current = Token{TOKEN_END_OF_FILE, KIND_UNSPECIFIC, src.size() - 1, "\\0"};
    }

    else if(lxr.peek_char() == ':') {
        _current = Token{TOKEN_CONST_TYPE_ASSIGNMENT, KIND_BINARY_EXPR_OPERATOR, index, {&src[index], 2}};
        lxr.advance_char(2);
    }

    else {
        _current = Token{TOKEN_TYPE_ASSIGNMENT, KIND_BINARY_EXPR_OPERATOR, index, {&src[index], 1}};
        lxr.advance_char(1);
    }
}

void
lexer_token_dot(Lexer &lxr) {

    auto &[src, index, curr_line, _current, _] = lxr;

    if(index >= src.size()) {
        _current = Token{TOKEN_END_OF_FILE, KIND_UNSPECIFIC, src.size() - 1, "\\0"};
    } else {
        _current = Token{TOKEN_DOT, KIND_PUNCTUATOR, index, {&src[index], 1}};
        lxr.advance_char(1);
    }
}

void
lexer_token_backslash(Lexer& lxr) {

    auto &[src, index, curr_line, _current, _] = lxr;

    if(index >= src.size()) {
        _current = Token{TOKEN_END_OF_FILE, KIND_UNSPECIFIC, src.size() - 1, "\\0"};
    } else {
        _current = Token{TOKEN_NAMESPACE_ACCESS, KIND_UNSPECIFIC, index, {&src[index], 1}};
        lxr.advance_char(1);
    }
}

void
lexer_token_null(Lexer& lxr) {
    lxr.current_ = Token{TOKEN_END_OF_FILE, KIND_UNSPECIFIC, lxr.src_.size() - 1, "\\0"};
}

void lexer_token_at(Lexer &lxr) {

    auto &[src, index, curr_line, _current, _] = lxr;

    if(index >= src.size()) {
        _current = Token{TOKEN_END_OF_FILE, KIND_UNSPECIFIC, src.size() - 1, "\\0"};
    } else {
        _current = Token{TOKEN_AT, KIND_PUNCTUATOR, index, {&src[index], 1}};
        lxr.advance_char(1);
    }
}


//
// We can leave the following tokens as "illegal" for now.
// can change easily if we decide to do anything with these characters.
// for now, they are unused and don't do anything.
//

void
lexer_token_backtick(Lexer& lxr) {

    auto &[src, index, curr_line, _current, _] = lxr;

    if(index >= src.size()) {
        _current = Token{TOKEN_END_OF_FILE, KIND_UNSPECIFIC, src.size() - 1, "\\0"};
    } else {
        _current = Token{TOKEN_ILLEGAL, KIND_PUNCTUATOR, index, {&src[index], 1}};
        lxr.advance_char(1);
    }
}

void
lexer_token_pound(Lexer &lxr) {

    auto &[src, index, curr_line, _current, _] = lxr;

    if(index >= src.size()) {
        _current = Token{TOKEN_END_OF_FILE, KIND_UNSPECIFIC, src.size() - 1, "\\0"};
    } else {
        _current = Token{TOKEN_ILLEGAL, KIND_PUNCTUATOR, index, {&src[index], 1}};
        lxr.advance_char(1);
    }
}