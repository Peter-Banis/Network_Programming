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
#include "server.h"

int main(int arc, char **argv) {
    /*char testAppend0[20];
    bzero(testAppend0,20);
    printf("Got here");
    char testAppend1[5];
    bzero(testAppend1,5);
    printf("Got here too");
    char testAppend2[12];
    bzero(testAppend2,12);
    printf("Got here as well");
    printf(bufAppend(testAppend1, testAppend0, 5, 20) == -1);
    testAppend0[0] = 'h';
    testAppend0[1] = 'e';
    testAppend0[2] = 'l';
    testAppend0[3] = 'l';
    testAppend0[4] = 'o';
    printf("Passed first bufAppend");
    bufAppend(testAppend0, "test", 20, strlen("test"));
    printf(0==strcmp("hellotest",bufAppend0));
    printf("Now passed second bufAppend");
    bzero(testAppend1,5);
    bzero(testAppend2, 12);
    printf(0==strcmp(bufAppend(testAppend1, "hell", 5, strlen("hell")),"hell"));   
    printf("Got past the bufAppend");*/
    GOSSIP("GOSSIP:mBHL7IKilvdcOFKR03ASvBNX//ypQkTRUvilYmB1/OY=:2018-01-09-16-18-20-001Z:Tom eats Jerry%");
    GOSSIP("GOSSIP:mBHL7IKilvdcOFKR03ASvBNX//ypQkTRUvilYmB1/OY=:2018-01-09-16-18-21-001Z:Tom eats Jerry%");
    GOSSIP("GOSSIP:mBHL7IKilvdcOFKR03ASvBNX//ypQkTRUvilYmB1/OY=:2018-01-09-16-18-20-001Z:Tom eats Jerry2%");
    GOSSIP("GOSSIP:mBHL7IKilvdcOFKR03ASvBNX//ypQkTRUvilYmB1/OY=:2018-01-09-16-18-20-001Z:Tom eats Jerryaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa%");
    GOSSIP("GOSSIP:mBHL7IKilvdcOFKR03ASvBNX//ypQkTRUvilYmB1/OY=:2018-01-09-16-18-20-001Z:a%");
    printf("Got past the gossip");
    PEER("PEER:John:PORT=2356:IP=163.118.239.68%");
    PEER("PEER:John:PORT=2356:IP=163.118.239.69%");
    PEER("PEER:Timothy:PORT=2356:IP=163.118.239.70%");
    printf("Got past the peer");
    printf(0 == strcmp("aaa", removeNewLines("aaa")));
    printf(0 == strcmp("aaa", removeNewLines("a\naa")));
    printf("Got past the remove new lines");
    printf(0 != isKnown("Tom eats Jerry"));
    printf(0 != isKnown("a"));
    printf(0 == isKnown("fifteen"));
    printf("Got past the isKnown");
    printf(2 == countDigit(10));
    printf(1 == countDigit(5));
    printf(3 == countDigit(143));
    printf("Got past the countDigit");
    return 0;


}
