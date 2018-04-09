#include <stdio.h>
#include "ASN1_Util.h"
#include "ASN1Decoder.h"
#include "ASN1Encoder.h"

struct UTF8String : public ASNObjArrayable {
    char * str;

    ASN1_Encoder * getEncoder() {
        ASN1_Encoder * r = new ASN1_Encoder();
        r->initSequence();
        //do stuff
        return r;
    }

    UTF8String * decode(ASN1_Decoder * d) {
        d = d->GetContentImplicit();
        //do stuff
        delete d;
        return this;
    }



}

