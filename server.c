/*This file is part of Network_Programming.

    Network_Programming is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 2 of the License, or
    (at your option) any later version.

    Network_Programming is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Network_Programming.  If not, see <http://www.gnu.org/licenses/>.

*/


/* A simple server in the internet domain using TCP
   The port number is passed as an argument 
   This version runs forever, forking off a separate 
   process for each connection
   gcc server2.c -lsocket
*/
#include <stdio.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>

void dostuff(int); /* function prototype */
void error(char *msg)
{
    perror(msg);
    exit(1);
}

int main(int argc, char *argv[])
{
     int sockfd, newsockfd, portno, clilen, pid;
     struct sockaddr_in serv_addr, cli_addr;

     if (argc < 2) {
         fprintf(stderr,"ERROR, no port provided\n");
         exit(1);
     }
     sockfd = socket(AF_INET, SOCK_STREAM, 0);
     if (sockfd < 0) 
        error("ERROR opening socket");
     bzero((char *) &serv_addr, sizeof(serv_addr));
     portno = atoi(argv[1]);
     serv_addr.sin_family = AF_INET;
     serv_addr.sin_addr.s_addr = INADDR_ANY;
     serv_addr.sin_port = htons(portno);
     if (bind(sockfd, (struct sockaddr *) &serv_addr,
              sizeof(serv_addr)) < 0) 
              error("ERROR on binding");
     listen(sockfd,5);
     clilen = sizeof(cli_addr);
     while (1) {
         newsockfd = accept(sockfd, 
               (struct sockaddr *) &cli_addr, &clilen);
         if (newsockfd < 0) 
             error("ERROR on accept");
         pid = fork();
         if (pid < 0)
             error("ERROR on fork");
         if (pid == 0)  {
             close(sockfd);
             dostuff(newsockfd);
             exit(0);
         }
         else close(newsockfd);
     } /* end of while */
     return 0; /* we never get here */
}

/******** DOSTUFF() *********************
 There is a separate instance of this function 
 for each connection.  It handles all communication
 once a connnection has been established.
 *****************************************/
void dostuff (int sock)
{
   char * commands[2] = {"quit\n", "file\n"};
   
   int n;
   char buffer[256];
   bzero(buffer,256);
   while (strcmp(buffer, commands[0]) != 0) {   
       bzero(buffer,256);
       n = read(sock,buffer,255);
       if (strcmp(buffer, commands[1]) == 0) {
            bzero(buffer,256);
            read(sock, buffer, 255);
            if (buffer[strlen(buffer)-1] == '\n')
                 buffer[strlen(buffer)-1] = '\0';
            int fd = open(buffer, O_CREAT | O_APPEND);
            while (n = read(sock, buffer, 255) > 0) {
                 printf("[DEBUG] does this write to console or the file\n");
                 write(fd, buffer, n); // FIX THIS
            }
       }
       if (n < 0) error("ERROR reading from socket");
       printf("Here is the message: %s\n",buffer);
       printf("Here is commands[0]: %s\n", commands[0]);
       printf("Here is commands[1]: %s\n", commands[1]);
       printf("Here is the result of strcmp(buffer, commands[1]): %d\n", strcmp(buffer, commands[1]));
       n = write(sock,"I got your message",18);
       if (n < 0) error("ERROR writing to socket");
   }
   int res;
   while(res = close(sock));
}
