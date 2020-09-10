#include "mmu.h"
#include "buffer_controller.h"
#include <iostream>
using namespace std;

buffer_controller_t::buffer_controller_t(mmu_t * _mmu, processor_t* _proc, unsigned int sz)
{
    mmu = _mmu;
	proc = _proc;
    buffer_sz = sz;
	buffer_count = 1;
	b = new Buffer_t[buffer_count];
	for(int i=0; i<buffer_count; i++){
    	b[i].buffer = new uint8_t [buffer_sz];
    	for (unsigned int j=0;j<sz; j++) 
			b[i].buffer[j]=0xdd;
    	b[i].buffer_rd_idx = 0;
		b[i].buffer_full = 0;
		b[i].buffer_available_clk = 0;
	}
    start=0;
    col_idx=0;
	available_clk = 0;
	fill_buffer_id = 0;
	read_buffer_id = 0;
	busy = false;
	
	/*
	if(op == 1)
		printf("HHT, no expansion\n");
	else if(op == 0){
		switch(format){
			case 0:printf("Express-CSR");break;
			case 1:printf("Express-BV");break;
			case 2:printf("Express-RL");break;
		}
	}
	*/
}

buffer_controller_t::~buffer_controller_t()
{
    if (b) delete [] b;
    //buffer = 0;
}

uint8_t *buffer_controller_t::set_metadata(reg_t addr)
{
	if(addr >= CONFIG_RANGE){}
	else{
	switch (addr)
        {                                                                                                                                    
		case OFFSET_N: { return (uint8_t*)&n; }                                                                                              
		case OFFSET_ROWS: { return (uint8_t*) &row_base; }
        case OFFSET_COLS: { return (uint8_t*) &col_base; }
        case OFFSET_V:    { return (uint8_t*) &v_base; }
        case OFFSET_BID:  { return (uint8_t*) &buf_id; }
        case OFFSET_START: { return (uint8_t*) &start; }
        default: return 0;
        }
	}
} 

void buffer_controller_t::write_to_buffer()					//Store request from software (vswv) writes data to buffer
{
	
}

void buffer_controller_t::write_buffer()					//If the buffer is full, write the buffer to memory
{
	
	
}

void buffer_controller_t::fill_buffer_csr_noexp()
{
    uint8_t * p;
	//std::cout<<start<<"--"<<b[fill_buffer_id].buffer_full<<"\n";
    if (start && !b[fill_buffer_id].buffer_full)		//bc enabled and buffer empty
    {
		under_process = true;
		bool req_host = 1;
        p = (uint8_t*) &(b[fill_buffer_id].buffer[b[fill_buffer_id].buffer_rd_idx]);
        // here is where the buffer controller must 
        // fill data into buffer from the vector using v[cols[col_idx]]
        // We fetch v[.] from the host via the MMU
        // this is two reads - once to read from cols[col_idx].
        // and using this, to read from v[.]
		//std::cout<<"base:"<<col_base<<" "<<col_idx<<"\n";
        reg_t v_idx = col_base + 4*col_idx++;
        reg_t val;
        mmu->load_slow_path(proc, v_idx, 4, (uint8_t*)&val, req_host); 		//these functions through bc have to have extra parameter to distinguish bc reqs over processor so that the delay gets updated to the respective units

        reg_t v_addr = v_base + val*4;
        mmu->load_slow_path(proc, v_addr, 4, p, req_host); 
		uint32_t data = *(uint32_t*)p;
        b[fill_buffer_id].buffer_rd_idx += 4;
        if (b[fill_buffer_id].buffer_rd_idx == buffer_sz){ 
			b[fill_buffer_id].buffer_rd_idx = 0; 
			b[fill_buffer_id].buffer_full = 1; 
			if(fill_buffer_id < (buffer_count-1))
				fill_buffer_id++;
			else
				fill_buffer_id = 0;
			//cout<<"buffer_full:"<<proc->clk<<"\n";
		}
		under_process = false;
    }
	else{
		//scroll throw buffers?
	} 
}

//expansion
//CASE:CSR
void buffer_controller_t::fill_buffer_csr_exp()
{
    uint8_t * p;
	//std::cout<<start<<"--"<<b[fill_buffer_id].buffer_full<<"\n";
    if (start && !b[fill_buffer_id].buffer_full)		//bc enabled and buffer empty
    {
		//std::cout<<"fill"<<"\n";
		under_process = true;
		bool req_host = 1;
		p = (uint8_t*) &(b[fill_buffer_id].buffer[b[fill_buffer_id].buffer_rd_idx]);
		
		if(first_time){
            row_pos = row_base;
            mmu->load_slow_path(proc, row_pos, 4, (uint8_t*)&nnz_present_row, req_host);
            first_time = false;
        }

		if(row_done){
		//cout<<col_idx<<"-"<<n<<"\n";
		col_element_count = 0;
			if(col_idx < nnz){
				index = 0;
				row_pos = row_pos + 4;
				nnz_present_row = nnz_next_row;
				mmu->load_slow_path(proc, row_pos, 4, (uint8_t*)&nnz_next_row, req_host);	
				nnz_cols = nnz_next_row - nnz_present_row;
				present_row++;
				//std::cout<<"present_row:"<<present_row<<" "<<nnz_cols<<"\n";
				//fflush(0);
				previous_processed_col = -1;	//useful in while loop
				/*if(nnz_cols == 0)
				{
					row_done = true;
					return;
				}*/
			}
			else{
				done = true;
				col_idx = 0;
				nnz_next_row = 0;
				present_col = -1;
				start = false;
				return;
			}
			row_done = false;
		}

		//if all nnz are done but row expansion is not done, fill up buffer for the rest of this row with zeros
		if(fill_all_zero){
			while(b[fill_buffer_id].buffer_rd_idx < buffer_sz){
				fill_zero(p);
				//index--;
				//col_idx--;
			}
			
		}

		while(index < nnz_cols){
			//std::cout<<"fill\n";
			col_pos = col_base + 4*col_idx;
			val_idx = col_idx;	
			///col_idx++;
			int32_t col_val;
			//std::cout<<"idx:"<<index<<" "<<nnz_cols<<"\n";
			mmu->load_slow_path(proc, col_pos, 4, (uint8_t*)&col_val, req_host);
			///index++;					
			//std::cout<<"col:"<<col_val<<" "<<previous_processed_col<<"\n";	

			if((col_val - previous_processed_col) > 8) {
				//index--;
				//col_idx--;
				//fill buffer with zeros
				while(b[fill_buffer_id].buffer_rd_idx < buffer_sz){
					fill_zero(p);
					//cout<<"fill\n";
					//check_buffer();
					//cout<<"-"<<col_element_count<<"\n";
				}
				break;
			}
			else if(col_val - previous_processed_col > 1){
				//index--;
				//col_idx--;
				int next_idx = col_val - previous_processed_col;
				while(next_idx>1 && b[fill_buffer_id].buffer_rd_idx < buffer_sz){
					next_idx--;
					fill_zero(p);
					//cout<<"fill\n";
					//cout<<"*"<<col_element_count<<" "<<b[fill_buffer_id].buffer_rd_idx<<"\n";
					//check_buffer();
				}	
				break;
			}
			index++;
			col_idx++;
		
			int32_t val_pos;
			val_pos = value_base + 4*val_idx;
			//std::cout<<"pos:"<<val_pos<<"\n";
			mmu->load_slow_path(proc, val_pos, 4, (uint8_t*)p, req_host);
			b[fill_buffer_id].buffer_rd_idx += 4;	
			//cout<<"fill-nnz\n";
			//check_buffer();
			col_element_count++;

			previous_processed_col = col_val;

			//std::cout<<"val:"<<*(int*)p<<" "<<index<<" "<<nnz_cols<<" "<<b[fill_buffer_id].buffer_rd_idx <<" "<<col_element_count<<"\n";		    
			break;
		}
		
		if(index == nnz_cols){
			//end of column, if buffer is empty fill up with 0's
				
			while(b[fill_buffer_id].buffer_rd_idx < buffer_sz && !b[fill_buffer_id].buffer_full){
				fill_zero(p);
				//cout<<"fill\n";
				//check_buffer();
			}
			fill_all_zero = true;
			//ready_flag = true;
			//cout<<col_element_count<<" "<<n<<"\n";
			if(col_element_count == n)
			{
				//cout<<"row_done\n";
				fill_all_zero = false;
				row_done = true;	//expansion for this row done
			}
			//cout<<col_element_count<<" "<<nnz_cols<<"\n";
		}
		//cout<<b[fill_buffer_id].buffer_rd_idx<<"\n";
		check_buffer();	
		//cout<<"buffer_full:"<<b[fill_buffer_id].buffer_full<<" "<<b[fill_buffer_id].buffer_rd_idx<<"\n";

		under_process = false;
		return;
    }
	else{
		//scroll throw buffers?
	} 
}

//CSE:BV
void buffer_controller_t::fill_buffer_bv_exp()
{
    uint8_t * p;
    //std::cout<<start<<"--"<<b[fill_buffer_id].buffer_full<<"\n";
    if (start && !b[fill_buffer_id].buffer_full)        //bc enabled and buffer empty
    {
        //std::cout<<"fill"<<"\n";
        under_process = true;
        bool req_host = 1;

        if(first_time){
            bv_pos = bv_base;
            //mmu->load_slow_path(proc, row_pos, 4, (uint8_t*)&nnz_present_row, req_host);
            first_time = false;
        }

        //std::cout<<"idx:"<<bv_idx<<"\n";
        //cout<<"left_cluster:"<<row_clusters<<" "<<col_seg<<" "<< (n/buffer_sz) <<"\n";
        if( bv_idx < (n * n)/8 && col_idx_done == false){       //if this row has more clusters
            //if(byte_word == 0){
            //cout<<"addr:"<<bv_pos<<"\n";
            mmu->load_slow_path(proc, bv_pos, 1, (uint8_t*)&bv_val, req_host);
            bv_pos = bv_pos + 1;
            bv_idx++;
            //}
            //cout<<"addr:"<<bv_pos<<"\n";
            //std::bitset<8> bin(bv_val);
            //cout<<"fill:"<<bin<<"\n";
            fill_nnz_array(bv_val, byte_word);
            //byte_word++;
            //if(byte_word == 4)
            //  byte_word = 0;
            col_idx_done = true;
        }
        else if( bv_idx == (n * n)/8 && col_idx_done == false){
            start = false;
            return;
        }
		if(col_idx_done){
            for(int i=resume_pos; i<nnz_array.size(); i++){
                p = (uint8_t*) &(b[fill_buffer_id].buffer[b[fill_buffer_id].buffer_rd_idx]);
                //cout<<"val_fill\n";
                if(nnz_array[i] != -1){
                    int32_t val_pos;
                    val_pos = value_base + 4*val_idx;
                    //std::cout<<"pos:"<<val_pos<<"\n";
                    mmu->load_slow_path(proc, val_pos, 4, (uint8_t*)p, req_host);
                    val_idx++;
                    resume_pos = i+1;
                    b[fill_buffer_id].buffer_rd_idx += 4;
                    //cout<<"fill-nnz\n";
                    check_buffer();
                    return;
                }
                else{
                    fill_zero(p);
                    b[fill_buffer_id].buffer_rd_idx += 4;
                    check_buffer();
                }
                //b[fill_buffer_id].buffer_rd_idx += 4;
                //cout<<"fill-nnz\n";
                //check_buffer();
                //col_element_count++;

                //previous_processed_col = col_val;

                //break;
            }
        }

        //cout<<b[fill_buffer_id].buffer_rd_idx<<"\n";
        check_buffer();
        //cout<<"buffer_full:"<<b[fill_buffer_id].buffer_full<<" "<<b[fill_buffer_id].buffer_rd_idx<<"\n";

        under_process = false;
        return;
    }
    else{
	}
}

//CASE:RL
void buffer_controller_t::fill_buffer_rl_exp()
{
    uint8_t * p;
    //std::cout<<start<<"--"<<b[fill_buffer_id].buffer_full<<"\n";
    if (start && !b[fill_buffer_id].buffer_full)        //bc enabled and buffer empty
    {
        //std::cout<<"fill"<<"\n";
        under_process = true;
        bool req_host = 1;

        if(first_time){
            row_pos = row_base;
            //mmu->load_slow_path(proc, row_pos, 4, (uint8_t*)&nnz_present_row, req_host);
            first_time = false;
        }

        if(row_done){
        //cout<<col_idx<<"-"<<n<<"\n";
        //cout<<"new_row\n";
        col_element_count = 0;
            if(present_row < n){
                index = 0;
                mmu->load_slow_path(proc, row_pos, 4, (uint8_t*)&row_clusters, req_host);
                row_pos = row_pos + 4;
                present_row++;
                //std::cout<<row_clusters<<"-\n";
                previous_processed_col = -1;    //useful in while loop
                col_seg = 0;
                processed_cols = 0;
                /*if(nnz_cols == 0)
                {
                    row_done = true;
                    return;
                }*/
                //if there is a pending count across row


            }
			else{
                //cout<<"done\n";
                done = true;
                col_idx = 0;
                nnz_next_row = 0;
                present_col = -1;
                start = false;
                return;
            }
            row_done = false;
        }

        //cout<<"left_cluster:"<<row_clusters<<" "<<col_seg<<" "<< (n/buffer_sz) <<"\n";
        if((row_clusters > 0 || col_seg < (n/(buffer_sz/4))) && col_idx_done == false){     //if this row has more clusters
            //leftovers from previous cluster
            if(pending_previous_cluster_num > 0){
                //cout<<"*pending\n";
                for(int i=0; i<pending_previous_cluster_num; i++){
                    nnz_array[i % (buffer_sz/4)] = pending_previous_cluster_start;
                    pending_previous_cluster_start++;
                    if(i == ((buffer_sz/4)-1) ){
                        //if vector is full but there are pending indeces left previous cluster copy them
                        pending_previous_cluster_num = ((pending_previous_cluster_num-1)-i);
                        //cout<<"*pending_count:"<<pending_previous_cluster_num<<" "<<pending_previous_cluster_start<<"\n";
                        col_idx_done = true;    //this indicates that col indeces are avaiable, values needs to be pushed to buffer.
                        resume_pos = 0;
                        col_seg++;
                        return;
                    }
                }
                pending_previous_cluster_num = 0;           //reset pending count
                //row_cluster--;
            }

            //starting a new cluster
            int32_t num_col, start_col;
            while(processed_cols < (buffer_sz/4)){
                col_pos = col_base + 4*col_idx;
                mmu->load_slow_path(proc, col_pos, 4, (uint8_t*)&num_col, req_host);
                col_idx++;
				col_pos = col_base + 4*col_idx;
                mmu->load_slow_path(proc, col_pos, 4, (uint8_t*)&start_col, req_host);
                col_idx++;

                processed_cols += num_col;

                //cout<<"start_col:"<<start_col<<"-"<<num_col<<" "<<start_col/(buffer_sz/4)<<"-"<<col_seg<<"\n";

                int32_t index = start_col;
                int32_t last_entry;         //will be used to track if vector overflowed with the cluster. then cluster is broken down and its information is stored in variables
                if(col_seg == start_col/(buffer_sz/4)){
                    for(int i=0; i<num_col; i++){
                        //cout<<"idx_fill:"<<index % (buffer_sz/4)<<"\n";
                        nnz_array[index % (buffer_sz/4)] = index;
                        last_entry = index % (buffer_sz/4);
                        index++;
                        if(index % (buffer_sz/4) < last_entry){             //new entry pointing to top of vector, so cluster is broken and remaining data are stored
                            //if vector is full but there are pending indeces left previous cluster copy them
                            pending_previous_cluster_num = ((num_col-1)-i);
                            pending_previous_cluster_start = index;
                            //cout<<"push_pending:"<<pending_previous_cluster_num<<" "<<pending_previous_cluster_start<<"\n";
                            col_idx_done = true;    //this indicates that col indeces are avaiable, values needs to be pushed to buffer.
                            col_seg++;  //since this vector size col segment is completed
                            resume_pos = 0;
                            processed_cols = 0;     //since vector is filled
                            row_clusters--;
                            return;
                        }
                    }
                    row_clusters--;
                }
                else{
                    col_idx = col_idx - 2;      //need to optimize but subtracting since not used
                    //cout<<"no_idx_fill\n";
                    col_idx_done = true;
                    resume_pos = 0;
                    col_seg++;
                    processed_cols = 0;     //since vector is filled
                    return;
				}
            }

        }
        else if(row_clusters == 0 && col_idx_done == false){
            row_done = true;
        }

        if(col_idx_done){
            for(int i=resume_pos; i<nnz_array.size(); i++){
                p = (uint8_t*) &(b[fill_buffer_id].buffer[b[fill_buffer_id].buffer_rd_idx]);                                                                                                  //cout<<"val_fill\n";
                if(nnz_array[i] != -1){
                    int32_t val_pos;
                    val_pos = value_base + 4*val_idx;
                    //std::cout<<"pos:"<<val_pos<<"\n";
                    mmu->load_slow_path(proc, val_pos, 4, (uint8_t*)p, req_host);
                    val_idx++;
                    resume_pos = i+1;
                    b[fill_buffer_id].buffer_rd_idx += 4;
                    //cout<<"fill-nnz\n";
                    check_buffer();
                    return;
                }
                else{
                    fill_zero(p);
                    b[fill_buffer_id].buffer_rd_idx += 4;
                    check_buffer();
                }
                //b[fill_buffer_id].buffer_rd_idx += 4;
                //cout<<"fill-nnz\n";
                //check_buffer();
                //col_element_count++;

                //previous_processed_col = col_val;

                //break;
            }
        }
		check_buffer();
        //cout<<"buffer_full:"<<b[fill_buffer_id].buffer_full<<" "<<b[fill_buffer_id].buffer_rd_idx<<"\n";

        under_process = false;
        return;
    }
    else{
        //scroll throw buffers?
    }
}



void buffer_controller_t::fill_nnz_array(int8_t &bv_val, int byte_word)
{
    long zero = 1, one = 2, two = 4, three = 8, four = 16, five = 32, six = 64, seven = 128;
    int mul = 1;
    /*if(byte_word == 0)
        mul = 1;
    else if(byte_word == 1)
        mul = 256;
    else if(byte_word == 2)
        mul = 65536;
    else if(byte_word == 3)
        mul = 16777216;*/


    if(bv_val & (mul * zero)) nnz_array[0] = 1;
    if(bv_val & (mul * one)) nnz_array[1] = 1;
    if(bv_val & (mul * two)) nnz_array[2] = 1;
    if(bv_val & (mul * three)) nnz_array[3] = 1;
    if(bv_val & (mul * four)) nnz_array[4] = 1;
    if(bv_val & (mul * five)) nnz_array[5] = 1;
    if(bv_val & (mul * six)) nnz_array[6] = 1;
    if(bv_val & (mul * seven)) nnz_array[7] = 1;

    //for(int i=0; i<nnz_array.size(); i++)
    //  cout<<nnz_array[i]<<" ";
    //cout<<"\n";
    //exit(0);
}


void buffer_controller_t::fill_zero(uint8_t *p)
{
	p = (uint8_t*) &(b[fill_buffer_id].buffer[b[fill_buffer_id].buffer_rd_idx]);
	*p = 0;                 
	*(p+1) = 0;                 
	*(p+2) = 0;                 
	*(p+3) = 0;                 
	col_element_count++;
	previous_processed_col++;
	if(op == 0 && format == 0)
		b[fill_buffer_id].buffer_rd_idx += 4;
}

void buffer_controller_t::check_buffer()
{
	if(b[fill_buffer_id].buffer_rd_idx == buffer_sz){	
		b[fill_buffer_id].buffer_rd_idx = 0;
		b[fill_buffer_id].buffer_full = 1;
		//cout<<"buffer_full\n";
		if(op == 0 && (format == 1 || format == 2))
			col_idx_done = false;
		if(op == 0 && format == 1)
			resume_pos = 0;
		if(fill_buffer_id < (buffer_count-1))
			fill_buffer_id++;
		else
			fill_buffer_id = 0;
	}
}

uint32_t buffer_controller_t::read_buffer()
{
	if(b[read_buffer_id].buffer_full)
	{
		uint32_t value = *(uint32_t*)&(b[read_buffer_id].buffer[b[read_buffer_id].buffer_rd_idx]);	
		//std::cout<<"read:"<<value<<"\n";
		//fflush(0);
		b[read_buffer_id].buffer_rd_idx += 4;
		if (b[read_buffer_id].buffer_rd_idx == buffer_sz){
			//std::cout<<"read_counter:"<<buffer_read_count<<"\n";
			//fflush(0);
			if(op == 0)
				std::fill(nnz_array.begin(), nnz_array.end(), -1);
			buffer_read_count++;
			b[read_buffer_id].buffer_rd_idx = 0;
			b[read_buffer_id].buffer_full = 0;
			if(read_buffer_id < (buffer_count-1))
				read_buffer_id++;
			else
				read_buffer_id = 0;
		}
		//std::cout<<"read_buf:"<<value<<"\n";
		return value;
	}
	else{
		//scroll through buffers?
	}
	cout<<"Error\n";
	return 0;
}

void buffer_controller_t::write32(reg_t addr, uint8_t * data)
{
    if (addr >= CONFIG_RANGE)
    {
        /*strncpy((char*)&buffer[buffer_wr_idx], (const char*)data, 4);
        buffer_wr_idx+=4;
        if (buffer_wr_idx == buffer_sz) buffer_wr_idx = 0;*/
    } else {
        uint32_t val = *(uint32_t*)data;
		std::cout<<"addr:"<<addr<<" val:"<<val<<"\n";
		fflush(0);
        switch (addr)
        {
        case OFFSET_EC: { op = (operation_t) val; break; }
        case OFFSET_STORAGE_TYPE: { format = (format_t) val; break; }
        case OFFSET_N: { n=val; break; }
        case OFFSET_ROWS:{ row_base=val; break; }
        case OFFSET_COLS:{ col_base=val; break; }
        case OFFSET_V:   { v_base=val; break; }
        case OFFSET_VALS:   { value_base=val; break; }
        case OFFSET_BITVECTOR:   { bv_base=val; break; }
        case OFFSET_BID: { buf_id=val; break; }
        case OFFSET_START: { start=val; break; }
        case OFFSET_NNZ: { nnz=val; break; }
        default: break;
		}
    } 
}
