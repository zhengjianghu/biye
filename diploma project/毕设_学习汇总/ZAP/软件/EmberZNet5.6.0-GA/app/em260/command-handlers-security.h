//
// command-handlers-zigbee-pro.h
//
// Author(s): Andrew Keesler
//
// Created March 20, 2015
//
// Prototypes for ZigBee PRO Security command handler functions.
//

//------------------------------------------------------------------------------
// Ezsp Command Handlers

EmberStatus emberAfPluginEzspSecurityHandleKeyCommandCallback(int8u index,
                                                              EmberKeyStruct* keyStruct);
                                                              
EmberStatus emberAfEzspAesMmoHashCommandCallback(EmberAesMmoHashContext* context,
                                                 boolean finalize,
                                                 int8u length,
                                                 int8u* data,
                                                 EmberAesMmoHashContext* returnContext);

EmberStatus emberAfPluginEzspSecurityHandleKeyCommandCallback(int8u index,
                                                              EmberKeyStruct* keyStruct);

EmberStatus emberAfEzspRemoveDeviceCommandCallback(EmberNodeId destShort,
                                                   EmberEUI64 destLong,
                                                   EmberEUI64 targetLong);

EmberStatus emberAfEzspUnicastNwkKeyUpdateCommandCallback(EmberNodeId destShort,
                                                          EmberEUI64 destLong,
                                                          EmberKeyData* key);
                                                          
void emberAfPluginEzspSecurityGetValueCommandCallback(EmberAfPluginEzspValueCommandContext* context);
void emberAfPluginEzspSecuritySetValueCommandCallback(EmberAfPluginEzspValueCommandContext *context);
void emberAfPluginEzspSecuritySetValueCommandCallback(EmberAfPluginEzspValueCommandContext* context);
void emberAfPluginEzspSecurityGetConfigurationValueCommandCallback(EmberAfPluginEzspConfigurationValueCommandContext* context);
void emberAfPluginEzspSecuritySetConfigurationValueCommandCallback(EmberAfPluginEzspConfigurationValueCommandContext* context);
void emberAfPluginEzspSecurityPolicyCommandCallback(EmberAfPluginEzspPolicyCommandContext* context);
