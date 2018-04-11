//
//  Peer
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
#include "PeerAnswer.h"

int main (int arc, char ** arg) {
    Peer* m = new Peer();
    m->name = "Peer";
    m->port = 12345;
    m->ip = "127.0.0.1";
    
    ASN1_Encoder* enc = m->getEncoder();
    byte* msg = enc->getBytes();
    
    ASN1_Decoder* d = new ASN1_Decoder(msg, enc->getBytesNb());
    Peer n;
    n.decode(d);
    printf("%s %d %s\n", n.name, n.port, n.ip);
    
    return 0;
}
