//
// Created by castle on 7/24/23.
//

#ifndef UPARSER_CTX_H
#define UPARSER_CTX_H

#include "build/location.hh"

namespace yy {
  class parser;
  class location;
}

#include <ranges>
#include <deque>
#include <stack>
#include <string>
#include <regex>
#include <unordered_map>
#include <utility>

#include "pattern_parser.h"

#include "runtime/runtime.h"

struct ulang_token {
  std::string type{};
  std::string str{};
};

struct ulang_grammar {
  struct token_rule {
    std::regex regex{};
  };
  struct non_terminal_node {
    enum {
      TERMINAL,
      NON_TERMINAL,
      SUB_NODES,
      SUB_OPTIONS,
      ZERO_OR_MORE,
    }                              type;
    std::string                    str;
    std::string pattern{};
    std::vector<non_terminal_node> sub_nodes{};
  };
  struct grammar_rule {
    non_terminal_node                     root_item{
      .type = non_terminal_node::SUB_OPTIONS,
      .sub_nodes = {},
    };
  };
  
  
  std::string name;
  std::unordered_map<std::string, token_rule> token_rules{};
  std::unordered_map<std::string, grammar_rule> grammar_rules{};
};

struct ulang_grammar_ctx {
  std::string grammar_name;
  ulang_grammar& grammar;
  
  std::deque<std::pair<yy::location, std::string>> substitutions{};
  
  ulang_grammar_ctx(std::string grammar_name, ulang_grammar& grammar)
    : grammar_name(std::move(grammar_name)), grammar(grammar) {}
  
private:
  std::deque<ulang_token> tokens{};
  
  std::string last_added_token{};
public:
  void add_token(const std::string& t) {
    //! LEXER
    last_added_token.append(t);
    std::string token_type;
    bool found = false;
    for (const auto &tr_pair: grammar.token_rules) {
      auto &[type, tr] = tr_pair;
      std::smatch match;
      if (std::regex_match(last_added_token, match, tr.regex)) {
        found = true;
        token_type = type;
        break;
      }
    }
    if (!found) {
      // TODO - Error TOKEN DOES NOT MATCH ANY RULE (yet)
      return;
    } else {
      std::string new_token = last_added_token;
      last_added_token.clear();
      if (tokens.empty() && token.type.empty()) {
        token = {
          .type = token_type,
          .str = new_token,
        };
      } else {
        tokens.push_back({
          .type = token_type,
          .str = new_token,
        });
      }
    }
  }
  void add_tokens(const std::vector<std::string>& ts) {
    for (auto &t: ts) {
      add_token(t);
    }
  }
  
  
private:
  ulang_token token{
    .type = "",
  };
  
  std::stack<std::vector<ulang_token>> parsed_tokens{{{}}};
  std::stack<std::vector<pattern_result>> result_stack{{{}}};
  
  void error(const std::string& msg) {
  
  }
  
  void next() {
    if (!token.type.empty()) {
      parsed_tokens.top().push_back(token);
    }
    if (tokens.empty()) {
      token = {
        .type = "EOI",
        .str = "EOI",
      };
    } else {
      token = tokens.front();
      tokens.pop_front();
    }
  }
  
  void backtrack() {
    if (!token.type.empty()) {
      tokens.push_front(token);
      token = {.type = ""};
    }
    while (!parsed_tokens.top().empty()) {
      tokens.push_front(parsed_tokens.top().back());
      parsed_tokens.top().pop_back();
    }
    next();
  }
  
  bool accept(const std::string& token_type) {
    if (token.type == token_type) {
      result_stack.top().push_back({.str = token.str});
      next();
      return true;
    }
    return false;
  }
  
  bool expect(const std::string& token_type) {
    if (accept(token_type)) {
      return true;
    }
    error("[expect] Unexpected token type");
  }
  
  bool handle_node(const ulang_grammar::non_terminal_node& node) {
    switch (node.type) {
      case ulang_grammar::non_terminal_node::TERMINAL:
        if (!accept(node.str)) return false;
        break;
      case ulang_grammar::non_terminal_node::NON_TERMINAL:
        if (!parse_non_terminal(node.str)) return false;
        break;
      case ulang_grammar::non_terminal_node::SUB_NODES:
        if (!handle_sub_nodes(node)) return false;
        break;
      case ulang_grammar::non_terminal_node::SUB_OPTIONS:
        if (!handle_sub_options(node)) return false;
        break;
      case ulang_grammar::non_terminal_node::ZERO_OR_MORE:
        if (!handle_zero_or_more(node)) return false;
        break;
    }
    return true;
  }
  
  bool handle_zero_or_more(const ulang_grammar::non_terminal_node& node) {
    std::size_t count = 0;
    while (handle_sub_options(node)) { count++; }
    if (count == 0) {
      result_stack.top().emplace_back();
    }
    return true;
  }
  bool handle_sub_options(const ulang_grammar::non_terminal_node& node) {
    bool r = false;
    for (const auto &n: node.sub_nodes) {
      if (handle_sub_nodes(n)) { r = true; break; }
    }
    return r;
  }
  
  bool handle_sub_nodes(const ulang_grammar::non_terminal_node& node) {
    result_stack.emplace();
    parsed_tokens.emplace();
    for (const auto &n: node.sub_nodes) {
      if (!handle_node(n)) {
        result_stack.pop();
        backtrack();
        parsed_tokens.pop();
        return false;
      }
    }
    auto parsed = parsed_tokens.top();
    parsed_tokens.pop();
    for (const auto &item: parsed) {
      parsed_tokens.top().push_back(item);
    }
    
    auto res = result_stack.top();
    result_stack.pop();
    // TODO - Process with pattern
    //if (result_stack.top().empty()) {
    //}
    if (node.pattern.empty()) {
      std::string s;
      for (const auto &_s: res) {
        s.append(_s.apply({}));
      }
      result_stack.top().emplace_back(s);
    } else {
      pattern_parser pparser{node.pattern};
      pattern_result p_res = pparser.parse(res);
      result_stack.top().push_back(p_res);
    //  int indent = 0;
    //  std::string p = node.pattern;
    //  for (std::size_t off = 0; off < p.size(); ++off) {
    //    if (p[off] == '$') {
    //      std::string num_s;
    //      off++;
    //      if (::isdigit(p[off])) {
    //        while (::isdigit(p[off])) {
    //          num_s += p[off];
    //          ++off;
    //        }
    //        if (!num_s.empty()) {
    //          std::string tmp = p.substr(0, off - num_s.size() - 1);
    //          std::size_t term_index = std::stol(num_s);
    //          std::string sub = res[term_index - 1];
    //          while (sub.ends_with('\n')) sub = sub.substr(0, sub.size() - 1);
    //          if (indent > 0) {
    //            std::string ind = "\n";
    //            for (int    i   = 0; i < indent; ++i) {
    //              if (tmp.empty() || tmp.back() == '\n') {
    //                sub = std::string {"\t"}.append(sub);
    //              }
    //              ind.append("\t");
    //            }
    //            sub             = regex_replace(sub, std::regex {
    //              "(\\n|\n)",
    //              std::regex_constants::ECMAScript
    //            }, ind);
    //          }
    //          tmp.append(sub);
    //          tmp.append(p.substr(off));
    //          p = tmp;
    //
    //          off += res[term_index - 1].size() - num_s.size() - 2;
    //        }
    //      } else if (p[off] == '[') {
    //        off++;
    //        std::vector<std::string> args{};
    //
    //        while (p[off] != ']') {
    //
    //        }
    //      }
    //    } else if (p[off] == '\\') {
    //      off++;
    //      switch (p[off]) {
    //        case 'n':
    //          p[off-1] = '\n';
    //          p.erase(p.begin()+off);
    //          if (indent > 0) {
    //            std::string ind = "";
    //            for (int i = 0; i < indent; ++i) { ind.append("\t"); }
    //            std::string tmp = p.substr(0,off);
    //            tmp.append(ind);
    //            tmp.append(p.substr(off));
    //            off += indent;
    //            p = tmp;
    //          }
    //          break;
    //      }
    //      off--;
    //    } else if (p[off] == '%') {
    //      off++;
    //      bool matches = false;
    //      switch (p[off]) {
    //        case 'i':
    //          off++;
    //          if (p[off] == '+') {
    //            indent++;
    //            matches = true;
    //          } else if (p[off] == '-') {
    //            indent--;
    //            matches = true;
    //          }
    //          if (matches) {
    //            p.erase(p.begin()+off);
    //            off--;
    //            p.erase(p.begin()+off);
    //          }
    //          break;
    //      }
    //      if (matches) {
    //        off--;
    //        p.erase(p.begin()+off);
    //      }
    //      off--;
    //    }
    //  }
    //  result_stack.top().push_back(p);
    }
    return true;
  }
  
  bool parse_non_terminal(const std::string& rule_name) {
    auto rule = grammar.grammar_rules[rule_name];
    auto r = handle_sub_options(rule.root_item);
    return r;
  }
  
  void tokenize(const std::string& str) {
    const char* s = str.c_str();
    std::string current;
    std::stack<ulang_token> matches{};
    for (std::size_t i = 0; i < str.size(); ++i) {
      current += s[i];
      
      bool found = false;
      for (const auto &token_type: this->grammar.token_rules) {
        std::smatch match;
        if (std::regex_match(current, match, token_type.second.regex)) {
          found = true;
          matches.push({
            .type = token_type.first,
            .str = current,
          });
          break;
        }
      }
      
      if (!found && !matches.empty()) {
        --i;
        auto t = matches.top();
        while (t.type.starts_with("_")) {
          matches.pop();
          if (!matches.empty()) {
            t = matches.top();
          } else {
            break;
          }
        }
        if (!matches.empty()) {
          tokens.push_back(matches.top());
        }
        
        current.clear();
        while (!matches.empty()) matches.pop();
      }
    }
    
    next();
  }
public:
  
  std::string parse(const std::string& str) {
    if (grammar_name == "default") return str;
    
    tokenize(str);
    
    parse_non_terminal("root");
    
    std::string s;
    for (const auto &item: result_stack.top()) {
      if (!item.str.empty()) {
        s = item.apply({}); //*** Parameters to 'root' rule
        break;
      }
    }
    
    return s;
  }
};

struct parser_ctx_t {
  explicit parser_ctx_t(std::string  str);
  ~parser_ctx_t();
  
  std::string str;
  
  void* lexer;
  yy::location* loc;
  yy::parser* parser;
  
  //cson_object* obj = new cson_object{};
  std::deque<ulang_token> token_list{};
  std::unordered_map<std::string, ulang_grammar> grammars{{ "default", ulang_grammar{
    .name = "default",
    .token_rules = {{"ANY", {std::regex(R"([^ \n\t\r\s]+)", std::regex_constants::ECMAScript)}}},
    .grammar_rules = {{"root", {
      .root_item = {
        .type = ulang_grammar::non_terminal_node::SUB_OPTIONS,
        .sub_nodes = {
          { .type = ulang_grammar::non_terminal_node::SUB_NODES,
            .sub_nodes = {
              { .type = ulang_grammar::non_terminal_node::TERMINAL,
                .str = "ANY",
              },
              { .type = ulang_grammar::non_terminal_node::ZERO_OR_MORE,
                .sub_nodes = {
                  { .type = ulang_grammar::non_terminal_node::SUB_NODES,
                    .sub_nodes = {
                      { .type = ulang_grammar::non_terminal_node::NON_TERMINAL,
                        .str = "root",
                      },}},}},},}},},}}}},}};
  
  std::stack<ulang_grammar_ctx> current_grammar{{ulang_grammar_ctx{
    "default", grammars["default"]
  }}};
  
  void push_grammar_ctx(const std::string& grammar) {
    current_grammar.emplace(
      grammar,
      grammars[grammar]
    );
  }
  
  void pop_grammar_ctx(yy::location l, yy::location w) {
    auto ctx = current_grammar.top();
    current_grammar.pop();
    std::string text = str.substr(l.begin.column, l.end.column - l.begin.column);
    std::string t;
    for (const auto &sub: ctx.substitutions) {
      auto &[p, s] = sub;
      t = text.substr(0, p.begin.column - l.begin.column);
      t.append(s);
      t.append(text.substr(p.end.column - l.begin.column));
      text = t;
    }
    
    std::string res = ctx.parse(text);
    current_grammar.top().substitutions.emplace_front(w, res); // Hopefully reverses the order
  }
  
  static inline bool is_terminal(const std::string& str) {
    return std::all_of(str.begin(), str.end(), [](char c){
      return c == '_' || c == '-' || ::isdigit(c) || ::isupper(c);
    });
    //return std::all_of(str.begin(), str.end(), ::isupper);
  }
  
  static inline std::string get_grammar_id(const std::string& grammar_tag) {
    auto s = grammar_tag.substr(grammar_tag.find_first_of(':')+1);
    s = s.substr(0, s.size()-1);
    if (s[0] == ' ') {
      s = s.substr(1);
    }
    if (s[s.size()-1] == ' ') {
      s = s.substr(0, s.size()-1);
    }
    return s;
  }
  static inline std::string get_grammar_tag_id(const std::string& grammar_tag) {
    std::string s = grammar_tag.substr(1);
    s = s.substr(0, grammar_tag.size()-3);
    if (s[0] == ' ') {
      s = s.substr(1);
    }
    if (s[s.size()-1] == ' ') {
      s = s.substr(0, s.size()-1);
    }
    return s;
  }
  
  std::string get_str(yy::location l) const {
    return str.substr(l.begin.column-1, l.end.column - l.begin.column);
  }
};

#endif //UPARSER_CTX_H
