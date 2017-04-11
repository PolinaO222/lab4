all:
	gcc -o generator generator.c -fsanitize=address
	gcc -o gamma gamma.c -fsanitize=address
