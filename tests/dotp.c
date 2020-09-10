#include "sparsev.h"
#include <stdlib.h>
#include <iostream>
#include <sstream>
using namespace std;

void usage(void)
{
    cout << "One argument needed" << endl;
    cout << "0 for dense" << endl;
    cout << "1 for software-sparse" << endl;
    cout << "2 for memory-engine-sparse" << endl;
}

int sparseDotP(void)
{
    int s = 0;
    for (int i=0;i<nnz_vec; i++)
    {
        s += w_dense[vec_sparse_csr_index[i]]*vec_sparse_csr_value[i];
    }
    
    return s;
}

int denseDotP(void)
{
    int s=0;
    for (int i=0;i<n_vec;i++)
    {
        s += w_dense[i] * vec_dense[i];
    }
    return s;
}

typedef struct {
    int v;
    int w;
    //bool done;
} data_item_t;
#define VERY_LARGE_FIFO 65536
data_item_t FIFO[VERY_LARGE_FIFO];
int x,y;

void initFIFO(void)
{
    for (int i=0;i<nnz_vec;i++)
    {
        //FIFO[i].done=false;
        FIFO[i].v=vec_sparse_csr_value[i];
        FIFO[i].w=w_dense[vec_sparse_csr_index[i]];
    } 
    //FIFO[nnz_vec-1].done=true;
    x=11;y=12;
}

int memoryEngineSparseDotP(void)
{
    int s = 0;
    bool done = 0;
    int i=0;
    //while (!done)
    for (int i=0;i<nnz_vec;i++)
    {
        s+=FIFO[i].v * FIFO[i].w;
        //s+=x*y;
        //done=FIFO[i++].done;
    }
    return s;
}

int main(int argc, char**argv)
{
    if (argc <= 1)
    {
        usage();
        exit(0);
    } 
    int choice;
    stringstream in(argv[1]);
    in >> choice; 

    int s;
    int LOOP_ITER_LIMIT=65536*2;
    if (choice == 0)
    {
        for (int i=0;i<LOOP_ITER_LIMIT;i++)
        {
            s = denseDotP();
        }
        cout <<"Dense: " << s << endl;
    } else if (choice == 1)
    {
        for (int i=0;i<LOOP_ITER_LIMIT;i++)
        {
            s = sparseDotP();
        }
        cout <<"Software-Sparse: " << s << endl;
    } else {
        initFIFO();
        for (int i=0;i<LOOP_ITER_LIMIT;i++)
        {
            s = memoryEngineSparseDotP();
        }
        cout <<"Memory Engine-Sparse: " << s << endl;
    }

}
