//NOTICE: you need to use libcurl-devel for C++ to use HTTP client, please read client.cpp and client.h seriously


#include <stdio.h> 
#include <iostream>
#include <string> 
#include "client.h"

using namespace std;

int main()
{

	CHttpClient hc;
	string res;
	int ret;

	//ret = hc.Get("http://172.31.222.94:9000/?operation=build&db_name=lubm&ds_path=data/LUBM_10.n3", res); 
	//cout<<res<<endl;
	
	//ret = hc.Get("http://172.31.222.94:9000/?operation=load&db_name=lubm", res); 
	//cout<<res<<endl;
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
	int tnum = 6;
	tnum = 3000;
	bool correctness = true;
	for(int i = 0; i < tnum; ++i)
	{
		ret = hc.Get("http://172.31.222.94:9000/?operation=query&format=json&sparql="+sparql[i%6],res);
		cout<< i <<endl;
                int m = 0;
		for(int ii = 0; ii<sparql[i%6].length(); ++ii)
		{
			if(sparql[i%6][ii] == '?')
				++m;
			if(sparql[i%6][ii] == '{')
				break;
		}
		int n = 0;
		for(int ii = 0; ii<res.length(); ++ii)	
		{
			if(res[ii] == '{')
				++n;
		}
		int Num = (n-3)/(m+1);
		if(Num!=result[i%6])
			correctness =false;
		
	}
	if (correctness == true)
		cout<< "The answers are correct!" <<endl;
	else
		cout<< "The answers exist errors!" <<endl;
	//ret = hc.Get("127.0.0.1:8080/query/data/ex0.sql", res); 
	//cout<<res<<endl;
	
	//ret = hc.Get("http://172.31.222.94:9000/?operation=monitor", res); 
	//cout<<res<<endl;
	
	//ret = hc.Get("http://172.31.222.94:9000/?operation=unload&db_name=lubm", res); 
	//cout<<res<<endl;

	return 0;
}

