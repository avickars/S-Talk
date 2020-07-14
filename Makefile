all: s-talk

s-talk: s-talk.c receiver.c display.c input.c sender.c
	gcc -pthread -o s-talk s-talk.c instructorList.o receiver.c display.c input.c sender.c

valgrind: s-talk
	valgrind --leak-check=full ./s-talk 12345 aidan-MS-7B93 22110

#valgrind: s-talk
#	valgrind ./s-talk 12345 aidan-MS-7B93 22110

clean:
	rm s-talk