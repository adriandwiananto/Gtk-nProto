#include "header.h"

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
