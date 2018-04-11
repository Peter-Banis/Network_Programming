//
//  PeerAnswer.h
//  
//
//  Created by Klaus Cipi on 4/5/18.
//

#include <stdio.h>
#include "BigInteger.h"
#include "ASN1_Util.h"
#include "ASN1Decoder.h"
#include "ASN1Encoder.h"
#include "Peer.h"

/*
 PeersAnswer ::= [1] EXPLICIT SEQUENCE OF Peer
 */

struct PeerAnswer : ASNObj
{
    int n_rcv;
    Peer* snd;
    Peer** rcv;
    
    ASN1_Encoder* getEncoder() {
        ASN1_Encoder * r = new ASN1_Encoder();
        r->initSequence();
        r->addToSequence(
                         ASN1_Encoder::getEncoder((ASNObjArrayable**)rcv, n_rcv)
                         );
        return r;
    }
    PeerAnswer* decode(ASN1_Decoder* d) {
        d = d->getContentImplicit();
        rcv = (Peer**)d->getSequenceOf(new Peer(), &n_rcv);
        delete d;
        return this;
    }
};
