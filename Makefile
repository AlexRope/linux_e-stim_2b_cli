all:
	gcc -DDISPLAY_STRING -o estim estim.c
	chmod +x estim
clean:
	rm -f estim
