build:
	make -C source
	make -C example
	
clean:
	make -C source clean
	make -C example clean
	
rebuild: clean build