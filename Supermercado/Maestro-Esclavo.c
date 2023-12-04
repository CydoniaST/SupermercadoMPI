#include <stdio.h>
#include <mpi.h>
#include <stdlib.h>
#include <time.h>


#define MAESTRO 0
#define CLIENTE_PRIORITARIO 1
#define CLIENTE_NORMAL 0
#define CAJA_PRIORITARIA 1
#define CAJA_NORMAL 0

//Sistema de cola de clientes
typedef struct{
    int id;
    int prioridad;
}Cliente;


typedef struct{
    Cliente cliente;
    struct cola *siguiente;
} Nodo;


typedef struct{
    Nodo* inicioCola;
    Nodo* finalCola;
} Cola;
//Final de Sistema de cola de clientes


void supermercado(){//Este metodo debe crear las colas, asignar clientes.

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

Cola* iniciarCola(){//Este metodo inicializa una cola vacia
    Cola* nuevaCola = (Cola*)malloc(sizeof(Cola));
    if(nuevaCola==NULL){
        printf("Error: no se ha podido crear la cola.");
        return -1;
    }
    cola->inicioCola = NULL;
    cola->finalCola = NULL;
    return nuevaCola;
}

void nuevoCliente(Cola* cola, Cliente cliente){//Este emetodo añade un nuevo cliente a la cola de cliente segun llegan
    Nodo* nuevoNodo = (Nodo*)malloc(sizeof(Nodo));
    nuevoNodo -> cliente = cliente;
    nuevoNodo-> siguiente = NULL;

    if(cola->finalCola == NULL){
        cola->inicioCola = nuevoNodo;
        cola->finalCola = nuevoNodo;
    }else{
        cola->finalCola->siguiente = nuevoNodo;
        cola->finalCola = nuevoNodo;
    }
}

void saleCliente(Cola* cola){//Este metodo elimina un cliente de la cola
    if(cola->inicioCola == NULL){
        printf("La cola esta vacia, no puede salir ningun cliente");
    }
    
    Nodo* nodoEliminado = cola->inicioCola;
    cola->inicioCola = cola->inicioCola->siguiente;
    free(nodoEliminado);
    if(cola->inicioCola == NULL){
        Cliente clienteVacio = {-1, -1};
        cola->finalCola = NULL;
    }

   free(nodoEliminado);
}
void comprobarHuecoCaja (){ //Metodo que comprueba si hau una caja libre para que entre un cliente

}

void clienteLlega(Cola* cola, int idCliente, int prioridadCliente){//LLega un cliente nuevo y lo mete en la cola
    Cliente nuevoCliente = { idCliente, prioridadCliente};
    nuevoCliente(cola, nuevoCliente);
    printf("El clientecon ID %d ha sido atendido y tiene prioridad %d\n", nuevoCliente.id, nuevoCliente.prioridad)
}

void clienteAtendido(Cola* cola){//Atiende un cliente en la caja y lo elimina de la cola
    Cliente clienteAtendido = saleCliente(cola);

    if(clienteAtendido.id != -1){
        printf("El cliente con ID: %d ha sido atendido", clienteAtendido.id);
    }else{
        printf("No hay cliente para atender.");
    }
}

int main(int argc, char* argv[]){

    int np, size;


    MPI_Init(&argc, &argv);   
}
