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
Peer ::= [APPLICATION 2] IMPLICIT SEQUENCE {name UTF8String, port INTEGER, ip PrintableString}
*/

static const byte TAG_2 = ASN1_Encoder::buildASN1byteType(ASN1_Encoder::CLASS_APPLICATION, ASN1_Encoder::PC_CONSTRUCTED,(byte)2);

struct Peer : public ASNObjArrayable
{
    char* name;
    int port;
    char* ip;
    
    ASN1_Encoder* getEncoder()  {
        ASN1_Encoder * r = new ASN1_Encoder();
        
        r->initSequence();
        r->addToSequence(new ASN1_Encoder(name));
        r->addToSequence(new ASN1_Encoder(port));
        r->addToSequence(new ASN1_Encoder(ip, false));
        r->setASN1TypeImplicit(TAG_2);
        return r;
    }
    
    Peer* decode(ASN1_Decoder* d) {
        d = d->getContentImplicit();
        name = d->getSkipString();
        port = d->getSkipIntValue();
        ip = d->getString();
        delete d;
        return this;
    }
    
    byte getASN1Type() {
        return TAG_2;
    }
    
    Peer* instance() {
        return new Peer();
    }
};
