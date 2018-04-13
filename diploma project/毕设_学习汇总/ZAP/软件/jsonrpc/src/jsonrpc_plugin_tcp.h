/*
 * Copyright (c) 2012 Jonghyeok Lee <jhlee4bb@gmail.com>
 *
 * jsonrpC is free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#ifndef jsonrpc_jsonrpc_plugin_tcp_h
#define jsonrpc_jsonrpc_plugin_tcp_h

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
	jsonrpc_handle_t	(* TcpOpen ) (int isClient, va_list ap);
	void				(* TcpClose) (jsonrpc_handle_t net);
	const char *		(* TcpRecv ) (jsonrpc_handle_t net, unsigned int timeout, void **desc);
	jsonrpc_error_t		(* TcpSend ) (jsonrpc_handle_t net, const char *data, void *desc);
	jsonrpc_error_t		(* TcpTick) (jsonrpc_handle_t net);
} jsonrpc_net_plugin_t;

const jsonrpc_net_plugin_t	* jsonrpc_plugin_tcp(void);


#ifdef  __cplusplus
}
#endif
#endif

