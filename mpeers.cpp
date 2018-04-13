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
#include "PeersQuery.h"

int main (int arc, char ** arg) {
    PeersQuery* m = new PeersQuery();
    
    ASN1_Encoder* enc = m->getEncoder();
    byte* msg = enc->getBytes();
    
    ASN1_Decoder* d = new ASN1_Decoder(msg, enc->getBytesNb());
    PeersQuery n;
    n.decode(d);
    
    if (n.getASN1Type() == TAG_3) {
        printf("Yes\n");
        printf("%d\n", enc->getBytesNb());
    }
    
    return 0;
}
