#include <iostream>
#include <fstream>

#include "parser/uparser.h"

int main() {
  //cson_parser::parse("int a = 2; #[grammar: some-gr] { }");
  std::ifstream file("../test.ul");
  std::string file_s;
  file_s.assign((std::istreambuf_iterator<char>(file)),(std::istreambuf_iterator<char>()));
  std::cout << file_s << std::endl;
  uparser::parse(file_s);
  //uparser::parse("#[grammar: ]");
  return 0;
}
//tokens {
//NUM = [0-9]+
//ID  = [a-zA-Z]+
//
//ASSIGN = "->"
//}
//rules {
//root = NUM
//}

//:init ASSIGN ID:id { int $id = $init; }

//#[grammar: id-gr] {
//tokens {
//ID  = [a-zA-Z]+
//
//TYPE_OP = "as"
//}
//rules {
//root = ID:id TYPE_OP ID:type { $id_$type }
//}
//}
//
//#[assign-gr]
//3 -> some_int
//#[assign-gr]
//4 -> #[id-gr] other as int
