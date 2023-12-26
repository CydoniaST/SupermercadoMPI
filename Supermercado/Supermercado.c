// Practica2ASO.cpp : Este archivo contiene la función "main". La ejecución del programa comienza y termina ahí.
//

//#include <iostream>
#include <mpi.h>
//#include <windows.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>  
#include <stdio.h>
#include <math.h>

//Variables Necesarias
#define MAX_CLIENTES 100
int indiceCola = 0;
int numClientes;

typedef struct{
    int* clientes;
    int inicio;
    int final;
    int capacidadOriginal;
}cola;


int salirDeCola(cola* cola) {

    int result = cola->clientes[cola->inicio];
    printf("\nSacando al cliente %d. ", result);

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
    //fflush(stdout);

    srand(time(NULL));
    int num = (rand() % 6 ) +5;

    //Sleep(num);
    
    sleep(num);
    
       //Simulación de trabajo
    //double start_time = MPI_Wtime();//Tiempo inicio de trabajo de caja
    //double paso_Del_Tiempo;

    //do {

    //    paso_Del_Tiempo = MPI_Wtime() - start_time;

    //    /* printf("Atendiendo cliente.\n");
    //     fflush(stdout);*/
    //} while (paso_Del_Tiempo < num);
    

    printf("El cliente %d ha sido atendido en %d.\n", cliente, num);
    //fflush(stdout);

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
        int cajasAbiertas = round(np / 2);
        MPI_Request requestAsincrono;

        printf("\nNumero de cajas abiertas: %d\n", cajasAbiertas);
        //fflush(stdout);

        //Bucle de envios de mensaje a los exclaves apra indicar si hay clientes o no en la cola
        //for (int i = 0; i < colaClientes->capacidadOriginal; i++) {

            // }
        while(1){
            //if (i >= numClientes - (np - 1)) {//sizeof(colaClientes->clientes) == 0) { //Si en la cola no hay clientes el trabajo pasa a ser 0 y se lo comunicamos a las cajas (np), para numClientes -> "i >= numClientes - (np - 1)"
              //  trabajo = 0;
                //MPI_Send(&trabajo, 1, MPI_INT, cajaDestino, 0, MPI_COMM_WORLD);
            //}
            //else {//Si en la cola aun hay clientes se lo comunicamos a las cajas para que sigan trabajando
              //  MPI_Send(&trabajo, 1, MPI_INT, cajaDestino, 0, MPI_COMM_WORLD);
            //}

            int clienteNuevo = salirDeCola(colaClientes);
            printf("\nEnviado cliente %d a la caja correspondiente.\n", clienteNuevo);
            //fflush(stdout);
            MPI_Send(&clienteNuevo, 1, MPI_INT, cajaDestino, 0, MPI_COMM_WORLD);

            //tiene que ser asincrono

        /*    MPI_Irecv(&clientesDeCaja, 1, MPI_INT, cajaDestino, 1, MPI_COMM_WORLD, &requestAsincrono);
            MPI_Wait(&requestAsincrono, MPI_STATUS_IGNORE);*/

            MPI_Recv(&clientesDeCaja, 1, MPI_INT, cajaDestino, 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

            printf("\nEl cliente %d sale de la caja y vuelve a entrar en la cola.\n", clientesDeCaja);
            //fflush(stdout);

            insertarCola(colaClientes, clientesDeCaja);

            printf("El cliente %d esta ahora en la cola\n", clientesDeCaja);

            printf("La cola ahora es : \n");
            //fflush(stdout);
            
            for (int x = 0; x < colaClientes->capacidadOriginal; x++) {
                printf("%d - ", colaClientes->clientes[x]);
            }
            //fflush(stdout);

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
        MPI_Request requestAsincrono;

        int trabajoEsclavo = 1;
        while (trabajoEsclavo) {
            int clienteRecibido;
            printf("\n Esta es la caja: %d\n", pid);
            //MPI_Recv(&trabajoEsclavo, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            MPI_Recv(&clienteRecibido, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            printf("Soy la caja %d y me ha llegado el cliente %d\n", pid, clienteRecibido);
           // fflush(stdout);

            int cliente = clienteAtendido(clienteRecibido);
            printf("El cliente es %d\n", cliente);
            //fflush(stdout);

            MPI_Send(&cliente, 1, MPI_INT, 0, 1, MPI_COMM_WORLD);
            //ALTERNATIVO
            //MPI_Isend(&cliente, 1, MPI_INT, 0, 1, MPI_COMM_WORLD, &requestAsincrono);
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

        //Copia de Clientes para que no se sobreescriba (por seguridad)
         //int* bufferClientes = (int*) malloc(sizeof(int));

        printf("\nIniciando trabajo del supermercado.");
        //fflush(stdout);


    }

    gestionDeClientes(pid, np, &colaSupermercado);



    MPI_Finalize();
    return 0;
}

