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
    m.n_rcv = 2;
    m.snd = new Peer();
    m.snd->name = "Test";
    m.snd->port = 12345;
    m.snd->ip = "127.0.0.1";
    m.rcv = new Peer*[0];
    m.rcv[0] = m.snd;
    
    m.snd = new Peer();
    m.snd->name = "Test2";
    m.snd->port = 12342;
    m.snd->ip = "127.0.0.2";
    m.rcv[1] = m.snd;
    
    ASN1_Encoder* enc = m.getEncoder();
    byte* msg = enc->getBytes();
    
    ASN1_Decoder* d = new ASN1_Decoder(msg, enc->getBytesNb());
    PeerAnswer n;
    n.decode(d);
    for (int i = 0; i < n.n_rcv ; i ++ ) {
        printf("%d: name=%s port=%d ip=%s\n", i, n.rcv[i]->name, n.rcv[i]->port, n.rcv[i]->ip);
    }
    
    return 0;
}
