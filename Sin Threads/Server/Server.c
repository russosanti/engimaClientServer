
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>

#define PORT 4000  //el puerto usado
#define SIZE_BUFF 128  //tamanio del buffer
#define MAX_ADIVINAR 4  //la cantidad de elementos de la cadena
#define CANT_JUGADAS 5 //cantidad de intentos
#define RAND_MAX 9 //el random maximo de 0 a 9



void error(char *msg);
int server(int port);
void cliente (int sock);
void cant_proc(int *MAXPROC);
int proc_handler(int sig);
void connection (int sock, char buffer[]);
void generate(char str[]);
int tratar(int sock,int jugada,char buffer[],char adivinar[]);
void WR(int sock, char buffer[]);
void RW(int sock, char buffer[]);
int existe(char ar[],char x);
void client_error(int socket,char buffer[]);


int proc; //variable global de procesos la defino aca para poder usarla en el handler


int main()
{
	server(PORT);   //llama a la rutina servidor
	return(0);
}


void error(char *msg)
{
    perror(msg);  //toma los errores del sistema
    exit(1);      //sale con error
}

//Pide el ingreso de un enter
void Enter()
{
    char enter[256];
    fflush(stdin);
    printf("Presione enter para continuar");
    fgets(enter,256,stdin);
    fflush(stdin);
    system("clear");
}
//-----------------------------------------------

int server(int port)
{
     int sockfd, newsockfd, portno, clilen, pid;
     struct sockaddr_in serv_addr, cli_addr;

     if (port<=0) {
         fprintf(stderr,"ERROR de configuracion del puerto\n");
         exit(1);                                        //evaluo el puerto
     }
     sockfd = socket(AF_INET, SOCK_STREAM, 0);   //creo un socket para los accept
     if (sockfd < 0) 
        error("ERROR creando el socket");
     bzero((char *) &serv_addr, sizeof(serv_addr));
     portno = port;
     serv_addr.sin_family = AF_INET;
     serv_addr.sin_addr.s_addr = INADDR_ANY;
     serv_addr.sin_port = htons(portno);
     if (bind(sockfd, (struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0)  //hago el bind del socket
    	 error("ERROR on binding");
     listen(sockfd,5);
     clilen = sizeof(cli_addr);
     proc = 0;  //cantidad de procesos abiertos
     int MAXPROC = 100;  //cantidad maxima de procesos por default
     cant_proc(&MAXPROC); //preg si se quiere configurar la cantidad maxima de procesos
     int n;  //n es para el error en el enviar (write) el estado del servidor
     printf("SERVIDOR INICIALIZADO\n\n");
     while (1) {
         newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen); //acepto el cliente y le creo un socket
         if (newsockfd < 0)
        	 error("ERROR on accept");
         proc = proc + 1;  //un proceso nuevo
         signal(SIGCHLD, proc_handler);  //me fijo si murio o no algun proceso
         if(proc>MAXPROC){  //si la cantidad de procesos es o no mayor al maximo
        	 proc = proc - 1;  //si es mayor rechazo al proceso y le mando en el socket que no puedo aceptar mas
        	 n = write(newsockfd,"SERVIDOR OCUPADO",16); //escribo que esta ocupado el server
        	 if (n < 0) error("ERROR writing - Conexion con el cliente perdida"); //si hay error escribiendo
         }else{
        	 n = write(newsockfd,"OK",16);  //escribo que puede aceptar mas clientes
        	 if (n < 0) error("ERROR writing - Conexion con el cliente perdida");
        	 pid = fork();  //hace el fork para el nuevo cliente
        	 if (pid < 0)  //si hay error en el fork
        		 error("ERROR on fork");
        	 if (pid == 0)  {       //si es el hijo
        		 close(sockfd);          //cierro el sockfd en el hijo y uso el new
        		 cliente(newsockfd); //y entro al juego
        		 exit(1);
        	 }else{
        		 close(newsockfd);  //cierra el socket y espera otra coneccion
        	 }
         }
     }
     return 0; //Nunca se llega pero debe devolver un valor
}

//configura la cantidad de procesos a aceptar
void cant_proc(int *MAXPROC)
{
	char opc;
	printf("Desea configurar la cantidad de procesos a aceptar? (S: SI , N: NO)\n");
	printf("(El default es 100)\n");
	scanf("%c",&opc);
	fflush(stdin);
	while(opc!='S' && opc!='N' && opc!='s' && opc!='n'){
		printf("La opcion puede ser 'S': SI o 'N': NO");
		scanf("%c",&opc);
		fflush(stdin);
	}
	system("clear");
	if(opc=='S' || opc=='s'){
		printf("Ingrese la maxima cantidad de procesos a aceptar por el servidor:\n");
		scanf("%d",MAXPROC);
		fflush(stdin);
		scanf("%c",&opc);  //para limpiar el buffer
		system("clear");
		printf("MAXIMA cantidad de procesos a aceptar cambiada\n\n");
		Enter();
	}
}

//handler para cuando muere un hijo
int proc_handler(int sig)
{
  pid_t pid;

  pid = wait(NULL);

  proc = proc - 1;

  printf("Procces %d exited.\n", pid);
}

//Funcion principal del juego
void cliente (int sock)
{
   // randomize
   char buffer[256];
   strcpy(buffer,"NEW");  //fuerzo la primer entrada
   int n;
   RW(sock,buffer);  //lee el comando
   while(strcasecmp(buffer,"EXIT")!=0){
	   if(strcasecmp(buffer,"NEW")==0){
		   connection(sock,buffer);
	   }
	   RW(sock,buffer);  //lee el siguiente comando
   }
}

//Controla la partida
void connection (int sock, char buffer[])
{
   int n;
   char adivinar[MAX_ADIVINAR+1];
   generate(adivinar); //genera la cadena a adivinar

   int jugada = 1;
   int i = 1;

   while(i && jugada<=CANT_JUGADAS){
	   bzero(buffer,256);
	   n = read(sock,buffer,SIZE_BUFF);  //lee el movimiento
	   if (n < 0) error("ERROR reading - Conexion con el cliente perdida");
	   i = tratar(sock,jugada,buffer,adivinar);  //trata el movimiento
	   n = write(sock,buffer,strlen(buffer));  //devuelve el resultado
	   if (n < 0) error("ERROR writing - Conexion con el cliente perdida");
	   jugada++;
   }
   bzero(buffer,256);
   n = read(sock,buffer,SIZE_BUFF);  //lee la petecion de el correcto
   if (n < 0) error("ERROR reading - Conexion con el cliente perdida");
   if(strcasecmp(buffer,"ADV")==0){  //se le envia el correcto
	   strcpy(buffer,adivinar);
   }else{
	   strcpy(buffer,"Error al leer peticion de adivinar, socket corrupto");
   }
   n = write(sock,buffer,strlen(buffer));  //se le manda el adivinar
   if (n < 0) error("ERROR writing - Conexion con el cliente perdida");
}

//genera el string para adivinar
void generate(char str[])
{
	int i = 0;
	char x;
	int y;
	srandom(time(NULL));
	while(i<=MAX_ADIVINAR-1){
		x=rnd(10)+'0';  //le sumo el 0 en ASCII para formar el caracter
		if(!existe(str,x)){
			str[i]=x;
			i=i+1;
		}
	}
}

//Devuelve random entre 0 y max
int rnd(int max) {
    return (random() % max);
  }

//Trata la cadena
int tratar(int sock,int jugada,char buffer[],char adivinar[])
{
	int bien = 0;
	int regular = 0;
	if(strcasecmp(buffer,"EXIT")==0 || strcasecmp(buffer,"NEW")==0){ //si es exit o new sale con 0
		return(0);
	}else{
		int i = 0;
		for(i=0;i<strlen(buffer);i++){
			if(buffer[i]==adivinar[i]){
				bien = bien+1;  //cuenta los bien
			}else{
				if(existe(adivinar,buffer[i])){
					regular = regular+1;  //cuenta los regular
				}
			}
		}
		bzero(buffer,256);
		if(bien==MAX_ADIVINAR){
			strcpy(buffer,"WIN");  //si ganaste te manda la palabra reservada WIN
			WR(sock,buffer);
			if(strcasecmp(buffer,"OK")!=0){
				client_error(sock,buffer);
			}
		}else{
			if(jugada==CANT_JUGADAS){
				strcpy(buffer,"END");  //se perdiste te manda la palabra reservada END
				WR(sock,buffer);
				if(strcasecmp(buffer,"OK")!=0){
					client_error(sock,buffer);
				}
			}
		}
		char aux[16];
		bzero(buffer,SIZE_BUFF);
		sprintf(aux,"%d",jugada); //armo el string de respuesta a la jugada
		strcat(buffer,aux);  //con el numero de jugada , bien y regular
		strcat(buffer," ");
		sprintf(aux,"%d",bien);
		strcat(buffer,aux);
		strcat(buffer," ");
		sprintf(aux,"%d",regular);
		strcat(buffer,aux);
		if(bien==MAX_ADIVINAR){
			return(0);   //devuelve 0 para salir de bucle, sino sigue jugando
		}else{
			return(1);
		}
	}
}

//Envia un msj y espera la respuesta
void WR(int sock, char buffer[])
{
	int n;
	n = write(sock,buffer,strlen(buffer));
	if (n < 0) error("ERROR reading - Conexion con el cliente perdida");
	bzero(buffer,SIZE_BUFF);
	n = read(sock,buffer,SIZE_BUFF);
	if (n < 0) error("ERROR writing - Conexion con el cliente perdida");
}

//Lee un msj y devuelve ok
void RW(int sock, char buffer[])
{
	int n;
	bzero(buffer,SIZE_BUFF);
	n = read(sock,buffer,SIZE_BUFF);
	if (n < 0) error("ERROR writing - Conexion con el cliente perdida");
	n = write(sock,"OK",strlen("OK"));
	if (n < 0) error("ERROR reading - Conexion con el cliente perdida");
}

//Chequea si existe x en ar
int existe(char ar[],char x)
{
	int i = 0;
	while(i<strlen(ar) && ar[i]!=x){
		i = i+1;
	}
	if(i>=strlen(ar)){
		return(0);
	}else{
		return(1);
	}
}

//Le manda al server Exit para que termine la jugada y cierra la aplicacion
void client_error(int socket,char buffer[])
{   //es por errores logicos no en tiempo de ejecucion
    strcpy(buffer,"EXIT");
    WR(socket,buffer);
    exit(0);
}
