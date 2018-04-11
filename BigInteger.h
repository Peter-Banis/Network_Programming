/*
 * BigInteger.h
 *  Copyright (C) 2018
 *
 *  Created on: Oct 3, 2017
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

#ifndef BIGINTEGER_H_
#define BIGINTEGER_H_


#include <string.h>
#include <stdio.h>
#include <math.h>

#define ERR 1
#define need_to_print(flag) ((flag) != 0)

#define System_err_println(debug_level, ...) \
  ({ \
    if (need_to_print(debug_level)) \
      printf(__VA_ARGS__); \
  })
#define System_out_println(msg, ...) \
  ({ \
    if (need_to_print(1)) \
      printf(msg, __VA_ARGS__); \
  })

typedef unsigned char byte;

struct BigInteger {
	static BigInteger ZERO; // a reference to a ZERO to be used when needed
	static BigInteger ONE; // a reference to a ZERO to be used when needed
	static BigInteger MINUS; // a reference to a ZERO to be used when needed
	bool immutable;
	byte * m_pB_storage;
	int m_i_storage_length;
	// the number of bytes really needed from the storage
	int m_i_used_bytes;
	BigInteger(BigInteger* op): immutable(false) {
		m_i_storage_length = op->m_i_used_bytes << 1;
		m_pB_storage = new byte[m_i_storage_length];
		memcpy(m_pB_storage, op->m_pB_storage, op->m_i_used_bytes);
		m_i_used_bytes = op->m_i_used_bytes;
	}
	BigInteger(BigInteger& op): immutable(false) {
		m_i_storage_length = op.m_i_used_bytes << 1;
		m_pB_storage = new byte[m_i_storage_length];
		memcpy(m_pB_storage, op.m_pB_storage, op.m_i_used_bytes);
		m_i_used_bytes = op.m_i_used_bytes;
	}
	/**
	 * Constructor with given data storage in 2s complement.
	 * Passed storage is not retained, and should be cleaned by caller.
	 * On empty data, a one byte 0 is initialized;
	 */
	BigInteger(byte * _storage, int _storage_length, byte* keep = NULL, bool _immutable = false): immutable(_immutable) {
		if (_storage_length <= 0) {
			m_i_storage_length = 1;
			m_pB_storage = new byte[m_i_storage_length];
			m_pB_storage[0] = 0;
			m_i_used_bytes = 1;
			return;
		}
		m_i_storage_length = _storage_length;
		if (keep) m_pB_storage = _storage;
		else {
			m_pB_storage = new byte[m_i_storage_length];
			memcpy(m_pB_storage, _storage, _storage_length);
		}
		m_i_used_bytes = _storage_length;
	}
	/**
	 * An integer specified in the first parameter
	 */
	BigInteger(int val, const char* dummy, bool _immutable = false): immutable(_immutable) {
		m_i_storage_length = sizeof(int);
		m_pB_storage = new byte[m_i_storage_length];
		m_i_used_bytes = 0;
		for (int i = 0; i < m_i_storage_length; i ++) {
			m_pB_storage[i] = val & 0xff;
			m_i_used_bytes ++;
			val >>= 8;
			if ((val == 0  && ! (m_pB_storage[i] & 0x80))  // if positive and done
					|| (val == -1 && (m_pB_storage[i] & 0x80))) // if negative and done
				break;
		}
	}
	/**
	 * Parse a decimal string starting with optional spaces, optional +-, and a number
	 * [ \t\n\r]*[+-]?[0-9]*
	 * Note that accepts an entry string "  " and empty +- "   +hello" and sets 0.
	 * No failure if str is null terminated.
	 * TODO: optimize - multiply with a group of 2 (byte), 9 (long) digits at a time.
	 */
	BigInteger(const char* str, int radix = 10): immutable(false) {
		if (radix == 16) {
			parseHex(str);
			return;
		}
		m_i_storage_length = ceil(ceil(strlen(str)/3.0)*10.0/8.0);
		m_pB_storage = new byte[m_i_storage_length];
		m_i_used_bytes = 0;
		setZero();
		bool negative = false;
		bool inNb = false; // in number (spaces tell termination)
		if (str == NULL) return;
		while (*str != 0) {
			if (! inNb) {
				if (*str == ' ' || *str == '\t' || *str == '\n') { str ++; continue;}
				if (*str == '+') {str ++; negative = false; inNb = true; continue;}
				if (*str == '-') {str ++; negative = true; inNb = true; continue;}
				if (*str < '0' || *str > '9') {break;} // illegal string. Let it be 0.
			}
			if (*str < '0' || *str > '9') {inNb = false; break;}

			// select here a number of subsequent digits and handle all at once. Now a single digit is used.
			int val = *str & 0x0f;
			this->_mul10();
			this->addByte(val);
			str ++;
		}
		if (negative) neg();
	}
	~BigInteger() {
		delete m_pB_storage;
	}
	/**
	 *  pointer to temporary (deallocated by structure)
	 */
	const byte* toByteArray(int* pUsedLength) {
		if (pUsedLength != NULL)
			*pUsedLength = m_i_used_bytes; //storage_length;
		return m_pB_storage;
	}
	/**
	 * Return pointer to temporary (do not deallocate!)
	 */
	const byte* toByteArray() {
		return m_pB_storage;
	}
	/**
	 * The used length of the storage
	 */
	int getByteArrayLen() {
		return m_i_used_bytes; //storage_length;
	}
	/**
	 * Make the storage longer if needed, and increment used bytes. On safe, set sign in extended bytes.
	 */
	void extendUnsafeUsedBytesBy (int inc, bool safe = 0) {
		if (immutable) return;
		if (m_i_used_bytes + inc > m_i_storage_length) {
			byte * _newstorage = new byte[m_i_used_bytes + inc];
			memcpy(_newstorage, m_pB_storage, m_i_used_bytes);
			delete m_pB_storage;
			m_pB_storage = _newstorage;
			m_i_storage_length = m_i_used_bytes + inc;
		}
		if (safe) {
			int sign = m_pB_storage[m_i_used_bytes - 1] & 0x80;
			if (sign) memset(m_pB_storage + m_i_used_bytes, 0xff, inc);
			else  memset(m_pB_storage + m_i_used_bytes, 0x00, inc);
		}
		m_i_used_bytes += inc;
	}
	/**
	 * Make the storage longer if needed, and increment used bytes. Set sign in extended bytes (if safe).
	 */
	void extendSafeUsedBytesBy (int inc, bool safe = 1) {
		extendUnsafeUsedBytesBy(inc, safe);
	}
protected:
	/**
	 * Called only from constructor
	 */
	void parseHex(const char* str) {
		m_i_storage_length = ceil(strlen(str)/2)+1;
		m_pB_storage = new byte[m_i_storage_length];
		m_i_used_bytes = 0;
		setZero();
		bool negative = false;
		bool inNb = false; // in number (spaces tell termination)
		if (str == NULL) return;
		while (*str != 0) {
			if (! inNb) {
				if (*str == ' ' || *str == '\t' || *str == '\n') { str ++; continue;}
				if (*str == '+') {str ++; negative = false; inNb = true; continue;}
				if (*str == '-') {str ++; negative = true; inNb = true; continue;}
				if (*str < '0' || *str > '9') {
					if (*str < 'A' || *str > 'F')
						break;
				} // illegal string. Let it be 0.
			}
			int val;
			if (*str < '0' || *str > '9') {
				if (*str < 'A' || *str > 'F') {
					inNb = false;
					break;
				} else {
					val = *str - ('A' - 10);
				}
			} else {
				val = *str & 0x0f;
			}
			this->shiftLeft(4);
			this->addByte(val);
			str ++;
		}
		if (negative) neg();
	}
public:
	/**
	 * Make the current number negative/positive, using 2's complement.
	 * May need extension.
	 *
	 */
	void neg() {
		if (immutable) return;
			int i;
			// skip low significant 0s and negate first non 0.
			for (i = 0; i < m_i_used_bytes; i ++) {
				if (! m_pB_storage[i]) continue;

				// check if extension needed (last byte and negative sign bit: 10000000b)
				if ((i == m_i_used_bytes - 1) && (m_pB_storage[i] == 0x80))
					extendSafeUsedBytesBy(1);

				m_pB_storage[i] = - m_pB_storage[i];
				break;
			}
			for (i ++; i < m_i_used_bytes; i ++) {
				m_pB_storage[i] = ~ m_pB_storage[i];
			}

			// check for reduction
			if (m_i_used_bytes > 1) {
				if (
						((m_pB_storage[m_i_used_bytes - 1] == 0xff)
						 && ((m_pB_storage[m_i_used_bytes - 2] & 0x80) == 0x80))
					)
				m_i_used_bytes --;
			}
	}
	/**
	 * Print a hex representation with bytes spaced by a space
	 */
	void dump() {
		for (int i = 0; i < m_i_used_bytes; i ++) {
			printf("%02x ", m_pB_storage[i]);
		}
		printf("\n");
	}
	/**
	 * Ass one byte and perform carry as needed
	 */
	void addByte(byte val) {
		if (immutable) return;
		int tmp = m_pB_storage[0] & 0x0ff;
		tmp += (val & 0x0ff);
		int carryin = tmp >> 8;
		m_pB_storage[0] = tmp;
		for (int i = 1; i < m_i_used_bytes; i ++) {
			tmp = m_pB_storage[i] & 0x0ff;
			tmp += carryin;
			m_pB_storage[i] = tmp;
			carryin = tmp >> 8;
		}
		if (carryin > 0) {
			extendUnsafeUsedBytesBy(1);
			m_pB_storage[m_i_used_bytes - 1] = carryin;
		}
	}
	/**
	 * Multiply with constant 10
	 */
	void _mul10() {
		if (immutable) return;
		shiftLeft(1);
		addShiftLeft(2);
	}
	void mul(int op) {
		if (immutable) return;
		int sign = this->getSignBit();
		if (sign) neg();
		if (op < 0) {
			sign = ! sign;
			op = -op;
		}
		long carry = 0;
		for (int i = 0; i < this->m_i_used_bytes; i ++) {
			long tmp = (long(op)) * (long(this->m_pB_storage[i])&0xff) + carry;
			this->m_pB_storage[i] = tmp & 0xff;
			carry = tmp >> 8;
		}
		while (carry > 0) {
			this->extendUnsafeUsedBytesBy(1);
			this->m_pB_storage[m_i_used_bytes - 1] = carry & 0xff;
			carry >>= 8;
		}
		if (this->getSignBit()) {
			this->extendUnsafeUsedBytesBy(1);
			this->m_pB_storage[m_i_used_bytes - 1] = 0;
		}
		if (sign) neg();
	}
	void mul(BigInteger* op) {
		if (immutable) return;
		BigInteger* tmp = new BigInteger(this);
		setZero();
		for (int i = 0; i < op->m_i_used_bytes; i++) {
			BigInteger* term = new BigInteger(tmp);
			term->mul(op->m_pB_storage[i] & 0x0ff);
			term->shiftLeft(i);
			add(term);
			delete term;
		}
		delete tmp;
	}
	/**
	 * Shift and add in a single passage
	 */
	void addShiftLeft(int _shift) {
		if (immutable) return;
		int BITS = 8;
		int tmp_sh, tmp_sum, right_shift =  BITS - _shift;
		int carryin_sum = 0, carryout_sum = 0;
		int carryin_sh = 0, carryout_sh = 0;
		int sign = getSignBit();

		for (int i = 0; i < m_i_used_bytes; i ++) {
			tmp_sh = (m_pB_storage[i] & 0x00ff) << _shift; // what is kept
			tmp_sh |= carryin_sh;								// what comes in
			carryout_sh = (m_pB_storage[i] & 0x00ff) >> right_shift; // tmp >> 8
			tmp_sh &= 0x00ff;
			tmp_sum = tmp_sh + m_pB_storage[i] +carryin_sum; 			// what was in
			m_pB_storage[i] = tmp_sum & 0xff;
			carryout_sum = (tmp_sum >> BITS);
			carryin_sum = carryout_sum;
			carryin_sh = carryout_sh;
		}

		if (carryout_sum > 0 || carryout_sh > 0 || sign != getSignBit()) {
			extendUnsafeUsedBytesBy(1);

			if (sign) {
				for (int j = 7; j >= 0; j --) {
					if (carryout_sh & (1 << j)) break;
					carryout_sh |= (1<<j);
				}
				carryout_sum += 0xff; // extending the original integer
			}

			m_pB_storage[m_i_used_bytes - 1] = (carryout_sum + carryout_sh) & 0x00ff;
		}
	}
	/**
	 * Shifting full bytes.
	 * duplicating length extra bytes are added to storage, if it has to be enlarged
	 */
	void shiftLeftBytes(int _bytes) {
		if (immutable) return;
		int extra_len = m_i_used_bytes + _bytes; // at least 0
		if (m_i_used_bytes + _bytes > m_i_storage_length) {
			int new_storage_length = m_i_used_bytes + _bytes + extra_len;
			byte * _newstorage = new byte[new_storage_length];
			memcpy (_newstorage + _bytes, m_pB_storage, m_i_used_bytes);
			delete m_pB_storage;
			m_pB_storage = _newstorage;
			m_i_storage_length = new_storage_length;
		} else {
			memmove (m_pB_storage + _bytes, m_pB_storage, m_i_used_bytes);
		}
		memset(m_pB_storage, 0x00, _bytes);

		m_i_used_bytes += _bytes;
	}
	/**
	 * Shifting full bytes.
	 */
	void shiftRightBytes(int _bytes) {
		if (immutable) return;

		if (_bytes >= m_i_used_bytes) throw NULL;

		if (m_i_used_bytes - _bytes < m_i_storage_length/4) {
			int new_storage_length = m_i_storage_length/2;
			byte * _newstorage = new byte[new_storage_length];
			memcpy (_newstorage, m_pB_storage + _bytes, m_i_used_bytes - _bytes);
			delete m_pB_storage;
			m_pB_storage = _newstorage;
			m_i_storage_length = new_storage_length;
		} else {
			memmove (m_pB_storage, m_pB_storage + _bytes, m_i_used_bytes - _bytes);
		}

		m_i_used_bytes -= _bytes;
	}
	void shiftRightBits(unsigned int _bits) {
		if (immutable) return;
		int bytes = _bits / 8;
		int __bits = _bits % 8;
		if (bytes > 0) shiftRightBytes(bytes);
		if (__bits > 0) shiftRight(__bits);
	}
	void shiftLeftBits(unsigned int _bits) {
		if (immutable) return;
		int bytes = _bits / 8;
		int __bits = _bits % 8;
		if (bytes > 0) shiftLeftBytes(bytes);
		if (__bits > 0) shiftLeft(__bits);
	}
	/**
	 * Arithmetic shift
	 * Supports at most a shift of 8.
	 */
	void shiftLeft(int _shift) {
		if (immutable) return;
		int right_shift = 8 - _shift;
		int sign = m_pB_storage[m_i_used_bytes - 1] & 0x80;
		byte mask = 1;
		mask <<= _shift; mask --;
		int carryin = 0, carryout = 0;
		for (int i = 0; i < m_i_used_bytes; i ++) {
			carryout = (m_pB_storage[i]>>right_shift) & mask;
			m_pB_storage[i] = (m_pB_storage[i] << _shift) | carryin;
			carryin = carryout;
		}
		// extend with a byte if carry out
		if (sign && carryout == mask && (m_pB_storage[m_i_used_bytes - 1] & 0x80)) return;
		if (carryout > 0 || sign != (m_pB_storage[m_i_used_bytes - 1] & 0x80)) {
			extendUnsafeUsedBytesBy(1);

			if (sign) {
				for (int j = 7; j >= 0; j --) {
					if (carryout & (1 << j)) break;
					carryout |= (1<<j);
				}
			}

			m_pB_storage[m_i_used_bytes - 1] = carryout;
		}
	}
	void shiftRight(int _shift) {
		if (immutable) return;
		int left_shift = 8 - _shift;
		int sign = m_pB_storage[m_i_used_bytes - 1] & 0x80;
		byte mask = 1;
		mask <<= _shift; mask --; mask <<= left_shift;
		int carryin = sign?mask:0, carryout = 0;
		for (int i = m_i_used_bytes - 1; i >= 0; i --) {
			carryout = (m_pB_storage[i]<<left_shift) & mask;
			m_pB_storage[i] = ((m_pB_storage[i] >> _shift) & ~mask) | carryin;
			carryin = carryout;
		}
		// shorten with a byte if possible
		if (m_i_used_bytes <= 1) return;
		if (
				((m_pB_storage[m_i_used_bytes - 1] == 255) && (m_pB_storage[m_i_used_bytes - 2] & 0x80))
				||
				((m_pB_storage[m_i_used_bytes - 1] == 0) && !(m_pB_storage[m_i_used_bytes - 2] & 0x80))
			) {
			m_i_used_bytes --;
		}
	}
	/**
	 * Returns 128 if negative, 0 otherwise.
	 */
	int getSignBit() {
		return this->m_pB_storage[this->m_i_used_bytes - 1] & 0x080;
	}
	BigInteger * add(BigInteger * operand) {
		if (immutable) return NULL;
		int sign = this->getSignBit();
		int len = m_i_used_bytes;
		if (len < operand->m_i_used_bytes)
			this->extendSafeUsedBytesBy(operand->m_i_used_bytes - m_i_used_bytes);
		len = m_i_used_bytes;

		int tmp = m_pB_storage[0] & 0x0ff;
		tmp += (operand->m_pB_storage[0] & 0x0ff);
		int carryin = tmp >> 8;
		m_pB_storage[0] = tmp;

		for (int i = 1; i < len; i ++) {
			tmp = 0;
			if (i < operand->m_i_used_bytes) tmp = operand->m_pB_storage[i] & 0x0ff;
			else tmp = operand->getSignBit()?0x0ff:0x0;
			tmp += carryin;
			tmp += m_pB_storage[i];
			m_pB_storage[i] = tmp;
			carryin = tmp >> 8;
		}
		//if (carryin > 0) {
			extendUnsafeUsedBytesBy(1);
			m_pB_storage[m_i_used_bytes - 1] = carryin;
			m_pB_storage[m_i_used_bytes - 1] += operand->getSignBit()?0x0ff:0x0;
			m_pB_storage[m_i_used_bytes - 1] += sign?0x0ff:0x0;
		//}
		while ((m_i_used_bytes > 1)
				&& (
						(m_pB_storage[m_i_used_bytes - 1] == 0 && ((m_pB_storage[m_i_used_bytes - 2] & 0x80) == 0))
						||
						(m_pB_storage[m_i_used_bytes - 1] == byte(0xff) && ((m_pB_storage[m_i_used_bytes - 2] & 0x80)))
				)) {
			m_i_used_bytes --;
		}
		return this;
	}
	int equals (BigInteger &val) {
		return this->cmp(val) == 0;
	}
	int cmp (BigInteger& op) {
		int common = op.m_i_used_bytes;
		if (this->getSignBit() && !op.getSignBit()) return -1;
		if (!this->getSignBit() && op.getSignBit()) return 1;

		if (! this->getSignBit()) {
			if (this->m_i_used_bytes > op.m_i_used_bytes) {
				for (int i = this->m_i_used_bytes - 1; i >= op.m_i_used_bytes; i --) {
					if (this->m_pB_storage[i]) return 1;
				}
			}
			if (this->m_i_used_bytes < op.m_i_used_bytes) {
				for (int i = op.m_i_used_bytes - 1; i >= this->m_i_used_bytes; i --) {
					if (op.m_pB_storage[i]) return -1;
				}
				common = this->m_i_used_bytes;
			}
		} else {
			if (this->m_i_used_bytes > op.m_i_used_bytes) {
				for (int i = this->m_i_used_bytes - 1; i >= op.m_i_used_bytes; i --) {
					if (this->m_pB_storage[i] != 0xff) return -1;
				}
			}
			if (this->m_i_used_bytes < op.m_i_used_bytes) {
				for (int i = op.m_i_used_bytes - 1; i >= this->m_i_used_bytes; i --) {
					if (op.m_pB_storage[i] != 0xff) return 1;
				}
				common = this->m_i_used_bytes;
			}
		}
		for (int i = common - 1; i >= 0; i --) {
			if ((int(this->m_pB_storage[i])&0xff) > (int(op.m_pB_storage[i])&0xff)) return 1;
			if ((int(this->m_pB_storage[i])&0xff) < (int(op.m_pB_storage[i])&0xff)) return -1;
		}
		return 0;
	}
	void set(BigInteger* val) {
		if (immutable) return;
		delete this->m_pB_storage;
		this->m_i_storage_length = this->m_i_used_bytes = val->m_i_used_bytes;
		this->m_pB_storage = new byte[this->m_i_storage_length];
		memcpy(this->m_pB_storage, val->m_pB_storage, val->m_i_used_bytes);
	}
	/**
	 * Only for positive numbers
	 * Adds a sign bit
	 */
	int getBitsNb() {
		int i;
		if (! getSignBit()) {
			for (i = this->m_i_used_bytes - 1; i >= 0; i--) {
				if (this->m_pB_storage[i]) break;
			}
			if (i < 0) return 0 + 1;
			for (int b = 7; b >= 0; b--) {
				if (this->m_pB_storage[i] & (1<<b)) return b + i*8 + 1 + 1;
			}
			return 0;
		}

		for (i = this->m_i_used_bytes - 1; i >= 0; i--) {
			if (this->m_pB_storage[i] != byte(0xff)) break;
		}
		if (i < 0) return 0 + 1;
		for (int b = 7; b >= 0; b--) {
			if ((this->m_pB_storage[i] & (1<<b)) == 0) return b + i*8 + 1 + 1;
		}
		return 0;
	}
	void setBitInPositive(int i) {
		if (immutable) return;
		int byte = i >> 3;
		int bit = i & 0x07;
		int msb = (bit == 7)?1:0;
		if (this->m_i_used_bytes < byte + 1 + msb) {
			this->extendSafeUsedBytesBy(byte + 1 + msb - this->m_i_used_bytes);
		}
		this->m_pB_storage[byte] |= 1 << bit;
	}
	void clearBitInPositive(int i) {
		if (immutable) return;
		int byte = i >> 3;
		int bit = i & 0x07;
		int msb = (bit == 7)?1:0;
		if (this->m_i_used_bytes < byte + 1 + msb) {
			this->extendSafeUsedBytesBy(byte + 1 + msb - this->m_i_used_bytes);
		}
		this->m_pB_storage[byte] &= ~ ( 1 << bit );
	}
	int testBit(int i) {
		int byte = i >> 3;
		int bit = i & 0x07;
		if (this->m_i_used_bytes < byte + 1) {
			return this->getSignBit() ? 1 : 0;
		}
		return this->m_pB_storage[byte] & 1 << bit;
	}
	int getBufferSize(int radix = 10) {
		switch (radix) {
		case 10: return (this->getBitsNb()/10+1)*3 + 2;
		case 8: return (this->getBitsNb()/3+1) + 2;
		case 16: return this->m_i_used_bytes*2 + 2;
		case 2:
		default:
			return this->getBitsNb() + 2;
		}
	}
	char *  toString(char * _buffer, int len, int _radix = 10) {
		BigInteger radix(_radix, NULL);
		BigInteger divident(this);
		int idx = 1;
		if (divident.getSignBit()) divident.neg();
		_buffer[len - 1] = 0;
		do {
			//printf("%d str\n", idx);
			BigInteger q(0, ""), r(0, "");
			//printf("%d will divide %d : %d\n", idx, divident.intValue(), radix.intValue());
			divident.divide(& radix, & q, & r);
			//printf("%d: q=%d r=%d will set\n", idx, q.intValue(), r.intValue());
			divident.set(& q);
			int digit = r.intValue();
			//printf("r=%d will set\n", digit);
			if (digit < 10)
				_buffer[len - 1 - idx] = digit + '0';
			else
				_buffer[len - 1 - idx] = digit - 10 + 'A';
			idx ++;
			//printf("%d will compare\n", q.intValue());
		} while (divident.cmp(ZERO)>0);
		if (this->getSignBit()) {
			_buffer[len - 1 - idx++] = '-';
		}
		return _buffer + len - idx;
	}
	/**
	 *
	 */
	void divide (BigInteger* divisor, BigInteger* q, BigInteger* r) {
		if (immutable) return;
		if (divisor->getBitsNb() == 1) throw NULL;
		int bits_divident = getBitsNb();
		int bits_divisor = divisor->getBitsNb();
		q->setZero();
		if (cmp(*divisor) < 0) {
			r->set(this);
			return;
		}
		int i = bits_divident - bits_divisor;
		BigInteger divident(this);
		if ( i >= 8) { // this if is a lazy optimization attempt ;)
			BigInteger* probes[8];
			for ( int k = 0; k < 8; k ++)
				{ probes[k] = new BigInteger(divisor); probes[k]->shiftLeftBits(k); }
			for (; i >= 0; i --) {
				BigInteger probe(probes[i%8]);
				// TODO: may compare and subtract probe without shifting it, with a new dedicated function
				probe.shiftLeftBytes(i/8);
				int _cmp = divident.cmp(probe);
				//printf ("<div-M> i=%d cmp %d vs %d = %d\n", i, divident.intValue(), probe.intValue(), _cmp);
				if (_cmp >= 0) {
					probe.neg();
					divident.add(&probe);
					q->setBitInPositive(i);
				}
				//printf ("<div-M> -> i=%d  divi=%d probe=%d\n", i, divident.intValue(), probe.intValue());
			}
			for ( int k = 0; k < 8; k ++) { delete probes[k]; }
			r->set(&divident);
			return;
		}
		for (; i >= 0; i --) {
			BigInteger probe(divisor);
			probe.shiftLeftBits(i);
			int _cmp = divident.cmp(probe);
			//printf ("<div> i=%d cmp %d vs %d => %d\n", i, divident.intValue(), probe.intValue(), _cmp);
			if (_cmp >= 0) {
				probe.neg();
				divident.add(&probe);
				q->setBitInPositive(i);
			}
			//printf ("div %d cmp %d %d\n", i, divident.intValue(), probe.intValue());
		}
		r->set(&divident);
	}
	void or_bits (byte val) {
		if (immutable) return;
		this->m_pB_storage[0] |= val;
	}
	void or_bits (BigInteger& op) {
		if (immutable) return;
		if (op.m_i_used_bytes > this->m_i_used_bytes) {
			this->extendSafeUsedBytesBy(op.m_i_used_bytes - this->m_i_used_bytes);
		}
		for (int i = 0; i < op.m_i_used_bytes; i ++) {
			this->m_pB_storage[i] |= op.m_pB_storage[i];
		}
	}
	void and_bits (BigInteger& op) {
		if (immutable) return;
		if (op.m_i_used_bytes < this->m_i_used_bytes) {
			this->m_i_used_bytes = op.m_i_used_bytes;
		}
		for (int i = 0; i < this->m_i_used_bytes; i ++) {
			this->m_pB_storage[i] &= op.m_pB_storage[i];
		}
	}
	byte getByte(int b) {
		if (b >= 0 && b < this->m_i_used_bytes)
			return this->m_pB_storage[b];
		else
			return 0;
	}
	int intValue () {
		int value = 0;
		if ((m_pB_storage[m_i_used_bytes - 1] & 0x080) > 0) {
			value = -1;
		}
		for (int i = m_i_used_bytes - 1; i >= 0; i --) {
			if (i >= sizeof(int)) continue;
			value = value << 8;
			value += (m_pB_storage[i] & 0x0ff);
		}
		return value;
	}
	/**
	 * Called on a BigInteger. Assumed to have at least one byte in storage.
	 */
	void setZero() {
		if (immutable) return;
		if (m_i_storage_length <  1) return;
		m_pB_storage[0] = 0;
		m_i_used_bytes = 1;
	}

};



#endif /* BIGINTEGER_H_ */
