# fork-and-pipe

The unix functions fork and pipe can be found in a lot of unix sources...

Fork() creates a duplicate new process. It is a strange beast but works very well. If the fork fails it returns less than zero. What you are supposed to do at this point is difficult the say, but since no duplicate child process is created, and if your program execution depends on the child doing some action, then there seems little else to do but exit(1). But in testing it seldom fails...

From the code perspecitive the call to fork returns twice. I am not sure of the order of the return, and whether it is always the same order, but one return is zero. This is the parent process and can continue on its way. The other return is a PID of the child process. It can do its own special extra actions, and exit, or continue... I think both processes cease if one or the other does a hard exit(n), but not sure...

The function pipe(int *pipefd[2]) creates a pipe, a unidirectional data channel that can be used for interprocess communication. The array pipefd is used to return two file descriptors referring to the ends of the pipe. pipefd[0] refers to the read end of the pipe. pipefd[1] refers to the write end of the pipe. Data written to the write end of the pipe is buffered until it is read from the read end of the pipe.

There is a similiar function in `<fcntl.h`, namely `int pipe2(int pipefd[2], int flags);`, but that is not dealt with here.

Since fork() creates a new process then such a pipe is often used to pass data, information, instructions between the two processes - interprocess communication (IPC).

A simple (untested) unix prototype of such a pipe/fork app is as follows...

```
#include <unistd.h>
#include <sys/types.h>
#include <errno.h>
#include <stdio.h>
#include <sys/wait.h>
#include <stdlib.h>
int var_glb; /* A global variable*/
int main(void)
{
    pid_t childPID;
    int var_lcl = 0;
    int pipefd[2];
    int rc;
    rc = pipe(pipefd);
    if (rc) {
        printf("failed to create a pipe!\n");
        return 1;
    }
    childPID = fork();
    if(childPID >= 0) { // fork was successful
        if(childPID == 0) { // child process
            var_lcl++;
            var_glb++;
            printf("\n Child Process :: var_lcl = [%d], var_glb[%d]\n", var_lcl, var_glb);
        } else { //Parent process
            var_lcl = 10;
            var_glb = 20;
            printf("\n Parent process :: var_lcl = [%d], var_glb[%d]\n", var_lcl, var_glb);
        }
    } else { // fork failed
        printf("\n Fork failed, quitting!!!!!!\n");
        return 1;
    }
    return 0;
}
```

This repo is about porting such an app to MS Windows (native). Windows does NOT have fork(), but a viable alternative is to use a pthread. And pipe has a different prototype in Windows - `int _pipe( int *pfds, unsigned int psize, int textmode );`. Note the leading underscore, and the additional pramaters. 

But this can be handled as a MACRO, like -

```
#ifdef WIN32
#define PIPE(a) _pipe(a, 1024, _O_BINARY)
#else
#define PIPE pipe
#endif
```

Here I have created a fork-and-pipe console app. Starting with a unix sample, I set about porting it to windows.

##### Dependencies

Naturally in windows has a dependency on finding installed [pthreads](https://www.sourceware.org/pthreads-win32/). While the source may appear old, circa 2012, it is well tested and very stable.

This project uses a [CMake](https://cmake.org/download/) for the configuration and generation of native build system, solution files.

And the source is kept in a GitHub repository, thus [git](https://git-scm.com/download/win) is required to clone that source.

##### Building

Commands to clone the source, and build the app 

```
$ cd \Projects # get to a projects root folder
$ git clone `<repository>` fork-and-pipe
$ cd fork-and-pipe\build
$ cmake .. [-DCMAKE_INSTALL_PREFIX=..\..\software] [-DCMAKE_PREFIX_PATH=\path\to\pthreads\include]
$ cmake --build . --config Release
```

Of course the CMake GUI can also be used to do the configuration, and generation. And if the build system chosen is MSVC, or some other build system with an IDE, it can be loaded to do the building.

Have FUN!

Geoff.

; eof - 20151014
