/*
 * client.c
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
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "protocol.h"


void errorhandler(char *errorMessage) {     //Error handling with error printing
    printf("%s", errorMessage);
}

void clearWinsock() {     //Winsock cleanup
#if defined WIN32
    WSACleanup();
#endif
}

int isValidInput(char *input) {    //check input
    char operator;
    int num1, num2;

    if (sscanf(input, "%c %d %d", &operator, &num1, &num2) == 3) {
        if (operator == '+' || operator == '-' || operator == '*' || operator == '/') {
            return 1; // Input valid
        }
    }
    return 0; // Input invalid
}
//funzione principale
int main(void) {
#if defined WIN32
	//Initializing Winsock on Windows platform
    WSADATA wsa_data;
    WORD version_requested;

    version_requested = MAKEWORD(2, 2);
    int result = WSAStartup(version_requested, &wsa_data);

    if (result != 0) {
        printf("Error at WSAStartup() \n");
        printf("A usable WinSock DLL cannot be found\n");
        return -1;
    }
#endif

    int c_socket; 		// socket creation

    c_socket = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);

    if (c_socket < 0) {
        errorhandler("Socket creation failed. \n");    // Error creating socket

        clearWinsock();
        return -1;
    }

    struct sockaddr_in sad;
    memset(&sad, 0, sizeof(sad)); // setting memory

    sad.sin_family = AF_INET;
    sad.sin_addr.s_addr = inet_addr("127.0.0.1");  //Server IP Address
    sad.sin_port = htons(PROTOPORT);         //Server Port

    if (connect(c_socket, (struct sockaddr *)&sad, sizeof(sad)) < 0) {
        errorhandler("Connection failed. \n");
        closesocket(c_socket); // Connection to socket failed
        clearWinsock();
        return -1;
    }

    // receiving data from the server
    int bytes_rcvd;
    char buf[BUFFER_SIZE]; // buffer for data from the server
    memset(buf, 0, BUFFER_SIZE); // ensures that the additional bytes contain 0
    printf("Operation received: ");        // setup to print the echoed string

    if ((bytes_rcvd = recv(c_socket, buf, BUFFER_SIZE - 1, 0)) <= 0) {
        errorhandler("recv() failed or connection closed prematurely");
        closesocket(c_socket);
        clearWinsock();
        return -1;
    }
    buf[bytes_rcvd] = '\0';
    printf("%s", buf); // Print the echo buffer

    // Printing the message for the initial operation
    char output_str[BUFFER_SIZE];
    int len;
    while (1) {
        memset(output_str, '\0', BUFFER_SIZE);
        puts("Insert operation in this form : operator [space] num1 [space] num2");
        gets(output_str);          // String input from keyboard

        if (isValidInput(output_str)) {
            len = strlen(output_str);
            if (send(c_socket, output_str, len, 0) != len) {
                errorhandler("send() sent a different number of bytes "); // first error
                closesocket(c_socket);
                clearWinsock();
                return -1;
            }
            break;
        } else {
            puts("Invalid operation. Please try again.");    // Invalid operation
        }
    }

    while (output_str[0] != '=') {
        // receiving string from server
        bytes_rcvd = 0;
        char string_rec[BUFFER_SIZE]; // using buffer
        if ((bytes_rcvd = recv(c_socket, string_rec, BUFFER_SIZE - 1, 0)) <= 0) {
            errorhandler("recv() failed or connection closed prematurely");
            closesocket(c_socket);
            clearWinsock();
            return -1;
        }

        string_rec[bytes_rcvd] = '\0';
        printf("\nThe result of the operation is: %s\n", string_rec); // print the result of the operation
        puts("Insert again with the same form : ");
        gets(output_str);
        while ((output_str[0] != '+') && (output_str[0] != '-') && (output_str[0] != '/') && (output_str[0] != '*') && (output_str[0] != '=')) {

            puts("The input is not valid or is empty\n"); // controlling that the input  is valid
            gets(output_str);
        }

        // Sending string to server
        len = strlen(output_str);
        if (send(c_socket, output_str, len, 0) != len) {
            errorhandler("send() sent a different number of bytes");
            closesocket(c_socket);
            clearWinsock();
            return -1;
        }
    }


    closesocket(c_socket);      // closing connection
    clearWinsock();
    printf("\n");

    return (0);
}
