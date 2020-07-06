CC		:= gcc

CFLAGS	:= -Wall -Wextra -g3
LDFLAGS	:= -lssl -lcrypto
EXEC	:= pair
SRC		:= pair.c struct_data.c protocole.c
OBJ		:= $(SRC:.c=.o)

all: $(EXEC)

$(EXEC): $(OBJ)
	@$(CC) -o $@ $^ $(LDFLAGS)

%.o: %.c
	@$(CC) -o $@ -c $< $(CFLAGS)

re: mrproper $(EXEC)

clean:
	@rm -rf *.o

mrproper: clean
	@rm -rf $(EXEC)
