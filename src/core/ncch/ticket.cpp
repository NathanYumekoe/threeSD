// Copyright 2020 Pengfei Zhu
// Licensed under GPLv2 or any later version
// Refer to the license.txt file included.

#include <cstring>
#include <string_view>
#include "common/alignment.h"
#include "common/assert.h"
#include "common/common_funcs.h"
#include "common/file_util.h"
#include "core/ncch/cia_common.h"
#include "core/ncch/ticket.h"

namespace Core {

bool Ticket::Load(const std::vector<u8> file_data, std::size_t offset) {
    if (!signature.Load(file_data, offset)) {
        return false;
    }
    TRY_MEMCPY(&body, file_data, offset + signature.GetSize(), sizeof(Body));
    return true;
}

bool Ticket::Save(FileUtil::IOFile& file) const {
    // signature
    if (!signature.Save(file)) {
        return false;
    }

    // body
    if (file.WriteBytes(&body, sizeof(body)) != sizeof(body)) {
        LOG_ERROR(Core, "Failed to write body");
        return false;
    }

    return true;
}

std::size_t Ticket::GetSize() const {
    return signature.GetSize() + sizeof(body);
}

constexpr std::string_view TicketIssuer = "Root-CA00000003-XS0000000c";

// TODO: Make use of this?
constexpr std::string_view TicketIssuerDev = "Root-CA00000004-XS00000009";

// From GodMode9
constexpr std::array<u8, 44> TicketContentIndex{
    {0x00, 0x01, 0x00, 0x14, 0x00, 0x00, 0x00, 0xAC, 0x00, 0x00, 0x00, 0x14, 0x00, 0x01, 0x00,
     0x14, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x28, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00,
     0x00, 0x84, 0x00, 0x00, 0x00, 0x84, 0x00, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}};

// Values taken from GodMode9
Ticket BuildFakeTicket(u64 title_id) {
    Ticket ticket{};

    ticket.signature.type = 0x10004;     // RSA_2048 SHA256
    ticket.signature.data.resize(0x100); // Size of RSA_2048 signature
    std::memset(ticket.signature.data.data(), 0xFF, ticket.signature.data.size());

    auto& body = ticket.body;
    std::memcpy(body.issuer.data(), TicketIssuer.data(), TicketIssuer.size());
    std::memset(body.ecc_public_key.data(), 0xFF, body.ecc_public_key.size());
    body.version = 0x01;
    std::memset(body.title_key.data(), 0xFF, body.title_key.size());
    body.title_id = title_id;
    body.common_key_index = 0x00;
    body.audit = 0x01;
    std::memcpy(body.content_index.data(), TicketContentIndex.data(), TicketContentIndex.size());
    // GodMode9 by default sets all remaining 0x80 bytes to 0xFF
    std::memset(body.content_index.data() + TicketContentIndex.size(), 0xFF, 0x80);
    return ticket;
}

} // namespace Core
