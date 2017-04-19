/*
 * dectran.cpp
 *
 *  Created on: Oct 30, 2014
 *      Author: Julian Kranz
 */

#include <algorithm>
#include <assert.h>
#include <cppgdsl/block.h>
#include <cppgdsl/gdsl.h>
#include <cppgdsl/gdsl_exception.h>
#include <cppgdsl/gdsl_exception.h>
#include <cppgdsl/instruction.h>
#include <cppgdsl/optimization.h>
#include <cppgdsl/rreil/statement/statement.h>
#include <limits.h>
#include <summy/big_step/dectran.h>
#include <summy/cfg/cfg.h>
#include <summy/cfg/node/address_node.h>
#include <summy/cfg/node/node_visitor.h>
#include <summy/transformers/decomposer.h>
#include <summy/transformers/goto_ip_adder.h>
#include <summy/transformers/ip_propagator.h>
#include <summy/transformers/transformer.h>
#include <summy/transformers/trivial_connector.h>
#include <vector>

using gdsl::gdsl_exception;

using namespace gdsl::rreil;
using namespace std;
using namespace std::experimental;

using std::upper_bound;

dec_interval::dec_interval(int_t from, int_t to) : from(from), to(to) {}

bool dec_interval::operator<(const dec_interval &other) const {
  return from < other.from;
}

// bool dec_interval::operator <(const int_t &v) const {
//  return from < v;
//}

std::vector<std::tuple<uint64_t, gdsl::rreil::statements_t>> dectran::decode_translate(
  bool decode_multiple) {
  std::vector<std::tuple<uint64_t, gdsl::rreil::statements_t>> prog;
  if(gdsl.bytes_left() <= 0) throw string("Unable to decode");

  int_t ip_before = gdsl.get_ip();

  do {
    int_t ip = gdsl.get_ip();

    //    cout << "Dec/Tran @0x" << hex << gdsl.get_ip() << dec << endl;

    gdsl::rreil::statements_t rreil;
    if(blockwise_optimized) {
      optional<gdsl::block> b = [&]() -> optional<gdsl::block> {
        try {
          return gdsl.decode_translate_block(gdsl::optimization_configuration::CONTEXT |
                                               gdsl::optimization_configuration::LIVENESS |
                                               gdsl::optimization_configuration::FSUBST,// |
//                                                gdsl::optimization_configuration::DELAYEDFSUBST,
            LONG_MAX);
        } catch(gdsl_exception &s) {
          cout << "GDSL Error @0x" << hex << gdsl.get_ip() << dec << ": " << s << endl;
          gdsl.reset_heap();
          return nullopt;
        }
      }();
      if(!b) continue;
      rreil = b->retrieve_statements();
    } else {
      optional<gdsl::instruction> insn = [&]() -> optional<gdsl::instruction> {
        try {
          return optional<gdsl::instruction>(gdsl.decode());
        } catch(gdsl_exception &s) {
          cout << "Decoding error @0x" << hex << gdsl.get_ip() << dec << ": " << s << endl;
          gdsl.reset_heap();
          return nullopt;
        }
      }();
      if(!insn) continue;
      //            printf("Instruction: %s\n", insn.to_string().c_str());
      //            printf("---------------------------------\n");
      rreil = insn.value().translate();
    }

    decode_iterations++;

    gdsl.reset_heap();

    //    printf("RReil (no transformations):\n");
    //    for(statement *s : *rreil)
    //      printf("%s\n", s->to_string().c_str());
    //
    prog.push_back(make_tuple(ip, std::move(rreil)));
  } while(decode_multiple && gdsl.bytes_left() > 0);

  int_t ip_after = gdsl.get_ip();

  if(ip_after > ip_before) {
    dec_interval decoded = dec_interval(ip_before, ip_after - 1);
    set<dec_interval>::iterator intervals_it = decoded_intervals.find(decoded);
    if(intervals_it != decoded_intervals.end() && intervals_it->to != decoded.to) {
      dec_interval combined = dec_interval(intervals_it->from, max(intervals_it->to, decoded.to));
      decoded_intervals.erase(intervals_it);
      decoded_intervals.insert(combined);
    } else
      decoded_intervals.insert(decoded);
  }

  if(prog.size() == 0) {
    prog.push_back(make_tuple(ip_before, gdsl::rreil::statements_t()));
  }

  return prog;
}

size_t dectran::initial_cfg(
  cfg::cfg &cfg, bool decode_multiple, std::experimental::optional<std::string> name) {
  size_t ip = (size_t)gdsl.get_ip();
  auto fmap_it = fmap.find(ip);
  if(!name && fmap_it != fmap.end()) name = fmap_it->second;

  auto prog = decode_translate(decode_multiple);
  //  cfg::translated_program_t prog_unowned;
  //  for(auto &elem : prog) {
  //    vector<gdsl::rreil::statement*> stmts_unowned;
  //    for(auto& stmt : get<1>(elem))
  //      stmts_unowned.push_back(stmt.get());
  //    prog_unowned.push_back(make_tuple(get<0>(elem),
  //    gdsl::iterable<gdsl::rreil::statement>(stmts_unowned)));
  //  }
  size_t head_node = cfg.add_program(prog, name);

  //  for(auto t : prog) {
  //    vector<gdsl::rreil::statement *> *rreil;
  //    tie(ignore, rreil) = t;
  //    for(auto stmt : *rreil)
  //      delete stmt;
  //    delete rreil;
  //  }

  //  vector<transformer *> transformers;
  //  transformers.push_back(new decomposer(&cfg, head_node));
  //  transformers.push_back(new goto_ip_adder(&cfg, head_node));
  //  transformers.push_back(new ip_propagator(&cfg, head_node));
  //  //  transformers.push_back(new trivial_connector(&cfg));
  //  for(auto t : transformers) {
  //    t->transform();
  //    delete t;
  //  }
  //
  //  /*
  //   * We're not interested in trivial updates...
  //   */
  //  cfg.clear_updates();

  return head_node;
}

dectran::dectran(cfg::cfg &cfg, gdsl::gdsl &gdsl, bool blockwise_optimized,
  bool speculative_decoding, function_map_t fmap)
    : cfg(cfg), blockwise_optimized(blockwise_optimized),
      speculative_decoding(speculative_decoding), gdsl(gdsl), fmap(fmap), decode_iterations(0) {}

dectran::dectran(
  cfg::cfg &cfg, gdsl::gdsl &gdsl, bool blockwise_optimized, bool speculative_decoding)
    : dectran(cfg, gdsl, blockwise_optimized, speculative_decoding, function_map_t()) {}

void dectran::print_decoding_holes() {
  auto it = decoded_intervals.begin();
  if(it == decoded_intervals.end()) return;
  int_t to = it->to;
  while(++it != decoded_intervals.end()) {
    if(it->from > to + 1) {
      cout << "Warning: Decoding hole from 0x" << hex << (to + 1) << " to 0x" << (it->from - 1)
           << "." << dec << endl;
      cout << "\tHole size: " << (it->from - to - 1) << endl;
    }
    if(it->to > to) {
      to = it->to;
    }
  }
}

int_t dectran::bytes_decoded() {
  auto it = decoded_intervals.begin();
  if(it == decoded_intervals.end()) return 0;
  int_t sum = 0;
  int_t from = it->from;
  int_t to = it->to;
  while(++it != decoded_intervals.end()) {
    if(it->from > to) {
      sum += to - from + 1;
      from = it->from;
      to = it->to;
    } else if(it->to > to) {
      to = it->to;
    }
  }
  sum += to - from + 1;
  return sum;
}

int_t dectran::start_addresses_decoded() {
  return decoded_intervals.size();
}
