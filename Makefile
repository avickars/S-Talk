all: s-talk

s-talk: s-talk.c receiver.c display.c input.c sender.c
	gcc -pthread -o s-talk s-talk.c receiver.c display.c input.c sender.c instructorList.o

valgrind: s-talk
	valgrind --leak-check=full --show-leak-kinds=all ./s-talk 12345 aidan-MS-7B93 22110

clean:
	rm s-talk