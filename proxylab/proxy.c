#include <stdio.h>
#include "csapp.h"

/* Recommended max cache and object sizes */
#define MAX_CACHE_SIZE 1049000
#define MAX_OBJECT_SIZE 102400
#define MAX_CACHE 10

/* You won't lose style points for including this long line in your code */
static const char *user_agent_hdr = "User-Agent: Mozilla/5.0 (X11; Linux x86_64; rv:10.0.3) Gecko/20120305 Firefox/10.0.3\r\n";

void doit(int clientfd);
int parse_uri(char *uri,char *hostname,char *path,char *port,char *request_head);
void request_headers(rio_t *rp,int fd);
void return_content(int serverfd, int clientfd,char *url);
void *thread(void *vargp);
int maxlrucache();

struct RWLOCK_T
{
    sem_t mutex;
    sem_t w;
    int readcnt;
};

struct Cache
{
    int lrunum;
    char url[MAXLINE];
    char content[MAX_OBJECT_SIZE];
    int contentsize;
};

struct Cache cache[MAX_CACHE];
struct RWLOCK_T* rw;

void rwlock_init();
char *reader(char *url,int *size);
void writer(char *buf,char *url,int size);


int main(int argc,char **argv)
{
    int listenfd;
    int *connfd;
    char hostname[MAXLINE],port[MAXLINE];
    socklen_t clientlen;
    struct sockaddr_storage clientaddr;
    pthread_t tid;

    signal(SIGPIPE,SIG_IGN);
    if(argc!=2)
    {
        fprintf(stderr,"usage: %s <port>\n",argv[0]);
        exit(1);
    }

    rw=Malloc(sizeof(struct RWLOCK_T));
    rwlock_init();

    listenfd=Open_listenfd(argv[1]);
    while(1)
    {
        clientlen=sizeof(clientaddr);
        connfd=Malloc(sizeof(int));
        *connfd=Accept(listenfd,(SA *)&clientaddr,&clientlen);
        Getnameinfo((SA *)&clientaddr,clientlen,hostname,MAXLINE,port,MAXLINE,0);
        printf("Accepted connection from (%s, %s)\n",hostname,port);
        Pthread_create(&tid,NULL,thread,connfd);
    }
}

void *thread(void *vargp)
{
    int connfd=*((int *)vargp);
    Pthread_detach(pthread_self());
    Free(vargp);
    doit(connfd);
    Close(connfd);
    return NULL;
}

void doit(int clientfd)
{

    char buf[MAXLINE],method[MAXLINE],uri[MAXLINE],version[MAXLINE];
    char hostname[MAXLINE],path[MAXLINE],port[MAXLINE],request_head[MAXLINE];
    int serverfd,size;
    rio_t rio;

    Rio_readinitb(&rio,clientfd);
    Rio_readlineb(&rio,buf,MAXLINE);
    sscanf(buf,"%s %s %s",method,uri,version);
    if(strcasecmp(method,"GET"))
    {
        printf("Not implemented");
        return;
    }

    char *content=reader(uri,&size);
    if(content!=NULL)
    {
        Rio_writen(clientfd,content,size);
        free(content);
    }
    else
    {
        parse_uri(uri,hostname,path,port,request_head);
        serverfd=Open_clientfd(hostname,port);
        Rio_writen(serverfd,request_head,strlen(request_head));
        request_headers(&rio,serverfd);
        return_content(serverfd,clientfd,uri);
        Close(serverfd);
    }
}

int parse_uri(char *uri,char *hostname,char *path,char *port,char *request_head)
{
    /* default case */
    sprintf(port,"80");
    char *end,*bp;
    char *tail=uri+strlen(uri);
    char *bg=strstr(uri,"//");
    if(bg!=NULL)
        bg=bg+2;
    else
        bg=uri;
    end=bg;
    while(*end!='/'&&*end!=':')
        end++;
    strncpy(hostname,bg,end-bg);
    bp=end+1;
    /* parse the port */
    if(*end==':')
    {
        bp=strstr(bg,"/");
        end++;
        strncpy(port,end,bp-end);
        end=bp;
    }
    strncpy(path,end,(int)(tail-end)+1);
    sprintf(request_head,"GET %s HTTP/1.0\r\nHost: %s\r\n",path,hostname);
    return 1;
}

/* read the request from client and rearrange them */
void request_headers(rio_t *rp,int fd)
{
    char buf[MAXLINE];

    sprintf(buf, "%s", user_agent_hdr);
    Rio_writen(fd, buf, strlen(buf));
    sprintf(buf, "Connection: close\r\n");
    Rio_writen(fd, buf, strlen(buf));
    sprintf(buf, "Proxy-Connection: close\r\n");
    Rio_writen(fd, buf, strlen(buf));

    /* other headers */
    for(Rio_readlineb(rp,buf,MAXLINE);strcmp(buf,"\r\n");Rio_readlineb(rp,buf,MAXLINE))
    {
        if(strncmp("Host",buf,4)==0||strncmp("User-Agent",buf,10)==0||
        strncmp("Connection",buf,10)==0||strncmp("Proxy-Connection",buf,16)==0)
            continue;
        printf("%s",buf);
        Rio_writen(fd,buf,strlen(buf));
    }
    Rio_writen(fd,buf,strlen(buf));
    return;
}

/* return the content of server */
void return_content(int serverfd, int clientfd,char *uri)
{
    size_t n,size=0;
    char buf[MAXLINE],content[MAX_OBJECT_SIZE];
    rio_t sio;

    Rio_readinitb(&sio,serverfd);
    while((n=Rio_readnb(&sio,buf,MAXLINE))>0)
    {
        Rio_writen(clientfd,buf,n);
        if(n+size<=MAX_OBJECT_SIZE)
            memcpy(content+size,buf,n);
        size+=n;
    }
    if(size<=MAX_OBJECT_SIZE)
        writer(content,uri,size);
}

/* cache with reader-writer model*/
void rwlock_init()
{
    rw->readcnt=0;
    sem_init(&rw->w,0,1);
    sem_init(&rw->mutex,0,1);
}

void writer(char *buf,char *url,int size)
{
    P(&rw->w);
    int index;
    for(index=0;index<MAX_CACHE;index++)
        if(cache[index].lrunum==0)
            break;

    /* use the LRU strategy */
    if(index==MAX_CACHE)
    {
        int minlru=cache[0].lrunum;

        index=0;
        for(int i=0;i<MAX_CACHE;i++)
            if(cache[i].lrunum<minlru)
            {
                minlru=cache[i].lrunum;
                index=i;
            }
    }

    cache[index].lrunum=maxlrucache()+1;
    strcpy(cache[index].url,url);
    memcpy(cache[index].content,buf,size);
    cache[index].contentsize=size;
    V(&rw->w);

    return;

}

/* use sem to make sure it goes right */
char *reader(char *url,int *size)
{
    P(&rw->mutex);
    rw->readcnt++;
    if(rw->readcnt==1)
        P(&rw->w);
    V(&rw->mutex);

    char *content = NULL;
    for(int i = 0;i < MAX_CACHE;i++)
        if(strcmp(url,cache[i].url)==0)
        {
            content=(char *)Malloc(cache[i].contentsize);
            memcpy(content,cache[i].content,cache[i].contentsize);
            int maxlru=maxlrucache();
            cache[i].lrunum=maxlru+1;
            *size=cache[i].contentsize;
            break;
        }

    P(&rw->mutex);
    rw->readcnt--;
    if(rw->readcnt == 0)
        V(&rw->w);
    V(&rw->mutex);
    return content;
}

/* find the maxlru of cache*/
int maxlrucache()
{
    int i;
    int max=0;
    for(i=0;i<MAX_CACHE;i++)
        if(cache[i].lrunum>max)
            max=cache[i].lrunum;
    return max;
}