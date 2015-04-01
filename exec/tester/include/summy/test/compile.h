/*
 * asm_compile.h
 *
 *  Created on: Dec 23, 2014
 *      Author: Julian Kranz
 */

#pragma once

#include <bjutil/binary/elf_provider.h>
#include <string>
#include <vector>
#include <stdint.h>

std::vector<uint8_t> asm_compile(std::string _asm);
elf_provider *asm_compile_elfp(std::string _asm);

std::string c_compile(std::string program);
