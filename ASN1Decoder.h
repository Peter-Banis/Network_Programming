/*
 * Decoder.h
 *  Copyright (C) 2018
 *
 *  Created on: Jan 3, 2018
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

#ifndef ASN1DECODER_H_
#define ASN1DECODER_H_
#include <stdlib.h>

/**
 * Headers needed for "read"-ing from sockets
 */
#include <sys/types.h>
#include <sys/uio.h>
#include <unistd.h>

#include <stdexcept>
using namespace std;

#include "BigInteger.h"
#include "ASN1_Util.h"
#include "ASN1Encoder.h"

#ifndef _NOEXCEPT
#define _NOEXCEPT throw ()
#endif

class RuntimeException : public exception {
	const char * message;
public:
	RuntimeException(const char* const message) {this->message = message;}
	virtual const char *what() const _NOEXCEPT {return message;}
};

class ASNLenRuntimeException : public exception {
	const char * message;
public:
	ASNLenRuntimeException(const char* const message) {this->message = message;}
	virtual const char *what() const _NOEXCEPT {return message;}
};

class ASN1DecoderFail : public exception {
	const char * message;
public:
	ASN1DecoderFail(const char* const message) {this->message = message;}
	virtual const char *what() const _NOEXCEPT {return message;}
};

class ASN1_Decoder {
private:
	static const bool DEBUG = false;
	static const bool _DEBUG = true;
	/**
	 * The buffer with encoded data. May be NULL if borrowed!
	 */
	byte* data;
	int data_length;
	/**
	 * If data borrowed, do not call delete in destructor
	 */
	bool borrowed;
	/**
	 * The offset in data where the current decoder head is positioned
	 */
	int offset;
	/**
	 * The length of the current component being decoded, after which null is returned even if "data" has more data
	 */
	int length;
public:
	/**
	 * @param data
	 * 		should be not NULL
	 * 	@param keep, if not null, then tells to keep/use the data array "as is" in the new object
	 * 	@borrowed, tells whether to own the data
	 */
	ASN1_Decoder(byte* data, int data_length, void* keep = NULL, bool _borrowed = false): borrowed(_borrowed) {
		if (data == NULL) throw
			("parameter data should not be null");
		init (data, data_length, 0, data_length, keep);
	}
	ASN1_Decoder(byte* data, int data_length, int offset, void* keep = NULL, bool _borrowed = false): borrowed(_borrowed) {
		printf("data=%d off=%d\n", data_length, offset);
		init (data, data_length, offset, data_length - offset, keep);
	}
	ASN1_Decoder(byte* data, int data_length, int offset, int length, void* keep = NULL, bool _borrowed = false): borrowed(_borrowed) {
		init (data, data_length, offset, length, keep);
	}
	ASN1_Decoder(ASN1_Decoder* d, bool _borrowed = false): borrowed(_borrowed) {
		init (d->data, d->data_length, offset, length, NULL);
	}
	~ASN1_Decoder() {
		freeMemory();
	}
	/**
	 * Generate a string with the hex content bytes separated by spaces.
	 * Caller must free result.
	 */
	char* toString() {
		char* result = ASN1_Util::byteToHex(data + offset, length, ASN1_Util::ASN1_SPACESTR, NULL, 0);
		return result;
	}
	/**
	 * Set borrowed structures to NULL, to detect bugs
	 * Any further use (other than destruction or re-borrow) would be a segmentation fault!
	 */
	bool safelyDiscardBorrowed() {
		if (borrowed) {
			data = NULL;
			data_length = 0;
			length = 0;
			offset = 0;
			return true;
		}
		return false;
	}
protected:
	void freeMemory() {
		if (! borrowed) delete[] data;
	}
	// called from constructors, or getFirstObj
	void init(byte* data, int data_length, int offset, int length, void* keep) {
		if (data == NULL) throw
				RuntimeException("parameter data should not be null");
		if (keep) {
			this->data = data;
			this->data_length = data_length;
			this->offset = offset;
			this->length = length;
			return;
		}
		this->data = new byte[this->length = this->data_length = length];
		this->offset = 0;
		memcpy(this->data, data + offset, length);
	}
public:
	char* dumpHex() {
		return ASN1_Util::byteToHex(data, data_length, offset, length, ASN1_Util::ASN1_SPACESTR, NULL, 0);
	}
	char* dumpHex(int header) {
		return ASN1_Util::byteToHex(data, data_length, offset, header, ASN1_Util::ASN1_SPACESTR, NULL, 0);
	}
	void printHex(char* name) {
		char * s;
		System_out_println("<Decoder::printHex> %s %s\n", name, s = dumpHex());
		delete s;
	}
	/**
	 * Returns the first byte of the type
	 * @return
	 */
	byte type () {
		if (length <= 0) return 0;
		return data[offset];
	}
	/**
	 * Returns the first byte of the type by a call to type().
	 * @return
	 */
	byte getTypeByte() {
		return type();
	}
	/**
	 * If no data here
	 * @return
	 */
	bool isEmpty() {
		return ((data == NULL) || (length <= 0));
	}
	/**
	 * Returns the tag, 0x1f in case there are more bytes!
	 * @return
	 */
	int tagVal() {
		if (length <= 0) return 0;
		return data[offset] & 0x1f;
	}
	/**
	 * Returns the class value, which can be one of:
	 * Encoder.CLASS_APPLICATION,
	 * Encoder.CLASS_CONTEXT,
	 * Encoder.CLASS_PRIVATE,
	 * Encoder.CLASS_UNIVERSAL
	 * @return
	 */
	int typeClass() {
		if (length <= 0) return 0;
		return (data[offset] & 0xc0) >> 6;
	}
	/**
	 * Returns the type of the object which can be one of:
	 *  Encoder.PC_CONSTRUCTED,
	 *  Encoder.PC_PRIMITIVE
	 * @return
	 */
	int typePC () {
		if (length <= 0) return 0;
		return (data[offset] & 0x20) >> 5;
	}

	/**
	 * Can use tagVal() if it is less than 31;
	 * @return
	 */
	BigInteger* getTagValueBN() {
		int len = typeLen();
		if (len == 1)
			return new BigInteger(tagVal(), "");
		return ASN1_Util::fromBase128(data, data_length, offset + 1, offset + length);
	}
	bool hasType(int ASN1class, int ASN1type, BigInteger* tag) {
		BigInteger* bn;
		bool result = true;
		if (this->typeClass() != ASN1class) return false;
		if (this->typePC() != ASN1type) return false;
		if (! (bn = this->getTagValueBN())->equals(*tag)) result = false;
		delete bn;
		return result;
	}
protected:
	int typeLen() {
		if (length <= 0) return 0;
		if (tagVal() != 0x1f) return 1;
		int k = offset+1;
		int len = 2;
		while ((k < length+offset) && (ASN1_Util::getUnsignedShort(data[k]) > 127)) {len++; k++;}
		if (len > length) return 0;
		return len;
	}
	int contentLength() {
		if (length < 2) {
			return 0;
		}
		int tlen = typeLen();
		if (tlen < 1) {
			return 0;
		}
		if (ASN1_Util::getUnsignedShort(data[offset+tlen]) < 128) {
			return data[offset+tlen];
		}
		int len_len = ASN1_Util::getUnsignedShort(data[offset+tlen])-128;
		if (length < len_len+tlen+1){
			return 0;
		}
		byte* len = new byte[len_len];
		ASN1_Encoder::copyBytesRev(len_len, len, 0, data_length, data, len_len, offset+tlen+1);
		BigInteger bi(len, len_len, len);
		int ilen = bi.intValue();
		return ilen;
	}
	int lenLen() {
		int tlen = typeLen();
		if (length < 2) return 0;
		if (ASN1_Util::getUnsignedShort(data[offset+tlen]) < 128) return 1;
		return 1 + ASN1_Util::getUnsignedShort(data[offset+tlen]) - 128;
	}
	/**
	 *
	 * @return -1 : not enough bytes in buffer to determine type and length of length
	 * 			-k: not enough bytes (k-1 needed) to determine length of length
	 * 		   larger than 0: returns length of the buffer needed to store the whole first ASN1 object
	 *
	 */
	 int objectLen() {
		int type_len = typeLen();
		if ((type_len == 0) || (type_len>=length)) return -1; // insufficient to find
		int len_len = lenLen();
		if((len_len==0) || (len_len+type_len>length)) return -(type_len+len_len+1);
		int content_len = contentLength();
		return type_len+len_len+content_len;
	}
public:
		/**
		 * @param extract  Changes this to get rid of the first Object
		 * @return A new Decoder Object for the first in list
		 */
		ASN1_Decoder* getFirstObjectBorrow(bool extract, ASN1_Decoder * result = NULL) {
			return getFirstObject(extract, true, result);
		}
		ASN1_Decoder* getFirstObjectInPlace() {
			if (length <= 0) return this;
			int tlen = typeLen();
			int llen = lenLen();
			int cLen = contentLength();
			int new_len = tlen + llen + cLen;
			if (new_len > length || new_len < 0) throw RuntimeException (
					"ASN1:Decoding:Invalid object length: Too long given available data:");
			length = new_len;
			return this;
		}
		/**
		 * Skip over the first element if present.
		 * Returns NULL is no element present, otherwise returns "this" modified to skip
		 */
		ASN1_Decoder* getFirstObjectSkip() {
			if (length <= 0) return NULL;
			int tlen = typeLen();
			int llen = lenLen();
			int cLen = contentLength();
			int new_len = tlen + llen + cLen;
			if (new_len > length || new_len < 0) {
				fprintf(stderr,"getFirstObjectSkip: Invalid next skip object:\n containerOffset=%d, containerLength=%d,\n objTypeLength=%d, objLength=%d, objContentLength=%d\n%s\n",
						offset, length, tlen, llen, cLen, this->toString());
				throw RuntimeException
					("ASN1:Decoding:Invalid skipped object length: Too long given available data");
			}
			offset += new_len;
			length -= new_len;
			if (offset < 0 || length < 0) throw RuntimeException
				("Invalid length: Arrive at negative offset/length after extracting");

			return this;
		}
		ASN1_Decoder* skipFirstObject() {
			return getFirstObjectSkip();
		}
		/**
		 * The parameter borrow, if set, shares the underlying data vector with the result.
		 * and configures the result to not delete it upon deletion (keeping ownership in this).
		 *
		 * Result (if not NULL) will be initialized with the result object data
		 */
		ASN1_Decoder* getFirstObject(bool extract, bool borrow = false, ASN1_Decoder * result = NULL) {
			if (length <= 0) return NULL;
			int tlen = typeLen();
			int llen = lenLen();
			int cLen = contentLength();
			int new_len = tlen + llen + cLen;
			if (new_len > length || new_len < 0) {
				fprintf(stderr,"getFirstObject: Invalid next object:\n containerOffset=%d, containerLength=%d,\n objTypeLength=%d, objLength=%d, objContentLength=%d\n%s\n",
						offset, length, tlen, llen, cLen, this->toString());
				throw RuntimeException
					("getFirstObject: ASN1:Decoding:Invalid object length: Too long given available data:");
			}
			int old_offset = offset;
			if (extract) {
				offset += new_len;
				length -= new_len;
				if (offset < 0 || length < 0) throw RuntimeException
					("getFirstObject: Invalid data: Arrive at negative offset after extracting");
			}
			if (result) {
				result->freeMemory();
				result->borrowed = borrow;
				result->init(data, data_length, old_offset, new_len, borrow?data:NULL);
				return result;
			}
			return new ASN1_Decoder(data, data_length, old_offset, new_len, borrow?data:NULL, borrow);
		}
		ASN1_Decoder* getFirstObjectBorrow(bool extract, byte type, ASN1_Decoder * result = NULL) {
			return getFirstObject(extract, type, false, result);
		}
		/**
		 * @param extract  Changes this to get rid of the first Object
		 * @return A new Decoder Object for the first in list, or NULL if none is present
		 * @throws ASN1DecoderFail if the type of the first object is not the same as "type" parameter
		 *
		 */
		ASN1_Decoder* getFirstObject(bool extract, byte type, bool borrow = false, ASN1_Decoder * result = NULL) {
			if (length <= 0) return NULL;
			byte found = getTypeByte();
			if ((found != type) && (getTypeByte() != ASN1_Encoder::TAG_NULL))
				throw ASN1DecoderFail
				("No type: but in");
			int tlen = typeLen();
			int llen = lenLen();
			int cLen = contentLength();
			int new_len = tlen + llen + cLen;
			if (new_len > length || new_len < 0) throw ASNLenRuntimeException
					("Too long");
			int old_offset = offset;
			if (extract) {
				offset += new_len;
				length -= new_len;
				if (offset < 0 || length < 0) throw ASNLenRuntimeException
						("Arrive at negative offset after extracting");
			}
			if (result) {
				result->freeMemory();
				result->borrowed = borrow;
				result->init(data, data_length, old_offset, new_len, borrow?data:NULL);
				return result;
			}
			return new ASN1_Decoder(data, data_length, old_offset, new_len, borrow?data:NULL, borrow);
		}
		/**
		 * Get the first object of source into this, borrowing;
		 */
		inline
		ASN1_Decoder* loadFirstObjectBorrow(ASN1_Decoder* source, bool extract) {
			source->getFirstObjectBorrow(extract, this);
			return this;
		}
		inline
		ASN1_Decoder* loadContentImplicitBorrow(ASN1_Decoder* source) {
			source->getContentImplicitBorrow(this);
			return this;
		}
		/**
		 * Default is to consider the tag was implicit.
		 * @return
		 * @throws ASN1DecoderFail
		 */
		inline
		ASN1_Decoder* getContent(ASN1_Decoder* result = NULL) {
			return getContentImplicit(false, false, result);
		}
		inline
		ASN1_Decoder* getContentInPlace() {
			return getContentImplicitInPlace();
		}
		inline
		ASN1_Decoder* getContentBorrow(ASN1_Decoder* result = NULL) {
			return getContentImplicitBorrow(result);
		}
		inline
		ASN1_Decoder* getContentImplicitInPlace() {
			return getContentImplicit(true);
		}
		/*
		 * A new decoder object but sharing the data vector from parent,
		 * Parent will free it
		 */
		inline
		ASN1_Decoder* getContentImplicitBorrow(ASN1_Decoder* result = NULL) {
			return getContentImplicit(false, true, result);
		}
		/**
		 * By default, lets this unchanged and returns a new Decoder.
		 * If borrow, keep data array and do not delete it in destructor
		 * @return
		 * @throws ASN1DecoderFail
		 */
		ASN1_Decoder* getContentImplicit(bool inplace = false, bool borrow = false, ASN1_Decoder* result = NULL) {
			//int DEBUG = 1;
			if (DEBUG) System_err_println(ERR, "getContent: Length: %d\n", length);
			if (length <= 0) throw ASNLenRuntimeException("Container length 0");
			int new_len;
			new_len = contentLength();
			if (DEBUG) System_err_println(ERR, "getContent: new_Length: %d\n", new_len);
			int new_off = typeLen()+lenLen();
			if (new_off > length) throw ASN1DecoderFail("Content exceeds container");
			if (new_off < 0) throw ASN1DecoderFail("Content has negative offset in container length");
			if (new_off + new_len > length) throw ASN1DecoderFail("Content exceeds container");
			if (new_len > length) throw ASN1DecoderFail("Too long");
			if (new_len < 0) throw ASN1DecoderFail("Negative length");
			if (inplace) {
				offset += new_off;
				length = new_len;
				return this;
			}
			if (result) {
				result->freeMemory();
				result->borrowed = borrow;
				result->init(data, data_length, offset + new_off, new_len, borrow?data:NULL);
				return result;
			}
			return new ASN1_Decoder(data, data_length, offset + new_off, new_len, borrow?data:NULL, borrow);
		}
		/**
		 *
		 * @return
		 * @throws ASN1DecoderFail
		 */
		inline
		ASN1_Decoder* getContentExplicit(bool inplace = false) {
			ASN1_Decoder* d = this->getContentImplicit(inplace);
			ASN1_Decoder* e = d->getFirstObjectInPlace();
			ASN1_Decoder* f = e->getContentImplicitInPlace();
			return f;
		}
		/**
		 * Just remove (i.e. skip) an explicit tag, leaving the (default) implicit one.
		 *
		 * If inplace, the decoder focuses on the content only.
		 * @return
		 * @throws ASN1DecoderFail
		 */
		inline
		ASN1_Decoder* removeExplicitASN1Tag(bool inplace = false) {
			//printf("remove explicit\n");
			ASN1_Decoder* d = this->getContentImplicit(inplace);
			//printf("removed explicit\n");
			ASN1_Decoder* r = d->getFirstObjectInPlace();
			return r;
		}
		/**
		 * Just remove (i.e. skip) an explicit tag, leaving the (default) implicit one.
		 *
		 * the decoder focuses on the content only.
		 * @return
		 * @throws ASN1DecoderFail
		 */
		inline
		ASN1_Decoder* removeExplicitASN1TagInplace() {
			ASN1_Decoder* d = this->getContentImplicit(true);
			ASN1_Decoder* r = d->getFirstObjectInPlace();
			return r;
		}
		bool getBoolean() {
			if (length <= 0) throw ASNLenRuntimeException("Boolean length");
			return data[offset+2] != 0;
		}
		bool getBoolean(byte type) {
			if (length <= 0) throw ASNLenRuntimeException("Boolean length");
			if (data[offset] != type) throw ASN1DecoderFail("Wrong boolean type");
			return data[offset+2] != 0;
		}
		bool getSkipBoolean() {
			bool result = getBoolean();
			skipFirstObject();
			return result;
		}
		bool getSkipBoolean(byte type) {
			bool result = getSkipBoolean();
			skipFirstObject();
			return result;
		}
		int getIntValue() {
			int val;
			BigInteger * bi = getInteger();
			val = bi->intValue();
			delete bi;
			return val;
		}
		BigInteger* getInteger() {
			int value_length;
			byte* value = new byte[value_length = contentLength()];
			ASN1_Encoder::copyBytes(value_length, value, 0, data_length, data, value_length, offset+typeLen()+lenLen());
			BigInteger* result = new BigInteger(value, value_length, value);
			return result;
		}
		BigInteger* getInteger(byte tagInteger) {
			if (getTypeByte() == ASN1_Encoder::TAG_NULL) return NULL;
			if (data[offset] != tagInteger) throw ASN1DecoderFail("Wrong tag");
			int value_length;
			byte* value = new byte[value_length = contentLength()];
			ASN1_Encoder::copyBytes(value_length, value, 0, data_length, data, value_length, offset+typeLen()+lenLen());
			BigInteger* result =  new BigInteger(value, value_length, value);
			return result;
		}
		int getSkipIntValue() {
			int result = getIntValue();
			skipFirstObject();
			return result;
		}
		BigInteger* getSkipInteger() {
			BigInteger* result = getInteger();
			skipFirstObject();
			return result;
		}
		BigInteger* getSkipInteger(byte tagInteger) {
			BigInteger* result = getInteger(tagInteger);
			skipFirstObject();
			return result;
		}
		/**
		 * For OCTETSTRING
		 * @return
		 */
		byte* getBytes() {
			if (!((data[offset]==ASN1_Encoder::TAG_OCTET_STRING) ||
					(data[offset]==ASN1_Encoder::TAG_BIT_STRING) ||
					(data[offset]==ASN1_Encoder::TAG_NULL))) {
			}
			if (getTypeByte() == ASN1_Encoder::TAG_NULL) return NULL;
			int value_length;
			byte* value = new byte[value_length = contentLength()];
			ASN1_Encoder::copyBytes(value_length, value, 0, data_length, data, value_length, offset+typeLen()+lenLen());
			return value;
		}
	byte* getBytesAnyType() {
		int value_length;
		byte* value = new byte[value_length = contentLength()];
		ASN1_Encoder::copyBytes(value_length, value, 0, data_length, data, value_length, offset+typeLen()+lenLen());
		return value;
	}
	byte* getBytes(byte type) {
		if (getTypeByte() == ASN1_Encoder::TAG_NULL) return NULL;
		if (data[offset] != type) throw ASN1DecoderFail
				("OCTET STR: type != data[offset]");
		int value_length = contentLength();
		byte* value = new byte[value_length];
		ASN1_Encoder::copyBytes(value_length, value, 0, data_length, data, value_length, offset + typeLen() + lenLen());
		return value;
	}
	byte* getSkipBytes() {
		byte* result = getBytes();
		skipFirstObject();
		return result;
	}
	byte* getSkipBytesAnyType() {
		byte* result = getBytesAnyType();
		skipFirstObject();
		return result;
	}
	byte* getSkipBytes(byte type) {
		byte* result = getBytes(type);
		skipFirstObject();
		return result;
	}
	byte* getSkipBitString_AnyType(byte* bits_padding, int bits_padding_length) {
		byte* result = getBitString_AnyType(bits_padding, bits_padding_length);
		skipFirstObject();
		return result;
	}
	/**
	 * The number of padding bits is stored in the parameter if this has at least one byte;
	 * @param bits_padding
	 * @return
	 */
	byte* getBitString_AnyType(byte* bits_padding, int bits_padding_length) {
		int value_length = contentLength() - 1;
		byte* value = new byte[value_length];
		ASN1_Encoder::copyBytes(value_length, value, 0, data_length, data, value_length, offset + typeLen() + lenLen() + 1);
		if ((bits_padding != NULL) && (bits_padding_length > 0))
			bits_padding[0] = data[offset + typeLen() + lenLen()];
		return value;
	}
	int* getOID(byte type) {
		if (data[offset] != type) throw ASN1DecoderFail
				("OCTET STR: type != data[]");
		int b_value_length = contentLength();
		byte* b_value = new byte[b_value_length];
		ASN1_Encoder::copyBytes(b_value_length, b_value, 0, data_length, data, b_value_length, offset+typeLen()+lenLen());
		int len = 2;
		for (int k = 1; k < b_value_length; k ++) if (b_value[k] >= 0) len ++;
		int* value = new int[len];
		value[0] = get_u32(b_value[0])/40;
		value[1] = get_u32(b_value[0])%40;
		for (int k = 2, crt = 1; k < len; k ++) {
			value[k]=0;
			while (b_value[crt] < 0) {
				value[k] <<= 7;
				value[k] += b_value[crt++]+128;//get_u32(b_value[crt++])-128;
			}
			value[k] <<= 7;
			value[k] += b_value[crt++];
			continue;
		}
		return value;
	}
	BigInteger** getBNOID(byte type) {
		if (data[offset] != type) throw ASN1DecoderFail
				("OCTET STR: type != data[offset]");
		int b_value_length;
		byte* b_value = new byte[b_value_length = contentLength()];
		ASN1_Encoder::copyBytes(b_value_length, b_value, 0, data_length, data, b_value_length, offset+typeLen()+lenLen());
		int len = 2;
		for (int k = 1; k < b_value_length; k ++) if (b_value[k] >= 0) len ++;
		BigInteger** value = new BigInteger*[len];
		int value0 = get_u32(b_value[0]) / 40;
		int value1 = get_u32(b_value[0]) % 40;
		if (value0 > 2) {
			value1 += 40 * (value0 - 2);
			value0 = 2;
		}
		value[0] = new BigInteger(value0, "");
		value[1] = new BigInteger(value1, "");
		for (int k = 2, crt = 1; k < len; k ++) {
			value[k] = & BigInteger::ZERO;
			while (b_value[crt] < 0) {
				value[k]->shiftLeft(7);
				value[k]->or_bits(((b_value[crt ++] + 128)));
			}
			value[k]->shiftLeft(7);
			value[k]->or_bits(((b_value[crt ++])));
			continue;
		}
		return value;
	}
	int* getSkipOID(byte type) {
		int* result = getOID(type);
		skipFirstObject();
		return result;
	}
	BigInteger ** getSkipBNOID(byte type) {
		BigInteger ** result = getBNOID(type);
		skipFirstObject();
		return result;
	}
	static int get_u32 (byte val) {
		if (val >= 0) return val;
		return 256 + (int) val;
	}
	byte* getAny () {
		int value_length;
		byte* value = new byte[value_length = contentLength()];
		ASN1_Encoder::copyBytes(value_length, value, 0, data_length, data, value_length, offset+typeLen()+lenLen());
		return value;
	}
	double getReal() {
		int value_length;
		byte* value = new byte[value_length = contentLength()];
		ASN1_Encoder::copyBytes(value_length, value, 0, data_length, data, value_length, offset+typeLen()+lenLen());
		return atof((const char*)value);
	}
	char* getString() {
		if (getTypeByte() == ASN1_Encoder::TAG_NULL) return NULL;
		int value_length;
		byte* value = new byte[(value_length = contentLength()) + 1];
		ASN1_Encoder::copyBytes(value_length, value, 0, data_length, data, value_length, offset+typeLen()+lenLen());
		value[value_length] = 0;
		return (char*) value;
	}
	char* getString(byte type) {
		if (getTypeByte() == ASN1_Encoder::TAG_NULL) return NULL;
		if (data[offset] != type) throw ASN1DecoderFail
				("String: exp=type != in= data[offset]");
		int value_length;
		byte* value = new byte[value_length = contentLength()];
		ASN1_Encoder::copyBytes(value_length, value, 0, data_length, data, value_length, offset+typeLen()+lenLen());
		return (char*)value;
	}
	char* getStringAnyType() {
		if (getTypeByte() == ASN1_Encoder::TAG_NULL) return NULL;
		int value_length;
		byte* value = new byte[value_length = contentLength()];
		ASN1_Encoder::copyBytes(value_length, value, 0, data_length, data, value_length, offset+typeLen()+lenLen());
		return (char*)value;
	}
	char* getSkipString() {
		char* result = getString();
		skipFirstObject();
		return result;
	}
	char* getSkipStringAnyType() {
		char* result = getStringAnyType();
		skipFirstObject();
		return result;
	}
	char* getSkipString(byte type) {
		char* result = getString(type);
		skipFirstObject();
		return result;
	}
	/**
	 * Any type (assume type was previously tested)
	 * @return
	 * @throws ASN1DecoderFail
	 */
	char* getGeneralizedTimeAnyType() {
		int cLength = contentLength();
		if (cLength == 0) return NULL;
		char* value = new char[cLength + 1];
		ASN1_Encoder::copyBytes(cLength, (byte*)value, 0, data_length, data, cLength, offset+typeLen()+lenLen());
		value[cLength] = 0;
		return value;
	}
	char* getGeneralizedTime(byte type) {
		if (getTypeByte() == ASN1_Encoder::TAG_NULL) return NULL;
		if (data[offset] != type) throw ASN1DecoderFail
				("generalizedTime: type != data[offset]");
		return getGeneralizedTimeAnyType();
	}
	/**
	 *  Only works assuming type = Encoder.TAG_GeneralizedTime
	 * @return
	 * @throws ASN1DecoderFail
	 */
	char* getGeneralizedTime_() {
		return getGeneralizedTime(ASN1_Encoder::TAG_GeneralizedTime);
	}
	char* getSkipGeneralizedTimeAnyType() {
		char* result = getGeneralizedTimeAnyType();
		skipFirstObject();
		return result;
	}
	char* getSkipGeneralizedTime(byte type) {
		char* result = getGeneralizedTime(type);
		skipFirstObject();
		return result;
	}
	char* getSkipGeneralizedTime_() {
		char* result = getGeneralizedTime_();
		skipFirstObject();
		return result;
	}
	Calendar* getGeneralizedTimeCalender(byte type) {
		char* time;
		Calendar* result = ASN1_Util::getCalendar(time = this->getGeneralizedTime(type));
		delete time;
		return result;
	}
	/**
	 * Only works assuming type = Encoder.TAG_GeneralizedTime
	 * @return
	 * @throws ASN1DecoderFail
	 */
	Calendar* getGeneralizedTimeCalender_() {
		char* time;
		Calendar* result = ASN1_Util::getCalendar(time = this->getGeneralizedTime(ASN1_Encoder::TAG_GeneralizedTime));
		delete time;
		return result;
	}
	/**
	 * Only works assuming type previously tested
	 * @return
	 * @throws ASN1DecoderFail
	 */
	Calendar* getGeneralizedTimeCalenderAnyType() {
		char* time;
		Calendar*  result = ASN1_Util::getCalendar(time = this->getGeneralizedTimeAnyType());
		delete time;
		return result;
	}
	Calendar* getSkipGeneralizedTimeCalender(byte type) {
		Calendar* result = getGeneralizedTimeCalender(type);
		skipFirstObject();
		return result;
	}
	Calendar* getSkipGeneralizedTimeCalender_() {
		Calendar* result = getGeneralizedTimeCalender_();
		skipFirstObject();
		return result;
	}
	Calendar* getSkipGeneralizedTimeCalenderAnyType() {
		Calendar* result = getGeneralizedTimeCalenderAnyType();
		skipFirstObject();
		return result;
	}
	/**
	 * Currently not expanding the buffer but rather abandon if too small.
	 * Also returns false on end of stream.
	 * @param is
	 * @return
	 * @throws IOException
	 */
	bool fetchAll(int is) {
		while (true) {
			int asrlen = objectLen();
			if ((asrlen > 0) && (asrlen > data_length - offset)) {
				return false; // not enough space
			}
			if ((asrlen < 0) || (length < asrlen)) {
				if (length == data_length - offset) return false; // at end
				int inc = read(is, data + data_length-length, length);
				if (inc <= 0) return false;
				length += inc;
				continue;
			}
			break; // enough data
		}
		return true;
	}
	int getMSGLength() {
		return length;
	}
    char* dumpHexDump() {
        return dumpHex(ASN1_Util::MAX_ASN1_DUMP);
    }
    /**
     * Returns an array of ints
     * @return
     */
	int* getIntsArray() {
		if (this->getTypeByte() == ASN1_Encoder::TAG_NULL) return NULL;
		ASN1_Decoder* dec_count, * dec_content;
		try {
			dec_count = this->getContent();
			dec_content = this->getContent();
		} catch (char* e) {
			return NULL;
		}
		BigInteger** ints;
		int count = 0;
		for (;;) {
			ASN1_Decoder* val = dec_count->getFirstObject(true);
			if (val == NULL) break;
			count ++;
			delete val;
		}
		delete dec_count;
		ints = new BigInteger*[count];
		count = 0;
		for (;;) {
			ASN1_Decoder* val = dec_content->getFirstObject(true);
			if (val == NULL) break;
			ints[count ++] = (val->getInteger());
			delete val;
		}
		delete dec_content;
		int* result = new int[count];
		count = 0;
		for (int k = 0; k < count; k++) {
			result[k] = ints[k]->intValue();
			delete ints[k];
		}
		delete ints;
		return result;
	}
	BigInteger** getBNIntsArray() {
		if (this->getTypeByte() == ASN1_Encoder::TAG_NULL) return NULL;
		ASN1_Decoder* dec_count;
		ASN1_Decoder* dec_content;
		try {
			dec_count = this->getContent();
			dec_content = this->getContent();
		} catch (char* e) {
			return NULL;
		}
		BigInteger** ints;
		int count = 0;
		for (;;) {
			ASN1_Decoder* val = dec_count->getFirstObject(true);
			if (val == NULL) break;
			count ++;
			delete val;
		}
		delete dec_count;
		ints = new BigInteger*[count];
		count = 0;
		for (;;) {
			ASN1_Decoder* val = dec_content->getFirstObject(true);
			if (val == NULL) break;
			ints[count ++] = (val->getInteger());
			delete val;
		}
		delete dec_content;
		return ints;
	}
	float* getFloatsArray() {
		if (this->getTypeByte() == ASN1_Encoder::TAG_NULL) return NULL;
		ASN1_Decoder* dec_count, * dec_content;
		try {
			dec_count = this->getContent();
			dec_content = this->getContent();
		} catch (char* e) {
			return NULL;
		}
		char** strings;
		int count = 0;
		for (;;) {
			ASN1_Decoder* val = dec_count->getFirstObject(true);
			if (val == NULL) break;
			count ++;
			delete val;
		}
		delete dec_count;
		strings = new char*[count];
		count = 0;
		for(;;) {
			ASN1_Decoder* val = dec_content->getFirstObject(true);
			if (val == NULL) break;
			strings[count ++] = (val->getString());
			delete val;
		}
		float* result = new float[count];
		for (int k = 0; k < count; k ++) {
			result[k] = atof(strings[k]);
		}
		delete strings;
		delete dec_content;
		return result;
	}
	static void encodeDecodeCalendar() {
	        Calendar* cal = Calendar::getInstance();

	        ASN1_Encoder* enc = new ASN1_Encoder(cal);
	        ASN1_Decoder* dec = new ASN1_Decoder(enc->getBytes(), enc->getBytesNb(), enc);
	        ASN1_Decoder* res = dec->getFirstObject(true);
	        Calendar* resultat = res->getGeneralizedTimeCalenderAnyType();

	        int m1 = cal->get(Calendar::MONTH);
	        int m2 = resultat->get(Calendar::MONTH);
	        System_out_println("Compared: %d vs %d\n", m1, m2);
	        delete enc;
	        delete dec;
	        delete res;
	        delete resultat;
	}
	/**
	 * compares the parameter tag with the result of getTypeByte() when there is a next object (peeked)
	 * @param tag
	 * @return
	 */
	bool isFirstObjectTagByte(byte tag) {
		ASN1_Decoder* d1, *d2 = NULL;
		bool b;
		b = ((d1 = getFirstObject(false)) != NULL) && ((d2 = getFirstObject(false))->getTypeByte() == tag);
		delete d1;
		if (d2) delete d2;
		return b;
	}
	/**
	 * Peeks the next object and compares to null
	 * @return
	 */
	bool isEmptyContainer() {
		bool result;
		ASN1_Decoder* tmp;
		result = ((tmp = getFirstObject(false)) == NULL);
		delete tmp;
		return result;
	}
	/**
	 * Test example.
	 * @param nb
	 * @throws ASN1DecoderFail
	 */
	static void encodeDecodeBNTAG(char * nb) {
		byte* msg;
		BigInteger* bn;
		ASN1_Encoder* e1, * enc = new ASN1_Encoder();
		enc->initSequence();
		enc->addToSequence (e1 = new ASN1_Encoder(bn = new BigInteger("1")));
		delete bn;
		delete e1;

		msg = enc->getBytes();

		enc->setASN1Type(ASN1_Encoder::CLASS_CONTEXT, ASN1_Encoder::PC_CONSTRUCTED, bn = new BigInteger(nb,16));
		delete bn;

		msg = enc->getBytes();

		ASN1_Decoder* dec = new ASN1_Decoder(msg, enc->getBytesNb(), msg);
		char*s;
		System_out_println("BN: decoder %s\n", s = dec->toString());
		delete s;

		ASN1_Decoder* d = dec->getContent();

		ASN1_Decoder* fo = d->getFirstObject(true);
		BigInteger* bi = fo->getInteger();
		System_out_println("BN: integer %d", bi->intValue());
		delete fo;
		delete d;
	}
	int getSequenceCountOf() {
	  ASN1_Decoder* d = this;
	  ASN1_Decoder* dec = d->getContentImplicit();
	  int n = 0;
	  for ( ; ; ) {
	    if (dec->getTypeByte() == 0) break;
	    dec->skipFirstObject(); n ++;
	  }
	  delete dec;
	  return n;
	}
	/**
	 * @param: ASN1 type byte  of the objects
	 * @param: instance  an instance of the object to be created by decoding
	 * @param: free_instance set to true to delete the instance after the use
	 * @param: count as result, the number of extracted object is stored in this variable, if not NULL
	 * @param: inplace Consuming the input
	// bool isType(Class<? extends ASNObjArrayable> c) {} // test if type is for some ASNObj class
	 *
	 * type not used
	 * By default, decoding done in place. Allocation performed for counting objects.
	 *
	 */
	ASNObjArrayable ** getSequenceOf(byte type, ASNObjArrayable* instance, int * count = NULL, bool free_instance=false, bool inplace=true) {
	  ASN1_Decoder* d = this;
	  //printf("getSequenceOf: start:\n%s\n", d->toString());
	  ASN1_Decoder* dec = d->getContentImplicit();
	  //printf("getSequenceOf: implicit off:\n%s\n", dec->toString());
	  int n = 0;
	  for ( ; ; ) {
		if (dec->getTypeByte() == 0) break;
		if (type != 0 && dec->getTypeByte() != type) throw ASN1DecoderFail("Wrong type item");
		//printf("getSequenceOf: type=%x n=%d\n", dec->getTypeByte(), n);
	    dec->skipFirstObject(); n ++;
		//printf("getSequenceOf: skipped:\n%s\n", dec->toString());
	  }
	  delete dec;
	  //printf("getSequenceOf: count:\n%d\n", n);
	  ASNObjArrayable ** result = new ASNObjArrayable*[n];
	  d->getContentImplicit(inplace);
	  //printf("getSequenceOf: inplace implicit off:\n%s\n", d->toString());
	  for ( int i = 0 ;  ; i ++ ) {
	    if (d->getTypeByte() == 0) break;
	    ASNObjArrayable * crt = instance->instance();
	    crt->decode(d);
	    result[i] = crt;
	    d->skipFirstObject();
	  }
	  //printf("getSequenceOf: end of story:\n%s\n", d->toString());
	  if (count != NULL) *count = n;
	  //printf("getSequenceOf: count=:\n%d\n",n);
	  if (free_instance) delete instance;
	  //printf("getSequenceOf: done\n");
	  return result;
	}
	/**
	 * @param: instance  an instance of the object to be created by decoding
	 * @param: free_instance set to true to delete the instance after the use
	 * @param: count as result, the number of extracted object is stored in this variable, if not NULL
	 *
	 * The expected ASN1 tag is set to 0 (any tag)
	 */
	ASNObjArrayable ** getSequenceOf(ASNObjArrayable* instance, int * count = NULL, bool free_instance=false) {
		return getSequenceOf(0, instance, count, free_instance);
	}
	/**
	 * @param: instance  an instance of the object to be created by decoding
	 * @param: free_instance set to true to delete the instance after the use
	 * @param: count as result, the number of extracted object is stored in this variable, if not NULL
	 *
	 * The expected ASN1 tag is set to 0 (any tag)
	 */
	ASNObjArrayable ** getSkipSequenceOf(ASNObjArrayable* instance, int * count = NULL, bool free_instance=false) {
		ASNObjArrayable ** result = getSequenceOf(instance, count, free_instance);
		skipFirstObject();
		return result;
	}
	/**
	 * @param: ASN1 type byte  of the objects
	 * @param: instance  an instance of the object to be created by decoding
	 * @param: free_instance set to true to delete the instance after the use
	 * @param: count as result, the number of extracted object is stored in this variable, if not NULL
	 */
	ASNObjArrayable ** getSkipSequenceOf(byte type, ASNObjArrayable* instance, int * count = NULL, bool free_instance=false) {
		ASNObjArrayable ** result = getSequenceOf(type, instance, count, free_instance);
		skipFirstObject();
		return result;
	}
};


#endif /* ASN1DECODER_H_ */
