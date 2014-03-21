#include "header.h"
#include <sqlite3.h>

int logNum()
{
	sqlite3 *db;
	int  rc;
	const char *zSql = "SELECT count(*) FROM TransLog;";
	sqlite3_stmt *res;
	const char *tail;

	int dataInt = 0;
	
	/* Open database */
	rc = sqlite3_open("log.db", &db);
	if( rc )
	{
		fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
		return FALSE;
	}

	/* Create SQL statement */
	if(sqlite3_prepare_v2(db, zSql, strlen(zSql), &res, &tail) != SQLITE_OK)
	{
		sqlite3_close(db);
		printf("Can't retrieve data: %s\n", sqlite3_errmsg(db));
		return(1);
	}

	while(sqlite3_step(res) == SQLITE_ROW)
	{
		dataInt = sqlite3_column_int(res,0);
#ifdef DEBUG_MODE
		printf("\n\nDataInt: %d\n\n", dataInt);
#endif
	}


	sqlite3_finalize(res);

	sqlite3_close(db);
	
	return dataInt;
}

gboolean createDB_and_table()
{
	sqlite3 *db;
	char *zErrMsg = 0;
	int  rc;
	char *sql;

	/* Open database */
	rc = sqlite3_open("log.db", &db);
	if( rc )
	{
		fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
		return FALSE;
	}

	/* Create SQL statement */
	sql = 	"CREATE TABLE TransLog("  \
			"ID INT PRIMARY KEY     NOT NULL," \
			"LOG           	BLOB    NOT NULL);" ;

	/* Execute SQL statement */
	rc = sqlite3_exec(db, sql, NULL, NULL, &zErrMsg);
	if( rc != SQLITE_OK )
	{
		fprintf(stderr, "SQL error: %s\n", zErrMsg);
		sqlite3_free(zErrMsg);
		return FALSE;
	}
	else fprintf(stdout, "Table created successfully\n");
	sqlite3_close(db);
	return TRUE;
}

gboolean write_lastTransaction_log()
{
	sqlite3 *db;
	char *zErrMsg = 0;
	int rc;
	char sql[512];
	memset(sql,0,512);
	unsigned char logHexInStr[255];
	memset(logHexInStr,0,255);
	unsigned int logRowToWrite = logNum()+1;
	if(encrypt_lastTransaction_log(logHexInStr, logRowToWrite) == FALSE)return FALSE;
	
	/* Open database */
	rc = sqlite3_open("log.db", &db);
	if( rc )
	{
		fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
		return FALSE;
	}

	/* Create SQL statement */
	sprintf(sql, "INSERT INTO TransLog(ID, LOG) VALUES(%u, x'%s');", logRowToWrite, logHexInStr);

	/* Execute SQL statement */
	rc = sqlite3_exec(db, sql, NULL, NULL, &zErrMsg);
	if( rc != SQLITE_OK )
	{
		fprintf(stderr, "SQL error: %s\n", zErrMsg);
		sqlite3_free(zErrMsg);
		return FALSE;
	}
	else fprintf(stdout, "Records created successfully\n");

	sqlite3_close(db);
	return TRUE;
}

gboolean update_encrypted_log(char* logToWriteInStr, int Row)
{
	sqlite3 *db;
	char *zErrMsg = 0;
	int rc;
	char sql[512];
	memset(sql,0,512);
	
	/* Open database */
	rc = sqlite3_open("log.db", &db);
	if( rc )
	{
		fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
		return FALSE;
	}

	/* Create SQL statement */
	sprintf(sql, "UPDATE TransLog SET LOG=x'%s' WHERE ID=%u;", logToWriteInStr, Row);

	/* Execute SQL statement */
	rc = sqlite3_exec(db, sql, NULL, NULL, &zErrMsg);
	if( rc != SQLITE_OK )
	{
		fprintf(stderr, "SQL error: %s\n", zErrMsg);
		sqlite3_free(zErrMsg);
		return FALSE;
	}
	else fprintf(stdout, "Records #%u updated successfully\n", Row);

	sqlite3_close(db);
	return TRUE;
}

int read_log_blob(unsigned char *dest, int row)
{
	sqlite3 *db;
	int  rc;
	char zSql[64];
	memset(zSql,0,64);
	sqlite3_stmt *res;
	const char *tail;
	char* blob;
	int len = 0;
	
	sprintf(zSql,"SELECT hex(LOG) FROM TransLog WHERE ID = %d;",row);

	/* Open database */
	rc = sqlite3_open("log.db", &db);
	if( rc )
	{
		fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
		return 0;
	}

	/* Create SQL statement */
	if(sqlite3_prepare_v2(db, zSql, strlen(zSql), &res, &tail) != SQLITE_OK)
	{
		sqlite3_close(db);
		printf("Can't retrieve data: %s\n", sqlite3_errmsg(db));
		return 0;
	}

	while(sqlite3_step(res) == SQLITE_ROW)
	{
		const void * pFileContent = sqlite3_column_blob(res, 0 );
		blob = (char*)pFileContent;
		len = sqlite3_column_bytes(res, 0 );
		memcpy(dest,blob,len);
#ifdef DEBUG_MODE
		printf ("file_content=:%s\n", dest );
		printf("len=%d\n",len);
#endif
	}

	sqlite3_finalize(res);

	sqlite3_close(db);
	
	return len;
}

void convert_DBdata_to_TreeView_Data(unsigned char *DB_BLOB_data, int logLen, 
unsigned int *lognum, char *timebuffer, uintmax_t *senderACCN, unsigned int*amount)
{
	unsigned char fromDBbyte[64];
	memset(fromDBbyte,0,64);
	
	unsigned char IV[16];
	memset(IV,0,16);

	unsigned char logKey[32];
	memset(logKey,0,32);
	
	unsigned char logDecrypted[32];
	memset(logDecrypted,0,32);
	int z=0;

	if(getLogKey(logKey)==FALSE)fprintf(stderr,"error deriving key\n");

	hexstrToBinArr(fromDBbyte,(gchar*)DB_BLOB_data,logLen/2);
	
	memcpy(IV, fromDBbyte+32, 16);	

#ifdef DEBUG_MODE
	printf("from DB: %s, length: %d\n",DB_BLOB_data,logLen);

	printf("byte array:\n");
	for(z=0;z<logLen/2;z++)printf("%02X ",fromDBbyte[z]);
	printf("\n");

	printf("IV:\n");
	for(z=0;z<16;z++)printf("%02X ", IV[z]);
	printf("\n");
#endif

	AES_KEY dec_key;
	AES_set_decrypt_key(logKey, 256, &dec_key);
	AES_cbc_encrypt(fromDBbyte, logDecrypted, 32, &dec_key, IV, AES_DECRYPT);

	unsigned int TS = (logDecrypted[24]<<24) | (logDecrypted[25]<<16) | (logDecrypted[26]<<8) | (logDecrypted[27]);
	time_t rawtime = TS;
	struct tm *timeinfo;
	timeinfo = localtime(&rawtime);
	strftime (timebuffer,80,"%d/%m/%Y %H:%M",timeinfo);

#ifdef DEBUG_MODE
	printf("log key:\n");
	for(z=0;z<32;z++)printf("%02X ", logKey[z]);
	printf("\n");

	printf("log decrypted:\n");
	for(z=0;z<32;z++)printf("%02X ", logDecrypted[z]);
	printf("\n");

	printf ("timestamp:%s\n",timebuffer);
#endif
	
	*amount = (logDecrypted[20]<<24) | (logDecrypted[21]<<16) | (logDecrypted[22]<<8) | (logDecrypted[23]);
	
	*senderACCN = 0;
	for(z=0; z<6; z++)
	{
		if(z)(*senderACCN) <<= 8;
		else (*senderACCN) = 0;
		(*senderACCN) |= logDecrypted[8+z];
#ifdef DEBUG_MODE
		printf("ACCN[%d]:%llx\n", z, (*senderACCN));
#endif
	}
	(*senderACCN) &= 0xFFFFFFFFFFFF;
	
	*lognum = (logDecrypted[0]<<16) | (logDecrypted[1]<<8) | (logDecrypted[2]);
}
