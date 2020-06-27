all: s-talk

s-talk: s-talk.c list.c
	gcc -o s-talk s-talk.c list.c

clean:
	rm s-talk