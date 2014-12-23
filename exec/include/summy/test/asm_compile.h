/*
 * asm_compile.h
 *
 *  Created on: Dec 23, 2014
 *      Author: Julian Kranz
 */

#pragma once

#include <string>
#include <vector>
#include <stdint.h>

std::vector<uint8_t> asm_compile(std::string _asm);
