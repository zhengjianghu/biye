/*
 * Copyright (c) 2012 Jonghyeok Lee <jhlee4bb@gmail.com>
 *
 * jsonrpC is free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#ifndef jsonrpc_jsonrpc_plugin_cjson_h
#define jsonrpc_jsonrpc_plugin_cjson_h

#include "Platform.h"

#include "Jsonrpc.h"

#ifdef  __cplusplus
extern "C" {
#endif

/**
 * JSON-RPC json api plug-in
 * -
 */
typedef struct
{
	jsonrpc_handle_t	(* parse  ) (const char *json);
	void				(* release) (jsonrpc_handle_t json);
	jsonrpc_handle_t	(* get    ) (jsonrpc_handle_t json, const char *path);
	jsonrpc_handle_t	(* get_at ) (jsonrpc_handle_t json, size_t index);
	const char *		(* get_key_at) (jsonrpc_handle_t json, size_t index);
	jsonrpc_bool_t		(* valueof) (jsonrpc_handle_t json, jsonrpc_json_t *value);
	size_t				(* length) (jsonrpc_handle_t json);
} jsonrpc_json_plugin_t;

const jsonrpc_json_plugin_t	* jsonrpc_plugin_cjson(void);

#ifdef  __cplusplus
}
#endif
#endif

