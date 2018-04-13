//
//  Peer.h
//
//
//  Created by Klaus Cipi on 4/5/18.
//

#include <stdio.h>
#include "BigInteger.h"
#include "ASN1_Util.h"
#include "ASN1Decoder.h"
#include "ASN1Encoder.h"

/*
 PeersQuery ::= [APPLICATION 3] IMPLICIT NULL
 */

static const byte TAG_3 = ASN1_Encoder::buildASN1byteType(ASN1_Encoder::CLASS_APPLICATION, ASN1_Encoder::PC_CONSTRUCTED,(byte)3);

struct PeersQuery : public ASNObjArrayable
{
    unsigned char* empty = NULL;
    
    ASN1_Encoder* getEncoder()  {
        ASN1_Encoder * r = new ASN1_Encoder();
        r->initSequence();
        r->addToSequence(new ASN1_Encoder(empty));
        r->setASN1TypeImplicit(TAG_3);
        return r;
    }
    
    PeersQuery* decode(ASN1_Decoder* d) {
        d = d->getContentImplicit();
        empty = d->getSkipBytesAnyType();
        delete d;
        return this;
    }
    
    byte getASN1Type() {
        return TAG_3;
    }
    
    PeersQuery* instance() {
        return new PeersQuery();
    }
};
