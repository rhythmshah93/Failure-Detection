CC = gcc

all: failure_detection

default: failure_detection

failure_detection:
	$(CC) failure_detection.c -o failure_detection -lpthread

clean:
	rm -rf failure_detection endpoints.txt