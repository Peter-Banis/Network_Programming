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
#include <unistd.h>
#include <ctype.h>

void dostuff(int);
int isKnown(char*);
int updateFile(char*, int);
int countDigit(int);
char* itoa(int, char*, int);
int PEER(char *);

void error(char *msg)
{
    perror(msg);
    exit(1);
}

int main(int argc, char **argv)
{
    int sockfd, newsockfd, portno, clilen, pid, c;
    struct sockaddr_in serv_addr, cli_addr;
    char * filePath;
    
    while ((c = getopt (argc, argv, "p:d:")) != -1)        //implementing getopt -p and -d
        switch (c)
        {
            case 'p':
                portno = atoi(optarg);
                break;
            case 'd':
                filePath = optarg;
                break;
            default:
            abort ();
        }

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) error("ERROR opening socket");
    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(portno);
    if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0)
            error("ERROR on binding");
    listen(sockfd,5);
    clilen = sizeof(cli_addr);
    while (1) {
        newsockfd = accept(sockfd,
            (struct sockaddr *) &cli_addr, &clilen);
        if (newsockfd < 0) error("ERROR on accept");
        pid = fork();
        if (pid < 0) error("ERROR on fork");
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
    bzero(message,1024);
    bzero(time,64);
    bzero(sha,126);
    int bindex = 0, index = 0;
    
    while (buf[bindex] != ':') { bindex++; } //skip GOSSIP:
    bindex++;
    while (buf[bindex] != ':') { sha[index++] = buf[bindex++]; }  //extract sha256 from gossip
    index = 0;
    bindex++;
    while (buf[bindex] != ':') { time[index++] = buf[bindex++]; }  //extract time from gossip
    index = 0;
    bindex++;
    while (buf[bindex] != '%') { message[index++] = buf[bindex++]; }  //extract message from gossip

    if (isKnown(message)) {
        error("DISCARDED");
    } else {
        FILE * fgossip;
        fgossip = fopen("ftest.txt", "a");               //open file to write
        fprintf(fgossip, "BEGIN\n");                     //write header
        fprintf(fgossip, "1:%s\n",message);              //write message
        fprintf(fgossip, "2:%s\n",time);                 //write timestamp
        fprintf(fgossip, "3:%s\n",sha);                  //write sha
        fprintf(fgossip, "END\n");                       //write footer
        
        if (fclose(fgossip)) { error("File not closed properly"); };  //close file
        
        /* ------------------ TO DO -------------------
         
        //broadcast the message (I have not idea how :P)
         
        -------------------------------------------  */
        
        error(message);                                  //print message
    }
}
/*
 * takes a message and checks if the message exists in GOSSIP file.
 * returns the line number were the message was found.
 * returns 0 if it does not find the message.
 */
int isKnown(char* obj) {
    int lineNumber = 1;
    
    if (access("ftest.txt", F_OK) == -1) { return 0; }  //test if the file exists
    FILE * fgossip;
    fgossip = fopen("ftest.txt", "r");                  //open file
    
    char currC;                                         //holds current char
    int skipFlag = 0, index = 0, objFlag = 0;
    while (fscanf(fgossip,"%c", &currC) == 1) {         //is eof?
        if (!skipFlag) {
            if (!objFlag) {
                if (currC == '1') {                     //line starts with 1? It is a message line.
                    objFlag = 1;
                    fscanf(fgossip,"%c", &currC);       //skiping ':'
                } else {
                    objFlag = 0;
                    skipFlag = 1;                       //line does start with 1? Skip it.
                }
            } else {
                if (obj[index] == '\0' && currC == '\n') {     //end of string? Found it!
                    if (fclose(fgossip)) { error("File not closed properly"); };
                    return lineNumber;
                } else if (currC != obj[index]){        //not a match
                    skipFlag = 1;
                    objFlag = 0;
                } else {                                    //match. Check the next char
                    index++;
                }
            }
        } else {                                         //skip the entire line
            if (currC == '\n'){
                index = 0;
                skipFlag = 0;
                lineNumber++;
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
    char name[200];         //tbh no name should be more than 15 or so characters
    char port[6];           //65536\0
    char ip[17];            //nnn.nnn.nnn.nnn\0
    
    bzero(name,200);
    bzero(port,6);
    bzero(ip,17);
    
    int index = 0;
    while (buf[index] != ':') { index++;} //skip PEER
    int offset = 0;
    index++;
    while (buf[index] != ':') { name[offset++] = buf[index++];}  //extract name from peer
    offset = 0;
    index++;
    while (buf[index] != ':') { port[offset++] = buf[index++];}  //extract port from peer
    offset = 0;
    index++;
    while (buf[index] != '%') { ip[offset++] = buf[index++]; }  //extract ip from gossip
    
    int lineToUpdate = isKnown(name);
    if (lineToUpdate) {
        if (updateFile(name, lineToUpdate) == -1) { return -1; }
    } else {
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
        
        if (fclose(fpeers)) { error("File not closed properly"); return -1; }
    }
    return 0;
}
/*
 * updates the address of a peer
 * returns 1 on successful update
 * returns -1 on any error
 */
int updateFile(char* ip, int line) {
    int deleteLine = line + 2;              //address line is 2 lines after the name line
    int index = 0;
    char currC;
    
    FILE * ffold;
    ffold = fopen("ftest.txt", "r");
    FILE * fupdated;
    fupdated = fopen("output.txt", "w");
    
    while (fscanf(ffold,"%c", &currC) == 1) {           //read until eof of the old file
        if (currC == '\n') { deleteLine--; }            //count the number of lines
        
        if (deleteLine == 1) {                          //found the line that will be replaced
            fprintf(fupdated, "\n3:%s\n", ip);          //print the new line
            deleteLine--;
            while (1) {                                 //skip the old line
                fscanf(ffold,"%c", &currC);
                if (currC == '\n'){
                    break;
                }
            }
        } else {
            fprintf(fupdated, "%c", currC);              //copy the unchanged
        }
    }
    
    if (fclose(ffold)) { error("File not closed properly"); return -1; }
    remove("ftest.txt");                                      //remove the old file
    if (fclose(fupdated)) { error("File not closed properly"); return -1; }
    rename("output.txt", "ftest.txt");                        //rename the new file
    return 1;
}
/*
 * NOTE: writes to socket directly, assumes stdin/out are mapped to socket
 * returns 1 on successful write
 * returns -1 on any error
 */
int PEERS() {
    char currC;
    int charNumber = 0, lineNumber = 0;
    if (access("ftest.txt", F_OK) == -1) { return -1; }   //test if the file exists
    FILE * fpeers;
    fpeers = fopen("ftest.txt", "r");
    
    while (fscanf(fpeers,"%c", &currC) == 1) {            //counting number of chars in the file
        if (currC != '\n') { charNumber++; }
        else { lineNumber++; }
    }
    if (fclose(fpeers)) { error("File not closed properly"); return -1; }
    int peersNumber = lineNumber/5;
    int totalChar = charNumber + 8 - (peersNumber * 14) + (peersNumber * 11) + countDigit(peersNumber);
    
    char message[totalChar + 1];                          //creating char array for the message
    bzero(message,totalChar + 1);
    
    int messageIndex = 0;
    
    char * intro = "PEERS|";
    
    for (int i = 0; i < strlen(intro); i++) {             //adding PEERS| to the message
        message[messageIndex++] = intro[i];
    }
    
    char peerNoArray[countDigit(peersNumber)];            //adding number of peers
    itoa(peersNumber,peerNoArray,10);
    
    for (int i = 0; i < strlen(peerNoArray); i++) {
        message[messageIndex++] = peerNoArray[i];
    }
    message[messageIndex++] = '|';
    
    fpeers = fopen("ftest.txt", "r");
    char * port = ":PORT=";
    char * ip = ":IP=";
    
    while (fscanf(fpeers,"%c", &currC) == 1) {            //adding peers to message
        if (currC == 'B' || currC == 'E') {
            while (1) {                                   //skip BEGIN and END lines
                fscanf(fpeers,"%c", &currC);
                if (currC == '\n') {
                    break;
                }
            }
        } else {
            if (currC == '1') {                           //distinuishing name line
                fscanf(fpeers,"%c", &currC);              //skip :
                while (1) {
                    fscanf(fpeers,"%c", &currC);
                    if (currC == '\n') {
                        break;
                    } else {
                        message[messageIndex++] = currC;  //adding name to message
                    }
                }
                for (int i = 0; i < strlen(port); i++) {  //adding :PORT=
                    message[messageIndex++] = port[i];
                }
            } else if (currC == '2') {                    //distinuishing port line
                fscanf(fpeers,"%c", &currC);              //skip :
                while (1) {
                    fscanf(fpeers,"%c", &currC);
                    if (currC == '\n') {
                        break;
                    } else {
                        message[messageIndex++] = currC; //adding port to message
                    }
                }
                for (int i = 0; i < strlen(ip); i++) {   //adding :IP=
                    message[messageIndex++] = ip[i];
                }
            } else {                                      //distinuishing ip line
                fscanf(fpeers,"%c", &currC);              //skip :
                while (1) {
                    fscanf(fpeers,"%c", &currC);
                    if (currC == '\n') {
                        break;
                    } else {
                        message[messageIndex++] = currC;  //adding ip to message
                    }
                }
                message[messageIndex++] = '|';
            }
        }
    }
    message[messageIndex++] = '%';                        //adding % to the end of message
    
    if (fclose(fpeers)) { error("File not closed properly"); return -1; }
    
    fprintf(stdin, "%s", message);                        //sending message
}
/*
 * counts the number of digits in a int
 */
int countDigit(int n)
{
    int count = 0;
    while (n != 0) {
        n = n / 10;
        ++count;
    }
    return count;
}
/*
 * converts int to char array
 */
char* itoa(int value, char* result, int base) {
    // check that the base if valid
    if (base < 2 || base > 36) { *result = '\0'; return result; }
    
    char* ptr = result, *ptr1 = result, tmp_char;
    int tmp_value;
    
    do {
        tmp_value = value;
        value /= base;
        *ptr++ = "zyxwvutsrqponmlkjihgfedcba9876543210123456789abcdefghijklmnopqrstuvwxyz" [35 + (tmp_value - value * base)];
    } while ( value );
    
    // Apply negative sign
    if (tmp_value < 0) *ptr++ = '-';
    *ptr-- = '\0';
    while(ptr1 < ptr) {
        tmp_char = *ptr;
        *ptr--= *ptr1;
        *ptr1++ = tmp_char;
    }
    return result;
}

/*
 *To clear processed commands is pretty easy
 *Take everything past that command and shift it down
 *by the size of the command
 * then we have to replace the now copied data with \0
 */
void clearBuffer(char * buffer, int howFar, int bufferSize) {
    int i = 0;
    for (i = howFar; i < bufferSize; i++) {
        buffer[i-howFar] = buffer[i];
        buffer[i] = '\0'; //will get rid of copied strings at the end
                          //that we no longer need
    }
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
           /*
            * A command exists to execute if:
            * There exists a string such that it starts with:
            *  G and ends with %
            *  PEER: and ends with %
            *  PEERS and ends with ?
            */
       int index = 0;
       if (buffer[0] == 'G') { //GOSSIP
           for (index = 0; index < 1024; index++) {
               if (buffer[index] == '%') {
                   char gssp[index];
                   strncpy(gssp, buffer, index);
                   GOSSIP(gssp);
                   clearBuffer(buffer, index);
                   break;
               }
           }
       } else if (buffer[5] == ':') { //only PEER has this character there
             for (index = 0; index < 1024; index++) {
                 if (buffer[index] == '%') {
                     char per[index];
                     strncpy(per, buffer, index);
                     PEER(per);
                     clearBuffer(buffer, index);
                     break;
                 }
             }
         } else {
               //must be peers
               PEERS();
               clearBuffer(buffer, 6);
          }            
   }
}
