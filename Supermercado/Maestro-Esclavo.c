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

//typedef struct {
//    int* clientes = (int*)malloc(sizeof(int) * numClientes);
//    int inicio;
//    int final;
//    int clientePrioritario;
//}cola;

void reordenarCola(int* clientes, int numClientes) {
    
    if (numClientes == 0) {
        printf("La cola esta vacia.");
        fflush(stdout);
    }

    for (int i = 0; i < numClientes; i++) {
        clientes[i] = clientes[i + 1]; // Movemos cada elemento una posición hacia adelante
    }
    clientes[numClientes-1] = -1; //Como la fila se va vaciando sustituyo las posiciones vacias de la cola por un -1.
}

int encontrarPrimeraPosicionVacia(int* clientes, int numClientes) {
    for (int i = 0; i < numClientes; ++i) {
        if (clientes[i] == -1) {
            return i; // Se ha encontrado una posición vacía en la cola.
        }
    }
    return -1; // Si no se encuentra ninguna posición vacía, se devuelve -1
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

void gestionDeClientes(int pid, int np, int* clientes, int numClientes) {
 
    if (pid == 0) {//Proceso Maestro (Clientes)
        int cajaDestino = 1;
        int clientesDeCaja;
        int indiceCaja;
        int trabajo = 1;
        int cola = 0;
        int* copiaClientes = NULL;
        int i = 0;
        int indiceDeCola = 0; //Indice que indica cual es el hueco vacio al que iran los clientes que lleguen al maestros de las cajas para entrar en la cola

    
     //Bucle de envios de mensaje a los exclaves apra indicar si hay clientes o no en la cola
        while(1){
            if (sizeof(clientes) == 0) { //Si en la cola no hay clientes el trabajo pasa a ser 0 y se lo comunicamos a las cajas (np), para numClientes -> "i >= numClientes - (np - 1)"
                trabajo = 0;
                MPI_Send(&trabajo, 1, MPI_INT, cajaDestino, 0, MPI_COMM_WORLD);
            }
            else {//Si en la cola aun hay clientes se lo comunicamos a las cajas para que sigan trabajando
                MPI_Send(&trabajo, 1, MPI_INT, cajaDestino, 0, MPI_COMM_WORLD);
            }
           
            printf("\nEnviado cliente %d a la caja correspondiente.", clientes[0]);
            fflush(stdout);
            MPI_Send(&clientes[0], 1, MPI_INT, cajaDestino, 0, MPI_COMM_WORLD);
            
            //Reordeno la cola para que los quientes vayan avanzando
            reordenarCola(clientes, numClientes);
     
            printf("La cola ahora es : \n");
            for (int x = 0; x < numClientes; x++) {
                printf("%d", clientes[x]);
            }

            MPI_Recv(&clientesDeCaja, 1, MPI_INT, cajaDestino, 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            printf("\nEl cliente %d sale de la caja y vuelve a entrar en la cola.\n", clientesDeCaja);
            fflush(stdout);

            Sleep(1000);
            printf("El cliente %d esta ahora en la cola\n", clientesDeCaja);
            fflush(stdout);

          for (int j = 0; j < numClientes; j++) {
                if (clientes[j] == -1) {
                    clientes[j] = clientesDeCaja;
                }
           }

            //Recibo el indice de la caja(revisar, seguramente no se necesite o este mal)
            MPI_Recv(&indiceCaja, 1, MPI_INT, cajaDestino, 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            printf("\nLa caja %d ha sido la ultima en operar...\n", indiceCaja);
            fflush(stdout);

            cajaDestino = (cajaDestino % np) + 1;
            if (cajaDestino == np) {//Cuando el destino sea igual al numero de procesos debera volver al nodo 1 y reiniciar los envios.
                cajaDestino = 1;
            }

            i++;
        }
       
    }else {//Procesos Esclavos (Cajas)

        int trabajoEsclavo = 1;
        while (trabajoEsclavo) {
            int clienteRecibido;
            printf("\n El trabajo es: %d\n", trabajoEsclavo);
            MPI_Recv(&trabajoEsclavo, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            MPI_Recv(&clienteRecibido, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            printf("Soy la caja %d y me ha llegado el cliente %d\n", pid, clienteRecibido);
            fflush(stdout);

            int cliente= clienteAtendido(clienteRecibido);
            //ALTERNATIVO

            MPI_Send(&cliente, 1, MPI_INT, 0, 1, MPI_COMM_WORLD);
            MPI_Send(&pid, 1, MPI_INT, 0, 1, MPI_COMM_WORLD); //Envio que caja se queda libre al atender al cliente
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
    //int* clientes = NULL;
    //int numClientes = atoi(argv[1]);

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
