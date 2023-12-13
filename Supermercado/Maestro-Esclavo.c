// Practica2ASO.cpp : Este archivo contiene la función "main". La ejecución del programa comienza y termina ahí.
//

#include <iostream>
#include <mpi.h>
#include <windows.h>
#include <stdlib.h>
#include<time.h>


//Variables Necesarias
#define MAX_CLIENTES 100
void clienteLlega() {
    

   
}

void clienteAtendido(int cliente) {
    printf("El cliente %d va a ser atendido.\n", cliente);
    fflush(stdout);

    srand(time(NULL));
    int num = 1 + rand() % (6 - 1);

    Sleep(num);

    printf("El cliente %d ha sido atendido.\n", cliente);
    fflush(stdout);
}

void gestionDeClientes(int pid, int np, int* clientes, int numClientes) {

   
    if (pid == 0) {//Proceso Maestro (Clientes)
        int cajaDestino = 1;
        int clientesNuevos;
        int trabajo = 1;
        
      
        for (int i = 0; i < numClientes; i++) {//Bucle de envios de mensaje a los exclaves apra indicar si hay clientes o no en la cola

            if (i >= numClientes - (np - 1)) { //Si en la cola no hay clientes el trabajo pasa a ser 0 y se lo comunicamos a las cajas (np)
                trabajo = 0;
                MPI_Send(&trabajo, 1, MPI_INT, cajaDestino, 0, MPI_COMM_WORLD);
            }
            else {//Si en la cola aun hay clientes se lo comunicamos a las cajas para que sigan trabajando
                MPI_Send(&trabajo, 1, MPI_INT, cajaDestino, 0, MPI_COMM_WORLD);
            }
            
            MPI_Send(&clientes[i], 1, MPI_INT, cajaDestino, 0, MPI_COMM_WORLD);

            printf("\nEnviado cliente %d a la caja correspondiente.", clientes[i]);
            fflush(stdout);

            cajaDestino = (cajaDestino % np) + 1;

            if (cajaDestino == np) {//Cuando el destino sea igual al numero de procesos debera volver al nodo 1 y reiniciar los envios.
                cajaDestino = 1;
       		}	

            //MPI_Recv()
        }

    }
    else {//Procesos Esclavos (Cajas)

        int trabajoEsclavo = 1;
        while (trabajoEsclavo) {
            int clienteRecibido;
            MPI_Recv(&trabajoEsclavo, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            MPI_Recv(&clienteRecibido, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            printf("Soy la caja %d y me ha llegado el cliente %d\n", pid, clienteRecibido);
            fflush(stdout);
            clienteAtendido(clienteRecibido);

            MPI_Send(&clienteRecibido, 1, MPI_INT, 0, 0, MPI_COMM_WORLD);
        }

    }
    if (pid == 0) {
        free(clientes);
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
    int* clientes = NULL;
    int numClientes = atoi(argv[1]);

    //Establecemos las cajas abiertas segun el numero de clientes
    int* cajas;
    int numCajas = numClientes * 2, x = 0;

    if (pid == 0) {//El proceso MAESTRO Inicia la cola y la imprime
        printf("El size de la cola es: %d. ", numClientes);

        clientes = (int*)malloc(sizeof(int) * numClientes);

        if (clientes == NULL) {
            printf("ERROR: No se pudo reservar memoria.");
            exit(1);
        }

        for (int i = 0; i < numClientes; i++) {
            clientes[i] = j + 1;
            j++;
        }

        printf("\nLos clientes de la cola son los siguientes: ");
        printf("[");
        for (int i = 0; i < numClientes; i++) {
            printf(" %d", clientes[i]);
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
    gestionDeClientes(pid, np, clientes, numClientes);

    MPI_Finalize();
    return 0;
}


