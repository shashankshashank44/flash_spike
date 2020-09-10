#include "memdelay.h"
#include <iostream>

using namespace std;

memdelay_t::memdelay_t()
{
    MemSize memsize = MemSize::four;    
    Mapping mapping = Mapping::RoBaCoCh;
	//Scheduling_policy = Scheduling_policy::FCFS;
	qsize = 32;

    switch(int(memsize)){
        case int(MemSize::two):
        channel_count = 2;
        bank_count = 4;
        break;
        case int(MemSize::four):
        channel_count = 4;
        bank_count = 4;
        break;
    }
    addr_bits[1] = log2(bank_count);    
    addr_bits[3] = log2(channel_count);  
    addr_bits[0] = set_rowcount(memsize);  
    
    for(int j=0; j<channel_count; j++)
    {
        Channel ch;
        ch.id = j;
        ch.next_free_clock = 0;
        for(int i=0; i<bank_count; i++)
        {
            Bank b;
            b.id = i;
            b.active_row = -1;
            b.next_free_clock = 0;
            b.last_type = 0;
			b.row_conflicts = 0;
			b.total_request_count = 0;
			b.request_count = 0;
		 	b.last_cmd = (int)Command::NOP;	
            ch.bank_list.push_back(b);
        } 
        channel_list.push_back(ch);
    }    
	timing_delays();		//initialize timing
	//cout<<"size:"<<future_timing.size()<<"\n";
	
	clk = 0;

	mem_speed = 4;		//800 MHz - mem ; 1000 MHz - cpu; 4:5-ratio of mem:proc
    /*for(int i=0; i<addr_bits.size(); i++)
        std::cout<<addr_bits[i]<<" ";
    std::cout<<"\n";*/

}

memdelay_t::~memdelay_t()
{
	uint64_t roi_row_conf = 0, roi_request_count = 0, total_request_count = 0;
    for(uint64_t j=0; j<channel_list.size(); j++)
    {
        for(uint64_t i=0; i<channel_list[j].bank_list.size(); i++)
        {
			//std::cout<<"Row_misses_of_bank:"<<channel_list[j].bank_list[i].id<<" "<<channel_list[j].bank_list[i].row_miss;
			roi_row_conf += channel_list[j].bank_list[i].row_conflicts;
			//std::cout<<"Requests_count_of_bank:"<<channel_list[j].bank_list[i].id<<" "<<channel_list[j].bank_list[i].request_count<<"\n";
			roi_request_count += channel_list[j].bank_list[i].request_count;	
			total_request_count += channel_list[j].bank_list[i].total_request_count;	
		}
	}
	std::cout<<"ROI Row conflicts:"<<roi_row_conf<<"\n";
	std::cout<<"ROI Request count:"<<roi_request_count<<"\n";
	std::cout<<"Total Request count:"<<total_request_count<<"\n";
}

void memdelay_t::timing_delays()
{
    SpeedEntry s;
	//vector<pair<int, timing_table>> future_timing;
	for(int k=0; k<5; k++)
	{
		switch(k)
		{
			timing_table tt;

			case 0:
			tt.cmd = (int)Command::NOP;
			tt.delay = 0;
			future_timing.push_back(make_pair((int)Command::NOP, tt));
			tt.cmd = (int)Command::ACT;
			tt.delay = s.nRAS;
			future_timing.push_back(make_pair((int)Command::NOP, tt));
			tt.cmd = (int)Command::PRE;
				tt.delay = 0;
			future_timing.push_back(make_pair((int)Command::NOP, tt));
			tt.cmd = (int)Command::READ;
				tt.delay = 0;
			future_timing.push_back(make_pair((int)Command::NOP, tt));
			tt.cmd = (int)Command::WRITE;
				tt.delay = 0;
			future_timing.push_back(make_pair((int)Command::NOP, tt));
			break;
			case 1:
			tt.cmd = (int)Command::NOP;
			tt.delay = 0;
			future_timing.push_back(make_pair((int)Command::ACT, tt));
			tt.cmd = (int)Command::ACT;
			tt.delay = s.nRC;
			future_timing.push_back(make_pair((int)Command::ACT, tt));
			tt.cmd = (int)Command::PRE;
				tt.delay = s.nRAS;
			future_timing.push_back(make_pair((int)Command::ACT, tt));
			tt.cmd = (int)Command::READ;
				tt.delay = s.nRCD;
			future_timing.push_back(make_pair((int)Command::ACT, tt));
			tt.cmd = (int)Command::WRITE;
				tt.delay = s.nRCD;
			future_timing.push_back(make_pair((int)Command::ACT, tt));
			break;
			case 2:
			tt.cmd = (int)Command::NOP;
			tt.delay = 0;
			future_timing.push_back(make_pair((int)Command::PRE, tt));
			tt.cmd = (int)Command::ACT;
			tt.delay = s.nRP;
			future_timing.push_back(make_pair((int)Command::PRE, tt));
			tt.cmd = (int)Command::PRE;
				tt.delay = 0;
			future_timing.push_back(make_pair((int)Command::PRE, tt));
			tt.cmd = (int)Command::READ;
				tt.delay = 0;
			future_timing.push_back(make_pair((int)Command::PRE, tt));
			tt.cmd = (int)Command::WRITE;
				tt.delay = 0;
			future_timing.push_back(make_pair((int)Command::PRE, tt));
			break;
			case 3:
			tt.cmd = (int)Command::NOP;
			tt.delay = 0;
			future_timing.push_back(make_pair((int)Command::READ, tt));
			tt.cmd = (int)Command::ACT;
			tt.delay = s.nRTP + s.nRP;
			future_timing.push_back(make_pair((int)Command::READ, tt));
			tt.cmd = (int)Command::PRE;
				tt.delay = s.nRTP;
			future_timing.push_back(make_pair((int)Command::READ, tt));
			tt.cmd = (int)Command::READ;
				tt.delay = s.nCCDL;
			future_timing.push_back(make_pair((int)Command::READ, tt));
			tt.cmd = (int)Command::WRITE;
				tt.delay = s.nCCDL;
			future_timing.push_back(make_pair((int)Command::READ, tt));
			break;
			case 4:
			tt.cmd = (int)Command::NOP;
			tt.delay = 0;
			future_timing.push_back(make_pair((int)Command::WRITE, tt));
			tt.cmd = (int)Command::ACT;
			tt.delay = s.nCWL + s.nBL + s.nWR + s.nRP;
			future_timing.push_back(make_pair((int)Command::WRITE, tt));
			tt.cmd = (int)Command::PRE;
				tt.delay = s.nCWL + s.nBL + s.nWR;
			future_timing.push_back(make_pair((int)Command::WRITE, tt));
			tt.cmd = (int)Command::READ;
				tt.delay = s.nCWL + s.nBL + s.nWTRL;
			future_timing.push_back(make_pair((int)Command::WRITE, tt));
			tt.cmd = (int)Command::WRITE;
				tt.delay = s.nCCDL;
			future_timing.push_back(make_pair((int)Command::WRITE, tt));
			break;
		}
	}
}

bool memdelay_t::search_line_queue(reg_t line){		//this function search queue to find line if final delays are completed.
	for(uint64_t i=0; i<channel_list.size(); i++)	
	{
		for(int j=0; j<channel_list[i].controller.done_queue.size(); j++)
		{
			//return if the requested line completed or not
			//cout<<"done:"<<i<<" "<<channel_list[i].controller.done_queue[j].first.addr<<" "<<channel_list[i].controller.done_queue[j].second<<"\n";	
			if(line == channel_list[i].controller.done_queue[j].first.addr){
				if(clk > channel_list[i].controller.done_queue[j].second)
					return true;
				else 
					return false;
			}			
		}
	}
	return false;
}

bool memdelay_t::tick()
{
	clk++;
    SpeedEntry s;
	//Channel c;
	bool something_pending = false;
	for(uint64_t i=0; i<channel_list.size(); i++)
	{
		//c = channel_list[i];
		//cout<<"channel:"<<channel_list[i].id<<" tick:"<<clk<<" rqueue:"<<channel_list[i].controller.rqueue.size()<<"\n";
		if(channel_list[i].controller.wqueue.size())
		{
			//writes
			channel_list[i].req = channel_list[i].controller.wqueue.front();
		}
		else if(channel_list[i].controller.rqueue.size())
		{
			//reads
			channel_list[i].req = channel_list[i].controller.rqueue.front();
		}
		else
        {
            for(int k=0; k<bank_count; k++)
            {
                if(channel_list[i].bank_list[k].next_free_clock >= clk){
                    //cout<<"false:"<<channel_list[i].bank_list[k].next_free_clock<<"\n";;
                    return false;
                }
                else    //if this is last channel, tick completed its job. return true
                {
                    if((i == (channel_list.size()-1))  && (k == (bank_count-1)) && !something_pending)
                    {
                        //cout<<"this:"<<i<<" "<<k<<"\n";
                        req_satisfied = true;
                        return true;
                    }
                    else
                        continue;
                }
            }
            continue;
        }
		/*else	//if this is last channel, tick completed its job. return true
		{
			if(i == (channel_list.size()-1))
				return true;
			else
				continue;
		}*/
		
		//if this request's bank is busy, skip this cycles
		//cout<<"bank:"<<channel_list[i].req.addr_break[1]<<"\n";
		//cout<<"check:"<<clk<<" "<<c.bank_list[c.req.addr_break[1]].next_free_clock<<"\n";
		if(clk < channel_list[i].bank_list[channel_list[i].req.addr_break[1]].next_free_clock)	//since FCFS, if this channel queue is stalled go to next channel
		{
			something_pending = true;
			continue;																
		}
		//cout<<c.req.addr_break[0]<<" "<<c.req.addr_break[1]<<"\n";
		//check if row is activated; if not run activate command and exit tick
		if(channel_list[i].req.addr_break[0] != channel_list[i].bank_list[channel_list[i].req.addr_break[1]].active_row)
		{
			//issue ACT
			//if(CSR_flag)
			//	cout<<"Channel:"<<i<<" Bank:"<<channel_list[i].bank_list[channel_list[i].req.addr_break[1]].id<<" active_row:"<<channel_list[i].bank_list[channel_list[i].req.addr_break[1]].active_row<<" req_row:"<<channel_list[i].req.addr_break[0]<<"\n";
			//cout<<(int)c.bank_list[c.req.addr_break[1]].last_cmd<<" "<<(int)Command::ACT<<"\n";
			//cout<<future_timing.size()<<"\n";
			int delay = future_timing[((int)channel_list[i].bank_list[channel_list[i].req.addr_break[1]].last_cmd * 4)+(int)Command::ACT].second.delay;	
			//cout<<"adelay:"<<delay+clk<<"\n";
			channel_list[i].bank_list[channel_list[i].req.addr_break[1]].next_free_clock = clk + delay;
			//cout<<"bclk:"<<channel_list[i].bank_list[channel_list[i].req.addr_break[1]].next_free_clock<<"\n";
			channel_list[i].bank_list[channel_list[i].req.addr_break[1]].last_cmd = (int)Command::ACT;
			channel_list[i].bank_list[channel_list[i].req.addr_break[1]].active_row = channel_list[i].req.addr_break[0];
			if(channel_list[i].bank_list[channel_list[i].req.addr_break[1]].active_row != -1 && (CSR_flag))		//if -1 row closed. else other row open, so conflict
				channel_list[i].bank_list[channel_list[i].req.addr_break[1]].row_conflicts++;
			return false;
		}
		else{
			//issue CMD and pop_front
			if(channel_list[i].req.type == Type::READ){
				int delay = future_timing[((int)channel_list[i].bank_list[channel_list[i].req.addr_break[1]].last_cmd * 4)+(int)Command::READ].second.delay;	
				//cout<<"rdelay:"<<delay<<"\n";
				channel_list[i].bank_list[channel_list[i].req.addr_break[1]].next_free_clock = clk + delay;
				channel_list[i].bank_list[channel_list[i].req.addr_break[1]].last_cmd = (int)Command::READ;	
				if(CSR_flag)
					channel_list[i].bank_list[channel_list[i].req.addr_break[1]].request_count++;	
				channel_list[i].bank_list[channel_list[i].req.addr_break[1]].total_request_count++;
				if(channel_list[i].req.pref == true)
					channel_list[i].controller.done_queue.push_back(make_pair(channel_list[i].controller.rqueue.front(), clk+delay));
				channel_list[i].controller.rqueue.pop_front();
				return false;
			}	
			else if(channel_list[i].req.type == Type::WRITE){
				int delay = future_timing[((int)channel_list[i].bank_list[channel_list[i].req.addr_break[1]].last_cmd * 4)+(int)Command::WRITE].second.delay;	
				//cout<<"wdelay:"<<delay<<"\n";
				channel_list[i].bank_list[channel_list[i].req.addr_break[1]].next_free_clock = clk + delay;
				channel_list[i].bank_list[channel_list[i].req.addr_break[1]].last_cmd = (int)Command::WRITE;	
				if(CSR_flag)
					channel_list[i].bank_list[channel_list[i].req.addr_break[1]].request_count++;	
				channel_list[i].bank_list[channel_list[i].req.addr_break[1]].total_request_count++;	
				if(channel_list[i].req.pref == true)
					channel_list[i].controller.done_queue.push_back(make_pair(channel_list[i].controller.wqueue.front(), clk+delay));
				channel_list[i].controller.wqueue.pop_front();
				return false;
			}	
		}	

	}
	return false;
} 


int memdelay_t::req_send(uint64_t addr, int type)
{
	//cout<<addr<<" "<<type<<"\n";
	Request req;
	req.addr = addr;
	switch(type)
	{
		case 1:req.type = Type::READ; req.pref = false; break;
		case 2:req.type = Type::WRITE; req.pref = false; break;
		case 3:req.type = Type::READ; req.pref = true; break;
	}
	//if(type == 3)
	//	cout<<"pref_mem:"<<addr<<" clk:"<<clk<<"\n";
	//req.type = type==1?Type::READ:Type::WRITE;
    int delay = 0;
    vector<int> addr_break={0, 0, 0, 0, 0};
    for(int i=(addr_bits.size()-1); i>=0; i--)
    {
        addr_break[i] = addr & ((1<<addr_bits[i]) - 1);
        addr >>= addr_bits[i];
        // std::cout<<addr_break[i]<<" "<<addr<<" "<<(addr & ((1<<addr_bits[i]) - 1))<<"\n";
        //printf("addr:%d\t",addr_break[i]);
    }
	//cout<<addr_break[0]<<" "<<addr_break[1]<<" "<<addr_break[2]<<" "<<addr_break[3]<<" "<<addr_break[4]<<"\n";
	req.addr_break = addr_break;
	if((type == 1 || type == 3 )&& channel_list[addr_break[3]].controller.rqueue.size() < qsize)
	{
		channel_list[addr_break[3]].controller.rqueue.push_back(req);
	}	
	else if(type == 2 && channel_list[addr_break[3]].controller.wqueue.size() < qsize)
	{
		channel_list[addr_break[3]].controller.wqueue.push_back(req);
	}	
	return true;
}

