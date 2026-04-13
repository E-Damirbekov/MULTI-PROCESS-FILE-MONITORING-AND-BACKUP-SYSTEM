cat <<EOF > Makefile
CC = gcc
CFLAGS = -Wall -g
OBJ = main.o scanner.o backup.o

all: monitor

monitor: \$(OBJ)
	\$(CC) \$(OBJ) -o monitor

%.o: %.c
	\$(CC) \$(CFLAGS) -c $< -o \$@

clean:
	rm -f *.o monitor logs/report.txt backup/*.txt
EOF