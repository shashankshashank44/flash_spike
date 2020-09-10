
#ifndef _RISCV_MEMDELAY_H
#define _RISCV_MEMDELAY_H

#include <vector>
#include <list>
#include <stdint.h>
#include <decode.h>
//#include "processor.h"
using namespace std;

class memdelay_t
{
    public:    
	//processor_t *proc;
    memdelay_t();
    ~memdelay_t();

    enum class Mapping{
        RoBaCoCh,
        ChBaRoCo
    };
    
	enum class Scheduling_policy{
        FCFS,
        FRFCFS
    };
  
    enum class Command : int
    {
        NOP, ACT, PRE,
        READ,  WRITE
    }; 

    enum class MemSize{
        two, four
    };

    enum class Type : int
    {
        READ, WRITE//, PREF
    };   

    struct Request{
        uint64_t addr;
        Type type;
		bool pref;
		vector<int> addr_break;
    };

    struct SpeedEntry {
        int rate = 1600;
        double freq = (400/3)*6, tCK = (3/0.4)/6;
        int nBL = 4, nCCDS = 4, nCCDL = 5, nRTRS = 2;
        int nCL = 11, nRCD = 11, nRP = 11, nCWL = 9;
        int nRAS = 28, nRC = 39;
        int nRTP = 6, nWTRS = 2, nWTRL = 6, nWR = 12;
        int nRRDS = 0, nRRDL = 0, nFAW = 0;
        int nRFC = 0, nREFI = 0;
        int nPD = 4, nXP = 5, nXPDLL = 0; // XPDLL not found in DDR4??
        int nCKESR = 5, nXS = 0, nXSDLL = 0; // nXSDLL TBD (nDLLK), nXS = (tRFC+10ns)/tCK
    };

	struct timing_table{
		int cmd;
		int delay;
	};
	vector<pair<int, timing_table>> future_timing;
    
	struct Bank
    {
        int64_t id;
        int64_t active_row;
        //Command cur_command;
        int last_type;          //last request type to this bank; (read,1 or write,2)
        int last_cmd;
		uint64_t next_free_clock;
        Request req;     
		//stats
		uint64_t row_conflicts;
		uint64_t request_count;
		uint64_t total_request_count;
	};

    struct Channel
    {
        int64_t id;
        //Command cur_command;
        vector<Bank> bank_list;
        uint64_t next_free_clock;
        Request req;
		struct Controller{
			list<Request> rqueue;
			list<Request> wqueue;
			vector<pair<Request, uint64_t>> done_queue;
		}controller;	
    };
    vector<Channel> channel_list;

    //var
    //vector<int> addr_bits = {14, 4, 7, 2, 5};       //Row, Bank, Column, Channel, Cacheline
    vector<int> addr_bits = {14, 2, 9, 2, 5};       //Row, Bank, Column, Channel, Cacheline
    int channel_count;
    int bank_count;
	uint64_t clk;
	float mem_speed;
	uint32_t qsize;
	bool CSR_flag = 0;
	bool req_satisfied;
    //functions
    int req_send(uint64_t addr, int type);
	bool tick();
	void timing_delays();
	bool search_line_queue(reg_t line);
 
    int log2(int val)
    {
        int n = 0;
        while(val >>= 1)
            n++;
        return n;
    } 

    int set_rowcount(MemSize memsize)
    {
        if(memsize == MemSize::two)
        {
            return (31 - (addr_bits[1] + addr_bits[2] + addr_bits[3] + addr_bits[4]));
        }
        else if(memsize == MemSize::four)
        {
            return (32 - (addr_bits[1] + addr_bits[2] + addr_bits[3] + addr_bits[4]));
        }
        else 
            return 0;
    }

};

#endif
