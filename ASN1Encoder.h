/*
 * Encoder.h
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

#ifndef ASN1ENCODER_H_
#define ASN1ENCODER_H_
#include "BigInteger.h"
#define LENGTH_GENERALIZED_TIME_NULL_TERMINATED 20

struct ASN1_Encoder;
struct ASN1_Decoder;
struct ASNObjArrayable {
	virtual ~ASNObjArrayable(){};
	virtual ASN1_Encoder* getEncoder() = 0;
	byte* encode();
	virtual ASNObjArrayable* decode(ASN1_Decoder* dec) = 0;
	ASNObjArrayable* decodeSkip(ASN1_Decoder* dec);

	virtual ASN1_Encoder* getEncoder(char** dictionary_GIDs) {return getEncoder();}
	/**
	 * Must be implemented whenever this object is encoded in a sequence (array/list)
	 * @return
	 * @throws CloneNotSupportedException
	 */
	virtual ASNObjArrayable* instance() = 0;
	/**
	 *
	 * @param dictionary_GIDs
	 * @param dependants : pass 0 for no dependents (ASNObj.DEPENDANTS_NONE)
	 *    pass -1 for DEPENDANTS_ALL.
	 *    Any positive number is decremented at each level.
	 *
	 *    Other custom schemas can be defined using remaining negative numbers.
	 * @return
	 */
	virtual ASN1_Encoder* getEncoder(char** dictionary_GIDs, int dependants) {
		return getEncoder(dictionary_GIDs);}

};
struct ASNObj : ASNObjArrayable {
	static const int DEPENDANTS_NONE = 0;
	static const int DEPENDANTS_ALL = -1;
	/**
	 * Must be implemented whenever this object is encoded in a sequence (array/list)
	 * @return
	 * @throws CloneNotSupportedException
	 */
	ASNObj* instance() { return NULL; }
};

struct ASN1_Encoder {
	/**
	 * Number of bytes used by this encoder (the length of the result)
	 */
	int bytes;
	/**
	 * the bytes of type and tag (initially length 0)
	 */
	int header_type_length;
	byte * header_type;
	/**
	 *  the bytes of "length" (initially length 0)
	 */
	int header_length_length;
	byte * header_length;
	/**
	 * data to be added to this.
	 */
	ASN1_Encoder* prefix_data;
	/**
	 * The actual data payload (originally length 0)
	 */
	int data_length;
	byte * data;

	static const byte PC_PRIMITIVE = 0;
	static const byte PC_CONSTRUCTED = 1;
	static const byte CLASS_UNIVERSAL=0;
	static const byte CLASS_APPLICATION=1;
	static const byte CLASS_CONTEXT=2;
	static const byte CLASS_PRIVATE=3;
	static const byte TAG_EOC=0;
	static const byte TAG_BOOLEAN=1;
	static const byte TAG_INTEGER=2;
	static const byte TAG_BIT_STRING=3;
	static const byte TAG_OCTET_STRING=4;
	static const byte TAG_NULL=5;
	static const byte TAG_OID=6;
	static const byte TAG_ObjectDescriptor=7;
	static const byte TAG_EXTERNAL=8;
	static const byte TAG_REAL=9;
	static const byte TAG_EMBEDDED_PDV=11;
	static const byte TAG_UTF8String=12;
	static const byte TAG_RELATIVE_OID=13;
	static const byte TAG_SEQUENCE=16+(1<<5); //0x30
	static const byte TAG_SET=17+(1<<5);
	static const byte TAG_NumericString=18;
	static const byte TAG_PrintableString=19;
	static const byte TAG_T61String=20;
	static const byte TAG_VideotextString2=1;
	static const byte TAG_IA5String=22;
	static const byte TAG_UTCTime=23;
	static const byte TAG_GeneralizedTime=24; //0x18
	static const byte TAG_GraphicString=25;
	static const byte TAG_VisibleString=26;
	static const byte TAG_GenerlString=27;
	static const byte TAG_UniversalString=28;
	static const byte TAG_CHARACTER_STRING=29;
	static const byte TAG_BMPString=30;

	static const byte TYPE_SEQUENCE=16+(1<<5);
	static const byte TYPE_SET=17+(1<<5);

	public:
	void init () {
		bytes = 0;
		header_type = new byte[header_type_length = 0];
		header_length = new byte[header_length_length = 0];
		prefix_data = NULL;
		data = new byte[data_length = 0];
	}
	~ASN1_Encoder () {
		delete header_type;
		delete header_length;
		delete data;
		if (prefix_data != NULL) delete prefix_data;
	}
	ASN1_Encoder () {
		init();
	}
	ASN1_Encoder(BigInteger* b) {
		init();
		delete data;
		data = new byte[data_length = b->m_i_used_bytes];
		memcpy(data, b->toByteArray(), data_length);
		setASN1Type(ASN1_Encoder::TAG_INTEGER);
		setASN1Length(data_length);
		bytes+=data_length;
	}
	/**
	 *
	 * @param b
	 * @param type
	 */
	ASN1_Encoder(BigInteger* b, byte type) {
		init();
		delete data;
		data = new byte[data_length = b->m_i_used_bytes];
		memcpy(data, b->toByteArray(), data_length);
		setASN1Type(type);
		setASN1Length(data_length);
		bytes += data_length;
	}
	ASN1_Encoder(long l) {
		init();
		BigInteger b(l, "");
		delete data;
		data = new byte[data_length = b.m_i_used_bytes];
		memcpy(data, b.toByteArray(), data_length);
		setASN1Type(ASN1_Encoder::TAG_INTEGER);
		setASN1Length(data_length);
		bytes=2+data_length;
	}
	/**
	 * Encode the integer i as an INTEGER
	 * @param i
	 */
	ASN1_Encoder(int i) {
		init();
		BigInteger b(i, "");
		delete data;
		data = new byte[data_length = b.m_i_used_bytes];
		memcpy(data, b.toByteArray(), data_length);
		setASN1Type(ASN1_Encoder::TAG_INTEGER);
		setASN1Length(data_length);
		bytes=2+data_length;
	}
	ASN1_Encoder (byte b) {
		init();
		delete data;
		data = new byte[data_length = 1]; data[0] = b;
		setASN1Type(ASN1_Encoder::TAG_INTEGER);
		setASN1Length(1);
		bytes = 2+data_length;
	}
	ASN1_Encoder(bool b) {
		init();
		delete data;
		data = new byte[1];
		data[0] = b ? (byte)255:(byte)0;
		data_length = 1;
		setASN1Type(ASN1_Encoder::TAG_BOOLEAN);
		setASN1Length(data_length);
		bytes=2+data_length;
	}
	/**
	 * Allocates if buffer not passed. Buffer should be 20 bytes
	 */
	static char* getGeneralizedTime(Calendar * time, char* result = NULL, int buffer_len = LENGTH_GENERALIZED_TIME_NULL_TERMINATED) {
		if (time == NULL) return NULL;
		if (result == NULL) result = new char[buffer_len = 20];
		snprintf(result, buffer_len, "%04d%02d%02d%02d%02d%02d.%03dZ",
				time->year, time->month+1, time->day, time->hour, time->minute, time->second, time->millisecond);
		return result;
	}
	ASN1_Encoder(Calendar * time) {
		init();
		if (time == NULL) {
			this->setNull();
			return;
		}
		char* UTC = getGeneralizedTime(time);
		if (UTC == NULL) {
			System_err_println(1, "<Encoder(Calendar)> Trying to encode wrong time (UTC null)\n");
			return;
		}
		delete data;
		int utclen = strlen(UTC);
		data = (byte*)UTC;
		data_length = utclen;
		setASN1Type(ASN1_Encoder::TAG_GeneralizedTime);
		setASN1Length (utclen);
		bytes = 2 + utclen;
	}
	/**
	 * Encode an OID
	 * @param oid
	 */
	ASN1_Encoder(int* oid, int oid_length) {
		int len = oid_length-1;
		if (len < 0) {
			return;
		}
		for (int k = 2; k < oid_length; k ++) {
			if (oid[k] > 127) {
				BigInteger tmp(oid[k], ASN1_Util::ASN1_EMPTYSTR);
				int l = ASN1_Util::base128_length(&tmp);
				len += l - 1;
			}
		}
		data = new byte[data_length = len];
		data[0]=(byte)(40*oid[0]+oid[1]);
		int coff = 1;
		for (int k = 2; k < oid_length; k ++) {
			BigInteger bn(oid[k], "");
			int b_length = ASN1_Util::toBase_128_length(& bn);
			byte* b = ASN1_Util::toBase_128(& bn, NULL, 0);
			for(int j = 0; j < b_length; j ++, coff ++) data[coff]=(byte)(b[j] | (byte)((j<b_length-1)?0x80:0));
		}
		setASN1Type(ASN1_Encoder::TAG_OID);
		setASN1Length(data_length);
		bytes+=data_length;
	}
	/**
	 * This is for encoding OIDs
	 * @param oid
	 */
	ASN1_Encoder(BigInteger* oid, int oid_length) {
		init();
		BigInteger first(oid[0]);
		first.mul(40);
		first.add(& oid[1]);
		int len_first = ASN1_Util::base128_length(&first);
		int len = len_first;
		for (int k = 2; k < oid_length; k ++) len += ASN1_Util::base128_length(& oid[k]);
		delete data;
		data = new byte[data_length = len];

		byte * b = ASN1_Util::base128(&first, NULL, 0);
		memcpy(data, b, len_first);
		int pos = len_first;
		for (int k = 2; k < oid_length; k ++) {
			len = ASN1_Util::base128_length(& oid[k]);
			b = ASN1_Util::base128(& oid[k], NULL, 0);
			memcpy (data + pos, b, len);
		}

		setASN1Type(ASN1_Encoder::TAG_OID);
		setASN1Length(data_length);
		bytes += data_length;
	}
	ASN1_Encoder(double r) {
		init();
		byte * b = (byte*)& r;
		delete data;
		data_length = ASN1_Util::lenHex(8, ASN1_Util::ASN1_EMPTYSTR);
		data = new byte[data_length];
		ASN1_Util::byteToHex(b, 8, (char*)data, data_length);
		setASN1Type(ASN1_Encoder::TAG_REAL);
		setASN1Length(data_length);
		bytes+=data_length;
	}

	/**
	 * Create a UTF8String Encoder
	 * @param s
	 */
	ASN1_Encoder (const char * s) {
		init();
		if (s == NULL) {
			this->setNull();
			return;
		}
		delete data;
		data = (byte *)strdup(s);
		data_length = strlen(s);
		setASN1Type(ASN1_Encoder::TAG_UTF8String);
		setASN1Length(data_length);
		bytes += data_length;
	}
	/**
	 * Create a BIT_STRING padded Encoder
	 * In the last byte, the useful bits are in the most significant 8-padding_bits bits
	 * @param s
	 * @param padding_bits
	 */
	ASN1_Encoder(byte padding_bits, byte* s, int s_length) {
		init();
		if (s == NULL) {
			this->setNull();
			return;
		}
		delete data;
		data = new byte[data_length = s_length+1];
		data[0] = padding_bits;
		ASN1_Encoder::copyBytes(data_length, (byte*)data, 1, s_length, (byte*)s, s_length, 0);
		setASN1Type(ASN1_Encoder::TAG_BIT_STRING);
		setASN1Length(data_length);
		bytes += data_length;
	}
	/**
	 * Create a BIT_STRING like padded Encoder
	 * In the last byte, the # of useful bits are in the most significant 8-padding_bits bits
	 * @param s
	 * @param padding_bits
	 */
	ASN1_Encoder( byte padding_bits, byte* s, int s_length, byte type) {
		init();
		if (s == NULL) {
			this->setNull();
			return;
		}
		delete data;
		data = new byte[data_length = s_length + 1];
		data[0] = padding_bits;
		ASN1_Encoder::copyBytes(data_length, data, 1, s_length, s, s_length, 0);
		setASN1Type(type);
		setASN1Length(data_length);
		bytes += data_length;
	}
	/**
	 * Factory for BIT_STRING
	 * @param padding_bits
	 * @param s
	 * @param type :Encoder.TAG_BIT_STRING
	 * @return
	 */
	static ASN1_Encoder get_BIT_STRING( byte padding_bits, byte* s, byte type) {
		return new ASN1_Encoder(padding_bits, s, type);
	}
	/**
	 * Create an OCTET STRING encoder.
	 * @param s
	 */
	ASN1_Encoder(byte* s, int s_length, byte type) {
		init();
		if (s == NULL) {
			this->setNull();
			return;
		}
		delete data;
		data = new byte[data_length = s_length];
		memcpy(data, s, s_length);
		setASN1Type(type);
		setASN1Length(data_length);
		bytes += data_length;
	}
	/**
	 * Returns NULLOCTETSTRING,
	 * i.e., NULL ASN object for a null parameter,
	 * and OCTETSTRING otherwise
	 * @param s
	 */
	ASN1_Encoder(byte* s, int s_length) {
		init();
		if (s == NULL) {
			this->setNull();
			return;
		}
		delete data;
		data = new byte[data_length = s_length];
		memcpy(data, s, s_length);
		setASN1Type(ASN1_Encoder::TAG_OCTET_STRING);
		setASN1Length(data_length);
		bytes += data_length;
	}
	/**
	 * Create string
	 * @param s
	 * @param ascii_vs_printable : if true IA5String else PrintableString
	 */
	ASN1_Encoder(const char* s, int s_length, bool ascii_vs_printable) {
		init();
		if (s == NULL) {
			this->setNull();
			return;
		}
		delete data;
		data = new byte[data_length = s_length];
		memcpy(data, s, s_length);
		if (ascii_vs_printable) setASN1Type(ASN1_Encoder::TAG_IA5String);
		else setASN1Type(ASN1_Encoder::TAG_PrintableString);
		setASN1Length(data_length);
		bytes += data_length;
	}
	ASN1_Encoder(const char* s, bool ascii_vs_printable) {
		init();
		if (s == NULL) {
			this->setNull();
			return;
		}
		int s_length = strlen(s);
		delete data;
		data = new byte[data_length = s_length];
		memcpy(data, s, s_length);
		if (ascii_vs_printable) setASN1Type(ASN1_Encoder::TAG_IA5String);
		else setASN1Type(ASN1_Encoder::TAG_PrintableString);
		setASN1Length(data_length);
		bytes += data_length;
	}
	/**
	 * String of type type
	 * @param s
	 * @param type
	 */
	ASN1_Encoder(const char* s, int s_length, byte type) {
		init ();
		if (s == NULL) {
			this->setNull();
			return;
		}
		data = new byte[data_length = s_length];
		memcpy(data, s, s_length);
		setASN1Type(type);
		setASN1Length(data_length);
		bytes += data_length;
	}
	ASN1_Encoder(const char* s, byte type) {
		init ();
		if (s == NULL) {
			this->setNull();
			return;
		}
		int s_length = strlen(s);
		delete data;
		data = new byte[data_length = s_length];
		memcpy(data, s, s_length);
		setASN1Type(type);
		setASN1Length(data_length);
		bytes += data_length;
	}

	/**
	 *
	 * @param param, an array of String[] to encode
	 * @param type, the type to assign to each String
	 * @return, an Encoder
	 */
	static ASN1_Encoder* getStringEncoder(char** param, int param_length, byte type) {
		if (param == NULL) {
			return ASN1_Encoder::getNullEncoder();
		}
		ASN1_Encoder *tmp, * enc = (new ASN1_Encoder())->initSequence();
		for (int k = 0; k < param_length; k ++) {
			if (param[k] != NULL) enc->addToSequence(tmp = new ASN1_Encoder(param[k], strlen(param[k]), type));
			else enc->addToSequence(tmp = ASN1_Encoder::getNullEncoder());
			delete tmp;
		}
		return enc;
	}
	static ASN1_Encoder* getEncoder(char** param, int param_length, byte type) {
		return getStringEncoder(param, param_length, type);
	}
	static ASN1_Encoder* getEncoder (ASNObjArrayable** n, int n_len) {
		ASN1_Encoder* r = new ASN1_Encoder();
		//printf("getEncoder %d\n", n_len);
		r->initSequence();
		for ( int i = 0 ; i < n_len ; i++ ) {
			//printf("i=%d\n", i);
			r->addToSequence(n[i]->getEncoder());
		}
		//printf("/getEncoder\n");
		return r;
	}
	static ASN1_Encoder* getEncoder (ASNObjArrayable** n, int n_len, byte type) {
		ASN1_Encoder* r = new ASN1_Encoder();
		r->initSequence();
		for ( int i = 0 ; i < n_len ; i++ ) {
			r->addToSequence(n[i]->getEncoder()->setASN1Type(type));
		}
		return r;
	}
	static ASN1_Encoder* getNullEncoder() {
		return (new ASN1_Encoder())->setNull();
	}

	ASN1_Encoder * setNull() {
		setASN1Type(ASN1_Encoder::TAG_NULL);
		setASN1Length(0);
		return this;
	}
protected:
	void incrementASN1Length(int inc) {
		BigInteger * len = contentLength();
		BigInteger * bi_inc = new BigInteger(inc, "");
		len = len->add(bi_inc);
		setASN1Length(len->intValue());
		delete len;
		delete bi_inc;
	}
public:
	/**
	 * sets the byte is header_type (updating bytes under assumption that the type was length was precomputed)
	 * @param tagASN1
	 * @return
	 */
	ASN1_Encoder * setASN1TypeImplicit(byte tagASN1) {
			int old_len_len = this->header_type_length;
			delete header_type;
			header_type = new byte[header_type_length = 1]; header_type[0] = tagASN1;
			bytes += header_type_length - old_len_len;
			return this;
	}
	/**
	 * sets the byte is header_type
	 * @param tagASN1
	 * @return
	 */
	ASN1_Encoder* setASN1TypeExplicit(byte tagASN1) {
		ASN1_Encoder* result =  new ASN1_Encoder();
			result->initSequence()->
			addToSequence(this)->setASN1Type(tagASN1);
		return result;
	}
	/**
	 * The defaut is implicit
	 * @param classASN1
	 * @param PCASN1
	 * @param tag_number
	 * @return
	 */
	ASN1_Encoder * setASN1Type(int classASN1, int PCASN1, BigInteger* tag_number) {
		return setASN1TypeImplicit(classASN1, PCASN1, tag_number);
	}
	ASN1_Encoder* setASN1TagExplicit(int _classASN1, int _privateASN1, BigInteger *tag_number) {
		ASN1_Encoder* e = new ASN1_Encoder(); e->initSequence();
		e->addToSequence(this);
		e->setASN1Type(_classASN1, _privateASN1, tag_number);
		return e;
	}
	/**
	 *
	 * @param classASN1  The ASN1 Class (UNIVERSAL/APPLICATION/CONTEXT/PRIVATE)
	 * @param PCASN1	Is this PRIMITIVE or CONSTRUCTED
	 * @param tag_number : at most 30
	 * @return returns this
	 */
	static byte buildASN1byteType(int classASN1, int PCASN1, byte tag_number){
		if ((tag_number) >= 31) { //tag_number&0x1F
			if (tag_number != ASN1_Encoder::TAG_SEQUENCE)
				System_err_println(ERR, "Need more bytes for: %d\n",tag_number);
			tag_number = (byte)(tag_number & (byte)0x1F);
			if(tag_number == 31) tag_number = 25;
		}
		int tag = ((classASN1&0x3)<<6)+((PCASN1&1)<<5)+(tag_number&0x1f);
		return (byte)tag;
	}
	/**
	 * Returns the bit 6 (PRIMITIVE or CONSTRUCTED)
	 * @param type
	 * @return
	 */
	static bool isASN1byteTypeCONSTRUCTED(byte type) {
		return (type & (1<<5)) != 0;
	}
	/**
	 * result can be Encoder.CLASS_UNIVERSAL, Encoder.CLASS_APPLICATION, Encoder.CLASS_CONTEXT or Encoder.CLASS_PRIVATE
	 * @param type
	 * @return
	 */
	static int getASN1byteTypeCLASS(byte type) {
		return (type >> 6) & 0x3;
	}
	/**
	 * Returns the last 5 bits
	 * @param type
	 * @return
	 */
	static int getASN1byteTypeTAG(byte type) {
		return type & 0x1F;
	}
	/**
	 *
	 * @param classASN1  The ASN1 Class (UNIVERSAL/APPLICATION/CONTEXT/PRIVATE)
	 * @param PCASN1	Is this PRIMITIVE or CONSTRUCTED
	 * @param tag_number
	 * @return returns this
	 */
	ASN1_Encoder * setASN1Type(int classASN1, int PCASN1, byte tag_number) {
		return setASN1Type(buildASN1byteType(classASN1, PCASN1, tag_number));
	}
	ASN1_Encoder * setASN1TypeImplicit(int classASN1, int PCASN1, byte tag_number) {
		return setASN1TypeImplicit(buildASN1byteType(classASN1, PCASN1, tag_number));
	}

	/**
	 * The default is implicit.
	 * @param tagASN1
	 * @return
	 */
	ASN1_Encoder * setASN1Type(byte tagASN1) {
		return setASN1TypeImplicit(tagASN1);
	}
	/**
	 *
	 * @param classASN1  The ASN1 Class (UNIVERSAL/APPLICATION/CONTEXT/PRIVATE)
	 * @param PCASN1	Is this PRIMITIVE or CONSTRUCTED
	 * @param tag_number
	 * @return returns this
	 */
	ASN1_Encoder * setASN1TypeExplicit(int classASN1, int PCASN1, byte tag_number) {
		return setASN1TypeExplicit(buildASN1byteType(classASN1, PCASN1, tag_number));
	}

	/**
	 *
	 * @param classASN1  The ASN1 Class (UNIVERSAL/APPLICATION/CONTEXT/PRIVATE)
	 * @param PCASN1	Is this PRIMITIVE or CONSTRUCTED
	 * @param tag_number a big integer
	 * @return returns this
	 */
	ASN1_Encoder * setASN1TypeExplicit(int classASN1, int PCASN1, BigInteger* tag_number) {
		ASN1_Encoder * result;
		result = new ASN1_Encoder();
		result->initSequence();
		result->addToSequence(this)->setASN1Type(classASN1, PCASN1, tag_number);
		return result;
	}
	/**
	 *
	 * @param classASN1  The ASN1 Class (UNIVERSAL/APPLICATION/CONTEXT/PRIVATE)
	 * @param PCASN1	Is this PRIMITIVE or CONSTRUCTED
	 * @param tag_number a big integer
	 * @return returns this
	 */
	ASN1_Encoder * setASN1TypeImplicit(int classASN1, int PCASN1, BigInteger* tag_number) {
		BigInteger BI31(31, "");
		if (tag_number->cmp(BI31) <= 0) {
			return this->setASN1Type(classASN1, PCASN1, (byte)tag_number->intValue());
		}
		// if the tag is 31 or bigger
		int old_header_type_len = this->header_type_length;
		int tag = (classASN1<<6)+(PCASN1<<5)+0x1f;
		int nb_length = ASN1_Util::toBase_128_length(tag_number);
		byte* nb = new byte[nb_length];
		ASN1_Util::toBase_128(tag_number, nb, nb_length);
		int tag_len = nb_length;
		header_type = new byte[header_type_length = tag_len+1];
		header_type[0] = (byte) tag;
		for (int k = 0; k < tag_len - 1; k ++) {
			header_type[k+1] = (byte) (nb[k] | 0x80);
		}
		header_type[tag_len] = (byte) nb[tag_len-1];

		bytes += header_type_length - old_header_type_len;
		return this;
	}
	protected:
	void setPrefix(ASN1_Encoder* prefix) {
		if (prefix_data == NULL) prefix_data = prefix;
		else {
			prefix_data->setPrefix(prefix);
		}
		bytes += prefix->bytes;
	}
	public:
	static void copyBytes(int results_length, byte results[], int offset, int src_length, const byte src[], int length) {
		copyBytes(results_length, results, offset, src_length, src, length, 0);
	}
	static void copyBytes(int dest_length, byte dest[], int dest_offset, int src_length, const byte src[], int length, int src_offset) {
		if (dest_length < length + dest_offset)
			System_err_println(ERR, "Destination too short: %d vs %d+%d\n", dest_length, dest_offset, length);
		if (src_length < length + src_offset)
			System_err_println(ERR, "Source too short: %d vs %d+%d\n", src_length, src_offset, length);

		for (int k = 0; k < length; k ++) {
			dest[k + dest_offset] = src[src_offset + k];
		}
	}
	int getBytesNb() {
		return bytes;
	}
	byte* getBytes() {
		byte* buffer = new byte[bytes];
		getBytesIterative(this, bytes, buffer, 0);
		return buffer;
	}
	byte* getBytes(byte* buffer) {
		getBytesIterative(this, bytes, buffer, 0);
		return buffer;
	}
	static int getBytesIterative(ASN1_Encoder* enc, int results_length, byte results[], int offset) {
		int data_bytes = 0;
		for ( ; enc != NULL; ) {
			int disp = 0;
			copyBytes(results_length, results, offset, enc->header_type_length, enc->header_type, enc->header_type_length);
			offset += enc->header_type_length;
			copyBytes(results_length, results, offset, enc->header_length_length, enc->header_length, enc->header_length_length);
			offset += enc->header_length_length;

			if (enc->prefix_data != NULL) disp = enc->prefix_data->bytes;

			copyBytes(results_length, results, offset + disp, enc->data_length, enc->data, enc->data_length);
			data_bytes += enc->data_length;
			enc = enc->prefix_data;
		}
		return data_bytes + offset;
	}
	ASN1_Encoder* initSequence() {
		delete header_type;
		header_type = new byte[this->header_type_length = 1];
		header_type[0] = buildASN1byteType(ASN1_Encoder::CLASS_UNIVERSAL, ASN1_Encoder::PC_CONSTRUCTED, ASN1_Encoder::TAG_SEQUENCE);

		delete header_length;
		header_length = new byte[this->header_length_length = 1];
		header_length[0] = 0x0;
		bytes = 2;
		return this;
	}
	/**
	 * Sets length in header_length (which is array of bytes).
	 * If length <= 127, then use a single byte.
	 * Else, one byte with (128+length) and the bytes needed to represent length (and sign).
	 *
	 * Difference in length is added/subtracted to "bytes"
	 * @param _bytes_len
	 */
	void setASN1Length(int _bytes_len) {
		int old_len_len = this->header_length_length;
		if (_bytes_len <= 127) {
			delete header_length;
			header_length = new byte[header_length_length = 1];
			header_length[0] = (byte) _bytes_len;
		} else {
			BigInteger* len = new BigInteger(_bytes_len,"");
			const byte* len_bytes = len->toByteArray();
			int len_bytes_length = len->getByteArrayLen();
			delete len;
			delete header_length;
			header_length = new byte[header_length_length = 1 + len_bytes_length];
			header_length[0] = (byte)(len_bytes_length | 0x80); // +128
			copyBytes(header_length_length, header_length, 1, len_bytes_length, len_bytes, len_bytes_length);
		}
		this->bytes += header_length_length - old_len_len;
	}
	BigInteger * contentLength() {
		if (header_length_length == 0) return &BigInteger::ZERO;
		if (header_length[0]>=0) return new BigInteger(header_length[0], "");
		byte* result = new byte[header_length_length-1];
		copyBytes(header_length_length-1, result, 0, header_length_length, header_length, header_length_length-1, 1);
		return new BigInteger(result, header_length_length-1);
	}
	/**
	 * Equivalent to addToSequence(asn1_data.getBytes)
	 * @param asn1_data
	 * @param efface (if true, delete the parameter after attachment)
	 * @return
	 */
	ASN1_Encoder* addToSequence(ASN1_Encoder* asn1_data, bool efface = true) {
		byte* msg;
		ASN1_Encoder* result = addToSequence(asn1_data->getBytesNb(), msg = asn1_data->getBytes());
		//delete[] msg;
		if (efface) delete asn1_data;
		return result;
	}
	/**
	 * Equivalent to addToSequence(asn1_data, 0, asn1_data.length);
	 * @param asn1_data
	 * @param move (if true, keep ownership of the data)
	 * @return
	 */
	ASN1_Encoder* addToSequence(int asn1_data_length, byte asn1_data[], bool  move = true) {
		return addToSequence(asn1_data, 0, asn1_data_length, move);
	}
	/**
	 * Adds data from array asn1_data from index offset, of length "length"
	 *
	 * Sets this to the top of the tree, and delegates previous content to the "prefix_data",
	 * updating "bytes"
	 *
	 * @param asn1_data
	 * @param offset
	 * @param length
	 * @param move (if true, keep ownership of the data)
	 * @return
	 */
	ASN1_Encoder* addToSequence(byte asn1_data[], int offset, int length, bool move = true) {
		if (data_length == 0) {
			int asn1_data_length = offset + length;
			delete[] data;
			if (move) {
				data = asn1_data + offset;
				data_length = length;
			} else {
				data = new byte[data_length = length];
				copyBytes(data_length, data, 0, asn1_data_length, asn1_data, length, offset);
			}
			this->incrementASN1Length(length);
			bytes += length;
		} else {
			ASN1_Encoder * e = new ASN1_Encoder();
			if (e->data) delete e->data;
			e->data = data; e->data_length = data_length;
			//if (e->prefix_data) delete e->prefix_data; // cannot be
			e->prefix_data = prefix_data;
			e->bytes = bytes - header_length_length - header_type_length;
			prefix_data = e;
			//if (data) delete data; // do not free since it is moved to prefix
			data = new byte[data_length = 0];
			addToSequence(asn1_data, offset, length, move);
		}
		return this;
	}
	void print() {
		System_err_println(1, "Object size: %d\n", bytes);
		System_err_println(1, "typ:[%d]  %s\n", this->header_type_length, ASN1_Util::byteToHex(this->header_type, this->header_type_length, ASN1_Util::ASN1_SPACESTR, NULL, 0));
		System_err_println(1, "len:[%d]  %s\n", this->header_length_length, ASN1_Util::byteToHex(this->header_length, this->header_length_length, ASN1_Util::ASN1_SPACESTR, NULL, 0));
		if (this->prefix_data != NULL) {
			System_err_println(1, "Prefix: \n");
			this->prefix_data->print();
		}
		System_err_println(1, "data:[%d] %s\n", this->data_length, ASN1_Util::byteToHex(this->data, this->data_length, ASN1_Util::ASN1_SPACESTR, NULL, 0));
		char* s;
		System_err_println(1, "All: %s\n", s = toString());
		delete s;
		System_err_println(1, "Done Object size: %d\n", bytes);
	}
	char * toString() {
		byte* str = getBytes();
		char* result = ASN1_Util::byteToHex(str, this->getBytesNb()," ", NULL, 0);
		delete str;
		return result;
	}
};

#endif /* ASN1ENCODER_H_ */
