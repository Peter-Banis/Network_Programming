#include <stdio.h>
#include "ASN1_Util.h"
#include "ASN1Decoder.h"
#include "ASN1Encoder.h"


struct Gossip :  public ASNObjArrayable {
    unsigned char * sha256hash;
    timestamp GeneralizedTime;
    char * message;

    ASN1_Encoder * getEncoder() {
        ASN1_Encoder * r = new ASN1_Encoder();
        r->initSequence();
        r->addToSequence(sha256hash);
        //r->addToSequence(GeneralizedTime); FIX THIS
        r->addToSequence(message, false);
        return r;
    }

    Gossip * decode(ASN1_Decoder * d) {
        sha256hash = d->getSkipString();
        //getTimestamp ok
        message = d->getString();
        delete d;
        return this;
    }


}



