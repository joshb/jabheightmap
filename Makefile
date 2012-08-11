CC=gcc
CFLAGS=-Wall -pedantic -I/usr/X11R6/include -I/usr/local/include
LDFLAGS=-L/usr/X11R6/lib -L/usr/local/lib -lm -lX11 -lXext -lGL -lGLU -lpng
OBJS=input.o main.o map.o my_math.o object.o octree.o texture.o world.o

engine:	$(OBJS)
	$(CC) $(LDFLAGS) $(OBJS) -o main

clean:
	rm -f main
	rm -f $(OBJS)

input.o: input.c
main.o: main.c
map.o: map.c
my_math.o: my_math.c
object.o: object.c
octree.o: octree.c
texture.o: texture.c
world.o: world.c
