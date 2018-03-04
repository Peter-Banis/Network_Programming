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

int removeNewLines(char * str) {
    int len = strlen(str);
    int state = 0; //if state is 1 we have encountered a newline and
                   //so we are going to be shifting
    unsigned int numNewline = 0; //no way we could have a string large enough for overflow
    for (int i = 0; i < len; i++) {
         if (state) {
             str[i]=str[i+1]
         }
         if (str[i] == '\n') { state = 1;} //once we've encountered a \n we never stop shifting, so there is no problem
    }
    int newLen = len-numNewline;
    if (str[newLen] == '\n') str[newLen] = '\0';
}
/*
 * takes: destination buffer, source buffer, length of dest, src buffers
 * returns 0 if successful append
 *         -1 if failure to append(space left in dest is too little for src) 
 */
int bufAppend(char * dest, char * src, int destLen, int srcLen) {
    int destN = strlen(dest);
    int srcN = strlen(source);
    if (destLen-destN < srcN) return -1; //not enough room
    for (int i = 0; i <= srcN; i++) { //using <= assures copying of '\0'
        dest[destN+i] = src[i];
    }
}
/*
 * takes a buffer with the gossip string
 * returns the number of peers we gossiped to
 *      (max number is int_max, if we gossiped to more it won't be reported)
 *      -1 if a peer could not be gossiped to
 *         could still have successful gossips to others even if -1 is returned
 */
int GOSSIP(char * buf) {

}
/*
 *   adds a peer to our peer set
 *   returns 0 on successful addition
 *   returns -1 on any failure
 */
int PEER(char * buf) {

}
/*
 * NOTE: writes to socket directly, assumes stdin/out are mapped to socket
 * returns 1 on successful write
 * returns -1 on any error
 */
int PEERS(char * buf) {
    

}


/******** DOSTUFF() *********************
 There is a separate instance of this function 
 for each connection.  It handles all communication
 once a connnection has been established.
 *****************************************/
void dostuff (int sock)
{
   char * commands[3] = {"GOSSIP", "PEER", "PEERS"};
   int n;
   int curUsed=0;//used for message concatenation
   char buffer[1024];  //will be used to hold the concatenated result
   bzero(buffer,1024); 
   char bufTemp[256]; //will be used to hold individual reads
   bzero(bufTemp, 256); 
   while (n = read(sock, bufTemp, 255) {
       if (n==-1) error("Error on reading socket");
       int r = bufAppend(buffer, bufTemp, 1024, 256);
       if (r==-1)dwa {
           //if this occurs then buffer should have some command to be executed
           //execute the commands it can and then retry bufAppend
           //this may seem  be pointless since we check for executable
           //commands every time we bufAppend succesfully
           //it may actually be pointless, but for now i'll leave it to be safe
       }
       else {
           //this means the append was successful
           //we will check if there is some command we can execute
           //if there is execute it

       }
   }
   int res;
   while(res = close(sock));
}
