#include "flashmodel.h"
#include <iostream>
#include "mmu.h"

using namespace std;

flashmodel_t::flashmodel_t(mmu_t* _mmu)
{
	mmu = _mmu;
	//Flash flash;
	//Flash_Cache fc;
	flash.addr_bits = {1, 2, 1, 1, 11, 6};
	fc.line_count = 524288;	//65536;			//Assuming flash cache is 2MB, each line is 32B
	fc.line_size = 32;
	flash_speed = 10;
	clk = 0;
}

flashmodel_t::~flashmodel_t()
{
	cout<<"Flash Cache hits:"<<flash_cache_hits<<"\n";
	cout<<"Flash Cache misses:"<<flash_cache_misses<<"\n";
	cout<<"Flash Cache dirt evicts:"<<fc.flash_cache_dirty_evicts<<"\n";
	cout<<"Total Request:"<<total_reqs<<"\n";
	cout<<"ROI Total Request:"<<roi_total_reqs<<"\n";
	cout<<"ROI Flash Cache hits:"<<roi_flash_cache_hits<<"\n";
	cout<<"ROI Flash Cache misses:"<<roi_flash_cache_misses<<"\n";
	cout<<"BC requests:"<<BC_reqs<<"\n";
	cout<<"BC requests hits:"<<BC_hits<<"\n";
	cout<<"BC requests misses:"<<BC_misses<<"\n";
}

int flashmodel_t::Flash_Cache::check_hit(Request &req, Flash &flash, int64_t &clk)
{
	Line l;
	l.addr = req.addr;
	l.tag = (req.addr >> ((int)log2(line_count) + (int)log2(line_size)) );
	uint64_t temp_addr = req.addr;
	temp_addr >>= (int)log2(line_size);
	l.dirty = 0;
	l.idx = temp_addr & ((1<<(int)log2(line_size)) - 1);
	l.req = req;
	//cout<<"check_hit:"<<l.tag<<" "<<l.idx<<"\n";
	auto line_pointer = find_if(lines.begin(), lines.end(), [&](Line line){return ((line.tag == l.tag) && (line.idx == l.idx));});
	if(line_pointer == lines.end())
	{
		Request evict_req;
		//miss
		
		//dirty write flag		
		bool dirty_write = 0;		

		//evict existing line
		//cout<<"req_addr:"<<l.addr<<"\n";
		evict_req = evict(l.idx, l.tag);
		if(evict_req.dirty_evict){			//if evicting line is dirty, then write to flash
			dirty_write = send_to_flash(evict_req, flash, clk);		//if dirty_write is 0, then the unit where the write is supposed to be written is busy. so everything should wait.
			if(dirty_write == 0)
				return 2;
		}

		//allocate a line
		allocate(l);		

		//send to flash
		//send_to_flash(line);

		return 0;
	}
	else{
		//hit
		//if request is write, make line dirty
		if(req.type == Type::WRITE)
			line_pointer->dirty = 1;	
		return 1;
	}
	
	return 0;	
}

flashmodel_t::Request flashmodel_t::Flash_Cache::evict(uint64_t idx, uint64_t tag){
	//dirty flag
	Request req;
	req.dirty_evict = 0;
	bool dirty_flag = 0;	
	//before eviction, find dirty line	
	auto dirty_line = find_if(lines.begin(), lines.end(), [&](Line line){return ((line.tag != tag) && (line.idx == idx));});
	if(dirty_line != lines.end())
	{
		if(dirty_line->dirty)
		{	
			dirty_flag = 1;
			flash_cache_dirty_evicts++;	
			req = dirty_line->req;
			req.dirty_evict = 1;
		}
	}
	//removes the line
	lines.erase(std::remove_if(lines.begin(), lines.end(), [&](Line const & l){return ((l.idx == idx) && (l.tag != tag));}), lines.end());		

	return req;
}

bool flashmodel_t::Flash_Cache::allocate(Line &line){

	if(lines.size() < line_count)
		lines.push_back(line);

	return 1;
}

bool flashmodel_t::Flash_Cache::send_to_flash(Request &req, Flash &flash, int64_t &clk){
	int64_t map_unit;
	map_unit = (req.addr_break[0] * 1000);
	map_unit += (req.addr_break[1] * 100);
	map_unit += (req.addr_break[2] * 10);
	map_unit += (req.addr_break[3] * 1);          //It has unique 4 digit number that denotes busy unit (plane, die, chip, channel)
	auto lut_find = find_if(flash.lookuptable.begin(), flash.lookuptable.end(), [&](flashmodel_t::Flash::LookUpTable_t l){return (l.id = map_unit);});
	if(lut_find != flash.lookuptable.end())		//if yes, don't process it
		return 0;

	flashmodel_t::Flash::LookUpTable_t lut;
	lut.id = map_unit;
	lut.req = req;
	lut.finish_clk = clk + Flash::write_delay;
	flash.lookuptable.push_back(lut);
	return 1;
}

bool flashmodel_t::enqueue(reg_t addr, int type, int req_host){
	total_reqs++;
	if(mmu->proc->CSR_flag)
		roi_total_reqs++;

	if(req_host == 1)
		BC_reqs++;

	Request req;
	req.addr = addr;
	vector<int> addr_break={0, 0, 0, 0, 0, 0};
	for(int i=(flash.addr_bits.size()-1); i>=0; i--){
		addr_break[i] = addr & ((1<<(flash.addr_bits[i])) - 1);
        addr >>= flash.addr_bits[i];
	}
	req.addr_break = addr_break;
	((type == 1)?(req.type = Type::READ):(req.type = Type::WRITE));
	if(type != 3)
		req.pref = 0;
	else if(type == 3)
		req.pref = 1;
	req.dirty_evict = 0;
	if(req_host == 1)
		req.host = 1;
	if(controller.ioqueue.size() < controller.size)
		controller.ioqueue.push_back(req);
	return 1;
}

bool flashmodel_t::search_line_queue(reg_t line){
	auto lut_find = find_if(flash.lookuptable.begin(), flash.lookuptable.end(), [&](Flash::LookUpTable_t l){return (l.req.addr = line);});
	if(lut_find != flash.lookuptable.end())		//if yes, don't process it
		return false;
	return true;	
}

void flashmodel_t::tick(){

	clk++;
	
	//check LUT and clear it
	int index = -1;
	for(int i=0; i<flash.lookuptable.size(); i++){
		if(clk > flash.lookuptable[i].finish_clk){
			index = i;
			break;
		}
	}
	if(index != -1){
		if(flash.lookuptable[index].req.pref == 0)		//only make req_satisfied true for regular requests, not for prefetch reqs
			mmu->req_satisfied = true;
		flash.lookuptable.erase(flash.lookuptable.begin()+index);
		return;
	}

	//check if IO queue is empty
	if(!controller.ioqueue.size())
		return;

	//Read the IO queue (if there is a demand request, then prioritize it or if prefetch reqs, FIFO)
	Request io_req;
	int64_t map_unit = 0;
	auto demand = find_if(controller.ioqueue.begin(), controller.ioqueue.end(), [&](Request r){return (r.pref == 0);});
	if(demand != controller.ioqueue.end()){
		//if this req maps to specific unit that is already busy, it should wait
		map_unit = (demand->addr_break[0] * 1000);
        map_unit += (demand->addr_break[1] * 100);
        map_unit += (demand->addr_break[2] * 10);
        map_unit += (demand->addr_break[3] * 1);          //It has unique 4 digit number that denotes busy unit (plane, die, chip, channel)
		io_req = *demand;
	}
	else{
		//send to flash cache; receives the response from cache function saying hit in cache or request sent to memory 
		io_req = controller.ioqueue.front();	
		map_unit = (io_req.addr_break[0] * 1000);
        map_unit += (io_req.addr_break[1] * 100);
        map_unit += (io_req.addr_break[2] * 10);
        map_unit += (io_req.addr_break[3] * 1);          //It has unique 4 digit number that denotes busy unit (plane, die, chip, channel)
	}	

	//checks IO Cache
	bool status = fc.check_hit(io_req, flash, clk); 

	//this status is when line is a miss and evicted line is dirty and also dirty line cannot write because of busy flash unit
	if(status == 2)
		return;

	//then respective flash block is busy (future available time is written)
	if(status == 0)		//miss in Flash Cache
	{
		//before check if the block is already busy
		auto lut_find = find_if(flash.lookuptable.begin(), flash.lookuptable.end(), [&](Flash::LookUpTable_t l){return (l.id = map_unit);});
		if(lut_find != flash.lookuptable.end())		//if yes, don't process it
			return;

		flash_cache_misses++;
		if(mmu->proc->CSR_flag)
			roi_flash_cache_misses++;
		if(io_req.host == 1)	
			BC_misses++;		

		Flash::LookUpTable_t lut;
		lut.id = map_unit;
		lut.req = io_req;
		lut.finish_clk = clk + Flash::read_delay;
		flash.lookuptable.push_back(lut);
		if(demand != controller.ioqueue.end())
			controller.ioqueue.erase(demand);
		else
			controller.ioqueue.pop_front();
	}
	else if(status == 1)		//hit in Flash Cache
	{
		//before check if the block is already busy
		auto lut_find = find_if(flash.lookuptable.begin(), flash.lookuptable.end(), [&](Flash::LookUpTable_t l){return (l.id = map_unit);});
		if(lut_find != flash.lookuptable.end())		//if yes, don't process it
			return;

		flash_cache_hits++;
		if(mmu->proc->CSR_flag)
			roi_flash_cache_hits++;
		if(io_req.host == 1)	
			BC_hits++;		
		
		Flash::LookUpTable_t lut;
		lut.id = map_unit;
		lut.req = io_req;
		lut.finish_clk = clk + Flash_Cache::read_delay;
		flash.lookuptable.push_back(lut);
		if(demand != controller.ioqueue.end())
			controller.ioqueue.erase(demand);
		else
			controller.ioqueue.pop_front();
	}

}
