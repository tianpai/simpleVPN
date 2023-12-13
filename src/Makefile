CC = gcc
SHOW_WARNINGS = -Wall
SSL_LIB = -lssl -lcrypto
ALL_FLAGS =  $(SHOW_WARNINGS) $(SSL_LIB)

# gcc client -c client.c ..... -lssl -lcrypto    works

# Common to both client and server
COMMON_OBJ = packet.o\
			 utils.o\
			 encrypt.o\
			 encap.o\
			 decap.o\
			 host_info.o

CLIENT_OBJ = client.o $(COMMON_OBJ)
SERVER_OBJ = server.o $(COMMON_OBJ)
DESTINATION_OBJ = destination.o $(COMMON_OBJ)

# all target builds both client, server, and destination
all: vpn_client vpn_server vpn_destination

# client: dependes on CLIENT_OBJ
vpn_client: $(CLIENT_OBJ)
	# link all the object files to single binary named vpn_client
	$(CC) -o vpn_client $(CLIENT_OBJ) $(SSL_LIB)

vpn_server: $(SERVER_OBJ)
 	# link all the object files to single binary named vpn_server
	$(CC) -o vpn_server $(SERVER_OBJ) $(SSL_LIB)

vpn_destination: $(DESTINATION_OBJ)
	# link all the object files to single binary named vpn_destination
	$(CC) -o vpn_destination $(DESTINATION_OBJ) $(SSL_LIB)

# rule for generating object files from .c files
# %.o: %.c means "for all" .o files,
# use the corresponding .c file to generate the .o file
%.o: %.c
	# compile the .c file to generate the .o file
	# $< is the first prerequisite (the .c file)
	# $@ is the target (the .o file)
	$(CC) -c -o $@ $< $(ALL_FLAGS)

clean:
	rm -f *.o vpn_client vpn_server vpn_destination
