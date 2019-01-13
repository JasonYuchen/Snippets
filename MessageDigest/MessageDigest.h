#pragma once
#include <string>
#include <cstring>
#include <vector>

namespace MessageDigest
{

using std::string;
using std::vector;

/*
* Original Author: JieweiWei
* Modified by: JasonYuchen
* License: Apache 2.0
*/

constexpr uint32_t
s11 = 7,
s12 = 12,
s13 = 17,
s14 = 22,
s21 = 5,
s22 = 9,
s23 = 14,
s24 = 20,
s31 = 4,
s32 = 11,
s33 = 16,
s34 = 23,
s41 = 6,
s42 = 10,
s43 = 15,
s44 = 21;

inline constexpr uint32_t F(uint32_t x, uint32_t y, uint32_t z) {
    return (((x) & (y)) | ((~x) & (z)));
}
inline constexpr uint32_t G(uint32_t x, uint32_t y, uint32_t z) {
    return (((x) & (z)) | ((y) & (~z)));
}
inline constexpr uint32_t H(uint32_t x, uint32_t y, uint32_t z) {
    return ((x) ^ (y) ^ (z));
}
inline constexpr uint32_t I(uint32_t x, uint32_t y, uint32_t z) {
    return ((y) ^ ((x) | (~z)));
}

inline constexpr uint32_t ROTATELEFT(uint32_t num, uint32_t n) {
    return (((num) << (n)) | ((num) >> (32 - (n))));
}

inline constexpr void FF(uint32_t &a, uint32_t b, uint32_t c, uint32_t d, uint32_t x, uint32_t s, uint32_t ac) {
    (a) += F((b), (c), (d)) + (x)+ac;
    (a) = ROTATELEFT((a), (s));
    (a) += (b);
}
inline constexpr void GG(uint32_t &a, uint32_t b, uint32_t c, uint32_t d, uint32_t x, uint32_t s, uint32_t ac) {
    (a) += G((b), (c), (d)) + (x)+ac;
    (a) = ROTATELEFT((a), (s));
    (a) += (b);
}
inline constexpr void HH(uint32_t &a, uint32_t b, uint32_t c, uint32_t d, uint32_t x, uint32_t s, uint32_t ac) {
    (a) += H((b), (c), (d)) + (x)+ac;
    (a) = ROTATELEFT((a), (s));
    (a) += (b);
}
inline constexpr void II(uint32_t &a, uint32_t b, uint32_t c, uint32_t d, uint32_t x, uint32_t s, uint32_t ac) {
    (a) += I((b), (c), (d)) + (x)+ac;
    (a) = ROTATELEFT((a), (s));
    (a) += (b);
}


class MD5 {
public:
    /*
    * @Generate a MD5 instance with lazy evaluation
    *
    * @example: MD5("test MD5 algorithm").digest().toString()
    */
    MD5(const string& message) {
        finished = false;
        haveString = false;
        /* Reset number of bits. */
        count[0] = count[1] = 0;
        /* Initialization constants. */
        state[0] = 0x67452301;
        state[1] = 0xefcdab89;
        state[2] = 0x98badcfe;
        state[3] = 0x10325476;

        init((const uint8_t*)message.c_str(), message.length());
    }


    vector<uint8_t> digest() {
        if (finished)
            return result_vec;

        finished = true;

        uint8_t bits[8];
        uint32_t oldState[4];
        uint32_t oldCount[2];
        uint32_t index, padLen;

        /* Save current state and count. */
        memcpy(oldState, state, 16);
        memcpy(oldCount, count, 8);

        /* Save number of bits */
        encode(count, bits, 8);

        /* Pad out to 56 mod 64. */
        index = (uint32_t)((count[0] >> 3) & 0x3f);
        padLen = (index < 56) ? (56 - index) : (120 - index);
        init(PADDING, padLen);

        /* Append length (before padding) */
        init(bits, 8);

        /* Store state in digest */
        encode(state, result, 16);

        /* Restore current state and count. */
        memcpy(state, oldState, 16);
        memcpy(count, oldCount, 8);

        result_vec = vector<uint8_t>(&result[0], &result[16]);

        return result_vec;
    }

    /*
    * @Convert digest to string value.
    *
    * @return the hex string of digest.
    */
    string toString() {
        if (haveString)
            return result_str;

        haveString = true;

        digest();
        result_str.reserve(16 << 1);
        for (const auto &val : result) {
            int a = val / 16;
            int b = val % 16;
            result_str.append(1, HEX_NUMBERS[a]);
            result_str.append(1, HEX_NUMBERS[b]);
        }
        return result_str;
    }

private:
    /*
    * @Initialization the md5 object, processing another message block,
    * and updating the context.
    *
    * @param {input} the input message.
    *
    * @param {len} the number btye of message.
    */
    void init(const uint8_t* input, size_t len) {

        uint32_t i, index, partLen;

        finished = false;

        /* Compute number of bytes mod 64 */
        index = (uint32_t)((count[0] >> 3) & 0x3f);

        /* update number of bits */
        if ((count[0] += ((uint32_t)len << 3)) < ((uint32_t)len << 3)) {
            ++count[1];
        }
        count[1] += ((uint32_t)len >> 29);

        partLen = 64 - index;

        /* transform as many times as possible. */
        if (len >= partLen) {

            memcpy(&buffer[index], input, partLen);
            transform(buffer);

            for (i = partLen; i + 63 < len; i += 64) {
                transform(&input[i]);
            }
            index = 0;

        }
        else {
            i = 0;
        }

        /* Buffer remaining input */
        memcpy(&buffer[index], &input[i], len - i);
    }

    /*
    * @MD5 basic transformation. Transforms state based on block.
    *
    * @param {block} the message block.
    */
    void transform(const uint8_t block[64]) {

        uint32_t a = state[0], b = state[1], c = state[2], d = state[3], x[16];

        decode(block, x, 64);

        /* Round 1 */
        FF(a, b, c, d, x[0], s11, 0xd76aa478U);
        FF(d, a, b, c, x[1], s12, 0xe8c7b756U);
        FF(c, d, a, b, x[2], s13, 0x242070dbU);
        FF(b, c, d, a, x[3], s14, 0xc1bdceeeU);
        FF(a, b, c, d, x[4], s11, 0xf57c0fafU);
        FF(d, a, b, c, x[5], s12, 0x4787c62aU);
        FF(c, d, a, b, x[6], s13, 0xa8304613U);
        FF(b, c, d, a, x[7], s14, 0xfd469501U);
        FF(a, b, c, d, x[8], s11, 0x698098d8U);
        FF(d, a, b, c, x[9], s12, 0x8b44f7afU);
        FF(c, d, a, b, x[10], s13, 0xffff5bb1U);
        FF(b, c, d, a, x[11], s14, 0x895cd7beU);
        FF(a, b, c, d, x[12], s11, 0x6b901122U);
        FF(d, a, b, c, x[13], s12, 0xfd987193U);
        FF(c, d, a, b, x[14], s13, 0xa679438eU);
        FF(b, c, d, a, x[15], s14, 0x49b40821U);

        /* Round 2 */
        GG(a, b, c, d, x[1], s21, 0xf61e2562U);
        GG(d, a, b, c, x[6], s22, 0xc040b340U);
        GG(c, d, a, b, x[11], s23, 0x265e5a51U);
        GG(b, c, d, a, x[0], s24, 0xe9b6c7aaU);
        GG(a, b, c, d, x[5], s21, 0xd62f105dU);
        GG(d, a, b, c, x[10], s22, 0x02441453U);
        GG(c, d, a, b, x[15], s23, 0xd8a1e681U);
        GG(b, c, d, a, x[4], s24, 0xe7d3fbc8U);
        GG(a, b, c, d, x[9], s21, 0x21e1cde6U);
        GG(d, a, b, c, x[14], s22, 0xc33707d6U);
        GG(c, d, a, b, x[3], s23, 0xf4d50d87U);
        GG(b, c, d, a, x[8], s24, 0x455a14edU);
        GG(a, b, c, d, x[13], s21, 0xa9e3e905U);
        GG(d, a, b, c, x[2], s22, 0xfcefa3f8U);
        GG(c, d, a, b, x[7], s23, 0x676f02d9U);
        GG(b, c, d, a, x[12], s24, 0x8d2a4c8aU);

        /* Round 3 */
        HH(a, b, c, d, x[5], s31, 0xfffa3942U);
        HH(d, a, b, c, x[8], s32, 0x8771f681U);
        HH(c, d, a, b, x[11], s33, 0x6d9d6122U);
        HH(b, c, d, a, x[14], s34, 0xfde5380cU);
        HH(a, b, c, d, x[1], s31, 0xa4beea44U);
        HH(d, a, b, c, x[4], s32, 0x4bdecfa9U);
        HH(c, d, a, b, x[7], s33, 0xf6bb4b60U);
        HH(b, c, d, a, x[10], s34, 0xbebfbc70U);
        HH(a, b, c, d, x[13], s31, 0x289b7ec6U);
        HH(d, a, b, c, x[0], s32, 0xeaa127faU);
        HH(c, d, a, b, x[3], s33, 0xd4ef3085U);
        HH(b, c, d, a, x[6], s34, 0x04881d05U);
        HH(a, b, c, d, x[9], s31, 0xd9d4d039U);
        HH(d, a, b, c, x[12], s32, 0xe6db99e5U);
        HH(c, d, a, b, x[15], s33, 0x1fa27cf8U);
        HH(b, c, d, a, x[2], s34, 0xc4ac5665U);

        /* Round 4 */
        II(a, b, c, d, x[0], s41, 0xf4292244U);
        II(d, a, b, c, x[7], s42, 0x432aff97U);
        II(c, d, a, b, x[14], s43, 0xab9423a7U);
        II(b, c, d, a, x[5], s44, 0xfc93a039U);
        II(a, b, c, d, x[12], s41, 0x655b59c3U);
        II(d, a, b, c, x[3], s42, 0x8f0ccc92U);
        II(c, d, a, b, x[10], s43, 0xffeff47dU);
        II(b, c, d, a, x[1], s44, 0x85845dd1U);
        II(a, b, c, d, x[8], s41, 0x6fa87e4fU);
        II(d, a, b, c, x[15], s42, 0xfe2ce6e0U);
        II(c, d, a, b, x[6], s43, 0xa3014314U);
        II(b, c, d, a, x[13], s44, 0x4e0811a1U);
        II(a, b, c, d, x[4], s41, 0xf7537e82U);
        II(d, a, b, c, x[11], s42, 0xbd3af235U);
        II(c, d, a, b, x[2], s43, 0x2ad7d2bbU);
        II(b, c, d, a, x[9], s44, 0xeb86d391U);

        state[0] += a;
        state[1] += b;
        state[2] += c;
        state[3] += d;
    }


    /*
    * @Encodes input (unsigned long) into output (byte).
    *
    * @param {input} usigned long.
    *
    * @param {output} byte.
    *
    * @param {length} the length of input.
    */
    void encode(const uint32_t* input, uint8_t* output, size_t length) {

        for (size_t i = 0, j = 0; j < length; ++i, j += 4) {
            output[j] = (uint8_t)(input[i] & 0xff);
            output[j + 1] = (uint8_t)((input[i] >> 8) & 0xff);
            output[j + 2] = (uint8_t)((input[i] >> 16) & 0xff);
            output[j + 3] = (uint8_t)((input[i] >> 24) & 0xff);
        }
    }

    /*
    * @Decodes input (byte) into output (usigned long).
    *
    * @param {input} bytes.
    *
    * @param {output} unsigned long.
    *
    * @param {length} the length of input.
    */
    void decode(const uint8_t* input, uint32_t* output, size_t length) {
        for (size_t i = 0, j = 0; j < length; ++i, j += 4) {
            output[i] = ((uint32_t)input[j]) | (((uint32_t)input[j + 1]) << 8) |
                (((uint32_t)input[j + 2]) << 16) | (((uint32_t)input[j + 3]) << 24);
        }
    }


private:
    /* Flag for mark whether calculate finished. */
    bool finished;
    bool haveString;

    /* state (ABCD). */
    uint32_t state[4];

    /* number of bits, low-order word first. */
    uint32_t count[2];

    /* input buffer. */
    uint8_t buffer[64];

    /* message digest. */
    uint8_t result[16];
    vector<uint8_t> result_vec;
    string result_str;

    /* padding for calculate. */
    static const uint8_t PADDING[64];

    /* Hex numbers. */
    static const char HEX_NUMBERS[16];
};

const uint8_t MD5::PADDING[64] = { 0x80 };
const char MD5::HEX_NUMBERS[16] = {
    '0', '1', '2', '3',
    '4', '5', '6', '7',
    '8', '9', 'a', 'b',
    'c', 'd', 'e', 'f'
};

}