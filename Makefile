all: s-talk

s-talk: s-talk.c list.c receiver.c display.c input.c sender.c
	gcc -pthread -o s-talk s-talk.c list.c receiver.c display.c input.c sender.c

clean:
	rm s-talk