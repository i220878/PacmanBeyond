# Makefile for Writing Make Files Example
 
# ****************************************************
# Targets needed to bring the executable up to date

sfml-app: PacManOverdrive.o Stuff.o Definitions.o
	g++ PacManOverdrive.o Stuff.o Definitions.o -o sfml-app -lsfml-audio -lsfml-window -lsfml-system -lsfml-graphics -lpthread
	
PacManOverdrive.o: PacManOverdrive.cpp Definitions.o
	g++ -c PacManOverdrive.cpp
	
Stuff.o: Stuff.h Stuff.cpp Definitions.o
	g++ -c Stuff.cpp
	
Definitions.o: Definitions.cpp
	g++ -c Definitions.cpp
	
