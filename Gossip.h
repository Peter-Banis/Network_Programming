#include <stdio.h>
#include "BigInteger.h"
#include "ASN1_Util.h"
#include "ASN1Decoder.h"
#include "ASN1Encoder.h"

/*
 Gossip ::= [APPLICATION 1] EXPLICIT SEQUENCE {sha256hash OCTET STRING, timestamp GeneralizedTime, message UTF8String}
 */

static const byte TAG_1 = ASN1_Encoder::buildASN1byteType(ASN1_Encoder::CLASS_APPLICATION, ASN1_Encoder::PC_CONSTRUCTED,(byte)1);
static const byte TAG_8 = ASN1_Encoder::buildASN1byteType(ASN1_Encoder::CLASS_UNIVERSAL, ASN1_Encoder::PC_PRIMITIVE,ASN1_Encoder::TAG_UTF8String);
static const byte TAG_U4 = ASN1_Encoder::buildASN1byteType(ASN1_Encoder::CLASS_UNIVERSAL, ASN1_Encoder::PC_PRIMITIVE,(byte)4);

struct Gossip : public ASNObjArrayable
{
    unsigned char * sha256hash;
    Calendar* timestamp;
    const char* message;
    
    ASN1_Encoder* getEncoder() {
        ASN1_Encoder * r = new ASN1_Encoder();
        r->initSequence();
        r->addToSequence(new ASN1_Encoder(sha256hash, 44, TAG_U4));
        Calendar* c1;
        c1 = ASN1_Util::CalendargetInstance();
        r->addToSequence(new ASN1_Encoder(c1));
        r->addToSequence(new ASN1_Encoder(message, TAG_8));
        r = r->setASN1TypeExplicit(TAG_1);
        return r;
    }
    
    Gossip* decode(ASN1_Decoder* d) {
        d = d->getContentExplicit();
        sha256hash = d->getSkipBytes();
        timestamp = d->getSkipGeneralizedTimeCalender_();
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
