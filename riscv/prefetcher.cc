#include <iostream>
#include "prefetcher.h"
#include "mmu.h"
//#include "processor.h"

using namespace std;

prefetcher_t::prefetcher_t(mmu_t* _mmu, processor_t* _proc)
{
	mmu = _mmu;
	proc = _proc;
	sb_size = 8;
	line_size = 32;
	sb = new Sbuffer_t;
	pp = new vector<reg_t>;
	sb->addr_buffer.reserve(sb_size);
	sb->last_prefetch_line = -1;
	prefetch_req = 0;
	prefetch_flag = 0;
	sb_reset = 0;
}

prefetcher_t::~prefetcher_t()
{
	cout<<"Total Prefetch resets:"<<sb_reset<<"\n";
	cout<<"Total Prefetches:"<<prefetch_req<<"\n";
	cout<<"Total Prefetch hits:"<<pf_hits<<"\n";
}

bool prefetcher_t::check_sb(reg_t line)
{
	int index = -1;
	for(int i=0; i<sb->addr_buffer.size(); i++)
	{
		if(line == sb->addr_buffer[i])
			index = i;
	}
	if(index != -1){
		sb->addr_buffer.erase(sb->addr_buffer.begin(), sb->addr_buffer.begin()+index);
		pp->erase(pp->begin(), pp->begin()+index);
		pf_hits++;
		pf_window_hits++;
		return true;
	}
	return false;
}

void prefetcher_t::prefetch(reg_t line)
{
	prefetch_call_count++;
	if(sb->last_prefetch_line == -1 ){
		//sb->last_prefetch_line = line;
		int32_t res;		
		reg_t addr_line = mmu->translate(m_cols_addr, 4, LOAD);
		addr_line = addr_line >> log_lines;
		addr_line = addr_line << log_lines;
		sb->last_prefetch_line = addr_line;
	}

	/*if(prefetch_call_count % 20 == 0){
		if(pf_window_hits == 0)
		{
			//std::cout<<"refresh:"<<prefetch_req<<"\n";;
			sb->addr_buffer.clear();
			pp->clear();
			sb_reset++;
			sb->last_prefetch_line = line;
			//sb->last_prefetch_line = -1;
		}
		else
			pf_window_hits = 0;
	}*/

	while(sb->addr_buffer.size() < sb_size){
		prefetch_req++;
		sb->last_prefetch_line += line_size;
		sb->addr_buffer.push_back(sb->last_prefetch_line);
		pp->push_back(sb->last_prefetch_line);		
		mmu->fm->enqueue(sb->last_prefetch_line, 3, 0);
		proc->tick();		
	}
}	

void prefetcher_t::initialize(reg_t addr, uint8_t * data)
{
	uint32_t val = *(uint32_t*)data;
	cout<<"prefetch_addr:"<<addr<<" val:"<<val<<"\n";
	switch(addr)
	{
		case M_COLS: {m_cols_addr = val; break;}
		case M_VALS: {m_vals_addr = val; break;}
		case V_VALS: {v_vals_addr = val; prefetch_flag = 1; break;}
		default: break;
	}
}
