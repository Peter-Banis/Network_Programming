//
//  Gossip.cpp
//
//
//  Created by Klaus Cipi on 4/5/18.
//

#include <stdio.h>
#include <string.h>
#include <iostream>
#include <fstream>
#include "BigInteger.h"
#include "ASN1_Util.h"
#include "ASN1Decoder.h"
#include "ASN1Encoder.h"
#include "Gossip.h"

int main (int arc, char ** arg) {
    Gossip* m = new Gossip();
    char * hash = "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa";
    m->sha256hash = (unsigned char *) hash;
    m->message = "hello";
    
    ASN1_Encoder* enc = m->getEncoder();
    byte* msg = enc->getBytes();
    for (int i = 0; i < enc->getBytesNb(); i++) {
        printf("%x", msg[i]);
    }
    printf("\nNumber of bytes: %d\n", enc->getBytesNb());
    
    ASN1_Decoder* d = new ASN1_Decoder(msg, enc->getBytesNb());
    Gossip n;
    n.decode(d);
    printf("sha:%s\ntime:%s\nmsg:%s\n", n.sha256hash, n.timestamp, n.message);
    
    return 0;
}
