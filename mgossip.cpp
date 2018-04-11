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
    /* ---- not complete ---- */
    Gossip* m = new Gossip();
    m->message = "Ishalla";
    
    ASN1_Encoder* enc = m->getEncoder();
    byte* msg = enc->getBytes();
    
    ASN1_Decoder* d = new ASN1_Decoder(msg, enc->getBytesNb());
    Gossip n;
    n.decode(d);
    printf("%s\n", n.message);
    
    return 0;
}
