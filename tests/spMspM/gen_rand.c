#include <iostream>
#include <stdlib.h>
#include <sstream>
#include <fstream>
#include <time.h>
using namespace std;

#define NUM_ARGS 5

void usage(void)
{
    cout <<"This program takes " << NUM_ARGS << " arguments: " << endl;
    cout <<"Number of dimensions [1 or 2]" << endl;
    cout <<"Size of dimension [for 2-dimensional, assume square shape]" << endl;
    cout <<"\% Sparsity desired" << endl;
    cout <<"Output C header file path-name" << endl;
    cout <<"Output data file prefix (without file extension)" << endl;
}

/* output goes to globals */
int *w;
int nnz_vec;

/* 2-dim */
int *rows;
int *cols;
int *vals;
int **A;
void genData(int num_dim, int dim_size, float sparsity);
void outData(ofstream& header, ofstream& data, int num_dim, int dim_size, bool compressed=true);
void outData1(ofstream& header, ofstream& data, int dim_size, bool compressed=true);
void outData2(ofstream& header, ofstream& data, int dim_size, bool compressed=true);

int main(int argc, char ** argv)
{
    if (argc <= NUM_ARGS) {
        usage();
        exit(0);
    }

    stringstream in_dim(argv[1]); 
    stringstream in_size(argv[2]); 
    stringstream in_sparse(argv[3]); 
    stringstream in_out_h_file(argv[4]); 
    stringstream in_out_data_file(argv[5]); 

    int num_dim;
    in_dim >> num_dim;
    int dim_size;
    in_size >> dim_size;
    float sparse;
    in_sparse >> sparse;

    /* generate data */
    genData(num_dim, dim_size, sparse);

    string out_h_file_name;
    string out_compressed_data_file_name;
    string out_uncompressed_data_file_name;
    in_out_h_file >> out_h_file_name;
    in_out_data_file >> out_compressed_data_file_name;
    out_uncompressed_data_file_name = out_compressed_data_file_name;
    out_compressed_data_file_name += "_csr.dat";
    out_uncompressed_data_file_name += "_dense.dat";
    ofstream out_h_file;
    ofstream out_compressed_data_file;
    ofstream out_uncompressed_data_file;
    out_h_file.open(out_h_file_name); 
    out_compressed_data_file.open(out_compressed_data_file_name); 
    out_uncompressed_data_file.open(out_uncompressed_data_file_name); 
    
    out_h_file << "#ifndef __DATA_H" << endl;
    out_h_file << "#define __DATA_H" << endl;

    /* write data */
    outData(out_h_file, out_compressed_data_file, num_dim, dim_size, true);
    outData(out_h_file, out_uncompressed_data_file, num_dim, dim_size, false);
    
    out_h_file << "#endif" << endl;

    return 0;
}

void genData(int num_dim, int dim_size, float sparsity)
{
    srand(time(NULL));
    cout <<"Sparsity " << sparsity << endl;
    if (num_dim == 1) {
       w = new int[dim_size];

       for (int i=0;i<dim_size;i++)
       {
           int x = (int)rand()%100;
           if (x <= sparsity) x = 0;
           w[i] = x;
           if (w[i] != 0) {
               nnz_vec++;
           }
       }
    } else {
       A = new int*[dim_size];
       for (int i=0;i<dim_size;i++) A[i] = new int[dim_size];
       for (int i=0;i<dim_size;i++)
       for (int j=0;j<dim_size;j++)
       {
           int x = (int)rand()%100;
           if (x <= sparsity) x = 0;
           A[i][j] = x;
       }
    }
}

void outData(ofstream& out_h, ofstream& out_data, int num_dim, int dim_size, bool compressed)
{
    if (num_dim == 1) outData1(out_h, out_data, dim_size, compressed);
    if (num_dim == 2) outData2(out_h, out_data, dim_size, compressed);
}

void outData1(ofstream& out_h, ofstream & out_data, int dim_size, bool compressed)
{
    // write dense first
    out_h << "int n_vec = " << dim_size << ";"<<endl;
    out_h << "int vec_dense["<<dim_size<<"] = {"<<endl;
    for (int i=0;i<dim_size;i++)
    {
        out_h << w[i];
        if (i<dim_size-1) {
            out_h << ",";
        }
    } 
    out_h << "};"<<endl;

    // write sparse;
    out_h << "int nnz_vec = " << nnz_vec << ";"<<endl;

    out_h << "int vec_sparse_csr_value["<<nnz_vec<<"]={"<<endl;
    int j=0;
    for (int i=0;i<dim_size;i++)
    {
        if (w[i] > 0) {
            out_h << w[i];
            if (j < nnz_vec-1) {
                out_h << ",";
            }
            j++;
        }
    }
    out_h << "};"<<endl;
    out_h << "int vec_sparse_csr_index["<<nnz_vec<<"]={"<<endl;
    j=0;
    for (int i=0;i<dim_size;i++)
    {
        if (w[i] > 0) {
            out_h << i;
            if (j < nnz_vec-1) {
                out_h << ",";
            }
            j++;
        }
    }
    out_h << "};"<<endl;

    // write sample w_dense
    out_h << "int w_dense["<<dim_size<<"]={"<<endl;
    for (int i=0;i<dim_size; i++)
    {
        out_h << (i+1);
        if (i<dim_size-1) out_h<<",";    
    }
    out_h << "};"<<endl;
    out_h.close();
    out_data.close();
}

void outData2(ofstream& out_h, ofstream& out_data, int dim_size, bool compressed)
{
    rows = new int[dim_size+1];
    cols = new int[dim_size * dim_size];
    vals = new int[dim_size * dim_size];

    int k=0;
    int nnz;
    rows[0]=0;
    for (int i=0;i<dim_size;i++)
    {
       nnz=0;
       for (int j=0;j<dim_size;j++)
       {
           if (A[i][j] != 0) {
              cols[k] = j;
              vals[k] = A[i][j];
              k++;
              nnz++;
           }  
       }
       rows[i+1]=rows[i]+nnz;
    }
    out_h << "int n="<<dim_size<<";"<<endl;
    out_data << dim_size << endl;

    /* write sparse matrix */
    out_h << "int A_sparse_rows["<<dim_size+1<<"]={"<<endl;
    for (int i=0;i<=dim_size;i++) {
        out_h << rows[i];
        if (compressed) out_data << rows[i] << endl;
        if (i<dim_size) out_h<<",";
        out_h << endl;
    }
    out_h << "};"<<endl;
    out_h << "int A_sparse_cols["<<k<<"]={"<<endl;
    for (int i=0;i<k;i++) {
        out_h << cols[i];
        if (compressed) out_data << cols[i] << endl;
        if (i<k-1) out_h<<",";
        out_h << endl;
    }
    out_h << "};"<<endl;
    out_h << "int A_sparse_vals["<<k<<"]={"<<endl;
    for (int i=0;i<k;i++) {
        out_h << vals[i];
        if (compressed) out_data << vals[i] << endl;
        if (i<k-1) out_h<<",";
        out_h << endl;
    }
    out_h << "};"<<endl;

    /* write dense matrix */
    out_h << "int A_dense["<<dim_size<<"]["<<dim_size<<"]={"<<endl;
    for (int i=0;i<dim_size; i++)
    {
        out_h << "{"<<endl;
        for (int j=0;j<dim_size;j++)
        {
            out_h << A[i][j];
            if (!compressed) out_data << A[i][j] << endl;
            if (j<dim_size-1) out_h << ",";
        }
        out_h << "}";
        if (i<dim_size-1) out_h<<",";    
        out_h << endl;
    }
    out_h << "};"<<endl;
    
    /* write w */
    out_h << "int w_dense["<<dim_size<<"]={"<<endl;
    for (int i=0;i<dim_size; i++)
    {
        out_h << (i+1);
        if (i<dim_size-1) out_h<<",";    
        out_data << (i+1) << endl;
    }
    out_h << "};"<<endl;
    out_h.close();
    out_data.close();
}

