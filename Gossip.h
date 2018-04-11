#include <stdio.h>
#include "BigInteger.h"
#include "ASN1_Util.h"
#include "ASN1Decoder.h"
#include "ASN1Encoder.h"

/*
 Gossip ::= [APPLICATION 1] EXPLICIT SEQUENCE {sha256hash OCTET STRING, timestamp GeneralizedTime, message UTF8String}
 */

ASN1_Encoder * e = new ASN1_Encoder();
static const byte TAG_1 = e->buildASN1byteType(e->CLASS_APPLICATION, e->PC_CONSTRUCTED,(byte)1);

struct Gossip : public ASNObjArrayable
{
    char * sha256hash;
    //char* timestamp;
    char* message;
    
    ASN1_Encoder* getEncoder() {
        ASN1_Encoder * r = new ASN1_Encoder();
        r->initSequence();
        r->addToSequence(new ASN1_Encoder(sha256hash));
        
        /* ---- does not work ----
        ASN1_Encoder * e3 = new ASN1_Encoder((byte)3);
        Calendar* c1;
        c1 = ASN1_Util::CalendargetInstance();
        e3 = new ASN1_Encoder(c1);
        delete c1;
        r->addToSequence(e3);
           ---- does not work ---- */
        
        r->addToSequence(new ASN1_Encoder(message, false));
        return r;
    }
    
    Gossip* decode(ASN1_Decoder* d) {
        d = d->getContentImplicit();
        sha256hash = d->getSkipString();
        message = d->getString();
        delete d;
        return this;
    }
    
    byte getASN1Type() {
        return TAG_1;
    }
    
    Gossip* instance() {
        return new Gossip();
    }
};
