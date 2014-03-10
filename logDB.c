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
	else fprintf(stdout, "Opened database successfully\n");

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
	else fprintf(stdout, "Opened database successfully\n");

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
	else fprintf(stderr, "Opened database successfully\n");

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
	else fprintf(stdout, "Opened database successfully\n");

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
