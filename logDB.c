#include "header.h"
#include <sqlite3.h>

gboolean openDB(const char* DBname)
{
	sqlite3 *db;
	int rc;

	rc = sqlite3_open(DBname, &db);

	if( rc )
	{
#ifdef DEBUG_MODE		
		fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
#endif		
		return 1;
	}
	else
	{
#ifdef DEBUG_MODE
		fprintf(stdout, "Opened database successfully\n");
#endif
		return 0;
	}
	sqlite3_close(db);
}
