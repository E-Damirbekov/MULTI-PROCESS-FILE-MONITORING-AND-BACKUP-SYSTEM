CC = gcc
CFLAGS = -Wall -g
OBJ = main.o scanner.o backup.o

all: monitor

monitor: $(OBJ)
	$(CC) $(OBJ) -o monitor

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

auto: all
	@echo "Running automation script..."
	@chmod +x scripts/run_system.sh
	@./scripts/run_system.sh

clean:
	rm -f *.o monitor logs/report.txt backup/*.txt
