/*
 * ASN1.cpp
 *  Copyright (C) 2018
 *
 *  Created on: Sep 27, 2017
 *      Author: Marius Silaghi <msilaghi@fit.edu>
 *              Florida Tech, Computer Science
 *
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU Affero General Public License as published by
 *     the Free Software Foundation; either the current version of the License, or
 *     (at your option) any later version.
 *
 *    This program is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU Affero General Public License
 *    along with this program; if not, write to the Free Software
 *    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include <stdio.h>
#include "BigInteger.h"
#include "ASN1_Util.h"
#include "ASN1Decoder.h"
#include "ASN1Encoder.h"
char const ASN1_Util::HEX[]={'0','1','2','3','4','5','6','7','8','9',
		'A','B','C','D','E','F'};
const char ASN1_Util::ASN1_EMPTYSTR[] = {0};
const char ASN1_Util::ASN1_SPACESTR[] = {' ',0};
byte* ASNObjArrayable::encode() {
	//System.out.println("will encode: " +this);
	return getEncoder()->getBytes();
}
ASNObjArrayable* ASNObjArrayable::decodeSkip(ASN1_Decoder* dec) {
	ASNObjArrayable* result = decode(dec);
	dec->skipFirstObject();
	return result;
}

int _main(int argc, char ** argv) {
	ASN1_Encoder* seq2 = (new ASN1_Encoder());
	seq2->initSequence();
	ASN1_Encoder * e3 = new ASN1_Encoder((byte)3);
	//printf("Print e3\n");
	//e3->print();
	//printf("Add e3\n");
	seq2->addToSequence(e3);
	//printf("Delete e3 once\n");
	delete e3;
	Calendar* c1;
	c1 = ASN1_Util::CalendargetInstance();
	//printf("Encode c1\n");
	e3 = new ASN1_Encoder(c1);
	//printf("Print e3 twice\n");
	//e3->print();
	//printf("Delete c1\n");
	delete c1;
	seq2->addToSequence(e3);
	//printf("Print seq2\n");
	//seq2->print();
	//printf("Delete e3\n");
	delete e3;

	ASN1_Encoder * my_int = new ASN1_Encoder((byte)124);
	//my_int->print();
	ASN1_Encoder * my_int2 = new ASN1_Encoder("1024");
	//my_int2->print();
	ASN1_Encoder * my_seq = (new ASN1_Encoder())->initSequence();
	my_seq->addToSequence(my_int);
	//my_seq->print();
	my_seq->addToSequence(my_int2); ///?
	//my_seq->print();
	my_seq->addToSequence(seq2);
	my_seq->print();

	//my_seq.print();
	ASN1_Decoder* dec = new ASN1_Decoder(my_seq->getBytes(), my_seq->getBytesNb(), 0);
	printf("Dec = %s\n", dec->toString());
	delete my_seq;
	ASN1_Decoder* dec_c1_1, * dec_c1 = dec->getContent();
	BigInteger* b;
	char* c;
	//printf("Dec c1 = %s\n", c = dec_c1->toString());
	//delete c;
	System_err_println(ERR, "124=: %d\n", (b = (dec_c1_1 = dec_c1->getFirstObject(true))->getInteger())->intValue());
	printf("Dec c1_1 = %s\n", dec_c1_1->toString());
	delete dec_c1_1; delete b;
	dec_c1_1 = dec_c1->getFirstObject(true);
	printf("Dec c1_1 = %s\n", c = dec_c1_1->toString());
	delete c;
	System_err_println(ERR, "1024=: %s\n", (c = dec_c1_1->getString()));
	delete dec_c1_1; delete c;
	ASN1_Decoder * dec_c2_1, * dec_c2 = dec_c1->getContent();
	printf("Dec c2 = %s\n", c = dec_c2->toString());
	System_err_println(ERR, "3=: %d\n", (b = (dec_c2_1=dec_c2->getFirstObjectBorrow(true))->getInteger())->intValue());
	delete dec_c2_1; delete b; delete c;
	System_err_println(ERR, "now=: %s\n", c = (dec_c2_1 = dec_c2->getFirstObjectBorrow(true))->getGeneralizedTime(ASN1_Encoder::TAG_GeneralizedTime));
	delete dec_c2_1; delete c;
	delete dec_c1;
	delete dec_c2;
	delete dec;

	return 0;
}


