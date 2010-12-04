/*
  Copyright (C) 2006-2007 Werner Dittmann

  This program is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

/*
 * Authors: Werner Dittmann <Werner.Dittmann@t-online.de>
 */

#include <libzrtpcpp/ZrtpPacketHello.h>


ZrtpPacketHello::ZrtpPacketHello() {
    DEBUGOUT((fprintf(stdout, "Creating Hello packet without data\n")));

    // The NumSupported* data is in ZrtpTextData.h 
    nHash = NumSupportedHashes;
    nCipher = NumSupportedSymCiphers;
    nPubkey = NumSupportedPubKeys;
    nSas = NumSupportedSASTypes;
    nAuth = NumSupportedAuthLenghts;

    // length is fixed Header plus HMAC size (2*ZRTP_WORD_SIZE)
    int32_t length = sizeof(HelloPacket_t) + (2 * ZRTP_WORD_SIZE);
    length += nHash * ZRTP_WORD_SIZE;
    length += nCipher * ZRTP_WORD_SIZE;
    length += nPubkey * ZRTP_WORD_SIZE;
    length += nSas * ZRTP_WORD_SIZE;
    length += nAuth * ZRTP_WORD_SIZE;

    // Don't change order of this sequence
    oHash = sizeof(Hello_t);
    oCipher = oHash + (nHash * ZRTP_WORD_SIZE);
    oAuth = oCipher + (nCipher * ZRTP_WORD_SIZE);
    oPubkey = oAuth + (nAuth * ZRTP_WORD_SIZE);
    oSas = oPubkey + (nPubkey * ZRTP_WORD_SIZE);
    oHmac = oSas + (nSas * ZRTP_WORD_SIZE);         // offset to HMAC

    void* allocated = &data;
    memset(allocated, 0, sizeof(data));

    zrtpHeader = (zrtpPacketHeader_t *)&((HelloPacket_t *)allocated)->hdr;	// the standard header
    helloHeader = (Hello_t *)&((HelloPacket_t *)allocated)->hello;

    setZrtpId();

    // minus 1 for CRC size 
    setLength(length / ZRTP_WORD_SIZE);
    setMessageType((uint8_t*)HelloMsg);

    setVersion((uint8_t*)zrtpVersion);

    uint32_t lenField = nHash << 16;
    for (int32_t i = 0; i < nHash; i++) {
        setHashType(i, (int8_t*)supportedHashes[i]);
    }

    lenField |= nCipher << 12;
    for (int32_t i = 0; i < nCipher; i++) {
        setCipherType(i,  (int8_t*)supportedCipher[i]);
    }

    lenField |= nAuth << 8;
    for (int32_t i = 0; i < nAuth; i++) {
        setAuthLen(i,  (int8_t*)supportedAuthLen[i]);
    }

    lenField |= nPubkey << 4;
    for (int32_t i = 0; i < nPubkey; i++) {
        setPubKeyType(i,  (int8_t*)supportedPubKey[i]);
    }

    lenField |= nSas;
    for (int32_t i = 0; i < nSas; i++) {
        setSasType(i,  (int8_t*)supportedSASType[i]);
    }
    helloHeader->flagLength = htonl(lenField);
}

ZrtpPacketHello::ZrtpPacketHello(uint8_t *data) {
    DEBUGOUT((fprintf(stdout, "Creating Hello packet from data\n")));

    zrtpHeader = (zrtpPacketHeader_t *)&((HelloPacket_t *)data)->hdr;	// the standard header
    helloHeader = (Hello_t *)&((HelloPacket_t *)data)->hello;

    uint32_t temp = ntohl(helloHeader->flagLength);

    nHash = (temp & (0xf << 16)) >> 16;
    nCipher = (temp & (0xf << 12)) >> 12;
    nAuth = (temp & (0xf << 8)) >> 8;
    nPubkey = (temp & (0xf << 4)) >> 4;
    nSas = temp & 0xf;

    oHash = sizeof(Hello_t);
    oCipher = oHash + (nHash * ZRTP_WORD_SIZE);
    oAuth = oCipher + (nCipher * ZRTP_WORD_SIZE);
    oPubkey = oAuth + (nAuth * ZRTP_WORD_SIZE);
    oSas = oPubkey + (nPubkey * ZRTP_WORD_SIZE);
    oHmac = oSas + (nSas * ZRTP_WORD_SIZE);         // offset to HMAC
}

ZrtpPacketHello::~ZrtpPacketHello() {
    DEBUGOUT((fprintf(stdout, "Deleting Hello packet: alloc: %x\n", allocated)));
}