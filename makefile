all:3dobjtool

3dobjtool:main.o
	g++ -o 3dobjtool main.o

main.o:main.cpp
	g++ -c -o main.o main.cpp

clean:
	rm -rf *.o
	rm -rf 3dobjtool


