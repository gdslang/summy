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
  std::vector<uint8_t> data;
  scope_exit exit([&]() {
    ofstream asm_file;
     asm_file.open("program.asm");
     asm_file << _asm << endl;
     asm_file.close();

     exit([&]() {
       system("rm program.asm");
     });

     if(system("as program.asm -o program.elf")) throw string("Unable to compile assembly program");
     exit([&]() {
       system("rm program.elf");
     });

     if(system("objcopy -O binary program.elf program.bin")) throw string("objcopy failed :-(");
     exit([&]() {
       system("rm program.bin");
     });

     basic_ifstream<char> bin_file;
     bin_file.open("program.bin", ios::binary);

//     data.insert(data.begin(), istreambuf_iterator<unsigned char>(bin_file), istreambuf_iterator<unsigned char>());
     while(bin_file.good())
       data.push_back(bin_file.get());
  });
  return data;
}
