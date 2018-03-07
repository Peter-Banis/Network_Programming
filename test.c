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
    char testAppend0[20];
    bzero(testAppend0,20);
    char testAppend1[5];
    bzero(testAppend1,5);
    char testAppend2[12];
    bzero(testAppend2,12);
    printf("should be 0, is %d\n", bufAppend(testAppend1, testAppend0, 5, 20));
    testAppend0[0] = 'h';
    testAppend0[1] = 'e';
    testAppend0[2] = 'l';
    testAppend0[3] = 'l';
    testAppend0[4] = 'o';
    bufAppend(testAppend0, "test", 20, strlen("test"));
    printf("should be 0, is %d\n",strcmp("hellotest",testAppend0));
    bzero(testAppend1,5);
    bzero(testAppend2, 12);
    bzero(testAppend1,5);
    printf("should be -1, is %d\n", bufAppend(testAppend1, "hellothere", 5, strlen("hellothere")));   
    printf("Got past the bufAppend, beginning GOSSIP\n");
    /*GOSSIP("GOSSIP:mBHL7IKilvdcOFKR03ASvBNX//ypQkTRUvilYmB1/OY=:2018-01-09-16-18-20-001Z:Tom eats Jerry%");
    GOSSIP("GOSSIP:mBHL7IKilvdcOFKR03ASvBNX//ypQkTRUvilYmB1/OY=:2018-01-09-16-18-21-001Z:Tom eats Jerry%");
    GOSSIP("GOSSIP:mBHL7IKilvdcOFKR03ASvBNX//ypQkTRUvilYmB1/OY=:2018-01-09-16-18-20-001Z:Tom eats Jerry2%");
    //GOSSIP("GOSSIP:mBHL7IKilvdcOFKR03ASvBNX//ypQkTRUvilYmB1/OY=:2018-01-09-16-18-20-001Z:Tom eats Jerryaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa%");
    GOSSIP("GOSSIP:mBHL7IKilvdcOFKR03ASvBNX//ypQkTRUvilYmB1/OY=:2018-01-09-16-18-20-001Z:a%");*/
    printf("Got past the gossip, beginning PEER\n");
    PEER("PEER:John:PORT=2356:IP=163.118.239.68%");
    PEER("PEER:John:PORT=2356:IP=163.118.239.69%");
    PEER("PEER:Timothy:PORT=2356:IP=163.118.239.70%");
    printf("Got past the peer, beginning removeNewLines\n");
    char buf0[] = "a\naa";
    char buf1[] = "aaa";
    char buf2[] = "\n";
    removeNewLines(buf0);
    printf("No segfault here\n");
    removeNewLines(buf1);
    printf("Second call and no segfault\n");
    removeNewLines(buf2);
    printf("Third call and no segfault\n");
    printf("Should be 0, is %d\n", strcmp("aaa",buf0 ));
    printf("Should be 0, is %d\n", strcmp("aaa", buf1));
    printf("Should be non-zero, is %d\n", strcmp("\n", buf2));
    printf("Got past the remove new lines, beginning isKnown\n");
    printf("Should be non-zero, is %d\n", isKnown("Tom eats Jerry"));
    printf("Should be non-zero, is %d\n",isKnown("a"));
    printf("Should be 0, is %d\n", isKnown("fifteen"));
    printf("Got past the isKnown, beginning countDigit\n");
    printf("Should be 2, is %d\n", countDigit(10));
    printf("Should be 1, is %d\n", countDigit(5));
    printf("Should be 3, is %d\n", countDigit(143));
    printf("Got past the countDigit, current test set complete\n");
    return 0;


}
