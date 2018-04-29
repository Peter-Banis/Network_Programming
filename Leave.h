#include <stdio.h>
#include "BigInteger.h"
#include "ASN1_Util.h"
#include "ASN1Decoder.h"
#include "ASN1Encoder.h"

/*
 Leave ::= [APPLICATION 4] EXPLICIT SEQUENCE {name UTF8String}
 */

static const byte TAG_4 = ASN1_Encoder::buildASN1byteType(ASN1_Encoder::CLASS_APPLICATION, ASN1_Encoder::PC_CONSTRUCTED,(byte)4);

struct Leave : public ASNObjArrayable
{
    const char* name;
    
    ASN1_Encoder* getEncoder() {
        ASN1_Encoder * r = new ASN1_Encoder();
        r->initSequence();
        r->addToSequence(new ASN1_Encoder(name, TAG_8));
        r = r->setASN1TypeExplicit(TAG_4);
        return r;
    }
    
    Leave* decode(ASN1_Decoder* d) {
        d = d->getContentExplicit();
        name = d->getString();
        delete d;
        return this;
    }
    
    byte getASN1Type() {
        return TAG_4;
    }
    
    Leave* instance() {
        return new Leave();
    }
};
