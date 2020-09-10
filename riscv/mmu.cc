// See LICENSE for license details.

#include "mmu.h"
#include "simif.h"
#include "processor.h"
#include "memdelay.h"
#include "prefetcher.h"
#include "flashmodel.h"

mmu_t::mmu_t(simif_t* sim, processor_t* proc)
 : sim(sim), proc(proc),
  check_triggers_fetch(false),
  check_triggers_load(false),
  check_triggers_store(false),
  matched_trigger(NULL)
{
  //md = new memdelay_t();
  pf = new prefetcher_t(this, proc);
  fm = new flashmodel_t(this);
  pf->log_lines = log_lines;
  //pf = new prefetcher_t(proc);
  flush_tlb();
  yield_load_reservation();
}

mmu_t::~mmu_t()
{
 //delete md;
  delete pf;
  delete fm;
  printf("Prefetch wait times:%ld\n",prefetch_wait);
}

void mmu_t::flush_icache()
{
  for (size_t i = 0; i < ICACHE_ENTRIES; i++)
    icache[i].tag = -1;
}

void mmu_t::flush_tlb()
{
  memset(tlb_insn_tag, -1, sizeof(tlb_insn_tag));
  memset(tlb_load_tag, -1, sizeof(tlb_load_tag));
  memset(tlb_store_tag, -1, sizeof(tlb_store_tag));

  flush_icache();
}

static void throw_access_exception(reg_t addr, access_type type)
{
  switch (type) {
    case FETCH: throw trap_instruction_access_fault(addr);
    case LOAD: throw trap_load_access_fault(addr);
    case STORE: throw trap_store_access_fault(addr);
    default: abort();
  }
}

reg_t mmu_t::translate(reg_t addr, reg_t len, access_type type)
{
  if (!proc)
    return addr;

  reg_t mode = proc->state.prv;
  if (type != FETCH) {
    if (!proc->state.debug_mode && get_field(proc->state.mstatus, MSTATUS_MPRV))
      mode = get_field(proc->state.mstatus, MSTATUS_MPP);
  }

  reg_t paddr = walk(addr, type, mode) | (addr & (PGSIZE-1));
  if (!pmp_ok(paddr, len, type, mode))
  {
    throw_access_exception(addr, type);
  }
  return paddr;
}

tlb_entry_t mmu_t::fetch_slow_path(reg_t vaddr)
{
  reg_t paddr = translate(vaddr, sizeof(fetch_temp), FETCH);

  if (auto host_addr = sim->addr_to_mem(paddr)) {
    return refill_tlb(vaddr, paddr, host_addr, FETCH);
  } else {
    if (!mmio_load(paddr, sizeof fetch_temp, (uint8_t*)&fetch_temp))
      throw trap_instruction_access_fault(vaddr);
    tlb_entry_t entry = {(char*)&fetch_temp - vaddr, paddr - vaddr};
    return entry;
  }
}

reg_t reg_from_bytes(size_t len, const uint8_t* bytes)
{
  switch (len) {
    case 1:
      return bytes[0];
    case 2:
      return bytes[0] |
        (((reg_t) bytes[1]) << 8);
    case 4:
      return bytes[0] |
        (((reg_t) bytes[1]) << 8) |
        (((reg_t) bytes[2]) << 16) |
        (((reg_t) bytes[3]) << 24);
    case 8:
      return bytes[0] |
        (((reg_t) bytes[1]) << 8) |
        (((reg_t) bytes[2]) << 16) |
        (((reg_t) bytes[3]) << 24) |
        (((reg_t) bytes[4]) << 32) |
        (((reg_t) bytes[5]) << 40) |
        (((reg_t) bytes[6]) << 48) |
        (((reg_t) bytes[7]) << 56);
  }
  abort();
}

bool mmu_t::mmio_ok(reg_t addr, access_type type)
{
  // Disallow access to debug region when not in debug mode
  if (addr >= DEBUG_START && addr <= DEBUG_END && !proc->state.debug_mode)
    return false;

  return true;
}

bool mmu_t::mmio_load(reg_t addr, size_t len, uint8_t* bytes)
{
  if (!mmio_ok(addr, LOAD))
    return false;

  return sim->mmio_load(addr, len, bytes);
}

bool mmu_t::mmio_store(reg_t addr, size_t len, const uint8_t* bytes)
{
  if (!mmio_ok(addr, STORE))
    return false;

  return sim->mmio_store(addr, len, bytes);
}

void mmu_t::tick()
{
  //process prefetch requests; since processor cannot see prefetcher, mmu does it
  if(pf_req_wait){
	if(fm->search_line_queue(pf_req_wait)){		//receives if this request is done or not; so that CPU can process the tick
	  //cout<<"avaiable at:"<<proc->clk<<"\n";
  	  pf_req_wait = 0;
	}
	else
	  prefetch_wait++;
  }

  //
	
}

void mmu_t::load_slow_path(processor_t *proc, reg_t addr, reg_t len, uint8_t* bytes, bool req_host)
{
  reg_t paddr = translate(addr, len, LOAD);
  //cout<<"addr:"<<addr<<" "<<paddr<<"\n";
  int L2_miss = 0;		//if L2_miss is 0, L1 hit; L2_miss == 1, L2 miss; L2_miss == 2, L2 hit.
  if (auto host_addr = sim->addr_to_mem(paddr)) {
    memcpy(bytes, host_addr, len);
    if (tracer.interested_in_range(paddr, paddr + PGSIZE, LOAD))
      L2_miss = tracer.trace(paddr, len, LOAD);
    else
      refill_tlb(addr, paddr, host_addr, LOAD);
  } else if (!mmio_load(paddr, len, bytes)) {  
    throw trap_load_access_fault(addr);
  }
  if (!matched_trigger) {
    reg_t data = reg_from_bytes(len, bytes);
    matched_trigger = trigger_exception(OPERATION_LOAD, addr, data);
    if (matched_trigger)
    {
      throw *matched_trigger;
    }
  }

   //if req is L2 miss, check prefetch buffer.
  bool sb_hit = false;
  reg_t line = paddr >> log_lines;
  line = line << log_lines;         //line size is 32 bytes in this case
  if(proc && req_host == 0 && L2_miss == 1 && pf->sb_size){
  	  //check if this request in in prefetch pending
	  vector<reg_t>::iterator it_pp;
	  it_pp = find(pf->pp->begin(), pf->pp->end(), line);
	  if(it_pp != pf->pp->end())	//matched
	  {
		//wait until it is cleared.
		pf_req_wait = line;//paddr;
		while(pf_req_wait){
			proc->tick();
		}
	  }
      sb_hit = pf->check_sb(line);
  }

  int64_t t1 = 0,t2 = 0;
  if(!sb_hit){
	  req_satisfied = false;
	  if(proc && L2_miss == 1){		//L1 miss
		fm->enqueue(paddr,1, req_host);
		if(req_host == 1)
			proc->bc->busy = true;	//if this request is BC, bc is put busy
		t1 = proc->clk;
		while(!req_satisfied)	//req_Satisfied	//it was md->req_satisfied
		{
			proc->tick();
		}
		t2 = proc->clk;
		if(req_host == 1)
			proc->bc->busy = false;
	  } 
  }
  
  if(proc && req_host == 0 && proc->CSR_flag ) {//adding mem wait cycles to memory_wait_cycles; represent mem. wait times; CSR_flag makes sure the control is in between tick functions
	if(L2_miss == 1 && !sb_hit)	//L1 miss
	{
  		proc->memory_wait_cycles += t2-t1;
  	}
	else if (sb_hit && L2_miss == 1)
  		proc->memory_wait_cycles += t2-t1;
  }
  if(proc && req_host == 0 && L2_miss == 1 && pf->prefetch_flag/*&& pf->sb_size*/)         //onfly CPU req go through pf and in the end "prefetch" is the flag to enable prefetch
    pf->prefetch(line);


  if(proc)	//for processing vector loads/stores; maintain updated clock
  	temp_clk = proc->clk;
}

void mmu_t::store_slow_path(reg_t addr, reg_t len, const uint8_t* bytes)
{
  reg_t paddr = translate(addr, len, STORE);

  if (!matched_trigger) {
    reg_t data = reg_from_bytes(len, bytes);
    matched_trigger = trigger_exception(OPERATION_STORE, addr, data);
    if (matched_trigger)
      throw *matched_trigger;
  }

  int L2_miss = 0;
  if (auto host_addr = sim->addr_to_mem(paddr)) {
    memcpy(host_addr, bytes, len);
    if (tracer.interested_in_range(paddr, paddr + PGSIZE, STORE))
      L2_miss = tracer.trace(paddr, len, STORE);
    else
      refill_tlb(addr, paddr, host_addr, STORE);
  } else if (!mmio_store(paddr, len, bytes)) {
    throw trap_store_access_fault(addr);
  }
  
  int64_t t1 = 0,t2 = 0;
  req_satisfied = false;
  if(proc && L2_miss == 1){
	fm->enqueue(paddr,2, 0);
	t1 = proc->clk;
	while(!req_satisfied)	//req_Satisfied	
  	{
		proc->tick();
	}
	t2 = proc->clk;
  }
  
  if(proc && proc->CSR_flag ) {//adding mem wait cycles to memory_wait_cycles; represent mem. wait times; CSR_flag makes sure the control is in between tick functions
	if(L2_miss == 1)	//L1 miss
  		proc->memory_wait_cycles += t2-t1;
  }
  
  if(proc)
  	temp_clk = proc->clk;
}

tlb_entry_t mmu_t::refill_tlb(reg_t vaddr, reg_t paddr, char* host_addr, access_type type)
{
  reg_t idx = (vaddr >> PGSHIFT) % TLB_ENTRIES;
  reg_t expected_tag = vaddr >> PGSHIFT;

  if ((tlb_load_tag[idx] & ~TLB_CHECK_TRIGGERS) != expected_tag)
    tlb_load_tag[idx] = -1;
  if ((tlb_store_tag[idx] & ~TLB_CHECK_TRIGGERS) != expected_tag)
    tlb_store_tag[idx] = -1;
  if ((tlb_insn_tag[idx] & ~TLB_CHECK_TRIGGERS) != expected_tag)
    tlb_insn_tag[idx] = -1;

  if ((check_triggers_fetch && type == FETCH) ||
      (check_triggers_load && type == LOAD) ||
      (check_triggers_store && type == STORE))
    expected_tag |= TLB_CHECK_TRIGGERS;

  if (pmp_homogeneous(paddr & ~reg_t(PGSIZE - 1), PGSIZE)) {
    if (type == FETCH) tlb_insn_tag[idx] = expected_tag;
    else if (type == STORE) tlb_store_tag[idx] = expected_tag;
    else tlb_load_tag[idx] = expected_tag;
  }

  tlb_entry_t entry = {host_addr - vaddr, paddr - vaddr};
  tlb_data[idx] = entry;
  return entry;
}

reg_t mmu_t::pmp_ok(reg_t addr, reg_t len, access_type type, reg_t mode)
{
  if (!proc)
    return true;

  reg_t base = 0;
  for (size_t i = 0; i < proc->state.n_pmp; i++) {
    reg_t tor = proc->state.pmpaddr[i] << PMP_SHIFT;
    uint8_t cfg = proc->state.pmpcfg[i];

    if (cfg & PMP_A) {
      bool is_tor = (cfg & PMP_A) == PMP_TOR;
      bool is_na4 = (cfg & PMP_A) == PMP_NA4;

      reg_t mask = (proc->state.pmpaddr[i] << 1) | (!is_na4);
      mask = ~(mask & ~(mask + 1)) << PMP_SHIFT;

      // Check each 4-byte sector of the access
      bool any_match = false;
      bool all_match = true;
      for (reg_t offset = 0; offset < len; offset += 1 << PMP_SHIFT) {
        reg_t cur_addr = addr + offset;
        bool napot_match = ((cur_addr ^ tor) & mask) == 0;
        bool tor_match = base <= cur_addr && cur_addr < tor;
        bool match = is_tor ? tor_match : napot_match;
        any_match |= match;
        all_match &= match;
      }

      if (any_match) {
        // If the PMP matches only a strict subset of the access, fail it
        if (!all_match)
          return false;

        return
          (mode == PRV_M && !(cfg & PMP_L)) ||
          (type == LOAD && (cfg & PMP_R)) ||
          (type == STORE && (cfg & PMP_W)) ||
          (type == FETCH && (cfg & PMP_X));
      }
    }

    base = tor;
  }

  return mode == PRV_M;
}

reg_t mmu_t::pmp_homogeneous(reg_t addr, reg_t len)
{
  if ((addr | len) & (len - 1))
    abort();

  if (!proc)
    return true;

  reg_t base = 0;
  for (size_t i = 0; i < proc->state.n_pmp; i++) {
    reg_t tor = proc->state.pmpaddr[i] << PMP_SHIFT;
    uint8_t cfg = proc->state.pmpcfg[i];

    if (cfg & PMP_A) {
      bool is_tor = (cfg & PMP_A) == PMP_TOR;
      bool is_na4 = (cfg & PMP_A) == PMP_NA4;

      bool begins_after_lower = addr >= base;
      bool begins_after_upper = addr >= tor;
      bool ends_before_lower = (addr & -len) < (base & -len);
      bool ends_before_upper = (addr & -len) < (tor & -len);
      bool tor_homogeneous = ends_before_lower || begins_after_upper ||
        (begins_after_lower && ends_before_upper);

      reg_t mask = (proc->state.pmpaddr[i] << 1) | (!is_na4);
      mask = ~(mask & ~(mask + 1)) << PMP_SHIFT;
      bool mask_homogeneous = ~(mask << 1) & len;
      bool napot_homogeneous = mask_homogeneous || ((addr ^ tor) / len) != 0;

      if (!(is_tor ? tor_homogeneous : napot_homogeneous))
        return false;
    }

    base = tor;
  }

  return true;
}

reg_t mmu_t::walk(reg_t addr, access_type type, reg_t mode)
{
  vm_info vm = decode_vm_info(proc->max_xlen, mode, proc->get_state()->satp);
  if (vm.levels == 0)
    return addr & ((reg_t(2) << (proc->xlen-1))-1); // zero-extend from xlen

  bool s_mode = mode == PRV_S;
  bool sum = get_field(proc->state.mstatus, MSTATUS_SUM);
  bool mxr = get_field(proc->state.mstatus, MSTATUS_MXR);

  // verify bits xlen-1:va_bits-1 are all equal
  int va_bits = PGSHIFT + vm.levels * vm.idxbits;
  reg_t mask = (reg_t(1) << (proc->xlen - (va_bits-1))) - 1;
  reg_t masked_msbs = (addr >> (va_bits-1)) & mask;
  if (masked_msbs != 0 && masked_msbs != mask)
    vm.levels = 0;

  reg_t base = vm.ptbase;
  for (int i = vm.levels - 1; i >= 0; i--) {
    int ptshift = i * vm.idxbits;
    reg_t idx = (addr >> (PGSHIFT + ptshift)) & ((1 << vm.idxbits) - 1);

    // check that physical address of PTE is legal
    auto pte_paddr = base + idx * vm.ptesize;
    auto ppte = sim->addr_to_mem(pte_paddr);
    if (!ppte || !pmp_ok(pte_paddr, vm.ptesize, LOAD, PRV_S))
      throw_access_exception(addr, type);

    reg_t pte = vm.ptesize == 4 ? from_le(*(uint32_t*)ppte) : from_le(*(uint64_t*)ppte);
    reg_t ppn = pte >> PTE_PPN_SHIFT;

    if (PTE_TABLE(pte)) { // next level of page table
      base = ppn << PGSHIFT;
    } else if ((pte & PTE_U) ? s_mode && (type == FETCH || !sum) : !s_mode) {
      break;
    } else if (!(pte & PTE_V) || (!(pte & PTE_R) && (pte & PTE_W))) {
      break;
    } else if (type == FETCH ? !(pte & PTE_X) :
               type == LOAD ?  !(pte & PTE_R) && !(mxr && (pte & PTE_X)) :
                               !((pte & PTE_R) && (pte & PTE_W))) {
      break;
    } else if ((ppn & ((reg_t(1) << ptshift) - 1)) != 0) {
      break;
    } else {
      reg_t ad = PTE_A | ((type == STORE) * PTE_D);
#ifdef RISCV_ENABLE_DIRTY
      // set accessed and possibly dirty bits.
      if ((pte & ad) != ad) {
        if (!pmp_ok(pte_paddr, vm.ptesize, STORE, PRV_S))
          throw_access_exception(addr, type);
        *(uint32_t*)ppte |= to_le((uint32_t)ad);
      }
#else
      // take exception if access or possibly dirty bit is not set.
      if ((pte & ad) != ad)
        break;
#endif
      // for superpage mappings, make a fake leaf PTE for the TLB's benefit.
      reg_t vpn = addr >> PGSHIFT;
      reg_t value = (ppn | (vpn & ((reg_t(1) << ptshift) - 1))) << PGSHIFT;
      return value;
    }
  }

  switch (type) {
    case FETCH: throw trap_instruction_page_fault(addr);
    case LOAD: throw trap_load_page_fault(addr);
    case STORE: throw trap_store_page_fault(addr);
    default: abort();
  }
}

void mmu_t::register_memtracer(memtracer_t* t)
{
  flush_tlb();
  tracer.hook(t);
}
