//
// Created by Diago on 2024-07-23.
//

#include <parser.hpp>


std::string
var_t_to_string(const var_t type) { // TODO: maybe convert this enum to an X macro too?

    switch(type) {
        case VAR_NONE:     return "None";
        case VAR_U8:       return "U8";
        case VAR_I8:       return "I8";
        case VAR_U16:      return "U16";
        case VAR_I16:      return "I16";
        case VAR_U32:      return "U32";
        case VAR_I32:      return "I32";
        case VAR_U64:      return "U64";
        case VAR_I64:      return "I64";
        case VAR_F32:      return "F32";
        case VAR_F64:      return "F64";
        case VAR_BOOLEAN:  return "Boolean";
        case VAR_VOID:     return "Void";
        default:           return "Unknown";
    }
}

var_t
token_to_var_t(const token_t tok_t) {

    switch(tok_t) {
        case TOKEN_KW_I8:   return VAR_I8;
        case TOKEN_KW_U8:   return VAR_U8;
        case TOKEN_KW_I16:  return VAR_I16;
        case TOKEN_KW_U16:  return VAR_U16;
        case TOKEN_KW_I32:  return VAR_I32;
        case TOKEN_KW_U32:  return VAR_U32;
        case TOKEN_KW_I64:  return VAR_I64;
        case TOKEN_KW_U64:  return VAR_U64;
        case TOKEN_KW_F32:  return VAR_F32;
        case TOKEN_KW_F64:  return VAR_F64;
        case TOKEN_KW_BOOL: return VAR_BOOLEAN;
        default:            return VAR_NONE;
    }
}

uint16_t
precedence_of(const token_t _operator) {

    switch(_operator) {

        case TOKEN_CONDITIONAL_AND:    return 13;
        case TOKEN_CONDITIONAL_OR:     return 12;
        case TOKEN_MUL:
        case TOKEN_DIV:
        case TOKEN_MOD:                return 8;
        case TOKEN_PLUS:
        case TOKEN_SUB:                return 7;
        case TOKEN_BITWISE_LSHIFT:
        case TOKEN_BITWISE_RSHIFT:     return 6;
        case TOKEN_COMP_GTE:
        case TOKEN_COMP_GT:
        case TOKEN_COMP_LTE:
        case TOKEN_COMP_LT:            return 5;
        case TOKEN_COMP_EQUALS:
        case TOKEN_COMP_NOT_EQUALS:    return 4;
        case TOKEN_BITWISE_AND:        return 3;
        case TOKEN_BITWISE_XOR_OR_PTR: return 2;
        case TOKEN_BITWISE_OR:         return 1;
        case TOKEN_VALUE_ASSIGNMENT:
        case TOKEN_PLUSEQ:
        case TOKEN_SUBEQ:
        case TOKEN_MULEQ:
        case TOKEN_DIVEQ:
        case TOKEN_MODEQ:
        case TOKEN_BITWISE_LSHIFTEQ:
        case TOKEN_BITWISE_RSHIFTEQ:
        case TOKEN_BITWISE_ANDEQ:
        case TOKEN_BITWISE_OREQ:
        case TOKEN_BITWISE_XOREQ:      return 0;

        default:
            panic("predence_of: default case reached");
    }
}

uint16_t
var_t_to_size_bytes(const var_t type) {

    // Assumes type is not a pointer.

    switch(type) {
        case VAR_U8:
        case VAR_I8:      return 1;
        case VAR_U16:
        case VAR_I16:     return 2;
        case VAR_U32:
        case VAR_I32:     return 4;
        case VAR_U64:
        case VAR_I64:     return 8;
        case VAR_F32:     return 4;
        case VAR_F64:     return 8;
        case VAR_BOOLEAN: return 1;
        default:
            panic("var_t_to_size_bytes: non size-convertible var_t passed as argument.");
    }
}

std::optional<char>
get_escaped_char_via_real(const char real) {

    switch(real) {
        case 'n':  return '\n';
        case 'b':  return '\b';
        case 'a':  return '\a';
        case 'r':  return '\r';
        case '\'': return '\'';
        case '\\': return '\\';
        case '"':  return '"';
        case 't':  return '\t';
        case '0':  return '\0';
        default:   return std::nullopt;
    }
}

std::optional<std::string>
remove_escaped_chars(const std::string_view& str) {

    std::string buffer;

    for(size_t i = 0; i < str.size(); i++) {
        if(str[i] == '\\') {
            if(i + 1 >= str.size()) {
                return std::nullopt;
            }

            if(const auto escaped = get_escaped_char_via_real(str[i + 1])) {
                buffer += *escaped;
                i++;
            } else {
                return std::nullopt;
            }
        }

        else {
            buffer += str[i];
        }
    }

    return buffer;
}

std::optional<std::string>
get_actual_string(const std::string_view& str) {

    if(auto actual = remove_escaped_chars(str)) {
        if(actual->size() < 3)
            return std::nullopt;

        actual->erase(0,1);
        actual->pop_back();
        return *actual;
    }

    return std::nullopt;
}

std::optional<char>
get_actual_char(const std::string_view& str) {

    if(const auto actual = remove_escaped_chars(str)) {
        if(actual->size() < 3)
            return std::nullopt;

        return (*actual)[1];
    }

    return std::nullopt;
}

std::optional<size_t>
lexer_token_lit_to_int(const token& tok) {

    size_t val = 0;

    if(tok == TOKEN_INTEGER_LITERAL) {
        try {
            val = std::stoull(std::string(tok.value));
        } catch(...) {
            return std::nullopt;
        }
    } else if(tok == TOKEN_CHARACTER_LITERAL) {
        if(const auto actual = get_actual_char(tok.value)) {
            val = static_cast<size_t>(*actual);
        } else {
            return std::nullopt;
        }
    } else {
        return std::nullopt; // Cannot be converted to an integer
    }

    return val;
}

std::string
lexer_token_type_to_string(const token_t type) {

#define X(NAME) case TOKEN_##NAME: return #NAME;
    switch(type) {
        TOKEN_LIST
        default: return "Unknown."; // failsafe
    }
#undef X
}

std::string
lexer_token_kind_to_string(const token_kind kind) {

#define X(NAME) case KIND_##NAME: return #NAME;
    switch(kind) {
        TOKEN_KIND_LIST
        default: return "Unknown"; // failsafe
    }
#undef X
}

void
lexer_display_token_data(const token& tok) {
    print(
        "Value: {}\nType: {}\nKind: {}\nFile Pos Index: {}\nLine Number: {}\n",
        tok.value,
        lexer_token_type_to_string(tok.type),
        lexer_token_kind_to_string(tok.kind),
        tok.src_pos,
        tok.line
    );
}

std::vector<std::string>
split_struct_member_path(std::string path) {

    if(path.empty() || path.front() != '.') {
        panic(fmt("split_struct_member_path: malformed path value: {}", path));
    }

    path.erase(0, 1);
    path += '.';

    std::vector<std::string> chunks;
    size_t start = 0;
    size_t dot   = path.find('.');

    while(dot != std::string::npos) {
        if(dot > start) {
            chunks.emplace_back(path.substr(start, dot - start));
        }

        start = dot + 1;
        dot = path.find('.', start);
    }

    return chunks;
}