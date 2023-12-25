// Practica2ASO.cpp : Este archivo contiene la función "main". La ejecución del programa comienza y termina ahí.
//

#include <iostream>
#include <mpi.h>
#include <windows.h>
#include <stdlib.h>
#include <time.h>


//Variables Necesarias
#define MAX_CLIENTES 100
int indiceCola = 0;
int numClientes;

struct cola{
    int* clientes;
    int inicio = 0;
    int final;
    int capacidadOriginal;
};


int salirDeCola(cola* cola) {
  
    int result = cola->clientes[cola->inicio];                        
    printf("Sacando al cliente %d. ", result);

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

        for (int i = 0; i < llenos; i++) {
            cola->clientes[i] = cola->clientes[i + cola->inicio];
        }

        cola->inicio = 0;

        cola->final = llenos;
        cola->clientes[cola->final] = clienteNuevo;
    }
}


int clienteAtendido(int cliente) {
    printf("El cliente %d va a ser atendido.\n", cliente);
    fflush(stdout);

    //srand(time(NULL));
    int num =rand() % (10000 - 1);

    Sleep(num);

    printf("El cliente %d ha sido atendido en %d.\n", cliente, num);
    fflush(stdout);

    return cliente;
}

void gestionDeClientes(int pid, int np, cola* colaClientes) {
 
    //colaClientes->final = colaClientes->capacidadOriginal-1;

    if (pid == 0) {//Proceso Maestro (Clientes)
        int cajaDestino = 1;
        int clientesDeCaja;
        int indiceCaja;
        int trabajo = 1;
        int* copiaClientes = NULL;
        int recibeTrabajo = 0;

        printf("\nHola\n");
        fflush(stdout);

     //Bucle de envios de mensaje a los exclaves apra indicar si hay clientes o no en la cola
        while(1){
            if (0){//sizeof(colaClientes->clientes) == 0) { //Si en la cola no hay clientes el trabajo pasa a ser 0 y se lo comunicamos a las cajas (np), para numClientes -> "i >= numClientes - (np - 1)"
                trabajo = 0;
                MPI_Send(&trabajo, 1, MPI_INT, cajaDestino, 0, MPI_COMM_WORLD);
            }
            else {//Si en la cola aun hay clientes se lo comunicamos a las cajas para que sigan trabajando
                MPI_Send(&trabajo, 1, MPI_INT, cajaDestino, 0, MPI_COMM_WORLD);
            }
           
            MPI_Request requestAsincrono;

            int clienteNuevo = salirDeCola(colaClientes);
            printf("\nEnviado cliente %d a la caja correspondiente.\n", clienteNuevo);
            fflush(stdout);
            MPI_Send(&clienteNuevo, 1, MPI_INT, cajaDestino, 0, MPI_COMM_WORLD);

            //tiene que ser asincrono
          
            //MPI_Irecv(&clientesDeCaja, 1, MPI_INT, cajaDestino, MPI_ANY_TAG, MPI_COMM_WORLD, &requestAsincrono);
            //MPI_Wait(&requestAsincrono, MPI_STATUS_IGNORE);

            MPI_Recv(&clientesDeCaja, 1, MPI_INT, cajaDestino, 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

            printf("\nEl cliente %d sale de la caja y vuelve a entrar en la cola.\n", clientesDeCaja);
            fflush(stdout);

            insertarCola(colaClientes, clientesDeCaja);

            printf("El cliente %d esta ahora en la cola\n", clientesDeCaja);

            printf("La cola ahora es : \n");
            fflush(stdout);
            for (int x = 0; x < colaClientes->capacidadOriginal; x++) {
                printf("%d - ", colaClientes->clientes[x]);
            }
            fflush(stdout);

            cajaDestino = (cajaDestino % np) + 1;
            if (cajaDestino == np) {//Cuando el destino sea igual al numero de procesos debera volver al nodo 1 y reiniciar los envios.
                cajaDestino = 1;
            }

        }
       
       
    }else {//Procesos Esclavos (Cajas)
        MPI_Request requestAsincrono;

        int trabajoEsclavo = 1;
        while (trabajoEsclavo) {
            int clienteRecibido;
            printf("\n El trabajo es: %d\n", trabajoEsclavo);
            MPI_Recv(&trabajoEsclavo, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            MPI_Recv(&clienteRecibido, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            printf("Soy la caja %d y me ha llegado el cliente %d\n", pid, clienteRecibido);
            fflush(stdout);

            int cliente= clienteAtendido(clienteRecibido);
            printf("El cliente es %d\n", cliente);
            fflush(stdout);

            MPI_Send(&cliente, 1, MPI_INT, 0, 1, MPI_COMM_WORLD);
            //ALTERNATIVO
            //MPI_Isend(&cliente, 1, MPI_INT, 0, MPI_ANY_TAG, MPI_COMM_WORLD, &requestAsincrono);
            //MPI_Wait(&requestAsincrono, MPI_STATUS_IGNORE);
        }

    }
    if (pid == 0) {
        free(colaClientes->clientes);
    }
         
}

int main(int argc, char* argv[])
{
    int pid, np;

    MPI_Init(NULL, NULL); //AQUI INICIA MPI Y DIVIDE TODO ENTRE LOS PROCESOS PARA EJECUTAR LA PARTE DE MPI

    MPI_Comm_rank(MPI_COMM_WORLD, &pid);
    MPI_Comm_size(MPI_COMM_WORLD, &np);
   
   
    double inicio, fin, tiempo;
    //Metodo encargado de gestionar la entrada de los clientes en las cajas
    //Encargado de seguir esquema Maestro-Esclavo
    //Primero craeamos la cola de clientes
    
    int j = 0;
    cola colaSupermercado;
    int clientesCola[5];
    
    //Establecemos las cajas abiertas segun el numero de clientes
    int* cajas;
    int numCajas = numClientes * 2, x = 0;

    if (pid == 0) {//El proceso MAESTRO Inicia la cola y la imprime
 
        colaSupermercado.capacidadOriginal = 5;
        colaSupermercado.clientes = (int*)malloc(colaSupermercado.capacidadOriginal * sizeof(int));
        colaSupermercado.final = colaSupermercado.capacidadOriginal-1;

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

        //Copia de Clientes para que no se sobreescriba (por seguridad)
         //int* bufferClientes = (int*) malloc(sizeof(int));

        cajas = (int*)malloc(sizeof(int) * numCajas);

        for (int i = 0; i < numCajas; i++) {
            cajas[i] = x + 1;
            x++;
        }

        printf("\nIniciando trabajo del supermercado.");
        fflush(stdout);   


    }

     gestionDeClientes(pid, np, &colaSupermercado);
    


    MPI_Finalize();
    return 0;
}


