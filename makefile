

main: congame

congame: congame.c
	gcc congame.c -o congame -lncurses -lm 

clean:
	rm congame