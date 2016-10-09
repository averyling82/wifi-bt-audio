/*
 * Copyright 2015, Broadcom Corporation
 * All Rights Reserved.
 *
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom Corporation.
 */

/* Originally taken from TropicSSL
 * https://gitorious.org/tropicssl/
 * commit: 92bb3462dfbdb4568c92be19e8904129a17b1eed
 * Whitespace converted (Tab to 4 spaces, LF to CRLF)
 * int arguments/returns changed to int32_t
 * change malloc/free to tls_host_malloc/tls_host_free
 * remove sscanf usage
 * add option for generalized time
 * add const to arguments where appropriate
 * fix base64_decode usage
 * add better hash_algorithm parsing
 * add typecasting where required
 * remove x509parse_crtfile, x509parse_crtfile and x509parse_keyfile
 * segregate functions which use sprintf
 * add support for RSA_SHA256, RSA_SHA384, RSA_SHA512 hashes
 * replace time() with tls_host_get_time_ms()
 * fix certificate chain verification in x509parse_verify()
 * add function x509parse_pubkey() to parse a public rsa key
 * add support for generalized time
 */

/*
 *    X.509 certificate and private key decoding
 *
 *    Based on XySSL: Copyright (C) 2006-2008     Christophe Devine
 *
 *    Copyright (C) 2009    Paul Bakker <polarssl_maintainer at polarssl dot org>
 *
 *    All rights reserved.
 *
 *    Redistribution and use in source and binary forms, with or without
 *    modification, are permitted provided that the following conditions
 *    are met:
 *
 *      * Redistributions of source code must retain the above copyright
 *        notice, this list of conditions and the following disclaimer.
 *      * Redistributions in binary form must reproduce the above copyright
 *        notice, this list of conditions and the following disclaimer in the
 *        documentation and/or other materials provided with the distribution.
 *      * Neither the names of PolarSSL or XySSL nor the names of its contributors
 *        may be used to endorse or promote products derived from this software
 *        without specific prior written permission.
 *
 *    THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 *    "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 *    LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 *    FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 *    OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 *    SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
 *    TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 *    PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 *    LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 *    NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 *    SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
/*
 *    The ITU-T X.509 standard defines a certificat format for PKI.
 *
 *    http://www.ietf.org/rfc/rfc2459.txt
 *    http://www.ietf.org/rfc/rfc3279.txt
 *
 *    ftp://ftp.rsasecurity.com/pub/pkcs/ascii/pkcs-1v2.asc
 *
 *    http://www.itu.int/ITU-T/studygroups/com17/languages/X.680-0207.pdf
 *    http://www.itu.int/ITU-T/studygroups/com17/languages/X.690-0207.pdf
 */

#include "tls_host_api.h"
#include "x509.h"
#include "base64.h"
#include "des.h"
#if defined(TROPICSSL_MD2_C)
#include "md2.h"
#endif /* if defined(TROPICSSL_MD2_C) */
#include "md5.h"
#include "sha1.h"
#include "sha2.h"
#include "sha4.h"
#include "bignum.h"

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>

/*
 * ASN.1 DER decoding routines
 */
static int32_t asn1_get_len(const unsigned char **p, const unsigned char *end, uint32_t *len)
{
    if ((end - *p) < 1)
        return (TROPICSSL_ERR_ASN1_OUT_OF_DATA);

    if ((**p & 0x80) == 0)
        *len = *(*p)++;
    else {
        switch (**p & 0x7F) {
        case 1:
            if ((end - *p) < 2)
                return (TROPICSSL_ERR_ASN1_OUT_OF_DATA);

            *len = (*p)[1];
            (*p) += 2;
            break;

        case 2:
            if ((end - *p) < 3)
                return (TROPICSSL_ERR_ASN1_OUT_OF_DATA);

            *len = ((*p)[1] << 8) | (*p)[2];
            (*p) += 3;
            break;

        default:
            return (TROPICSSL_ERR_ASN1_INVALID_LENGTH);
            break;
        }
    }

    if (*len > (int)(end - *p))
        return (TROPICSSL_ERR_ASN1_OUT_OF_DATA);

    return (0);
}

static int32_t asn1_get_tag(const unsigned char **p,
            const unsigned char *end, uint32_t *len, int32_t tag)
{
    if ((end - *p) < 1)
        return (TROPICSSL_ERR_ASN1_OUT_OF_DATA);

    if (**p != tag)
        return (TROPICSSL_ERR_ASN1_UNEXPECTED_TAG);

    (*p)++;

    return (asn1_get_len(p, end, len));
}

static int32_t asn1_get_bool(const unsigned char **p, const unsigned char *end, int32_t *val)
{
    int32_t ret;
    uint32_t len;

    if ((ret = asn1_get_tag(p, end, &len, ASN1_BOOLEAN)) != 0)
        return (ret);

    if (len != 1)
        return (TROPICSSL_ERR_ASN1_INVALID_LENGTH);

    *val = (**p != 0) ? 1 : 0;
    (*p)++;

    return (0);
}

static int32_t asn1_get_int(const unsigned char **p, const unsigned char *end, int32_t *val)
{
    int32_t ret;
    uint32_t len;

    if ((ret = asn1_get_tag(p, end, &len, ASN1_INTEGER)) != 0)
        return (ret);

    if (len > (int)sizeof(int32_t) || (**p & 0x80) != 0)
        return (TROPICSSL_ERR_ASN1_INVALID_LENGTH);

    *val = 0;

    while (len-- > 0) {
        *val = (*val << 8) | **p;
        (*p)++;
    }

    return (0);
}

static int32_t asn1_get_mpi(const unsigned char **p, const unsigned char *end, mpi *X)
{
    int32_t ret;
    uint32_t len;

    if ((ret = asn1_get_tag(p, end, &len, ASN1_INTEGER)) != 0)
        return (ret);

    ret = mpi_read_binary(X, *p, len);

    *p += len;

    return (ret);
}

/*
 *    Version     ::=  INTEGER  {  v1(0), v2(1), v3(2)  }
 */
static int32_t x509_get_version(const unsigned char **p, const unsigned char *end, int32_t *ver)
{
    int32_t ret;
    uint32_t len;

    if ((ret = asn1_get_tag(p, end, &len,
                ASN1_CONTEXT_SPECIFIC | ASN1_CONSTRUCTED | 0))
        != 0) {
        if (ret == TROPICSSL_ERR_ASN1_UNEXPECTED_TAG)
            return (*ver = 0);

        return (ret);
    }

    end = *p + len;

    if ((ret = asn1_get_int(p, end, ver)) != 0)
        return (TROPICSSL_ERR_X509_CERT_INVALID_VERSION | ret);

    if (*p != end)
        return (TROPICSSL_ERR_X509_CERT_INVALID_VERSION |
            TROPICSSL_ERR_ASN1_LENGTH_MISMATCH);

    return (0);
}

/*
 *    CertificateSerialNumber     ::=  INTEGER
 */
static int32_t x509_get_serial(const unsigned char **p,
               const unsigned char *end, x509_buf * serial)
{
    int32_t ret;

    if ((end - *p) < 1)
        return (TROPICSSL_ERR_X509_CERT_INVALID_SERIAL |
            TROPICSSL_ERR_ASN1_OUT_OF_DATA);

    if (**p != (ASN1_CONTEXT_SPECIFIC | ASN1_PRIMITIVE | 2) &&
        **p != ASN1_INTEGER)
        return (TROPICSSL_ERR_X509_CERT_INVALID_SERIAL |
            TROPICSSL_ERR_ASN1_UNEXPECTED_TAG);

    serial->tag = *(*p)++;

    if ((ret = asn1_get_len(p, end, &serial->len)) != 0)
        return (TROPICSSL_ERR_X509_CERT_INVALID_SERIAL | ret);

    serial->p = *p;
    *p += serial->len;

    return (0);
}

/*
 *    AlgorithmIdentifier     ::=  SEQUENCE    {
 *         algorithm                 OBJECT IDENTIFIER,
 *         parameters                 ANY DEFINED BY algorithm OPTIONAL    }
 */
static int32_t x509_get_alg(const unsigned char **p, const unsigned char *end, x509_buf *alg)
{
    int32_t ret;
    uint32_t len;

    if ((ret = asn1_get_tag(p, end, &len,
                ASN1_CONSTRUCTED | ASN1_SEQUENCE)) != 0)
        return (TROPICSSL_ERR_X509_CERT_INVALID_ALG | ret);

    end = *p + len;
    alg->tag = **p;

    if ((ret = asn1_get_tag(p, end, &alg->len, ASN1_OID)) != 0)
        return (TROPICSSL_ERR_X509_CERT_INVALID_ALG | ret);

    alg->p = *p;
    *p += alg->len;

    if (*p == end)
        return (0);

    /*
     * assume the algorithm parameters must be NULL
     */
    if ((ret = asn1_get_tag(p, end, &len, ASN1_NULL)) != 0)
        return (TROPICSSL_ERR_X509_CERT_INVALID_ALG | ret);

    if (*p != end)
        return (TROPICSSL_ERR_X509_CERT_INVALID_ALG |
            TROPICSSL_ERR_ASN1_LENGTH_MISMATCH);

    return (0);
}

/*
 *    RelativeDistinguishedName ::=
 *      SET OF AttributeTypeAndValue
 *
 *    AttributeTypeAndValue ::= SEQUENCE {
 *      type       AttributeType,
 *      value       AttributeValue }
 *
 *    AttributeType ::= OBJECT IDENTIFIER
 *
 *    AttributeValue ::= ANY DEFINED BY AttributeType
 */
static int32_t x509_get_name(const unsigned char **p, const unsigned char *end, x509_name *cur)
{
    int32_t ret;
    uint32_t len;
    const unsigned char *end2;
    x509_buf *oid;
    x509_buf *val;

    if ((ret = asn1_get_tag(p, end, &len,
                ASN1_CONSTRUCTED | ASN1_SET)) != 0)
        return (TROPICSSL_ERR_X509_CERT_INVALID_NAME | ret);

    end2 = end;
    end = *p + len;

    if ((ret = asn1_get_tag(p, end, &len,
                ASN1_CONSTRUCTED | ASN1_SEQUENCE)) != 0)
        return (TROPICSSL_ERR_X509_CERT_INVALID_NAME | ret);

    if (*p + len != end)
        return (TROPICSSL_ERR_X509_CERT_INVALID_NAME |
            TROPICSSL_ERR_ASN1_LENGTH_MISMATCH);

    oid = &cur->oid;
    oid->tag = **p;

    if ((ret = asn1_get_tag(p, end, &oid->len, ASN1_OID)) != 0)
        return (TROPICSSL_ERR_X509_CERT_INVALID_NAME | ret);

    oid->p = *p;
    *p += oid->len;

    if ((end - *p) < 1)
        return (TROPICSSL_ERR_X509_CERT_INVALID_NAME |
            TROPICSSL_ERR_ASN1_OUT_OF_DATA);

    if (**p != ASN1_BMP_STRING && **p != ASN1_UTF8_STRING &&
        **p != ASN1_T61_STRING && **p != ASN1_PRINTABLE_STRING &&
        **p != ASN1_IA5_STRING && **p != ASN1_UNIVERSAL_STRING)
        return (TROPICSSL_ERR_X509_CERT_INVALID_NAME |
            TROPICSSL_ERR_ASN1_UNEXPECTED_TAG);

    val = &cur->val;
    val->tag = *(*p)++;

    if ((ret = asn1_get_len(p, end, &val->len)) != 0)
        return (TROPICSSL_ERR_X509_CERT_INVALID_NAME | ret);

    val->p = *p;
    *p += val->len;

    cur->next = NULL;

    if (*p != end)
        return (TROPICSSL_ERR_X509_CERT_INVALID_NAME |
            TROPICSSL_ERR_ASN1_LENGTH_MISMATCH);

    /*
     * recurse until end of SEQUENCE is reached
     */
    if (*p == end2)
        return (0);

    cur->next = (x509_name *) tls_host_malloc ("x509", sizeof( x509_name ) );

    if (cur->next == NULL)
        return (1);

    return (x509_get_name(p, end2, cur->next));
}

static inline char* parse_two_digit_decimal_string_uint16( char* string, uint16_t *output )
{
    *output = (string[0] -'0')*10 + (string[1] -'0');
    return string + 2;
}

static inline char* parse_two_digit_decimal_string_uint8( char* string, uint8_t *output )
{
    *output = (string[0] -'0')*10 + (string[1] -'0');
    return string + 2;
}

static int32_t parse_x509_DateTime( char* string, x509_time *datetime, uint8_t short_year )
{
    char* curr = string;

    curr = parse_two_digit_decimal_string_uint16( curr, &datetime->year );

    if ( short_year == 0 )
    {
        /* Long year representation */
        uint16_t tmp_year = datetime->year;
        curr = parse_two_digit_decimal_string_uint16( curr, &datetime->year );
        datetime->year += tmp_year * 100;
    }
    curr = parse_two_digit_decimal_string_uint8(  curr, &datetime->mon );
    curr = parse_two_digit_decimal_string_uint8(  curr, &datetime->day );
    curr = parse_two_digit_decimal_string_uint8(  curr, &datetime->hour );
    curr = parse_two_digit_decimal_string_uint8(  curr, &datetime->min );
    curr = parse_two_digit_decimal_string_uint8(  curr, &datetime->sec );

    if ( ( ( datetime->year >= 100 ) && ( short_year != 0 ) ) ||
         ( datetime->mon > 12 ) ||
         ( datetime->mon == 0 ) ||
         ( datetime->day > 31 ) ||
         ( datetime->day == 0 ) ||
         ( datetime->hour > 24 ) ||
         ( datetime->min  > 60 ) ||
         ( datetime->sec  > 60 ) )
    {
        memset( datetime,  0, sizeof( *datetime ) );
        return ( TROPICSSL_ERR_X509_CERT_INVALID_DATE );
    }

    if ( short_year != 0 )
    {
        /* Adjust year to be between 1990 and 2089 */
        datetime->year +=  100 * ( datetime->year < 50 );
        datetime->year += 1900;
    }

    return ( 0 );
}


/*
 *    Validity ::= SEQUENCE {
 *         notBefore        Time,
 *         notAfter        Time }
 *
 *    Time ::= CHOICE {
 *         utcTime        UTCTime,
 *         generalTime    GeneralizedTime }
 */
static int32_t x509_get_dates(const unsigned char **p,
              const unsigned char *end, x509_time * from, x509_time * to)
{
    int32_t ret;
    uint32_t len;
    char date[64];
    uint8_t short_year = 0;

    if ((ret = asn1_get_tag(p, end, &len,
                ASN1_CONSTRUCTED | ASN1_SEQUENCE)) != 0)
        return (TROPICSSL_ERR_X509_CERT_INVALID_DATE | ret);

    end = *p + len;

    /*
     * TODO: also handle GeneralizedTime
     */
    if( ( ( ret = asn1_get_tag( p, end, &len, ASN1_UTC_TIME ) ) == 0 ) || ( len != 13 ) )
    {
        short_year = 1;
    }
    else if( ( ( ret = asn1_get_tag( p, end, &len, ASN1_GENERALISED_TIME ) ) != 0 ) || ( len != 15 ) )
    {
        return (TROPICSSL_ERR_X509_CERT_INVALID_DATE | ret);
    }

    memset(date, 0, sizeof(date));
    memcpy(date, *p, (len < (int)sizeof(date) - 1) ?
           len : (int)sizeof(date) - 1);

    ret = parse_x509_DateTime( date, from, short_year );
    if ( ret != 0 )
    {
        return ret;
    }

    *p += len;

    short_year = 0;
    if( ( ( ret = asn1_get_tag( p, end, &len, ASN1_UTC_TIME ) ) == 0 ) || ( len != 13 ) )
    {
        short_year = 1;
    }
    else if( ( ( ret = asn1_get_tag( p, end, &len, ASN1_GENERALISED_TIME ) ) != 0 ) || ( len != 15 ) )
    {
        return (TROPICSSL_ERR_X509_CERT_INVALID_DATE | ret);
    }

    memset(date, 0, sizeof(date));
    memcpy(date, *p, (len < (int)sizeof(date) - 1) ?
           len : (int)sizeof(date) - 1);

    ret = parse_x509_DateTime( date, to, short_year );
    if ( ret != 0 )
    {
        return ret;
    }

    *p += len;

    if (*p != end)
        return (TROPICSSL_ERR_X509_CERT_INVALID_DATE |
            TROPICSSL_ERR_ASN1_LENGTH_MISMATCH);

    return (0);
}

/*
 *    SubjectPublicKeyInfo  ::=  SEQUENCE     {
 *         algorithm              AlgorithmIdentifier,
 *         subjectPublicKey      BIT STRING }
 */
static int32_t x509_get_pubkey( const unsigned char **p,
               const unsigned char *end,
               x509_buf * pk_alg_oid, mpi * N, mpi * E)
{
    int32_t ret;
    uint32_t len;
    const unsigned char *end2;

    if ((ret = x509_get_alg(p, end, pk_alg_oid)) != 0)
        return (ret);

    /*
     * only RSA public keys handled at this time
     */
    if (pk_alg_oid->len != 9 ||
        memcmp(pk_alg_oid->p, OID_PKCS1_RSA, 9) != 0)
        return (TROPICSSL_ERR_X509_CERT_UNKNOWN_PK_ALG);

    if ((ret = asn1_get_tag(p, end, &len, ASN1_BIT_STRING)) != 0)
        return (TROPICSSL_ERR_X509_CERT_INVALID_PUBKEY | ret);

    if ((end - *p) < 1)
        return (TROPICSSL_ERR_X509_CERT_INVALID_PUBKEY |
            TROPICSSL_ERR_ASN1_OUT_OF_DATA);

    end2 = *p + len;

    if (*(*p)++ != 0)
        return (TROPICSSL_ERR_X509_CERT_INVALID_PUBKEY);

    /*
     *      RSAPublicKey ::= SEQUENCE {
     *              modulus                   INTEGER,      -- n
     *              publicExponent    INTEGER       -- e
     *      }
     */
    if ((ret = asn1_get_tag(p, end2, &len,
                ASN1_CONSTRUCTED | ASN1_SEQUENCE)) != 0)
        return (TROPICSSL_ERR_X509_CERT_INVALID_PUBKEY | ret);

    if (*p + len != end2)
        return (TROPICSSL_ERR_X509_CERT_INVALID_PUBKEY |
            TROPICSSL_ERR_ASN1_LENGTH_MISMATCH);

    if ((ret = asn1_get_mpi(p, end2, N)) != 0 ||
        (ret = asn1_get_mpi(p, end2, E)) != 0)
        return (TROPICSSL_ERR_X509_CERT_INVALID_PUBKEY | ret);

    if (*p != end)
        return (TROPICSSL_ERR_X509_CERT_INVALID_PUBKEY |
            TROPICSSL_ERR_ASN1_LENGTH_MISMATCH);

    return (0);
}

static int32_t x509_get_sig(const unsigned char **p, const unsigned char *end, x509_buf *sig)
{
    int32_t ret;
    uint32_t len;

    sig->tag = **p;

    if ((ret = asn1_get_tag(p, end, &len, ASN1_BIT_STRING)) != 0)
        return (TROPICSSL_ERR_X509_CERT_INVALID_SIGNATURE | ret);

    if (--len < 1 || *(*p)++ != 0)
        return (TROPICSSL_ERR_X509_CERT_INVALID_SIGNATURE);

    sig->len = len;
    sig->p = *p;

    *p += len;

    return (0);
}

/*
 * X.509 v2/v3 unique identifier (not parsed)
 */
static int32_t x509_get_uid(const unsigned char **p,
            const unsigned char *end, x509_buf *uid, int32_t n)
{
    int32_t ret;

    if (*p == end)
        return (0);

    uid->tag = **p;

    if ((ret = asn1_get_tag(p, end, &uid->len,
                ASN1_CONTEXT_SPECIFIC | ASN1_CONSTRUCTED | n))
        != 0) {
        if (ret == TROPICSSL_ERR_ASN1_UNEXPECTED_TAG)
            return (0);

        return (ret);
    }

    uid->p = *p;
    *p += uid->len;

    return (0);
}

/*
 * X.509 v3 extensions (only BasicConstraints are parsed)
 */
static int32_t x509_get_ext(const unsigned char **p,
            const unsigned char *end,
            x509_buf * ext, int32_t *ca_istrue, int32_t *max_pathlen)
{
    int32_t ret;
    uint32_t len;
    int32_t is_critical = 1;
    int32_t is_cacert   = 0;
    const unsigned char *end2;

    if (*p == end)
        return (0);

    ext->tag = **p;

    if ((ret = asn1_get_tag(p, end, &ext->len,
                ASN1_CONTEXT_SPECIFIC | ASN1_CONSTRUCTED | 3))
        != 0) {
        if (ret == TROPICSSL_ERR_ASN1_UNEXPECTED_TAG)
            return (0);

        return (ret);
    }

    ext->p = *p;
    end = *p + ext->len;

    /*
     * Extensions  ::=      SEQUENCE SIZE (1..MAX) OF Extension
     *
     * Extension  ::=  SEQUENCE      {
     *              extnID          OBJECT IDENTIFIER,
     *              critical        BOOLEAN DEFAULT FALSE,
     *              extnValue       OCTET STRING  }
     */
    if ((ret = asn1_get_tag(p, end, &len,
                ASN1_CONSTRUCTED | ASN1_SEQUENCE)) != 0)
        return (TROPICSSL_ERR_X509_CERT_INVALID_EXTENSIONS | ret);

    if (end != *p + len)
        return (TROPICSSL_ERR_X509_CERT_INVALID_EXTENSIONS |
            TROPICSSL_ERR_ASN1_LENGTH_MISMATCH);

    while (*p < end) {
        if ((ret = asn1_get_tag(p, end, &len,
                    ASN1_CONSTRUCTED | ASN1_SEQUENCE)) != 0)
            return (TROPICSSL_ERR_X509_CERT_INVALID_EXTENSIONS |
                ret);

        if (memcmp(*p, "\x06\x03\x55\x1D\x13", 5) != 0) {
            *p += len;
            continue;
        }

        *p += 5;

        if ((ret = asn1_get_bool(p, end, &is_critical)) != 0 &&
            (ret != TROPICSSL_ERR_ASN1_UNEXPECTED_TAG))
            return (TROPICSSL_ERR_X509_CERT_INVALID_EXTENSIONS |
                ret);

        if ((ret = asn1_get_tag(p, end, &len, ASN1_OCTET_STRING)) != 0)
            return (TROPICSSL_ERR_X509_CERT_INVALID_EXTENSIONS |
                ret);

        /*
         * BasicConstraints ::= SEQUENCE {
         *              cA                                              BOOLEAN DEFAULT FALSE,
         *              pathLenConstraint               INTEGER (0..MAX) OPTIONAL }
         */
        end2 = *p + len;

        if ((ret = asn1_get_tag(p, end2, &len,
                    ASN1_CONSTRUCTED | ASN1_SEQUENCE)) != 0)
            return (TROPICSSL_ERR_X509_CERT_INVALID_EXTENSIONS |
                ret);

        if (*p == end2)
            continue;

        if ((ret = asn1_get_bool(p, end2, &is_cacert)) != 0) {
            if (ret == TROPICSSL_ERR_ASN1_UNEXPECTED_TAG)
                ret = asn1_get_int(p, end2, &is_cacert);

            if (ret != 0)
                return
                    (TROPICSSL_ERR_X509_CERT_INVALID_EXTENSIONS
                     | ret);

            if (is_cacert != 0)
                is_cacert = 1;
        }

        if (*p == end2)
            continue;

        if ((ret = asn1_get_int(p, end2, max_pathlen)) != 0)
            return (TROPICSSL_ERR_X509_CERT_INVALID_EXTENSIONS |
                ret);

        if (*p != end2)
            return (TROPICSSL_ERR_X509_CERT_INVALID_EXTENSIONS |
                TROPICSSL_ERR_ASN1_LENGTH_MISMATCH);

        max_pathlen++;
    }

    if (*p != end)
        return (TROPICSSL_ERR_X509_CERT_INVALID_EXTENSIONS |
            TROPICSSL_ERR_ASN1_LENGTH_MISMATCH);

    *ca_istrue = is_critical & is_cacert;

    return (0);
}

/*
 * Parse one or more certificates and add them to the chained list
 */
static int32_t _x509parse_crt_1(x509_cert *chain, const unsigned char *buf, uint32_t buflen)
{
    int32_t ret;
    int32_t len;
    const unsigned char *s1, *s2;
    unsigned char *p;
    const unsigned char *end;
    x509_cert *crt;

    crt = chain;

    while (crt->version != 0)
        crt = crt->next;

    /*
     * check if the certificate is encoded in base64
     */
    s1 = (unsigned char *)strstr((char *)buf,
                     "-----BEGIN CERTIFICATE-----");

    if (s1 != NULL) {
        s2 = (unsigned char *)strstr((char *)buf,
                         "-----END CERTIFICATE-----");

        if (s2 == NULL || s2 <= s1)
            return (TROPICSSL_ERR_X509_CERT_INVALID_PEM);

        s1 += 27;
        if (*s1 == '\r')
            s1++;
        if (*s1 == '\n')
            s1++;
        else
            return (TROPICSSL_ERR_X509_CERT_INVALID_PEM);

        /*
         * get the DER data length and decode the buffer
         */
        len = 3 * ( (((uint32_t) ( s2 - s1 )) + 3 ) / 4);


        if( ( p = (unsigned char *) tls_host_malloc( "x509",  len ) ) == NULL )
            return (1);

        len = base64_decode( s1, (uint32_t) ( s2 - s1 ), p, len, BASE64_STANDARD );
        if ( len < 0 )
        {
            tls_host_free( p );
            return ( TROPICSSL_ERR_X509_CERT_INVALID_PEM );
        }

        /*
         * update the buffer size and offset
         */
        s2 += 25;
        if (*s2 == '\r')
            s2++;
        if (*s2 == '\n')
            s2++;
        else {
            tls_host_free ( p );
            return (TROPICSSL_ERR_X509_CERT_INVALID_PEM);
        }

        buflen -= s2 - buf;
        buf = s2;
    } else {
        /*
         * nope, copy the raw DER data
         */
        p = (unsigned char *) tls_host_malloc( "x509",  len = buflen );

        if (p == NULL)
            return (1);

        memcpy(p, buf, buflen);

        buflen = 0;
    }

    crt->raw.p = p;
    crt->raw.len = len;
    end = p + len;

    /*
     * Certificate  ::=      SEQUENCE  {
     *              tbsCertificate           TBSCertificate,
     *              signatureAlgorithm       AlgorithmIdentifier,
     *              signatureValue           BIT STRING      }
     */
    if( ( ret = asn1_get_tag((const unsigned char**) &p, end, (uint32_t*)&len,
                ASN1_CONSTRUCTED | ASN1_SEQUENCE)) != 0) {
        x509_free(crt);
        return (TROPICSSL_ERR_X509_CERT_INVALID_FORMAT);
    }

    if (len != (int)(end - p)) {
        x509_free(crt);
        return (TROPICSSL_ERR_X509_CERT_INVALID_FORMAT |
            TROPICSSL_ERR_ASN1_LENGTH_MISMATCH);
    }

    /*
     * TBSCertificate  ::=  SEQUENCE  {
     */
    crt->tbs.p = p;

    if( ( ret = asn1_get_tag((const unsigned char**) &p, end, (uint32_t*)&len,
                ASN1_CONSTRUCTED | ASN1_SEQUENCE)) != 0) {
        x509_free(crt);
        return (TROPICSSL_ERR_X509_CERT_INVALID_FORMAT | ret);
    }

    end = p + len;
    crt->tbs.len = end - crt->tbs.p;

    /*
     * Version      ::=      INTEGER  {      v1(0), v2(1), v3(2)  }
     *
     * CertificateSerialNumber      ::=      INTEGER
     *
     * signature                    AlgorithmIdentifier
     */
    ret = x509_get_version( (const unsigned char**) &p, end, &crt->version );
    if (ret != 0 )
    {
        crt->version = 1;
    }
    if( (ret = x509_get_serial((const unsigned char**) &p, end, &crt->serial)) != 0 ||
        (ret = x509_get_alg((const unsigned char**) &p, end, &crt->sig_oid1)) != 0) {
        x509_free(crt);
        return (ret);
    }

    crt->version++;

    if (crt->version > 3) {
        x509_free(crt);
        return (TROPICSSL_ERR_X509_CERT_UNKNOWN_VERSION);
    }

    if (crt->sig_oid1.len != 9 ||
        memcmp(crt->sig_oid1.p, OID_PKCS1, 8) != 0) {
        x509_free(crt);
        return (TROPICSSL_ERR_X509_CERT_UNKNOWN_SIG_ALG);
    }

    const unsigned char hash_algorithm = crt->sig_oid1.p[8];

    /**
     * Hash Algorithms for PKCS1 RSA Certificates
     * 1 = No Hash
     * 2 = MD2
     * 3 = MD4
     * 4 = MD5
     * 5 = SHA-1
     * 6 = rsaOAEPEncryptionSET
     * 7 = id-RSAES-OAEP
     * 10 = RSASSA-PSS
     * 11 = SHA-256
     * 12 = SHA-384
     * 13 = SHA-512
     */

    if ( ( hash_algorithm < 2 ) ||
         ( ( hash_algorithm > 5 ) && ( hash_algorithm < 11 ) ) ||
         ( hash_algorithm > 13 ) )
    {
        x509_free(crt);
        return (TROPICSSL_ERR_X509_CERT_UNKNOWN_SIG_ALG);
    }

    /*
     * issuer                               Name
     */
    crt->issuer_raw.p = p;

    if ((ret = asn1_get_tag((const unsigned char**) &p, end, (uint32_t*)&len,
                ASN1_CONSTRUCTED | ASN1_SEQUENCE)) != 0) {
        x509_free(crt);
        return (TROPICSSL_ERR_X509_CERT_INVALID_FORMAT | ret);
    }

    if ((ret = x509_get_name((const unsigned char**) &p, p + len, &crt->issuer )) != 0) {
        x509_free(crt);
        return (ret);
    }

    crt->issuer_raw.len = p - crt->issuer_raw.p;

    /*
     * Validity ::= SEQUENCE {
     *              notBefore          Time,
     *              notAfter           Time }
     *
     */
    if ((ret = x509_get_dates((const unsigned char**) &p, end, &crt->valid_from,
                  &crt->valid_to)) != 0) {
        x509_free(crt);
        return (ret);
    }

    /*
     * subject                              Name
     */
    crt->subject_raw.p = p;

    if ((ret = asn1_get_tag((const unsigned char**) &p, end, (uint32_t*)&len,
                ASN1_CONSTRUCTED | ASN1_SEQUENCE)) != 0) {
        x509_free(crt);
        return (TROPICSSL_ERR_X509_CERT_INVALID_FORMAT | ret);
    }

    if ((ret = x509_get_name((const unsigned char**) &p, p + len, &crt->subject)) != 0) {
        x509_free(crt);
        return (ret);
    }

    crt->subject_raw.len = p - crt->subject_raw.p;

    /*
     * SubjectPublicKeyInfo  ::=  SEQUENCE
     *              algorithm                        AlgorithmIdentifier,
     *              subjectPublicKey         BIT STRING      }
     */
    if ((ret = asn1_get_tag((const unsigned char**) &p, end, (uint32_t*)&len,
                ASN1_CONSTRUCTED | ASN1_SEQUENCE)) != 0) {
        x509_free(crt);
        return (TROPICSSL_ERR_X509_CERT_INVALID_FORMAT | ret);
    }

    if ((ret = x509_get_pubkey((const unsigned char**) &p, p + len, &crt->pk_oid,
                   &crt->rsa.N, &crt->rsa.E)) != 0) {
        x509_free(crt);
        return (ret);
    }

    if ((ret = rsa_check_pubkey(&crt->rsa)) != 0) {
        x509_free(crt);
        return (ret);
    }

    crt->rsa.len = mpi_size(&crt->rsa.N);

    /*
     *      issuerUniqueID  [1]      IMPLICIT UniqueIdentifier OPTIONAL,
     *                                               -- If present, version shall be v2 or v3
     *      subjectUniqueID [2]      IMPLICIT UniqueIdentifier OPTIONAL,
     *                                               -- If present, version shall be v2 or v3
     *      extensions              [3]      EXPLICIT Extensions OPTIONAL
     *                                               -- If present, version shall be v3
     */
    if (crt->version == 2 || crt->version == 3) {
        ret = x509_get_uid((const unsigned char**) &p, end, &crt->issuer_id, 1);
        if (ret != 0) {
            x509_free(crt);
            return (ret);
        }
    }

    if (crt->version == 2 || crt->version == 3) {
        ret = x509_get_uid((const unsigned char**) &p, end, &crt->subject_id, 2);
        if (ret != 0) {
            x509_free(crt);
            return (ret);
        }
    }

    if (crt->version == 3) {
        ret = x509_get_ext((const unsigned char**) &p, end, &crt->v3_ext,
                   &crt->ca_istrue, &crt->max_pathlen);
        if (ret != 0) {
            x509_free(crt);
            return (ret);
        }
    }

    if (p != end) {
        x509_free(crt);
        return (TROPICSSL_ERR_X509_CERT_INVALID_FORMAT |
            TROPICSSL_ERR_ASN1_LENGTH_MISMATCH);
    }

    end = crt->raw.p + crt->raw.len;

    /*
     *      signatureAlgorithm       AlgorithmIdentifier,
     *      signatureValue           BIT STRING
     */
    if ((ret = x509_get_alg((const unsigned char**) &p, end, &crt->sig_oid2)) != 0) {
        x509_free(crt);
        return (ret);
    }

    if (memcmp(crt->sig_oid1.p, crt->sig_oid2.p, 9) != 0) {
        x509_free(crt);
        return (TROPICSSL_ERR_X509_CERT_SIG_MISMATCH);
    }

    if ((ret = x509_get_sig((const unsigned char**) &p, end, &crt->sig)) != 0) {
        x509_free(crt);
        return (ret);
    }

    if (p != end) {
        x509_free(crt);
        return (TROPICSSL_ERR_X509_CERT_INVALID_FORMAT |
            TROPICSSL_ERR_ASN1_LENGTH_MISMATCH);
    }

    crt->next = (x509_cert *) tls_host_malloc( "x509",  sizeof( x509_cert ) );

    if (crt->next == NULL) {
        x509_free(crt);
        return (1);
    }

    crt = crt->next;
    memset(crt, 0, sizeof(x509_cert));

    if (buflen > 0)
        return (x509parse_crt(crt, buf, buflen));

    return (0);
}

/*
 * Parse one or more certificates and add them to the chained list
 */
int32_t x509parse_crt(x509_cert * chain, const unsigned char *buf, uint32_t buflen)
{
    int ret;
    unsigned char *buf1 = malloc(buflen);
    if (buf1 == NULL) {
          return (1);
    }
    memcpy(buf1, buf, buflen);
    ret = _x509parse_crt_1(chain, buf1, buflen);
    free(buf1);
    return ret;
}

#if defined(TROPICSSL_DES_C)
/*
 * Read a 16-byte hex string and convert it to binary
 */
static int32_t x509_get_iv(const unsigned char *s, unsigned char iv[8])
{
    int32_t i, j, k;

    memset(iv, 0, 8);

    for (i = 0; i < 16; i++, s++) {
        if (*s >= '0' && *s <= '9')
            j = *s - '0';
        else if (*s >= 'A' && *s <= 'F')
            j = *s - '7';
        else if (*s >= 'a' && *s <= 'f')
            j = *s - 'W';
        else
            return (TROPICSSL_ERR_X509_KEY_INVALID_ENC_IV);

        k = ((i & 1) != 0) ? j : j << 4;

        iv[i >> 1] = (unsigned char)(iv[i >> 1] | k);
    }

    return (0);
}

/*
 * Decrypt with 3DES-CBC, using PBKDF1 for key derivation
 * Works on buf data in-place
 */
static void x509_des3_decrypt(unsigned char des3_iv[8],
                  unsigned char *buf, int32_t buflen,
                  const unsigned char *pwd, int32_t pwdlen)
{
    md5_context md5_ctx;
    des3_context des3_ctx;
    unsigned char md5sum[16];
    unsigned char des3_key[24];

    /*
     * 3DES key[ 0..15] = MD5(pwd || IV)
     *              key[16..23] = MD5(pwd || IV || 3DES key[ 0..15])
     */
    md5_starts(&md5_ctx);
    md5_update(&md5_ctx, pwd, pwdlen);
    md5_update(&md5_ctx, des3_iv, 8);
    md5_finish(&md5_ctx, md5sum);
    memcpy(des3_key, md5sum, 16);

    md5_starts(&md5_ctx);
    md5_update(&md5_ctx, md5sum, 16);
    md5_update(&md5_ctx, pwd, pwdlen);
    md5_update(&md5_ctx, des3_iv, 8);
    md5_finish(&md5_ctx, md5sum);
    memcpy(des3_key + 16, md5sum, 8);

    des3_set3key_dec(&des3_ctx, des3_key);
    des3_crypt_cbc(&des3_ctx, DES_DECRYPT, buflen, des3_iv, buf, buf);

    memset(&md5_ctx, 0, sizeof(md5_ctx));
    memset(&des3_ctx, 0, sizeof(des3_ctx));
    memset(md5sum, 0, 16);
    memset(des3_key, 0, 24);
}
#endif

/*
 * Parse a PUBLIC RSA key
 */
int32_t x509parse_pubkey( rsa_context *rsa, unsigned char *buf, int32_t buflen, unsigned char *pwd, int32_t pwdlen )
{
    int32_t ret, enc;
    uint32_t len;
    const unsigned char *s1, *s2;
    const unsigned char *p = buf, *end;
    x509_buf pk_oid;
    unsigned char *decode_buf = NULL;
   unsigned char des3_iv[8];
    s1 = (unsigned char *)strstr((char *)buf,
                     "-----BEGIN PUBLIC KEY-----" );

    if (s1 != NULL) {
        s2 = (unsigned char *)strstr((char *)buf,
                         "-----END PUBLIC KEY-----");

        if (s2 == NULL || s2 <= s1)
            return (TROPICSSL_ERR_X509_KEY_INVALID_PEM);

        s1 += 26;
        if (*s1 == '\r')
            s1++;
        if (*s1 == '\n')
            s1++;
        else
            return (TROPICSSL_ERR_X509_KEY_INVALID_PEM);

        enc = 0;

        if( memcmp( s1, "Proc-Type: 4,ENCRYPTED", 22 ) == 0 ) {
#if defined(TROPICSSL_DES_C)
            enc++;

            s1 += 22;
            if (*s1 == '\r')
                s1++;
            if (*s1 == '\n')
                s1++;
            else
                return (TROPICSSL_ERR_X509_KEY_INVALID_PEM);

            if (memcmp(s1, "DEK-Info: DES-EDE3-CBC,", 23) != 0)
                return (TROPICSSL_ERR_X509_KEY_UNKNOWN_ENC_ALG);

            s1 += 23;
            if (x509_get_iv(s1, des3_iv) != 0)
                return (TROPICSSL_ERR_X509_KEY_INVALID_ENC_IV);

            s1 += 16;
            if (*s1 == '\r')
                s1++;
            if (*s1 == '\n')
                s1++;
            else
                return (TROPICSSL_ERR_X509_KEY_INVALID_PEM);
#else
            return (TROPICSSL_ERR_X509_FEATURE_UNAVAILABLE);
#endif
        }

        len = 3 * ( (((uint32_t) ( s2 - s1 )) + 3 ) / 4);

        decode_buf = (unsigned char *) tls_host_malloc( "x509",  len );
        if( decode_buf == NULL )
        {
            return( 1 );
        }

        len = base64_decode( s1, s2 - s1, decode_buf, len, BASE64_STANDARD );
        if( len < 0 )
        {
            tls_host_free ( decode_buf );
            return( ret | TROPICSSL_ERR_X509_KEY_INVALID_PEM );
        }

        buflen = len;
        p = decode_buf;

        if (enc != 0) {
#if defined(TROPICSSL_DES_C)
            if (pwd == NULL) {
                tls_host_free ( buf );
                return
                    (TROPICSSL_ERR_X509_KEY_PASSWORD_REQUIRED);
            }

            x509_des3_decrypt(des3_iv, buf, buflen, pwd, pwdlen);

            if (buf[0] != 0x30 || buf[1] != 0x82 ||
                buf[4] != 0x02 || buf[5] != 0x01) {
                tls_host_free ( buf );
                return
                    (TROPICSSL_ERR_X509_KEY_PASSWORD_MISMATCH);
            }
#else
            return (TROPICSSL_ERR_X509_FEATURE_UNAVAILABLE);
#endif
        }
    }

    memset(rsa, 0, sizeof(rsa_context));

    end = p + buflen;

    /*
     *  RSAPublicKey ::= SEQUENCE {
     *      modulus           INTEGER,  -- n
     *      publicExponent    INTEGER,  -- e
     *  }
     */
    if ((ret = asn1_get_tag(&p, end, &len,
                ASN1_CONSTRUCTED | ASN1_SEQUENCE)) != 0) {
        if (s1 != NULL)
            tls_host_free ( decode_buf );

        rsa_free(rsa);
        return (TROPICSSL_ERR_X509_KEY_INVALID_FORMAT | ret);
    }

    end = p + len;

    if ((ret = x509_get_pubkey( &p, p + len, &pk_oid, &rsa->N, &rsa->E)) != 0) {
        if (s1 != NULL)
            tls_host_free ( decode_buf );

        rsa_free(rsa);
        return( ret );
    }

    rsa->len = mpi_size( &rsa->N );

    if (p != end) {
        if (s1 != NULL)
            tls_host_free ( decode_buf );

        rsa_free(rsa);
        return (TROPICSSL_ERR_X509_KEY_INVALID_FORMAT |
            TROPICSSL_ERR_ASN1_LENGTH_MISMATCH);
    }

    if( ( ret = rsa_check_pubkey( rsa ) ) != 0 ){
        if (s1 != NULL)
            tls_host_free ( decode_buf );

        rsa_free(rsa);
        return (ret);
    }

    if (s1 != NULL)
        tls_host_free ( decode_buf );

    return (0);
}

/*
 * Parse a private RSA key
 */
static int32_t _x509parse_key_1(rsa_context * rsa, const unsigned char *buf, uint32_t buflen,
          const unsigned char *pwd, uint32_t pwdlen)
{
    int32_t ret, enc;
    uint32_t len;
    unsigned char *decode_buf = NULL;
    const unsigned char *s1, *s2;
    const unsigned char *p = buf, *end;
    unsigned char des3_iv[8];

    s1 = (unsigned char *)strstr((char *)buf,
                     "-----BEGIN RSA PRIVATE KEY-----");

    if (s1 != NULL) {
        s2 = (unsigned char *)strstr((char *)buf,
                         "-----END RSA PRIVATE KEY-----");

        if (s2 == NULL || s2 <= s1)
            return (TROPICSSL_ERR_X509_KEY_INVALID_PEM);

        s1 += 31;
        if (*s1 == '\r')
            s1++;
        if (*s1 == '\n')
            s1++;
        else
            return (TROPICSSL_ERR_X509_KEY_INVALID_PEM);

        enc = 0;

        if (memcmp(s1, "Proc-Type: 4,ENCRYPTED", 22) == 0) {
#if defined(TROPICSSL_DES_C)
            enc++;

            s1 += 22;
            if (*s1 == '\r')
                s1++;
            if (*s1 == '\n')
                s1++;
            else
                return (TROPICSSL_ERR_X509_KEY_INVALID_PEM);

            if (memcmp(s1, "DEK-Info: DES-EDE3-CBC,", 23) != 0)
                return (TROPICSSL_ERR_X509_KEY_UNKNOWN_ENC_ALG);

            s1 += 23;
            if (x509_get_iv(s1, des3_iv) != 0)
                return (TROPICSSL_ERR_X509_KEY_INVALID_ENC_IV);

            s1 += 16;
            if (*s1 == '\r')
                s1++;
            if (*s1 == '\n')
                s1++;
            else
                return (TROPICSSL_ERR_X509_KEY_INVALID_PEM);
#else
            return (TROPICSSL_ERR_X509_FEATURE_UNAVAILABLE);
#endif
        }

        len = 3 * ( (((uint32_t) ( s2 - s1 )) + 3 ) / 4);


        decode_buf = (unsigned char *) tls_host_malloc( "x509",  len );
        if( decode_buf == NULL )
        {
            return (1);
        }

        len = base64_decode( s1, s2 - s1, decode_buf, len, BASE64_STANDARD );
        if ( len < 0 )
        {
            tls_host_free ( decode_buf );
            return (ret | TROPICSSL_ERR_X509_KEY_INVALID_PEM);
        }

        buflen = len;
        p = decode_buf;

        if (enc != 0) {
#if defined(TROPICSSL_DES_C)
            if (pwd == NULL) {
                tls_host_free ( decode_buf );
                return
                    (TROPICSSL_ERR_X509_KEY_PASSWORD_REQUIRED);
            }

            x509_des3_decrypt(des3_iv, decode_buf, buflen, pwd, pwdlen);

            if (decode_buf[0] != 0x30 || decode_buf[1] != 0x82 ||
                decode_buf[4] != 0x02 || decode_buf[5] != 0x01) {
                tls_host_free ( decode_buf );
                return
                    (TROPICSSL_ERR_X509_KEY_PASSWORD_MISMATCH);
            }
#else
            return( TROPICSSL_ERR_X509_FEATURE_UNAVAILABLE );
#endif
        }
    }

    memset( rsa, 0, sizeof( rsa_context ) );

    end = p + buflen;

    /*
     *      RSAPrivateKey ::= SEQUENCE {
     *              version                   Version,
     *              modulus                   INTEGER,      -- n
     *              publicExponent    INTEGER,      -- e
     *              privateExponent   INTEGER,      -- d
     *              prime1                    INTEGER,      -- p
     *              prime2                    INTEGER,      -- q
     *              exponent1                 INTEGER,      -- d mod (p-1)
     *              exponent2                 INTEGER,      -- d mod (q-1)
     *              coefficient               INTEGER,      -- (inverse of q) mod p
     *              otherPrimeInfos   OtherPrimeInfos OPTIONAL
     *      }
     */
    if ((ret = asn1_get_tag(&p, end, &len,
                ASN1_CONSTRUCTED | ASN1_SEQUENCE)) != 0) {
        if (s1 != NULL)
            tls_host_free ( decode_buf );

        rsa_free( rsa );
        return( TROPICSSL_ERR_X509_KEY_INVALID_FORMAT | ret );
    }

    end = p + len;

    if ((ret = asn1_get_int(&p, end, &rsa->ver)) != 0) {
        if (s1 != NULL)
            tls_host_free ( decode_buf );

        rsa_free(rsa);
        return (TROPICSSL_ERR_X509_KEY_INVALID_FORMAT | ret);
    }

    if (rsa->ver != 0) {
        if (s1 != NULL)
            tls_host_free ( decode_buf );

        rsa_free( rsa );
        return( ret | TROPICSSL_ERR_X509_KEY_INVALID_VERSION );
    }

    if ((ret = asn1_get_mpi((const unsigned char**) &p, end, &rsa->N  ) ) != 0 ||
        (ret = asn1_get_mpi((const unsigned char**) &p, end, &rsa->E  ) ) != 0 ||
        (ret = asn1_get_mpi((const unsigned char**) &p, end, &rsa->D  ) ) != 0 ||
        (ret = asn1_get_mpi((const unsigned char**) &p, end, &rsa->P  ) ) != 0 ||
        (ret = asn1_get_mpi((const unsigned char**) &p, end, &rsa->Q  ) ) != 0 ||
        (ret = asn1_get_mpi((const unsigned char**) &p, end, &rsa->DP ) ) != 0 ||
        (ret = asn1_get_mpi((const unsigned char**) &p, end, &rsa->DQ ) ) != 0 ||
        (ret = asn1_get_mpi((const unsigned char**) &p, end, &rsa->QP ) ) != 0) {
        if (s1 != NULL)
            tls_host_free (decode_buf);

        rsa_free(rsa);
        return (ret | TROPICSSL_ERR_X509_KEY_INVALID_FORMAT);
    }

    rsa->len = mpi_size(&rsa->N);

    if (p != end) {
        if (s1 != NULL)
            tls_host_free ( decode_buf );

        rsa_free(rsa);
        return (TROPICSSL_ERR_X509_KEY_INVALID_FORMAT |
            TROPICSSL_ERR_ASN1_LENGTH_MISMATCH);
    }

    if ((ret = rsa_check_privkey(rsa)) != 0) {
        if (s1 != NULL)
            tls_host_free ( decode_buf );

        rsa_free(rsa);
        return (ret);
    }

    if (s1 != NULL)
        tls_host_free ( decode_buf );

    return( 0 );
}

/*
 * Parse a private RSA key
 */
int32_t x509parse_key(rsa_context * rsa,
          const unsigned char *key, uint32_t keylen,
          const unsigned char *pwd, uint32_t pwdlen)
{
    int ret;
    unsigned char *buf1;
    buf1 = malloc(keylen);
    if (buf1 == NULL) {
        return (1);
    }
    memcpy(buf1, key, keylen);
    ret = _x509parse_key_1(rsa, buf1, keylen, pwd, pwdlen);
    free(buf1);
    return ret;
}




#ifdef ENABLE_X509_FUNCTIONS_WHICH_USE_SPRINTF


#if defined _MSC_VER && !defined snprintf
#define snprintf _snprintf
#endif

/*
 * Store the name in printable form into buf; no more
 * than (end - buf) characters will be written
 */
int32_t x509parse_dn_gets(char *buf, const char *end, const x509_name *dn)
{
    int32_t i;
    unsigned char c;
    const x509_name *name;
    char s[128], *p;

    memset(s, 0, sizeof(s));

    name = dn;
    p = buf;

    while (name != NULL) {
        if (name != dn)
            p += snprintf(p, end - p, ", ");

        if (memcmp(name->oid.p, OID_X520, 2) == 0) {
            switch (name->oid.p[2]) {
            case X520_COMMON_NAME:
                p += snprintf(p, end - p, "CN=");
                break;

            case X520_COUNTRY:
                p += snprintf(p, end - p, "C=");
                break;

            case X520_LOCALITY:
                p += snprintf(p, end - p, "L=");
                break;

            case X520_STATE:
                p += snprintf(p, end - p, "ST=");
                break;

            case X520_ORGANIZATION:
                p += snprintf(p, end - p, "O=");
                break;

            case X520_ORG_UNIT:
                p += snprintf(p, end - p, "OU=");
                break;

            default:
                p += snprintf(p, end - p, "0x%02X=",
                          name->oid.p[2]);
                break;
            }
        } else if (memcmp(name->oid.p, OID_PKCS9, 8) == 0) {
            switch (name->oid.p[8]) {
            case PKCS9_EMAIL:
                p += snprintf(p, end - p, "emailAddress=");
                break;

            default:
                p += snprintf(p, end - p, "0x%02X=",
                          name->oid.p[8]);
                break;
            }
        } else
            p += snprintf(p, end - p, "\?\?=");

        for (i = 0; i < name->val.len; i++) {
            if (i >= (int)sizeof(s) - 1)
                break;

            c = name->val.p[i];
            if (c < 32 || c == 127 || (c > 128 && c < 160))
                s[i] = '?';
            else
                s[i] = c;
        }
        s[i] = '\0';
        p += snprintf(p, end - p, "%s", s);
        name = name->next;
    }

    return (p - buf);
}

/*
 * Return an informational string about the
 * certificate, or NULL if memory allocation failed
 */
char *x509parse_cert_info(char *buf, size_t buf_size,
              const char *prefix, const x509_cert * crt)
{
    int32_t i, n;
    char *p;
    const char *end;

    p = buf;
    end = buf + buf_size - 1;

    p += snprintf(p, end - p, "%scert. version : %ld\n",
              prefix, (long)crt->version);
    p += snprintf(p, end - p, "%sserial number : ", prefix);

    n = (crt->serial.len <= 32)
        ? crt->serial.len : 32;

    for (i = 0; i < n; i++)
        p += snprintf(p, end - p, "%02X%s",
                  crt->serial.p[i], (i < n - 1) ? ":" : "");

    p += snprintf(p, end - p, "\n%sissuer    name  : ", prefix);
    p += x509parse_dn_gets(p, end, &crt->issuer);

    p += snprintf(p, end - p, "\n%ssubject name  : ", prefix);
    p += x509parse_dn_gets(p, end, &crt->subject);

    p += snprintf(p, end - p, "\n%sissued    on      : "
              "%04ld-%02ld-%02ld %02ld:%02ld:%02ld", prefix,
              (long)crt->valid_from.year, (long)crt->valid_from.mon,
              (long)crt->valid_from.day, (long)crt->valid_from.hour,
              (long)crt->valid_from.min, (long)crt->valid_from.sec);

    p += snprintf(p, end - p, "\n%sexpires on      : "
              "%04ld-%02ld-%02ld %02ld:%02ld:%02ld", prefix,
              (long)crt->valid_to.year, (long)crt->valid_to.mon,
              (long)crt->valid_to.day, (long)crt->valid_to.hour,
              (long)crt->valid_to.min, (long)crt->valid_to.sec);

    p += snprintf(p, end - p, "\n%ssigned using  : RSA+", prefix);

    switch (crt->sig_oid1.p[8]) {
    case RSA_MD2:
        p += snprintf(p, end - p, "MD2");
        break;
    case RSA_MD4:
        p += snprintf(p, end - p, "MD4");
        break;
    case RSA_MD5:
        p += snprintf(p, end - p, "MD5");
        break;
    case RSA_SHA1:
        p += snprintf(p, end - p, "SHA1");
        break;
    case RSA_SHA256:
        p += snprintf( p, end - p, "SHA256" );
        break;
    case RSA_SHA384:
        p += snprintf( p, end - p, "SHA384" );
        break;
    case RSA_SHA512:
        p += snprintf( p, end - p, "SHA512" );
        break;
    default:
        p += snprintf(p, end - p, "???");
        break;
    }

    snprintf(p, end - p, "\n%sRSA key size  : %ld bits\n", prefix,
              (long)(crt->rsa.N.n * (int) sizeof(uint32_t) * 8));

    return (buf);
}

#endif /* ifdef ENABLE_X509_FUNCTIONS_WHICH_USE_SPRINTF */

/*
 * Return 0 if the certificate is still valid, or BADCERT_EXPIRED
 */
int32_t x509parse_expired(const x509_cert *crt)
{
    struct tm *lt;
    time_t tt;

    tt = (time_t)( tls_host_get_time_ms( ) / 1000 );
    lt = localtime(&tt);

    if (lt->tm_year > crt->valid_to.year - 1900)
        return (BADCERT_EXPIRED);

    if (lt->tm_year == crt->valid_to.year - 1900 &&
        lt->tm_mon > crt->valid_to.mon - 1)
        return (BADCERT_EXPIRED);

    if (lt->tm_year == crt->valid_to.year - 1900 &&
        lt->tm_mon == crt->valid_to.mon - 1 &&
        lt->tm_mday > crt->valid_to.day)
        return (BADCERT_EXPIRED);

    return (0);
}

static void x509_hash(const unsigned char *in, int32_t len, int32_t alg, unsigned char out[64])
{
    switch (alg) {
#if defined(TROPICSSL_MD2_C)
    case RSA_MD2:
        md2(in, len, out);
        break;
#endif
#if defined(TROPICSSL_MD4_C)
    case RSA_MD4:
        md4(in, len, out);
        break;
#endif
    case RSA_MD5:
        md5(in, len, out);
        break;
    case RSA_SHA1:
        sha1(in, len, out);
        break;
    case RSA_SHA256:
        sha2( in, len, out, 0 );
        break;
    case RSA_SHA384:
        sha4( in, len, out, 1 );
        break;
    case RSA_SHA512:
        sha4( in, len, out, 0 );
        break;
    default:
        memset(out, '\xFF', len);
        break;
    }
}

/*
 * Verify the certificate validity
 */
int32_t x509parse_verify(const x509_cert *crt,
             const x509_cert *trust_ca, const char *cn, int32_t *flags)
{
    int32_t cn_len;
    int32_t hash_id;
    const x509_cert *cur;
    const x509_cert *trusted_ca_iter;
    const x509_name *name;
    unsigned char hash[64];

    *flags = x509parse_expired(crt);

    if (cn != NULL) {
        name = &crt->subject;
        cn_len = strlen(cn);

        while (name != NULL) {
            if (memcmp(name->oid.p, OID_CN, 3) == 0 &&
                memcmp(name->val.p, cn, cn_len) == 0 &&
                name->val.len == cn_len)
                break;

            name = name->next;
        }

        if (name == NULL)
            *flags |= BADCERT_CN_MISMATCH;
    }

    *flags |= BADCERT_NOT_TRUSTED;

    /* Traverse through cert chain attempting to find a matching trusted cert */
    for ( cur = crt; ( *flags & BADCERT_NOT_TRUSTED ) != 0 && cur->version != 0; cur = cur->next )
    {
        /* Verify current certificate is correctly signed by next */
        if ( cur->next->version != 0 )
        {
            hash_id = cur->sig_oid1.p[ 8 ];

            x509_hash( cur->tbs.p, cur->tbs.len, hash_id, hash );

            if ( rsa_pkcs1_verify( &cur->next->rsa, RSA_PUBLIC, hash_id, 0, hash, cur->sig.p ) != 0 )
            {
                return ( TROPICSSL_ERR_X509_CERT_VERIFY_FAILED );
            }
        }

        /* Check if current cert has been issued by trusted root cert */
        for ( trusted_ca_iter = trust_ca; trusted_ca_iter->version != 0; trusted_ca_iter = trusted_ca_iter->next )
        {
            if ( cur->issuer_raw.len == trusted_ca_iter->subject_raw.len &&
                 memcmp( cur->issuer_raw.p, trusted_ca_iter->subject_raw.p, cur->issuer_raw.len ) == 0 )
            {
                hash_id = cur->sig_oid1.p[ 8 ];

                x509_hash( cur->tbs.p, cur->tbs.len, hash_id, hash );

                if ( rsa_pkcs1_verify( &trusted_ca_iter->rsa, RSA_PUBLIC, hash_id, 0, hash, cur->sig.p ) != 0 )
                {
                    return ( TROPICSSL_ERR_X509_CERT_VERIFY_FAILED );
                }

                *flags &= ~BADCERT_NOT_TRUSTED;
                break;
            }
        }
    }

    if (*flags != 0)
        return (TROPICSSL_ERR_X509_CERT_VERIFY_FAILED);

    return (0);
}

/*
 * Unallocate all certificate data
 */
void x509_free(x509_cert * crt)
{
    x509_cert *cert_cur = crt;
    x509_cert *cert_prv;
    x509_name *name_cur;
    x509_name *name_prv;

    if (crt == NULL)
        return;

    do {
        rsa_free(&cert_cur->rsa);

        name_cur = cert_cur->issuer.next;
        while (name_cur != NULL) {
            name_prv = name_cur;
            name_cur = name_cur->next;
            memset(name_prv, 0, sizeof(x509_name));
            tls_host_free ( name_prv );
        }

        name_cur = cert_cur->subject.next;
        while (name_cur != NULL) {
            name_prv = name_cur;
            name_cur = name_cur->next;
            memset(name_prv, 0, sizeof(x509_name));
            tls_host_free ( name_prv );
        }

        if (cert_cur->raw.p != NULL) {
            memset(cert_cur->raw.p, 0, cert_cur->raw.len);
            tls_host_free ( cert_cur->raw.p );
        }

        cert_cur = cert_cur->next;
    } while (cert_cur != NULL);

    cert_cur = crt;
    do {
        cert_prv = cert_cur;
        cert_cur = cert_cur->next;

        memset(cert_prv, 0, sizeof(x509_cert));
        if (cert_prv != crt)
            tls_host_free ( cert_prv );
    } while (cert_cur != NULL);
}

#if defined(TROPICSSL_SELF_TEST)

#include <certs.h>

/*
 * Checkup routine
 */
int32_t x509_self_test(int32_t verbose)
{
    int32_t ret, i, j;
    x509_cert cacert;
    x509_cert clicert;
    rsa_context rsa;

    if (verbose != 0)
        printf("  X.509 certificate load: ");

    memset(&clicert, 0, sizeof(x509_cert));

    ret = x509parse_crt(&clicert, (const unsigned char *)test_cli_crt,
                strlen(test_cli_crt));
    if (ret != 0) {
        if (verbose != 0)
            printf("failed\n");

        return (ret);
    }

    memset(&cacert, 0, sizeof(x509_cert));

    ret = x509parse_crt(&cacert, (const unsigned char *)test_ca_crt,
                strlen(test_ca_crt));
    if (ret != 0) {
        if (verbose != 0)
            printf("failed\n");

        return (ret);
    }

    if (verbose != 0)
        printf("passed\n  X.509 private key load: ");

    i = strlen(test_ca_key);
    j = strlen(test_ca_pwd);

    if ((ret = x509parse_key(&rsa,
                 (const unsigned char *)test_ca_key, i,
                 (const unsigned char *)test_ca_pwd, j)) != 0) {
        if (verbose != 0)
            printf("failed\n");

        return (ret);
    }

    if (verbose != 0)
        printf("passed\n  X.509 signature verify: ");

    ret = x509parse_verify(&clicert, &cacert, "Joe User", &i);
    if (ret != 0) {
        if (verbose != 0)
            printf("failed\n");

        return (ret);
    }

    if (verbose != 0)
        printf("passed\n\n");

    x509_free(&cacert);
    x509_free(&clicert);
    rsa_free(&rsa);

    return (0);
}

#endif
