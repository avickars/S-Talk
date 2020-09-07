all: s-talk

s-talk: s-talk.c receiver.c display.c input.c sender.c
	gcc -pthread -o s-talk s-talk.c receiver.c display.c input.c sender.c instructorList.o

clean:
	rm s-talk