// Copyright 2014 Silicon Laboratories, Inc.

#include PLATFORM_HEADER
#include CONFIGURATION_HEADER
#include EMBER_AF_API_EMBER_TYPES
#include EMBER_AF_API_HAL

void emberAfPluginHeartbeatTickCallback(void)
{
  static int32u lastMs = 0;
  int32u nowMs = halCommonGetInt32uMillisecondTick();
  if (EMBER_AF_PLUGIN_HEARTBEAT_PERIOD_QS * MILLISECOND_TICKS_PER_QUARTERSECOND
      < elapsedTimeInt32u(lastMs, nowMs)) {
    halToggleLed(BOARD_HEARTBEAT_LED);
    lastMs = nowMs;
  }
}
