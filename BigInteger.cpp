/*
 * BigInteger.cpp
 *  Copyright (C) 2018
 *
 *  Created on: Sep 28, 2017
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
#include "BigInteger.h"

BigInteger BigInteger::ZERO ((byte*)NULL, 0, NULL, true);
BigInteger BigInteger::ONE (1, NULL, true);
BigInteger BigInteger::MINUS (-1, NULL, true);

int _main () {
	BigInteger * n = NULL, * m = NULL, * q = new BigInteger(0,""), * r = new BigInteger(0, "");
	char buffer[12];

	m = new BigInteger("2400");
	printf("m = %d  #=%d\n", m->intValue(), m->getBitsNb()); m->dump();
	n = new BigInteger("10");
	printf("n = %d  #=%d\n", n->intValue(), n->getBitsNb()); n->dump();
	m->divide(n, q, r);
	printf("q = %d  #=%d\n", q->intValue(), q->getBitsNb()); q->dump();
	printf("r = %d  #=%d\n", r->intValue(), r->getBitsNb()); r->dump();


	m = new BigInteger("240001000");
	printf("m = %d %s  #=%d\n", m->intValue(), m->toString(buffer, 12), m->getBitsNb()); m->dump();
	n = new BigInteger("240001");
	printf("n = %d  #=%d\n", n->intValue(), n->getBitsNb()); n->dump();
	m->divide(n, q, r);
	printf("q = %d  #=%d\n", q->intValue(), q->getBitsNb()); q->dump();
	printf("r = %d  #=%d\n", r->intValue(), r->getBitsNb()); r->dump();


	int k = 2048000;

	n = NULL;
	n = new BigInteger(k, NULL);
	n->dump();
	printf("n = %d\n", n->intValue()); n->dump();
	n->neg();
	printf("n-neg = %d\n", n->intValue()); n->dump();
	n->shiftRightBits(15);
	printf("n>>1 = %d\n", n->intValue()); n->dump();
	n->shiftLeftBits(15);
	printf("n<<1 = %d %d\n", n->intValue(), n->getSignBit()); n->dump();

	m = new BigInteger(n);
	n->mul(new BigInteger("10"));
	printf("n*10 = %d s=%d\n", n->intValue(), n->getSignBit()); n->dump();
	m->_mul10();
	printf("m = %d\n", m->intValue()); m->dump();
	printf("cmp %d\n", m->cmp(*n));


	n->neg();
	printf("n-2neg = %d\n", n->intValue()); n->dump();
	n->shiftRightBits(15);
	printf("n>>1 = %d\n", n->intValue()); n->dump();
	n->shiftLeftBits(15);
	printf("n<<1 = %d %d\n", n->intValue(), n->getSignBit()); n->dump();


	n = new BigInteger("  -3100");

	n->dump();
	printf("%d\n", n->intValue());
	n->neg();
	n->dump();
	printf("%d\n", n->intValue());
	n->neg();
	n->dump();
	printf("%d\n", n->intValue());

	n->addShiftLeft(7);
	n->dump();
	printf("a = %d\n", n->intValue());

	n = NULL;
	n = new BigInteger(k, NULL);
	n->dump();
	printf("k    = %d\n", n->intValue());

	n = NULL;
	n = new BigInteger(k*128, NULL);
	n->dump();
	printf("k*16 = %d\n", n->intValue());

	n = NULL;
	n = new BigInteger(k*129, NULL);
	n->dump();
	printf("k*17 = %d\n", n->intValue());

	n = new BigInteger("  -1024");
	n->dump();
	printf("%d\n", n->intValue());
	n->extendSafeUsedBytesBy(3);
	n->dump();


	//BigInteger n("0");
	//n.dump();

	return 0;
}
