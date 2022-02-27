CC=gcc

mysh: mysh.c utils.c alias.c linkedList.c mysh.h alias.h linkedList.h
	$(CC) -o mysh -Wall -Werror -g mysh.c utils.c alias.c linkedList.c

clean:
	rm -f mysh