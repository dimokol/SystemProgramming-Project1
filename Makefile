# compiler
CC = gcc

# Compile options. Το -I<dir> λέει στον compiler να αναζητήσει εκεί include files
CFLAGS = -Wall -g
# Αρχεία .o
OBJS = mysh.o main.o

# Το εκτελέσιμο πρόγραμμα
EXEC = mysh

# Παράμετροι για δοκιμαστική εκτέλεση
ARGS =

$(EXEC): $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $(EXEC)

clean:
	rm -f $(EXEC) $(OBJS)

run: $(EXEC)
	./$(EXEC) $(ARGS)

valgrind: $(EXEC)
	valgrind ./$(EXEC) $(ARGS)