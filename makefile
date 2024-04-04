G = gcc -g
O = -o
names = main player teamLead

files:
	$(G) main.c $(O) main
	$(G) player.c $(O) player
	$(G) teamLead.c $(O) teamLead


clean:
	rm -f $(names)