#ifndef LIBKERMIT
#define LIBKERMIT

#include <stdlib.h>
#include <string.h>

#ifdef __GNUC__
#define PACK( __Declaration__ ) __Declaration__ __attribute__((__packed__))
#endif

#ifdef _MSC_VER
#define PACK( __Declaration__ ) __pragma( pack(push, 1) ) __Declaration__ __pragma( pack(pop))
#endif

#define MAXL 250
#define TIME 5

typedef struct {
    int len;
    char payload[1400];
} msg;

/* Mini-Kermit structure */
PACK( struct mini_kermit {
	char soh;
	unsigned char len;
	unsigned char seq;
	char type;
	char *data;
	unsigned short check;
	char mark;
});

PACK( struct init_package_data {
    unsigned char maxl;
	char time;
	char npad;
	char padc;
	char eol;
	char qctl;
	char qbin;
	char chkt;
	char rept;
	char capa;
	char r;
});

void initialize_init_package_data(init_package_data *S_package_data);
void send_init(msg *init, int seq);
void send_package(msg *init, char *data, unsigned char len, unsigned char seq, char type);
unsigned short crc16_ccitt(const char *buf, int len);

#endif // LIBKERMIT

