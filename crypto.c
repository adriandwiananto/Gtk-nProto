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
	printf("pass salt combined: %s \n", pass_salt_combined);
	
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
