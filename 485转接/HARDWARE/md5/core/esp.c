/*
 * embedded IPsec
 * Copyright (c) 2003 Niklaus Schild and Christian Scheurer, HTI Biel/Bienne
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT
 * SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT
 * OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
 * IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY
 * OF SUCH DAMAGE.
 *
 */

/** @file esp.c
 *  @brief This module contains the Encapsulating Security Payload code
 *
 *  @author  Niklaus Schild <n.schild@gmx.ch> <BR>
 *
 *  <B>OUTLINE:</B>
 *
 *  <B>IMPLEMENTATION:</B>
 * All functions work in-place (i.g. mainipulate directly the original
 * packet without copying any data). For the encapsulation routine,
 * the caller must ensure that space for the new IP and ESP header are
 * available in front of the packet:
 *
 *  <pre>
 *                              | pointer to packet header
 *     ________________________\/________________________________________________
 *    |          ¦       ¦      ¦                             ¦ padd       ¦ ev. |
 *    | Ethernet ¦ newIP ¦ ESP  ¦   original (inner) packet   ¦ next-proto ¦ ICV |
 *    |__________¦_______¦______¦_____________________________¦____________¦_____|
 *    ¦                         ¦                             ¦                  ¦
 *    ¦<-room for new headers-->¦                             ¦<-   room tail  ->¦
 *  </pre>
 *
 * This document is part of <EM>embedded IPsec<BR>
 * Copyright (c) 2003 Niklaus Schild and Christian Scheurer, HTI Biel/Bienne<BR>
 * All rights reserved.<BR>
 * This file contains code from the OpenSSL Project<BR>
 * portions Copyright (c) 1998-2003 OpenSSL (www.openssl.org)
 *</EM><HR>
 */

#include <string.h>

#include "ipsec/ipsec.h"
#include "ipsec/util.h"
#include "ipsec/debug.h"

#include "ipsec/sa.h"
#include "ipsec/des.h"
#include "ipsec/md5.h"
#include "ipsec/sha1.h"

#include "ipsec/esp.h"

#include "aes.h"
#include "hmac_sha2.h"

__u32 ipsec_esp_bitmap 	= 0;        		/**< save session state to detect replays - must be 32 bits. 
											 *   Note: must be initialized with zero (0x00000000) when
											 *         a new SA is established! */
__u32 ipsec_esp_lastSeq	= 0;         		/**< save session state to detect replays
											 *   Note: must be initialized with zero (0x00000000) when
											 *         a new SA is established! */



/**
 * Returns the number of padding needed for a certain ESP packet size
 *
 * @param	len		the length of the packet
 * @return	the length of padding needed
 */
__u8 ipsec_esp_get_padding(int len, __u8 enc_alg)
{
	int padding = 0;
	int align = 0;

	switch(enc_alg){
	case IPSEC_3DES:
		align = 8;
		break;
	case IPSEC_AES_ECB:
	case IPSEC_AES_CBC:
	case IPSEC_AES_OFB:
	case IPSEC_AES_CFB:
	case IPSEC_AES_CTR:
		align = 16;
		break;
	default:
		align = 0; /* no padding */
		break;
	}

	for(padding = 0; padding < align; padding++)
		if(((len+padding) % align) == 0)
			break ;
	return padding ;
}

/**
 * Decapsulates an IP packet containing an ESP header.
 *
 * @param	packet 	pointer to the ESP header
 * @param 	offset	pointer to the offset which is passed back
 * @param 	len		pointer to the length of the decapsulated packet
 * @param 	sa		pointer to the SA
 * @return IPSEC_STATUS_SUCCESS 	if the packet could be decapsulated properly
 * @return IPSEC_STATUS_FAILURE		if the SA's authentication algorithm was invalid or if ICV comparison failed
 * @return IPSEC_STATUS_BAD_PACKET	if the decryption gave back a strange packet
 */
ipsec_status ipsec_esp_decapsulate(ipsec_ip_header *packet, int *offset, int *len, void *satmp)
{
	sad_entry *sa = (sad_entry *)satmp;
	int ret_val = IPSEC_STATUS_NOT_INITIALIZED;			/* by default, the return value is undefined */
	__u8 				ip_header_len ;
	int					local_len ;
	int					payload_offset ;
	int					payload_len ;
	ipsec_ip_header		*new_ip_packet ;
	esp_packet			*esp_header ;

	char 				cbc_iv[IPSEC_ESP_IV_SIZE_MAX];
	unsigned char 		digest[IPSEC_MAX_AUTHKEY_LEN];

	IPSEC_LOG_TRC(IPSEC_TRACE_ENTER,
				  "ipsec_esp_decapsulate",
				  ("packet=%p, *offset=%d, *len=%d sa=%p",
				   (void *)packet, *offset, *len, (void *)sa)
				  );
	
	ip_header_len = (packet->v_hl & 0x0f) * 4 ;
	esp_header = (esp_packet*)(((char*)packet)+ip_header_len) ;
	payload_offset = ip_header_len + IPSEC_ESP_SPI_SIZE + IPSEC_ESP_SEQ_SIZE ;
	payload_len = ipsec_ntohs(packet->len) - ip_header_len - IPSEC_ESP_HDR_SIZE ;


	print_hex((char*)esp_header, IPSEC_ESP_HDR_SIZE, "esp_header");
	print_hex((char*)esp_header + IPSEC_ESP_HDR_SIZE, sa->iv_bytes_len, "iv");

	if(sa->auth_alg != 0)
	{

		/* preliminary anti-replay check (without updating the global sequence number window)     */
		/* This check prevents useless ICV calculation if the Sequence Number is obviously wrong  */
		ret_val = ipsec_check_replay_window(ipsec_ntohl(esp_header->sequence), ipsec_esp_lastSeq, ipsec_esp_bitmap);
		if(ret_val != IPSEC_AUDIT_SUCCESS)
		{
			IPSEC_LOG_AUD("ipsec_esp_decapsulate", IPSEC_AUDIT_SEQ_MISMATCH,
						  ("packet rejected by anti-replay check (lastSeq=%08lx, seq=%08lx, window size=%d)",
						   ipsec_esp_lastSeq, ipsec_ntohl(esp_header->sequence), IPSEC_SEQ_MAX_WINDOW) );
			return ret_val;
		}

		/* recalcualte ICV */
		switch(sa->auth_alg) {

		case IPSEC_HMAC_MD5:
			hmac_md5((unsigned char *)esp_header, payload_len - sa->icv_bytes_len +IPSEC_ESP_HDR_SIZE,
					 (unsigned char *)sa->authkey, sa->auth_key_len/*IPSEC_AUTH_MD5_KEY_LEN*/, (unsigned char *)&digest);
			ret_val = IPSEC_STATUS_SUCCESS;
			break;
		case IPSEC_HMAC_SHA1:
			hmac_sha1((unsigned char *)esp_header, payload_len- sa->icv_bytes_len +IPSEC_ESP_HDR_SIZE,
					  (unsigned char *)sa->authkey, sa->auth_key_len/*IPSEC_AUTH_SHA1_KEY_LEN*/, (unsigned char *)&digest);

			//			void hmac_sha1(unsigned char* text, int text_len,
			//						   unsigned char*  key, int key_len, unsigned char*  digest)

			ret_val = IPSEC_STATUS_SUCCESS;
			break;
		case IPSEC_HMAC_SHA224:

			//			void hmac_sha224(const unsigned char *key, unsigned int key_size,
			//			                 const unsigned char *message, unsigned int message_len,
			//			                 unsigned char *mac, unsigned mac_size);

			hmac_sha224((unsigned char *)sa->authkey, sa->auth_key_len/8,
						(unsigned char *)esp_header, payload_len- sa->icv_bytes_len +IPSEC_ESP_HDR_SIZE,
						(unsigned char *)&digest, sa->icv_bytes_len);

			ret_val = IPSEC_STATUS_SUCCESS;
			break;
		case IPSEC_HMAC_SHA256:
			printf("IPSEC_HMAC_SHA256 \n");
			hmac_sha256((unsigned char *)sa->authkey, sa->auth_key_len/8,
						(unsigned char *)esp_header, payload_len- sa->icv_bytes_len +IPSEC_ESP_HDR_SIZE,
						(unsigned char *)&digest, sa->icv_bytes_len);

			print_hex(digest, sa->icv_bytes_len, "icv");

			ret_val = IPSEC_STATUS_SUCCESS;
			break;
		case IPSEC_HMAC_SHA384:
			hmac_sha384((unsigned char *)sa->authkey, sa->auth_key_len/8,
						(unsigned char *)esp_header, payload_len- sa->icv_bytes_len +IPSEC_ESP_HDR_SIZE,
						(unsigned char *)&digest, sa->icv_bytes_len);

			ret_val = IPSEC_STATUS_SUCCESS;
			break;
		case IPSEC_HMAC_SHA512:
			hmac_sha512((unsigned char *)sa->authkey, sa->auth_key_len/8,
						(unsigned char *)esp_header, payload_len- sa->icv_bytes_len +IPSEC_ESP_HDR_SIZE,
						(unsigned char *)&digest, sa->icv_bytes_len);

			ret_val = IPSEC_STATUS_SUCCESS;
			break;
		default:
			IPSEC_LOG_ERR("ipsec_esp_decapsulate", IPSEC_STATUS_FAILURE, ("unknown HASH algorithm for this ESP")) ;
			IPSEC_LOG_TRC(IPSEC_TRACE_RETURN, "ipsec_esp_decapsulate", ("return = %d", IPSEC_STATUS_FAILURE) );
			return IPSEC_STATUS_FAILURE;
		}
		
		/* compare ICV */
		if(memcmp(((char*)esp_header)+IPSEC_ESP_HDR_SIZE+payload_len - sa->icv_bytes_len, digest,  sa->icv_bytes_len ) != 0) {
			IPSEC_LOG_ERR("ipsec_esp_decapsulate", IPSEC_STATUS_FAILURE, ("ESP ICV does not match")) ;
			IPSEC_LOG_TRC(IPSEC_TRACE_RETURN, "ipsec_esp_decapsulate", ("return = %d", IPSEC_STATUS_FAILURE) );
			return IPSEC_STATUS_FAILURE;
		}

		/* reduce payload by ICV */
		payload_len -=  sa->icv_bytes_len;

		/* post-ICV calculationn anti-replay check (this call will update the global sequence number window) */
		ret_val = ipsec_update_replay_window(ipsec_ntohl(esp_header->sequence), (__u32 *)&ipsec_esp_lastSeq, (__u32 *)&ipsec_esp_bitmap);
		if(ret_val != IPSEC_AUDIT_SUCCESS)
		{
			IPSEC_LOG_AUD("ipsec_esp_decapsulate", IPSEC_AUDIT_SEQ_MISMATCH,
						  ("packet rejected by anti-replay update (lastSeq=%08lx, seq=%08lx, window size=%d)",
						   ipsec_esp_lastSeq, ipsec_ntohl(esp_header->sequence), IPSEC_SEQ_MAX_WINDOW) );
			return ret_val;
		}

	}


	/* decapsulate the packet according the SA */
	switch(sa->enc_alg){
	case IPSEC_3DES:
		/* copy IV from ESP payload */
		memcpy(cbc_iv, ((char*)packet)+payload_offset, sa->iv_bytes_len);

		/* decrypt ESP packet */
		cipher_3des_cbc(((char*)packet)+payload_offset + sa->iv_bytes_len, payload_len - sa->iv_bytes_len,
						(unsigned char *)sa->enckey, (char*)&cbc_iv,
						DES_DECRYPT, ((char*)packet)+payload_offset + sa->iv_bytes_len);

		printf("key len: %d \n", strlen(sa->enckey));
		//		void cipher_3des_cbc(unsigned char* text, int text_len,
		//		                     unsigned char* key, unsigned char* iv,
		//							 int mode, unsigned char*  output)
		break;
	case IPSEC_AES_CBC:{
		/* copy IV from ESP payload */
		memcpy(cbc_iv, ((char*)packet)+payload_offset, sa->iv_bytes_len);

		AES_KEY dec_key;
		AES_set_decrypt_key((unsigned char *)sa->enckey, sa->enc_key_len, &dec_key);
		AES_cbc_encrypt(((unsigned char*)packet)+payload_offset + sa->iv_bytes_len,
						((unsigned char*)packet)+payload_offset + sa->iv_bytes_len,
						(payload_len - sa->iv_bytes_len),
						&dec_key,
						(unsigned char*)&cbc_iv,
						AES_DECRYPT);
	}
		break;
	case IPSEC_AES_ECB:
		/* none IV*/



		break;
	case IPSEC_AES_OFB:
		/* copy IV from ESP payload */
		memcpy(cbc_iv, ((char*)packet)+payload_offset, sa->iv_bytes_len);


		break;
	case IPSEC_AES_CFB:
		/* copy IV from ESP payload */
		memcpy(cbc_iv, ((char*)packet)+payload_offset, sa->iv_bytes_len);


		break;
	case IPSEC_AES_CTR:
		/* copy IV from ESP payload */
		memcpy(cbc_iv, ((char*)packet)+payload_offset, sa->iv_bytes_len);


		break;
	default:
		return IPSEC_STATUS_FAILURE;
	}



	*offset = payload_offset + sa->iv_bytes_len ;

	new_ip_packet = (ipsec_ip_header*)(((char*)packet) + payload_offset + sa->iv_bytes_len) ;
	local_len = ipsec_ntohs(new_ip_packet->len) ;

	print_hex((unsigned char*)new_ip_packet, local_len, "local new_ip_packet");

	if( (local_len < IPSEC_MIN_IPHDR_SIZE) || (local_len > IPSEC_MTU))
	{
		IPSEC_LOG_ERR("ipsec_esp_decapsulate", IPSEC_STATUS_FAILURE, ("decapsulated strange packet")) ;
		IPSEC_LOG_TRC(IPSEC_TRACE_RETURN, "ipsec_esp_decapsulate", ("return = %d", IPSEC_STATUS_BAD_PACKET) );
		return IPSEC_STATUS_BAD_PACKET;
	}
	*len = local_len ;

	sa->sequence_number++ ;

	IPSEC_LOG_TRC(IPSEC_TRACE_RETURN, "ipsec_esp_decapsulate", ("return = %d", IPSEC_STATUS_SUCCESS) );
	return IPSEC_STATUS_SUCCESS;
}

/**
 * Encapsulates an IP packet into an ESP packet which will again be added to an IP packet.
 *
 * @param	packet		pointer to the IP packet
 * @param 	offset		pointer to the offset which will point to the new encapsulated packet
 * @param 	len			pointer to the length of the new encapsulated packet
 * @param 	sa			pointer to the SA
 * @param 	src_addr	source IP address of the outer IP header
 * @param 	dest_addr	destination IP address of the outer IP header
 * @return 	IPSEC_STATUS_SUCCESS		if the packet was properly encapsulated
 * @return 	IPSEC_STATUS_TTL_EXPIRED	if the TTL expired
 * @return  IPSEC_STATUS_FAILURE		if the SA contained a bad authentication algorithm
 */
ipsec_status ipsec_esp_encapsulate(ipsec_ip_header *packet, int *offset, int *len, void *satmp, __u32 src_addr, __u32 dest_addr)
{
	sad_entry *sa = (sad_entry *)satmp;
	int ret_val = IPSEC_STATUS_NOT_INITIALIZED;			/* by default, the return value is undefined */
	__u8				tos ;
	int					inner_len ;
	int					payload_offset ;
	int					payload_len ;
	__u8				padd_len ;
	__u8				*pos ;
	__u8				padd ;
	ipsec_ip_header		*new_ip_header ;
	ipsec_esp_header	*new_esp_header ;

	unsigned char 		digest[IPSEC_MAX_AUTHKEY_LEN];

	IPSEC_LOG_TRC(IPSEC_TRACE_ENTER,
				  "ipsec_esp_encapsulate",
				  ("packet=%p, *offset=%d, *len=%d, sa=%p, src_addr=%lu, dest_addr=%lu",
				   (void *)packet, *offset, *len, (void *)sa, src_addr, dest_addr)
				  );

	/* set new packet header pointers */
	new_ip_header = (ipsec_ip_header*)(((char*)packet) - sa->iv_bytes_len - IPSEC_ESP_HDR_SIZE - IPSEC_MIN_IPHDR_SIZE) ;
	new_esp_header = (ipsec_esp_header*)(((char*)packet) - sa->iv_bytes_len - IPSEC_ESP_HDR_SIZE) ;
	payload_offset = (((char*)packet) - ((char*)new_ip_header)) ;

	inner_len = ipsec_ntohs(packet->len) ;


	/* save TOS from inner header */
	tos = packet->tos ;

	/** @FIXME TTL update and checksum calculation */
	// packet->ttl--;
	// packet->chksum = ip_chksum(packet, sizeof(ip_header));
	if (packet->ttl == 0)
	{
		IPSEC_LOG_TRC(IPSEC_TRACE_RETURN, "ipsec_esp_encapsulate", ("return = %d", IPSEC_STATUS_TTL_EXPIRED) );
		return IPSEC_STATUS_TTL_EXPIRED;
	}
	
	/* add padding if needed */
	padd_len = ipsec_esp_get_padding(inner_len+2, sa->enc_alg) ;
	pos = ((char*)packet)+inner_len ;
	if(padd_len != 0)
	{
		padd = 1 ;
		while(padd <= padd_len)
			*pos++ = padd++ ;
	}
	


	/* append padding length and next protocol field to the payload */
	*pos++ = padd_len ;
	/* in tunnel mode the next protocol field is always IP */
	*pos = 0x04 ;

	payload_len = IPSEC_ESP_HDR_SIZE + sa->iv_bytes_len + inner_len + padd_len + 2 ;


	printf("payload_offset: %d \n", payload_offset);
	printf("inner_len: %d \n", inner_len);
	printf("padd_len: %d \n", padd_len);
	printf("payload_len: %d \n", payload_len);

	print_hex(sa->iv, sa->iv_bytes_len, "iv");

	/* decapsulate the packet according the SA */
	switch(sa->enc_alg){
	case IPSEC_3DES:
		/* encrypt ESP packet */
		cipher_3des_cbc((__u8 *)packet, inner_len+padd_len+2,
						(__u8 *)sa->enckey, sa->iv,
						DES_ENCRYPT, (__u8 *)packet);
		break;
	case IPSEC_AES_CBC:{
		unsigned char iv_enc[AES_BLOCK_SIZE];
		memcpy(iv_enc, sa->iv, sa->iv_bytes_len);


		AES_KEY enc_key;
		AES_set_encrypt_key(sa->enckey, sa->enc_key_len, &enc_key);
		AES_cbc_encrypt((__u8 *)packet, (__u8 *)packet, (inner_len+padd_len+2), &enc_key, iv_enc, AES_ENCRYPT);
		print_hex((__u8 *)packet, (inner_len+padd_len+2), "packet encrypted");

	}
		break;
	case IPSEC_AES_ECB:
		/* none IV*/



		break;
	case IPSEC_AES_OFB:{
		unsigned char iv_enc[AES_BLOCK_SIZE];
		memcpy(iv_enc, sa->iv, sa->iv_bytes_len);



	}
		break;
	case IPSEC_AES_CFB:{
		unsigned char iv_enc[AES_BLOCK_SIZE];
		memcpy(iv_enc, sa->iv, sa->iv_bytes_len);



	}


		break;
	case IPSEC_AES_CTR:{
		unsigned char iv_enc[AES_BLOCK_SIZE];
		memcpy(iv_enc, sa->iv, sa->iv_bytes_len);



	}


		break;
	default:
		return IPSEC_STATUS_FAILURE;
	}


	/* insert IV in fron of packet */
	memcpy( ((char*)packet) - sa->iv_bytes_len, sa->iv, sa->iv_bytes_len);

	print_hex((char *)packet - sa->iv_bytes_len,
			  (sa->iv_bytes_len + inner_len+padd_len+2), "packet encrypted + iv");


	/* setup ESP header */
	new_esp_header->spi = sa->spi;

	/** 1st packet needs to be sent out with squ = 1 */
	sa->sequence_number++ ;
	new_esp_header->sequence_number = ipsec_htonl(sa->sequence_number);

	print_hex((char *)new_esp_header,
			  (IPSEC_ESP_HDR_SIZE + sa->iv_bytes_len + inner_len+padd_len+2),
			  "esph + iv + packet encrypted");


	/* calculate the ICV if needed */
	if(sa->auth_alg != 0)
	{
		/* recalcualte ICV */
		switch(sa->auth_alg) {

		case IPSEC_HMAC_MD5:
			printf("IPSEC_HMAC_MD5 \n");
			hmac_md5((unsigned char *)new_esp_header, payload_len,
					 (unsigned char *)sa->authkey, sa->auth_key_len/*IPSEC_AUTH_MD5_KEY_LEN*/, (unsigned char *)&digest);
			ret_val = IPSEC_STATUS_SUCCESS;
			break;
		case IPSEC_HMAC_SHA1:
			printf("IPSEC_HMAC_SHA1 \n");
			hmac_sha1((unsigned char *)new_esp_header, payload_len,
					  (unsigned char *)sa->authkey, sa->auth_key_len/*IPSEC_AUTH_SHA1_KEY_LEN*/, (unsigned char *)&digest);
			ret_val = IPSEC_STATUS_SUCCESS;

			break;
		case IPSEC_HMAC_SHA224:
			printf("IPSEC_HMAC_SHA224 \n");

			hmac_sha224((unsigned char *)sa->authkey, sa->auth_key_len/8,
						(unsigned char *)new_esp_header, payload_len,
						(unsigned char *)&digest, sa->icv_bytes_len);

			ret_val = IPSEC_STATUS_SUCCESS;
			break;
		case IPSEC_HMAC_SHA256:
			printf("IPSEC_HMAC_SHA256 \n");
			print_hex(sa->authkey, sa->auth_key_len/8, "authkey");

			hmac_sha256(sa->authkey, sa->auth_key_len/8,
						(unsigned char *)new_esp_header, payload_len,
						(unsigned char *)&digest, sa->icv_bytes_len);

			print_hex(digest, sa->icv_bytes_len, "ICV (hmac_sha256)");
			ret_val = IPSEC_STATUS_SUCCESS;
			break;
		case IPSEC_HMAC_SHA384:
			printf("IPSEC_HMAC_SHA384 \n");
			print_hex(sa->authkey, sa->auth_key_len/8, "authkey");

			hmac_sha384(sa->authkey, sa->auth_key_len/8,
						(unsigned char *)new_esp_header, payload_len,
						(unsigned char *)&digest, sa->icv_bytes_len);

			ret_val = IPSEC_STATUS_SUCCESS;
			break;
		case IPSEC_HMAC_SHA512:
			printf("IPSEC_HMAC_SHA512 \n");
			print_hex(sa->authkey, sa->auth_key_len/8, "authkey");

			hmac_sha512(sa->authkey, sa->auth_key_len/8,
						(unsigned char *)new_esp_header, payload_len,
						(unsigned char *)&digest, sa->icv_bytes_len);

			ret_val = IPSEC_STATUS_SUCCESS;
			break;
		default:
			printf("sa->auth_alg = %d \n", sa->auth_alg);
			IPSEC_LOG_ERR("ipsec_esp_encapsulate", IPSEC_STATUS_FAILURE, ("unknown HASH algorithm for this ESP")) ;
			IPSEC_LOG_TRC(IPSEC_TRACE_RETURN, "ipsec_esp_encapsulate", ("return = %d", IPSEC_STATUS_FAILURE) );
			return IPSEC_STATUS_FAILURE;
		}
		
		/* set ICV */
		memcpy(((char*)new_esp_header)+payload_len, digest, sa->icv_bytes_len);
		
		/* increase payload by ICV */
		payload_len += sa->icv_bytes_len ;
	}

	print_hex((char *)new_esp_header, payload_len, "esph + iv + packet encrypted + icv");


	/* setup IP header */
	new_ip_header->v_hl = 0x45 ;
	new_ip_header->tos = tos ;
	new_ip_header->len = ipsec_htons(payload_len+ IPSEC_MIN_IPHDR_SIZE);
	new_ip_header->id = 1000 ;	/**@FIXME id must be generated properly and incremented */
	new_ip_header->offset = 0 ; /*@FIXME set value for offset*/
	new_ip_header->ttl = 64 ; /*@FIXME set value for ttl*/
	new_ip_header->protocol = IPSEC_PROTO_ESP ;
	new_ip_header->chksum = 0 ;
	new_ip_header->src = src_addr ;
	new_ip_header->dest = dest_addr ;

	/* set checksum */
	new_ip_header->chksum = ipsec_ip_chksum(new_ip_header, sizeof(ipsec_ip_header)) ;

	print_hex((char *)new_ip_header, (payload_len + IPSEC_MIN_IPHDR_SIZE), "ipsec new_ip_header");


	/* setup return values */
	*offset = payload_offset*(-1) ;
	*len = payload_len + IPSEC_MIN_IPHDR_SIZE ;

	IPSEC_LOG_TRC(IPSEC_TRACE_RETURN, "ipsec_esp_encapsulate", ("return = %d", IPSEC_STATUS_SUCCESS) );
	return IPSEC_STATUS_SUCCESS;
}

