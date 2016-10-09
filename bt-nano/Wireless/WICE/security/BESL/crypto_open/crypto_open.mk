#
# Copyright 2015, Broadcom Corporation
# All Rights Reserved.
#
# This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
# the contents of this file may not be disclosed to third parties, copied
# or duplicated in any form, in whole or in part, without the prior
# written permission of Broadcom Corporation.
#

NAME := Lib_crypto_open

GLOBAL_INCLUDES := . \
                   srp

$(NAME)_SOURCES += \
                   aes.c \
                   arc4.c \
                   bignum.c \
                   camellia.c \
                   certs.c \
                   chacha_reference.c \
                   curve25519.c \
                   des.c \
                   ed25519/ed25519.c \
                   md5.c \
                   poly1305.c \
                   seed.c \
                   sha1.c \
                   sha2.c \
                   sha4.c \
                   x509parse.c


# Used by x509parse
$(NAME)_DEFINES += TROPICSSL_DES_C

# Used by AES
$(NAME)_DEFINES += TROPICSSL_AES_ROM_TABLES

# Used by Bignum
$(NAME)_DEFINES += TROPICSSL_HAVE_LONGLONG

# Used by SRP
$(NAME)_DEFINES += STDC_HEADERS \
                   USE_SRP_SHA_512 \
                   OPENSSL

# Used by SEED
$(NAME)_DEFINES += OPENSSL_SMALL_FOOTPRINT

#$(NAME)_CFLAGS  = $(COMPILER_SPECIFIC_PEDANTIC_CFLAGS)

$(NAME)_ALWAYS_OPTIMISE := 1

$(NAME)_UNIT_TEST_SOURCES := unit/crypto_unit.cpp \
                             unit/chacha_test_vectors.c \
                             unit/chacha_test.c \
                             unit/test-curve25519.c \
                             ed25519/test.c
