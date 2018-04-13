
typedef enum {
  EMBER_AF_FILE_DESCRIPTOR_OPERATION_NONE =  0x00,
  EMBER_AF_FILE_DESCRIPTOR_OPERATION_READ =  0x01,
  EMBER_AF_FILE_DESCRIPTOR_OPERATION_WRITE = 0x02,
} EmberAfFileDescriptorOperation;

typedef void (*EmberAfFileDescriptorReadyCallback)(void* data, EmberAfFileDescriptorOperation operation);

typedef struct {
  EmberAfFileDescriptorReadyCallback callback;
  void* dataPassedToCallback;
  EmberAfFileDescriptorOperation operation;
  int fileDescriptor;
} EmberAfFileDescriptorDispatchStruct;

EmberStatus emberAfPluginFileDescriptorDispatchAdd(EmberAfFileDescriptorDispatchStruct* dispatchStruct);
EmberStatus emberAfPluginFileDescriptorDispatchWaitForEvents(int32u timeoutMs);
boolean emberAfPluginFileDescriptorDispatchRemove(int fileDescriptor);
