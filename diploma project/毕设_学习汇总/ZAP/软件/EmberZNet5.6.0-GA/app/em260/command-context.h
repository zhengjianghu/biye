//
// command-context.h
//
// May 20, 2015
//
// Declarations of command context structs.
//
// Copyright 2015 Silicon Laboratories, Inc.
//

#ifndef __COMMAND_CONTEXT_H__
#define __COMMAND_CONTEXT_H__

#include "app/util/ezsp/ezsp-enum.h"

typedef struct {
  boolean consumed;
  EzspStatus status;
  
  union {
    EzspValueId valueId;
    EzspExtendedValueId extendedValueId;
  };
  
  int32u characteristics;

  int8u valueLength;
  int8u value[128];
} EmberAfPluginEzspValueCommandContext;

typedef struct {
  boolean consumed;

  EzspPolicyId policyId;
  EzspStatus status;
  EzspDecisionId decisionId;
} EmberAfPluginEzspPolicyCommandContext;

typedef struct {
  boolean consumed;
  EzspConfigId configId;
  EzspStatus status;
  int16u value;
} EmberAfPluginEzspConfigurationValueCommandContext;

#endif /* __COMMAND_CONTEXT_H__ */

