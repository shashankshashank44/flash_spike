
#ifndef _RISCV_FLASHMODEL_H
#define _RISCV_FLASHMODEL_H

#include <vector>
#include <list>
#include <stdint.h>
#include <decode.h>
#include "processor.h"
#include <math.h>
using namespace std;
	

class flashmodel_t
{
    public:    
	//processor_t *proc;
	mmu_t* mmu;
    flashmodel_t(mmu_t* mmu);
    ~flashmodel_t();	
	
	//stats
	uint64_t flash_cache_hits = 0;
	uint64_t flash_cache_misses = 0;
	uint64_t total_reqs = 0;
	uint64_t roi_total_reqs = 0;
	uint64_t roi_flash_cache_hits = 0;
	uint64_t roi_flash_cache_misses = 0;
	uint64_t BC_reqs = 0;
	uint64_t BC_hits = 0;
	uint64_t BC_misses = 0;

	enum class Type : int
    {
        READ, WRITE//, PREF
    };
	
	struct Request{
		uint64_t addr;
		Type type;
		bool pref;
		vector<int> addr_break;	
		bool dirty_evict;		//this variable only used when dirty evict; This is not required but using here for simulation purposes 
		int host;				//if host is 0, CPU sent the req else if host is 1, BC sent request
		/*int page;
		int block;
		int plane;
		int die;
		int chip;
		int channel;*/
	};
	
	//This simple flash structure should have request queues and speed entries modelled
	struct Flash{
		vector<int> addr_bits; //= {1, 2, 1, 1, 11, 6};		//Channel, Chip, Die, Plane, Block, Page     
		
		//Look up table
		struct LookUpTable_t{
			int64_t id;			//unique 4 digit id of this request derived from its channel, chip, die and plane id
			Request req;
			int64_t finish_clk;		//clock at which this req. is done
		};

		vector<LookUpTable_t> lookuptable;
	
		//Delay model
		static const int64_t program_delay = 0;	//820 us                                                                                                                           
		static const int64_t erase_delay = 0;		//3000 us
        static const int64_t read_delay = 50;		//cycles @200MHz or 60 us
        static const int64_t write_delay = 50;		//60 us
	
		//functions	
	}flash;

	//This is the Flash Cache 
	struct Flash_Cache{
		struct Line{
			uint64_t addr;
			uint64_t tag;
			uint64_t idx;
			Type type;
			bool dirty;
			Request req;
		};
	
		//Stats
		uint64_t flash_cache_dirty_evicts = 0;

		//Delay
        static const int64_t read_delay = 10;		//
		
		int64_t line_count;			//flash_cache_size/line_size;	if size is 2048KB, then line_count is 2048/32 = 65536 lines
		int64_t line_size; 
		vector<Line> lines;
		
		int check_hit(Request &req, Flash &flash, int64_t &clk);		//checks if hit or miss
		Request evict(uint64_t idx, uint64_t tag);			//evicts a line incase of allocation of new line
		bool allocate(Line &line);		//allocates a new line incase of a miss
		bool send_to_flash(Request &req, Flash &flash, int64_t &clk);		//sends the request to flash

	}fc;
	
	//variables
	int flash_speed;
	int64_t clk;

	//function
	void tick();
	bool enqueue(reg_t addr, int type, int req_host);
	bool search_line_queue(reg_t pf_req_wait);	

	
	//IO queue
	struct Controller{
		list<Request> ioqueue;
		int size = 16;			//max size
	}controller;


};

#endif
