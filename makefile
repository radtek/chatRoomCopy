CC = gcc
CFLAGS += -g -I/root/clanguage/chatRoom/ \
		  -I/usr/include/mysql/ -L/usr/lib64/mysql/
LD = ld
LDFLAGS += -lpthread -lmysqlclient

target = server_daemon client_daemon

server_daemon_obj = server_daemon.o threadPool.o linklist.o server_proc.o 
client_daemon_obj = client_daemon.o client_send.o mysql.o client_file.o

client_daemon1_obj = client_daemon.o client_send.o mysql.o

server_daemon : $(server_daemon_obj)
	gcc $(CFLAGS) $^ -o $@ $(LDFLAGS) 

client_daemon : $(client_daemon_obj)
	gcc $(CFLAGS) $^ -o $@ $(LDFLAGS)

#client_daemon1 : $(client_daemon1_obj)
#	gcc $(CFLAGS) $^ -o $@ $(LDFLAGS)


clean:
	rm  -r $(target) *.o

