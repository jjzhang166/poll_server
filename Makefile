.PHONY:all
all:poll_client poll_server
poll_client:poll_client.c
	gcc -o $@ $^
poll_server:poll_server.c
	gcc -o $@ $^
.PHONY:clean
clean:
	rm -f poll_client poll_server
