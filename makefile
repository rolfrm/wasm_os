all:
	make -C libawsm
	cp libawsm/libawsm.so ./
	make -f makefile.host
#make -f makefile.demo	
clean:
	rm ./libawsm.so
	make -f makefile.demo clean
	make -f makefile.game clean
	make clean -C libawsm
