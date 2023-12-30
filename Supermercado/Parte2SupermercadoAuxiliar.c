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
    int* colaPrioridades;
    int inicioP;
    int finalP;
}cola;

//VARIABLES AUXILIARES

    
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

void insertarPrioridades(cola* cola, int prioridad) {
    //Comprobacion de la cola
    
    
    if (cola->finalP < (cola->capacidadOriginal - 1)) {
        cola->colaPrioridades[cola->finalP + 1] = prioridad;
        cola->finalP++;
    }
    else {
        int llenos = (cola->finalP - cola->inicioP) + 1;
	int espacioLibre = cola->capacidadOriginal -llenos;

	if(espacioLibre > 0){
		
		for (int i = 0; i < llenos; i++) {
		    cola->colaPrioridades[i] = cola->colaPrioridades[i + cola->inicioP];
		}
		
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

void imprimirCola(cola* cola){
     printf("\nLos clientes de la cola son los siguientes: ");
        printf("[");
        for (int i = 0; i < cola->capacidadOriginal; i++) {
            printf(" %d", cola->clientes[i]);
        }
        printf("]");
}

void imprimirColaP(cola* cola){
     printf("\nLos clientes de la cola son los siguientes: ");
        printf("[");
        for (int i = 0; i < cola->capacidadOriginal; i++) {
            printf(" %d", cola->colaPrioridades[i]);
        }
        printf("]");
}

void gestionDeClientes(int pid, int np, cola* colaClientes) {

    if (pid == 0) {//Proceso Maestro (Clientes)
        int cajaDestino = 1;
        int clientesDeCaja;
        int prioridadCliente;
 

        int cajasAbiertas = round(np / 2);
        MPI_Request requestAsincrono;
        MPI_Request requestAsincronoP;
	MPI_Status status;
	int flag = 0;
	
	//int contadorCajas = 0;
	
        printf("\nNumero de cajas abiertas: %d\n", cajasAbiertas);

        //Bucle infinito de clientes a sus respectivas cajas y de las cajas al maestro que se encarga de meterlos de nuevo en la cola
 	while(1){    
            	 
            	
		     	 int clienteNuevo = salirDeCola(colaClientes);
	                 int prioridad = salirPrioridad(colaClientes);			   
			 MPI_Send(&clienteNuevo, 1, MPI_INT, cajaDestino, 0, MPI_COMM_WORLD);	
		 	 MPI_Send(&prioridad, 1, MPI_INT, cajaDestino, 1, MPI_COMM_WORLD);	
		 	
		    //Recepción asíncrona
		    MPI_Irecv(&clientesDeCaja, 1, MPI_INT, cajaDestino, 1, MPI_COMM_WORLD, &requestAsincrono);
		    MPI_Irecv(&prioridadCliente, 1, MPI_INT, cajaDestino, 2, MPI_COMM_WORLD, &requestAsincronoP);
		    
		    //BUCLE INFINITO QUE OBLIGA A ESPERAR A QUE LLEGUE UNA FLAG DE LA RECEPCION ASINCRONA PARA COMENZAR CON EL TRABAJO DE IINSERTAR CLIENTES EN LA COLA DE NUEVO
		    while(!flag){
		    	MPI_Test(&requestAsincrono, &flag, &status);
		    	MPI_Test(&requestAsincronoP, &flag, &status);
		    }
		    
		    if(flag){
		     	//printf("SACANDO AL CLIENTE %d POR DIOS\n", clienteNuevo);
		        //printf("\nEl cliente %d sale de la caja y vuelve a entrar en la cola.\n", clienteNuevo);
		    	
		    	insertarCola(colaClientes, clientesDeCaja);
		    	insertarPrioridades(colaClientes, prioridadCliente);
		    	printf("El cliente %d esta ahora en la cola\n", clientesDeCaja);
		    	imprimirCola(colaClientes);
		    	imprimirColaP(colaClientes);
		    	
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
        int prioridadRecibida;
   
        while (1) {
            
	    //RECEPCION DE CLIENTE
	     
            printf("\n Esta es la caja: %d\n", pid);
            MPI_Recv(&clienteRecibido, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            MPI_Recv(&prioridadRecibida, 1, MPI_INT, 0, 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            printf("Soy la caja %d y me ha llegado el cliente %d con prioridad %d \n", pid, clienteRecibido, prioridadRecibida);

	    
            int cliente = clienteAtendido(clienteRecibido, getpid(), prioridadRecibida);
         
	    
            //ENVIO DE CLIENTE   
            MPI_Send(&cliente, 1, MPI_INT, 0, 1, MPI_COMM_WORLD); 

            
            //Cambio de prioridad
            if(prioridadRecibida == 1){
            	prioridadRecibida = 0;
            }else{
            	prioridadRecibida = 0;
            }
            
            MPI_Send(&prioridadRecibida, 1, MPI_INT, 0, 2, MPI_COMM_WORLD);   
             
            printf("El cliente %d vuelve a la cola con prioridad %d\n", cliente, prioridadRecibida);
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
        colaSupermercado.colaPrioridades = (int*)malloc(colaSupermercado.capacidadOriginal * sizeof(int));
         colaSupermercado.inicioP = 0;
        colaSupermercado.finalP = colaSupermercado.capacidadOriginal - 1;

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
