/*
模擬店員與店長同步處理的動作
在本作業中,利用pthread的方式來練習process coordination的觀念
編譯: gcc -o s1071533_prog3 s1071533_prog3.cpp -lpthread -lstdc++
執行: ./s1071533_prog3 m p (注: m為功能選項, p為該功能參數)
*/
#include <stdio.h>
#include <cstdlib>
#include <unistd.h>
#include <pthread.h>

using namespace std;

const int MAXIMUM_THREAD_NUMBER = 6;
const int INGREDIENT_NUMBER = 4;
const char ingr[INGREDIENT_NUMBER][10] = {"Bean", "Milk", "Syrup", "Cinnamon"};
int desk[INGREDIENT_NUMBER]{};
int *cm, cm_num = 0, fc_num = 0;
int remain_coffee = 100;
int curt_thread_num = 0;
pthread_mutex_t desk_lock = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t thread_lock = PTHREAD_MUTEX_INITIALIZER;

void error_and_die(const char *msg);

void *thread_cm_func(void *ptr);

void *thread_fc_func(void *ptr);

struct FC
{
    int hold;
    int remain = 100;
    void set_hold(char ingr_char)
    {
        for (int i = 0; i < INGREDIENT_NUMBER; i++)
        {
            if (ingr_char == ingr[i][0])
            {
                hold = i;
                break;
            }
            if (i == INGREDIENT_NUMBER - 1)
                error_and_die("wrong ingredient");
        }
    };
};
FC *fc;

int main(int argc, char *argv[])
{
    srand(0);
    pthread_t threads[MAXIMUM_THREAD_NUMBER];
    int mode = (int)atoi(argv[1]);
    switch (mode)
    {
    // CM * 1 & FC * 3
    case 0:
        cm_num = 1;
        fc_num = 3;
        break;
    // CM * 1 & FC * 4
    case 1:
        cm_num = 1;
        fc_num = 4;
        break;
    case 2:
        error_and_die("wrong option");
        break;
    // CM * 2 & FC * 3
    case 3:
        cm_num = 2;
        fc_num = 3;
        break;
    default:
        error_and_die("wrong option");
        break;
    }
    cm = new int[cm_num];
    for (int i = 0; i < cm_num; i++)
    {
        curt_thread_num++;
        cm[i] = i + 1;
        pthread_create(&threads[i], NULL, thread_cm_func, (void *)&cm[i]);
    }
    fc = new FC[fc_num];
    for (int i = 0; i < fc_num; i++)
    {
        curt_thread_num++;
        fc[i].set_hold(argv[i + 2][0]);
        pthread_create(&threads[i], NULL, thread_fc_func, (void *)&fc[i]);
    }
    // wait all thread terminate
    while (1)
    {
        pthread_mutex_lock(&thread_lock);
        if (!curt_thread_num)
            break;
        pthread_mutex_unlock(&thread_lock);
    }
}

void error_and_die(const char *msg)
{
    perror(msg);
    exit(EXIT_FAILURE);
}

// CM check desk and put ingredient on desk
void *thread_cm_func(void *ptr)
{
    pthread_detach(pthread_self());
    int *cm_number = (int *)ptr, choise;
    bool put_ingr;
    while (1)
    {
        put_ingr = true;
        pthread_mutex_lock(&desk_lock);
        if (remain_coffee == 0)
        {
            pthread_mutex_unlock(&desk_lock);
            break;
        }
        // check desk
        //printf("CM(%d): 確認料理台\n", *cm_number);
        for (int i = 0; i < INGREDIENT_NUMBER; i++)
        {
            if (desk[i] < cm_num)
            {
                put_ingr = false;
                break;
            }
        }
        while (!put_ingr)
        {
            // chose ingredient
            choise = rand() % INGREDIENT_NUMBER;
            printf("CM(%d): 選擇 %s\n", *cm_number, ingr[choise]);
            // check chosen ingredient whether can put on desk
            if (desk[choise] < cm_num)
            {
                desk[choise]++;
                printf("CM(%d): 放置 %s\n", *cm_number, ingr[choise]);
                break;
            }
            printf("CM(%d): %s 已經放滿\n", *cm_number, ingr[choise]);
        }
        pthread_mutex_unlock(&desk_lock);
    }
    // remove thread
    pthread_mutex_lock(&thread_lock);
    curt_thread_num--;
    pthread_mutex_unlock(&thread_lock);
    return NULL;
}

// FC check desk and take away all ingredient from desk
void *thread_fc_func(void *ptr)
{
    pthread_detach(pthread_self());
    FC *fc_ptr = (FC *)ptr;
    bool get_ingr;
    while (1)
    {
        get_ingr = false;
        pthread_mutex_lock(&desk_lock);
        if (remain_coffee == 0)
        {
            pthread_mutex_unlock(&desk_lock);
            break;
        }
        // check desk
        printf("FC(%s): 確認料理台\n", ingr[fc_ptr->hold]);
        for (int i = 0; i < INGREDIENT_NUMBER; i++)
        {
            if (fc_ptr->hold != i && desk[i] == 0)
            {
                get_ingr = true;
                break;
            }
        }
        if (!get_ingr)
        {
            // take away all ingredient
            printf("FC(%s): 拿走所有食材\n", ingr[fc_ptr->hold]);
            for (int i = 0; i < INGREDIENT_NUMBER; i++)
            {
                if (fc_ptr->hold == i)
                    fc_ptr->remain--;
                else
                    desk[i]--;
            }
            remain_coffee--;
            printf("FC(%s): 製作第 %d 杯咖啡\n", ingr[fc_ptr->hold], 100 - remain_coffee);
        }
        pthread_mutex_unlock(&desk_lock);
        // wait 5ms
        usleep(5000);
    }
    printf("FC(%s): 總共做出 %d 杯,剩 %d 份\n", ingr[fc_ptr->hold], 100 - fc_ptr->remain, fc_ptr->remain);
    // remove thread
    pthread_mutex_lock(&thread_lock);
    curt_thread_num--;
    pthread_mutex_unlock(&thread_lock);
    return NULL;
}
