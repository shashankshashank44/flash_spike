#ifndef _PREFETCHER_H
#define _PREFETCHER_H

#include <vector>
#include <algorithm>
#include "decode.h"
#include "processor.h"

using namespace std;

typedef enum{
	M_COLS = 0,
	M_VALS = 4,
	V_VALS = 8
};

class prefetcher_t
{
public:
	mmu_t * mmu;
	processor_t *proc;
	prefetcher_t(mmu_t* mmu, processor_t* proc);
	~prefetcher_t();

	//Prefetcher structure
	struct Sbuffer_t{
		vector<reg_t> addr_buffer;
		reg_t last_prefetch_line;
		long sb_available_clk;
	};
	struct Sbuffer_t *sb;

	//Pending prefetch requests
	/*struct Pending_Prefetch_t{
		reg_t addr;	
	};
	vector<Pending_Prefetch_t> *pp;*/
	vector<reg_t> *pp;

	//var
	int sb_size;
	int line_size;
	std::vector<reg_t>::iterator addr_buffer_it;
	uint64_t pf_hits = 0;
	uint64_t pf_window_hits = 0;
	uint64_t prefetch_call_count = 0;
	uint64_t prefetch_req;
	reg_t m_cols_addr;
	reg_t m_vals_addr;
	reg_t v_vals_addr;
	int log_lines;
	bool prefetch_flag;
	uint64_t sb_reset;

	//func
	bool check_sb(reg_t line);
	void prefetch(reg_t line);
	void initialize(reg_t addr, uint8_t * data);
};

#endif
