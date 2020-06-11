/*
 * tik.c
 *
 * Copyright (c) 2020, DarkMatterCore <pabloacurielz@gmail.com>.
 *
 * This file is part of wad2cntbin (https://github.com/DarkMatterCore/wad2cntbin).
 *
 * wad2cntbin is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * wad2cntbin is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "utils.h"
#include "tik.h"
#include "crypto.h"

bool tikGetTicketTypeAndSize(const void *buf, size_t buf_size, u8 *out_type, size_t *out_size)
{
    if (!buf || buf_size < TIK_MIN_SIZE || (!out_type && !out_size))
    {
        ERROR_MSG("Invalid parameters!");
        return false;
    }
    
    u32 sig_type = 0;
    size_t offset = 0;
    u8 type = TikType_None;
    const u8 *buf_u8 = (const u8*)buf;
    
    memcpy(&sig_type, buf_u8, sizeof(u32));
    sig_type = bswap_32(sig_type);
    
    switch(sig_type)
    {
        case SignatureType_Rsa4096Sha1:
        case SignatureType_Rsa4096Sha256:
            type = TikType_SigRsa4096;
            offset += sizeof(SignatureBlockRsa4096);
            break;
        case SignatureType_Rsa2048Sha1:
        case SignatureType_Rsa2048Sha256:
            type = TikType_SigRsa2048;
            offset += sizeof(SignatureBlockRsa2048);
            break;
        case SignatureType_Ecc480Sha1:
        case SignatureType_Ecc480Sha256:
            type = TikType_SigEcc480;
            offset += sizeof(SignatureBlockEcc480);
            break;
        case SignatureType_Hmac160Sha1:
            type = TikType_SigHmac160;
            offset += sizeof(SignatureBlockHmac160);
            break;
        default:
            ERROR_MSG("Invalid signature type value! (0x%" PRIx32 ").", sig_type);
            return false;
    }
    
    offset += sizeof(TikCommonBlock);
    
    if (offset > buf_size)
    {
        ERROR_MSG("Calculated end offset exceeds certificate buffer size! (0x%" PRIx64 " > 0x%" PRIx64 ").", offset, buf_size);
        return false;
    }
    
    if (out_type) *out_type = type;
    if (out_size) *out_size = offset;
    
    return true;
}

u8 *tikReadTicketFromFile(FILE *fd, size_t ticket_size)
{
    if (!fd || ticket_size < TIK_MIN_SIZE)
    {
        ERROR_MSG("Invalid parameters!");
        return NULL;
    }
    
    u8 *ticket = NULL;
    size_t res = 0;
    
    u8 ticket_type = 0;
    size_t ticket_detected_size = 0;
    
    bool success = false;
    
    /* Allocate memory for the ticket. */
    ticket = malloc(ticket_size);
    if (!ticket)
    {
        ERROR_MSG("Unable to allocate 0x%" PRIx64 " bytes ticket buffer!", ticket_size);
        return NULL;
    }
    
    /* Read ticket. */
    res = fread(ticket, 1, ticket_size, fd);
    if (res != ticket_size)
    {
        ERROR_MSG("Failed to read 0x%" PRIx64 " bytes long ticket!", ticket_size);
        goto out;
    }
    
    /* Check if the ticket size is valid. */
    if (!tikGetTicketTypeAndSize(ticket, ticket_size, &ticket_type, &ticket_detected_size) || ticket_size != ticket_detected_size) goto out;
    
    success = true;
    
out:
    if (!success && ticket)
    {
        free(ticket);
        ticket = NULL;
    }
    
    return ticket;
}

TikCommonBlock *tikGetCommonBlockFromBuffer(void *buf, size_t buf_size, u8 *out_ticket_type)
{
    if (!buf || buf_size < TIK_MIN_SIZE)
    {
        ERROR_MSG("Invalid parameters!");
        return NULL;
    }
    
    u8 ticket_type = 0;
    u8 *buf_u8 = (u8*)buf;
    TikCommonBlock *tik_common_block = NULL;
    
    if (!tikGetTicketTypeAndSize(buf, buf_size, &ticket_type, NULL))
    {
        ERROR_MSG("Invalid ticket!");
        return NULL;
    }
    
    switch(ticket_type)
    {
        case TikType_SigRsa4096:
            tik_common_block = (TikCommonBlock*)(buf_u8 + sizeof(SignatureBlockRsa4096));
            break;
        case TikType_SigRsa2048:
            tik_common_block = (TikCommonBlock*)(buf_u8 + sizeof(SignatureBlockRsa2048));
            break;
        case TikType_SigEcc480:
            tik_common_block = (TikCommonBlock*)(buf_u8 + sizeof(SignatureBlockEcc480));
            break;
        case TikType_SigHmac160:
            tik_common_block = (TikCommonBlock*)(buf_u8 + sizeof(SignatureBlockHmac160));
            break;
        default:
            ERROR_MSG("Invalid ticket type value!");
            break;
    }
    
    if (tik_common_block && out_ticket_type) *out_ticket_type = ticket_type;
    
    return tik_common_block;
}

bool tikIsTitleExportable(TikCommonBlock *tik_common_block)
{
    if (!tik_common_block)
    {
        ERROR_MSG("Invalid parameters!");
        return false;
    }
    
    u64 title_id = bswap_64(tik_common_block->title_id);
    u32 tid_upper = TITLE_UPPER(title_id);
    
    return (tid_upper == TITLE_TYPE_DOWNLOADABLE_CHANNEL || tid_upper == TITLE_TYPE_DISC_BASED_CHANNEL || tid_upper == TITLE_TYPE_DLC);
}

void tikFakesignTicket(void *buf, size_t buf_size)
{
    if (!buf || buf_size < TIK_MIN_SIZE) return;
    
    u8 ticket_type = 0;
    TikCommonBlock *tik_common_block = NULL;
    
    tik_common_block = tikGetCommonBlockFromBuffer(buf, buf_size, &ticket_type);
    if (!tik_common_block) return;
    
    /* Wipe signature. */
    switch(ticket_type)
    {
        case TikType_SigRsa4096:
        {
            SignatureBlockRsa4096 *sig_rsa_4096 = (SignatureBlockRsa4096*)buf;
            memset(sig_rsa_4096->signature, 0, sizeof(sig_rsa_4096->signature));
            break;
        }
        case TikType_SigRsa2048:
        {
            SignatureBlockRsa2048 *sig_rsa_2048 = (SignatureBlockRsa2048*)buf;
            memset(sig_rsa_2048->signature, 0, sizeof(sig_rsa_2048->signature));
            break;
        }
        case TikType_SigEcc480:
        {
            SignatureBlockEcc480 *sig_ecc_480 = (SignatureBlockEcc480*)buf;
            memset(sig_ecc_480->signature, 0, sizeof(sig_ecc_480->signature));
            break;
        }
        case TikType_SigHmac160:
        {
            SignatureBlockHmac160 *sig_hmac_160 = (SignatureBlockHmac160*)buf;
            memset(sig_hmac_160->signature, 0, sizeof(sig_hmac_160->signature));
            break;
        }
        default:
            break;
    }
    
    /* Wipe ECDH data and console ID. */
    memset(tik_common_block->ecdh_data, 0, sizeof(tik_common_block->ecdh_data));
    tik_common_block->console_id = 0;
    
    /* Modify ticket until we get a hash that starts with 0x00. */
    u8 hash[SHA1_HASH_SIZE] = {0};
    u16 *padding = (u16*)tik_common_block->reserved_3;
    
    for(u16 i = 0; i < 65535; i++)
    {
        *padding = bswap_16(i);
        mbedtls_sha1((u8*)tik_common_block, sizeof(TikCommonBlock), hash);
        if (hash[0] == 0) break;
    }
}
