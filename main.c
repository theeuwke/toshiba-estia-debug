#include <arpa/inet.h> // inet_addr()
#include <netdb.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h> // bzero()
#include <sys/socket.h>
#include <unistd.h> // read(), write(), close()
#define MAX (130*2+1)
#define PORT 4001
#define SA struct sockaddr
#include <endian.h>
#include <errno.h>

//#define DEBUG_RAW
#ifdef DEBUG_RAW
#define raw_print(fmt, ...) printf(fmt, ##__VA_ARGS__)
#else
#define raw_print(fmt, ...) do {} while (0)
#endif

#define TYPE 1

int read_packet(int sockfd, unsigned char *buff, int len) {
	int ret, bytes_read = 0;

	bzero(buff, sizeof(buff));
	do {
		ret = read(sockfd, &buff[bytes_read], len - bytes_read);
		if(ret > 0)
			bytes_read += ret;
		else
			return ret;
	} while (bytes_read < len);
	raw_print("read: %d\n", bytes_read);
	return bytes_read;
}

#define PACKET_SIZE 126 /* bytes */
#define ALIGN 12

void decode_packet(unsigned char *buff, int type, int shift, int align, int var, bool collum) {
	int index = 0;
	char * ascii;
	uint16_t * word;
	int16_t * sword;
	uint32_t * dword;
	int32_t * sdword;
	uint16_t l_type;
	float * fword;
	int max_8;
	int max_16;
	int max_32; 
	ascii = (char *) (&buff[2]+shift);
	word = (uint16_t *) (&buff[2]+shift);
	sword = (int16_t *) (&buff[2]+shift);
	dword = (uint32_t *) (&buff[2]+shift);
	sdword = (int32_t *) (&buff[2]+shift);
	fword = (float *) (&buff[2]+shift);
	
	l_type = be16toh(((unsigned short*)buff)[0]);

	if((l_type != (unsigned short) type) && (type != 255))
		return;

	if((l_type == 0) || (type != 255))
		system("clear");

	if(var != 255) {
		index = var;
		max_8 = max_16 = max_32 = var + 1;
	} else {
		max_8 = PACKET_SIZE;
		max_16 = max_8 / 2;
		max_32 = max_8 / 4;
	}

	printf("decode type: %d\n", l_type);

	if(collum == true) {
		// print header
		printf("raw:\t");
		for(int j = 0; j < max_8; j++)
			printf("%x ", buff[j]);
		printf("\n");

		printf("index ascii    hex       u8      s8      u16be     u16   s16be     s16         u32be         u32       s32be         s32\n");

		if(align != 12)
			max_8 = align;

		for(int i = index; i < max_8; i++) {
			if(i % 4 == 0)
				printf("------------------------------------------------------------------------------------------------------------------------\n");
			printf("%*d\t", 5, i);
			printf("%*c\t", 3, ((ascii[i] > 32)  && (ascii[i] < 127)) ? ascii[i] : ' ');
			printf("%*x\t", 2, buff[i+2]);
			printf("%*d\t", 3, buff[i+2]);
			printf("%*d\t", 3, ascii[i]);
			if(i % 2 == 0) {
				word = (uint16_t *) (&buff[i+2]+shift);
				sword = (int16_t *) (&buff[i+2]+shift);
				printf("%*d\t", 6, be16toh(word[0]));
				printf("%*d\t", 6, word[0]);
				printf("%*d\t", 6, be16toh(sword[0]));
				printf("%*d\t", 6, sword[0]);
			}
			if(i % 4 ==0) {
				dword = (uint32_t *) (&buff[i+2]+shift);
				sdword = (int32_t *) (&buff[i+2]+shift);
				//idword be
				printf("%*d", 12, be32toh(dword[0]));
				//word le
				printf("%*d", 12, dword[0]);
				//word be
				printf("%*d", 12, be32toh(sdword[0]));
				//word le
				printf("%*d", 12, sdword[0]);
			}
			printf("\n");
		}
	} else {

		printf("ascii:\t");
		for(int i = index; i < max_8; i++)
			printf("%c", ascii[i]);
		printf("\n");

		printf("index:\t");
		for(int i = index; i< max_8; i++)
			if((i < 10) && (i > 0))
				printf("%*d", align+1, i);
			else
				printf("%*d", align, i);
		printf("\n");

		//hex
		printf("raw:\t");
		for(int i = 0; i < max_8; i++)
			printf("%*x ", align, buff[i]);
		printf("\n");

#if 0 //TODO
      //dec unsigned
		printf("char:\t");
		for(int i = 2; i < 128; i++)
			for(int j = 0; j < align-1; j++)
				printf(" ");
		printf("%c", ascii[i]);
		printf("\n");
#endif

		//dec unsigned
		printf("u8:\t");
		for(int i = index; i < max_8; i++)
			printf("%*d ", align, buff[i]);
		printf("\n");

		//ascii
		printf("s8:\t");
		for(int i = index; i < max_8; i++)
			printf("%*d ", align, ascii[i]);
		printf("\n");

		//word be
		printf("u16be:\t");
		for(int i = index; i < max_16; i++)
			printf("%*d ", align, be16toh(word[i]));
		printf("\n");

		//word le
		printf("u16le:\t");
		for(int i = index; i < max_16; i++)
			printf("%*d ", align, word[i]);
		printf("\n");

		//word be
		printf("s16be:\t");
		for(int i = index; i < max_16; i++)
			printf("%*d ", align, be16toh(sword[i]));
		printf("\n");

		//word le
		printf("s16le:\t");
		for(int i = index; i < max_16; i++)
			printf("%*d ", align, sword[i]);
		printf("\n");

		//idword be
		printf("u32be:\t");
		for(int i = index; i < max_32; i++)
			printf("%*d ", align, be32toh(dword[i]));
		printf("\n");

		//word le
		printf("u132le:\t");
		for(int i = index; i < max_32; i++)
			printf("%*d ", align, dword[i]);
		printf("\n");

		//word be
		printf("s32be:\t");
		for(int i = index; i < max_32; i++)
			printf("%*d ", align, be32toh(sdword[i]));
		printf("\n");

		//word le
		printf("s32le:\t");
		for(int i = index; i < max_32; i++)
			printf("%*d ", align, sdword[i]);
		printf("\n");

#ifdef FLOAT
		//word be
		printf("f32be:\t");
		for(int i = index; i < max_32; i++) {
			uint32_t u32_tmp = be32toh(dword[i]);
			float *f_tmp;
			f_tmp = (float *) &u32_tmp;
			printf("%*.2f ", align, *f_tmp);
		}
		printf("\n");

		//word le
		printf("f32le:\t");
		for(int i = index; i < max_32; i++)
			printf("%*.2f ", align, fword[i]);
		printf("\n");
#endif
	}
}

void func(int sockfd, int type, int shift, int align, int var, bool collum)
{
	unsigned char buff[MAX];
	int n, ret, index;
	int sync_bytes, bytes_read;

	for (;;) {
		sync_bytes = 0;
		ret = read_packet(sockfd, buff, MAX);

		/* find sync */
		for(n = 0; n < ret; n++) {
			raw_print("%02x ", buff[n]);
			if(buff[n] == 0xf8) {
				raw_print("sync byte found at: %d\n", n);
				sync_bytes++;
			}
			if((sync_bytes == 2)) {
				/* read remainder to be in sync */
				index = n + 1;
				if((ret - index) >= 128) {
					decode_packet(&buff[index], type, shift, align, var, collum);
				}
				sync_bytes = 0;
			}
			raw_print("\n");
		}
	}
}

int main(int argc, char *argv[])
{
	int sockfd, connfd, ret;
	struct sockaddr_in servaddr, cli;
	int option;
	int port = PORT;
	int type = 255;
	char *ip_address = NULL;
	int shift = 0;
	int align = ALIGN;
	int var = 255;
	bool collum = false;

	opterr = 0;
	while ( (option=getopt(argc, argv, "i:p:t:hs:a:v:c")) != EOF ) {
		switch ( option ) {
			case 'p': port = atoi(optarg);
				  break;
			case 'i': ip_address = optarg;
				  break;
			case 't': type = atoi(optarg);
				  break;
			case 's': shift = atoi(optarg);
				  break;
			case 'a': align = atoi(optarg);
				  break;
			case 'v': var = atoi(optarg);
				  break;
			case 'c': collum = true;
				  break;
			case 'h': fprintf(stderr,"Unknown option %c\n", optopt);
				  exit(1);
				  break;
		}
	}

	if (ip_address) 
		printf("ip address: %s, ", ip_address);
	else {
		fprintf(stderr, "error: no ip address defined\n");
		return -EINVAL;
	}
	if (port)    
		printf("port: %d\n", port);
	if(type != 255)
		printf("type: %d\n", type);

	if(shift)
		printf("shift: %d\n", shift);

	// socket create and verification
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd == -1) {
		printf("socket creation failed...\n");
		exit(0);
	}
	else
		printf("Socket successfully created..\n");
	bzero(&servaddr, sizeof(servaddr));

	// assign IP, PORT
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = inet_addr(ip_address);
	servaddr.sin_port = htons(PORT);

	// connect the client socket to server socket
	ret = connect(sockfd, (SA*)&servaddr, sizeof(servaddr));
	if (ret != 0) {
		printf("connection with the server failed...\n");
		exit(0);
	}
	else
		printf("connected to the server..\n");

	// function for chat
	func(sockfd, type, shift, align, var, collum);

	// close the socket
	close(sockfd);
}
