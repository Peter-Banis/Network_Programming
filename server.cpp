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
#include <sys/time.h>
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
#include <time.h>
#include <openssl/sha.h>
#include <openssl/bio.h>
#include <openssl/evp.h>
#include <openssl/buffer.h>
#include <math.h>
#include "BigInteger.h"
#include "ASN1_Util.h"
#include "ASN1Decoder.h"
#include "ASN1Encoder.h"
#include "PeerAnswer.h"
#include "PeersQuery.h"
#include "Gossip.h"
#include "Leave.h"

void* serverThread(void*);
void* clientThread(void*);
void clientLEAVE(char*);
void clientPEER(char*);
void clientPEERS();
void clientGOSSIP(char*);
void constructPeers(PeerAnswer);
void produceHash(char*, char**);
void sendMessage(unsigned char*, int);
void* tcpConnection(void*);
int bufAppendByte(unsigned char*, unsigned char*, int, int, int, int);
void clearBuffer(unsigned char*, int, int);
int removeNewLines(char*);
int commandCount(char*);
int isValidForm(char * buf);
void udpConnection(int, struct sockaddr_in, char*);
int GOSSIP(char*, char*, unsigned char*, int);
int isKnown(char*, char*, char);
void broadcastToPeersTCP(char*, int, char*);  //WORKS!!TESTED!!But nut used in the final version
void broadcastToPeersUDP(unsigned char*, int, char*, int);  //UDP used for broadcasting instead of TCP
int peerInfo(int, char*, char*);
int peerNumber(char*);
int LEAVE(char*, char*);
int removeEntries(char* , char* , int, int* , int);
int PEER(char *, char *);
int updateFile(char*, int, char* ,char*, int);
int PEERS(int, struct sockaddr_in, char *, int);
char* copyPEER(char*);
int countDigit(int);
char* itoa(int, char*, int);
void error(const char*);
void sig_chld(int);
void base64Encode(unsigned char *, int len, char **);
int addPeerLeave(char *, char *);

struct holder {
    char ip[INET_ADDRSTRLEN];
    int sockfd, port;
};


//GETOPT
char *filenamePath, *initMessage, *initTimestamp, *serverIP;
int clientTCP = 1, forgetPeer = 172800;  //2 days
//TCP
int socktcp, n;
struct sockaddr_in serveraddr;
struct hostent *server;
//UDP
struct sockaddr_in si_other;
int sockudp, slen = sizeof(si_other);

sem_t mutex_fpeers;                         //Semaphore for peers file
sem_t mutex_fgossip;                        //Semaphore for gossip file
sem_t sem_client;

int main(int argc, char **argv)
{
    long portno = -1, clientport = -1;
    int c;
    while ((c = getopt (argc, argv, "p:d:c:s:m:t:TUD:")) != -1)
        switch (c)
        {
            case 'p':
                portno = atoi(optarg);
                break;
            case 'd':
                filenamePath = optarg;
                break;
            case 'c':
                clientport = atoi(optarg);
                break;
            case 's':
                serverIP = optarg;
                break;
            case 'm':
                initMessage = optarg;
                break;
            case 't':
                initTimestamp = optarg;
                break;
            case 'T':
                clientTCP = 1;
                break;
            case 'U':
                clientTCP = 0;
                break;
            case 'D':
                forgetPeer = atoi(optarg);
                break;
            default:
                error("Error on getopt");
        }
    
    sem_init(&sem_client, 0, 0);
    
    pthread_t server, client;
    pthread_create(&server, NULL, serverThread, (void *) portno);
    sem_wait(&sem_client);
    pthread_create(&client, NULL, clientThread, (void *) clientport);
    
    pthread_exit(NULL);
}
/*
 * CLIENT handles client
 * INPUT: args: client argument
 * OUTPUT: void
 */
void* clientThread(void* args) {
    long portno = (long) args;
    
    char userInput[1024];
    bzero(userInput, 1024);
    
    if (portno == -1 || serverIP == NULL) {
        error("ERROR: Please provide IP and PORT for client");
        return NULL;
    }
    
    if (clientTCP) {
        //Establish TCP client
        if ((socktcp = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
            error("ERROR oppening socket!"); return NULL;
        }
        if ((server = gethostbyname(serverIP)) == NULL) {
            close(socktcp);
            error("ERROR, host non found!"); return NULL;
        }
        bzero((char *) &serveraddr, sizeof(serveraddr));
        serveraddr.sin_family = AF_INET;
        bcopy((char *)server->h_addr, (char *)&serveraddr.sin_addr.s_addr, server->h_length);
        serveraddr.sin_port = htons(portno);
        if (connect(socktcp, (struct sockaddr *) &serveraddr, sizeof(serveraddr)) < 0) {
            close(socktcp);
            error("ERROR, server not available!"); return NULL;
        }
        
    } else {
        //Establish UDP client
        if ( (sockudp = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1) {
            error("ERROR, opening socket!"); return NULL;
        }
        memset((char *) &si_other, 0, sizeof(si_other));
        si_other.sin_family = AF_INET;
        si_other.sin_port = htons(portno);
        
        if (inet_aton(serverIP, &si_other.sin_addr) == 0) {
            close(sockudp);
            error("ERROR, host non found!");
            return NULL;
        }
    }
    
    if (initMessage != NULL) {
        clientGOSSIP(initMessage);
    }
    
    while(1) {
        fgets(userInput,1024,stdin);
        removeNewLines(userInput);
        printf("User Input: %s\n", userInput);
        if (!strcmp(userInput, "quit")) exit(0);
        
        if (!strncmp(userInput, "PEERS?", 6) && strlen(userInput) == 6) {
            clientPEERS();
        } else if (!strncmp(userInput, "PEER", 4)){
            printf("User Input in PEER: %s\n", userInput);
            clientPEER(userInput);
        } else if (!strncmp(userInput, "LEAVE:", 6)) {
            clientLEAVE(userInput);
        } else {
            clientGOSSIP(userInput);
        }
        
        bzero(userInput, 1024);
    }
    
    if (clientTCP) {
        close(socktcp);
    } else {
        close(sockudp);
    }
}

void clientLEAVE(char* buf) {
    //Testing if the LEAVE command is valid!
    for (int i = 0; i < 1024; i++) {
        if (buf[i] == '%') {
            break;
        }
        if (i == 1023) {
            error("ERROR, command non found!");
            return;
        }
    }
    
    char lname[256];
    bzero(lname, 256);
    
    int index = 0;
    while (buf[index + 6] != '%') {
        lname[index] = buf[index + 6];
        index++;
    }
    
    Leave* l = new Leave();
    l->name = lname;
    
    ASN1_Encoder* enc = l->getEncoder();
    byte* msg = enc->getBytes();
    
    sendMessage(msg, enc->getBytesNb());
}

void clientPEERS() {
    PeersQuery* m = new PeersQuery();
    
    ASN1_Encoder* enc = m->getEncoder();
    byte* asn1 = enc->getBytes();
    
    sendMessage(asn1, enc->getBytesNb());
    
    unsigned char msg[2048];
    bzero(msg, 2048);
    
    if (clientTCP) {
        unsigned char bufTemp[1024];                  //Holds individual commands.
        bzero(bufTemp, 1024);
        int n, count = 0;
        while (n = read(socktcp, bufTemp, 1024)) {                    //Read from socket
            bufAppendByte(msg, bufTemp, count, n, 2048, 1024);
            count += n;
            try {
                
                ASN1_Decoder* d = new ASN1_Decoder(msg, n);
                PeerAnswer pa;
                pa.decode(d);
                constructPeers(pa);
                
                break;
            } catch (const std::exception& e) {
                continue;
            }
        }
    } else {
        unsigned int len = sizeof(si_other);
        int r = recvfrom(sockudp, msg, 1024, 0, (struct sockaddr *) &si_other, &len); //Recive command.
        
        ASN1_Decoder* d = new ASN1_Decoder(msg, r);
        PeerAnswer n;
        n.decode(d);
        
        constructPeers(n);
    }
    
}
void constructPeers(PeerAnswer n) {
    char buffer[2048];
    bzero(buffer, 2048);
    
    memcpy(buffer, "PEERS|", 6);
    int index = 6;
    char numberOFPeers[countDigit(n.n_rcv)];
    itoa(n.n_rcv, numberOFPeers, 10);
    memcpy(buffer + index, numberOFPeers, countDigit(n.n_rcv));
    index += countDigit(n.n_rcv);
    memcpy(buffer + index, "|", 1);
    index += 1;
    
    for (int i = 0; i < n.n_rcv ; i++ ) {
        
        memcpy(buffer + index, n.rcv[i]->name, strlen(n.rcv[i]->name));
        index += strlen(n.rcv[i]->name);
        memcpy(buffer + index, ":PORT=", 6);
        index += 6;
        int charsInPort = countDigit(n.rcv[i]->port);
        char portNumber[charsInPort];
        itoa(n.rcv[i]->port, portNumber, 10);
        memcpy(buffer + index, portNumber, charsInPort);
        index += charsInPort;
        memcpy(buffer + index, ":IP=", 4);
        index += 4;
        memcpy(buffer + index, n.rcv[i]->ip, strlen(n.rcv[i]->ip));
        index += strlen(n.rcv[i]->ip);
        memcpy(buffer + index, "|", 1);
        index += 1;
    
    }
    
    buffer[index] = '%';
    
    error(buffer);
}
void clientPEER(char* buf) {
    Peer* m = new Peer();
    
    if (isValidForm(buf) != 2) {
        const char* errorCommand = "error";
        
        m->name = errorCommand;
        m->port = 0;
        m->ip = errorCommand;
        
        ASN1_Encoder* enc = m->getEncoder();
        byte* msg = enc->getBytes();
        
        sendMessage(msg, enc->getBytesNb());
        return;
    }
    
    char name[200];
    char port[6];
    char ip[17];
    
    bzero(name,200);
    bzero(port,6);
    bzero(ip,17);
    
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
    
    m->name = name;
    m->port = atoi(port);
    m->ip = ip;
    
    ASN1_Encoder* enc = m->getEncoder();
    byte* msg = enc->getBytes();
    
    sendMessage(msg, enc->getBytesNb());
}
void clientGOSSIP(char* buf) {

    char * hash;
    produceHash(buf, &hash);
    
    Gossip * g = new Gossip();
    g->message = buf;
    g->sha256hash = (unsigned char*) hash;
    
    ASN1_Encoder* enc = g->getEncoder();
    byte* msg = enc->getBytes();
    
    sendMessage(msg, enc->getBytesNb());
    
}
void base64encode(unsigned char * shahash, int len, char ** encoded) {
    BIO * bio, * b64;
    BUF_MEM * buf;
    b64 = BIO_new(BIO_f_base64());
    bio = BIO_new(BIO_s_mem());
    bio = BIO_push(b64, bio);

    BIO_set_flags(bio, BIO_FLAGS_BASE64_NO_NL);
    BIO_write(bio, shahash, len);
    BIO_flush(bio);
    BIO_get_mem_ptr(bio, &buf);
    BIO_set_close(bio, BIO_NOCLOSE);
    BIO_free_all(bio);
    *encoded=(*buf).data;
}
void produceHash(char * message, char ** hash) {
    timeval curTime;
    gettimeofday(&curTime, NULL);
    int milli = curTime.tv_usec / 1000;
    
    char timeArray[24];
    bzero(timeArray, 24);
    time_t rawtime;
    struct tm* timeInfo;
    
    time(&rawtime);
    timeInfo = gmtime(&rawtime);
    strftime(timeArray, 24, "%Y-%m-%d-%H-%M-%S-000Z", timeInfo);
    
    char milisec[3];
    bzero(milisec, 3);
    itoa(milli, milisec, 10);
    int numberofDigits = countDigit(milli);
    
    if (numberofDigits == 3) {
        for (int i = 0; i < numberofDigits; i++) {
            timeArray[i + 20] = milisec[i];
        }
        timeArray[23] = 'Z';
    } else if (numberofDigits == 2) {
        for (int i = 0; i < numberofDigits; i++) {
            timeArray[i + 21] = milisec[i];
        }
        timeArray[23] = 'Z';
    } else {
        timeArray[22] = milisec[0];
        timeArray[23] = 'Z';
    }
    
    int preHashLength = 25 + strlen(message);
    char preHash[preHashLength];               //contains the pre hash
    bzero(preHash, preHashLength);
    int index = 0;
    memcpy(preHash, timeArray, 24);
    index += 24;
    memcpy(preHash + index, ":", 1);
    index += 1;
    memcpy(preHash + index, message, strlen(message));
    
    //compute hash for PREHASH message
    unsigned char * shahash = SHA256((unsigned char *)preHash, preHashLength, 0);
    //base64 shahash
    base64encode(shahash, SHA256_DIGEST_LENGTH, hash);
    //pass it by reference to hash byte array
    hash[44] = 0;
}

void sendMessage(unsigned char* buf, int bufLen) {
    if (clientTCP) {
        if (write(socktcp, buf, bufLen) < 0) {
            close(socktcp);
            error("ERROR sending message!"); return;
        }
    } else {
        if (sendto(sockudp, buf, bufLen , 0 , (struct sockaddr *) &si_other, slen) == -1) {
            close(sockudp);
            error("ERROR sending message!"); return;
        }
    }
}
/*
 * SERVERTHREAD handles server
 * INPUT: args: server argument
 * OUTPUT: void
 */
void* serverThread(void* args) {
    long portno = (long) args;
    
    if (portno == -1 || filenamePath == NULL) {
        error("ERROR: Please provide FILE DIRECTORY and PORT for server");
        return NULL;
    }
    
    unsigned clilen;
    long newsockfd;
    int sockfd, pid, udpfd, maxfdp1, ready;
    struct sockaddr_in serv_addr, cli_addr;
    ssize_t n;
    fd_set rset;
    char msg[1024];
    bzero(msg, 1024);
    
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
    
    sem_post(&sem_client);
    
    //Establish a signal handler
    signal(SIGCHLD, sig_chld);
    
    FD_ZERO(&rset);
    maxfdp1 = ((sockfd > udpfd) ? (sockfd) : (udpfd)) + 1;
    
    //daemon(1, 1);                                //Deamon line commented out for testing
    
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
            
            //TCP connection timeout
            struct timeval timeout;
            timeout.tv_sec = 20;
            timeout.tv_usec = 0;
            if (setsockopt (newsockfd, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout, sizeof(timeout)) < 0)
            error("setsockopt failed\n");
            if (setsockopt (newsockfd, SOL_SOCKET, SO_SNDTIMEO, (char *)&timeout, sizeof(timeout)) < 0)
            error("setsockopt failed\n");
            
            struct holder hold;
            hold.port = ntohs(cli_addr.sin_port);
            inet_ntop(AF_INET, &cli_addr.sin_addr, hold.ip, 30);
            hold.sockfd = newsockfd;
            
            printf("[DEBUG] HOLD CONTAINS %d FOR SOCKFD\n", hold.sockfd);
            printf("[DEBUG] CONNECTOR HAS PORT %d\n", hold.port);
            printf("[DEBUG] CONNECTOR HAS IP %d\n", hold.ip);

            pthread_t tcp;
            pthread_create(&tcp, NULL, tcpConnection, (void *) &hold);
        }
        //Handle UDP connections
        if (FD_ISSET(udpfd, &rset)) {
            udpConnection(udpfd, cli_addr, filenamePath);
        }
    }
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
//adding support for LEAVE command
    if (!strncmp(buf, "LEAVE:", 6)) {
        return 4;
    }
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

int checkIfExipired(char* path) {
    char filePath[strlen(path) + 30];
    strcpy(filePath, path);
    strcat(filePath, "ftimeout.txt");
    char filePeerPath[strlen(path) + 30];
    strcpy(filePeerPath, path);
    strcat(filePeerPath, "fpeers.txt");
    char fileTempPath[strlen(path) + 30];
    strcpy(fileTempPath, path);
    strcat(fileTempPath, "output.txt");
    
    if (access(filePath, F_OK) == -1) {
        return 0;
    }
    if (access(filePeerPath, F_OK) == -1) {
        return 0;
    }
    
    int writeFlag = 0;

    int numberOFPeers = peerNumber(filePeerPath);
    int peerPosition[numberOFPeers];
    
    for (int i = 0; i < numberOFPeers; i++) {
        peerPosition[i] = 0;
    }
    
    int currentTime = time(NULL);
    int numberofDigitsT = countDigit(currentTime);
    char currT[numberofDigitsT];
    bzero(currT, numberofDigitsT);
    itoa(currentTime, currT, 10);
    
    FILE * fout;
    fout = fopen(filePath, "r");
    char currC;
    int index = 0;
    
    while (fscanf(fout,"%c", &currC) == 1) {
        if (currC == '3') {
            fscanf(fout,"%c", &currC);
            for (int i = 0; i < numberofDigitsT; i++) {
                fscanf(fout,"%c", &currC);
                if (currC < currT[i]) {
                    peerPosition[index] = 1;
                    writeFlag++;
                    break;
                } else if (currC > currT[i]) {
                    peerPosition[index] = 0;
                    break;
                }
            }
            while(1) {
                fscanf(fout,"%c", &currC);
                if (currC == '\n') break;
            }
            index++;
        } else {
            while (1) {
                fscanf(fout,"%c", &currC);
                if (currC == '\n') break;
            }
        }
    }
    
    if (writeFlag) {
        removeEntries(filePeerPath, fileTempPath, 5, peerPosition, numberOFPeers);
        removeEntries(filePath, fileTempPath, 4, peerPosition, numberOFPeers);
    }
    
    return 0;
}

int removeEntries(char* fileName, char* tempFile, int sizeOfEntry, int* peerPos, int len) {
    int deleteLines = sizeOfEntry;
    int indexOfPeers = 0;
    char currC;
    
    FILE * ffold;
    ffold = fopen(fileName, "r");
    FILE * fupdated;
    fupdated = fopen(tempFile, "w");
    
    while (fscanf(ffold,"%c", &currC) == 1) {
        if (peerPos[indexOfPeers] == 1) {
            for (int i = 0; i < sizeOfEntry; i++) {
                while (1) {
                    if (currC == '\n') break;
                    fscanf(ffold,"%c", &currC);
                }
                if (i != sizeOfEntry - 1) {
                    fscanf(ffold,"%c", &currC);
                }
            }
        } else if (peerPos[indexOfPeers] == 0) {
            while (1) {
                fprintf(fupdated, "%c", currC);
                if (currC == '\n') {
                    deleteLines--;
                }
                if (deleteLines == 0) {
                    break;
                }
                fscanf(ffold,"%c", &currC);
            }
            deleteLines = sizeOfEntry;
        } else {
            fprintf(fupdated, "%c", currC);
            fscanf(ffold,"%c", &currC);
        }
        
        indexOfPeers++;
        
        if (indexOfPeers == len) {
            break;
        }
    }
    
    if (fclose(ffold)) {
        error("ERROR, file not closed properly");
        return -1;
    }
    remove(fileName);                                  //remove the old file
    if (fclose(fupdated)) {
        error("ERROR, file not closed properly");
        return -1;
    }
    rename(tempFile, fileName);
    
    return 0;
}

int addPeerLeave(char * portAndIP, char * path) {
    char filePath[strlen(path) + 30];
    strcpy(filePath, path);
    strcat(filePath, "ftimeout.txt");
    char filePathTemp[35];
    bzero(filePathTemp, 35);
    strcat(filePathTemp, filePath);
    strcat(filePathTemp, ".swp");
    
    for (int i = 0; i < strlen(portAndIP); i++) {
        if (portAndIP[i] == ':') {
            portAndIP[i] = '|';
        }
    }
    
   //due to how isKnown posts for semaphores, we require a unique value.
   //for this case, we use 4 as the search line inidactor
   int t = isKnown(portAndIP, filePath, '4');
   if (t) {
        //update
        error("Known port and IP for: ");
        error(portAndIP);
        char timestamp[30];
        sprintf(timestamp, "%d", time(NULL) + forgetPeer);
        error("Giving timestamp:");
        error(timestamp);
        error("");
        //updateFile(char* ip, int line, char * peersPath, char * tempPath) 
        updateFile(timestamp, t-1, filePath, filePathTemp, 3);
        return 2;
    }
    
    FILE * fp = fopen(filePath, "a");
    fprintf(fp, "BEGIN\n");
    fprintf(fp, "4:%s\n", portAndIP);
    fprintf(fp, "3:%d\n", time(NULL) + forgetPeer);
    fprintf(fp, "END\n");
    
    if (fclose(fp)) {
        error("ERROR, file not closed properly");
        return -1;
    }
    
    return 0;
}

int contentLength(byte * buf) {
    if (buf[0] == 0x63) {                    //PEERS?
        return 1;
    } else {
        byte len = buf[1];
        if (len < 0x80) {                    //Short definite form of GOSSIP or PEER
            return (buf[1] + 2);
        } else {                             //Long definte form of GOSSIP or PEER
            int kLength = buf[1];
            kLength = kLength & 0b01111111;
            
            int numberofDigitsInLength = 0;
            for (int i = 0; i < kLength; i++) {
                numberofDigitsInLength += countDigit(buf[i + 2]);
            }
            
            char length[numberofDigitsInLength];
            char* lengthPtr = length;
            bzero(length, numberofDigitsInLength);
            
            int index = 0;
            for (int i = 0; i < kLength; i++) {
                int digits = countDigit(buf[i + 2]);
                itoa(buf[i + 2], lengthPtr + index, 16);
                index += digits;
            }
            int elementLength = 0;
            for (int i = 0; i < numberofDigitsInLength; i++) {
                int singleDigits = length[i] & 0b00001111;
                elementLength += singleDigits * pow(16, i);
            }
            
            return (elementLength + 1 + 1 + kLength);
        }
    }
}
/*
 * TCPCONNECTION handles commands that are send by a TCP client
 * INPUT: sock: socket; path: file directory path
 * OUTPUT: void
 */
void* tcpConnection (void *vargp){
    struct holder * hold = (holder *) vargp;
    long sock = hold->sockfd;
    printf("[DEBUG] SOCKFD IS: %d\n", sock);
    struct sockaddr_in empty;
    int n, commands, elementLength, bytesInBuffer = 0;
    char buffer[1024];          //Holds multiple commands (if needed i.e. concatination).
    bzero(buffer,1024);
    
    unsigned char bufferByte[1024];          //Holds ASN1 encdoing.
    bzero(bufferByte,1024);
    unsigned char bufTemp[512];          //Holds individual commands.
    bzero(bufTemp, 512);
    int i = 0;
    while (n = read(sock, bufTemp, 511)) {                    //Read from socket
        if (n == -1) {
            error("ERROR client connection timed out");
            break;
        }
        
        bufAppendByte(bufferByte, bufTemp, bytesInBuffer, n, 1024, 512);        //Append bufTemp to buffer.
        bytesInBuffer += n;
        
        elementLength = contentLength(bufferByte);
        
        if (bytesInBuffer >= elementLength) {
            commands = 1;
        } else {
            commands = 0;
        }
        
        while (commands) {
            elementLength = contentLength(bufferByte);
            
            if (bytesInBuffer < elementLength) {
                commands = 0;
                break;
            }
            
            try {
                ASN1_Decoder* d = new ASN1_Decoder(bufferByte, elementLength);
                
                if (d->tagVal() == 1) {
                    Gossip n;
                    n.decode(d);
                    
                    memcpy(buffer, "GOSSIP:", 7);
                    int index = 7;
                    memcpy(buffer + index, n.sha256hash, 44);
                    index += 44;
                    memcpy(buffer + index, ":", 1);
                    index += 1;
                    char date[8];
                    bzero(date, 8);
                    itoa(n.timestamp->year, date, 10);
                    memcpy(buffer + index, date, 4);
                    index += 4;
                    memcpy(buffer + index, "-", 1);
                    index += 1;
                    bzero(date, 8);
                    itoa(n.timestamp->month, date, 10);
                    if (countDigit(n.timestamp->month) % 2 == 0) {
                        memcpy(buffer + index, date, 2);
                        index += 2;
                    } else {
                        memcpy(buffer + index, "0", 1);
                        index += 1;
                        memcpy(buffer + index, date, 1);
                        index += 1;
                    }
                    memcpy(buffer + index, "-", 1);
                    index += 1;
                    bzero(date, 8);
                    itoa(n.timestamp->day, date, 10);
                    if (countDigit(n.timestamp->day) % 2 == 0) {
                        memcpy(buffer + index, date, 2);
                        index += 2;
                        memcpy(buffer + index, "-", 1);
                        index += 1;
                        
                        memcpy(buffer + index, date + 2, 2);
                        index += 2;
                        memcpy(buffer + index, "-", 1);
                        index += 1;
                        
                        memcpy(buffer + index, date + 4, 2);
                        index += 2;
                        memcpy(buffer + index, "-", 1);
                        index += 1;
                        
                        memcpy(buffer + index, date + 6, 2);
                        index += 2;
                        memcpy(buffer + index, "-", 1);
                        index += 1;
                    } else {
                        memcpy(buffer + index, "0", 1);
                        index += 1;
                        memcpy(buffer + index, date, 1);
                        index += 1;
                        memcpy(buffer + index, "-", 1);
                        index += 1;
                        
                        memcpy(buffer + index, date + 1, 2);
                        index += 2;
                        memcpy(buffer + index, "-", 1);
                        index += 1;
                        
                        memcpy(buffer + index, date + 3, 2);
                        index += 2;
                        memcpy(buffer + index, "-", 1);
                        index += 1;
                        
                        memcpy(buffer + index, date + 5, 2);
                        index += 2;
                        memcpy(buffer + index, "-", 1);
                        index += 1;
                    }
                    
                    memcpy(buffer + index, "000Z:", 5);
                    index += 5;
                    memcpy(buffer + index, n.message, strlen(n.message));
                    index += strlen(n.message);
                    memcpy(buffer + index, "%", 1);
                    
                } else if (d->tagVal() == 2) {
                    Peer p;
                    p.decode(d);
                    
                    memcpy(buffer, "PEER:", 5);
                    int index = 5;
                    memcpy(buffer + index, p.name, strlen(p.name));
                    index += strlen(p.name);
                    memcpy(buffer + index, ":PORT=", 6);
                    index += 6;
                    int charsInPort = countDigit(p.port);
                    char portNumber[charsInPort];
                    itoa(p.port, portNumber, 10);
                    memcpy(buffer + index, portNumber, charsInPort);
                    index += charsInPort;
                    memcpy(buffer + index, ":IP=", 4);
                    index += 4;
                    memcpy(buffer + index, p.ip, strlen(p.ip));
                    index += strlen(p.ip);
                    buffer[index] = '%';
                    
                } else if (d->tagVal() == 3) {
                    memcpy(buffer, "PEERS?", 6);
                } else if (d->tagVal() == 4) {
                    Leave l;
                    l.decode(d);
                    
                    memcpy(buffer, "LEAVE:", 6);
                    memcpy(buffer + 6, l.name, strlen(l.name));
                    memcpy(buffer + 6 + strlen(l.name), "%", 1);
                } else {
                    memcpy(buffer, "error", 5);
                }
                
            } catch (const std::exception& e) {
                memcpy(buffer, "error", 5);
            }
            
            printf("Buffer contains: %s\n", buffer);
            
            int t = isValidForm(buffer);
            if (t == 1) {                                                       //GOSSIP command entry point.
                char  portAndIP[30];//longer than needed by who cares
                itoa(hold->port, portAndIP, 10);
                strcat(portAndIP, ":");
                strcat(portAndIP, hold->ip);
                printf("[DEBUG] assembled port and IP is: %s\n", portAndIP);
                //addPeerLeave(portAndIP, filenamePath);
                
                GOSSIP(buffer, filenamePath, bufferByte, elementLength);        //Handle GOSSIP command
                clearBuffer(bufferByte, elementLength, 1024);
                bzero(buffer, 1024);
            } else if (t == 2) {                           //PEER command entry point.
                PEER(buffer, filenamePath);        //Handle GOSSIP command
                clearBuffer(bufferByte, elementLength, 1024);
                bzero(buffer, 1024);
            } else if (t == 3) {                           //PEERS? command entry point.
                PEERS(sock, empty, filenamePath, 1);       //Handle PEERS? command
                clearBuffer(bufferByte, elementLength, 1024);
                bzero(buffer, 1024);
            } else if (t == 4) {
                LEAVE(buffer, filenamePath);        //Handle LEAVE command
                clearBuffer(bufferByte, elementLength, 1024);
                bzero(buffer, 1024);
            } else {                                       //Faulty command entry point.
                error("ERROR, command non found!");
                clearBuffer(bufferByte, elementLength, 1024);
                bzero(buffer, 1024);
            }
            
            int newBufferLength = bytesInBuffer - elementLength;
            
            if (elementLength == 0) {
                commands = 0;
            } else if (newBufferLength > 0) {
                commands = 1;
                bytesInBuffer = newBufferLength;
            } else {
                commands = 0;
            }
        }
        
        bzero(bufTemp, 512);
        bytesInBuffer = 0;
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
 * BUFAPPENDBYTE appends a smaller buffer (bufTemp) into a larger one (buffer)
 * INPUT: dest: destination buffer; src: source buffer; destLen: length of destination buffer
 *        srcLen: length of source buffer
 * OUTPUT: 0 if successful append
 *        -1 if failure to append
 */
int bufAppendByte(unsigned char * dest, unsigned char * src, int d, int s, int destLen, int srcLen) {
    int destN = d;
    int srcN = s;
    
    if (destN+srcN+1 > destLen) return -1;                      //Not enough space
    for (int i = d; i < d + s; i++) {
        dest[i] = src[i];
    }
    return 0;
}
/*
 * CLEARBUFFER clears a number of characters (i.e. an entire command) from a buffer
 * INPUT: buffer: a buffer; howFar: number of spaces to clear from the buffer
 *        bufferSize: size of buffer
 * OUTPUT: void
 */
void clearBuffer(unsigned char * buffer, int howFar, int bufferSize) {
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
    unsigned char buffer[1024];
    bzero(buffer, 1024);
    
    n = recvfrom(udpfd, buffer, 1024, 0, (struct sockaddr *) &cli_addr, &len); //Recive command.
    
    ASN1_Decoder* d;
    
    try {
        d = new ASN1_Decoder(buffer, n);
    } catch (const std::exception& e) {
        error("ERROR: decoding in server failed");
    }
    
    if (d->tagVal() == 1) {
        Gossip g;
        g.decode(d);
        
        memcpy(msg, "GOSSIP:", 7);
        int index = 7;
        memcpy(msg + index, g.sha256hash, 44);
        index += 44;
        memcpy(msg + index, ":", 1);
        index += 1;
        char date[8];
        bzero(date, 8);
        itoa(g.timestamp->year, date, 10);
        memcpy(msg + index, date, 4);
        index += 4;
        memcpy(msg + index, "-", 1);
        index += 1;
        bzero(date, 8);
        itoa(g.timestamp->month, date, 10);
        if (countDigit(g.timestamp->month) % 2 == 0) {
            memcpy(msg + index, date, 2);
            index += 2;
        } else {
            memcpy(msg + index, "0", 1);
            index += 1;
            memcpy(msg + index, date, 1);
            index += 1;
        }
        memcpy(msg + index, "-", 1);
        index += 1;
        bzero(date, 8);
        itoa(g.timestamp->day, date, 10);
        if (countDigit(g.timestamp->day) % 2 == 0) {
            memcpy(msg + index, date, 2);
            index += 2;
            memcpy(msg + index, "-", 1);
            index += 1;
            
            memcpy(msg + index, date + 2, 2);
            index += 2;
            memcpy(msg + index, "-", 1);
            index += 1;
            
            memcpy(msg + index, date + 4, 2);
            index += 2;
            memcpy(msg + index, "-", 1);
            index += 1;
            
            memcpy(msg + index, date + 6, 2);
            index += 2;
            memcpy(msg + index, "-", 1);
            index += 1;
        } else {
            memcpy(msg + index, "0", 1);
            index += 1;
            memcpy(msg + index, date, 1);
            index += 1;
            memcpy(msg + index, "-", 1);
            index += 1;
            
            memcpy(msg + index, date + 1, 2);
            index += 2;
            memcpy(msg + index, "-", 1);
            index += 1;
            
            memcpy(msg + index, date + 3, 2);
            index += 2;
            memcpy(buffer + index, "-", 1);
            index += 1;
            
            memcpy(msg + index, date + 5, 2);
            index += 2;
            memcpy(msg + index, "-", 1);
            index += 1;
        }
        
        memcpy(msg + index, "000Z:", 5);
        index += 5;
        memcpy(msg + index, g.message, strlen(g.message));
        index += strlen(g.message);
        memcpy(msg + index, "%", 1);
        
    } else if (d->tagVal() == 2) {
        Peer p;
        p.decode(d);
        
        memcpy(msg, "PEER:", 5);
        int index = 5;
        memcpy(msg + index, p.name, strlen(p.name));
        index += strlen(p.name);
        memcpy(msg + index, ":PORT=", 6);
        index += 6;
        int charsInPort = countDigit(p.port);
        char portNumber[charsInPort];
        itoa(p.port, portNumber, 10);
        memcpy(msg + index, portNumber, charsInPort);
        index += charsInPort;
        memcpy(msg + index, ":IP=", 4);
        index += 4;
        memcpy(msg + index, p.ip, strlen(p.ip));
        index += strlen(p.ip);
        msg[index] = '%';
        
    } else if (d->tagVal() == 3) {
        memcpy(msg, "PEERS?", 6);
    } else if (d->tagVal() == 4) {
        Leave l;
        l.decode(d);
        
        memcpy(msg, "LEAVE:", 6);
        memcpy(msg + 6, l.name, strlen(l.name));
        memcpy(msg + 6 + strlen(l.name), "%", 1);
    } else {
        memcpy(msg, "error", 5);
    }
    
    int t = isValidForm(msg);
    if (t == 1) {
        
        struct holder hold;
        hold.port = ntohs(cli_addr.sin_port);
        inet_ntop(AF_INET, &cli_addr.sin_addr, hold.ip, 30);
        char  portAndIP[30];//longer than needed by who cares
        itoa(hold.port, portAndIP, 10);
        strcat(portAndIP, ":");
        strcat(portAndIP, hold.ip);
        printf("[DEBUG UDP] assembled port and IP is: %s\n", portAndIP);
        //addPeerLeave(portAndIP, path);
        
        GOSSIP(msg, path, buffer, n);
    } else if (t == 2) {
        PEER(msg, path);
    } else if (t == 3) {
        PEERS(udpfd, cli_addr, path, 0);
    } else if (t == 4) {
        LEAVE(msg, path);
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
int GOSSIP(char * buf, char * path, unsigned char * asn1buf, int asn1buflength) {
    char filePath[strlen(path) + 15];
    strcpy(filePath, path);
    strcat(filePath, "fgossip.txt");
    char filePathPeer[strlen(path) + 15];
    strcpy(filePathPeer, path);
    strcat(filePathPeer, "fpeers.txt");
    
    checkIfExipired(path);
    
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
            broadcastToPeersUDP(asn1buf, i, filePathPeer, asn1buflength);
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
void broadcastToPeersUDP(unsigned char* buf, int index, char* path, int lengthBuf) {
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
    if (sendto(s, buf, lengthBuf , 0 , (struct sockaddr *) &si_other, slen)==-1) {
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

int LEAVE(char * buf, char * path) {
    char filePath[strlen(path) + 30];
    strcpy(filePath, path);
    strcat(filePath, "ftimeout.txt");
    char filePeerPath[strlen(path) + 30];
    strcpy(filePeerPath, path);
    strcat(filePeerPath, "fpeers.txt");
    char fileTempPath[strlen(path) + 30];
    strcpy(fileTempPath, path);
    strcat(fileTempPath, "output.txt");
    
    int nameLen = strlen(buf) - 6 - 1;
    char name[nameLen + 1];
    bzero(name, nameLen + 1);
    
    int index = 0;
    while (buf[index + 6] != '%') {
        name[index] = buf[index + 6];
        index++;
    }
    printf("Name: %s\n", name);
    
    if (access(filePath, F_OK) == -1) {
        return 0;
    }
    if (access(filePeerPath, F_OK) == -1) {
        return 0;
    }
    
    int numberOFPeers = peerNumber(filePeerPath);
    int peerPos[numberOFPeers];
    
    for (int i = 0; i < numberOFPeers; i++) {
        peerPos[i] = 0;
    }
    
    printf("Number of peers: %d\nArray of PEERS: ", numberOFPeers);
    
    for (int i = 0; i < numberOFPeers; i++) {
        printf("%d", peerPos[i]);
        printf("-");
    }
    printf("\n");
    
    int locationOfPeer = isKnown(name, filePeerPath, '1');
    printf("Location of PEER: %d\n", locationOfPeer);
    if (locationOfPeer) {
        peerPos[ (locationOfPeer - 2)/5 ] = 1;
        printf("Position in the array: %d\n", (locationOfPeer - 2)/5);
        removeEntries(filePeerPath, fileTempPath, 5, peerPos, numberOFPeers);
        removeEntries(filePath, fileTempPath, 4, peerPos, numberOFPeers);
    } else {
        error("ERROR, peer not found");
    }
    
    return 0;
}
/*
* PEER adds or updates peers in the peer file.
* INPUT: path: file directory path; buf: a buffer
* OUTPUT: 0 if adding or updating was succesful
*        -1 if any errors accured
*/
int PEER(char * buf, char * path) {
    
    printf("Timestamp: %d\n", time(NULL));
    
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
    char fileTimePath[strlen(path) + 30];
    strcpy(fileTimePath, path);
    strcat(fileTimePath, "ftimeout.txt");
    
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
    
    char temp[6+17+2]; //port:ip + nullterminator
    bzero(temp, 6+17+2);
    strcat(temp, port);
    strcat(temp, ":");
    strcat(temp, ip);
    addPeerLeave(temp, path);
    error("past addPeerLeave");
    
    int lineToUpdate = isKnown(name, filePath, '1');            //Does peer exist in the file?
    if (lineToUpdate) {
        if (updateFile(ip, lineToUpdate, filePath, filePathTemp, 3) == -1) { return -1; }  //Yes? Update it.
        int lineInTimestamp = ((lineToUpdate - 2)/5 ) * 4;
        if (updateFile(temp, lineInTimestamp, fileTimePath, filePathTemp, 4) == -1) { return -1; }  //Yes? Update it.
    } else {                                                                            //No? Add it.
        error("writing to file");
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
int updateFile(char* ip, int line, char * peersPath, char * tempPath, int match) {
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
            fprintf(fupdated, "\n%d:%s\n", match, ip);          //print the new line
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
    printf("Timestamp: %d\n", time(NULL));
    checkIfExipired(path);
    
    char filePath[strlen(path) + 15];
    strcpy(filePath, path);
    strcat(filePath, "fpeers.txt");
    
    unsigned len = sizeof(cli_addr);
    char currC;
    const char* noPeers = "PEERS|0|%\n";
    int charNumber = 0, lineNumber = 0;
    
    sem_wait(&mutex_fpeers);                         //Semaphore waits
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
    
    const char * intro = "PEERS|";
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
    const char * port = ":PORT=";
    const char * ip = ":IP=";
    
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
    
    PeerAnswer pa;
    pa.n_rcv = peersNumber;
    pa.rcv = new Peer*[peersNumber];
    int index = 7 + countDigit(peersNumber);
    int j = 0;
    char asn1port[6];
    bzero(asn1port,6);
    char names[peersNumber][64];
    char ips[peersNumber][17];
    
    for (int i = 0; i < peersNumber; i++) {
        Peer* p = new Peer();
        bzero(names[i], 64);
        bzero(ips[i], 17);
        while (message[index] != ':') { names[i][j++] = message[index++]; }
        p->name = names[i];
        index += 6;
        j = 0;
        while (message[index] != ':') { asn1port[j++] = message[index++]; }
        p->port = atoi(asn1port);
        index += 4;
        j = 0;
        while (message[index] != '|') { ips[i][j++] = message[index++]; }
        p->ip = ips[i];
        index += 1;
        j = 0;
        pa.rcv[i] = p;
    }
    int bytesEncoded = 0;
    
    ASN1_Encoder* enc = pa.getEncoder();
    byte* msg = enc->getBytes();
    bytesEncoded = enc->getBytesNb();
    
    if (fclose(fpeers)) { error("ERROR, file not closed properly"); sem_post(&mutex_fpeers); return -1; }
    sem_post(&mutex_fpeers);                                 //Semaphore signals
    
    if (tcpFlag) {
        write(sockfd, msg, bytesEncoded);          //Sending message over TCP
    } else {
        sendto(sockfd, msg, bytesEncoded, 0, (struct sockaddr *) &cli_addr, len); //UDP
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
 * ERRORBYTE prints the message to error stream
 * INPUT: msg: string
 * OUTPUT: void
 */
void errorByte(unsigned char *msg) {
    fprintf(stderr, "%s\n", msg);
}
/*
 * ERROR prints the message to error stream
 * INPUT: msg: string
 * OUTPUT: void
 */
void error(const char *msg) {
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
