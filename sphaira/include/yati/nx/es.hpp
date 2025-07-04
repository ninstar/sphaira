#pragma once

#include <switch.h>
#include <span>
#include "ncm.hpp"
#include "keys.hpp"

namespace sphaira::es {

enum { TicketModule = 507 };

enum : Result {
    // found ticket has missmatching rights_id from it's name.
    Result_InvalidTicketBadRightsId = MAKERESULT(TicketModule, 71),
    Result_InvalidTicketVersion = MAKERESULT(TicketModule, 72),
    Result_InvalidTicketKeyType = MAKERESULT(TicketModule, 73),
    Result_InvalidTicketKeyRevision = MAKERESULT(TicketModule, 74),
};

enum TicketSigantureType {
    TicketSigantureType_RSA_4096_SHA1 = 0x010000,
    TicketSigantureType_RSA_2048_SHA1 = 0x010001,
    TicketSigantureType_ECDSA_SHA1 = 0x010002,
    TicketSigantureType_RSA_4096_SHA256 = 0x010003,
    TicketSigantureType_RSA_2048_SHA256 = 0x010004,
    TicketSigantureType_ECDSA_SHA256 = 0x010005,
    TicketSigantureType_HMAC_SHA1_160 = 0x010006,
};

enum TicketTitleKeyType {
    TicketTitleKeyType_Common = 0,
    TicketTitleKeyType_Personalized = 1,
};

enum TicketPropertiesBitfield {
    TicketPropertiesBitfield_None = 0,
    // temporary ticket, removed on restart
    TicketPropertiesBitfield_Temporary = 1 << 4,
};

struct TicketData {
    u8 issuer[0x40];
    u8 title_key_block[0x100];
    u8 ticket_version1;
    u8 title_key_type;
    u16 ticket_version2;
    u8 license_type;
    u8 master_key_revision;
    u16 properties_bitfield;
    u8 _0x148[0x8];
    u64 ticket_id;
    u64 device_id;
    FsRightsId rights_id;
    u32 account_id;
    u8 _0x174[0xC];
};
static_assert(sizeof(TicketData) == 0x180);

struct EticketRsaDeviceKey {
    u8 ctr[AES_128_KEY_SIZE];
    u8 private_exponent[0x100];
    u8 modulus[0x100];
    u32 public_exponent;                ///< Stored using big endian byte order. Must match ETICKET_RSA_DEVICE_KEY_PUBLIC_EXPONENT.
    u8 padding[0x14];
    u64 device_id;
    u8 ghash[0x10];
};
static_assert(sizeof(EticketRsaDeviceKey) == 0x240);

// es functions.
Result Initialize();
void Exit();
Service* GetServiceSession();

// todo: find the ipc that gets personalised tickets.
// todo: if ipc doesn't exist, manually parse es personalised save.
// todo: add personalised -> common ticket conversion.
// todo: make the above an option for both dump and install.

Result ImportTicket(const void* tik_buf, u64 tik_size, const void* cert_buf, u64 cert_size);
Result CountCommonTicket(s32* count);
Result CountPersonalizedTicket(s32* count);
Result ListCommonTicket(s32 *out_entries_written, FsRightsId* out_ids, s32 count);
Result ListPersonalizedTicket(s32 *out_entries_written, FsRightsId* out_ids, s32 count);
Result ListMissingPersonalizedTicket(s32 *out_entries_written, FsRightsId* out_ids, s32 count); // untested
Result GetCommonTicketSize(u64 *size_out, const FsRightsId* rightsId);
Result GetCommonTicketData(u64 *size_out, void *tik_data, u64 tik_size, const FsRightsId* rightsId);
Result GetCommonTicketAndCertificateSize(u64 *tik_size_out, u64 *cert_size_out, const FsRightsId* rightsId); // [4.0.0+]
Result GetCommonTicketAndCertificateData(u64 *tik_size_out, u64 *cert_size_out, void* tik_buf, u64 tik_size, void* cert_buf, u64 cert_size, const FsRightsId* rightsId); // [4.0.0+]

// ticket functions.
Result GetTicketDataOffset(std::span<const u8> ticket, u64& out);
Result GetTicketData(std::span<const u8> ticket, es::TicketData* out);
Result SetTicketData(std::span<u8> ticket, const es::TicketData* in);

Result GetTitleKey(keys::KeyEntry& out, const TicketData& data, const keys::Keys& keys);
Result DecryptTitleKey(keys::KeyEntry& out, u8 key_gen, const keys::Keys& keys);
Result PatchTicket(std::span<u8> ticket, const keys::Keys& keys);

} // namespace sphaira::es
