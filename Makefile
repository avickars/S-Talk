all: s-talk

s-talk: s-talk.c list.c
	gcc -pthread -o s-talk s-talk.c list.c

clean:
	rm s-talk