# __FUSE plugin__
A simple FUSE plugin written in C.

Description
===========
This plugin is a special file system that works with input bytes, dividing them into files of a fixed size and sending them to the target directory.

Requirements
===========
The plugin uses an external [libfuse library](https://github.com/libfuse/libfuse).

I used these commands for my Ubuntu machine:
```
sudo apt-get update
sudo apt-get install libfuse-dev
```

Compiling
===========
There is a makefile in the repository. To compile, you need to open a terminal, go to the program folder and enter the following:
```
man@man:~/folder123$ make
```

Usage
===========
To use this plugin, you must have two directories:

1) Fuse is the directory where you should create the file for redirection (it is advised to have 1 file)
2) The target directory where files of fixed size and date in the name will appear

The target directory will look something like this:

man@man:~/project/targetfuse$ ls -l

total 16

-rw-r--r-- 1 man man 10 Dec 6 18:16 2022-12-06--18:16:52

-rw-r--r-- 1 man man 10 Dec 6 18:16 2022-12-06--18:16:54

-rw-r--r-- 1 man man 10 Dec 6 18:16 2022-12-06--18:16:56

-rw-r--r-- 1 man man 10 Dec 6 18:16 2022-12-06--18:16:58

Example
===========

targetfuse - Your target directory
mainfuse   - Your fuse based directory
```
man@man:~/project$ make
man@man:~/project$ ./a.out mainfuse
put target directory
>>/home/project/targetfuse
man@man:~/project$ cd mainfuse
man@man:~/project/mainfuse$ echo HelloWorld >> mainfile
man@man:~/project/mainfuse$ echo Bye >> mainfile
man@man:~/project/mainfuse$ echo HeyStop >> mainfile
man@man:~/project/mainfuse$ cd ..
man@man:~/project$ cd targetfuse
man@man:~/project/targetfuse$ ls -l

total 8
-rw-r--r-- 1 man man 10 Dec  6 18:34 2022-12-06--18:34:30
-rw-r--r-- 1 man man 10 Dec  6 18:34 2022-12-06--18:34:44

man@man:~/project/targetfuse$ cat 2022-12-06--18:34:30
HelloWorld
man@man:~/project/targetfuse$ cat 2022-12-06--18:34:44
ByeHeyStop
man@man:~/project/targetfuse$ cd ..
man@man:~/project$
```

If you no longer need a mount point or you are recompiling, you can run plugin so with the following command:
```
man@man:~/project$ fusermount -u mainfuse
```

If you need to debug the code or see the outputs of, for example, the printf() function, then you can run the plugin like this:
```
man@man:~/project$ make
man@man:~/project$ ./a.out -f mainfuse
put target directory
>>/home/project/targetfuse
```
And after this open new terminal and do all write/read operations you need. The outputs of the debug function will be displayed in the first terminal. After this launch, you will not have to unmount the mainfuse directory again.