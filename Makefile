.PHONY: r

Program: r
	gcc -std=gnu99 -Wall -Wextra -Werror -pedantic proj2.c -o proj2 -lpthread

r:
	rm -f /dev/shm/xgurec00*
