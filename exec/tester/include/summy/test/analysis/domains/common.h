/*
 * common.h
 *
 *  Created on: Mar 30, 2016
 *      Author: Julian Kranz
 */

#pragma once

#include <bjutil/binary/elf_provider.h>
#include <summy/analysis/domains/summary_dstack.h>
#include <summy/big_step/analysis_dectran.h>
#include <map>

struct _analysis_result {
  analysis_dectran *dt;

  analysis::summary_dstack *ds_analyzed;
  std::map<size_t, size_t> addr_node_map;
  elf_provider *elfp;

  size_t max_it;

  _analysis_result() {
    dt = NULL;
    ds_analyzed = NULL;
    elfp = NULL;
    max_it = 0;
  }

  ~_analysis_result() {
    delete ds_analyzed;
    delete elfp;
    delete dt;

    ds_analyzed = NULL;
    elfp = NULL;
    dt = NULL;
    max_it = 0;
  }
};

enum language_t { ASSEMBLY, C, CPP };

void state(_analysis_result &r, string program, language_t lang, bool gdsl_optimize, uint8_t compiler_opt);
void state_asm(_analysis_result &r, string _asm, bool gdsl_optimize = false);
void state_c(_analysis_result &r, string c, bool gdsl_optimize = false);
void state_cpp(_analysis_result &r, string c, bool gdsl_optimize = false);
