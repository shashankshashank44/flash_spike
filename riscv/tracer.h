// See LICENSE for license details.

#ifndef _RISCV_TRACER_H
#define _RISCV_TRACER_H

#include "processor.h"
#include <iostream>
using namespace std;

static inline void trace_opcode(processor_t* p, insn_bits_t opc, insn_t insn) {
  bool found = 0;
  //if(p->record_clk)
	//std::cout<<"ins:"<<p->cycles<<"\n";
  for(uint64_t i=0; i<p->lookup_table.size(); i++)
  {
    if(opc == p->lookup_table[i].match_opcode)
    {
      p->cycles += p->lookup_table[i].delay;
      found = 1;
      break;
    }
  }
  if (!found) p->cycles += 1;
  //printf("cycles_in_tarcer:%ld\n",p->cycles);
  //fflush(stdout);
}

#endif
