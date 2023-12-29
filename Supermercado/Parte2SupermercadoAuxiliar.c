// Practica2ASO.cpp : Este archivo contiene la función "main". La ejecución del programa comienza y termina ahí.
//

#include <mpi.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>  
#include <stdio.h>
#include <math.h>

typedef struct{
    int* clientes;
    int inicio;
    int final;
    int capacidadOriginal;
}cola;

//VARIABLES AUXILIARES

    
int salirDeCola(cola* cola) {

    int result = cola->clientes[cola->inicio];
  

    cola->inicio++;
    return result;
}

void insertarCola(cola* cola, int clienteNuevo) {
    //Comprobacion de la cola
    
    
    if (cola->final < (cola->capacidadOriginal - 1)) {
        cola->clientes[cola->final + 1] = clienteNuevo;
        cola->final++;
    }
    else {
        int llenos = (cola->final - cola->inicio) + 1;
	int espacioLibre = cola->capacidadOriginal -llenos;

	if(espacioLibre > 0){
		
		for (int i = 0; i < llenos; i++) {
		    cola->clientes[i] = cola->clientes[i + cola->inicio];
		}
		
		cola->inicio = 0;

		cola->final = llenos-1;
		
		cola->clientes[cola->final+1] = clienteNuevo;
		cola->final++;
        }else{
        	
        	printf("\nLa cola esta llena.\n");
        }
     
    } 
}


int clienteAtendido(int cliente, int pid) {
    printf("El cliente %d va a ser atendido.\n", cliente);
 
    unsigned int semilla = (unsigned int)(time(NULL)) + pid + cliente;
    
    srand(semilla);
    int esperaNormal = (rand() % 6 ) +5;
    int esperaPrioritaria = (rand() % 11 ) +10;
    
    //COMPROBACION DE CLIENTE PRIORITARIO O NO PRIORITARIO
    if(cliente % 2 == 0){
    
    	sleep(esperaNormal);
    	printf("El cliente %d ha sido atendido en %d.\n", cliente, esperaNormal);
    }else{
    
    	sleep(esperaPrioritaria);
        printf("El cliente %d ha sido atendido en %d.\n", cliente, esperaPrioritaria);
    }

    return cliente;
}

void imprimirCola(cola* cola){
     printf("\nLos clientes de la cola son los siguientes: ");
        printf("[");
        for (int i = 0; i < cola->capacidadOriginal; i++) {
            printf(" %d", cola->clientes[i]);
        }
        printf("]");
}

void gestionDeClientes(int pid, int np, cola* colaClientes) {

    if (pid == 0) {//Proceso Maestro (Clientes)
        int cajaDestino = 1;
        int clientesDeCaja;
        int indiceCaja;
        int trabajo = 0;
        int* copiaClientes = NULL;
        int recibeTrabajo = 0;
        int cajasAbiertas = round(np / 2);
        MPI_Request requestAsincrono;
	MPI_Status status;
	int flag = 0;
	
        printf("\nNumero de cajas abiertas: %d\n", cajasAbiertas);

        //Bucle infinito de clientes a sus respectivas cajas y de las cajas al maestro que se encarga de meterlos de nuevo en la cola
 	while(1){    
            	 
                    int clienteNuevo = salirDeCola(colaClientes);
			   
	            //MPI_Send(&clienteNuevo, 1, MPI_INT, cajaDestino, 0, MPI_COMM_WORLD);
		    MPI_Isend(&clienteNuevo, 1, MPI_INT, cajaDestino, 0, MPI_COMM_WORLD,&requestAsincrono);
		    
		    //Recepción asíncrona
		    MPI_Irecv(&clientesDeCaja, 1, MPI_INT, cajaDestino, 1, MPI_COMM_WORLD, &requestAsincrono);
		    
		    //BUCLE INFINITO QUE OBLIGA A ESPERAR A QUE LLEGUE UNA FLAG DE LA RECEPCION ASINCRONA PARA COMENZAR CON EL TRABAJO DE IINSERTAR CLIENTES EN LA COLA DE NUEVO
		    while(!flag){
		    	MPI_Test(&requestAsincrono, &flag, &status);
		    }
		    
		    if(flag){
		     	
		        printf("\nEl cliente %d sale de la caja y vuelve a entrar en la cola.\n", clienteNuevo);
		    	
		    	insertarCola(colaClientes, clientesDeCaja);
		    	printf("El cliente %d esta ahora en la cola\n", clientesDeCaja);
		    	imprimirCola(colaClientes);
		    	
		    	flag = 0;
		    }
		    
		    	    
		    // Limitar cajaDestino a un máximo del 50%
		    if (cajaDestino < cajasAbiertas) {
		    
		        cajaDestino = (cajaDestino % np) + 1;
		        if (cajaDestino == np) {//Cuando el destino sea igual al numero de procesos debera volver al nodo 1 y reiniciar los envios.
		            cajaDestino = 1;
		        }
		    }
		    else {
		        cajaDestino = 1;
		    }	     	
        }
      
    }
    else {//Procesos Esclavos (Cajas)
	
        int trabajoEsclavo = 1;
        int clienteRecibido;
        
        
        while (1) {
            
	    //RECEPCION DE CLIENTE
	     
            printf("\n Esta es la caja: %d\n", pid);
            MPI_Recv(&clienteRecibido, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            printf("Soy la caja %d y me ha llegado el cliente %d\n", pid, clienteRecibido);

	    int cliente = clienteAtendido(clienteRecibido, getpid());
            	  
            //ENVIO DE CLIENTE   
            MPI_Send(&cliente, 1, MPI_INT, 0, 1, MPI_COMM_WORLD);    
	    printf("El cliente %d vuelve a la cola en 3 segundos\n", cliente);
	    sleep(3);
       
        }

    }

}

int main(int argc, char* argv[])
{
    int pid, np;

    MPI_Init(&argc, &argv); //AQUI INICIA MPI Y DIVIDE TODO ENTRE LOS PROCESOS PARA EJECUTAR LA PARTE DE MPI

    MPI_Comm_rank(MPI_COMM_WORLD, &pid);
    MPI_Comm_size(MPI_COMM_WORLD, &np);


    double inicio, fin, tiempo;
    //Metodo encargado de gestionar la entrada de los clientes en las cajas
    //Encargado de seguir esquema Maestro-Esclavo
    //Primero craeamos la cola de clientes

    int j = 0;
    cola colaSupermercado;


    if (pid == 0) {//El proceso MAESTRO Inicia la cola y la imprime

        colaSupermercado.capacidadOriginal = 6;
        colaSupermercado.clientes = (int*)malloc(colaSupermercado.capacidadOriginal * sizeof(int));
        colaSupermercado.inicio = 0;
        colaSupermercado.final = colaSupermercado.capacidadOriginal - 1;

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


        printf("\nIniciando trabajo del supermercado.");

    }

    gestionDeClientes(pid, np, &colaSupermercado);

    MPI_Finalize();
    return 0;
}
