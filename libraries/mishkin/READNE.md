# Mishkin Allocator

Mishkin, is the allocator that I wrote to use around the compiler and more C related projects. It is a simple arena allocator.

## Getting started

To start coding you need to bootstrap the build script, I am currently using no build system since I haven't coded one yet. To build the build script you can run the following command (Unix-only).

```sh
$ gcc build.c -Wall -Wextra -pedantic
```

Then you can just run the following command to build everything:

```sh
$ ./a.out
```

If you want to build the tests you can run the following command:

```sh
$ ./a.out --test
```
