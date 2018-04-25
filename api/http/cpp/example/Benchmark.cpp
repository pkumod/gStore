#include "client.h"
#include <pthread.h>
#include <iostream>
#include <sstream>
#include <string>
#include <cstring>
#include <fstream>
using namespace std;
#define tnum 10
#define bnum 10
#define lnum 10
#define unum 10
bool correctness = true;
bool lcorrectness = true;
bool ucorrectness = true;

pthread_mutex_t mutex;

string int2string(int n)
{
    string s;
    stringstream ss;
    ss<<n;
    ss>>s;
    return s;
}


struct MyThread_args{
	int num;
	string sparql;
	int rnum;
};
struct mm_args{
	string db_name;
};
void* MyThread_run(void* thread_args)
{
	struct MyThread_args *args;
	args = (struct MyThread_args *)thread_args;
	CHttpClient hc;
	string res;
	int ret;
	ret = hc.Get("http://172.31.222.93:9016/?operation=query&db_name=dbpedia&format=json&sparql="+args->sparql,res);
	int m = 0;
	for(int i = 0; i<args->sparql.length(); ++i)
	{
		if(args->sparql[i]=='?')
			++m;
		if(args->sparql[i]=='{')
			break;
	}
	int n = 0;
	for(int i = 0; i<res.length(); ++i)
	{
		if(res[i]=='{')
			++n;
	}
	int Num = (n-3)/(m+1);
	//if(Num<=10)
	//{
	//	ofstream f("result/"+int2string(args->num)+".txt");
	//	f<<res<<endl;
	//	f.close();
	//}	
	
	if(args->rnum != Num)
	{

		correctness = false;
		cout << "sparql: " << args->sparql << endl;
		cout << "Num: " << Num << endl;
	}
	pthread_exit(NULL);
}
void* load_thread(void* thread_args)
{
	struct mm_args *args;
	args = (struct mm_args *)thread_args;
	string db_name = args->db_name;
	CHttpClient hc;
	string res;
	int ret;
	ret = hc.Get("http://172.31.222.93:9016/?operation=load&db_name="+db_name+"&username=root&password=123456", res);
	res ="load: " + res + "\n";
	cout << res;
	pthread_exit(NULL);
}
void* unload_thread(void* thread_args)
{
	struct mm_args *args;
	args = (struct mm_args *)thread_args;
	string db_name = args->db_name;

	CHttpClient hc;
	string res;
	int ret;
	ret = hc.Get("http://172.31.222.93:9016/?operation=unload&db_name="+db_name+"&username=root&password=123456", res);
	res = "unload: " + res + "\n";
	cout << res;
	pthread_exit(NULL);
}
int main()
{
	string * db_name = new string[4];
	db_name[0] = "lubm";
	db_name[1] = "lubm1";
	db_name[2] = "test";
	db_name[3] = "dbpedia";

	CHttpClient hc;
	string res;
	int ret;
	for(int i = 0; i < 3; i++)
	{
		ret = hc.Get("http://172.31.222.93:9016/?operation=build&db_name="+db_name[i]+"&ds_path=data/lubm/LUBM_10.n3&username=root&password=123456", res);
		cout << res << endl;
	}

	int result[6] = {10, 14, 14, 199424, 33910, 1039};
	string* sparql = new string[6];
        sparql[0] = "select ?v0 where\
                   {\
                   ?v0 <http://www.w3.org/1999/02/22-rdf-syntax-ns#type> <http://dbpedia.org/class/yago/LanguagesOfBotswana> .\
                   ?v0 <http://www.w3.org/1999/02/22-rdf-syntax-ns#type>    <http://dbpedia.org/class/yago/LanguagesOfNamibia> .\
                   ?v0 <http://www.w3.org/1999/02/22-rdf-syntax-ns#type> <http://dbpedia.org/ontology/Language> .\
                   }";
        sparql[1] = "select ?v0 where\
                   {\
                   ?v0 <http://dbpedia.org/ontology/associatedBand> <http://dbpedia.org/resource/LCD_Soundsystem> .\
                   }";
        sparql[2] = "select ?v2 where\
                   {\
                   <http://dbpedia.org/resource/!!Destroy-Oh-Boy!!> <http://dbpedia.org/property/title> ?v2 .\
                   }";
        sparql[3] = "select ?v0 ?v2 where\
                   {\
                   ?v0 <http://dbpedia.org/ontology/activeYearsStartYear> ?v2 .\
                   }";
        sparql[4] = "select ?v0 ?v1 ?v2 where\
                   {\
                   ?v0 <http://dbpedia.org/property/dateOfBirth> ?v2 .\
                   ?v1 <http://dbpedia.org/property/genre> ?v2 .\
                   }";
        sparql[5] = "select ?v0 ?v1 ?v2 ?v3 where\
                   {\
                   ?v0 <http://dbpedia.org/property/familycolor> ?v1 .\
                   ?v0 <http://dbpedia.org/property/glotto> ?v2 .\
                   ?v0 <http://dbpedia.org/property/lc> ?v3 .\
                   }";
	pthread_t qt[tnum];
	pthread_t lqt[lnum];
	pthread_t uqt[unum];
	void *status;
	struct MyThread_args args[tnum];
	struct mm_args aas[lnum];
	for(int i = 0;i<tnum;i++)
	{
		args[i].num=i;
		args[i].sparql=sparql[i%6];
		args[i].rnum=result[i%6];
		pthread_create(&qt[i],NULL,MyThread_run,(void *)&args[i]);
	}
	for(int i = 0; i < lnum; i++)
	{
		//pthread_create(build_thread, db_name[i%3]);
		aas[i].db_name = db_name[i%4];
		pthread_create(&lqt[i], NULL, load_thread, (void *)&aas[i]);
		pthread_create(&uqt[i], NULL, unload_thread, (void *)&aas[i]);
	}
	
	for(int i = 0;i<tnum;i++)
	{
		pthread_join(qt[i],&status);
		
	}
	for(int i = 0; i < lnum; i++)
	{
		pthread_join(lqt[i], &status);
		pthread_join(uqt[i], &status);
	}
	if(correctness == true)
		cout<< "The answers are correct!" <<endl;
	else
		cout<< "The answers exist errors!" <<endl;
	//if(lcorrectness == true)
	//	cout<< "load operations are correct!" <<endl;
	//else
	//	cout<< "load operations exist errors!" <<endl;
	//if(ucorrectness == true)
	//	cout<< "unload operations are correct!" <<endl;
	//else
	//	cout<< "unload operations exist errors!" <<endl;

	pthread_exit(NULL);
	return 0;
}

