#include "header.h"

static void sha256_hash_string (unsigned char hash[SHA256_DIGEST_LENGTH], char outputBuffer[65]);

void passwordhashing(char *hashed, const gchar *password, const gchar *salt)
{
	SHA256_CTX context;
	/*pass+salt container*/
	gchar pass_salt_combined[256];
	/*hash result container (in binary)*/
	unsigned char hashbin[SHA256_DIGEST_LENGTH];
	int i;
	
	/*combine pass+salt*/
	memset(pass_salt_combined, 0, 256);
	memcpy(pass_salt_combined, password, strlen(password));
	strcat(pass_salt_combined, salt);
	//~ printf("pass salt combined: %s \n", pass_salt_combined);
	
	/*hash pass+salt and write the binary result to hashbin*/
	SHA256_Init(&context);
	SHA256_Update(&context, (unsigned char*)pass_salt_combined, strlen(pass_salt_combined));
	SHA256_Final(hashbin, &context);
	
	/*convert hash result from binary to string*/
	for(i = 0; i<SHA256_DIGEST_LENGTH; i++)
	{
		sprintf(&hashed[i*2], "%02X", hashbin[i]);
	}
	hashed[SHA256_DIGEST_LENGTH*2]=0;
}

void json_log_array_hashing(char *hashed, const char *json_array)
{
	SHA256_CTX context;
	/*hash result container (in binary)*/
	unsigned char hashbin[SHA256_DIGEST_LENGTH];
	int i;
	
	/*hash log and write the binary result to hashbin*/
	SHA256_Init(&context);
	SHA256_Update(&context, (unsigned char*)json_array, strlen(json_array));
	SHA256_Final(hashbin, &context);
	
	/*convert hash result from binary to string*/
	for(i = 0; i<SHA256_DIGEST_LENGTH; i++)
	{
		sprintf(&hashed[i*2], "%02X", hashbin[i]);
	}
	hashed[SHA256_DIGEST_LENGTH*2]=0;
}

char *unbase64(unsigned char *input, int length)
{
	BIO *b64, *bmem;

	char *buffer = (char *)malloc(length);
	memset(buffer, 0, length);

	b64 = BIO_new(BIO_f_base64());
	BIO_set_flags(b64, BIO_FLAGS_BASE64_NO_NL);
	bmem = BIO_new_mem_buf(input, length);
	bmem = BIO_push(b64, bmem);

	BIO_read(bmem, buffer, length);

	BIO_free_all(bmem);
	return buffer;
}

char *base64(const unsigned char *input, int length)
{
	BIO *bmem, *b64;
	BUF_MEM *bptr;

	b64 = BIO_new(BIO_f_base64());
	bmem = BIO_new(BIO_s_mem());
	b64 = BIO_push(b64, bmem);
	BIO_write(b64, input, length);
	BIO_flush(b64);
	BIO_get_mem_ptr(b64, &bptr);

	char *buff = (char *)malloc(bptr->length);
	memcpy(buff, bptr->data, bptr->length-1);
	buff[bptr->length-1] = 0;

	BIO_free_all(b64);

	return buff;
}

gboolean wrap_aes_key(unsigned char *out, unsigned char *wrapper_key, unsigned char *key_to_wrap)
{
	char wrp_key_len = 0;
	//~ unsigned char wrapped_key[(KEY_LEN_BYTE)+8];
	AES_KEY enc_KEK;
	AES_set_encrypt_key(wrapper_key,256,&enc_KEK);
	wrp_key_len = AES_wrap_key(&enc_KEK, 0, out, key_to_wrap, KEY_LEN_BYTE);
	if(wrp_key_len != ((KEY_LEN_BYTE)+8))return FALSE;	
	else return TRUE;
}

gboolean unwrap_aes_key(unsigned char *out, unsigned char *wrapper_key, unsigned char *key_to_unwrap)
{
	/* unwrap key */
	char unwrp_key_len = 0;
	AES_KEY dec_KEK;
	AES_set_decrypt_key(wrapper_key,256,&dec_KEK);
	unwrp_key_len = AES_unwrap_key(&dec_KEK, 0, out, key_to_unwrap, KEY_LEN_BYTE+8);
	if(unwrp_key_len != KEY_LEN_BYTE)return FALSE;
	else return TRUE;
}

gboolean derive_key(unsigned char *out, const gchar *password, const gchar *salt, unsigned int iteration)
{
	/*int PKCS5_PBKDF2_HMAC_SHA1(const char *pass, int passlen,
                           unsigned char *salt, int saltlen, int iter,
                           int keylen, unsigned char *out);*/
	
	if(PKCS5_PBKDF2_HMAC_SHA1(password, strlen(password), (const unsigned char *)salt, strlen(salt), iteration, SHA256_DIGEST_LENGTH, out) == 1)return TRUE;
	else return FALSE;
}

void getTransKey(unsigned char* aes_key, const gchar* password, const gchar* ACCN, gboolean printResult)
{
	char wrapped_base64[80];
	memset(wrapped_base64,0,80);
	
	//~ unsigned char aes_key[KEY_LEN_BYTE];
	unsigned char KeyEncryptionKey[KEY_LEN_BYTE];
	int i=0;
	
	/* derive key from password + ACCN */
	if(derive_key(KeyEncryptionKey, password, ACCN, 10000) == FALSE)
	{
		error_message("KEK derivation error");
	}
	else
	{
		if (printResult)
		{
			printf("derived key: ");
			for(i=0;i<KEY_LEN_BYTE;i++)printf("%.02X ",KeyEncryptionKey[i]);
			printf("\n\n");
		}
	}
	
	if(get_string_from_config(wrapped_base64,"security.transaction")==FALSE)
	{
		error_message("failed to get transaction key from config");
	}
	if(printResult)printf("b64 key from config : %s\n\n",wrapped_base64);
	unsigned char *wrapped_unbase64 = (unsigned char *) unbase64((unsigned char *)wrapped_base64, strlen(wrapped_base64)+1);

	if (printResult)
	{
		printf("wrapped unbase64 key: ");
		for(i=0;i<KEY_LEN_BYTE+8;i++)printf("%.02X ",*(wrapped_unbase64+i));
		printf("\n\n");
	}
	
		/* unwrap key using KEK */
	if(unwrap_aes_key(aes_key, KeyEncryptionKey, (unsigned char *)wrapped_unbase64) == FALSE)
	{
		error_message("error unwrapping key");
	}
	else
	{
		if (printResult)
		{
			printf("wrapped key: ");
			int i=0;
			for(i=0;i<KEY_LEN_BYTE;i++)printf("%.02X ",aes_key[i]);
			printf("\n\n");
		}
	}
}

gboolean decrypt_transaction_frame(unsigned char* output, unsigned char* input, unsigned char* IV)
{
	/* DO NOT USE IV VALUE AGAIN! 
	 * AFTER DECRYPT USING OpenSSL, IV VALUE CHANGED!! 
	 */

	const gchar *passwordStr;
	uintmax_t ACCN;
	gchar ACCNstr[32];
	memset(ACCNstr, 0, 32);

	passwordStr = gtk_entry_get_text(GTK_ENTRY(passwordwindow->text_entry));

	if(get_INT64_from_config(&ACCN, "application.ACCN") == TRUE)
		sprintf(ACCNstr, "%ju", ACCN);
	else 
		return FALSE;
	
	unsigned char aes_key[32];
	memset(aes_key,0,32);
	
	getTransKey(aes_key, passwordStr, ACCNstr, FALSE);
	
	AES_KEY dec_key;
	AES_set_decrypt_key(aes_key, 256, &dec_key);
	AES_cbc_encrypt(input, output, 32, &dec_key, IV, AES_DECRYPT);
	
	return TRUE;
}

gboolean encrypt_lastTransaction_log(unsigned char* logHexInStr, unsigned int logNum)
{
	unsigned char logPlain[32];
	uintmax_t ACCN;
	if(get_INT64_from_config(&ACCN, "application.ACCN") == FALSE) return FALSE;

	logPlain[0] = (logNum >> 16) & 0xFF;
	logPlain[1] = (logNum >> 8) & 0xFF;
	logPlain[2] = logNum & 0xFF;
	logPlain[3] = lastTransactionData.PT;
	memset(logPlain+4,0,4);
	//SWAPPED:wrong ACCN position
	//~ memcpy(logPlain+8,lastTransactionData.ACCNbyte,6);
	
	int i=0;
	for(i=5; i>=0; i--)
	{
		if(i<5)ACCN >>= 8;
		//SWAPPED:wrong ACCN position
		//~ logPlain[14+i] = ACCN & 0xFF;
		logPlain[8+i] = ACCN & 0xFF;
	}
	
	//SWAPPED:wrong ACCN position
	memcpy(logPlain+14,lastTransactionData.ACCNbyte,6);

	memcpy(logPlain+20, lastTransactionData.AMNTbyte,4);
	memcpy(logPlain+24, lastTransactionData.TSbyte,4);
	logPlain[28] = 0;	//STAT
	logPlain[29] = 0;	//CNL
	memset(logPlain+30,2,2); //PADDING

	unsigned char* buf_ptr;
	
#ifdef DEBUG_MODE	
	unsigned char plainByte[80];
	memset(plainByte,0,80);
	buf_ptr = plainByte;
	for (i = 0; i < 32; i++)
	{
		buf_ptr += sprintf((char*)buf_ptr, "%02X", logPlain[i]);
	}
	*(buf_ptr + 1) = '\0';
	printf("plainByte: %s\n", plainByte);
#endif

	unsigned char logKey[32];
	memset(logKey,0,32);
	
	unsigned char logEncrypted[32];
	memset(logEncrypted,0,32);
	
	unsigned char logToWrite[48];
	memset(logToWrite,0,48);
	
	if(getLogKey(logKey) == FALSE)
	{
		fprintf(stderr,"error derive log key\n");
		return FALSE;
	}
	else
	{
		unsigned char IV[16]; //, iv_dec[AES_BLOCK_SIZE];
		RAND_bytes(IV, 16);
		memcpy(logToWrite+32,IV,16);

#ifdef DEBUG_MODE		
		printf("IV:\n");
		for(i=0;i<16;i++)printf("%02X ", IV[i]);
		printf("\n");
		printf("log key:\n");
		for(i=0;i<32;i++)printf("%02X ", logKey[i]);
		printf("\n");
		printf("logPlain:\n");
		for(i=0;i<32;i++)printf("%02X ", logPlain[i]);
		printf("\n");
#endif

		AES_KEY enc_key;
		AES_set_encrypt_key(logKey, 256, &enc_key);
		AES_cbc_encrypt(logPlain, logEncrypted, 32, &enc_key, IV, AES_ENCRYPT);

#ifdef DEBUG_MODE		
		printf("log encrypted:\n");
		for(i=0;i<32;i++)printf("%02X ", logEncrypted[i]);
		printf("\n");
		
		unsigned char logDecrypted[32];
		memset(logDecrypted,0,32);
		
		memcpy(IV, logToWrite+32, 16);
		printf("IV:\n");
		for(i=0;i<16;i++)printf("%02X ", IV[i]);
		printf("\n");
		printf("log key:\n");
		for(i=0;i<32;i++)printf("%02X ", logKey[i]);
		printf("\n");
		
		AES_KEY dec_key;
		AES_set_decrypt_key(logKey, 256, &dec_key);
		AES_cbc_encrypt(logEncrypted, logDecrypted, 32, &dec_key, IV, AES_DECRYPT);

		printf("log decrypted:\n");
		for(i=0;i<32;i++)printf("%02X ", logDecrypted[i]);
		printf("\n");
#endif
		
		memcpy(logToWrite,logEncrypted,32);
	}

#ifdef DEBUG_MODE	
	printf("log to write (encrypted+iv):\n");
	for(i=0;i<48;i++)printf("%02X ", logToWrite[i]);
	printf("\n");
#endif

	buf_ptr = logHexInStr;
	for (i = 0; i < 48; i++)
	{
		buf_ptr += sprintf((char*)buf_ptr, "%02X", logToWrite[i]);
	}
	*(buf_ptr + 1) = '\0';
	printf("logHexInStr: %s\n", logHexInStr);
	
	return TRUE;
}

gboolean getLogKey(unsigned char* logKey)
{
	uintmax_t ACCN;
	get_INT64_from_config(&ACCN, "application.ACCN");
	const gchar *password;
	password = gtk_entry_get_text(GTK_ENTRY(passwordwindow->text_entry));

	gchar ACCNstr[32];
	memset(ACCNstr, 0, 32);
	sprintf(ACCNstr, "%ju", ACCN);
	
	if(derive_key(logKey, password, ACCNstr, 9000)==FALSE)
	{
		fprintf(stderr,"error deriving key\n");
		return FALSE;
	}
	else
		return TRUE;
}

int calc_sha256_of_file (char* path, char output[65])
{
    FILE* file = fopen(path, "rb");
    if(!file) return -1;

    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256_CTX sha256;
    SHA256_Init(&sha256);
    const int bufSize = 32768;
    char* buffer = malloc(bufSize);
    int bytesRead = 0;
    if(!buffer) return -1;
    while((bytesRead = fread(buffer, 1, bufSize, file)))
    {
        SHA256_Update(&sha256, buffer, bytesRead);
    }
    SHA256_Final(hash, &sha256);

    sha256_hash_string(hash, output);
    fclose(file);
    free(buffer);
    return 0;
}      

static void sha256_hash_string (unsigned char hash[SHA256_DIGEST_LENGTH], char outputBuffer[65])
{
    int i = 0;

    for(i = 0; i < SHA256_DIGEST_LENGTH; i++)
    {
        sprintf(outputBuffer + (i * 2), "%02x", hash[i]);
    }

    outputBuffer[64] = 0;
}
