#include <stdio.h>
#include <string.h>
#include <ctype.h>
int isValidForm(char *);

int main(int argc, char** argv) {
    //should pass
    printf("Begin valid GOSSIP testing\n");
    printf("1 == %d\n", isValidForm("GOSSIP:mBHL7IKilvdcOFKR03ASvBNX//ypQkTRUvilYmB1/OY=:2018-01-09-16-18-20-001Z:Tom eats Jerry%"));
    printf("1 == %d\n", isValidForm("GOSSIP:mBHL7IKilvdcOFKR03ASvBNX//ypQkTRUvilYmB1/OY=:2018-01-09-16-18-21-001Z:Tom eats Je%"));
    printf("Begin valid PEER testing\n");
    printf("2 == %d\n", isValidForm("PEER:John:PORT=2356:IP=163.118.239.68%"));
    printf("2 == %d\n", isValidForm("PEER:John:PORT=23565:IP=163.118.239.68%"));
    printf("2 == %d\n", isValidForm("PEER:John:PORT=2:IP=163.118.239.68%"));
    printf("2 == %d\n", isValidForm("PEER:J:PORT=2356:IP=163.118.239.68%"));
    printf("2 == %d\n", isValidForm("PEER:John:PORT=2356:IP=163.118.239.168%"));
    printf("Begin valid PEERS? testing\n");
    printf("3 == %d\n", isValidForm("PEERS?"));
    //begin error testing
    printf("Begin invalid testing, generic\n");
    printf("-1 == %d\n", isValidForm("G"));
    printf("-1 == %d\n", isValidForm("P"));
    printf("-1 == %d\n", isValidForm("D"));
    printf("Begin invalid PEER testing\n");
    printf("-1 == %d\n", isValidForm("PEER::PORT=2356:IP=163.118.239.68%"));
    printf("-1 == %d\n", isValidForm("PEER:John:PORT2356:IP=163.118.239.68%"));
    printf("-1 == %d\n", isValidForm("PEER:John:ORT=2356:IP=163.118.239.68%"));
    printf("-1 == %d\n", isValidForm("PEER:John:PORT=2356:IP163.118.239.68%"));
    printf("-1 == %d\n", isValidForm("PEER:John:PORT=2356:P=163.118.239.68%"));
    printf("-1 == %d\n", isValidForm("PEER:John:PORT=2356:IP=163.118.239.6822%"));
    printf("-1 == %d\n", isValidForm("PEER:John:PORT=2356:IP=163.118.239.6.8%"));
    printf("-1 == %d\n", isValidForm("PEER:John:PORT=2356:IP=163.118.239.68"));
    printf("Begin invalid GOSSIP testing\n");
    printf("-1 == %d\n", isValidForm("GOSSIP:mBHL7IKilvdcOFKR03ASvBNX//ypQkTRUvilYmB1/OY=:2018-01-09-16-18-20-001Z:Tom eats Jerry"));
    printf("-1 == %d\n", isValidForm("GOSSIP:mBHL7IKilvdcOFKR03ASvBNX//ypQkTRUvilYmBa1/OY=:2018-01-09-16-18-20-001Z:Tom eats Jerry%"));
    printf("-1 == %d\n", isValidForm("GOSSIP:mBHL7IKilvdcOFKR03ASvBNX//ypQkTRUvilmB1/OY=:2018-01-09-16-18-20-001Z:Tom eats Jerry%"));
    printf("-1 == %d\n", isValidForm("GOSSIP:mBHL7IKilvdcOFKR03ASvBNX//ypQkTRUvilYmB1/OY=:2018-01-09-1618-20-001Z:Tom eats Jerry%"));
    printf("-1 == %d\n", isValidForm("GOSSIP:mBHL7IKilvdcOFKR03ASvBNX//ypQkTRUvilYmB1/OY=:2018-01-09-16-18-2-0-001Z:Tom eats Jerry%"));
    printf("-1 == %d\n", isValidForm("GOSSP:mBHL7IKilvdcOFKR03ASvBNX//ypQkTRUvilYmB1/OY=:2018-01-09-16-18-20-001Z:Tom eats Jerry%"));
    printf("-1 == %d\n", isValidForm("GOSSIP::2018-01-09-16-18-20-001Z:Tom eats Jerry%"));
    printf("-1 == %d\n", isValidForm("GOSSIP:mBHL7IKilvdcOFKR03ASvBNX//ypQkTRUvilYmB1/OY=::Tom eats Jerry%"));
    printf("Begin invalid PEERS? testing\n");
    printf("-1 == %d\n", isValidForm("PEERST?"));
    printf("-1 == %d\n", isValidForm("PEARS?"));

}


int isValidForm(char * buf) { 
 //a command can be of three types, GOSSIP, PEER or PEERS? 
 //The form of GOSSIP is "GOSSIP:"[sha]:[time]:[message]% 
     if (buf[0] == 'G') { 
         char cmpbuf[7]; 
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
 
 
     } else if (buf[0] == 'P') { 
          //first check for PEERS? since it's the easiest 
          char buftemp[7]; 
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
          if (i == 5) return -1; //no name
          if (buf[i] == '\0' || buf[i] == '%') return -1; //missing [PORT], [IP]
          //now check for PORT= 
          ++i; 
          // confirm PORT= 
          bzero(buftemp, 7); 
          int limit = i+5; 
          int offset = 0; 
          for (; i < limit; i++, offset++) { 
              if (buf[i] == '%' || buf[i] == '\0') return -1;//doesn't have PORT= 
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
                  if (offset == 0) return -1; //was .. 
                  offset=0; 
                  limit++; 
              } else {
                  if (isdigit(buf[i])) offset++;
                  else return -1; //invalid character in ip
                  if (limit > 3) return -1; //too many '.' in IP 
                  if (offset >  3) return -1; //an IP segment must have no more than three digits, the only option if we have recorded three digits and did not record '.' is a malformed IP 
              }
          } 
          if (buf[i] != '%' || limit != 3) return -1; 
          return 2; 
     }  
     return -1; //malformed from the start 
} 

