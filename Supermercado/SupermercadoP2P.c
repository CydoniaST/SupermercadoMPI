#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <math.h>

typedef struct{
    int* clientes;
    int inicio;
    int final;
    int capacidadOriginal;
    int* colaPrioridades;
    int inicioP;
    int finalP;
}cola;

// Funciones auxiliares para la cola y las prioridades

int salirDeCola(cola* cola) {

    int result = cola->clientes[cola->inicio];
  

    cola->inicio++;
    return result;
}

int salirPrioridad(cola* cola) {

    int result = cola->colaPrioridades[cola->inicio-1];
  

    cola->inicioP++;
    return result;
}

void insertarCola(cola* cola, int clienteNuevo, int prioridad) {
    //Comprobacion de la cola
    
    
    if (cola->final < (cola->capacidadOriginal - 1)) {
        cola->clientes[cola->final + 1] = clienteNuevo;
        cola->final++;
        cola->colaPrioridades[cola->finalP + 1] = prioridad;
        cola->finalP++;
    }
    else {
        int llenos = (cola->final - cola->inicio) + 1;
        int llenosP = (cola->finalP - cola->inicioP) + 1;
	int espacioLibre = cola->capacidadOriginal -llenos;

	if(espacioLibre > 0){
		
		for (int i = 0; i < llenos; i++) {
		    cola->clientes[i] = cola->clientes[i + cola->inicio];
		    cola->colaPrioridades[i] = cola->colaPrioridades[i + cola->inicioP];
		}
		//CLIENTES
		cola->inicio = 0;

		cola->final = llenos-1;
		
		cola->clientes[cola->final+1] = clienteNuevo;
		cola->final++;
		//PRIORIDADES
		cola->inicioP = 0;

		cola->finalP = llenos-1;
		
		cola->colaPrioridades[cola->finalP+1] = prioridad;
		cola->finalP++;
        }else{
        	
        	printf("\nLa cola esta llena.\n");
        }
     
    } 
}

int clienteAtendido(int cliente, int pid, int prioridad) {
    printf("El cliente %d va a ser atendido.\n", cliente);
 
    unsigned int semilla = (unsigned int)(time(NULL)) + pid + cliente;
    
    srand(semilla);
    
    if(prioridad ==1){
    	int esperaPrioridad = (rand() % 11 ) +10; 
    	sleep(esperaPrioridad);
    	    

   	printf("El cliente %d tiene prioridad y ha sido atendido en %d.\n", cliente, esperaPrioridad);
    }else{
    	   
    	int esperaNormal = (rand() % 6 ) +5;
    	sleep(esperaNormal);
    	    

    	printf("El cliente %d no tiene prioridad y ha sido atendido en %d.\n", cliente, esperaNormal);
    }
    


    return cliente;
}


// Función para imprimir la cola
void imprimirCola(cola* cola) {
    printf("\nLos clientes en la cola son: ");
    printf("[");
    for (int i = cola->inicio; i <= cola->final; i++) {
        printf(" %d", cola->clientes[i]);
    }
    printf("]\n");
}

// Función para imprimir las prioridades
void imprimirPrioridades(cola* cola) {
    printf("Las prioridades son: ");
    printf("[");
    for (int i = cola->inicioP; i <= cola->finalP; i++) {
        printf(" %d", cola->colaPrioridades[i]);
    }
    printf("]\n");
}

// Función principal para la gestión de clientes
void gestionDeClientes(int pid, int np, cola* colaClientes) {
	int clientesDeCaja;
        int prioridadCliente;
        int cliente;
        
        int prioridadRecibida;
    	int cajaDestino = 1;
    	int flag = 0;
    	MPI_Status status;
    	
    //CONTROL DE CAJAS ABIERTAS
	int cajasAbiertas = round(np / 2);
	if(colaClientes->capacidadOriginal > cajasAbiertas * 2){
		cajasAbiertas ++;
	}
	
    while (1) {
        
        MPI_Request requestAsincrono;

	if(pid = 0){
	
                int clienteNuevo = salirDeCola(colaClientes); // Simula la llegada de un cliente
                int prioridad = salirPrioridad(colaClientes); // Genera aleatoriamente la prioridad del cliente
                MPI_Send(&clienteNuevo, 1, MPI_INT, cajaDestino, 0, MPI_COMM_WORLD);
		MPI_Send(&prioridad, 1, MPI_INT, cajaDestino, 1, MPI_COMM_WORLD);	
		
		printf("Soy la caja %d y me ha llegado el cliente %d con prioridad %d \n", pid, clienteNuevo, prioridad);		
		int cliente = clienteAtendido(clienteNuevo, getpid(), prioridad);
		
	        MPI_Irecv(&cliente, 1, MPI_INT, cajaDestino, 1, MPI_COMM_WORLD, &requestAsincrono);
		MPI_Irecv(&prioridadCliente, 1, MPI_INT, cajaDestino, 2, MPI_COMM_WORLD, &requestAsincrono);
                printf("El cliente %d entra a la cola con prioridad %d.\n", cliente, prioridadCliente);
                 insertarCola(colaClientes,clienteNuevo, prioridadCliente);
	         imprimirCola(colaClientes);
		 imprimirPrioridades(colaClientes);
                
         }

 
		 MPI_Recv(&cliente, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
		 MPI_Recv(&prioridadCliente, 1, MPI_INT, 0, 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
	 	
	 	 int cliente = clienteAtendido(cliente, getpid(), prioridadCliente);
	 	
	 	int clienteNuevo = salirDeCola(colaClientes); // Simula la llegada de un cliente
                int prioridad = salirPrioridad(colaClientes); 
	 	 MPI_Isend(&clienteNuevo, 1, MPI_INT, cajaDestino, 1, MPI_COMM_WORLD, &requestAsincrono);
		 MPI_Isend(&prioridad, 1, MPI_INT, cajaDestino, 2, MPI_COMM_WORLD, &requestAsincrono);
		 insertarCola(colaClientes,cliente, prioridadCliente);
	         imprimirCola(colaClientes);
		 imprimirPrioridades(colaClientes);

        // Sincronización para asegurar que todos los procesos hayan atendido a sus clientes antes de continuar
        MPI_Barrier(MPI_COMM_WORLD);
        
        if (cajaDestino < cajasAbiertas) {
		    
		cajaDestino = (cajaDestino % np) + 1;
	        if (cajaDestino == np) {//Cuando el destino sea igual al numero de procesos debera volver al nodo 1 y reiniciar los envios.
	            cajaDestino = 1;
	        }
	}
	else {
	        cajaDestino = 1;
	}

 	
        // Espera para simular la llegada de más clientes
        sleep(2);
    }


    free(colaClientes->clientes);
    free(colaClientes->colaPrioridades);
}

int main(int argc, char* argv[]) {
    int pid, np;

    MPI_Init(&argc, &argv);

    MPI_Comm_rank(MPI_COMM_WORLD, &pid);
    MPI_Comm_size(MPI_COMM_WORLD, &np);

     int j = 0;
    cola colaSupermercado;
     colaSupermercado.capacidadOriginal = 6;
        colaSupermercado.clientes = (int*)malloc(colaSupermercado.capacidadOriginal * sizeof(int));
        colaSupermercado.inicio = 0;
        colaSupermercado.final = colaSupermercado.capacidadOriginal - 1;
        colaSupermercado.colaPrioridades = (int*)malloc(colaSupermercado.capacidadOriginal * sizeof(int));
         colaSupermercado.inicioP = 0;
        colaSupermercado.finalP = colaSupermercado.capacidadOriginal - 1;
    if(pid==0){
    	

        printf("El size de la cola es: %d. ", colaSupermercado.capacidadOriginal);

        if (colaSupermercado.clientes == NULL) {
            printf("ERROR: No se pudo reservar memoria.");
            exit(1);
        }

        for (int i = 0; i < colaSupermercado.capacidadOriginal; i++) {

            colaSupermercado.clientes[i] = j + 1;
            j++;
        }

        printf("\nLos clientes de la cola son los siguientes: ");
        printf("[");
        for (int i = 0; i < colaSupermercado.capacidadOriginal; i++) {
            printf(" %d", colaSupermercado.clientes[i]);
        }
        printf("]");
        //Creacion de cola finalizada

	//COLA DE PRIORIDADES
	for (int i = 0; i < colaSupermercado.capacidadOriginal; i++) {

            colaSupermercado.colaPrioridades[i] = rand()%2;
           
        }
        
         printf("\nLas prioridades son: ");
        printf("[");
        for (int i = 0; i < colaSupermercado.capacidadOriginal; i++) {
            printf(" %d", colaSupermercado.colaPrioridades[i]);
        }
        printf("]");
        
        printf("\nIniciando trabajo del supermercado.");
     }

    gestionDeClientes(pid, np, &colaSupermercado);

    MPI_Finalize();
    return 0;
}
