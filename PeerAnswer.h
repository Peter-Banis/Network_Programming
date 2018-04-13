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

static const byte TAG_PA1 = ASN1_Encoder::buildASN1byteType(ASN1_Encoder::CLASS_CONTEXT, ASN1_Encoder::PC_CONSTRUCTED,(byte)1);

struct PeerAnswer : ASNObj
{
    int n_rcv;
    Peer** rcv;
    
    ASN1_Encoder* getEncoder() {
        ASN1_Encoder * r = ASN1_Encoder::getEncoder((ASNObjArrayable**)rcv, n_rcv);
        r = r->setASN1TypeExplicit(TAG_PA1);
        return r;
    }
    PeerAnswer* decode(ASN1_Decoder* d) {
        //d = d->removeExplicitASN1TagInplace();
        d = d->removeExplicitASN1Tag();
        rcv = (Peer**)d->getSequenceOf(new Peer(), &n_rcv);
        delete d;
        return this;
    }
};
