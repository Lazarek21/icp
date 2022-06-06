.PHONY=clean run
ThreadApp:ThreadApp.pro
	qmake -o NewMakefile ThreadApp.pro
	make -f NewMakefile
	
clean: 
	make -f NewMakefile distclean
	rm -f 

run: ThreadApp
	./ThreadApp

