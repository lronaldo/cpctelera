#define TIMEOUT -1
#define SERIAL_ERROR -8
#define BREAK -16

#ifdef __cplusplus
extern "C" {
#endif

HANDLE SerialInit(char*, DWORD, int, int, char, char, int, int, int, int); 

int SerialGetc(HANDLE);

int SerialPutc(HANDLE, char);

char SerialGets(char*, int, HANDLE);

void SerialPuts(HANDLE, char*);

int SerialGetModemStatus(HANDLE, int);

void SerialClearRxBuffer(HANDLE);

void SerialClose(HANDLE); 

#ifdef __cplusplus
}
#endif