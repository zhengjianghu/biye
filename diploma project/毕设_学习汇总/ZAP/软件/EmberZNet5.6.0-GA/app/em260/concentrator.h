/*
 * File: concentrator.h
 * Description: Concentrator support for the NCP.
 *
 * Author(s): Maurizio Nanni, maurizio.nanni@ember.com
 *
 * Copyright 2012 by Ember Corporation. All rights reserved.
 */

#include "app/util/ezsp/ezsp-frame-utilities.h"

extern EmberEventControl emberAfPluginConcentratorSupportLibraryEventControl;
void emberAfPluginConcentratorSupportLibraryEventHandler(void);

void emConcentratorSupportRouteErrorHandler(EmberStatus status, EmberNodeId nodeId);

void emConcentratorSupportDeliveryErrorHandler(EmberOutgoingMessageType type,
                                               EmberStatus status);
                                               
EmberStatus emberAfPluginEzspZigbeeProSetConcentratorCommandCallback(boolean on,
                                                                     int16u concentratorType,
                                                                     int16u minTime,
                                                                     int16u maxTime,
                                                                     int8u routeErrorThreshold,
                                                                     int8u deliveryFailureThreshold,
                                                                     int8u maxHops);

#define ADDRESS_DISCOVERY_DELAY_QS 2
