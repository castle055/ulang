//
// Created by castle on 7/24/23.
//

#ifndef UPARSER_CTX_H
#define UPARSER_CTX_H

namespace yy {
  class parser;
  class location;
}

#include <deque>
#include <stack>
#include <string>
#include <regex>
#include <unordered_map>
#include <utility>

struct ulang_token {
  std::string str{};
};

struct ulang_grammar {
  struct token_rule {
    std::regex regex{};
  };
  struct grammar_rule {
    std::vector<std::vector<std::string>> options{};
    
    std::vector<std::vector<std::string>> patterns{};
    std::vector<std::unordered_map<std::string, std::size_t>> index_maps{};
  };
  
  std::string name;
  std::unordered_map<std::string, token_rule> token_rules{};
  std::unordered_map<std::string, grammar_rule> grammar_rules{};
};

struct ulang_grammar_ctx {
  std::string grammar_name;
  ulang_grammar& grammar;
  
  ulang_grammar_ctx(std::string grammar_name, ulang_grammar& grammar)
    : grammar_name(std::move(grammar_name)), grammar(grammar) {}
  
  std::stack<std::string> rule_stack{{"root"}};
  std::stack<std::vector<std::pair<std::string, std::string>>> accepted_tokens{{{},{}}};
  std::stack<std::vector<std::vector<std::string>>> result_tokens{{{},{}}};
  
  std::stack<unsigned long> current_option{{0}};
  std::stack<unsigned long> current_term{{0}};
  
private:
  std::size_t last_added_count = 0;
  void rule_complete() {
    auto tokens = accepted_tokens.top();
    std::string rule_name = rule_stack.top();
    auto rule = grammar.grammar_rules[rule_name];
    
    accepted_tokens.pop();
    for (const auto &item: tokens) {
      accepted_tokens.top().push_back(item);
    }
    
    // Compute Result
    std::vector<std::string> result{};
    for (const auto &t: rule.patterns[current_option.top()]) {
      if (t.starts_with('$')) {
        if (t.ends_with(']')) {
          std::size_t token_index_start = t.find_first_of('[');
          std::size_t token_index_end = t.find_first_of(']');
          std::string name = t.substr(1, token_index_start-1);
          std::size_t token_index = std::stoul(t.substr(token_index_start+1, token_index_end-token_index_start));
          std::size_t term_index = rule.index_maps[current_option.top()][name];
          result.push_back(result_tokens.top()[term_index][token_index]);
          last_added_count = 1;
        } else {
          std::size_t term_index = rule.index_maps[current_option.top()][t.substr(1)];
          last_added_count = result_tokens.top()[term_index].size();
          for (const auto &item: result_tokens.top()[term_index]) {
            result.push_back(item);
          }
        }
      } else {
        result.push_back(t);
      }
    }
    result_tokens.pop();
    result_tokens.top().push_back(result);
    
    current_term.pop();
    current_option.pop();
    rule_stack.pop();
  }
public:
  
  int feed_token(const std::string& token_type, const std::string& token, const std::string& next_token_type, const std::string& next_token) {
    std::string rule_name = rule_stack.top();
    auto rule = grammar.grammar_rules[rule_name];
    //! PARSER
    std::string current = rule.options[current_option.top()][current_term.top()];
    if (current.contains(':')) {
      current = current.substr(0, current.find_first_of(':'));
    }
    bool is_leaf;
    if (grammar.grammar_rules.contains(current)) {
      is_leaf = false;
    } else if (grammar.token_rules.contains(current)) {
      is_leaf = true;
    } else {
      // TODO - Error GRAMMAR USES RULE THAT DOES NOT EXIST
      return -1;
    }
    
    if (is_leaf) {
      if (token_type == current) {
        current_term.top()++;
        accepted_tokens.top().emplace_back(token_type, token);
        result_tokens.top().push_back({{token}});
      } else {
        current_option.top()++;
        if (current_option.top() >= rule.options.size()) {
          return -1;
        }
        current_term.top() = 0;
        
        auto tokens = accepted_tokens.top();
        accepted_tokens.top().clear();
        result_tokens.top().clear();
        auto t = tokens.begin();
        for (; std::next(t) != tokens.end(); t++) {
          auto next = std::next(t);
          if (feed_token(t->first, t->second, next->first, next->second) == -1) return -1;
        }
        if (feed_token(t->first, t->second, token_type, token) == -1) return -1;
        if (feed_token(token_type, token, next_token_type, next_token) == -1) return -1;
      }
    } else {
      accepted_tokens.emplace();
      rule_stack.emplace(current);
      current_option.push(0);
      current_term.push(0);
      result_tokens.emplace();
      if (feed_token(token_type, token, next_token_type, next_token) == -1) {
        rule_stack.pop();
        current_term.pop();
        current_option.pop();
        current_option.top()++;
        if (current_option.top() >= rule.options.size()) {
          return -1;
        }
        current_term.top() = 0;
        
        auto sub_tokens = accepted_tokens.top();
        accepted_tokens.pop();
        auto tokens = accepted_tokens.top();
        accepted_tokens.top().clear();
        result_tokens.pop();
        auto t = tokens.begin();
        for (; std::next(t) != tokens.end(); t++) {
          auto next = std::next(t);
          if (feed_token(t->first, t->second, next->first, next->second) == -1) return -1;
        }
        if (feed_token(t->first, t->second, sub_tokens[0].first, sub_tokens[0].second) == -1) return -1;
        
        t = sub_tokens.begin();
        for (; std::next(t) != sub_tokens.end(); t++) {
          auto next = std::next(t);
          if (feed_token(t->first, t->second, next->first, next->second) == -1) return -1;
        }
        if (feed_token(t->first, t->second, token_type, token) == -1) return -1;
        if (feed_token(token_type, token, next_token_type, next_token) == -1) return -1;
      } else {
        current_term.top()++;
      }
    }
    
    if (current_term.top() >= rule.options[current_option.top()].size()) {
      auto tokens = accepted_tokens.top();
      
      if (current_option.top() < rule.options.size()-1 && !next_token.empty()) {
        accepted_tokens.top().clear();
        current_term.top() = 0;
        current_option.top()++;
        
        rule_stack.push(rule_stack.top());
        current_term.push(current_term.top());
        current_option.push(current_option.top());
        result_tokens.emplace();
        result_tokens.emplace();
        bool found = true;
        auto t     = tokens.begin();
        for (; found && std::next(t) != tokens.end(); t++) {
          auto next = std::next(t);
          if (feed_token(t->first, t->second, next->first, next->second) != 0) {
            found = false;
          } else if(current_term.top() >= rule.options[current_option.top()].size()-1) {
            found = false;
            break;
          }
        }
        if (found && feed_token(t->first, t->second, "", "") != 0) {
          found = false;
        }
        if (found && feed_token(next_token_type, next_token, "", "") == 0) {
          // ^ weird but this way feed_token() is not called if found == false
          auto res = result_tokens.top();
          if (!res.empty()) {
            for (std::size_t i = 0; i < last_added_count; ++i) {
              res[res.size()-1].pop_back();
            }
            if (res[res.size()-1].empty()) {
              res.pop_back();
            }
          }
          current_term.top()--;
          accepted_tokens.top().pop_back();
          result_tokens.pop();
          result_tokens.top() = res;
        } else {
          result_tokens.pop();
          rule_complete();
        }
      } else {
        rule_complete();
      }
    }
    
    return 0;
  }
  
  std::pair<std::string, std::string> previous_token{"",""};
  void feed_raw_token(const std::string& token) {
    //! LEXER
    std::string token_type;
    bool found = false;
    for (const auto &tr_pair: grammar.token_rules) {
      auto &[type, tr] = tr_pair;
      std::smatch match;
      if (std::regex_match(token, match, tr.regex)) {
        found = true;
        token_type = type;
        break;
      }
    }
    if (!found) {
      // TODO - Error TOKEN DOES NOT MATCH ANY RULE
      return;
    }
    
    if (previous_token.second.empty()) {
      previous_token = {token_type, token};
    } else {
      feed_token(previous_token.first, previous_token.second, token_type, token);
      previous_token = {token_type, token};
    }
  }
};

struct parser_ctx_t {
  parser_ctx_t();
  ~parser_ctx_t();
  
  void* lexer;
  yy::location* loc;
  yy::parser* parser;
  
  //cson_object* obj = new cson_object{};
  std::deque<ulang_token> token_list{};
  std::unordered_map<std::string, ulang_grammar> grammars{{ "default", ulang_grammar{
    .name = "default",
    .token_rules = {{"ANY", {std::regex(R"([^ \n\t\r\s]+)")}}},
    .grammar_rules = {{"root", {
      .options = {{"ANY:item"}, {"root:prev", "ANY:item"}},
      .patterns = {{"$item"}, {"$prev", "$item"}},
      .index_maps = {{{"item", 0}},{{"prev", 0},{"item", 1}}},
    }}},
  }}};
  std::stack<ulang_grammar_ctx> current_grammar{{ulang_grammar_ctx{
    "default", grammars["default"]
  }}};
  
  void push_grammar_ctx(const std::string& grammar) {
    current_grammar.emplace(
      grammar,
      grammars[grammar]
    );
  }
};

#endif //UPARSER_CTX_H
