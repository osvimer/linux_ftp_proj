all: make_server make_client
make_server: server
	cd server; make
make_client: client
	cd client; make
clean_server:
	cd server; make clean
clean_client:
	cd client; make clean
.PHONY: all rebuild clean make_server make_client clean_server clean_client
clean: clean_server clean_client
rebuild: clean all

