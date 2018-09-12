# Makefile for lws
project = lws

SRCS += src/sock_server.c

all: $(project)

$(project):
	@gcc $(SRCS)  -o $(project)
	@echo "Build" $(project)
	
clean:
	-@rm -f $(project)