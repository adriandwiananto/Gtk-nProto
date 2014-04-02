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

gboolean send_reg_jsonstring_to_server(gchar* aesKeyString, unsigned int* retTS, const char* jsonString, const char* serverName)
{
	CURL *curl;
	CURLcode res;

	char *dataBuffer;
	dataBuffer = (char *) malloc ((strlen(jsonString)+5)*sizeof(char));
	if(dataBuffer == NULL) 
		return FALSE;
	
	memset(dataBuffer,0,sizeof(dataBuffer));
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
			free(response.ptr);
			curl_easy_cleanup(curl);
			return FALSE;
		}
		
#ifdef DEBUG_MODE		
		printf("response in string:%s\n", response.ptr);
		printf("length:%d\n", response.len);
#endif
		
		//~ memcpy(serverResponse, response.ptr, response.len);
		json_object * jobj_response = json_tokener_parse(response.ptr);
		json_object * jobj_error = json_object_object_get(jobj_response,"error");
		if(jobj_error != NULL)
			return FALSE;
		json_object * response_status = json_object_object_get(jobj_response,"result");
		if(!strcmp(json_object_get_string(response_status),"Error"))
			return FALSE;
		if(!strcmp(json_object_get_string(response_status),"error"))
			return FALSE;
			
		json_object* json_key = json_object_object_get(jobj_response, "key");
		memcpy(aesKeyString, json_object_get_string(json_key), strlen(json_object_get_string(json_key)));

		json_object* json_TS = json_object_object_get(jobj_response, "last_sync_at");
		*retTS = json_object_get_int(json_TS);

		free(response.ptr);
		
		/* always cleanup */ 
		curl_easy_cleanup(curl);
	}

	return TRUE;
}

gboolean send_log_jsonstring_to_server(gchar* aesKeyString, const char* jsonHeader, const char* jsonLogs, const char* serverName, int* balance_on_server)
{
	const char *headerPost = "header=";
	const char *logsPost = "&logs=";
	CURL *curl;
	CURLcode res;

	char *dataBuffer;
	int total_len = strlen(jsonHeader)+strlen(jsonLogs)+strlen(headerPost)+strlen(logsPost);
	
	dataBuffer = (char *) malloc (total_len+1);
	if(dataBuffer == NULL) 
		return FALSE;
	
	int index = 0;
	
	memset(dataBuffer,0,sizeof(dataBuffer));

	memcpy(dataBuffer,headerPost,strlen(headerPost));
	index += strlen(headerPost);
	
	memcpy(dataBuffer+index,jsonHeader, strlen(jsonHeader));
	index += strlen(jsonHeader);
	
	memcpy(dataBuffer+index,logsPost,strlen(logsPost));
	index += strlen(logsPost);
	
	memcpy(dataBuffer+index,jsonLogs, strlen(jsonLogs));
	index += strlen(jsonLogs);
	
	dataBuffer[index] = '\0';

#ifdef DEBUG_MODE	
	printf("dataBuffer:%s\n",dataBuffer);
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
		printf("response length:%d\n", response.len);
#endif
		
		//~ memcpy(serverResponse, response.ptr, response.len);
		json_object * jobj_response_root = json_tokener_parse(response.ptr);
		json_object * jobj_response_result = json_object_object_get(jobj_response_root, "result");
		if(!strcmp(json_object_get_string(jobj_response_result),"Error"))
			return FALSE;
		if(!strcmp(json_object_get_string(jobj_response_result),"error"))
			return FALSE;
			
		json_object * jobj_response_balance = json_object_object_get(jobj_response_root, "balance");
		*balance_on_server = json_object_get_int(jobj_response_balance);
		
		json_object * jobj_response_key = json_object_object_get(jobj_response_root, "key");
		json_object * jobj_response_key_renew = json_object_object_get(jobj_response_key, "renew");
		if(json_object_get_boolean(jobj_response_key_renew) == TRUE){
			json_object * jobj_response_key_new_key = json_object_object_get(jobj_response_key, "new_key");
			memcpy(aesKeyString, json_object_get_string(jobj_response_key_new_key), strlen(json_object_get_string(jobj_response_key_new_key)));
		}
		write_int64_to_config((uintmax_t)time(NULL), "application.LATS");
		
		free(response.ptr);
		
		/* always cleanup */ 
		curl_easy_cleanup(curl);
	}
	
	return TRUE;
}
