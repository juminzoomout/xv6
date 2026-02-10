// init: The initial user-level program

#include "types.h"
#include "stat.h"
#include "user.h"
#include "fcntl.h"

char *argv[] = { "sh", 0 };

int
main(void)
{
  int pid, wpid;

  if(open("console", O_RDWR) < 0){
    mknod("console", 1, 1);
    open("console", O_RDWR);
  }
  dup(0);  // stdout
  dup(0);  // stderr

  for(;;){
    printf(1, "init: starting sh\n");
    // printf가 아닌 cprintf를 쓰는 이유: printf는 c 라이브러리(libc)에 있는 건데 운영체제 커널(kernel mode)에서는 라이브러리 못씀. cprintf는 xv6 커널 내에서 printf처럼 쓰도록 설계된 함수
    // 가 아니라 printf써야함 main.c가 커널영역이고 커널 부팅된 후 첫번째 사용자 프로세스가 init임. 즉, init은 user mode.
    // 에 추가로 여기서 printf는 stdio헤더 파일에서 가져오는 게 아님. printf.c라는 파일이 또 있다. 거기서 구현해놓은 거를 쓰는 거임. stdio헤더 불러오면 printf가 겹쳐서 오류 발생
    printf(1, "Student ID: 2020314597\n");
    printf(1, "Name: Jumin Kim\n");
    printf(1, "==========Any Message==========\n");
    pid = fork();
    if(pid < 0){
      printf(1, "init: fork failed\n");
      exit();
    }
    if(pid == 0){
      exec("sh", argv);
      printf(1, "init: exec sh failed\n");
      exit();
    }
    while((wpid=wait()) >= 0 && wpid != pid)
      printf(1, "zombie!\n");
  }
}
