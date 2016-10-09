#define _GNU_SOURCE

#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <time.h>

#define FRAME_LEN 		1408			// 4 * 352
#define THRESHOLD 		500			// 44100 fr/sec * 2 sa/fr * 2 B/sa * 4 sec / 1408 B

int infifo;
int outfifo;

signed short *silence;

int rx_len;
int pipe_size;

int counter;

unsigned char audio_buffer[FRAME_LEN];

int main( int argc, char *argv[] ) {

	if (argc != 3) {
		printf("Needs two arguments: input FIFO, output FIFO\n");
		return -1;
	}

	infifo = open(argv[1], (O_RDONLY | O_NONBLOCK));
	outfifo = open(argv[2], O_WRONLY);

	pipe_size = fcntl(outfifo, F_SETPIPE_SZ, 1 * FRAME_LEN);

	silence = malloc(FRAME_LEN);
	memset(silence, 0, FRAME_LEN);

	struct timespec ts;
	ts.tv_sec = 0;
	ts.tv_nsec = 5000;

	while(1) {

		rx_len = read(infifo, (void*)audio_buffer, FRAME_LEN);	//Filestream, buffer to store in, number of bytes to read (max)

		if (rx_len < 0)
		{
			//An error occured (waiting for new data)
			switch(counter) {
			case(THRESHOLD):
				write(outfifo, silence, FRAME_LEN);
				break;
			default:
				counter++;
			}
		}
		else if (rx_len == 0)
		{
			//No data waiting
			write(outfifo, silence, FRAME_LEN);
		}
		else
		{
			// Write data
			counter = 0;
			write(outfifo, audio_buffer, FRAME_LEN);
		}

		nanosleep(&ts, NULL);

	}

	return 0;

}
