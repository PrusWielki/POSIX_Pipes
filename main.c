//I declare that this piece of work which is the basis for recognition of
//achieving learning outcomes in the OPS course was completed on my own.
//Patryk Prusak 305794

#define _GNU_SOURCE
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <time.h>
#include <limits.h>
#define ERR(source) (fprintf(stderr,"%s:%d\n",__FILE__,__LINE__),\
                     perror(source),kill(0,SIGKILL),\
		     exit(EXIT_FAILURE))
//#define DBG
//#define PIPE_BUF 20
volatile sig_atomic_t last_signal = 0;
int sethandler( void (*f)(int), int sigNo)
{
    struct sigaction act;
    memset(&act, 0, sizeof(struct sigaction));
    act.sa_handler = f;
    if (-1==sigaction(sigNo, &act, NULL))
        return -1;
    return 0;
}

void sig_handler(int sig)
{
    last_signal = sig;
}



void argument_check(int t,int n,int r,int b)
{
    if(t<50||t>500)
        ERR("Wrong t argument ");
    else if(n<3||n>30)
        ERR("Wrong n argument ");
    else if(r<0||r>100)
        ERR("Wrong r argument ");
    else if(b<1||b>PIPE_BUF-6)
        ERR("Wrong b argument");
    else
        return;
}
void get_arguments(int *t,int *n,int *r,int *b,char **argv)
{
    *t=atoi(argv[1]);
    *n=atoi(argv[2]);
    *r=atoi(argv[3]);
    *b=atoi(argv[4]);
}
int main(int argc, char**argv)
{
    srand(time(NULL));
    //char sent1[1]="b",sent2[1]="b",sent3,sent4;
    //char s[1]="a";
    char inj[]="injected";
    if(argc!=5)
        ERR("There should be 4 arguments");
    pid_t m1, m2, c1,c2;
    int t,n,r,b;
    struct timespec tt= {0,0};
    struct timespec t2= {0,t*1000000};

//    if(sethandler(sigpipe_handler,SIGPIPE))ERR("sethandler");
    //signal(SIGPIPE,SIGABRT);
    int pipe_mp[2];
    if(pipe(pipe_mp))ERR("pipe mp");
    get_arguments(&t,&n,&r,&b,argv);
    argument_check(t,n,r,b);
    char *buf1=malloc(sizeof(char)*PIPE_BUF);
    char *buf2=malloc(sizeof(char)*PIPE_BUF);
    char *rec1=malloc(sizeof(char)*PIPE_BUF);
    char *rec2=malloc(sizeof(char)*PIPE_BUF);
    m1=fork();
    if(m1==0)
    {
        int pipe_c1m1[2];

        if(pipe(pipe_c1m1))ERR("pipe mp");

        c1=fork();
        if(c1==0)
        {
            if(sethandler(sig_handler,SIGINT))
                ERR("c1 sethandler");



            if(close(pipe_c1m1[0]))ERR("close pipe mp1");
#ifdef DBG
            printf("c1 -%ld\n",(long)getpid());
#endif
            srand(getpid()*time(NULL));
            nanosleep(&tt,NULL);

            int buffer_size=rand()%(PIPE_BUF-6-b+1)+b;
            char *buff_c1=malloc(sizeof(char)*PIPE_BUF-6);

            for(int i=0; i<n; i++)
            {
                if(last_signal==SIGINT)
                {
                    if(close(pipe_c1m1[1]))ERR("close pipe mp1");
                    exit(0);
                }
                nanosleep(&t2,NULL);
                for(int j=0; j<buffer_size-1; j++)
                {
                    buff_c1[j]='a' + (rand() % 26);
                }
                buff_c1[buffer_size-1]='\0';
                TEMP_FAILURE_RETRY(write(pipe_c1m1[1],buff_c1,buffer_size));
#ifdef DBG
                printf("c1 : sent:%s\n",buff_c1);
#endif
                buffer_size=rand()%(PIPE_BUF-6-b+1)+b;

            }
            if(close(pipe_c1m1[1]))ERR("close pipe mp1");
#ifdef DBG
            printf("c1  -%ld\n",(long)getpid());
#endif
            free(buff_c1);
            exit(0);

        }
        else if(c1>0)
        {
            if(sethandler(SIG_IGN,SIGINT))
                ERR("c1 sethandler");
            if(close(pipe_mp[0]))ERR("close pipe mp1");
            if(close(pipe_c1m1[1]))ERR("close pipe mp1");
#ifdef DBG
            printf("m1 -%ld\n",(long)getpid());
#endif
            // waitpid(c1, NULL, 0);
            for(int i=0; i<n; i++)
            {
                if(TEMP_FAILURE_RETRY(read(pipe_c1m1[0],buf1,PIPE_BUF))==0)
                    exit(0);
#ifdef DBG
                printf("m1 received:%s -%ld\n",buf1,(long)getpid());
#endif
                double rand_val=(double)rand()/RAND_MAX;
                if(rand_val<=(double)r/100)
                {
                    strcat(buf1,inj);

                }
                TEMP_FAILURE_RETRY(write(pipe_mp[1],buf1,PIPE_BUF));
            }
            if(close(pipe_mp[1]))ERR("close pipe mp1");
            exit(0);
        }
        else
            ERR("fork for c1");
    }
    else if(m1>0)
    {
        m2=fork();
        if(m2==0)
        {
            int pipe_c2m2[2];

            if(pipe(pipe_c2m2))ERR("pipe mp");

            c2=fork();
            if(c2==0)
            {
                if(sethandler(sig_handler,SIGINT))
                    ERR("c1 sethandler");
                if(close(pipe_c2m2[0]))ERR("close pipe mp1");
#ifdef DBG
                printf("c2 - %ld\n",(long)getpid());
#endif
                nanosleep(&tt,NULL);
                srand(getpid()*time(NULL));
                int buffer_size=rand()%(PIPE_BUF-6-b+1)+b;
                char *buff_c2=malloc(sizeof(char)*PIPE_BUF-6);
                for(int i=0; i<n; i++)
                {
                    if(last_signal==SIGINT)
                    {
#ifdef DBG
                        printf("EXITING\n");
#endif // DBG
                        if(close(pipe_c2m2[1]))ERR("close pipe mp1");
                        exit(0);
                    }
                    nanosleep(&t2,NULL);
                    for(int j=0; j<buffer_size-1; j++)
                    {
                        buff_c2[j]='a' + (rand() % 26);
                    }
                    buff_c2[buffer_size-1]='\0';
                    TEMP_FAILURE_RETRY(write(pipe_c2m2[1],buff_c2,buffer_size));
#ifdef DBG
                    printf("c2: sent:%s\n",buff_c2);
#endif
                    buffer_size=rand()%(PIPE_BUF-6-b+1)+b;
                }
                if(close(pipe_c2m2[1]))ERR("close pipe mp1");
#ifdef DBG
                printf("c2  -%ld\n",(long)getpid());
#endif
                free(buff_c2);
                exit(0);

            }
            else if(c2>0)
            {
                if(sethandler(SIG_IGN,SIGINT))
                    ERR("c1 sethandler");
                if(close(pipe_mp[0]))ERR("close pipe mp1");
                if(close(pipe_c2m2[1]))ERR("close pipe mp1");
#ifdef DBG
                printf("m2 -%ld\n",(long)getpid());
#endif
                // waitpid(c2, NULL, 0);
                for(int i=0; i<n; i++)
                {
                    if(TEMP_FAILURE_RETRY(read(pipe_c2m2[0],buf2,PIPE_BUF))==0)
                        exit(0);
#ifdef DBG
                    printf("m2 received:%s -%ld\n",buf2,(long)getpid());
#endif
                    double rand_val=(double)rand()/RAND_MAX;
                    if(rand_val<=(double)r/100)
                    {
                        strcat(buf2,inj);

                    }
                    TEMP_FAILURE_RETRY(write(pipe_mp[1],buf2,PIPE_BUF));
                }
                if(close(pipe_mp[1]))ERR("close pipe mp1");
                exit(0);

            }
            else
                ERR("fork for c2");

        }
        else if(m2>0)
        {
            if(sethandler(SIG_IGN,SIGINT))
                ERR("c1 sethandler");

            if(close(pipe_mp[1]))ERR("close pipe mp1");
#ifdef DBG
            printf("p-%ld\n",(long)getpid());
#endif
            //waitpid(m1, NULL, 0);
            for(int i=0; i<n; i++)
            {

                if(TEMP_FAILURE_RETRY(read(pipe_mp[0],rec1,PIPE_BUF))==0)
                    exit(0);
                //printf("p received:%s  %s -%ld\n",rec1,rec2,(long)getpid());
                printf("%d: %ld: %s\n",i,strlen(rec1),rec1);
                if(TEMP_FAILURE_RETRY(read(pipe_mp[0],rec2,PIPE_BUF))==0)
                    exit(0);
                //waitpid(m2, NULL, 0);
                printf("%d: %ld: %s\n",i,strlen(rec2),rec2);
            }
        }
        else
            ERR("fork");

    }
    else
    {
        ERR("fork");
    }
    free(buf1);
    free(buf2);
    free(rec1);
    free(rec2);
    return 0;
}
