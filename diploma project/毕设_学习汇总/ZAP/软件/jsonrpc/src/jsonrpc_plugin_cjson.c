/*
 * Copyright (c) 2012 Jonghyeok Lee <jhlee4bb@gmail.com>
 *
 * jsonrpC is free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#include "jsonrpc_plugin_cjson.h"

#include "cJSON.h"
#include <float.h>


static jsonrpc_handle_t	jsonrpc_cjson_parse  (const char *json)
{
	cJSON * root = cJSON_Parse(json);
	
	return (jsonrpc_handle_t)root;
}

static void				jsonrpc_cjson_release(jsonrpc_handle_t json)
{
	cJSON_Delete((cJSON *)json);
}

static jsonrpc_handle_t	jsonrpc_cjson_get    (jsonrpc_handle_t json, const char *key)
{
	return (jsonrpc_handle_t)cJSON_GetObjectItem((cJSON *)json, key);
}

static jsonrpc_handle_t	jsonrpc_cjson_get_at (jsonrpc_handle_t json, size_t index)
{
	return (jsonrpc_handle_t)cJSON_GetArrayItem((cJSON *)json, index);
}

static const char *		jsonrpc_cjson_get_key_at(jsonrpc_handle_t json, size_t index)
{
	cJSON * obj = cJSON_GetArrayItem((cJSON *)json, index);
	return (const char *)obj->string;
}

static jsonrpc_bool_t		jsonrpc_cjson_valueof (jsonrpc_handle_t json, jsonrpc_json_t *value)
{
	cJSON * v = (cJSON *)json;

	switch (v->type)
	{
	case cJSON_String:
		value->type = JSONRPC_TYPE_STRING;
		if (v->valuestring == NULL)
			return JSONRPC_FALSE;
		value->u.string = v->valuestring;
		break;

	case cJSON_Number:
		value->type = JSONRPC_TYPE_NUMBER;
		value->u.number = v->valuedouble;
		break;

	case cJSON_Object:
		value->type = JSONRPC_TYPE_OBJECT;
		value->u.object = (jsonrpc_handle_t)json;
		break;

	case cJSON_Array:
		value->type = JSONRPC_TYPE_ARRAY;
		value->u.array = (jsonrpc_handle_t)json;
		break;

	case cJSON_True:
		value->type = JSONRPC_TYPE_BOOLEAN;
		value->u.boolean = JSONRPC_TRUE;
		break;

	case cJSON_False:
		value->type = JSONRPC_TYPE_BOOLEAN;
		value->u.boolean = JSONRPC_FALSE;
		break;

	case cJSON_NULL:
		value->type = JSONRPC_TYPE_NULL;
		break;

    default:
		return JSONRPC_FALSE;
	}
	return JSONRPC_TRUE;
}

static size_t				jsonrpc_cjson_length(jsonrpc_handle_t json)
{
	return cJSON_GetArraySize((cJSON *)json);
}


const jsonrpc_json_plugin_t	* jsonrpc_plugin_cjson (void)
{
	static const jsonrpc_json_plugin_t plugin_cjson = {
		jsonrpc_cjson_parse,
		jsonrpc_cjson_release,
		jsonrpc_cjson_get,
		jsonrpc_cjson_get_at,
		jsonrpc_cjson_get_key_at,
		jsonrpc_cjson_valueof,
		jsonrpc_cjson_length
	};
	return &plugin_cjson;
}


