G = gcc -g
O = -o
names = main

files:
	$(G) main.c $(O) main
	$(G) player.c $(O) player


clean:
	rm -f $(names)