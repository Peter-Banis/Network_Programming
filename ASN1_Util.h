/*
 * ASN1_Util.h
 *  Copyright (C) 2018
 *
 *  Created on: Dec 31, 2017
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

#ifndef ASN1_UTIL_H_
#define ASN1_UTIL_H_
#include <time.h>
#include "BigInteger.h"
#define YEAR_ROOT_CTIME 1900

struct Calendar {
	int year, month, day, hour, minute, second, millisecond;
	static const int MILLISECOND = 12;
	static const int MONTH = 11;
	Calendar() {
		struct tm timeb;
		time_t crtime;
		time(& crtime);
		gmtime_r(& crtime, & timeb);

		year = timeb.tm_year + YEAR_ROOT_CTIME;
		month = timeb.tm_mon;
		day = timeb.tm_mday;
		hour = timeb.tm_hour;
		minute = timeb.tm_min;
		second = timeb.tm_sec;
		millisecond = 0;
	}
	void set (int _year, int _month, int _day, int _hour, int _minute, int _second)
	{
		year=_year; month=_month; day=_day; hour=_hour; minute=_minute; second=_second;
	}
	void set (int type, int data) {
		switch (type) {
		case MILLISECOND: millisecond = data; break;
		case MONTH: month = data;
		break;
		}
	}
	int get (int type) {
		switch (type) {
		case MILLISECOND: return millisecond;
		case MONTH: return month;
		}
		return 0;
	}
	static Calendar* getInstance() {
		return new Calendar();
	}
};

class Integer {
public:
	static int parseInt(char * str, int len) {
		char buf [len + 1];
		snprintf(buf, len + 1, "%s", str);
		BigInteger val(buf);
		return val.intValue();
	}
};

class ASN1_Util {
public:
	static const char ASN1_EMPTYSTR[];
	static const char ASN1_SPACESTR[];
private:
	static const bool _DEBUG = false;
public:
	static const int MAX_ASN1_DUMP = 20;
    static char const HEX[];
    static char* byteToHex(byte* b, int b_length, int off, int len, const char* separator, char* result, int rsize) {
		if (result == NULL) {
			rsize = lenHex(b_length, separator);
			result = new char[rsize];
		}
		if (b == NULL) {
				snprintf(result, rsize, "%s", "NULL");
				return result;
		}
		result[0] = 0;
		if (off < 0) return result;
		int pos = 0;
		int sep_len = strlen(separator);
		for (int i = off; i < off + len; i ++) {
				if (i >= b_length || pos + 2 + sep_len > rsize) break;
				sprintf(result + pos, "%s%c%c", separator, HEX[(b[i]>>4) & 0x0f], HEX[b[i] & 0x0f]);
				pos += 2 + sep_len;
		}
		return result;
    }
    /**
     *
     */
    static char* byteToHex(byte* b, int b_length, const char* sep, char* result, int rsize) {
		if (result == NULL) {
			rsize = lenHex (b_length, sep);
			result = new char[rsize];
		}
		if (b == NULL) {
			snprintf(result, rsize, "%s", "NULL");
			return result;
		}
		result[0] = 0;
		int pos = 0;
		int sep_len = strlen(sep);
		for (int i = 0; i < b_length; i ++) {
			if (pos + 2 + sep_len > rsize) break;
			sprintf(result + pos, "%s%c%c", sep, HEX[(b[i]>>4) & 0x0f], HEX[b[i] & 0x0f]);
			pos += 2 + sep_len;
		}
		return result;
	}

	static char* byteToHex(byte* b, int b_length, char* result, int rsize) {
		if (result == NULL) {
			rsize = lenHex(b_length, ASN1_EMPTYSTR);
			result = new char[rsize];
		}
		return ASN1_Util::byteToHex(b, b_length, ASN1_EMPTYSTR, result, rsize);
    }
	static int lenHex (int b_length, const char * sep) {
		return 2 * b_length + strlen (sep)*(b_length-1) + 1;
	}
	/**
	 * Return now at UTC
	 * @return
	 */
	static Calendar* CalendargetInstance() {
		return Calendar::getInstance();
	}
	/**
	 * Get a Calendar for this gdate, or null in case of failure
	 * @param gdate
	 * @return
	 */
	static Calendar* getCalendar(char* gdate) {
		return getCalendar(gdate, NULL);
	}
	/**
	 * Get a Calendar for this gdate, or ndef in case of failure
	 * @param gdate
	 * @param def
	 * @return
	 */
	static Calendar* getCalendar(char* gdate, Calendar* def) {
		char _gdate[18];
		if ((gdate == NULL) || (strlen(gdate) < 14)) {return def;}
		snprintf (_gdate, 18, "%s0000000", gdate);

		Calendar* date = CalendargetInstance();
		try  {
			date->set(Integer::parseInt(_gdate, 4),
					Integer::parseInt(_gdate + 4, 6) - 1,
					Integer::parseInt(_gdate + 6, 8),
					Integer::parseInt(_gdate + 8, 10),
					Integer::parseInt(_gdate + 10, 12),
					Integer::parseInt(_gdate + 12, 14));
			date->set(Calendar::MILLISECOND, Integer::parseInt(_gdate + 15, 18));
		} catch (void * e) {return def;}
		return date;
	}
	/**
	 * @param val
	 * @return
	 */
	static short getUnsignedShort(byte val) {
		if (val >= 0) return val;
		return (short)(val + 256);
	}
	/**
	 * Convert to base 128 (bigendian), using shifts.
	 * @param val
	 * @return
	 */

	static int base128_length(BigInteger* _val) {
		int length = 0;
		BigInteger val(_val);
		int sign = val.getSignBit();
		int part = val.getByte(0) & 0x07f;
		val.shiftRight(7);
		length ++;
		while (
				! (! sign && val.equals(BigInteger::ZERO) && !(part & 0x40))
				&&
				! (sign && val.equals(BigInteger::MINUS) && (part & 0x40))
				) {
			part = val.getByte(0) & 0x7f;
			val.shiftRight(7);
			part += 128;
			length ++;
		};
		return length;
	}
	static byte* base128(BigInteger* _val, byte * result, int result_length) {
		BigInteger val(_val);
		int sign = val.getSignBit();
		if (result == NULL) {
			result_length = base128_length(_val);
			result = new byte[result_length];
		}
		int part = val.getByte(0) & 0x07f;
		val.shiftRight(7);
		result[result_length - 1] = part;
		int k = result_length - 2;
		while (
				! (! sign && val.equals(BigInteger::ZERO) && !(part & 0x40))
				&&
				! (sign && val.equals(BigInteger::MINUS) && (part & 0x40))
				) {
			part = val.getByte(0) & 0x7f;
			val.shiftRight(7);
			part += 128;
			result[k --] = part;
		};
		return result + k + 1;
	}
	/**
	 * This is based on bit shifting (using base128()).
	 * Returns array of bytes.
	 * caller deletes if passing NULL
	 * @param val
	 * @return
	 */
	static byte* toBase_128(BigInteger* _val, byte* result, int result_length) {
		BigInteger val(_val);
		if (result == NULL) {
			result_length = toBase_128_length(_val);
			result = new byte[result_length];
		}
		int sign = val.getSignBit();
		int part = val.getByte(0) & 0x07f;
		val.shiftRight(7);
		result[result_length - 1] = part;
		int pos = result_length - 2;
		while
			(
					! (! sign && val.equals(BigInteger::ZERO) && !(part & 0x40))
					&&
					! (sign && val.equals(BigInteger::MINUS) && (part & 0x40))
			)
		{
			part = val.getByte(0) & 0x7f;
			val.shiftRight(7);
			part += 128;
			result[pos --] = part;
			if (pos < 0) break;
		};
		return result + pos + 1;
	}
	static int toBase_128_length(BigInteger* _val) {
		BigInteger val(_val);
		int sign = val.getSignBit();
		int part = val.getByte(0) & 0x07f;
		val.shiftRight(7);
		int result = 1;
		while
			(
					! (! sign && val.equals(BigInteger::ZERO) && !(part & 0x40))
					&&
					! (sign && val.equals(BigInteger::MINUS) && (part & 0x40))
			)
		{
			part = val.getByte(0) & 0x7f;
			val.shiftRight(7);
			part += 128;
			result ++;
		};
		return result;
	}
	/**
	 * Decodes from base 128 under the assumption that it is bigendian, terminated by a byte smaller than 128,
	 * all other bytes being OR-ed with 128.
	 * @param b128
	 * @param offset
	 * @param limit
	 * @return
	 */
	static BigInteger* fromBase128(byte* b128, int b128_length, int offset, int limit) {
		BigInteger* result = new BigInteger(0, "");
		int k = offset;
		while ((k < limit) && ((b128[k] & 0x80) != 0)) {
			result->shiftLeft(7);
			result->or_bits(b128[k] & 0x7f);
			k++;
		}
		if (k < limit) {
			if ((b128[k] & 0x80) != 0) {
				if (_DEBUG)
					System_out_println("Util: fromBase_128: last byte > %d\n", 127);
			}
			result->shiftLeft(7);
			result->or_bits(b128[k]); // here  & 0x7f would be redundant
		}
		return result;
	}
};

#endif /* ASN1_UTIL_H_ */
