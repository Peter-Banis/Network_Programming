/* ------------------------------------------------------------------------- */
/*   Copyright (C) 2018
 Author:  kcipi2015@myfit.edu, pbanis2015@my.fit.edu
 Florida Tech, Computer Science
 
 This program is free software; you can redistribute it and/or modify
 it under the terms of the GNU Affero General Public License as published by
 the Free Software Foundation; either the current version of the License, or
 (at your option) any later version.
 
 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.
 
 You should have received a copy of the GNU Affero General Public License
 along with this program; if not, write to the Free Software
 Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.              */
/* ------------------------------------------------------------------------- */

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
#include <syslog.h>
#include <arpa/inet.h>
#include <errno.h>
#include <signal.h>
#include <sys/wait.h>
#include <sys/select.h>
#include <errno.h>
#include <netdb.h>
#include <pthread.h>
#include <semaphore.h>

void* tcpConnection(void*);
int bufAppend(char*, char*, int, int);
void clearBuffer(char*, int, int);
int removeNewLines(char*);
int commandCount(char*);
int isValidForm(char * buf);
void udpConnection(int, struct sockaddr_in, char*);
int GOSSIP(char*, char*);
int isKnown(char*, char*, char);
void broadcastToPeersTCP(char*, int, char*);  //WORKS!!TESTED!!But nut used in the final version
void broadcastToPeersUDP(char*, int, char*);  //UDP used for broadcasting instead of TCP
int peerInfo(int, char*, char*);
int peerNumber(char*);
int PEER(char *, char *);
int updateFile(char*, int, char* ,char*);
int PEERS(int, struct sockaddr_in, char *, int);
int countDigit(int);
char* itoa(int, char*, int);
void error(char*);
void sig_chld(int);

char* filenamePath;
sem_t mutex_fpeers;                         //Semaphore for peers file
sem_t mutex_fgossip;                        //Semaphore for gossip file

int main(int argc, char **argv)
{
    unsigned clilen;
    int sockfd, newsockfd, portno, pid, c, udpfd, maxfdp1, ready;
    struct sockaddr_in serv_addr, cli_addr;
    ssize_t n;
    fd_set rset;
    char msg[1024];
    bzero(msg, 1024);
    
    while ((c = getopt (argc, argv, "p:d:")) != -1)        //Adding option -p -d
        switch (c)
        {
            case 'p':
                portno = atoi(optarg);                     //Get port number.
                break;
            case 'd':
                filenamePath = optarg;                     //Get filepath address.
                break;
            default:
            abort ();
        }
    //Initialize Semaphores
    sem_init(&mutex_fpeers, 0, 1);
    sem_init(&mutex_fgossip, 0, 1);
    
    //TCP listening socket
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
    
    //UDP listening socket
    udpfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) error("ERROR opening socket");
    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port = htons(portno);
    if (bind(udpfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0)
        error("ERROR on binding");
    
    //Establish a signal handler
    signal(SIGCHLD, sig_chld);
    
    FD_ZERO(&rset);
    maxfdp1 = ((sockfd > udpfd) ? (sockfd) : (udpfd)) + 1;
    
    while (1) {
        FD_SET(sockfd, &rset);
        FD_SET(udpfd, &rset);
        
        if ((ready = select(maxfdp1, &rset, NULL, NULL, NULL)) < 0) {
            if (errno == EINTR) continue;
            else error("ERROR on select");
        }
        //Handle TCP connections
        if (FD_ISSET(sockfd, &rset)) {
            
            newsockfd = accept(sockfd,
                               (struct sockaddr *) &cli_addr, &clilen);
            if (newsockfd < 0) error("ERROR on accept");
            
            pthread_t tcp;
            pthread_create(&tcp, NULL, tcpConnection, (void *) newsockfd);
        }
        //Handle UDP connections
        if (FD_ISSET(udpfd, &rset)) {
            udpConnection(udpfd, cli_addr, filenamePath);
        }
    }
    return 0;
}
/*
 * ISVALIDFORM test for faulty commands
 * INPUT:   a buffer to analyze for a valid command
 * OUTPUT: -1 if malformed or incomplete command
 *          1 if a valid GOSSIP command
 *          2 if a valid PEER command
 *          3 if a valid PEERS command
 */
int isValidForm(char * buf) {
//a command can be of three types, GOSSIP, PEER or PEERS?
//The form of GOSSIP is "GOSSIP:"[sha]:[time]:[message]%"
    if (buf[0] == 'G') {
        char cmpbuf[7];
        bzero(cmpbuf, 7);
        strncpy(cmpbuf, buf, 6);
        int res = strcmp("GOSSIP", cmpbuf);
        if (res != 0) return -1;
        if (buf[6] != ':') return -1;
        int i;
        int size = 0;
        for (i = 7; buf[i] != ':' && buf[i] != '\0' && buf[i] != '%'; i++) {
            size++;
        }
        if (buf[i] == '\0' || buf[i] == '%' || size != 44) return -1; //missing elements time:message%
        if (buf[i] != ':') return -1;
        //at this point we must examine the time stamp
        //timestamp has a length of 24 and 6 '-' characters
        int curIndex = ++i; //to make it easy to determine length
        int dashes = 0;
        size = 0;
        for (; buf[i] != ':' && buf[i] != '\0' && buf[i] != '%'; i++) {
             size++;
             if (buf[i] == '-') dashes++;
             if (dashes > 6) return -1;
        }
        if (buf[i] == '\0' || buf[i] == '%' || size != 24) {
            return -1;
        }
        //the message component can be any length, so we only need to confirm we reach a %
        for (; buf[i] != '%' && buf[i] != '\0'; i++) {

        }
        if (buf[i] == '\0') return -1;
        return 1; //valid GOSSIP string
//The form of PEER is "PEER:"[name]:PORT=[port]:IP=[ip]%"
//The form of PEERS is "PEERS?\n"
    } else if (buf[0] == 'P') {
         //first check for PEERS? since it's the easiest
         char buftemp[7];
         bzero(buftemp, 7);
         strncpy(buftemp, buf, 6);
         if (strcmp(buftemp, "PEERS?") == 0) return 3;
         //could still be a PEER command
         bzero(buftemp, 7);
         strncpy(buftemp, buf, 4);
         if (strcmp(buftemp, "PEER") != 0) return -1;
         if (buf[4] != ':') return -1;
         int i;
         for (i = 5; buf[i] != ':' && buf[i] != '%' && buf[i] != '\0'; i++) {
               //a name can be arbitrary, so we do nothing
         }
         if (i == 5) return -1; //missing name
         if (buf[i] == '\0' || buf[i] == '%') return -1; //missing [PORT], [IP]
         //now check for PORT=
         ++i;
         // confirm PORT=
         bzero(buftemp, 7);
         int limit = i+5;
         int offset = 0;
         for (; i < limit; i++, offset++) {
             if (buf[i] == '%' || buf[i] == '\0') return -1; //doesn't have PORT=
             buftemp[offset] = buf[i];
         }
         if (strcmp(buftemp, "PORT=") != 0) return -1;
         limit = i+5;//max port is 65535 = 5 digits
         int test = i;
         //valid port so long as it is all numbers before ':' and a maximum of five of them
         for (;i < limit; i++) {
             if (!isdigit(buf[i]) && buf[i] != ':') return -1; //failure
             if (buf[i] == ':') break; //because 1: 11: 111: 1111: 11111: are all valid options
         }
         if (i == test) return -1; //was PORT=: which is invalid
         //we know buf[i] is ':' and it is a valid port, so now we check the IP= and a valid IP
         bzero(buftemp, 7);
         i++;
         limit = i+3;
         offset = 0;
         for (; i < limit; i++, offset++) {
             if (buf[i] == '%' || buf[i] == '\0') return -1;
             buftemp[offset] = buf[i];
         }
         if (strcmp(buftemp, "IP=") != 0) return -1;
         //now have to confirm the [nnn.nnn.nnn.nnn pattern, where each section has a length of 1-3
         offset = 0;
         limit = 0; //limit will be reused as a count of which set we are at
         for (; buf[i] != '%' && buf[i] != '\0'; i++) {
             if (buf[i] == '.') {
                 if (offset == 0) return -1; // was ..
                 offset=0;
                 limit++;
             } else {
                 if (isdigit(buf[i])) offset++;
                 else return -1; //invalid character in address
                 if (limit > 3) return -1; // too many '.' in IP
                 if (offset > 3) return -1; //no IP has more than three digits per secgment
             }
         }
         if (buf[i] != '%' || limit != 3) return -1;
         return 2;
    }
    return -1;//malformed from start

}



/*
 * TCPCONNECTION handles commands that are send by a TCP client
 * INPUT: sock: socket; path: file directory path
 * OUTPUT: void
 */
void* tcpConnection (void *vargp){
    int sock = (int) vargp;
    struct sockaddr_in empty;
    int n, commands;
    char buffer[1024];          //Holds multiple commands (if needed i.e. concatination).
    bzero(buffer,1024);
    char bufTemp[256];          //Holds individual commands.
    bzero(bufTemp, 256);
    
    while (n = read(sock, bufTemp, 255)) {                    //Read from socket
        if (n == -1) { error("ERROR on reading sockets"); }
        int r = bufAppend(buffer, bufTemp, 1024, 256);        //Append bufTemp to buffer.
        commands = commandCount(buffer);                      //Count commands in buffer.
        
        while (commands > 0) {
            removeNewLines(buffer);
            /*
             * A command exists to execute if:
             * There exists a string such that it starts with:
             *  G and ends with %
             *  PEER: and ends with %
             *  PEERS and ends with ?
             */
            int index = 0;
            int t = isValidForm(buffer);
            if (t == 1) {                                  //GOSSIP command entry point.
                for (index = 0; index < 1024; index++) {
                    if (buffer[index] == '%') {
                        char gssp[index + 1];
                        strncpy(gssp, buffer, index + 1);  //Prepare GOSSIP command in gssp
                        gssp[index + 1] = '\0';
                        
                        GOSSIP(gssp, filenamePath);        //Handle GOSSIP command
                        
                        clearBuffer(buffer, index+1,1024); //Remove GOSSIP command from buffer.
                        break;
                    }
                }
            } else if (t == 2) {                           //PEER command entry point.
                for (index = 0; index < 1024; index++) {
                    if (buffer[index] == '%') {
                        char per[index + 1];
                        strncpy(per, buffer, index + 1);   //Prepare PEER command in per
                        per[index + 1] = '\0';
                        
                        PEER(per, filenamePath);           //Handle PEER command
                        
                        clearBuffer(buffer, index+1,1024); //Remove PEER command from buffer.
                        break;
                    }
                }
            } else if (t == 3) {                           //PEERS? command entry point.
                PEERS(sock, empty, filenamePath, 1);       //Handle PEERS? command
                clearBuffer(buffer, 6,1024);               //Remove PEERS? command from buffer.
            } else {                                       //Faulty command entry point.
                error("ERROR, command non found!3");
                clearBuffer(buffer, n,1024);
            }
            bzero(bufTemp, 256);                           //Clear bufTemp
            commands--;
        }
        bzero(bufTemp, 256);
    }
    close(sock);
}
/*
 * REMOVENEWLINES clears a buffer from newline characters '\n'
 * INPUT: str: a buffer;
 * OUTPUT: count: number of newline characters '\n'
 */
int removeNewLines(char * str) {
    int len = strlen(str);
    int count = 0;
    int i, shiftRest;
    
    for (i = 0; i < len; i++) {
        if (str[i] == '\n') {                                     //Found a '\n'
            for (shiftRest = i; shiftRest < len; shiftRest++) {   //Shift left to remove the '\n' found above.
                str[shiftRest] = str[shiftRest+1];
            }
            count++;
            len--;
            i--;
        }
    }
    str[len] = '\0';
    
    return count;
}
/*
 * BUFAPPEND appends a smaller buffer (bufTemp) into a larger one (buffer)
 * INPUT: dest: destination buffer; src: source buffer; destLen: length of destination buffer
 *        srcLen: length of source buffer
 * OUTPUT: 0 if successful append
 *        -1 if failure to append
 */
int bufAppend(char * dest, char * src, int destLen, int srcLen) {
    int destN = strlen(dest);
    int srcN = strlen(src);
    
    if (destN+srcN+1 > destLen) return -1;                      //Not enough space
    strcat(dest, src);
    
    return 0;
}
/*
 * CLEARBUFFER clears a number of characters (i.e. an entire command) from a buffer
 * INPUT: buffer: a buffer; howFar: number of spaces to clear from the buffer
 *        bufferSize: size of buffer
 * OUTPUT: void
 */
void clearBuffer(char * buffer, int howFar, int bufferSize) {
    int i = 0;
    for (i = howFar; i < bufferSize; i++) {
        buffer[i-howFar] = buffer[i];        //Shift the remainding commands to the beginning of buffer.
        buffer[i] = '\0';                    //Remove leftovers.
    }
}
/*
 * COMMANDCOUNT counts the number of commands in a buffer
 * INPUT: buf: a buffer
 * OUTPUT: count: number fo command in buf(buffer)
 */
int commandCount(char * buf) {
    int i, count = 0;
    for (i = 0; i < 1024; i++) {
        if (buf[i] == '%' || buf[i] == '?') {     //Has '?' or '%'? Should be a command.
            count++;
        }
    }
    return count;
}
/*
 * UDPCONNECTION handles commands that are send by a UDP client
 * INPUT: udpfd: socket; path: file directory path; cli_addr: client adress
 * OUTPUT: void
 */
void udpConnection(int udpfd, struct sockaddr_in cli_addr, char* path) {
    int n;
    unsigned len = sizeof(cli_addr);
    char msg[1024];
    bzero(msg, 1024);
    
    n = recvfrom(udpfd, msg, 1024, 0, (struct sockaddr *) &cli_addr, &len); //Recive command.
    removeNewLines(msg);
    int t = isValidForm(msg);
    if (t == 1) {
        GOSSIP(msg, path);
    } else if (t == 2) {
        PEER(msg, path);
    } else if (t == 3) {
        PEERS(udpfd, cli_addr, path, 0);
    } else {
        error("ERROR, command non found!");
    }
}
/*
 * GOSSIP handles GOSSIP command
 * INPUT: buf: a buffer; path: file directory path
 * OUTPUT: -1 if the message has already been received
 *          0 if the message was stored, broadcasted, and displayed
 */
int GOSSIP(char * buf, char * path) {
    char filePath[strlen(path) + 15];
    strcpy(filePath, path);
    strcat(filePath, "fgossip.txt");
    char filePathPeer[strlen(path) + 15];
    strcpy(filePathPeer, path);
    strcat(filePathPeer, "fpeers.txt");
    
    char sha[126];
    char time[64];
    char message[1024];
    bzero(message,1024);
    bzero(time,64);
    bzero(sha,126);
    int bindex = 0, index = 0, i;
    
    while (buf[bindex] != ':') { bindex++; }                         //skip GOSSIP:
    bindex++;
    while (buf[bindex] != ':') { sha[index++] = buf[bindex++]; }     //extract sha256 from gossip
    index = 0;
    bindex++;
    while (buf[bindex] != ':') { time[index++] = buf[bindex++]; }    //extract time from gossip
    index = 0;
    bindex++;
    while (buf[bindex] != '%') { message[index++] = buf[bindex++]; } //extract message from gossip

    if (isKnown(sha, filePath, '3')) {
        error("DISCARDED");
        return -1;
    } else {
        sem_wait(&mutex_fgossip);                                      //Semaphore waits
        FILE * fgossip;
        fgossip = fopen(filePath, "a");                                //Open file to write
        fprintf(fgossip, "BEGIN\n");                                   //Write header
        fprintf(fgossip, "1:%s\n",message);                            //Write message
        fprintf(fgossip, "2:%s\n",time);                               //Write timestamp
        fprintf(fgossip, "3:%s\n",sha);                                //Write sha
        fprintf(fgossip, "END\n");                                     //Write footer
        
        if (fclose(fgossip)) { error("File not closed properly"); };   //Close file
        sem_post(&mutex_fgossip);                                      //Semaphore signals

        int numberOFPeers = peerNumber(filePathPeer);
        for (i = 0; i < numberOFPeers; i++) {
            broadcastToPeersUDP(buf, i, filePathPeer);
        }

        error(message);                                                //Display message
        return 0;    
    }
}
/*
 * BROADCASTTOPEERSUDP sends gossips to all the peers
 * INPUT: buf: a buffer; index: peer position inside the file;
 *        path: file directory path;
 * OUTPUT: void
 */
void broadcastToPeersUDP(char* buf, int index, char* path) {
    struct sockaddr_in si_other;
    int s, i, slen = sizeof(si_other);
    int portno; char hostname[17];
    portno = peerInfo(index, hostname, path);           //Retrive PORT and IP of a peer
    
    //Establish UDP client
    if ( (s = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1) {
        error("ERROR, opening socket!"); return;
    }
    memset((char *) &si_other, 0, sizeof(si_other));
    si_other.sin_family = AF_INET;
    si_other.sin_port = htons(portno);
    
    if (inet_aton(hostname , &si_other.sin_addr) == 0) {
        close(s);
        error("ERROR, host non found!");
        return;
    }
    if (sendto(s, buf, strlen(buf) , 0 , (struct sockaddr *) &si_other, slen)==-1) {
        close(s);
        error("ERROR, sending gossip!");
        return;
    }
    
    close(s);                                //Close socket.
}
/*
 * BROADCASTTOPEERSTCP sends gossips to all the peers
 * INPUT: buf: a buffer; index: peer position inside the file;
 *        path: file directory path; cli_addr: client adress
 * OUTPUT: void
 */
void broadcastToPeersTCP(char * buf, int index, char * path) {
    int portno; char hostname[17];
    portno = peerInfo(index, hostname, path);           //Retrive PORT and IP of a peer
    int sockfd, n;
    struct sockaddr_in serveraddr;
    struct hostent *server;
    
    //Establish TCP client
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        error("ERROR oppening socket!"); return;
    }
    if ((server = gethostbyname(hostname)) == NULL) {
        close(sockfd);
        error("ERROR, host non found!"); return;
    }
    bzero((char *) &serveraddr, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
    bcopy((char *)server->h_addr, (char *)&serveraddr.sin_addr.s_addr, server->h_length);
    serveraddr.sin_port = htons(portno);
    if (connect(sockfd, (struct sockaddr *) &serveraddr, sizeof(serveraddr)) < 0) {
        close(sockfd);
        error("ERROR, server not available!"); return;
    }
    if (write(sockfd, buf, strlen(buf)) < 0) {   //Send gossip.
        close(sockfd);
        error("ERROR writing to PEER!"); return;
    }
    close(sockfd);                                //Close socket.
}
/*
 * ISKNOWN checks if a string is in a file
 * INPUT: obj: a buffer; filename: file directory path;
 * OUTPUT: lineNumber: line where the match was found
 *         0 if nothing matches
 *        -1 if any errors accured
 */
int isKnown(char* obj, char* filename, char match) {
    int lineNumber = 1;
    if (match == '1') {
        sem_wait(&mutex_fpeers);                           //Semaphore waits
    } else {
        sem_wait(&mutex_fgossip);                            //Semaphore waits
    }
    if (access(filename, F_OK) == -1) {
        if (match == '1') {
            sem_post(&mutex_fpeers);                             //Semaphore signals
        } else {
            sem_post(&mutex_fgossip);                            //Semaphore signals
        }
        return 0;                                           //Test if the file exists
    }
    FILE * fgossip;
    fgossip = fopen(filename, "r");                         //Open file
    
    char currC;                                             //Holds current char
    int skipFlag = 0, index = 0, objFlag = 0;
    while (fscanf(fgossip,"%c", &currC) == 1) {             //Is eof? Stop.
        if (!skipFlag) {
            if (!objFlag) {
                if (currC == match) {                       //Line starts with match char? It might be a match line.
                    objFlag = 1;
                    fscanf(fgossip,"%c", &currC);           //Skip ':'
                } else {
                    objFlag = 0;
                    skipFlag = 1;                           //Line does start with match char? Skip it.
                }
            } else {
                if (obj[index] == '\0' && currC == '\n') {  //End of string? Found it!
                    if (fclose(fgossip)) { error("ERROR, file not closed properly"); };
                    if (match == '1') {
                        sem_post(&mutex_fpeers);            //Semaphore signals
                    } else {
                        sem_post(&mutex_fgossip);           //Semaphore signals
                    }
                    return lineNumber;
                } else if (currC != obj[index]){            //Not a match
                    skipFlag = 1;
                    objFlag = 0;
                } else {                                    //Match. Check the next char
                    index++;
                }
            }
        } else {                                            //Skip the entire line
            if (currC == '\n'){
                index = 0;
                skipFlag = 0;
                lineNumber++;
            }
        }
    }
    if (fclose(fgossip)) { error("ERROR, file not closed properly"); };   //Close file.
    if (match == '1') {
        sem_post(&mutex_fpeers);                             //Semaphore signals
    } else {
        sem_post(&mutex_fgossip);                            //Semaphore signals
    }
    return 0;
}
/*
 * PEERNUMBER finds the number of peers.
 * INPUT: path: file directory path;
 * OUTPUT: peers: number of peers
 *        -1 if any errors accured
 */
int peerNumber(char * path) {
    char currC;
    int lineNumber = 0;
    
    sem_wait(&mutex_fpeers);                                             //Semaphore waits
    if (access(path, F_OK) == -1) {
        sem_post(&mutex_fpeers);                                         //Semaphore signals
        return -1;
    }
    FILE * finfo;
    finfo = fopen(path, "r");
    
    while (fscanf(finfo,"%c", &currC) == 1) {
        if (currC == '\n') lineNumber++;                                 //Count number of line in file.
    }
    int peers = lineNumber/5;                                           //Every peer is written in 5 lines.
    if (fclose(finfo)) { error("ERROR, file not closed properly"); };   //Close file.
    sem_post(&mutex_fpeers);                                            //Semaphore signals
    return peers;
}
/*
 * PEERINFO finds the peer's PORT and IP.
 * INPUT: path: file directory path; peerIndex: position of peer in the file.
 *        destination: IP buffer
 * OUTPUT: port: port number; destination: IP string
 *        -1 if any errors accured
 */
int peerInfo(int peerIndex, char * destination, char * path) {
    bzero(destination, 17);
    char portChar[6];
    bzero(portChar, 6);
    int port, index = 0;
    char currC;
    
    sem_wait(&mutex_fpeers);                                   //Semaphore waits
    if (access(path, F_OK) == -1) {
        sem_post(&mutex_fpeers);                                            //Semaphore signals
        return -1;
    }
    FILE * finfo;
    finfo = fopen(path, "r");
    
    int line = (peerIndex * 5) + 3;
    
    while (fscanf(finfo,"%c", &currC) == 1) {           //read until eof of the old file
        if (currC == '\n') { line--;}                   //count the number of lines
        
        if (line == 1) {                                //found the port line
            fscanf(finfo,"%c", &currC);                 //skiping '\n'
            if (currC == '2') {
                fscanf(finfo,"%c", &currC);             //skip :
                line--;
                while (1) {
                    fscanf(finfo,"%c", &currC);
                    if (currC == '\n'){ break; }
                    portChar[index++] = currC;
                }
            } else {
                return -1;
            }
        } else if (line == 0) {                          //found ip line
            if (currC == '3') {
                index = 0;
                fscanf(finfo,"%c", &currC);             //skip :
                line--;
                while (1) {
                    fscanf(finfo,"%c", &currC);
                    if (currC == '\n'){ break; }
                    destination[index++] = currC;
                }
            } else {
                sem_post(&mutex_fpeers);                               //Semaphore signals
                return -1;
            }
        }
    }
    port = atoi(portChar);
    
    if (fclose(finfo)) { error("ERROR, file not closed properly"); };  //close file
    sem_post(&mutex_fpeers);                                           //Semaphore signals
    return port;
}
/*
* PEER adds or updates peers in the peer file.
* INPUT: path: file directory path; buf: a buffer
* OUTPUT: 0 if adding or updating was succesful
*        -1 if any errors accured
*/
int PEER(char * buf, char * path) {
    char name[200];
    char port[6];
    char ip[17];
    
    bzero(name,200);
    bzero(port,6);
    bzero(ip,17);
    
    char filePath[strlen(path) + 15];
    strcpy(filePath, path);
    strcat(filePath, "fpeers.txt");
    char filePathTemp[strlen(path) + 15];
    strcpy(filePathTemp, path);
    strcat(filePathTemp, "output.txt");
    
    int index = 0;
    while (buf[index] != ':') { index++;}                        //skip PEER
    int offset = 0;
    index++;
    while (buf[index] != ':') { name[offset++] = buf[index++];}  //extract name from peer
    offset = 0;
    index += 6;
    while (buf[index] != ':') { port[offset++] = buf[index++];}  //extract port from peer
    offset = 0;
    index += 4;
    while (buf[index] != '%') { ip[offset++] = buf[index++]; }  //extract ip from gossip
    
    int lineToUpdate = isKnown(name, filePath, '1');            //Does peer exist in the file?
    if (lineToUpdate) {
        if (updateFile(ip, lineToUpdate, filePath, filePathTemp) == -1) { return -1; }  //Yes? Update it.
    } else {                                                                            //No? Add it.
        sem_wait(&mutex_fpeers);                                       //Semaphore waits
        FILE * fpeers;
        fpeers = fopen(filePath,"a");                           //Open file.
        fprintf(fpeers, "BEGIN\n");                             //Write peer fields to file.
        fprintf(fpeers, "1:%s\n", name);                        //...
        fprintf(fpeers, "2:%s\n", port);                        //...
        fprintf(fpeers, "3:%s\n", ip);                          //...
        fprintf(fpeers, "END\n");                               //...
        
        if (fclose(fpeers)) {
            error("ERROR, file not closed properly");
            sem_post(&mutex_fpeers);
            return -1;
        }
        sem_post(&mutex_fpeers);                                       //Semaphore signals
    }
    return 0;
}
/*
 * UPDATEFILE updates peers content in the peer file.
 * INPUT: tempPath, peersPath: file directory path; line: line number where peer to be updated is.
 *        ip: new IP string
 * OUTPUT: 1 if adding or updating was succesful
 *        -1 if any errors accured
 */
int updateFile(char* ip, int line, char * peersPath, char * tempPath) {
    int deleteLine = line + 2;              //address line is 2 lines after the name line
    int index = 0;
    char currC;
    
    sem_wait(&mutex_fpeers);                                   //Semaphore waits
    FILE * ffold;
    ffold = fopen(peersPath, "r");
    FILE * fupdated;
    fupdated = fopen(tempPath, "w");
    
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
            fprintf(fupdated, "%c", currC);              //copy unchanged lines
        }
    }
    
    if (fclose(ffold)) { error("ERROR, file not closed properly"); sem_post(&mutex_fpeers); return -1; }
    remove(peersPath);                                  //remove the old file
    if (fclose(fupdated)) { error("ERROR, file not closed properly"); sem_post(&mutex_fpeers); return -1; }
    rename(tempPath, peersPath);                        //rename the new file
    sem_post(&mutex_fpeers);                                   //Semaphore signals
    return 1;
}
/*
 * PEERS sends a string to client that contains all peers in the server.
 * INPUT: path: file directory path; sockfd: socket; cli_addr: client address
 *        tcpFlag: 1 if connection is over TCP
 *                 0 if connection is over UDP
 * OUTPUT: 1 if writing was succesful
 *        -1 if any errors accured
 */
int PEERS(int sockfd, struct sockaddr_in cli_addr, char * path, int tcpFlag) {
    char filePath[strlen(path) + 15];
    strcpy(filePath, path);
    strcat(filePath, "fpeers.txt");
    
    unsigned len = sizeof(cli_addr);
    char currC;
    char* noPeers = "PEERS|0|%\n";
    int charNumber = 0, lineNumber = 0;
    
    sem_wait(&mutex_fpeers);                                       //Semaphore waits
    if (access(filePath, F_OK) == -1) {              //test if the file exists
        if (tcpFlag) {
            write(sockfd, noPeers, strlen(noPeers)); //sending message over TCP
        } else {
            sendto(sockfd, noPeers, strlen(noPeers), 0, (struct sockaddr *) &cli_addr, len); //UDP
        }
        sem_post(&mutex_fpeers);
        return -1;
    }
    FILE * fpeers;
    fpeers = fopen(filePath, "r");
    
    while (fscanf(fpeers,"%c", &currC) == 1) {
        if (currC != '\n') { charNumber++; }          //Find total number of chars in the file.
        else { lineNumber++; }                        //Find number of lines in the file.
    }
    
    if (fclose(fpeers)) { error("ERROR, file not closed properly"); sem_post(&mutex_fpeers); return -1; }
    sem_post(&mutex_fpeers);                                 //Semaphore signals
    
    int peersNumber = lineNumber/5;                   //Calculate number of peers.
    int totalChar = charNumber + 8 - (peersNumber * 14) + (peersNumber * 11) + countDigit(peersNumber);

    char message[totalChar + 2];                      //Creating char array for the message
    bzero(message,totalChar + 2);
    
    int messageIndex = 0;
    
    char * intro = "PEERS|";
    int i;   
    for (i = 0; i < strlen(intro); i++) {             //Adding PEERS| to the message
        message[messageIndex++] = intro[i];
    }
    
    char peerNoArray[countDigit(peersNumber)];        //Adding number of peers
    itoa(peersNumber,peerNoArray,10);
    
    for (i = 0; i < strlen(peerNoArray); i++) {
        message[messageIndex++] = peerNoArray[i];
    }
    message[messageIndex++] = '|';
    
    sem_wait(&mutex_fpeers);                                       //Semaphore waits
    fpeers = fopen(filePath, "r");
    char * port = ":PORT=";
    char * ip = ":IP=";
    
    while (fscanf(fpeers,"%c", &currC) == 1) {            //Adding peers to message
        if (currC == 'B' || currC == 'E') {
            while (1) {                                   //Skip BEGIN and END lines
                fscanf(fpeers,"%c", &currC);
                if (currC == '\n') {
                    break;
                }
            }
        } else {
            if (currC == '1') {                           //Distinuishing name line
                fscanf(fpeers,"%c", &currC);              //Skip :
                while (1) {
                    fscanf(fpeers,"%c", &currC);
                    if (currC == '\n') {
                        break;
                    } else {
                        message[messageIndex++] = currC;  //Adding name to message
                    }
                }
                int i;
                for (i = 0; i < strlen(port); i++) {      //Adding :PORT=
                    message[messageIndex++] = port[i];
                }
            } else if (currC == '2') {                    //Distinuishing port line
                fscanf(fpeers,"%c", &currC);              //Skip :
                while (1) {
                    fscanf(fpeers,"%c", &currC);
                    if (currC == '\n') {
                        break;
                    } else {
                        message[messageIndex++] = currC;  //Adding port to message
                    }
                }
                int i;
                for (i = 0; i < strlen(ip); i++) {        //Adding :IP=
                    message[messageIndex++] = ip[i];
                }
            } else {                                      //Distinuishing IP line
                fscanf(fpeers,"%c", &currC);              //Skip :
                while (1) {
                    fscanf(fpeers,"%c", &currC);
                    if (currC == '\n') {
                        break;
                    } else {
                        message[messageIndex++] = currC;  //Adding IP to message
                    }
                }
                message[messageIndex++] = '|';
            }
        }
    }
    message[messageIndex++] = '%';                        //Adding % to the end of message
    message[messageIndex] = '\n';
    
    if (fclose(fpeers)) { error("ERROR, file not closed properly"); sem_post(&mutex_fpeers); return -1; }
    sem_post(&mutex_fpeers);                                 //Semaphore signals
    
    if (tcpFlag) {
        write(sockfd, message, strlen(message));          //Sending message over TCP
    } else {
        sendto(sockfd, message, strlen(message), 0, (struct sockaddr *) &cli_addr, len); //UDP
    }

    return 1;
}
/*
 * COUNTDIGIT counts the number of digits in an integer.
 * INPUT: n: a number
 * OUTPUT: count: number of digits in an integer
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
 * ITOA converts integers to strings
 * INPUT: value: integer; result: string containing the integer; base: integer's base (i.e. 10, 16...)
 * OUTPUT: result: string containing the integer
 */
char* itoa(int value, char* result, int base) {
    if (base < 2 || base > 36) { *result = '\0'; return result; }
    
    char* ptr = result, *ptr1 = result, tmp_char;
    int tmp_value;
    
    do {
        tmp_value = value;
        value /= base;
        *ptr++ = "zyxwvutsrqponmlkjihgfedcba9876543210123456789abcdefghijklmnopqrstuvwxyz" [35 + (tmp_value - value * base)];
    } while ( value );
    
    if (tmp_value < 0) *ptr++ = '-';                 // Apply negative sign
    *ptr-- = '\0';
    while(ptr1 < ptr) {
        tmp_char = *ptr;
        *ptr--= *ptr1;
        *ptr1++ = tmp_char;
    }
    return result;
}
/*
 * ERROR prints the message to error stream
 * INPUT: msg: string
 * OUTPUT: void
 */
void error(char *msg) {
    fprintf(stderr, "%s\n", msg);
}
/*
 * SIG_CHLD handles signals send by child processes
 * INPUT: sig: signal value
 * OUTPUT: void
 */
void sig_chld(int sig) {
    pid_t pid;
    int store;
    pid = waitpid(sig, &store, 0);
}
