#include "insona_util.h"
#include <string.h>
#include <stdio.h>

int8u hexToInt(int8u ch)
{
  return ch - (ch >= 'a' ? 'a' - 10
               : (ch >= 'A' ? 'A' - 10
                  : (ch <= '9' ? '0'
                     : 0)));
}

int32s string2SignedInt(char* data){
  int32u value = 0;
	if(data[0]=='-'){
    value = string2UnsignedInt(data+1);
    return (0-value);
	}else{
    return string2UnsignedInt(data);
  }

}

int32u string2UnsignedInt(char* data)
{
  char *string = data;
  int32u result = 0;
  int8u base = 10;
  int8u i;
  
  int8u length = strlen(data);
  printf("length:%d\n", length);
  for (i = 0; i < length; i++) {
    int8u next = string[i];
    printf("next:%d\n", next);
    if (FALSE_C && i == 0 && next == '-') {
      // do nothing
    } else if ((next == 'x' || next == 'X')
               && result == 0
               && (i == 1 || i == 2)) {
      base = 16;
    } else {
      int8u value = hexToInt(next);
      if (value < base) {
        result = result * base + value;
      } else {
        return 0;
      }
    }
  }
  return result;
}


void sendDataParse(int8u* dstp,char* srdp){
  char delim[] = ",";
  char *result = NULL;
  int8u temp = 0;
  int8u len = 0;
  int8u p = 0;
  printf("%s,%s\n","sendDataParse",srdp );
  result = strtok(srdp,delim);
  while(NULL!=result){
    len = strlen(result);
    printf("%s,%d\n",result,len);
    if(result[0]=='['){
      result++;
    }else if(result[len-1]==']'){
      result[len-1]='\0';
    }
    temp =string2UnsignedInt(result);
    dstp[p] = temp;
    p++;
    result = strtok(NULL,delim);
  }
  printf("sendDataParse %s\n", dstp);
}