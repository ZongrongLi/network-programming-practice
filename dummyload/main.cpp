#include<iostream>
#include<pthread.h>
#include<thread>
#include<unistd.h>
#include<assert.h>
#include<boost/noncopyable.hpp>
#include <sys/ipc.h>
#include <semaphore.h>
#include <fcntl.h>
#include<math.h>
#include<iostream>
#include"boost/noncopyable.hpp"

using namespace std;



double g_percent =20;

sem_t mutex;
#define PI 3.14159265358979323846
void fun(){
    int cnt=0;
    double  val = PI/180.0;
    while(1){
        int err = 2*g_percent - 100;

        bool busy=false;
        for (int i = 0; i < 100; ++i)
        {
          bool busy = false;
          if (err > 0)
          {
            busy = true;
            err += 2*(g_percent - 100);
            // printf("%2d, ", i);
          }
          else
          {
            err += 2*g_percent;
          }

         if(busy)  sem_post(&mutex);

          // if(i%5==0)sem_post(&mutex);

          usleep(10000);
        }
        cnt+=5;
        g_percent = cos(cnt*val)*100;

        if(g_percent<0)g_percent*=-1;
        if(g_percent<10e-6)g_percent+=1;
        // printf("%lf\n",g_percent);
    }



}

void loop(){
    for(int i=0;i<5000000;i++);
}

int main(int argc, char** argv)
{
    sem_init(&mutex,0,1);
     printf("pid:%d\n",getpid());

    std::thread th(fun);
    while(1){
        sem_wait(&mutex);
      //  printf("2\n");
       loop();
    }
    th.join();


    while(1){
        int c =0;
        for(int i=0;i<5000;i++)c+=200;
    };

}
