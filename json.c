#include "header.h"

const char* get_key_inString_from_json_response(json_object* jobj)
{
	json_object* jobj_parse;
	
	jobj_parse = json_object_object_get(jobj, "key");
	
	return json_object_get_string(jobj_parse);
}

json_object* create_registration_json(const gchar* ACCN, char* HWID)
{
	/*Creating a json object*/
	json_object * jobj = json_object_new_object();

	/*Creating a json string*/
	json_object *jstring = json_object_new_string(ACCN);

	/*Creating a json string*/
	json_object *jstring1 = json_object_new_string(HWID);

	/*Form the json object*/
	/*Each of these is like a key value pair*/
	json_object_object_add(jobj,"ACCN", jstring);
	json_object_object_add(jobj,"HWID", jstring1);
	
	return jobj;
}
