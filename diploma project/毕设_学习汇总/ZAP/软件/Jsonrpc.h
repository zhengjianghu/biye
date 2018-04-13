/*
 * Copyright (c) 2012 Jonghyeok Lee <jhlee4bb@gmail.com>
 *
 * jsonrpC is free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#ifndef _jsonrpc_h
#define _jsonrpc_h


#include "Platform.h"
#include "PlatformMalloc.h"

#ifdef  __cplusplus
extern "C" {
#endif

#define	JSONRPC_NAME_LEN		128

/**
 * JSON-RPC error codes
 * @see json-rpc.org
 */
typedef enum
{
	  JSONRPC_ERROR_OK					= 0
	, JSONRPC_ERROR_PARSE_ERROR		    = -32700
	, JSONRPC_ERROR_INVALID_REQUEST		= -32600
	, JSONRPC_ERROR_METHOD_NOT_FOUND	= -32601
	, JSONRPC_ERROR_INVALID_PARAMS		= -32602
	, JSONRPC_ERROR_INTERNAL			= -32603
	, JSONRPC_ERROR_RESERVED_FOR_SERVER_END		= -32099
		, JSONRPC_ERROR_SERVER_OUT_OF_MEMORY
		, JSONRPC_ERROR_SERVER_INTERNAL
		, JSONRPC_ERROR_SERVER_TIMEOUT
	, JSONRPC_ERROR_RESERVED_FOR_SERVER_BEGIN	= -32000
} jsonrpc_error_t;


/**
 * JSON-RPC data types
 */
typedef enum {
	JSONRPC_TYPE_NUMBER    = 'i',
	JSONRPC_TYPE_BOOLEAN   = 'b',
	JSONRPC_TYPE_STRING    = 's',
	JSONRPC_TYPE_ARRAY	   = 'a',
	JSONRPC_TYPE_OBJECT    = 'o',
	JSONRPC_TYPE_NULL	   = 'n',
	JSONRPC_TYPE_UNDEFINED = 'u'
} jsonrpc_type_t;
#define JSONRPC_TYPES		"ibsaonu"

/**
 * JSON-RPC boolean type
 */
typedef enum {
	JSONRPC_FALSE,
	JSONRPC_TRUE
} jsonrpc_bool_t;

typedef void *	jsonrpc_handle_t;	///< handle type (general purpose)

/**
 * JSON-RPC json type
 *
 */
typedef struct
{
	jsonrpc_type_t		type;		///< type of the value
	union
	{
		double				number;		///< number value
		jsonrpc_bool_t		boolean;	///< boolean value
		char				*string;	///< string value
		jsonrpc_handle_t	array;		///< array json handle
		jsonrpc_handle_t	object;		///< object json handle
	} u;
} jsonrpc_json_t;

/**
 * JSON-RPC parameter
 *
 */
typedef struct
{
	/**
	 * name of param
	 */
	char				name[JSONRPC_NAME_LEN];
	size_t				index;
	jsonrpc_json_t		json;	///< json value
} jsonrpc_param_t;


/**
 * JSON-RPC method
 * -
 * @param	argc	argument count
 * @param	argv	arguments
 * @param	print_result	write result
 * @param	ctx		print context
 */
typedef jsonrpc_error_t (* jsonrpc_method_t) (
								int argc
								, const jsonrpc_param_t *argv
								, void (* print_result)(void *ctx, const char *fmt, ...)
								, void *ctx
							);




/**
 * JSON-RPC server
 *
 */
typedef struct jsonrpc_server	jsonrpc_server_t;

jsonrpc_server_t *
jsonrpc_server_open (int isClient, ...);

void
jsonrpc_server_close (jsonrpc_server_t *self);

jsonrpc_error_t
jsonrpc_server_register_method (
				jsonrpc_server_t *self
				, jsonrpc_bool_t has_return
				, jsonrpc_method_t method
				, const char *method_name
				, const char *param_signature
			);

jsonrpc_error_t
jsonrpc_server_run (jsonrpc_server_t *self, unsigned int timeout);

jsonrpc_error_t
jsonrpc_notify_client (
				jsonrpc_server_t *self,
				const char *method_name,
				const char *param_signature
				, ...);

void
jsonrpc_set_alloc_funcs (
                         void * (* _malloc) (size_t n, void *userdata)
                         , void * (* _realloc) (void *mem, size_t n, void *userdata)
                         , void (* _free)(void *mem, void *userdata)
                         , void *userdata
                         );


#ifdef  __cplusplus
}
#endif

#endif  //_jsonrpc_h
