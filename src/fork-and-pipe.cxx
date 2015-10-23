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
#include <string>

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
    int cnt = 0;
    printf("Child: Read from pipe %d\n", file);
    stream = fdopen (file, "r");
    while ((c = fgetc (stream)) != EOF) {
        putchar (c);
        cnt++;
    }
    fclose (stream);
    printf("Child: Read %d bytes\n", cnt);
#endif
}

 /* Write some random text to the pipe. */

 void
 write_to_pipe (int file)
{
    printf("Parent: Write to pipe %d\n", file );
#ifdef _MSC_VER
    static const char *m1 = "hello, world!\n";
    static const char *m2 = "goodbye, world!\n";
    int len = (int)strlen(m1);
    int rc, total = 0;
    int bad = 1;
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
    printf("Pipe read %d, write %d\n", mypipe[0], mypipe[1]);

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
       printf("child returns to os...\n");
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
       printf("parent returns to os...\n");
       return EXIT_SUCCESS;
   }
#endif // _MSC_VER
   return 0;
}

#ifdef _MSC_VER
/////////////////////////////////////////////////////////////////////
// methods to execute a process
// from : http://www.experts-exchange.com/articles/1595/Execute-a-Program-with-C.html
// simple: system( cmd );
// OR
int execute_a_process( const char *proc, const char *params)
{
    int nRet = (int)ShellExecute( 0,"open",proc,params,0,SW_SHOWNORMAL);
    if ( nRet <= 32 ) {
        DWORD dw= GetLastError(); 
        char szMsg[250];
        FormatMessage(
            FORMAT_MESSAGE_FROM_SYSTEM,	
            0, dw, 0,
            szMsg, sizeof(szMsg),
            NULL 
        );
        // AMessageBox( szMsg, "Error launching Calculator" );
        fprintf(stderr,"Error launching '%s': %s\n", proc, szMsg);
        nRet = 1;
    } else {
        nRet = 0;
    }
    return nRet;
}
int use_create_process( const char *proc, const char *params)
{
    PROCESS_INFORMATION ePI = {0};
    STARTUPINFO         rSI = {0};

    rSI.cb          = sizeof( rSI );
    rSI.dwFlags     = STARTF_USESHOWWINDOW;
    rSI.wShowWindow = SW_SHOWNORMAL;  // or SW_HIDE or SW_MINIMIZED

    BOOL fRet= CreateProcess(
        proc,    // "c:\\windows\\notepad.exe",  // program name 
        (LPSTR)params,  //  " c:\\temp\\report.txt",     // ...and parameters
        NULL, NULL,  // security stuff (use defaults)
        TRUE,        // inherit handles (not important here)
        0,           // don't need to set priority or other flags
        NULL,        // use default Environment vars
        NULL,        // don't set current directory
        &rSI,        // where we set up the ShowWIndow setting
        &ePI         // gets populated with handle info
    );
    if (!fRet) {
        DWORD dw= GetLastError(); 
        char szMsg[250];
        FormatMessage(
            FORMAT_MESSAGE_FROM_SYSTEM,	
            0, dw, 0,
            szMsg, sizeof(szMsg),
            NULL 
        );
        fprintf(stderr,"Error launching '%s': %s\n", proc, szMsg);
        return 1;
    }
    return 0;

}

////////////////////////////////////////////////////////////////////////
// execute process, creating pipes for stdout, stderr redirection
// wait for process to exit...

int ReadFromPipeNoWait( HANDLE hPipe, char* pDest, int nMax )
{
    DWORD nBytesRead = 0;
    DWORD nAvailBytes;
    char cTmp;
    memset( pDest, 0, nMax );
    // -- check for something in the pipe
    PeekNamedPipe( hPipe, &cTmp, 1, NULL, &nAvailBytes, NULL );
    if ( nAvailBytes == 0 ) {
         return( nBytesRead );
    }
    // OK, something there... read it
    ReadFile( hPipe, pDest, nMax-1, &nBytesRead, NULL); 
    return( nBytesRead );
}

BOOL ExecAndProcessOutput(LPCSTR szCmd, LPCSTR szParms  )
{
    SECURITY_ATTRIBUTES rSA  =    {0};
    rSA.nLength        = sizeof(SECURITY_ATTRIBUTES);
    rSA.bInheritHandle = TRUE;

    HANDLE hReadPipe, hWritePipe;
    CreatePipe( &hReadPipe, &hWritePipe, &rSA, 25000 );

    PROCESS_INFORMATION rPI= {0};
    STARTUPINFO         rSI= {0};
    rSI.cb          = sizeof(rSI);
    rSI.dwFlags     = STARTF_USESHOWWINDOW |STARTF_USESTDHANDLES;
    rSI.wShowWindow = SW_HIDE;  // or SW_SHOWNORMAL or SW_MINIMIZE
    rSI.hStdOutput  = hWritePipe;
    rSI.hStdError   = hWritePipe;

    std::string sCmd;
    sCmd = "\"";
    sCmd += szCmd;
    sCmd += "\" ";
    sCmd += szParms;

    BOOL fRet = CreateProcess(NULL,(LPSTR)(LPCSTR)sCmd.c_str(), NULL,
              NULL,TRUE,0,0,0, &rSI, &rPI );
    if ( !fRet ) {
        DWORD dw= GetLastError(); 
        char szMsg[250];
        FormatMessage(
            FORMAT_MESSAGE_FROM_SYSTEM,	
            0, dw, 0,
            szMsg, sizeof(szMsg),
            NULL 
        );
        fprintf(stderr,"Failed CreateProcess '%s': %s\n", sCmd.c_str(), szMsg);
        return( FALSE );
    }
   //------------------------- and process its stdout every 100 ms
   char dest[1000];
   std::string sProgress = "";
   DWORD dwRetFromWait= WAIT_TIMEOUT;
   while ( dwRetFromWait != WAIT_OBJECT_0 ) {
        dwRetFromWait = WaitForSingleObject( rPI.hProcess, 100 );
        if ( dwRetFromWait == WAIT_ABANDONED ) {  // crash?
            break;
        }
        //--- else (WAIT_OBJECT_0 or WAIT_TIMEOUT) process the pipe data
        while ( ReadFromPipeNoWait( hReadPipe, dest, sizeof(dest) ) > 0 ) {
            // ------------------ Do something with the output.
            // ------------------ Eg, insert at the end of an edit box
            //int iLen= gpEditBox->GetWindowTextLength();
            //gpEditBox->SetSel(    iLen, iLen);
            //gpEditBox->ReplaceSel( dest );
        }
    }
    CloseHandle( hReadPipe  );
    CloseHandle( hWritePipe );
    CloseHandle( rPI.hThread); 
    CloseHandle( rPI.hProcess);
    // MessageBox("All done!");
    return TRUE;
}

#endif // _MSC_VER

/* eof */
