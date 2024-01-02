//
// Created by castle on 7/24/23.
//

#ifndef UPARSER_H
#define UPARSER_H

#include <string>
#include <fstream>
#include <vector>
#include <unordered_map>

namespace ulang {
  using str = std::string;
  
  struct definition_t {
    enum class parameter_type_e {
      STRING,
      NUMBER,
      BLOCK
    };
    
    struct overload_t {
      std::vector<std::pair<str, parameter_type_e>> parameters;
      str pattern;
      
      bool matches(const std::vector<parameter_type_e>& p_types) const {
        if (p_types.size() != parameters.size()) return false;
        for (int i = 0; const auto& pt: p_types) {
          if (parameters[i++].second != pt) return false;
        }
        return true;
      }
      
      str apply(const std::vector<str>& params) const {
        // IT IS ASSUMED THAT `matches()` RETURNS TRUE FOR THIS OVERLOAD
        for (int i = 0; const auto& [id, ptype]: parameters) {
          const str& param = params[i++];
          // TODO - Apply params and return payload
        }
      }
    };
    
    str identifier;
    std::vector<overload_t> overloads{};
  };
}

namespace uparser {
  class file_t {
    std::ifstream istream;
  public:
    const std::string path;
    
    explicit file_t(const std::string& path)
      : istream(path), path(path) {
    }
    
    ~file_t() {
      istream.close();
    }
  };
  
  class annotated_file_t {
    const file_t file;
    
  public:
    explicit annotated_file_t(file_t file)
      : file(file) {
        // find things in file and store that info in this class.
    }
  }
  
  class module_t {
    std::unordered_map<std::string, annotated_file_t> files{};
    std::string main_file;
    
  public:
    explicit module_t(file_t main_file)
      : main_file(main_file.path) {
        add_file(main_file);
    }
    
    void add_file(file_t file) {
      if (!files.contains(file.path)) {
        files[file.path] = annotated_file_t{file};
      }
    }
    
    std::string generate_output() {
      
    }
  };
  
  void parse(const std::string& str);
}


void sjfjekciebt() {
  uparser::file_t file{"../../test.ul"};
  uparser::module_t mod{file};
  
  auto out = mod.generate_output();
}

#endif //UPARSER_H
