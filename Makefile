CC=g++
# set compiler options:
#	-g = debugging
#	-O# = optimisation
COPT=-g
default:
	$(CC) $(COPT) ./src/tx_ethparse1_ck.cpp -o ./bin/tx_ethparse1_ck $(LIBS) $(CFLAGS) -Wall
#	$(CC) $(COPT) ./src/tx_ethparse_RP.cpp -o ./bin/tx_ethparse1_ck $(LIBS) $(CFLAGS) -Wall
clean:
	rm ./bin/tx_ethparse1_ck
