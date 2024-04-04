G = gcc -g
O = -o
names = main player teamLead functions.o

files:
	gcc -c functions.c -o functions.o
	$(G) player.c functions.o $(O) player
	$(G) teamLead.c functions.o $(O) teamLead
	$(G) main.c $(O) main


clean:
	rm -f $(names)