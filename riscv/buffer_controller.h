#ifndef _BUFF_CTRL_H
#define _BUFF_CTRL_H

#include <stdint.h>
#include "decode.h"

// this class represents our buffer controller

/*typedef enum {
    OFFSET_N = 0,
    OFFSET_ROWS = 4,
    OFFSET_COLS = 8,
    OFFSET_V = 12,
    OFFSET_BID=16,
    OFFSET_START=24
} MMR_OFFSETS;*/

typedef enum {
    OFFSET_EC = 0, // expand or compress. 0 -- expand, 1 -- do not expand
    OFFSET_STORAGE_TYPE = 4, // CSR = 0, Bit vector = 1, run-length =2, and so on
    OFFSET_N = 8,
    OFFSET_ROWS = 12, // CSR rows address
    OFFSET_COLS = 16, // CSR cols address
    OFFSET_V = 20, // V address
    OFFSET_VALS=24, // CSR vals address
    OFFSET_BITVECTOR=28, // Bitvector address
    OFFSET_BID=32, // Buffer ID
	OFFSET_NNZ=36,
    OFFSET_START=48 // Start bit
} MMR_OFFSETS;

typedef enum {OP_EXPAND=0, OP_NO_EXPAND=1} operation_t;
typedef enum {FMT_CSR=0, FMT_BV=1, FMT_RL=2} format_t;


class buffer_controller_t
{
private:
    const unsigned int CONFIG_RANGE = 1024;
	
	unsigned int buffer_sz;
    unsigned long n;
    reg_t row_base;
    reg_t col_base;
    reg_t v_base;
	reg_t value_base;
	reg_t bv_base;
    unsigned long buf_id;
    //unsigned long col_idx; // state of buffer controller
	int buffer_count;
	int previous_processed_col;

	bool first_time = true;
    bool done = false;
    bool row_done = true;
	bool fill_all_zero = false;
	int col_element_count = 0;
    uint32_t col_idx = 0;
	bool ready_flag = false;
	bool col_idx_done = false;

    reg_t row_pos;
    reg_t row_next_pos;
    reg_t nnz_cols;
    reg_t col_pos;
	reg_t row_clusters = 0;
    reg_t nnz_next_row = 0;
    reg_t nnz_present_row = 0;
    reg_t index;
	reg_t cluster_row;

	int32_t pending_previous_cluster_start = 0;
    int32_t pending_previous_cluster_num = 0;
    int32_t col_seg = 0;
    int32_t val_idx = 0;
    int32_t present_row = 0;   //A row
    int32_t present_col = -1;   //B col or Bt row
    int32_t nnz;
    int32_t processed_cols = 0; //this counts the column entries of possibly multipl clusters for one vector
    int32_t resume_pos = 0;
	int32_t byte_word = 0;
    int8_t bv_val = 0;
    int32_t bv_idx = 0;
    reg_t bv_pos;

    mmu_t * mmu;
	processor_t *proc;
public:
    buffer_controller_t(mmu_t* mmu, processor_t* proc, unsigned int sz);
    ~buffer_controller_t();
    //void fill_buffer();
    
	void fill_buffer_csr_noexp();
    void fill_buffer_csr_exp();
    void fill_buffer_bv_exp();
    void fill_buffer_rl_exp();
    
	void fill_zero(uint8_t *p);
    uint8_t *set_metadata(reg_t addr);
    void write32(reg_t addr, uint8_t * data);
	uint32_t read_buffer();
	void write_to_buffer();
	void write_buffer();
	void check_buffer();
 	void fill_nnz_array(int8_t &bv_val, int byte_word);
	uint64_t available_clk;   
    unsigned long start;
	
	struct Buffer_t{
		uint8_t *buffer;
		unsigned int buffer_rd_idx=0;
		unsigned int buffer_wr_idx=0;
		bool buffer_full;
		uint64_t buffer_available_clk;
	};
	struct Buffer_t *b;
	int fill_buffer_id;
	int read_buffer_id;
	bool under_process = false;
	bool busy;
	
	int op;
	int format;

	//debug purpose
	long buffer_read_count = 0;
	long prev_clk = 0;

	//temp
    std::vector<int32_t> nnz_array{std::vector<int32_t>(8, -1)};
};

#endif
