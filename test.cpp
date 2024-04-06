#include "csapp.h"
#include <bits/stdc++.h>
/* Recommended max cache and object sizes */
#define MAX_CACHE_SIZE 1049000
#define MAX_OBJECT_SIZE 102400
#define MAX_CACHE 10
/* You won't lose style points for including this long line in your code */


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

struct Cache cache[1000000];
//struct rwlock_t *rl;
int now_lru;

int main(){
   // rl = (struct rwlock_t*)malloc(sizeof(struct rwlock_t));
}