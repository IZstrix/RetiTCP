/*
 * server.c
 *
 *  Created on: 22 nov 2023
 *      Author: Daniele Lillo 777283 &&  Michele Caricola 777304
 */


#if defined WIN32
#include <winsock.h>
#else
#define closesocket close
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <ctype.h>
#endif
#include <stdio.h>
#include <stdlib.h>
#include "protocol.h"

void errorhandler(char* errorMessage) {
	printf("%s", errorMessage);		// Error handling with error printing
}

void clearWinsock() {
	#if defined WIN32
	WSACleanup();                 //Winsock cleanup
	#endif
}

double add(double num1, double num2) 	//function for addiction
{
    double res = num1 + num2;
    return res;
}

double sub(double num1, double num2)	//function for subtraction
{
	double res = num1 - num2;
    return res;
}

double mult(double num1, double num2)	//function for multiplication
{
	double res = num1 * num2;
    return res;
}

double division(double num1, double num2)	//function for division
{
	double res = num1 / num2;
    return res;
}

int main(int argc, char *argv[]) {

	int port;

	if(argc > 1) {

		port = atoi(argv[1]);       //Sets the command line port if specified
	}
	else {

		port = PROTOPORT;     //Uses the port defined in the "protocol.h" file
	}

	#if defined WIN32	//Initializing Winsock on Windows platform

	WSADATA wsa_data;
	WORD version_requested;

	version_requested = MAKEWORD(2,2);
	int res = WSAStartup(version_requested, &wsa_data);

	if(res != 0) {

		printf("Error at WSAStartup() \n");
		printf("A usable WinSock DLL cannot be found\n");
		return 0;
	}
	#endif

	int my_socket;		//Creating socket

	my_socket = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);

	if(my_socket < 0) {

		errorhandler("Socket creation failed. \n");
		clearWinsock();
		return -1;
	}

	struct sockaddr_in sad;
	memset(&sad, 0, sizeof(sad));		// ensures that the additional bytes contain 0

	sad.sin_family = AF_INET;
	sad.sin_addr.s_addr = inet_addr("127.0.0.1");
	sad.sin_port = htons(port);

	if(bind(my_socket, (struct sockaddr*) &sad, sizeof(sad)) < 0) {

		errorhandler("bind() failed. \n");
		closesocket(my_socket);
		return -1;
	}

	struct sockaddr_in cad;
	int client_socket;
	int client_len;

	while(1) {

		puts("");
		printf("Waiting for a client to connect to the server... \n");

		//waiting connection
		if(listen(my_socket, QLEN) < 0) {

			errorhandler("listen() failed. \n");		//waiting connection failed
			closesocket(my_socket);
			clearWinsock();
			return -1;
		}

		client_len = sizeof(cad);

		if((client_socket = accept(my_socket, (struct sockaddr*) &cad, &client_len)) < 0) {

			errorhandler("accept() failed. \n");
			closesocket(my_socket);
			clearWinsock();
			return -1;
		}

		printf("Connection established with: %s:%d\n", inet_ntoa(cad.sin_addr), ntohs(cad.sin_port));

		//Sending data to client
		char* string_inputing = "Connection established\n"; //String that needs to be sent
		int string_len = strlen(string_inputing); // Determines length

		//sending data to client
		if (send(client_socket, string_inputing, string_len, 0) != string_len)
		{
			errorhandler("send() sent a different number of bytes than expected");
			closesocket(my_socket);
			clearWinsock();
			return -1;
		}

		int bytes_rcvd;
		char string_input[BUFFER_SIZE]; // buffer for data from the server
		memset(string_input, 0, BUFFER_SIZE); // ensures that the additional bytes contain 0
		printf("Operation Received from the client: "); // setup to print the echoed string

		//gets message from client
		if ((bytes_rcvd = recv(client_socket, string_input, BUFFER_SIZE - 1, 0)) <= 0)
		{
			errorhandler("recv() failed or connection closed prematurely");
			closesocket(client_socket);
			clearWinsock();
			return -1;
		}
		string_input[bytes_rcvd] = '\0';

		int connection_checking = 0;

		while(strcmp(string_input,"=")!= 0 && (connection_checking != 1))
		{

			char oper[1];
			int number1;
			int number2;
			char *token;

			double res;

			token = strtok(string_input, " ");
			strcpy(oper, token);

			token = strtok(NULL, " ");
			number1 = atoi(token);

			token = strtok(NULL, " ");
			number2 = atoi(token);

			printf(" %d %s %d. ", number1, oper, number2);


			switch(*oper)
			{
				case '+':
					res = add(number1, number2);
					break;

				case '-':
					res = sub(number1, number2);
					break;

				case '*':
					res = mult(number1, number2);
					break;

				case '/':
					res = division(number1, number2);
					break;

				default:
					puts("The selected operator isn't available.");
			}

			printf("The result is: %g\n", res);

			char buf[6];
			gcvt(res, 6, buf);	//converting result in string

			string_len = strlen(buf);

			//sending manipulated data to client
			if (send(client_socket, buf, string_len, 0) != string_len)
			{
				errorhandler("send() sent a different number of bytes than expected");
				closesocket(client_socket);
				clearWinsock();
				return -1;
			}

			memset(string_input, 0, BUFFER_SIZE); // ensures that the additional bytes contain 0
			printf("Received: "); // Setup to print the echoed string


			if ((bytes_rcvd = recv(client_socket, string_input, BUFFER_SIZE - 1, 0)) <= 0)
			{
				errorhandler("recv() failed or connection closed prematurely");
				closesocket(client_socket);
				connection_checking = 1;
			}

			string_input[bytes_rcvd] = '\0';
		}

	}

}
