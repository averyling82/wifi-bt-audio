#ifndef __WPAS_CRYPTO_H_
#define __WPAS_CRYPTO_H_

#define ETHER_ADDRLEN                   6
#define PMK_EXPANSION_CONST             "Pairwise key expansion"
#define PMK_EXPANSION_CONST_SIZE        22
#define PMKID_NAME_CONST                "PMK Name"
#define PMKID_NAME_CONST_SIZE           8
#define GMK_EXPANSION_CONST             "Group key expansion"
#define GMK_EXPANSION_CONST_SIZE        19
#define RANDOM_EXPANSION_CONST          "Init Counter"
#define RANDOM_EXPANSION_CONST_SIZE 12
#define PTK_LEN_CCMP                    48

#define LargeIntegerOverflow(x) ((x.field.HighPart == 0xffffffff) && \
                                (x.field.LowPart == 0xffffffff))
#define LargeIntegerZero(x) os_memset(&x.charData, 0, 8);

#define Octet16IntegerOverflow(x) (LargeIntegerOverflow(x.field.HighPart) && \
                                  LargeIntegerOverflow(x.field.LowPart))
#define Octet16IntegerZero(x) os_memset(&x.charData, 0, 16);

#define SetNonce(ocDst, oc32Counter) set_eapol_key_iv(ocDst, oc32Counter)


int32 password_hash (
    uint8 *password,
    uint8 *ssid,
    int16 ssidlength,
    uint8 *output);

void octet_to_large_int(OCTET_STRING f, LARGE_INTEGER * li);

int32 replay_counter_not_larger(LARGE_INTEGER li1, OCTET_STRING f);

#ifdef CONFIG_SOFTAP_SUPPORT

int32 replay_counter_equal(LARGE_INTEGER li1, OCTET_STRING f);

int32 replay_counter_larger(LARGE_INTEGER li1, OCTET_STRING f);

void set_replay_counter(OCTET_STRING *f, uint32 h, uint32 l);

#endif

void inc_large_int(LARGE_INTEGER * x);

OCTET32_INTEGER *inc_oct32_int(OCTET32_INTEGER * x);

int32 check_mic(OCTET_STRING EAPOLMsgRecvd, uint8 *key, int32 keylen);

void calc_mic(OCTET_STRING EAPOLMsgSend, int32 algo, uint8 *key, int32 keylen);

void calc_gtk(uint8 *addr, uint8 *nonce,
              uint8 *keyin, int32 keyinlen,
              uint8 *keyout, int32 keyoutlen);

void calc_ptk(uint8 *addr1, uint8 *addr2,
              uint8 *nonce1, uint8 *nonce2,
              uint8 * keyin, int32 keyinlen,
              uint8 *keyout, int32 keyoutlen);

#ifdef CONFIG_SOFTAP_SUPPORT
void enc_gtk(struct wpa_priv *priv, struct sta_info *pstat,
             uint8 *kek, int32 keklen, uint8 *key,
             int32 keylen, uint8 *out, uint16 *outlen);
#endif

int32 dec_wpa2_key_data(WPA_STA_INFO* pStaInfo, uint8 *key, int32 keylen, uint8 *kek, int32 keklen, uint8 *kout);

int32 dec_gtk(OCTET_STRING EAPOLMsgRecvd, uint8 *kek, int32 keklen, int32 keylen, uint8 *kout);

void gen_nonce(uint8 *nonce, uint8 *addr);

#ifdef CONFIG_SOFTAP_SUPPORT
void integrity_check_fail(struct wpa_priv *priv);
#endif

#endif

