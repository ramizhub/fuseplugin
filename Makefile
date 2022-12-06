FUSE: fuseplugin.c
	gcc fuseplugin.c `pkg-config fuse --cflags --libs`