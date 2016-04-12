all: make_server make_client
make_server:
	cd server; make
make_client:
	cd client; make
clean_server:
	cd server; make clean
clean_client:
	cd client; make clean
.PHONY: all rebuild clean 
clean: clean_server clean_client
rebuild: clean all
