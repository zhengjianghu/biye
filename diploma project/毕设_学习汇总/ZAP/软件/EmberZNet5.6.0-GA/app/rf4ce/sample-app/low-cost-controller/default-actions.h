// Copyright 2014 Silicon Laboratories, Inc.
/**
 * The purpose of the default-actions is to define the default (built-in)
 * actions for the remote controller.
 *
 * Because the controller supports both rf and ir actions, each of the
 * actions are defined and placed at the appropriate place in the corresponding
 * table.
 *
 * rf mappings must be according to "ZRC Profile Specification Version 2.0" and
 * ir mappings must be according to one of the formats supported by the
 * infrared-led-driver.
 */

#ifndef __DEFAULT_ACTIONS_H__
#define __DEFAULT_ACTIONS_H__


typedef struct
{
  const int16u format;
  const int8u *data;
  const int8u length;
} DefaultActionsIr_t;

typedef struct
{
  const int8u code;
  const int8u config;
} DefaultActionsRf_t;

extern const DefaultActionsIr_t defaultActionsIr[ 49];
extern const DefaultActionsRf_t defaultActionsRf[ 49];

#endif // __DEFAULT_ACTIONS_H__
