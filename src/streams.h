//
// Created by castle on 2/10/24.
//

#ifndef ULANG_STREAMS_H
#define ULANG_STREAMS_H

#include <fstream>
#include <string>

namespace ulang {
  template <typename T, typename Marker>
  struct stream_t {
    virtual T& current() = 0;
    virtual T& advance() = 0;
    virtual T& backtrack() = 0;
    
    virtual bool at_end() = 0;
    
    virtual Marker mark_current() = 0;
    virtual void goto_marker(const Marker& marker) = 0;
  };
  
  struct char_stream_t: public stream_t<char, std::size_t> { };
  
  struct file_stream_t: public char_stream_t {
    explicit file_stream_t(const std::string& path) {
      std::ifstream file_input{path};
      for (std::string line; std::getline(file_input, line);) {
        file.append(line);
        file.append("\n");
      }
    }
    
    char & current() override {
      return file[current_offset];
    }
    char & advance() override {
      current_offset++;
      return current();
    }
    char & backtrack() override {
      current_offset--;
      return current();
    }
    
    bool at_end() override {
      return current_offset == file.size();
    }
    
    std::size_t mark_current() override {
      return current_offset;
    }
    void goto_marker(const std::size_t &marker) override {
      current_offset = marker;
    }
  
  private:
    std::string file;
    std::size_t current_offset = 0UL;
  };
  
}

#endif //ULANG_STREAMS_H
