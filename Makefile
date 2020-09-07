all: s-talk

s-talk: s-talk.c receiver.c display.c input.c sender.c
	gcc -pthread -o s-talk s-talk.c receiver.c display.c input.c sender.c list.c

clean:
	rm s-talk