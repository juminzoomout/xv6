#include "types.h"
#include "user.h"
#include "stat.h"

int main()
{
    // pa1
    //int i;
    //for (i=1; i<11; i++) {
    //    printf(1, "%d: ", i);
    //    if (getpname(i))
    //        printf(1, "Wrong pid\n");
    //}

    // 정상작동
    //printf(1, "%d\n", getnice(1));
    //ps(1);
    //printf(1, "%d\n", setnice(1, 1));
    //ps(0);

    // 오작동
    //printf(1, "%d\n", getnice(-7)); // pid 0보다 작음
    //printf(1, "%d\n", getnice(34)); // pid 없는 거
    //printf(1, "%d\n", setnice(37, 25)); // pid 없는 거
    //printf(1, "%d\n", setnice(1, 42)); // invalid value
    //ps(34); // pid 없는 거


    // pa2
    int parent = getpid();
    int p;

    ps(0);
    printf(1, "after setnice(1,5), setnice(2,5)\n");
    setnice(1,5);
    setnice(2,5);
    ps(0);

    p=fork();
    printf(1,"fork 상속확인 %d \n",p);
    ps(0);
    if(p<0) printf(1,"fork error\n");
    if(p==0){
      setnice(parent,39);
      printf(1,"setnice(parent,39)\n");
      ps(0);
      printf(1,"\n");
      exit();
    }
    else {
      wait();
      setnice(p,39);
      printf(1,"setnice(child,39)\n");
      ps(0);
      printf(1,"\n");
    }
    printf(1,"afterfork\n");
    ps(0);
    
    exit();
}
