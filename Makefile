default:
	gcc oof.c -o p1 -lsqlite3 `pkg-config --cflags --libs gtk+-3.0` && ./p1
