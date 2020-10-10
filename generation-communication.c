/*
Collatz conjecture(考拉茲猜想)
計算每一步驟的數值變化並紀錄最大值以及其是第幾次算出的結果
練習利用fork()系統呼叫產生新的process以及process彼此之間使用POSIX shared memory互相傳遞資料
編譯: gcc -o s1071533_prog1 s1071533_prog1.c -lrt
執行: ./s1071533_prog1 n
*/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/file.h>
#include <sys/mman.h>

struct region {
    pid_t child_pid;
    int times;//計算執行次數以及判斷同步
    int number;
};
struct region *rptr;

void error_and_die(const char *msg) {
  perror(msg);
  exit(EXIT_FAILURE);
}

int main(int argc, char *argv[]) {
  int r;

  const char *memname = "shm_data";

  int fd = shm_open(memname, O_CREAT | O_TRUNC | O_RDWR, 0666);
  if (fd == -1)
    error_and_die("shm_open");

  r = ftruncate(fd, sizeof(struct region));
  if (r != 0)
    error_and_die("ftruncate");

  rptr = mmap(0, sizeof(struct region), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
  if (rptr == MAP_FAILED)
    error_and_die("mmap");
  close(fd);

  if (argc==1)
    error_and_die("noParameter");

    rptr->number=atoi(argv[1]);
    rptr->times=0;

    pid_t pid = fork();

    //Child Process
    if (pid == 0) {
      int number;

      while (rptr->number!=1) {
        while (rptr->times%2==0);//synchronize
    
        if (rptr->number==1)
          break;

        number=rptr->number;
        if (number%2==0)
	  number/=2;
        else
	  number=number*3+1;

        printf("[%d Child]: %d\n",rptr->child_pid,number);

        rptr->number=number;
        rptr->times++;
      }
      exit(0);
    }
    //Parent Process
    else {
      int number,max=atoi(argv[1]),turn=0;

      pid_t parent_pid=getpid();
      rptr->child_pid=pid;
      rptr->times++;

      while (rptr->number!=1) {
        while (rptr->times%2==1);//synchronize
      
        if (rptr->number==1) {
	  printf("[%d Parent]: %d\n",parent_pid,1);
	  break;
        }

        if (rptr->number>max) {//計算child process的最大值及其是第幾次
	  max=rptr->number;
  	  turn=rptr->times-1;
        }

        number=rptr->number;
        if (number%2==0)
	  number/=2;
        else
	  number=number*3+1;

        if (number>max) {//計算parent process的最大值及其是第幾次
	  max=number;
	  turn=rptr->times;
        }

        printf("[%d Parent]: %d\n",parent_pid,number);

        rptr->number=number;
        rptr->times++;
      }

      printf("Max: %d\nTurn: %d\n",max,turn);
    }

  r = munmap(rptr, sizeof(struct region));
  if (r != 0)
    error_and_die("munmap");

  r = shm_unlink(memname);
  if (r != 0)
    error_and_die("shm_unlink");

  return 0;
}
