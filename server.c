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

//add global variable that will hold the number of peers seen

void dostuff(int); /* function prototype */
int isKnownGossip(char*);

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

/*
 KLAUS: removeNewLines will only remove the first '/n', so I suggested a solution.
 */
int removeNewLines(char * str) {
    int len = strlen(str);
    int count = 0;

    int i, shiftRest;
    for (i = 0; i < len; i++) {
        if (str[i] == '\n') {
            count++;
            /*
             *We have to move everything in the remaining string downwards
             *At the core the goal is to turn [a,a,\n,a,\n,\0] into [a,a,a,\0]
             * which is trivially divided into
             * [a,a,a,\n,\0]
             * and into 
             * [a,a,a,\0]
             * and is easy to see that the removal of a \n is really
             * decrementing the position of every element after it
             */ 
            for (shiftRest = i; shiftRest < len; shiftRest++) {
                str[shiftRest] = str[shiftRest+1];
            }
            len--;
            i--;
        }
    }
    return count;
}
/*
 * takes: destination buffer, source buffer, length of dest, src buffers
 * returns 0 if successful append
 *         -1 if failure to append(space left in dest is too little for src) 
 */
int bufAppend(char * dest, char * src, int destLen, int srcLen) {
    int destN = strlen(dest);
    int srcN = strlen(src);
    if (destLen-destN < srcN) return -1; //not enough room
    int i;
    for (i = 0; i <= srcN; i++) { //using <= assures copying of '\0'
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

    char sha[126];
    char time[64];
    char message[1024];
    int bindex = 0, index = 0, length = strlen(buf);
    
    while (buf[bindex] != ':') {                        //skip GOSSIP:
        bindex++;
    }
    bindex++;
    while (buf[bindex] != ':') {                        //extract sha256 from gossip
        sha[index] = buf[bindex];
        index++;
        bindex++;
    }
    sha[index] = '\0';
    index = 0;
    bindex++;
    while (buf[bindex] != ':') {                        //extract time from gossip
        time[index] = buf[bindex];
        index++;
        bindex++;
    }
    time[index] = '\0';
    index = 0;
    bindex++;
    while (buf[bindex] != '%') {                        //extract message from gossip
        message[index] = buf[bindex];
        index++;
        bindex++;
    }
    message[index] = '\0';
    /* Checking if a message is known must occur here
     * A message is known if all three components of the message
     * already exist in our file
     * that is, we must have an identical [sha],[time],[message]
     * While the sha is redundent, [time] is important
     * for example, we could receive the following messages
     * John: "Hello" at 01:00pm
     * Mary: "Hello" at 01:01pm
     * The second message is not known even if the first is
     * However without checking the timestamp we would not know
     * 
     */
    if (isKnownGossip(message)) {
        error("DISCARDED");
    } else {
        FILE * fgossip;
        fgossip = fopen("ftest.txt", "a");               //open file to write
        
        fprintf(fgossip, "BEGIN\n");                     //write header
        fprintf(fgossip, "1:%s\n",sha);                  //write sha
        fprintf(fgossip, "2:%s\n",time);                 //write timestamp
        fprintf(fgossip, "3:%s\n",message);              //write message
        fprintf(fgossip, "END\n");                       //write footer
        
        if (fclose(fgossip)) { error("File not closed properly"); };  //close file
        
        //broadcast the message (I have not idea how :P)
        
        error(message);                                  //print message
    }
}
/*
 * takes a message and checks if the message exists in GOSSIP file.
 * returns 1 if it does find the message.
 * returns 0 if it does not find the message.
 */
int isKnownGossip(char* message) {
    FILE * fgossip;
    fgossip = fopen("ftest.txt", "r");                  //open file
    
    char currC;                                         //holds current char
    int skipFlag = 0, index = 0;
    while (fscanf(fgossip,"%c", &currC) == 1) {         //is eof?
        if (!skipFlag) {
            if (message[index] == '\0' && currC == '\n') {     //end of string? Found it!
                if (fclose(fgossip)) { error("File not closed properly"); };
                return 1;
            } else if (currC != message[index]){        //not a match
                skipFlag = 1;
            } else {                                    //match. Check the next char
                index++;
            }
        } else {                                         //skip the entire line
            if (currC == '\n'){
                index = 0;
                skipFlag = 0;
            }
        }
    }
    if (fclose(fgossip)) { error("File not closed properly"); };
    return 0;
}

/*
 *   adds a peer to our peer set
 *   returns 0 on successful addition
 *   returns -1 on any failure
 */
int PEER(char * buf) {
    //exctract name, ip, address
    char name[200]; //tbh no name should be more than 15 or so characters
    char port[6]; //65536\0
    char ip[17]; //nnn.nnn.nnn.nnn\0
    bzero(name,200);
    bzero(port,6);
    bzero(ip,17);
    int index = 0;
    while (buf[index] != ':') { index++;} //skip PEER
    int offset = 0;
    while (buf[index] != ':') { name[offset++] = buf[index++];}
    offset = 0;
    while (buf[index] != ':') { ip[offset++] = buf[index++];}
    //if (KNOWN) {IGNORE}


    /*
     * now that data has been gathered into three strings
     * put this data into a file
     *
     */
     FILE * fpeers;
     fpeers = fopen("fpeerstest.txt","a");
     fprintf(fpeers, "BEGIN\n");
     fprintf(fpeers, "1:%s\n", name);
     fprintf(fpeers, "2:%s\n", port);
     fprintf(fpeers, "3:%s\n", ip);
     fprintf(fpeers, "END\n");

     if (fclose(fpeers)) { error("File not closed properly");}


    //if (isKnownPeer())
        //updatePeer()
    //else
        //addPeer()
    
}
/*
 * NOTE: writes to socket directly, assumes stdin/out are mapped to socket
 * returns 1 on successful write
 * returns -1 on any error
 */
int PEERS(char * buf) {
    //printAllPeers()
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
   while (n = read(sock, bufTemp, 255)) {
       if (n==-1) error("Error on reading socket");
       int r = bufAppend(buffer, bufTemp, 1024, 256);
       if (r==-1) {
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
   //int res;
   //while(res = close(sock));
}
