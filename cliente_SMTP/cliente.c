/*******************************************************
Protocolos de Transporte
Grado en Ingenier�a Telem�tica
Dpto. Ingen�er�a de Telecomunicaci�n
Univerisdad de Ja�n

Fichero: cliente.c
Versi�n: 1.0
Fecha: 23/09/2012
Descripci�n:
Cliente de eco sencillo TCP.

Autor: Juan Carlos Cuevas Mart�nez

*******************************************************/
#include <stdio.h>
#include <winsock.h>
#include <time.h>
#include <conio.h>

#include "protocol.h"





int main(int *argc, char *argv[])
{
	time_t tiempo = time(0);
	struct tm *tlocal = localtime(&tiempo);
	char output[128];
	SOCKET sockfd;
	struct sockaddr_in server_in;
	char buffer_in[2024]="", buffer_out[2024]="",input[2024]="",asunto[2024]="",salida[2024]="",to[2024]="",from[2024]="";
	int recibidos=0,enviados=0;
	int estado=S_HELO;
	char option;
	int valor;
	int com = 0; 
	int control = 0;
	WORD wVersionRequested;
	WSADATA wsaData;
	int err;

	char ipdest[16];
	char default_ip[16]="127.0.0.1";

	//Inicializaci�n Windows sockets
	wVersionRequested=MAKEWORD(1,1);
	err=WSAStartup(wVersionRequested,&wsaData);
	if(err!=0)
		return(0);

	if(LOBYTE(wsaData.wVersion)!=1||HIBYTE(wsaData.wVersion)!=1)
	{
		WSACleanup();
		return(0);
	}
	//Fin: Inicializaci�n Windows sockets

	/*Creaci�n del Socket TCP del servidor.*/
	do{
		sockfd=socket(AF_INET,SOCK_STREAM,0); 
		/***************************************************************************************************************************
		Aqu� se crea el socket al utilizar la funci�n socket, la cual es la primitiva que nos permite crear un descriptor de socket.
		La funci�n socket se iguala a la variable sockfd, la cual recibir� el valor de retorno de la funci�n socket, si no ocurre 
		ning�n error, la funci�n devuelve un descriptor que referencia un nuevo socket (valor int). En otro caso devuelve el valor -1
		(macro INVALID_SOCKET), el c�digo de error espec�fico se puede obtener llamando a WSAGetLastError (o GetLastError). 

		En caso de no tener un fallo, este valor entero no negativo, ser� el que llamaremos un descriptor socket, y es pasado a otras
		funciones API para identificar la abstraci�n socket sobre la cual se va a llevar a cabo la operaci�n.

		int socket(int af, int type, int protocol); 
		af: Familia de direcciones, en este caso AF_INET -> IPv4.
		type: Especificaci�n del tipo del nuevo socket, en este caso SOCK_STREAM -> usar conexiones TCP. 
		protocol: Indica el procolo dentro de la familia elegida -> Por defecto usamos 0, protocolo por defecto de la familia.
		****************************************************************************************************************************/
		if(sockfd==INVALID_SOCKET) //En caso de que la funci�n socket = sockfd me devuelva un error (INVALID_SOCKET), imprimo error al 
		{                          //usuario a la hora de crear el Socket. 
			printf("CLIENTE> ERROR AL CREAR SOCKET\r\n");
			exit(-1);
		}
		else //En caso de que no haya error, se imprime al cliente que se ha creado el SOCKET correctamente. 
		{
			printf("CLIENTE> SOCKET CREADO CORRECTAMENTE\r\n");

			//Pasamos a pedirle al direcci�n IP de destino al usuario. 
			printf("CLIENTE> Introduzca la IP destino (pulsar enter para IP por defecto): ");
			gets(ipdest); //gets me toma lo que imprimo en la ventana CMD. 
			/*strcmp, me compara los dos string que se le introducen como argumentos, y en caso de ser iguales, dir�a que ipdest es 0 
			y por tanto el usuario quiere usar la IP por defecto.*/
			if(strcmp(ipdest,"")==0)
				strcpy(ipdest,default_ip); //En caso de cumplirse el if, me copia en ipdest la direcci�n default_ip. 

			//Estructuras de direcciones de IPv4.
			server_in.sin_family=AF_INET;                //Campo de la estructura sockaddr_in para el Protocolo de Internet. 
			server_in.sin_port=htons(TCP_SERVICE_PORT);  //Campo de la estructura sockaddr_in para introducir en el que escucha el servidor, el puerto que est� ya definido como #define TCP_SERVICE_PORT 6000.
			server_in.sin_addr.s_addr=inet_addr(ipdest); //Campo de la estructura sockaddr_in para introducir la direcci�n de Internet (32 bits).
			//inet_addr: function converts a string containing an IPv4 dotted-decimal address into a proper address.
			//htons: function makes sure that numbers are stored in memory in network byte order
			estado=S_HELO;//Inicializo el estado en S_HELO, estado 0 definido en la cabecera protocol.h. 

			/***************************************************************************************************************************
			Un socket TCP debe ser conectado a otro socket antes de que cualquier dato pueda ser enviado a trav�s de �l. El cliente 
			debe iniciar la conexi�n mientras el servidor espera pas�vamente por clientes que se conecten a �l. Para establecer la 
			conexi�n con el servidor, utilizamos la funci�n connect(). 

			int connect(int socket, struct sockaddr *foreignAddress, unsigned int addressLength)

			socket -> Es el descriptor creado para un determinado socket.

			foreignAddress -> Declarado como un puntero hacia un sockaddr ser� siempre un puntero a un sockaddr_in conteniendo la 
			direcci�n de internet y el puerto del servidor. 

			addressLength -> especifica la longitud de la direcci�n de la estructura

			Cuando connect() vuelve exit�samente, el socket es conectado y la comunicaci�n puede proceder con llamadas a send() y recv().
			****************************************************************************************************************************/
			if(connect(sockfd,(struct sockaddr*)&server_in,sizeof(server_in))==0)
			{
				printf("CLIENTE> CONEXION ESTABLECIDA CON %s:%d\r\n",ipdest,TCP_SERVICE_PORT);


				//Inicio de la m�quina de estados
				do{
					switch(estado)
					{
					case S_HELO:
						strcpy(salida,"");
						strcpy(input,"");
						strcpy(from,"");
						strcpy(to,"");
						strcpy(buffer_in,"");
						strcpy(buffer_out,"");
						estado=S_ORIG;
						// Se recibe el mensaje de bienvenida
						break;

					case S_ORIG:
						
						printf("CLIENTE>Introduzca su direccion: (enter para salir): ");
						
						gets(input);

						if(strlen(input)==0){
							sprintf_s (buffer_out, sizeof(buffer_out), "%s %s",SD,CRLF);
							estado=S_QUIT;
						}else{
							sprintf_s (buffer_out, sizeof(buffer_out), "MAIL FROM: %s%s",input,CRLF);
							strcat(from,input);
							sscanf(buffer_in,"%d",&com);
							if(com>=200 && com<300)
								estado=S_DEST;	}					
						break;
					case S_DEST:
						
						printf("CLIENTE>Introduzca la direccion de destino: (enter para salir): ");
						gets(input);
						if(strlen(input)==0){
							sprintf_s (buffer_out, sizeof(buffer_out), "%s%s",SD,CRLF);
							estado=S_QUIT;
						}else{
							sprintf_s (buffer_out, sizeof(buffer_out), "RCPT TO:%s%s",input,CRLF);	
							strcat(to,input);
							estado=S_DATA;	}			
						break;

					case S_DATA:
						sscanf(buffer_in,"%d",&com);
						if(com>=200 && com<300){

							sprintf_s (buffer_out, sizeof(buffer_out), "DATA%s",CRLF);	
							estado=S_SUBJ;}else estado = S_DEST;
							break;
					case S_SUBJ:
						
						printf("CLIENTE>Introduce el asunto(enter para salir): ");	
						gets(asunto);
						if(strlen(asunto)==0){
							sprintf_s (buffer_out, sizeof(buffer_out), "%s%s",SD,CRLF);
							estado=S_QUIT;
						}else{
							estado=S_MENS;}

					case S_MENS:
						
						printf("CLIENTE>Introduce el mensaje(fin para acabar): ");
						do{
							gets(input);	

							strftime(output,128,"%d/%m/%y %H:%M:%S",tlocal);
							if(strcmp(input,"fin")!=0){
								valor = 1;
								strcat(salida,CRLF);
								strcat(salida,input);}else valor = 0;

						}while(valor==1);
						sprintf_s (buffer_out, sizeof(buffer_out), "Subject: %s%sTo: %s%sFrom: %s%sDate: %s%s%s%s.%s",asunto,CRLF,to,CRLF,from,CRLF,output,CRLF,salida,CRLF,CRLF);
						strcpy(salida,"");
						strcpy(input,"");
						strcpy(from,"");
						strcpy(to,"");
						
						estado=S_ORIG;
						break;


					}
					/***************************************************************************************************************************
					Envio de mensajes TCP.
					int send(int socket, const void *msg, unsigned int msg_len, int flags);

					socket -> Es el descriptor creado para un determinado socket.

					msg    -> Apunta al mensaje para enviar. 

					msgLength -> Es la longitud (bytes) del mensaje.

					send me devuelve el n�mero de bytes si no hay error, si me env�a un -1 es error y un 0 es que se libera la conexi�n de forma 
					acordada o por alg�n problema entre servidor y cliente. 
					****************************************************************************************************************************/
					if(estado!=S_HELO){//En todo momento que el estado no sea S_HELO, se ejecutan las funciones recv y send. Las cuales
						//son las que nos permitir�n transmitir y recibir informaci�n del servidor. 
						enviados=send(sockfd,buffer_out,(int)strlen(buffer_out),0);

						if(enviados<0) {//En el caso de que la funci�n send devuelva un valor negativo informo al usuario del error. 
							DWORD error=GetLastError();//Tomo el tipo de error producido. 
							printf("CLIENTE> Error %d en el envio de datos\r\n",error);//Imprimo el error al usuario. 
							estado=S_QUIT;//Cierro la conexi�n. 
						}
					}else{
						if(enviados<=0)//En el caso de que send me devuelva un valor negativo/cero.
						{
							DWORD error=GetLastError();//Tomo el error que ha tenido lugar. 
							if(enviados<0)//En caso de que se termine la conexi�n aviso al usuario.
							{
								printf("CLIENTE> Error %d en envio de datos\r\n",error);
								estado=S_QUIT;
							}
							else
							{
								printf("CLIENTE> Conexion con el servidor cerrada\r\n");
								estado=S_QUIT;
							}
						}
					}
					/********************************************************************************************************************************
					Envio de mensajes TCP.
					int recv(int socket, void *rcvBuffer, unsigned int bufferLength, int flags);

					socket       -> Es el descriptor creado para un determinado socket.

					rcvBuffer    -> Apunta al buffer --que es, un �rea en memoria como un car�cter array-- donde los datos recibidos ser�n colocados.

					bufferLength -> Nos da la longitud del buffer, esta longitud es el n�mero m�ximo de bytes que pueden ser recibidos de una vez.

					recv me devuelve el n�mero de bytes si no hay error, si me env�a un -1 es error y un 0 es que se libera la conexi�n de forma 
					acordada o por alg�n problema entre cliente y servidor. 
					*******************************************************************************************************************************/
					recibidos=recv(sockfd,buffer_in,512,0);
					if(recibidos<=0)//Compruebo lo que me devuelve la funci�n recv, y en caso de devolver valores de error se lo hago saber al usuario y libero la conexi�n.
					{
						DWORD error=GetLastError();
						if(recibidos<0)
						{
							printf("CLIENTE> Error %d en la recepcion de datos\r\n",error);
							estado=S_QUIT;
						}
						else
						{
							printf("CLIENTE> Conexion con el servidor cerrada\r\n");
							estado=S_QUIT;											
						}
					}else //En caso de que recv no me devuelva un valor de error, imprimo los datos que reciba el cliente del servidor y son guardados en el buffer_in.
					{
						buffer_in[recibidos]=0x00;//Los arrays en C deben acabar en 0x00.
						printf(buffer_in);//Imprimo lo que recibo. 
						//if(estado!=S_DATA && strncmp(buffer_in,OK,2)==0) 
						//estado++;  
					}

				}while(estado!=S_QUIT);



			}
			else
			{
				printf("CLIENTE> ERROR AL CONECTAR CON %s:%d\r\n",ipdest,TCP_SERVICE_PORT);//Error al tratar de conectar con el servidor. 
			}		
			// fin de la conexion de transporte
			closesocket(sockfd);//Comando para cerrar la conexi�n int closesocket(int socket);

		}	
		printf("-----------------------\r\n\r\nCLIENTE> Volver a conectar (S/N)\r\n");
		option=_getche();//lee un solo car�cter de la consola con repetici�n, es decir, el car�cter se muestra en la consola.

	}while(option!='n' && option!='N');



	return(0);

}
