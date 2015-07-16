/*
 * asm_compile.cpp
 *
 *  Created on: Dec 23, 2014
 *      Author: Julian Kranz
 */

#include <bjutil/scope.h>
#include <stdlib.h>
#include <summy/test/compile.h>
#include <iostream>
#include <fstream>
#include <iterator>
#include <string>
#include <sstream>
#include <stdio.h>

using namespace std;

static std::vector<uint8_t> asm_compile(std::string _asm, bool objcopy) {
  std::vector<uint8_t> data;
  scope_exit exit([&]() {
    ofstream asm_file;
    asm_file.open("program.asm");
    asm_file << _asm << endl;
    asm_file.close();

    exit([&]() { system("rm program.asm"); });

    if(system("as program.asm -o program.elf")) throw string("Unable to compile assembly program");
    exit([&]() { system("rm program.elf"); });

    string file;
    if(objcopy) {
      if(system("objcopy -O binary program.elf program.bin")) throw string("objcopy failed :-(");
      exit([&]() { system("rm program.bin"); });
      file = "program.bin";
    } else
      file = "program.elf";

    FILE *f = fopen(file.c_str(), "r");
    size_t read;
    while(true) {
      uint8_t next;
      read = fread(&next, 1, 1, f);
      if(!read) break;
      data.push_back(next);
    }
    fclose(f);

    //     basic_ifstream<char> bin_file;
    //     bin_file.open("program.bin", ios::binary);
    //     data.insert(data.begin(), istreambuf_iterator<unsigned char>(bin_file), istreambuf_iterator<unsigned
    //     char>());
    //     while(bin_file.good()) {
    //       data.push_back(bin_file.get());
    //     }
  });
  return data;
}

std::vector<uint8_t> asm_compile(std::string _asm) {
  return asm_compile(_asm, true);
}

elf_provider *asm_compile_elfp(std::string _asm) {
  vector<uint8_t> data = asm_compile(_asm, false);
  char *buffer = (char *)malloc(data.size());
  for(size_t i = 0; i < data.size(); i++)
    buffer[i] = data[i];
  return new elf_provider(buffer, data.size());
}

std::string c_compile(std::string program, unsigned int opt) {
  string filename = "program.elf";

  scope_exit exit([&]() {
    ofstream c_file;
    c_file.open("program.c");
    c_file << program << endl;
    c_file.close();

    stringstream ss;
    ss << "gcc -O" << opt << " -std=c11 -w program.c -o program.elf";

    exit([&]() { system("rm program.c"); });

    if(system(ss.str().c_str())) throw string("Unable to compile assembly program");
  });

  return filename;
}
