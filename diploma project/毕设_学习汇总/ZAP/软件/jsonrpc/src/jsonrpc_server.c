/*
 * Copyright (c) 2012 Jonghyeok Lee <jhlee4bb@gmail.com>
 *
 * jsonrpC is free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */





#include "Platform.h"
#include "Jsonrpc.h"

#include "jsonrpc_macro.h"
#include "jsonrpc_memory.h"
#include "jsonrpc_mstream.h"

#include "jsonrpc_plugin_cjson.h"
#include "jsonrpc_plugin_tcp.h"
#include "Logger.h"


DEFINE_LOGGER(jsonrpc,200,LOG_DEBUG);		//**模块日志属性设置宏：日志模块名称、模块长度、日志等级**//


#define	JSONRPC_JSONAPI(server)	(&(server)->json)
#define	JSONRPC_MEMSTREAM_NUM	3
#define	JSONRPC_TEMPVALUE_NUM	10

typedef struct
{
	char				name[JSONRPC_NAME_LEN];
	jsonrpc_method_t	method;
	jsonrpc_bool_t		has_return;
	size_t				argc;
	jsonrpc_param_t		*argv;
} jsonrpc_procedure_t;


struct jsonrpc_server
{
	jsonrpc_json_plugin_t	json;
	jsonrpc_net_plugin_t	net;

	jsonrpc_handle_t		net_handle;

	struct {
		jsonrpc_mstream_t	*mstream[JSONRPC_MEMSTREAM_NUM];
		jsonrpc_bool_t		used[JSONRPC_MEMSTREAM_NUM];
		size_t				index;
	} stream;

    struct {
        jsonrpc_param_t     *argv;
        size_t              argc;
    } param;

	struct {
		void				*buf;
		size_t				size;
	} tempbuf;

	struct {
		jsonrpc_json_t		value[JSONRPC_TEMPVALUE_NUM];
		size_t				index;
	} tempval;

	struct {
		jsonrpc_procedure_t	**list;
		size_t				count;
		size_t				list_length;
		jsonrpc_bool_t		sorted;
	} proc;
};


JSONRPC_PRIVATE int	check_null_func (void *plugin, size_t n)
{
	void **funcs;

	if (plugin == NULL || n == 0)
		return -1;

	for (n /= sizeof(void *), funcs = (void **)plugin ; n && *funcs ; n--, funcs++)
		;
	return (int)n;
}

JSONRPC_PRIVATE jsonrpc_mstream_t * get_memstream (jsonrpc_server_t *self, jsonrpc_bool_t auto_release)
{
	size_t	i = self->stream.index;
	int		n = JSONRPC_MEMSTREAM_NUM;
	do
	{
		i = (i + 1) % JSONRPC_MEMSTREAM_NUM;
		if (self->stream.mstream[i] == NULL)
		{
			self->stream.mstream[i] = jsonrpc_mstream_open();
		}
	} while (n-- && (self->stream.used[i] || self->stream.mstream[i] == NULL));
	if (n < 0)
		return NULL;
	self->stream.index = i;
	if (!auto_release)
		self->stream.used[i] = JSONRPC_TRUE;
	jsonrpc_mstream_rewind(self->stream.mstream[i]);
	return self->stream.mstream[i];
}

JSONRPC_PRIVATE void	release_memstream (jsonrpc_server_t *self, jsonrpc_mstream_t *stream)
{
	int	n = JSONRPC_MEMSTREAM_NUM;
	while (n--)
	{
		if (self->stream.mstream[n] == stream)
		{
			self->stream.used[n] = JSONRPC_FALSE;
			break;
		}
	}
}

JSONRPC_PRIVATE jsonrpc_param_t * get_temp_param (jsonrpc_server_t *self, size_t size)
{
	if (self->param.argc < size)
	{
		jsonrpc_param_t	*argv = (jsonrpc_param_t *)jsonrpc_malloc(size * sizeof(jsonrpc_param_t));
		if (argv == NULL)
			return NULL;
		if (self->param.argv)
			jsonrpc_free(self->param.argv);
		self->param.argv = argv;
		self->param.argc = size;
	}
	memset(self->param.argv, 0, sizeof(jsonrpc_param_t) * size);
	return self->param.argv;
}

JSONRPC_PRIVATE void *	get_temp_buf (jsonrpc_server_t *self, size_t size)
{
	if (self->tempbuf.size < size)
	{
		void *buf = jsonrpc_realloc(self->tempbuf.buf, size);
		JSONRPC_THROW(buf == NULL, return NULL);
		self->tempbuf.buf  = buf;
		self->tempbuf.size = size;
	}
	return self->tempbuf.buf;
}

JSONRPC_PRIVATE jsonrpc_json_t * get_temp_value (jsonrpc_server_t *self)
{
	size_t	i = self->tempval.index;

	self->tempval.index = (self->tempval.index + 1) % JSONRPC_TEMPVALUE_NUM;
	memset(self->tempval.value + i, 0, sizeof(jsonrpc_json_t));

	return self->tempval.value + i;
}


JSONRPC_PRIVATE jsonrpc_bool_t	str_startwith (const char *str, char c)
{
	while (isspace(*str))
		str++;
	return (*str == c) ? JSONRPC_TRUE : JSONRPC_FALSE;
}

JSONRPC_PRIVATE const char *make_batch_object (jsonrpc_server_t *self, const char *request)
{
	jsonrpc_mstream_t	*stream;

	stream = get_memstream(self, JSONRPC_TRUE);
	if (!stream)
		return NULL;
	jsonrpc_mstream_print(stream, "{\"batch\":%s}", request);
	return jsonrpc_mstream_getbuf(stream);
}

JSONRPC_PRIVATE char * strdup_without_space (jsonrpc_server_t *self, const char *str)
{
	char *dup, *d;

	dup = (char *)get_temp_buf(self, strlen(str) + 1);
	if (dup)
	{
		for (d = dup ; *str != '\0' ; str++)
		{
			if (!isspace(*str))
				*d++ = *str;
		}
		*d = '\0';
	}
	return dup;
}


JSONRPC_PRIVATE int parse_param_signature (jsonrpc_server_t *self, const char *param_signature, int paramc, jsonrpc_param_t *paramv)
{
	int		count = 0;
	char	*src;
	char	*token, *save;
	jsonrpc_bool_t	typeonly;

	if (param_signature == NULL)
		return 0;	// void

	src = strdup_without_space(self, param_signature);
	JSONRPC_THROW(src == NULL, return -1);

	if (strchr(src, ':'))
		typeonly = JSONRPC_FALSE;
	else typeonly = JSONRPC_TRUE;

	token = strtok_r(src, ",", &save);
	while (token)
	{
		if (typeonly)
		{
			for (; *token != '\0' ; token++)
			{
				JSONRPC_THROW(strchr(JSONRPC_TYPES, *token) == NULL, return -1);
                JSONRPC_THROW(count == paramc, return paramc + 1);/*返回大于paramc表示缓存不够*/

                paramv[count].name[0] = '\0';
                paramv[count].json.type = (jsonrpc_type_t)*token;
                count++;
			}
		}
		else
		{
			char *temp;
			char *name = strtok_r(token, ":", &temp);
			char *type = strtok_r(NULL , ":", &temp);

			JSONRPC_THROW(name == NULL || type == NULL || strchr(JSONRPC_TYPES, *type) == NULL || type[1] != '\0'
				, return -1
			);
            JSONRPC_THROW(count == paramc, return paramc + 1);/*返回大于paramc表示缓存不够*/

            JSONRPC_STRNCPY(paramv[count].name, name, JSONRPC_NAME_LEN);
            paramv[count].json.type = (jsonrpc_type_t)*type;
			count++;
		}
		token = strtok_r(NULL, ",", &save);
	}
	return count;
}

JSONRPC_PRIVATE int compare_procedure_name (const void *arg1, const void *arg2)
{
	const jsonrpc_procedure_t *lhs = *(const jsonrpc_procedure_t **)arg1;
	const jsonrpc_procedure_t *rhs = *(const jsonrpc_procedure_t **)arg2;

	return strcmp(lhs->name, rhs->name);
}

JSONRPC_PRIVATE jsonrpc_procedure_t ** find_procedure (jsonrpc_server_t *self, const char *name, size_t *overload)
{
	void 	*ret;
	size_t	i, s, e;
	jsonrpc_procedure_t	key, *pk;

	if (self->proc.count == 0)
		return NULL;

	if (!self->proc.sorted)
	{
		if (self->proc.count > 1)
			qsort(self->proc.list, self->proc.count, sizeof(jsonrpc_procedure_t *), compare_procedure_name);
		self->proc.sorted = JSONRPC_TRUE;
	}

	JSONRPC_STRNCPY(key.name, name, JSONRPC_NAME_LEN);
	pk  = &key;
	ret = bsearch(&pk, self->proc.list, self->proc.count, sizeof(jsonrpc_procedure_t *), compare_procedure_name);
	if (ret == NULL)
		return NULL;

	i   = ((size_t)ret - (size_t)self->proc.list) / sizeof(jsonrpc_procedure_t *);
	for (s = i ; s > 0 && strcmp(self->proc.list[s - 1]->name, name) == 0 ; s--)
		; // find first position of overloaded method
	for (e = i ; e < self->proc.count - 1 && strcmp(self->proc.list[e + 1]->name, name) == 0 ; e++)
		; // find last position of overloaded method

	if (overload)
		*overload = e - s + 1;
	return self->proc.list + s;
}


JSONRPC_PRIVATE jsonrpc_bool_t add_procedure (jsonrpc_server_t *self, jsonrpc_procedure_t *proc)
{
	if (self->proc.count == self->proc.list_length)
	{
		size_t	size;
		if (self->proc.list_length == 0)
			size = 32;	// default
		else
			size = self->proc.list_length * 2;

		JSONRPC_THROW(
			(self->proc.list = (jsonrpc_procedure_t **)jsonrpc_realloc(
							self->proc.list, sizeof(jsonrpc_procedure_t *) * size)) == NULL
			, return JSONRPC_FALSE
		);
		self->proc.list_length = size;
	}

	self->proc.list[self->proc.count++] = proc;
	self->proc.sorted = JSONRPC_FALSE;
	return JSONRPC_TRUE;
}


JSONRPC_PRIVATE const char * get_error_message (jsonrpc_error_t error)
{
/*
	-32700	Parse error	Invalid JSON was received by the server.
	An error occurred on the server while parsing the JSON text.
	-32600	Invalid Request	The JSON sent is not a valid Request object.
	-32601	Method not found	The method does not exist / is not available.
	-32602	Invalid params	Invalid method parameter(s).
	-32603	Internal error	Internal JSON-RPC error.
	-32000 to -32099	Server error	Reserv
*/
	switch (error)
	{
	case JSONRPC_ERROR_PARSE_ERROR:          return "Parse error";
	case JSONRPC_ERROR_INVALID_REQUEST:      return "Invalid Request";
	case JSONRPC_ERROR_METHOD_NOT_FOUND:     return "Method not found";
	case JSONRPC_ERROR_INVALID_PARAMS:       return "Invalid params";
	case JSONRPC_ERROR_INTERNAL:             return "Internal error";
	case JSONRPC_ERROR_SERVER_OUT_OF_MEMORY: return "Server: Out of memory";
	case JSONRPC_ERROR_SERVER_INTERNAL:      return "Server: Internal error";
	default:
		break;
	}
	return "Unknown error";
}


JSONRPC_PRIVATE const char * get_error_object (jsonrpc_server_t *self, jsonrpc_error_t error, const jsonrpc_json_t *id)
{
	jsonrpc_mstream_t	*stream;

	if (id == NULL
		&& error != JSONRPC_ERROR_PARSE_ERROR
		&& error != JSONRPC_ERROR_INVALID_REQUEST)
	{
		return NULL;	// The server MUST NOT reply except "Parse error/Invalid Request".
	}

	JSONRPC_THROW((stream = get_memstream(self, JSONRPC_TRUE)) == NULL, return NULL);

	jsonrpc_mstream_print(stream, "{");
	{
		jsonrpc_mstream_print(stream, "\"jsonrpc\":\"%s\"", JSONRPC_VERSION);
		jsonrpc_mstream_print(stream, ",\"error\":{");
		{
			jsonrpc_mstream_print(stream, "\"code\":%d,\"message\":\"%s\"", (int)error, get_error_message(error));
		}
		jsonrpc_mstream_print(stream, "}");
		if (id)
		{
			if (id->type == JSONRPC_TYPE_NUMBER)
				jsonrpc_mstream_print(stream, ",\"id\":%.0lf", id->u.number);
			else if (id->type == JSONRPC_TYPE_STRING)
				jsonrpc_mstream_print(stream, ",\"id\":\"%s\"", id->u.string);
			else
				jsonrpc_mstream_print(stream, ",\"id\":null");
		}
		else jsonrpc_mstream_print(stream, ",\"id\":null");
	}
	jsonrpc_mstream_print(stream, "}");

	return jsonrpc_mstream_getbuf(stream);
}


JSONRPC_PRIVATE jsonrpc_bool_t is_valid_json_value (const jsonrpc_json_t *value)
{
	switch(value->type)
	{
	case JSONRPC_TYPE_BOOLEAN:
		if (value->u.boolean == JSONRPC_TRUE || value->u.boolean == JSONRPC_FALSE)
			return JSONRPC_TRUE;
		break;

	case JSONRPC_TYPE_STRING:
	case JSONRPC_TYPE_ARRAY:
	case JSONRPC_TYPE_OBJECT:
		if (value->u.object)	// NULL check
			return JSONRPC_TRUE;
		break;

	default:
		return JSONRPC_TRUE;
	}
	return JSONRPC_FALSE;
}

JSONRPC_PRIVATE jsonrpc_json_t * get_json_value (
								  jsonrpc_server_t *self
								, jsonrpc_handle_t handle
								, const char *key
								, const char *signature
							)
{
	jsonrpc_handle_t		val;
	jsonrpc_json_t	*ret;

	val = JSONRPC_JSONAPI(self)->get(handle, key);
	if (!val)
		return NULL;

	ret = get_temp_value(self);
	if (!JSONRPC_JSONAPI(self)->valueof(val, ret))
		return NULL;

	if (signature && strchr(signature, (int)ret->type) == NULL)
		return NULL;

	if (!is_valid_json_value(ret))
		return NULL;

	return ret;
}

JSONRPC_PRIVATE jsonrpc_error_t parse_request (
								  jsonrpc_server_t *self
								, jsonrpc_handle_t req
								, const jsonrpc_json_t **version
								, const jsonrpc_json_t **method
								, const jsonrpc_json_t **params
								, const jsonrpc_json_t **id
							)
{
	
	LogInfo("Enter the parse_request\n");
	const jsonrpc_json_t	*val;

	JSONRPC_THROW((val = get_json_value(self, req, "jsonrpc", "s")) == NULL
		, return JSONRPC_ERROR_INVALID_REQUEST
	);
	*version = val;
	LogInfo("Parse the version OK\n");
	JSONRPC_THROW((val = get_json_value(self, req, "method", "s")) == NULL
		, return JSONRPC_ERROR_INVALID_REQUEST
	);
	*method = val;
	LogInfo("Parse the method OK\n");
	val = get_json_value(self, req, "params", NULL);
	JSONRPC_THROW(val && val->type != JSONRPC_TYPE_OBJECT && val->type != JSONRPC_TYPE_ARRAY
		, return JSONRPC_ERROR_INVALID_REQUEST
	);
	LogInfo("Parse the params OK params, type=%c\n", val->type);
	*params = val;
	val = get_json_value(self, req, "id", NULL);
	JSONRPC_THROW(val && val->type != JSONRPC_TYPE_NUMBER && val->type != JSONRPC_TYPE_STRING
		, return JSONRPC_ERROR_INVALID_REQUEST
	);
	*id = val;
	LogInfo("Parse the id OK\n");
	return JSONRPC_ERROR_OK;
}

#include "cJSON.h"
JSONRPC_PRIVATE jsonrpc_error_t parse_params (
								  jsonrpc_server_t *self
								, const jsonrpc_json_t *params
								, size_t *paramc
								, jsonrpc_param_t **paramv
							)
{
	size_t	i, n;
	jsonrpc_param_t		*pv;
	jsonrpc_handle_t	handle;
	const char 			*key;
	jsonrpc_json_t	*value;

	if (params)
		n = JSONRPC_JSONAPI(self)->length(params->u.object);
	else
		n = 0;
	if (n == 0)
	{
		*paramc = 0;
		*paramv = NULL;
		printf("no params\n");
		return JSONRPC_ERROR_OK;
	}

	pv = get_temp_param(self, n);
	JSONRPC_THROW(pv == NULL, return JSONRPC_ERROR_SERVER_OUT_OF_MEMORY);
	
	//LogDebug("--soso--%s n = %d\n",__FUNCTION__,n);
	for (i = 0 ; i < n ; i++)
	{
		handle = JSONRPC_JSONAPI(self)->get_at(params->u.object, i);
		//LogDebug("para %d  str = %s\n", i, cJSON_Print((cJSON *)handle));

		
		JSONRPC_THROW(!handle, return JSONRPC_ERROR_SERVER_INTERNAL);
		//LogDebug("ERR1\n");
		value  = get_temp_value(self);
		if (!JSONRPC_JSONAPI(self)->valueof(handle, value))
			value = NULL;
		JSONRPC_THROW(value == NULL || !is_valid_json_value(value), return JSONRPC_ERROR_SERVER_INTERNAL);
		//LogDebug("ERR2\n");
		if (params->type == JSONRPC_TYPE_OBJECT)
		{
			JSONRPC_THROW((key = JSONRPC_JSONAPI(self)->get_key_at(params->u.object, i)) == NULL, return JSONRPC_ERROR_SERVER_INTERNAL);
			//LogDebug("ERR3\n");
			JSONRPC_STRNCPY(pv[i].name, key, JSONRPC_NAME_LEN);
		}
		memcpy(&(pv[i].json), value, sizeof(jsonrpc_json_t));
		
		//LogDebug("--soso--%s pv[i].name = %s\n",__FUNCTION__,pv[i].name);	
	}
	*paramc = n;
	*paramv = pv;
	return JSONRPC_ERROR_OK;
}

JSONRPC_PRIVATE int compare_param_index (const void *arg1, const void *arg2)
{
	const jsonrpc_param_t *lhs = (const jsonrpc_param_t *)arg1;
	const jsonrpc_param_t *rhs = (const jsonrpc_param_t *)arg2;

	return (int)lhs->index - (int)rhs->index;
}

JSONRPC_PRIVATE jsonrpc_bool_t match_params_and_sort (
									  jsonrpc_server_t *self
									, jsonrpc_procedure_t *proc
									, size_t paramc
									, jsonrpc_param_t *paramv
								)
{
	jsonrpc_bool_t	do_sort;
	size_t			i, j;
	size_t			skip_flag;	// for optimize

	if (proc->argc != paramc)
		return JSONRPC_FALSE;

	if (paramv[0].name[0] == '\0') // check matched (without name)
	{
		for (i = 0 ; i < proc->argc ; i++)
		{
			if (proc->argv[i].json.type != paramv[i].json.type)
				break;
		}
		return i == proc->argc ? JSONRPC_TRUE : JSONRPC_FALSE;
	}

	do_sort = JSONRPC_FALSE;
	for (i = 0, skip_flag = 0 ; i < proc->argc ; i++)
	{
		for (j = 0 ; j < paramc ; j++)
		{
			if (j < 32 && (skip_flag & (1 << j)))
				continue;
			if (proc->argv[i].json.type != paramv[j].json.type)
				continue;
			if (strcmp(proc->argv[i].name, paramv[j].name) != 0)
				continue;

			if (j != i)
				do_sort = JSONRPC_TRUE;
			paramv[j].index = i;
			if (j < 32)
				skip_flag |= (1 << j);
			break;
		}
		if (j == paramc)
			return JSONRPC_FALSE;
	}
	if (do_sort && paramc > 1)
		qsort(paramv, paramc, sizeof(jsonrpc_param_t), compare_param_index);
	return JSONRPC_TRUE;
}

JSONRPC_PRIVATE const char * execute_request (jsonrpc_server_t *self, jsonrpc_handle_t request)
{
	const jsonrpc_json_t	*version;
	const jsonrpc_json_t	*method;
	const jsonrpc_json_t	*params;
	const jsonrpc_json_t	*id;
	jsonrpc_error_t			err;
	jsonrpc_procedure_t		**procs, *proc;
	size_t					i, n;
	size_t					paramc;
	jsonrpc_param_t			*paramv;
	jsonrpc_mstream_t		*result;
	jsonrpc_mstream_t		*response;

	//--> {"jsonrpc": "2.0", "method": "subtract", "params": {"subtrahend": 23, "minuend": 42}, "id": 3}
	
	LogInfo("Enter the execute_request\n");
	JSONRPC_THROW(parse_request(self, request, &version, &method, &params, &id) != JSONRPC_ERROR_OK
		, return get_error_object(self, JSONRPC_ERROR_INVALID_REQUEST, NULL)
	);
	//LogInfo("parse_request OK \n");
	JSONRPC_THROW(strcmp(version->u.string, JSONRPC_VERSION) != 0
		, return get_error_object(self, JSONRPC_ERROR_INVALID_REQUEST, NULL)
	);

	procs = find_procedure(self, method->u.string, &n);
	JSONRPC_THROW(procs == NULL
		, return get_error_object(self, JSONRPC_ERROR_METHOD_NOT_FOUND, id)
	);
	//LogInfo("find_procedure ok params=%d\n",(int)params);
	err = parse_params(self, params, &paramc, &paramv);
	JSONRPC_THROW(err != JSONRPC_ERROR_OK, return get_error_object(self, err, id));
	//LogInfo("parse_params ok\n");

	if (paramc > 0)
	{
		for (i = 0 ; i < n ; i++)
		{
			if (match_params_and_sort(self, procs[i], paramc, paramv))
				break;
		}
		if (i == n)
			return get_error_object(self, JSONRPC_ERROR_METHOD_NOT_FOUND, id); //LogError("get_error_object\n");

		proc = procs[i];
	} else proc = procs[0];

	JSONRPC_THROW((id && !proc->has_return) || (!id && proc->has_return)
		, return get_error_object(self, JSONRPC_ERROR_METHOD_NOT_FOUND, id)
	);

	//LogInfo("id match OK\n");
	if (id == NULL) // Notification
	{
		//LogInfo("id=NULL\n");
		(void)proc->method((int)paramc, paramv, NULL, NULL);
		return NULL;
	}

	JSONRPC_THROW((result = get_memstream(self, JSONRPC_TRUE)) == NULL
		, return get_error_object(self, JSONRPC_ERROR_SERVER_OUT_OF_MEMORY, id)
	);
	
	err = proc->method((int)paramc, paramv
			, (void (*)(void *ctx, const char *,...))jsonrpc_mstream_print
			, (void *)result
		);
	JSONRPC_THROW(err != JSONRPC_ERROR_OK, return get_error_object(self, err, id));
	//LogInfo("Get the result ok\n");
	JSONRPC_THROW((response = get_memstream(self, JSONRPC_TRUE)) == NULL
		, return get_error_object(self, JSONRPC_ERROR_SERVER_OUT_OF_MEMORY, id)
	);

	jsonrpc_mstream_print(response, "{");
	{
		//LogInfo("Enter the jsonrpc_mstream_print\n");
		jsonrpc_mstream_print(response, "\"jsonrpc\":\"%s\"", JSONRPC_VERSION);
		jsonrpc_mstream_print(response, ",\"result\":%s", jsonrpc_mstream_getbuf(result));
		if (id->type == JSONRPC_TYPE_NUMBER)
			jsonrpc_mstream_print(response, ",\"id\":%.0lf", id->u.number);
		else if (id->type == JSONRPC_TYPE_STRING)
			jsonrpc_mstream_print(response, ",\"id\":\"%s\"", id->u.string);
		else
			//LogInfo("id return null\n");
			jsonrpc_mstream_print(response, ",\"id\":null");
	}
	jsonrpc_mstream_print(response, "}");

	return jsonrpc_mstream_getbuf(response);
}

JSONRPC_PRIVATE const char * execute (jsonrpc_server_t *self, const char *data)
{
	jsonrpc_handle_t	request;
	jsonrpc_handle_t	value;
	jsonrpc_error_t		error;
	jsonrpc_mstream_t	*resbuf;
	const char *		response;
	jsonrpc_json_t	*json_value;
	
	LogInfo("Enter the execute\n");
	error = JSONRPC_ERROR_OK;
	JSONRPC_THROW(!(request = JSONRPC_JSONAPI(self)->parse(data)), {
		LogDebug("parse request data  error\n");
		error = JSONRPC_ERROR_PARSE_ERROR;
		goto RESPONSE;
	});
	json_value = get_temp_value(self);
	JSONRPC_THROW(!JSONRPC_JSONAPI(self)->valueof(request, json_value), {
		LogDebug("get the request data error\n");
		error = JSONRPC_ERROR_SERVER_INTERNAL;
		goto RESPONSE;
	});
	JSONRPC_THROW(json_value->type != JSONRPC_TYPE_OBJECT, {
		LogDebug("the project of request is out of types\n");
		error = JSONRPC_ERROR_INVALID_REQUEST;
		goto RESPONSE;
	});

	if ((json_value = get_json_value(self, request, "batch", "a")) != NULL)	// batch
	{
		LogDebug("when the request is the type of batch:\n");
		size_t	i, c, n;
		jsonrpc_handle_t batch = json_value->u.array;

		JSONRPC_THROW((n = JSONRPC_JSONAPI(self)->length(batch)) == 0, {
			LogDebug("the length of the batch request is 0 \n");
			error = JSONRPC_ERROR_INVALID_REQUEST;
			goto RESPONSE;
		});
		JSONRPC_THROW((resbuf = get_memstream(self, JSONRPC_FALSE)) == NULL, {
			error = JSONRPC_ERROR_SERVER_INTERNAL;
			goto RESPONSE;
		});

		jsonrpc_mstream_print(resbuf, "[");
		for (i = 0, c = 0 ; i < n ; i++)
		{
			value = JSONRPC_JSONAPI(self)->get_at(batch, i);
			if (!value)
				response = get_error_object(self, JSONRPC_ERROR_INVALID_REQUEST, NULL);
			else
			{
				response = execute_request(self, value);
				if(response == NULL)
				{
					printf("execute_request failed\n");
				}
			}

			if (response)
			{
				if (c > 0)
					jsonrpc_mstream_print(resbuf, ",");
				jsonrpc_mstream_print(resbuf, response);
				c++;
			}
		}
		jsonrpc_mstream_print(resbuf, "]");
		if (c == 0)
			response = NULL;
		else response = jsonrpc_mstream_getbuf(resbuf);

		release_memstream(self, resbuf);
	}
	else //if (json_value->type == JSONRPC_TYPE_OBJECT)
	{
		response = execute_request(self, request);
	}

RESPONSE:
	if (request)
		JSONRPC_JSONAPI(self)->release(request);
	if (error != JSONRPC_ERROR_OK)
		return get_error_object(self, error, NULL);
	return response;
}



jsonrpc_server_t *
jsonrpc_server_open (int isClient,
	//const jsonrpc_json_plugin_t *ijson, const jsonrpc_net_plugin_t *inet, 
	...)
{
	
	LogInfo("Enter the jsonrpc_server_open:\n");
	jsonrpc_server_t	*self;

    const jsonrpc_json_plugin_t *ijson = jsonrpc_plugin_cjson();
	const jsonrpc_net_plugin_t *inet = jsonrpc_plugin_tcp();
	JSONRPC_THROW(
		check_null_func((void *)ijson, sizeof(jsonrpc_json_plugin_t)) != 0
		&& (inet == NULL || (inet && check_null_func((void *)inet, sizeof(jsonrpc_net_plugin_t)) != 0))
		, return NULL
	);

	self = (jsonrpc_server_t *)jsonrpc_calloc(1, sizeof(jsonrpc_server_t));
	JSONRPC_THROW(self == NULL, return NULL);
	JSONRPC_THROW(get_temp_param(self, 16/* default argc */) == NULL, goto ERROR);

	memcpy(&self->json, ijson, sizeof(jsonrpc_json_plugin_t));
	if (inet)
	{
		va_list	ap;
		
		memcpy(&self->net, inet, sizeof(jsonrpc_net_plugin_t));
		
		va_start(ap, isClient);
		self->net_handle = self->net.TcpOpen(isClient, ap);
		va_end(ap);
		JSONRPC_THROW(!self->net_handle, goto ERROR);
	}
	return self;
ERROR:
	LogError("The jsonrpc_server_open Error\n");
	jsonrpc_server_close(self);
	return NULL;
}

void
jsonrpc_server_close (jsonrpc_server_t *self)
{
	size_t	i;

	if(self == NULL)
		return ;


	if (self->net_handle)
		self->net.TcpClose(self->net_handle);
	
	
	for (i = 0 ; i < JSONRPC_MEMSTREAM_NUM ; i++)
	{
		if (self->stream.mstream[i])
			jsonrpc_mstream_close(self->stream.mstream[i]);
	}

	
	if (self->param.argv)
		jsonrpc_free(self->param.argv);

	if (self->tempbuf.buf)
		jsonrpc_free(self->tempbuf.buf);


	if (self->proc.list)
	{
		for (i = 0 ; i < self->proc.count ; i++)
		{
			if (self->proc.list[i])
			{
				if (self->proc.list[i]->argv)
					jsonrpc_free(self->proc.list[i]->argv);
				jsonrpc_free(self->proc.list[i]);
			}
		}
		jsonrpc_free(self->proc.list);
	}
	jsonrpc_free(self);
}

jsonrpc_error_t
jsonrpc_server_register_method (
							jsonrpc_server_t *self
							, jsonrpc_bool_t has_return
							, jsonrpc_method_t method
							, const char *method_name
							, const char *param_signature
						)
{
	jsonrpc_procedure_t	*proc;
	int		ret;
	size_t	size;

	JSONRPC_THROW((proc = (jsonrpc_procedure_t *)jsonrpc_malloc(sizeof(jsonrpc_procedure_t))) == NULL,
		return JSONRPC_ERROR_SERVER_OUT_OF_MEMORY
	);

	memset(proc, 0, sizeof(jsonrpc_procedure_t));
	JSONRPC_STRNCPY(proc->name, method_name, JSONRPC_NAME_LEN);
	proc->method = method;
	proc->has_return = has_return;

    for (size = self->param.argc ; get_temp_param(self, size) ; size *= 2)
    {
        ret = parse_param_signature(self, param_signature, (int)self->param.argc, self->param.argv);
        JSONRPC_THROW(ret < 0, return JSONRPC_ERROR_INVALID_PARAMS);

        if (ret <= (int)self->param.argc)
        {
			if (ret > 0)
			{
				proc->argv = (jsonrpc_param_t *)jsonrpc_memdup(self->param.argv, sizeof(jsonrpc_param_t) * (size_t)ret);
				JSONRPC_THROW(proc->argv == NULL, break);
				proc->argc = (size_t)ret;
			}
			JSONRPC_THROW(!add_procedure(self, proc), break);

			return JSONRPC_ERROR_OK;
		}
    }
	if (proc->argv)
		jsonrpc_free(proc->argv);
	jsonrpc_free(proc);
	return JSONRPC_ERROR_SERVER_OUT_OF_MEMORY;
}


const char *
jsonrpc_server_execute (jsonrpc_server_t *self, const char *request)
{
	
	LogInfo("jsonrpc_server_execute\n");
	if (str_startwith(request, '[')) // batch
	{
		request = make_batch_object(self, request);
		if (request == NULL)
			LogDebug("the request is not the batch but single\n");
			return NULL;
	}
	return execute(self, request);
}

jsonrpc_error_t
jsonrpc_server_run (jsonrpc_server_t *self, unsigned int timeout)
{

	jsonrpc_error_t	error;
	const char *req;
	const char *res;
	void *desc;
	
	
	//**所有的操作，都是直接对全局结构体变量进行操作，而不需要返回特定的分步操作结果**//
	JSONRPC_THROW(self->net.TcpOpen == NULL, return JSONRPC_ERROR_INVALID_REQUEST);

	error = self->net.TcpTick(self->net_handle);		//**error接收数据，并将接受到的数据进行解析，存储在rcvmsg；同时负责下一次循环回来时，对上一次包装好的数据的发送**//
	if (error != JSONRPC_ERROR_OK)					//**error = recv、parse、send(next turn)**//
		//LogDebug("self->error error\n");
		return error;

	req = self->net.TcpRecv(self->net_handle, timeout, &desc);		//**修改解析状态位，导出具体json请求**//
	if (req == NULL)
		//LogDebug("self->recv return NULL\n");
		return JSONRPC_ERROR_SERVER_TIMEOUT;

	LogDebug("req : %s\n", req);
	res = jsonrpc_server_execute(self, req);			//**jsonrpc**//
	JSONRPC_THROW(res == NULL, return JSONRPC_ERROR_SERVER_INTERNAL);

	LogDebug("resp : %s\n",res);
	return self->net.TcpSend(self->net_handle, res, desc);			//**对调用结果进行包装，等待被发送给客户端**//
}


/***********************************************************************/
static char * escaped_string(const char *str);



jsonrpc_error_t	jsonrpc_notify_client (
				jsonrpc_server_t *self,
				const char *method_name,
				const char *param_signature
				, ...)
{
	size_t  size;
	int		ret;
	int     i;
	jsonrpc_mstream_t *stream;
	double valuenumber;
	jsonrpc_bool_t valueboolean;
	char * valuestring;
	char * escapedstring;


	for (size = self->param.argc ; get_temp_param(self, size) ; size *= 2)
	{
		ret = parse_param_signature(self, param_signature, (int)self->param.argc, self->param.argv);
		JSONRPC_THROW(ret < 0, return JSONRPC_ERROR_INVALID_PARAMS);

		if (ret <= (int)self->param.argc)/*缓冲区够用*/
		{
			JSONRPC_THROW((stream = get_memstream(self, JSONRPC_TRUE)) == NULL
				, return JSONRPC_ERROR_SERVER_INTERNAL);
			jsonrpc_mstream_print(stream, "{");
			{
				jsonrpc_mstream_print(stream, "\"jsonrpc\":\"%s\"", JSONRPC_VERSION);
				escapedstring = escaped_string(method_name);
				jsonrpc_mstream_print(stream, ",\"method\":%s", escapedstring);
				free(escapedstring);
				if (ret > 0)
				{
					va_list	ap;
					if(self->param.argv[0].name[0] == '\0') /*position params*/
					{
						jsonrpc_mstream_print(stream, ",\"params\":[");
						va_start(ap, param_signature);
						for(i=0;i<ret;i++)
						{
							if(i!=0) 
								jsonrpc_mstream_print(stream, ",");
							switch(self->param.argv[i].json.type)
							{
								case JSONRPC_TYPE_NUMBER:
									valuenumber = va_arg(ap, double);
									self->param.argv[i].json.u.number = valuenumber;
									jsonrpc_mstream_print(stream, "%.0lf", valuenumber);
									LogDebug("stream : %s\n",stream);
									break;
								case JSONRPC_TYPE_BOOLEAN:
									valueboolean = va_arg(ap, jsonrpc_bool_t);
									self->param.argv[i].json.u.boolean = valueboolean;
									jsonrpc_mstream_print(stream, "%s", (valueboolean==JSONRPC_FALSE)?"false":"true");
									break;
								case JSONRPC_TYPE_STRING:
									valuestring = va_arg(ap, char *);
									escapedstring = escaped_string(valuestring);
									jsonrpc_mstream_print(stream, "%s", escapedstring);
									free(escapedstring);
									break;
								case JSONRPC_TYPE_ARRAY:
									break;
								case JSONRPC_TYPE_OBJECT:
									break;
								case JSONRPC_TYPE_NULL:
									break;
								default:
									break;
							}

						}
						jsonrpc_mstream_print(stream, "]");
					}
					else/*named params*/
					{
						jsonrpc_mstream_print(stream, ",\"params\":{");
						va_start(ap, param_signature);
						for(i=0;i<ret;i++)
						{
							if(i!=0) 
								jsonrpc_mstream_print(stream, ",");
							switch(self->param.argv[i].json.type)
							{
								case JSONRPC_TYPE_NUMBER:
									valuenumber = va_arg(ap, double);
									self->param.argv[i].json.u.number = valuenumber;
									jsonrpc_mstream_print(stream, "\"%s\":%.0lf",self->param.argv[i].name, valuenumber);									
									//LogDebug("stream : %s\n",stream);
									break;
								case JSONRPC_TYPE_BOOLEAN:
									valueboolean = va_arg(ap, jsonrpc_bool_t);
									self->param.argv[i].json.u.boolean = valueboolean;
									jsonrpc_mstream_print(stream, "\"%s\":%s", self->param.argv[i].name, (valueboolean==JSONRPC_FALSE)?"false":"true");
									break;
								case JSONRPC_TYPE_STRING:
									valuestring = va_arg(ap, char *);
									escapedstring = escaped_string(valuestring);
									jsonrpc_mstream_print(stream, "\"%s\":%s", self->param.argv[i].name,escapedstring);
									free(escapedstring);
									break;
								case JSONRPC_TYPE_ARRAY:
									break;
								case JSONRPC_TYPE_OBJECT:
									break;
								case JSONRPC_TYPE_NULL:
									break;
								default:
									break;
							}

						}

						
						//jsonrpc_mstream_print(stream, "this is params");
						jsonrpc_mstream_print(stream, "}");
					}
					va_end(ap);
				}
			}
			jsonrpc_mstream_print(stream, "}");
			const char * res = jsonrpc_mstream_getbuf(stream);
    		self->net.TcpSend(self->net_handle, res, NULL);
			printf("notify : %s\n", res);
			break;
		}

	}

    
	return JSONRPC_ERROR_OK;
}


/* Render the cstring provided to an escaped version that can be printed. */
static char * escaped_string(const char *str)
{
	const char *ptr;char *ptr2,*out;int len=0,flag=0;unsigned char token;

	for (ptr=str;*ptr;ptr++) flag|=((*ptr>0 && *ptr<32)||(*ptr=='\"')||(*ptr=='\\'))?1:0;
	if (!flag)
	{
		len=ptr-str;
		out=(char*)malloc(len+3);
		if (!out) return 0;
		ptr2=out;*ptr2++='\"';
		strcpy(ptr2,str);
		ptr2[len]='\"';
		ptr2[len+1]=0;
		return out;
	}


	if (!str)
	{
		out=(char*)malloc(3);
		if (!out) return 0;
		strcpy(out,"\"\"");
		return out;
	}
	ptr=str;while ((token=*ptr) && ++len) {if (strchr("\"\\\b\f\n\r\t",token)) len++; else if (token<32) len+=5;ptr++;}

	out=(char*)malloc(len+3);
	if (!out) return 0;

	ptr2=out;ptr=str;
	*ptr2++='\"';
	while (*ptr)
	{
		if ((unsigned char)*ptr>31 && *ptr!='\"' && *ptr!='\\') *ptr2++=*ptr++;
		else
		{
			*ptr2++='\\';
			switch (token=*ptr++)
			{
				case '\\':	*ptr2++='\\';	break;
				case '\"':	*ptr2++='\"';	break;
				case '\b':	*ptr2++='b';	break;
				case '\f':	*ptr2++='f';	break;
				case '\n':	*ptr2++='n';	break;
				case '\r':	*ptr2++='r';	break;
				case '\t':	*ptr2++='t';	break;
				default: sprintf(ptr2,"u%04x",token);ptr2+=5;	break;	/* escape and print */
			}
		}
	}
	*ptr2++='\"';*ptr2++=0;
	return out;
}


