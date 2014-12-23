/*
 * asm_compile.cpp
 *
 *  Created on: Dec 23, 2014
 *      Author: Julian Kranz
 */

#include <summy/test/asm_compile.h>
#include <bjutil/scope.h>
#include <stdlib.h>
#include <iostream>
#include <fstream>
#include <iterator>

using namespace std;

std::vector<uint8_t> asm_compile(std::string _asm) {
  ofstream asm_file;
  asm_file.open("program.asm");
  asm_file << _asm << endl;
  asm_file.close();

  scope_exit exit([&]() {
    system("rm program.asm");
  });

  if(system("as program.asm -o program.elf")) throw string("Unable to compile assembly program");
  exit.add([&]() {
    system("rm program.elf");
  });

  if(system("objcopy -O binary program.elf program.bin")) throw string("objciopy failed :-(");
  exit.add([&]() {
    system("rm program.bin");
  });

  ifstream bin_file;
  bin_file.open("program.bin");

  std::vector<uint8_t> data;
  data.insert(data.begin(), istream_iterator<uint8_t>(bin_file), istream_iterator<uint8_t>());

  return data;
}
