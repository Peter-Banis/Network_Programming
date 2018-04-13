//
//  PeerAnswer
//  
//
//  Created by Klaus Cipi on 4/5/18.
//

#include <stdio.h>
#include <iostream>
#include <fstream>
#include "BigInteger.h"
#include "ASN1_Util.h"
#include "ASN1Decoder.h"
#include "ASN1Encoder.h"
#include "PeerAnswer.h"

int main (int arc, char ** arg) {
    PeerAnswer m;
    m.n_rcv = 1;
    m.rcv = new Peer*[m.n_rcv];
    //for (int i = 0; i < m.n_rcv; i++) {
        Peer* p = new Peer();
        p->name = "Testaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa";
        p->port = 12345;
        p->ip = "127.0.0.1";
        m.rcv[0] = p;
    //}
    printf("Error\n");
    ASN1_Encoder* enc = m.getEncoder();
    byte* msg = enc->getBytes();
    printf("number of bytes: %d\n",enc->getBytesNb());
    printf("\n");
    printf("Error\n");
    ASN1_Decoder* d = new ASN1_Decoder(msg, enc->getBytesNb());
    PeerAnswer n;
    printf("Error\n");
    n.decode(d);
    printf("Error\n");
    for (int i = 0; i < n.n_rcv ; i ++ ) {
        printf("%d: name=%s port=%d ip=%s\n", i, n.rcv[i]->name, n.rcv[i]->port, n.rcv[i]->ip);
    }
    
    return 0;
}
