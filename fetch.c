#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>

#include "jsmn.h"



struct string {
  char *ptr;
  size_t len;
};

void init_string(struct string *s) {
  s->len = 0;
  s->ptr = malloc(s->len+1);
  if (s->ptr == NULL) {
    fprintf(stderr, "malloc() failed\n");
    exit(EXIT_FAILURE);
  }
  s->ptr[0] = '\0';
}

size_t writefunc(void *ptr, size_t size, size_t nmemb, struct string *s)
{
  size_t new_len = s->len + size*nmemb;
  s->ptr = realloc(s->ptr, new_len+1);
  if (s->ptr == NULL) {
    fprintf(stderr, "realloc() failed\n");
    exit(EXIT_FAILURE);
  }
  memcpy(s->ptr+s->len, ptr, size*nmemb);
  s->ptr[new_len] = '\0';
  s->len = new_len;

  return size*nmemb;
}

char *pcaSessionHeaderId = NULL;

int makeLoginRequest()
{
	int result=0;
	CURL *curl;
	CURLcode res;
	
	const char data[]="{'PublicApiKey':'6f9229b6-74ad-46d6-89a3-a4ec5a773db4', 'ApiVersion':'2'}";

	curl = curl_easy_init();
	if(curl) 
	{
	    struct string s;
    	init_string(&s);
  
		curl_easy_setopt(curl, CURLOPT_URL, "http://riverlevelsapi.shoothill.com/ApiAccount/ApiLogin");
		
		curl_easy_setopt(curl, CURLOPT_POST, 1L);
		
		curl_easy_setopt(curl, CURLOPT_POSTFIELDS,  data);

		struct curl_slist *chunk = NULL;

      	chunk = curl_slist_append(chunk, "Content-Type: application/json");
      	curl_easy_setopt(curl, CURLOPT_HTTPHEADER, chunk);
		
    	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writefunc);
	    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &s);

		/* Perform the request, res will get the return code */ 
		res = curl_easy_perform(curl);
		/* Check for errors */ 
		if(res != CURLE_OK)
		{
			fprintf(stderr, "curl_easy_perform() failed: %s\n",
		  	curl_easy_strerror(res));
		}
		else
		{
			int r;
			int x;
			char caySession[200];
			jsmn_parser p;
			jsmntok_t tok[10];
			// ok, parse the JSON
			jsmn_init(&p);
			
			memset(caySession,0,sizeof(caySession));	
			r = jsmn_parse(&p, s.ptr, strlen(s.ptr), tok, 10);
			
			for (x=0;x<10;x++)
			{
				if (strncmp(s.ptr+tok[x].start, "SessionHeaderId", tok[x].end - tok[x].start)==0)
				{
					strncpy(caySession, s.ptr+tok[x+1].start, tok[x+1].end - tok[x+1].start);
				}
			}
			
			if (strlen(caySession)>0)
			{
				pcaSessionHeaderId=strdup(caySession);
			
				result=1;
			}
		}

		/* always cleanup */ 
		curl_easy_cleanup(curl);
	}
	return result;	
}


char *convertToString(const char *pcaTxt, jsmntok_t *curTok)
{
	int intSize = curTok->end - curTok->start;
	
	char *pcaReturn = malloc(intSize+1);
	memset(pcaReturn,0, intSize+1);
	memcpy(pcaReturn, pcaTxt+curTok->start, intSize);
	
	return(pcaReturn);
}


int makeBoreHoleRequest(const char *pcaID, char *pcaBuffer)
{
	int result=0;
	CURL *curl;
	CURLcode res;
	
	curl = curl_easy_init();
	if(curl) 
	{
	    struct string s;
    	init_string(&s);
  
		curl_easy_setopt(curl, CURLOPT_URL, "http://riverlevelsapi.shoothill.com/TimeSeries/GetTimeSeriesStations?dataTypes=7");
		
//		curl_easy_setopt(curl, CURLOPT_GET, 1L);
		
		struct curl_slist *chunk = NULL;
		
		char cayBuffer[200];
		sprintf(cayBuffer,"SessionHeaderId: %s", pcaSessionHeaderId);

      //	chunk = curl_slist_append(chunk, "Content-Type: application/json");
      	chunk = curl_slist_append(chunk, cayBuffer);
      	
      	curl_easy_setopt(curl, CURLOPT_HTTPHEADER, chunk);
		
    	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writefunc);
	    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &s);

		/* Perform the request, res will get the return code */ 
		res = curl_easy_perform(curl);
		/* Check for errors */ 
		if(res != CURLE_OK)
		{
			fprintf(stderr, "curl_easy_perform() failed: %s\n",
		  	curl_easy_strerror(res));
		}
		else
		{
			//printf("%s\r\n", s.ptr);
			
			// ok parse out the required data
			
			int r;
			int x;
		
			char *pcaFound = NULL;
		
			jsmn_parser p;
			int maxtok = 100000;
			jsmntok_t *tok = (jsmntok_t*)malloc(maxtok * sizeof(jsmntok_t));
			// ok, parse the JSON
			jsmn_init(&p);
			
			r = jsmn_parse(&p, s.ptr, strlen(s.ptr), tok, maxtok);
			
			for (x=0;x<maxtok;x++)
			{		
				if (tok[x].end>=0 && tok[x].start>=0)
				{ 
					char *pcaCur = convertToString(s.ptr, &tok[x]);
			
					//printf("cur = %s\r\n", pcaCur);
				
					if (strcmp(pcaCur, "id")==0)
					{
						//printf("found id\r\n");
						
						// now read out the ID
						char *pcaCurID = convertToString(s.ptr, &tok[x+1]);
						
						if (strcmp(pcaCurID, pcaID)==0)
						{						
							// ok, now look for the current value
					
							int y;
							for (y=x+1;y<maxtok;y++)
							{		
								char *pcaCur2 = convertToString(s.ptr, &tok[y]);
						
								if (strcmp(pcaCur2, "currentValue")==0)
								{
								
									pcaFound = convertToString(s.ptr, &tok[y+1]);
									break;
								}
							
								free(pcaCur2);
							}		
							
						}
						
						free(pcaCurID);		
					}
					
					free(pcaCur);
				}
			}
			
			if (pcaFound!=NULL)
			{
				strcpy(pcaBuffer,pcaFound);
				free(pcaFound);
				result=1;
			}
		}

		/* always cleanup */ 
		curl_easy_cleanup(curl);
	}
	return result;		
}


int main(void)
{
	char cayBuffer[2000];
	

	if (makeLoginRequest())
	{

		
		char cayBuffer[200];

		// 10983
		// "TOLPUDDLE BALL GW"
		if (makeBoreHoleRequest("10983", cayBuffer))
		{
			printf("%s\r\n", cayBuffer);
		}

	
	}

	return 0;
}


