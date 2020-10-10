/*
Jaccard similarity coefficient(傑卡德相似係數)
計算每個文件與其他文件的JSC,計算後儲存以重複利用
熟悉如何使用Pthreads的API,撰寫multithreaded program
編譯: gcc -o s1071533_prog2 s1071533_prog2.cpp -lpthread -lstdc++
執行: ./s1071533_prog2 data.txt
*/
#include <stdio.h>
#include <cstdlib>
#include <fstream>
#include <cstring>
#include <vector>
#include <set>
#include <algorithm>
#include <time.h>
#include <pthread.h>

using namespace std;

struct Document
{
    int id_num;
    string doc_id;
};

const int MAXIMUM_THREAD_NUMBER = 50;
Document doc[MAXIMUM_THREAD_NUMBER];            // id and index
vector<string> elem_vec[MAXIMUM_THREAD_NUMBER]; // document content set
int thread_num;
float elem_jsc[MAXIMUM_THREAD_NUMBER][MAXIMUM_THREAD_NUMBER]{}; // history of jsc
float jsc_avg[MAXIMUM_THREAD_NUMBER];

void error_and_die(const char *msg);

int readFile(char file_name[]);

void *thread_function(void *ptr);

int main(int argc, char *argv[])
{
    clock_t start = clock(), end;
    pthread_t threads[MAXIMUM_THREAD_NUMBER];
    thread_num = readFile(argv[1]);
    for (int i = 0; i < thread_num; i++)
    {
        pthread_create(&threads[i], NULL, thread_function, (void *)&doc[i]);
        printf("[Main thread] create TID:%ld, DocID:%s\n", threads[i], doc[i].doc_id.c_str());
    }
    for (int i = 0; i < thread_num; i++)
    {
        pthread_join(threads[i], NULL);
    }
    int heighest_jsc_id_num;
    float heighest_jsc = 0;
    for (int i = 0; i < thread_num; i++)
    {
        if (heighest_jsc < jsc_avg[i])
        {
            heighest_jsc = jsc_avg[i];
            heighest_jsc_id_num = i;
        }
    }
    printf("[Main thread] KeyDocID:%s HighestJ:%.5f\n", doc[heighest_jsc_id_num].doc_id.c_str(), heighest_jsc);
    end = clock();
    printf("[Main thread] CPU time: %.3fms\n", ((double)(end - start)) * 1000 / CLOCKS_PER_SEC);
}

void error_and_die(const char *msg)
{
    perror(msg);
    exit(EXIT_FAILURE);
}

// read file & collate data
int readFile(char file_name[])
{
    char temp_word[20], temp;
    int id_num = -1;
    set<string> elem_set;
    fstream readFile(file_name, ios::in);
    if (!readFile)
        error_and_die("File");
    while (readFile.getline(temp_word, 20))
    {
        // read document id
        id_num++;
        doc[id_num].id_num = id_num;
        doc[id_num].doc_id = temp_word;
        int i = 0;
        // read document content & save as global set
        while (readFile.get(temp))
        {
            if (('a' <= temp && temp <= 'z') || ('A' <= temp && temp <= 'Z'))
                temp_word[i++] = temp;
            else if (temp == ' ')
            {
                temp_word[i] = '\0';
                i = 0;
                elem_set.insert(temp_word);
                continue;
            }
            else if (temp == '\n')
            {
                if (strlen(temp_word) != 0)
                    elem_set.insert(temp_word);
                break;
            }
        }
        elem_vec[id_num].assign(elem_set.begin(), elem_set.end());
        elem_set.clear();
        sort(elem_vec[id_num].begin(), elem_vec[id_num].end());
    }
    return id_num + 1;
}

// calculate Jaccard similarity coefficient
void *thread_function(void *ptr)
{
    clock_t start = clock(), end;
    Document *doc_ptr = (Document *)ptr;
    unsigned long int Tid = pthread_self();
    float total = 0, avg;
    vector<string> inter_vec;
    printf("[TID=%ld] DocI:%s\n", Tid, doc_ptr->doc_id.c_str());
    for (int i = 1; i < thread_num; i++)
    {
        int inter_id_num = (doc_ptr->id_num + i) % thread_num;
        float jsc = elem_jsc[doc_ptr->id_num][inter_id_num];
        if (jsc == 0) // check jsc whether have been calculated
        {

            set_intersection(elem_vec[doc_ptr->id_num].begin(), elem_vec[doc_ptr->id_num].end(),
                             elem_vec[inter_id_num].begin(), elem_vec[inter_id_num].end(),
                             back_inserter(inter_vec));
            float inter_size = inter_vec.size();
            inter_vec.clear();
            jsc = inter_size / (elem_vec[doc_ptr->id_num].size() + elem_vec[inter_id_num].size() - inter_size);
            elem_jsc[doc_ptr->id_num][inter_id_num] = elem_jsc[inter_id_num][doc_ptr->id_num] = jsc;
        }
        printf("[TID=%ld] J(%s,%s)=%.5f\n", Tid, doc_ptr->doc_id.c_str(),
               doc[inter_id_num].doc_id.c_str(), jsc);
        total += jsc;
    }
    avg = total / (thread_num - 1);
    jsc_avg[doc_ptr->id_num] = avg;
    printf("[TID=%ld] AvgJ:%.5f\n", Tid, avg);
    end = clock();
    printf("[TID=%ld] CPU time: %.3fms\n", Tid, ((double)(end - start)) * 1000 / CLOCKS_PER_SEC);
    pthread_exit(0);
}
