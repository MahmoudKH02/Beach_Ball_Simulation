G = gcc -g
O = -o
names = main player teamLead functions.o drawer

files:
	gcc -c functions.c -o functions.o
	$(G) player.c functions.o $(O) player
	$(G) teamLead.c functions.o $(O) teamLead
	$(G) main.c functions.o $(O) main
	$(G) drawer.c $(O) drawer -lglut -lGLU -lGL -lm


clean:
	rm -f $(names)