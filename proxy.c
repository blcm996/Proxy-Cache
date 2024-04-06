
#include "csapp.h"
/* Recommended max cache and object sizes */
#define MAX_CACHE_SIZE 1049000
#define MAX_OBJECT_SIZE 102400
#define MAX_CACHE 10
/* You won't lose style points for including this long line in your code */
static const char *user_agent_hdr = "User-Agent: Mozilla/5.0 (X11; Linux x86_64; rv:10.0.3) Gecko/20120305 Firefox/10.0.3\r\n";
struct Uri
{
    /* data */
    char host[MAXLINE];
    char port[MAXLINE];
    char path[MAXLINE];
};
struct Header
{
    /* data */
    char name[MAXLINE];
    char value[MAXLINE];
};

struct rwlock_t{
    sem_t lock;
    sem_t writelock;
    int readers;
};

struct Cache
{
    int timecrp;
    char key[MAXLINE];
    char value[MAX_OBJECT_SIZE];
};

struct Cache cache[MAX_CACHE];
struct rwlock_t *rl;
int now_lru;

void thread(void*);
void doit(int);
int send_to_server(struct Uri*, struct Header*, int);
int read_cache(int, char*);
void write_cachw(char*, char*);
void rwlock_init();
void change_httpdata(rio_t*, struct Uri*, char*);
void parse_uri(char*, struct Uri*);



int main(int argc, char **argv)
{
    printf("%s", "heelok\n");
    int listen_fd, conn_fd;
    pthread_t pid;
    char hostname[MAXLINE], port[MAXLINE];
    socklen_t clientlen;
    struct sockaddr_storage clientaddr;

    if(argc != 2){
        fprintf(stderr, "usage: %s <port>\n", argv[0]);
        exit(1);
    }
    rl = (struct rwlock_t*)malloc(sizeof(struct rwlock_t));
    rwlock_init();
    //signal(SIGPIPE, sigpipe_handler);	//捕获SIGPIPE信号
    listen_fd = Open_listenfd(argv[1]);
    while (1) {
	    clientlen = sizeof(clientaddr);
	    conn_fd = Accept(listen_fd, (SA *)&clientaddr, &clientlen); //line:netp:tiny:accept
        Getnameinfo((SA *) &clientaddr, clientlen, hostname, MAXLINE, 
                    port, MAXLINE, 0);
        printf("Accepted connection from (%s, %s)\n", hostname, port);
	    Pthread_create(&pid, NULL, thread, (void*)&conn_fd);                                   //line:netp:tiny:doit
	    
    }
    
    return 0;
}

void thread(void* v){
    int fd = *(int*)v;
    
    Pthread_detach(pthread_self());
    doit(fd);
    Close(fd);
}

void rwlock_init(){
    now_lru = 0;
    rl->readers = 0;
    sem_init(&rl->lock, 0, 1);
    sem_init(&rl->writelock, 0, 1);
}

int read_cache(int fd, char* url){
    sem_wait(&rl->lock);
    if(rl->readers == 0)
        sem_wait(&rl->writelock);
    rl->readers++;
    sem_post(&rl->lock);
    //find cache
    int flag = 0;
    for(int i = 0; i < MAX_CACHE; i++){
        if(strcmp(url, cache[i].key) == 0){
            Rio_writen(fd, cache[i].value, strlen(cache[i].value));
            printf("proxy send %ld bytes to client\n", strlen(cache[i].value));
            cache[i].timecrp++;
            flag = 1;
            break;
        }
            
    }

    sem_wait(&rl->lock);
    rl->readers--;
    if(rl->readers == 0)
        sem_post(&rl->writelock);
    sem_post(&rl->lock);
    return flag;
}

void write_cache(char* buf, char* url){
    sem_wait(&rl->writelock);
    int idx;
    while (cache[now_lru].timecrp != 0)
    {
        cache[now_lru].timecrp = 0;
        now_lru = (now_lru + 1)%MAX_CACHE;
    }
    
    idx = now_lru;
    strcpy(cache[idx].key, url);
    strcpy(cache[idx].value, buf);
    sem_post(&rl->writelock);
    return;
}

void doit(int fd) {
    char buf[MAXLINE], uri[MAXLINE], method[MAXLINE], version[MAXLINE];
    char new_uri[MAXLINE], temp_uri[MAXLINE];
    rio_t rio, server_rio;
    struct Uri *request_line = (struct Uri *)malloc(sizeof(struct Uri));

    Rio_readinitb(&rio, fd);
    Rio_readlineb(&rio, buf, MAXLINE);

    sscanf(buf, "%s %s %s", method, uri, version);

    strcpy(temp_uri, uri);

    if(strcmp(method, "GET") != 0){
        fprintf(stderr, "Can't deal with %s method!\n", method);
        return;
    }
    
    if((read_cache(fd, temp_uri) != 0))
        return;

    parse_uri(uri, request_line);
    printf("uri is %s\n", uri);
    change_httpdata(&rio, request_line, new_uri);
    printf("new uri is %s\n", new_uri);
    printf("host is %s, port is %s\n", request_line->host, request_line->port);

    //Get info from cache instated
    int server_fd = Open_clientfd(request_line->host, request_line->port);
    size_t n;
 
    Rio_readinitb(&server_rio, server_fd);
    Rio_writen(server_fd, new_uri, strlen(new_uri));

    char cache[MAX_OBJECT_SIZE];
    
    long long sum = 0;
    while((n = Rio_readlineb(&server_rio, buf, MAXLINE)) != 0){
        Rio_writen(fd, buf, n);
        sum += n;
        strcat(cache, buf);
    }
    printf("proxy send %lld bytes to client\n", sum);
    if(sum < MAX_OBJECT_SIZE)
        write_cache(cache, temp_uri);
    Close(server_fd);
    return;
}


void parse_uri(char *url, struct Uri *linep) {
    char* hostpose = strstr(url, "//");
    if(hostpose == NULL){
        char* pathpose = strstr(url, "/");
        if(pathpose != NULL)
            strcpy(linep->path,pathpose);
        strcpy(linep->port, "80");
        return;
    }
    char* portpose = strstr(hostpose + 2, ":");
    if(portpose != NULL){
        int tmp; 
        sscanf(portpose + 1, "%d%s", &tmp, linep->path);
        sprintf(linep->port, "%d", tmp);
        *portpose = '\0';
    }
    else{
        char* pathpose = strstr(hostpose + 2, ":");
        if(pathpose != NULL){
            strcpy(linep->path, pathpose);
            strcpy(linep->port, "80");
            *pathpose = '\0';
        }
    }
    strcpy(linep->host, hostpose + 2);
    return;
}

void change_httpdata(rio_t* rio, struct Uri* uri, char* new_uri){
    static const char* con_hdr = "Connection: close\r\n";
    static const char* pcon_hdr = "Proxy-Connection: close\r\n";
    char buf[MAXLINE];
    char reqline[MAXLINE], host_hdr[MAXLINE], con_data[MAXLINE];
    sprintf(reqline, "Get %s HTTP/1.0\r\n", uri->path);
    while(Rio_readlineb(rio, buf, MAXLINE) > 0){
        if(strcmp(buf, "\r\n") == 0){
            strcat(con_data, "\r\n");
            break;
        }
        else if(strncasecmp(buf, "Host:", 5) == 0)
            strcpy(host_hdr, buf);
        else if(!strncasecmp(buf, "Connection:", 11) && !strncasecmp(buf, "Proxy_Connection:", 17) && !strncasecmp(buf, "User-agent:", 11))
            strcat(con_data, buf);
    }
    if(!strlen(host_hdr))
        sprintf(host_hdr, "Host: %s\r\n", uri->host);
    sprintf(new_uri, "%s%s%s%s%s%s", reqline, host_hdr, con_hdr, pcon_hdr, user_agent_hdr, con_data);
    return;
}

