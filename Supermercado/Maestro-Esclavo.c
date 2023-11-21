#include<stdio.h>
#include<mpi.h>
#include <stdlib.h>
#include <time.h>


#define MAESTRO 0

//Peer2Peer
struct cliente{
    int id;

    struct cliente *siguiente;
};

typedef struct persona Nodo;

Nodo *final;
Nodo *inicio;
//Final Peer2Peer


int supermercado(){

    MPI_Status status;

    int cliente;

    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    MPI_Comm_size(MPI_COMM_WORLD, &size);

    if(rank == MAESTRO){
        cajasAbiertas = size/2;

        while(clienteID < MAX_CLIENTES){
            
        }
    }else{

        while(1){
            MPI_Recv( &cliente , MPI_INT ,0 , MPI_ANY_TAG , MPI_Comm comm , &status);
        }
    }
}

int main(int argc, char* argv[]){

    int np, size;


    MPI_Init(&argc, &argv);   
}