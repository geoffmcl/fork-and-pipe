#include <sys/types.h>
#ifdef _MSC_VER
#include <Windows.h>
#include <io.h>
#include <process.h>
#include <fcntl.h>
#include <pthread.h>
#define DOSLIKE
#else
#include <unistd.h>
#endif
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

#ifdef _MSC_VER
typedef int pid_t;
#endif

#ifdef DOSLIKE
static int textmode = _O_BINARY; // or _O_TEXT
#define PIPE(a) _pipe(a, 1024, textmode)
#else
#define PIPE pipe
#endif

/* Read characters from the pipe and echo them to stdout. */
#define MX_BUF 1024

void read_from_pipe (int file)
{
#ifdef _MSC_VER
    static char buff[MX_BUF+4];
    int rc, cnt = 0;
    printf("Child: Read from pipe %d\n", file);
    rc = read(file,buff,MX_BUF);
    if (rc > 0) {
        while (rc > 0) {
            buff[rc] = 0;
            cnt++;
            printf("Child Read:%d: '%s' (%d)\n", cnt, buff, rc);
            rc = read(file,buff,MX_BUF);
        }
    } else if (rc == 0) {
        printf("Returned EOF\n");
    } else {
        printf("read error!\n");
    }
#else // !_MSC_VER
    FILE *stream;
    int c;
    stream = fdopen (file, "r");
    while ((c = fgetc (stream)) != EOF)
        putchar (c);
    fclose (stream);
#endif
}

 /* Write some random text to the pipe. */

 void
 write_to_pipe (int file)
{
#ifdef _MSC_VER
    static const char *m1 = "hello, world!\n";
    static const char *m2 = "goodbye, world!\n";
    int len = (int)strlen(m1);
    int rc, total = 0;
    int bad = 1;
    printf("Parent: Write to pipe %d\n", file );
    rc = write(file,m1,len);
    if (rc == len) {
        total += len;
        len = (int)strlen(m2);
        rc = write(file,m2,len);
        if (rc == len) {
            total += len;
            printf("Written 2 messages, %d len, to pipe %d\n", total, file);
            bad = 0;
        }
    }
    if (bad) {
        perror("write pipe");
        printf("Failed to write to pipe %d\n", file );
        //close(file);
    }
#else
   FILE *stream;
   stream = fdopen (file, "w");
   fprintf (stream, "hello, world!\n");
   fprintf (stream, "goodbye, world!\n");
   fclose (stream);
#endif
}

#ifdef _MSC_VER
static void *child_proc(void *context)
{
    int *mypipe = (int *)context;

	pthread_detach ( pthread_self() );

    read_from_pipe (mypipe[0]); // read descriptor

    printf("Child: thread ending... return 0\n");

    // close(mypipe[1]);  // close write descriptor
    // close(mypipe[0]);
    return 0;
}
#endif

// MSVC DEBUG: PATH=F:\Projects\software\bin;%PATH% for pthreadVC2.dll
int main (int argc, char **argv)
{
    pid_t pid;
    int mypipe[2];

    printf("Running '%s'...\n", argv[0]);
    printf("Create the pipe...\n");
    if (PIPE(mypipe))
    {
        fprintf (stderr, "Pipe failed.\n");
        return EXIT_FAILURE;
    }

    /* Create the child process. */
#ifdef _MSC_VER
    pthread_t th;
    printf("Create the thread...\n");
	pid = pthread_create ( &th, NULL, &child_proc, mypipe );
    if (pid) {
        /* The thread failed. */
        fprintf (stderr, "Thread failed.\n");
        return EXIT_FAILURE;
    }

    /* This is the parent process. */
    write_to_pipe (mypipe[1]);  // write descriptor
    printf("Parent: Sleep 1 sec...\n");
    Sleep(1000);
    printf("Parent: Awake... close %d... Sleep 1 sec...\n", mypipe[1]);
    /*  Close other end first. */
    //close (mypipe[0]);  // close read descriptor
    close (mypipe[1]);  // close write descriptor
    Sleep(1000);
    printf("Parent: Exit %d...\n",EXIT_SUCCESS);
    return EXIT_SUCCESS;
#else
    printf("Fork the process...\n");
    pid = fork ();
    if (pid == (pid_t) 0)
    {
       /* This is the child process.
          Close other end first. */
       close (mypipe[1]);
       read_from_pipe (mypipe[0]);
       return EXIT_SUCCESS;
   }
   else if (pid < (pid_t) 0)
   {
       /* The fork failed. */
       fprintf (stderr, "Fork failed.\n");
       return EXIT_FAILURE;
   }
   else
   {
       /* This is the parent process.
          Close other end first. */
       close (mypipe[0]);
       write_to_pipe (mypipe[1]);
       return EXIT_SUCCESS;
   }
#endif // _MSC_VER
   return 0;
}

/* eof */
