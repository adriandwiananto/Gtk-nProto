/* credits for init_string and callback function to: 
 * http://stackoverflow.com/a/2329792/3095632
 */

#include "header.h"

static void init_string(ResponseString *s) 
{
	s->len = 0;
	s->ptr = malloc(s->len+1);
	if (s->ptr == NULL) 
	{
		fprintf(stderr, "malloc() failed\n");
		exit(EXIT_FAILURE);
	}
	s->ptr[0] = '\0';
}

static size_t curl_response_to_string(void *ptr, size_t size, size_t nmemb, ResponseString *s)
{
	size_t new_len = s->len + size*nmemb;
	s->ptr = realloc(s->ptr, new_len+1);
	if (s->ptr == NULL) 
	{
		fprintf(stderr, "realloc() failed\n");
		exit(EXIT_FAILURE);
	}
	memcpy(s->ptr+s->len, ptr, size*nmemb);
	s->ptr[new_len] = '\0';
	s->len = new_len;

	return size*nmemb;
}

gboolean send_jsonstring_to_server(gchar* aesKeyString, const char* jsonString, const char* serverName)
{
	CURL *curl;
	CURLcode res;

	char *dataBuffer;
	dataBuffer = (char *) malloc (strlen(jsonString)+5);
	if(dataBuffer == NULL) return FALSE;
	strcpy(dataBuffer,"data=");
	memcpy(dataBuffer+5,jsonString, strlen(jsonString));

#ifdef DEBUG_MODE	
	printf("dataBuffer = %s\n",dataBuffer);
#endif

	/* get a curl handle */ 
	curl = curl_easy_init();
	if(curl) 
	{
		ResponseString response;
		init_string(&response);
		
		/* First set the URL that is about to receive our POST. This URL can
		just as well be a https:// URL if that is what should receive the
		data. */ 
		curl_easy_setopt(curl, CURLOPT_URL, serverName);
		
		/* Now specify the POST data */ 
		curl_easy_setopt(curl, CURLOPT_POSTFIELDS, dataBuffer);
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, curl_response_to_string);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
		
		/* Perform the request, res will get the return code */ 
		res = curl_easy_perform(curl);
		
		/* Check for errors */ 
		if(res != CURLE_OK)
		{
			fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
			return FALSE;
		}
		
#ifdef DEBUG_MODE		
		printf("response in string:%s\n", response.ptr);
		printf("length:%d\n", response.len);
#endif
		
		//~ memcpy(serverResponse, response.ptr, response.len);
		json_object * jobj_response = json_tokener_parse(response.ptr);
		jobj_response = json_object_object_get(jobj_response, "key");
		memcpy(aesKeyString, json_object_get_string(jobj_response), strlen(json_object_get_string(jobj_response)));

		free(response.ptr);
		
		/* always cleanup */ 
		curl_easy_cleanup(curl);
	}

	return TRUE;
}
