.SUFFIXES:
.PHONY: x, pc1, pc2, fum, clean

compilador:=g++
opcionesc:= -std=c++11 -pthread -Wfatal-errors

x: monitor_em_exe
	./$<

pc1: prodcons1_su_exe
	./$<

pc1b: prodcons1_su_fifo_exe
	./$<

pc2: prodcons_mu_exe
	./$<

fum: fumadores_exe
	./$<

esc: lector_escritor_exe
	./$<

fuesm: fumadores_estanqueros_multiples_exe
	./$<

fumfin: fumadores_con_fin_exe
	./$<

%_exe: %.cpp scd.cpp scd.h
	$(compilador) $(opcionesc)  -o $@ $< scd.cpp

clean:
	rm -f *_exe
