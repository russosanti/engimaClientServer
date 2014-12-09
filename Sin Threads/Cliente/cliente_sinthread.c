
#include <stdlib.h>
#include <string.h>
#include "listas.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <pthread.h>

#define PUERTO 4000
#define MAXIP 32
#define MAXBUFF 32
#define NROJUGADAS 4
#define RESPUESTA 3    //NRO JUGADA BIEN Y REGULAR
#define MAX_INTENTOS 5 //para el help lo uso nomas


void error(char *msg);
void Enter();
void get_ip(char ip[]);
int check_ip(char ip[]);
int client(char adress[],int port);
void disponibilidad(int socket);
void jugar(int socket);
void ingresar_comando(char buffer[]);
int comando_valido(char buffer[]);
void tratar_comando(int socket,char buffer[]);
void game(int socket,char buffer[],int *ganados,int *perdidos);
int elegir_tipo();
void estrategia(tlista_s *jugadas,char mov[],char mov_ant[],int bien, int regular,int *mi_bien,int *mi_regular);
void selec_estrat(tlista_s *jugadas,char mov[],char mov_ant[],int mi_bien,int mi_regular);
void reemplazar(char mov[],int x);
void mezclar(char mov[],int x);
int existe(int ar[],int tope,int bus);
int diferencias(char ar1[],char ar2[]);
void generate(char str[]);
int rnd(int max);
int apariciones(char ar[],char x);
void move(char buffer[]);
int move_val(char buffer[]);
void IO(int socket,char buffer[]);
void tratar_jugada(char buffer[],int *control,int automatico,int *bien,int *regular);
void server_error(int socket,char buffer[],int error);
void help_comando();
void help_move();
void help_objetivo();




int main()
{
    char ip[MAXIP];
    get_ip(ip);
    if(strcasecmp(ip,"EXIT")!=0){
        client(ip,PUERTO);//client("localhost",PUERTO);
        return(1);
    }else{
        return(0);
    }
}


// Trata los errores de los sockets
void error(char *msg)
{
    perror(msg);   //errores del sistema
    exit(0);
}
//----------------------------------------

//Pide el ingreso de un enter
void Enter()
{
    char enter[256];
    printf("Presione enter para continuar");
    fgets(enter,256,stdin);
    system("clear");
}
//-----------------------------------------------

//Pide el ingreso de la ip o localhost
void get_ip(char ip[])
{
    printf("Ingrese la ip (Recuerde 'xxx.xxx.xxx.xxx' o localhost):\n");
    fgets(ip,36,stdin);
    fflush(stdin);  // x lo tanto tengo que pararme en el ultimo y borrarlo
    ip[strlen(ip)-1]='\0';
    while((strcasecmp(ip,"EXIT")!=0) && check_ip(ip)){
        printf("Ingrese la ip (Recuerde 'xxx.xxx.xxx.xxx' o localhost):\n");
        fgets(ip,36,stdin);
        fflush(stdin);  // x lo tanto tengo que pararme en el ultimo y borrarlo
        ip[strlen(ip)-1]='\0';
    }
    system("clear");
}
//---------------------------------------------------------------

//Valida la ip ingresada
int check_ip(char ip[])
{
    char *auxip=(char*)malloc(sizeof(ip));
    char *ptr;//=(char*)malloc(sizeof(ip));
    if(strcasecmp(ip,"localhost")==0){
        return(0);
    }else{
        if(strlen(ip)>15){   //chequa la longitud de la IP
            printf("Longitud de la ip superada\nDebe ser menor a 15\n");
            return(1);
        }else{
            strcpy(auxip,ip);
            if(ptr=strchr(auxip,'.')==NULL){    //se fija que no falte la puntuacion
                printf("Falta puntuacion en la ip\n");
                return(1);
            }else{
                return(0);
                //podria seguir analizando pero si es invalida me tira invalid connection en el error
            }
        }
    }
}
//------------------------------------------------------------------

//Crea la conexion con el servidor
int client(char adress[],int port)
{
    int sockfd;
    struct sockaddr_in serv_addr;
    struct hostent *server;

    if(port<=2000){   //Los puertos de numero bajo los usa el SO
       fprintf(stderr,"Numero de puerto (%d) debe estar entre 2000 y 65000\n", port);
       exit(0);
    }
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)  //en el caso que no se puede inicializar el socket
        error("ERROR. Por favor reinicie la aplicacion");
    server = gethostbyname(adress);
    if (server == NULL) {
        fprintf(stderr,"ERROR, IP \n");
        exit(0);
    }
    bzero((char *) &serv_addr, sizeof(serv_addr));  //pone la cadena en /0
    serv_addr.sin_family = AF_INET;
    bcopy((char *)server->h_addr,(char *)&serv_addr.sin_addr.s_addr,server->h_length);
    serv_addr.sin_port = htons(port);
    if (connect(sockfd,&serv_addr,sizeof(serv_addr)) < 0)  //si el servidor no esta disponible
        error("ERROR conectandose al servidor, intente nuevamente mas tarde");

    disponibilidad(sockfd);   //chequea si pude conectarme exitosamente y el servidor tiene lugar
    jugar(sockfd);  //funcion principal del juego
    return 0;
}
//-------------------------------------------------------------------------------------

//Chequea si el servidor puede aceptar mas procesos o no
void disponibilidad(int socket)
{
    int err;
    char buffer[MAXBUFF];
    bzero(buffer,MAXBUFF);
    err = read(socket,buffer,MAXBUFF);      //leo el mensaje del servidor
    if (err < 0)
        error("ERROR comunicandose con el servidor, Intente nuevamente mas tarde");

    if(strcasecmp(buffer,"SERVIDOR OCUPADO")==0){  //si el mensaje es de ocupado salgo
        printf("Servidor Ocupado.\nIntente Nuevamente mas tarde\n\n");
        Enter();
        exit(0);
    }else{
        if(strcasecmp(buffer,"OK")==0){  //si me da permiso sigo
            printf("Conexion con el Servidor Establecida\n\n");
            Enter();
        }else{  //si no es ninguna de las 2 hay algo mal en el msj
            printf("Error conectandose con el servidor. Respuesta inconsistente #1");
        }
    }
}

//Crea un bucle para seguir jugando mientras no sea un EXIT
void jugar(int socket)
{
    char buffer[MAXBUFF];
    int ganados = 0;
    int perdidos = 0;
    printf("\t\tGANADOS||PERDIDOS\n");  //muestra el sistema de puntaje de ganados y perdidos
    printf("\t\t  %d   ||   %d\n\n",ganados,perdidos);
    ingresar_comando(buffer);   //pide el comando
    tratar_comando(socket,buffer);   //le manda el comando al servidor
    while(strcasecmp(buffer,"EXIT")!=0){
        if(strcasecmp(buffer,"NEW")==0){
            game(socket,buffer,&ganados,&perdidos);   //funcion de la partida (el juego)
        }else{
            if(strcasecmp(buffer,"HELP")==0){
                help_comando();   //muestra todos los help
                help_move();
                help_objetivo();
            }
        }
        printf("\t\tGANADOS||PERDIDOS\n");
        printf("\t\t  %d   ||   %d\n\n",ganados,perdidos);
        ingresar_comando(buffer);
        tratar_comando(socket,buffer);
    }
    printf("GRACIAS POR HABER JUGADO\n");
}
//------------------------------------------------

//Pide el ingreso de un comando
void ingresar_comando(char buffer[])
{
    printf("INGRESE COMANDO:\n");  //pide el ingreo del comando
    fgets(buffer,MAXBUFF-1,stdin);
    fflush(stdin);  // x lo tanto tengo que pararme en el ultimo y borrarlo
    buffer[strlen(buffer)-1]='\0';
    system("clear");
    while(!comando_valido(buffer)){
        help_comando();   //si el comando esta mal muestra la ayuda de los comandos
        printf("REINGRESE COMANDO:\n");
        fgets(buffer,MAXBUFF-1,stdin);
        fflush(stdin);  // x lo tanto tengo que pararme en el ultimo y borrarlo
        buffer[strlen(buffer)-1]='\0';
        system("clear");
    }
}

//Valida el ingreso de un comando valido
int comando_valido(char buffer[])
{
    if((strcasecmp(buffer,"NEW")==0) || (strcasecmp(buffer,"EXIT")==0) || (strcasecmp(buffer,"HELP")==0)){
        return(1);
    }else{
        return(0);
    }
}

//Envia el comando
void tratar_comando(int socket,char buffer[])
{
    if(strcasecmp(buffer,"HELP")!=0){
        char control[MAXBUFF];
        strcpy(control,buffer);
        IO(socket,control);
        if(strcasecmp(control,"OK")!=0){
            server_error(socket,buffer,1);
        }
    }
}

//Funcion de la partida cuando se reinicia entra de nuevo
void game(int socket,char buffer[],int *ganados,int *perdidos)
{
    strcpy(buffer,"/0");  //fuerzo el /0 para que entre por primera vez
    int control = 0;   //indica que no es un comando new exit y eso
    int end = 0;
    tlista_s jugadas;   //una lista con las jugadas anteriores para que no se repitan
    char mov[NROJUGADAS+1];     //PARA EL AUTOMATICO
    char mov_ant[NROJUGADAS+1]; //PARA EL AUTOMATICO
    int automatico=elegir_tipo();
    int bien = -1;
    int regular = -1;
    int mi_bien = -1;   //PARA EL AUTOMATICO
    int mi_regular = -1;//PARA EL AUTOMATICO
    printf("EL JUEGO VA A COMENZAR\n");
    Enter();
    system("clear");
    bzero(mov,NROJUGADAS+1);  //pongo en /0 mis movidas (para AUTOMATICO)
    bzero(mov_ant,NROJUGADAS+1);
    if(automatico){
        printf("MODO AUTOMATICO COMENZANDO\n");  //indica que se prepare para la jugadas
    }
    lcrear(&jugadas);     //inicializo la lista
    while(!control && !end){
        if(automatico){
            Enter();
            bzero(buffer,MAXBUFF);
            estrategia(&jugadas,mov,mov_ant,bien,regular,&mi_bien,&mi_regular);  //genero la estrategia
            strcpy(buffer,mov);
            printf("%s\n",buffer);
        }else{
            move(buffer);  //si no es automatico pide el ingreso
        }
        if(strcasecmp(buffer,"HELP")==0){
            help_comando();   //si se pide ayuda muestra esto
            help_move();
            help_objetivo();
        }else{
            IO(socket,buffer);   //manda y recibe
            tratar_jugada(buffer,&control,automatico,&bien,&regular);  //trata lo recibido
        }
    }
    if(strcasecmp(buffer,"WIN")==0){ //si cuando salis ganaste
        strcpy(buffer,"OK");
        IO(socket,buffer);  //hace IO de la ultima jugada y la trata
        tratar_jugada(buffer,&control,automatico,&bien,&regular);
        printf("ADIVINASTE!!!!\n\n");
        *ganados = *ganados+1;  //suma uno a los juegos ganados
    }else{
        if(strcasecmp(buffer,"END")==0){  //si se perdio
            strcpy(buffer,"OK");
            IO(socket,buffer);   //hace IO de la ultima jugada y la trata
            tratar_jugada(buffer,&control,automatico,&bien,&regular);
            printf("GAME OVER\n\n");
            *perdidos = *perdidos+1;  //suma uno a los juegos perdidos
        }
    }
    strcpy(buffer,"ADV");  //se pide el codigo correcto
    IO(socket,buffer);   //se encvia el codigo correcto y se muestra
    printf("El codigo correcto es: %s\n",buffer);
    Enter();
    system("clear");
}
//---------------------------------------------------------------------------

//Pide la forma en que se juega
int elegir_tipo()
{
    int opc;
    int clr[8];
    printf("Elija su modo de juego:\n\n");
    printf("1) Modo Interactivo (Usted Juega)\n2) Modo Automatico (La maquina juega por usted)\n");
    scanf("%d",&opc);
    fflush(stdin);
    while(opc!=1 && opc!=2){
        printf("Reingrese opcion: ");
        scanf("%d",&opc);
    }
    fgets(clr,7,stdin);  //tengo que hacer xq sino me arrastra un enter
    fflush(stdin);
    system("clear");
    return(opc-1);
}

//Funcuion para la estrategia
void estrategia(tlista_s *jugadas,char mov[],char mov_ant[],int bien, int regular,int *mi_bien,int *mi_regular)
{
    if(bien>*mi_bien){     //si hay mas bien que en la mano anterior sigo
        strcpy(mov_ant,mov);
        *mi_bien = bien;  //reemplazo los bien, regular y las jugadas
        *mi_regular = regular;
        selec_estrat(jugadas,mov,mov_ant,*mi_bien,*mi_regular);
    }else{
        if(bien == *mi_bien){    //si los bien son los mismo comparo el regular
            if(regular >= *mi_regular){ //si el regular es mayor sigo la jugada
                strcpy(mov_ant,mov);   //reemplazo los bien, regular y las jugadas
                *mi_bien = bien;
                *mi_regular = regular;
                selec_estrat(jugadas,mov,mov_ant,*mi_bien,*mi_regular);
            }else{   // si estoy en un peor caso vuelvo para atras y elijo otro camino
                strcpy(mov,mov_ant);   //vuelvo atras
                selec_estrat(jugadas,mov,mov_ant,*mi_bien,*mi_regular);
            }
        }else{      //vuelvo para atras y elijo otro camino
            strcpy(mov,mov_ant);
            selec_estrat(jugadas,mov,mov_ant,*mi_bien,*mi_regular);
        }
    }
}

//Selecciono la estrategia segun el caso en el que este
void selec_estrat(tlista_s *jugadas,char mov[],char mov_ant[],int mi_bien,int mi_regular)
{
    Tdato_String dato;  //tipo de dato usado en la lista (era mas simple que una matriz
    int existe=0;     //el existe para el buscar en la lista las jugadas anteriores
    do{
        if(mi_bien==-1 && mi_regular==-1){
            //strcpy(mov,"1234");  //puedo arrancar por aca tambien como un default
            generate(mov);
        }else{
            if(mi_bien==0 && mi_regular==0){
                generate(mov);
            }else{
                reemplazar(mov,NROJUGADAS-(mi_bien+mi_regular));   //la cantidad a remplazar son los que no estan ni bien ni regular
                mezclar(mov,mi_regular);   //la cantidad a mezclar son los que estan regular
            }
        }
        lbuscar(jugadas,mov,&existe);   //busco en mi lista de jugadas si existe o no
    }while(existe);  //si la jugada existe genero otra distinta con el mismo criterio
    strcpy(dato.clave,mov);
    linsertarorden(jugadas,dato,'a');  //inserto la jugada en la lista
}


//Pre{x<= NROJUGADAS}
void reemplazar(char mov[],int x)
{
    int posiciones[NROJUGADAS];  //las posiciones reemplazadas para que no se repitan
    char cambios[NROJUGADAS+1];
    int tope=-1; //el tope del arreglo de posiciones
    int ctope = -1;
    int pos = 0;
    char reemplazo;
    bzero(cambios,NROJUGADAS+1);
    srandom(time(NULL));  //inicializo la semilla
    while(x>0){      //la cantidad de reemplazos
        do{
            pos=rnd(NROJUGADAS);   //elijo la posicion a cambiar al azar
        }while(existe(posiciones,tope,pos));//si la pos ya fue elejida, elijo otra
        tope = tope + 1;
		posiciones[tope] = pos;
		do{
		    reemplazo=rnd(10)+'0';  //le sumo el 0 en ASCII para formar el caracter
		}while(apariciones(mov,reemplazo)!=0 || apariciones(cambios,reemplazo));  //si el numero a elejir existe,elijo otro
		mov[pos]=reemplazo;  //lo reemplazo
		ctope = ctope + 1;
		cambios[ctope] = reemplazo;
		x=x-1;
    }
}

//mezcla determinadas posiciones del arreglo
void mezclar(char mov[],int x)
{
    int pos = 0;
    int pos2 = 0;
    char aux[NROJUGADAS+1];
    strcpy(aux,mov);
    char a;
    srandom(time(NULL));
    if(x!=0){
        do{
            pos=rnd(NROJUGADAS);  //elije la posicion 1 y la 2
            pos2=rnd(NROJUGADAS);
            a = mov[pos];
            mov[pos] = mov[pos2];
            mov[pos2] = a;
        }while(diferencias(mov,aux)<x);  //mientras los cambios no sean los pedidos en x sigue barajando
    }
}

//se fija si existe el valor bus
int existe(int ar[],int tope,int bus)
{
    int i = 0;
    while(i<=tope && ar[i]!=bus){
        i=i+1;
    }
    if(i>tope){
        return(0);
    }else{
        return(1);
    }
}

//determina los caracteres distintos entre ar1 y ar2
int diferencias(char ar1[],char ar2[])
{
    int i = 0;
	int cambio = 0;
	while(i<strlen(ar1)){
		if(ar1[i]!=ar2[i]){
		    cambio++;
		}
		i = i+1;
	}
	return(cambio);
}

//Genera el codigo para la jugada automatica
void generate(char str[])
{
	int i = 0;
	char x;
	srandom(time(NULL));
	while(i<=NROJUGADAS-1){
		x=rnd(10)+'0';  //le sumo el 0 en ASCII para formar el caracter
		if(apariciones(str,x)==0){
			str[i]=x;
			i=i+1;
		}
	}
}

//Me devuelve un random entre 0 y max
int rnd(int max)
{
    return (random() % max);
}

//Chequea si existe o no en la cadena
int apariciones(char ar[],char x)
{
	int i = 0;
	int ap = 0;
	while(i<strlen(ar)){
		if(ar[i]==x){
		    ap++;
		}
		i = i+1;
	}
	return(ap);
}

//Pide el ingreso del movimiento
void move(char buffer[])
{
        printf("Ingrese jugada (Ingrese 'Exit' para salir de la partida): ");
        bzero(buffer,MAXBUFF);
        fgets(buffer,MAXBUFF-1,stdin); //si hago esto me toma /n
        fflush(stdin);  // x lo tanto tengo que pararme en el ultimo y borrarlo
        buffer[strlen(buffer)-1]='\0';
        while(!move_val(buffer)){
            printf("Reingrese jugada (Ingrese 'Exit' para salir de la partida): ");
            bzero(buffer,MAXBUFF);
            fgets(buffer,MAXBUFF-1,stdin); //si hago esto me toma /n
            fflush(stdin);
            buffer[strlen(buffer)-1]='\0';
        }
}
//-----------------------------------------------------------------------

//Valida el movimiento
int move_val(char buffer[]) // valido el ingreso
{
    if((strcasecmp(buffer,"EXIT")==0) || (strcasecmp(buffer,"HELP")==0)){
        return(1);
    }else{
        if(strlen(buffer)>4){ // si es mayor al maximo de la jugada
            printf("Su jugada excedio los 4 numeros (0-9)\n");
            return(0);
        }else{
            if(strlen(buffer)<4){  // si es menor al nro de la jugada
                printf("Su jugada tiene menos que 4 numeros\n");
                return(0);
            }else{
                if(atoi(buffer)==0){ //si no se puede convertir
                    printf("La jugada debe contener 4 numeros (del 0 al 9)\n");
                    return(0);
                }else{
                    int i=0;
                    while(i<=NROJUGADAS-1 && isdigit(buffer[i])){ //arranca de indice 0
                        i++;
                    }
                    if(i==NROJUGADAS){
                        i=0;
                        while(i<=NROJUGADAS-1 && apariciones(buffer,buffer[i])==1){
                            i++;
                        }
                        if(i==NROJUGADAS){
                            return(1);
                        }else{
                            printf("Los numeros no se pueden repetir\n");
                            return(0);
                        }
                    }else{
                        printf("La jugada debe contener 4 numeros (del 0 al 9)\n");
                        return(0);
                    }
                }
            }
        }
    }
}
//-----------------------------------------------------------------------------------------

//Manda y recibe el msj (buffer) entre el socket y lo pone en buffer
void IO(int socket,char buffer[])
{
    int err;

    err = write(socket,buffer,strlen(buffer));
    if (err < 0)
        error("ERROR comunicandose con el servidor, Intente nuevamente mas tarde");
    bzero(buffer,MAXBUFF);
    err = read(socket,buffer,MAXBUFF);
    if (err < 0)
        error("ERROR comunicandose con el servidor, Intente nuevamente mas tarde");
}
//--------------------------------------------

void tratar_jugada(char buffer[],int *control,int automatico,int *bien,int *regular)
{
    char *ptr;
    *control = ((strcasecmp(buffer,"EXIT")==0) || (strcasecmp(buffer,"WIN")==0) || (strcasecmp(buffer,"END")==0));
    if(!*control){
        int count=0;
        if((ptr = strtok(buffer," "))!=NULL){
            count++;  //cuento que strtok me de bien los 3 que necesito
            printf("%s)\n",ptr);   //divido el numero de la jugada
            if((ptr = strtok(NULL," "))!=NULL){
                count++;
                printf("Bien: %s\n",ptr);  //saco el bien
                if(automatico){
                    *bien = atoi(ptr);  //para el automatico paso el bien para la estrategia
                }
                    if((ptr = strtok(NULL," "))!=NULL){
                        count++;
                        printf("Regular: %s\n\n",ptr);  //saco el regular
                        if(automatico){
                            *regular = atoi(ptr);
                        }
                    }
            }
        }
        if(count!=RESPUESTA){  //si te da menos el strtok no encontre los 3 que necesito
            printf("Error");
            server_error(socket,buffer,2);
        }
    }
}

//Manda un exit al server antes de cerrar el cliente y cierra
void server_error(int socket,char buffer[],int error)
{
    strcpy(buffer,"EXIT");
    IO(socket,buffer);
    switch(error){
        case 1:
            printf("Error iniciando la comunicacion con el servidor, contactese con el proveedor\n\n");
        break;

        case 2:
            printf("Respuesta del servidor inconsistente , contactese con el proveedor\n\n");
        break;
    }
    Enter();
    system("clear");
    exit(0);
}
//------------------------------------------------

//Muestra la ayuda de los comandos
void help_comando()
{
    system("clear");
    printf("Comandos validos:\n\n");
    printf("New: Crea un juego nuevo\n");
    printf("Help: Muestra la ayuda\n");
    printf("Exit: Sale del juego\n");
    Enter();
    system("clear");
}

//Muestra la ayuda de las jugadas
void help_move()
{
    system("clear");
    printf("Jugadas validas:\n\n");
    printf("Exit: Sale de la partida actual y te lleva al menu comandos\n");
    printf("4 digitos numericos que no se pueden repetir\n");
    Enter();
    system("clear");
}

//Muestra los objetivos del juego
void help_objetivo()
{
    system("clear");
    printf("Objetivo:\n\n");
    printf("Adivinar el numero de 4 digitos del generador en %d intentos\n",MAX_INTENTOS);
    printf("Se puede jugar tanto en:\n    Modo Interactivo: Jugando el usuario contra la maquina\n");
    printf("    Modo Automatico: Dejando que la computadora decida las jugadas)\n");
    Enter();
    system("clear");
}
