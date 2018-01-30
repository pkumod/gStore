function query_entity(num, dp)
{
	var url = "http://172.31.222.72:1234/api/search/KeywordSearch?MaxHits=5&QueryString="+dp;
		var element = document.getElementById("hideLink"+num);
		element.setAttribute("href", url);
		if(/msie/i.test(navigator.userAgent))
		{
			element.fireEvent("onclick");
		}
		else
		{
			var e = document.createEvent("MouseEvents");
			e.initEvent("click", true, true);
			element.dispatchEvent(e);
		}
}

function query_sparql(num,dp) {
	var encodeVal = escape(dp);
	//alert(encodeVal);
	format = "txt";
	var argu = "?operation=query&format=" + format + "&sparql=" + dp;
	var encodeArgu = escape(argu);
	if(format != "html")
	{
		/*
		$.get(encodeArgu, function(data, status){
			if(status == "success")
		{
			alert("success");
		}
		});
		*/
		var element = document.getElementById("hideLink"+num);
		element.setAttribute("href", encodeArgu);
		/*
		if(format == "txt")
			element.setAttribute("download", "sparql.txt");
		else if(format == "csv")
			element.setAttribute("download", "sparql.csv");
		else if(format == "json")
			element.setAttribute("download", "sparql.json");
		*/
		if(/msie/i.test(navigator.userAgent))
		{
			element.fireEvent("onclick");
		}
		else
		{
			var e = document.createEvent("MouseEvents");
			e.initEvent("click", true, true);
			element.dispatchEvent(e);
		}
	}
	else
	{
		$.get(encodeArgu, function(data, status){
			if(status=="success"){
				//toTxt();
				//alert(data);
				var parts = data.split("+");
				var fileName = parts[2];
			    var lines = Number(parts[1]);
				//alert(lines);
				if(lines > 100)
					lines = 100;
				//alert(lines);
				var items = parts[3].split("\n");
				//alert(items[0]);
				var valNum = items[0].split("?");
				var rows = valNum.length - 1;
				//alert(rows);
				var page = '<html><div align="left"><a href="javascript:void(0);" id="back" style="font-size:16px;color:blue">Click to Return</a>';
				page = page + '<a id="download" style="font-size:16px;margin-left:20px">Click to Download</a>';
				page = page + '<a href="/" id="trick" style="display: none">Click to back</a>';
				page = page + '<p>Total answers: ';
				page = page + parts[1];
				page = page + '</p>';
				if(parts[0] == "1")
				{
					
					page = page + '<p>Number of answers is too large, show only 100 of them, click to download the full result!</p>'; 
					
				}
				page = page + '</div><table border="1" style = "font-size:medium">';
				page = page + "<tr>";
		
				for(var ii = 1; ii <= rows; ii++)
				{
					page = page + "<th>" + "?" + valNum[ii] + "</th>";
				}
				page = page + "</tr>";
				var i = 1;
				var j = 0;
				for (i = 1; i <= lines; i++)
				{
					page = page + "<tr>";
					//alert(items[i]);

					/*
					var tmpItem = items[i].replace(/"([^"]*)"/g, "<$1>");
					//alert(tmpItem);
					var item = tmpItem.split(">");
					//alert(item.length);
					for(j = 0; j < rows; j++)
					{
						//alert(item[j]);
						if(j < item.length)
							page = page + '<td>' + item[j].replace("<","") + '</td>';
						else
							page = page + '<td>' + " " + '</td>';
					
					}
					*/

					
					//alert(items[i]);
					var item = items[i].split("\t");
					//alert(item.length);
					for(j = 0; j < rows; j++)
					{
						//alert(item[j]);
						if(item[j] == "")
							item[j] = " ";
						page = page + '<td>' + item[j].replace("<","&lt;") + '</td>';
					}
					
					page = page + "</tr>";
				}
				page = page + '</table></div></html>';
				$("#main_body").empty();
				$("#main_body").append(page);

				var tmp1 = "?operation=download&filepath=" + fileName;
				var request1 = escape(tmp1);
				var element1 = document.getElementById("download");
				element1.setAttribute("href", request1);
				element1.setAttribute("download", "sparql.txt");
	
				var tmp2 = "?operation=delete&filepath=" + fileName;
				var request2 = escape(tmp2);	
				var element2 = document.getElementById("back");
				//!Notice: element2 is a "<a>" tag, and it has two actions, href and onclick, be careful with the executing order of these two actions.
				//in this case, we use a call-back function to prevent strange things. we return to the origin web page after the request to delete file returns successfully.
				element2.onclick = function(){
					$.get(request2, function(data, status){
						//alert("delete return");
						var element3 = document.getElementById("trick");
						if(/msie/i.test(navigator.userAgent))
						{
							element3.fireEvent("onclick");
						}
						else
						{
							var e1 = document.createEvent("MouseEvents");
							e1.initEvent("click", true, true);
							element3.dispatchEvent(e1);
						}
					});
				}
			}
		});
	}
}

function handle1(example, query)
{
                if (example === "q1")
                {
                    document.getElementById(query).value =    "select ?v0 where { ?v0 <http://dbpedia.org/ontology/associatedBand> <http://dbpedia.org/resource/LCD_Soundsystem> . }";
                }
                if (example === "q2")
                {
                    document.getElementById(query).value =    "select ?v0 where { ?v0 <http://dbpedia.org/ontology/associatedBand>  <http://dbpedia.org/resource/Illa> . }";
				} 
}

function handle2(example, query)
{
                if (example === "q1")
                {
                    document.getElementById(query).value =    "select ?v2 where { <http://dbpedia.org/resource/!!Destroy-Oh-Boy!!> <http://dbpedia.org/property/title> ?v2 .}";
                }
                if (example === "q2")
                {
                    document.getElementById(query).value =    "select ?v2 where { <http://dbpedia.org/resource/Zombie_Heaven> <http://dbpedia.org/property/title> ?v2 .}";
				} 
}

function handle3(example, query)
{
                if (example === "q1")
                {
                    document.getElementById(query).value =    "select ?v0 where { ?v0 <http://www.w3.org/1999/02/22-rdf-syntax-ns#type> <http://dbpedia.org/class/yago/LanguagesOfBotswana> . ?v0 <http://www.w3.org/1999/02/22-rdf-syntax-ns#type>    <http://dbpedia.org/class/yago/LanguagesOfNamibia> . ?v0 <http://www.w3.org/1999/02/22-rdf-syntax-ns#type> <http://dbpedia.org/ontology/Language> .}";
                }
                if (example === "q2")
                {
                    document.getElementById(query).value =    "select ?v0 where { ?v0 <http://www.w3.org/1999/02/22-rdf-syntax-ns#type> <http://dbpedia.org/class/yago/LanguagesOfBotswana> . }";
				} 
}

function handle4(example, query)
{
                if (example === "q1")
                {
                    document.getElementById(query).value =    "select ?p where { <http://dbpedia.org/resource/Garak_Market_Station> ?p        \"Munjeong\"@en. }";
                }
                if (example === "q2")
                {
                    document.getElementById(query).value =    "select ?p where { <http://dbpedia.org/resource/Jangji_Station>      ?p        \"Munjeong\"@en. }";
				} 
}

function handle5(example, query)
{
                if (example === "q1")
                {
                    document.getElementById(query).value =    "select ?s ?p where { ?s      ?p        \"Munjeong\"@en. }";
                }
                if (example === "q2")
                {
                    document.getElementById(query).value =    "select ?s ?p where { ?s      ?p        \"SMS\"@en. }";
				} 
}

function handle6(example, query)
{
                if (example === "q1")
                {
                    document.getElementById(query).value =    "select ?v0 ?v2 where { ?v0 <http://dbpedia.org/ontology/activeYearsStartYear> ?v2 . }";
                }
                if (example === "q2")
                {
                    document.getElementById(query).value =    "select ?v0 ?v2 where { ?v0 <http://dbpedia.org/property/pinyin> ?v2 . }";
				} 
}

function handle7(example, query)
{
                if (example === "q1")
                {
                    document.getElementById(query).value =    "select ?s where { <http://dbpedia.org/resource/Ashi_guruma>       <http://dbpedia.org/property/subClass>  ?s  .}";
                }
                if (example === "q2")
                {
                    document.getElementById(query).value =    "select ?s where { <http://dbpedia.org/resource/Yoko_otoshi>       <http://dbpedia.org/property/subClass>  ?s  .}";
				} 
}

function handle8(example, query)
{
                if (example === "q1")
                {
                    document.getElementById(query).value =    "select ?s where { ?s       <http://dbpedia.org/property/subClass>  <http://dbpedia.org/resource/List_of_judo_techniques> . }";
                }
                if (example === "q2")
                {
                    document.getElementById(query).value =    "select ?s where { ?s       <http://dbpedia.org/property/subClass>  <http://dbpedia.org/resource/Randori-no-kata> . }";
				} 
}

function handle9(example, query)
{
                if (example === "q1")
                {
                    document.getElementById(query).value =    "select ?v0 ?v1 ?v2 where { ?v0 <http://dbpedia.org/ontology/associatedBand> ?v2 . ?v0 <http://dbpedia.org/property/pastMembers> ?v1 . ?v1 <http://dbpedia.org/ontology/associatedBand> ?v2 .  }";
                }
                if (example === "q2")
                {
                    document.getElementById(query).value =    "select ?v0 ?v1 ?v2 where { ?v0 <http://dbpedia.org/property/dateOfBirth> ?v2 . ?v1 <http://dbpedia.org/property/pinyin> ?v2 . ?v0  <http://dbpedia.org/ontology/activeYearsStartYear> ?v1 . }";
				} 
}

function handle10(example, query)
{
                if (example === "q1")
                {
                    document.getElementById(query).value =    "select ?v0 ?v1 where { ?v0 <http://dbpedia.org/property/familycolor> ?v1 . ?v0 <http://dbpedia.org/property/glotto> ?v2 . ?v0 <http://dbpedia.org/property/lc> ?v3 . }";
                }
                if (example === "q2")
                {
                    document.getElementById(query).value =    "select ?v0 ?v1 ?v2 where { ?v0 <http://dbpedia.org/property/familycolor> ?v1 . ?v0 <http://dbpedia.org/property/glotto> ?v2 . }";
				} 
}

function handle11(example, query)
{
                if (example === "q1")
                {
                    document.getElementById(query).value =    "select ?v0 ?v3 where { ?v0 <http://dbpedia.org/property/dateOfBirth> ?v1 . ?v2 <http://dbpedia.org/property/genre> ?v1 .  ?v2  <http://xmlns.com/foaf/0.1/homepage> ?v3. }";
                }
                if (example === "q2")
                {
                    document.getElementById(query).value =    "select ?v0 ?v3 where { ?v0 <http://dbpedia.org/property/dateOfBirth> ?v1 . ?v2 <http://dbpedia.org/property/genre> ?v1 .  ?v2 <http://dbpedia.org/property/alias> ?v3. }";
				} 
}

function handle12(example, query)
{
                if (example === "q1")
                {
                    document.getElementById(query).value =    "select ?v0 ?v1 where { ?v0 <http://dbpedia.org/ontology/associatedBand> <http://dbpedia.org/resource/LCD_Soundsystem> . ?v0 <http://dbpedia.org/ontology/associatedBand> <http://dbpedia.org/resource/Six_Finger_Satellite> .  ?v0  <http://www.w3.org/1999/02/22-rdf-syntax-ns#type> ?v1 . <http://dbpedia.org/resource/!PAUS3> <http://www.w3.org/1999/02/22-rdf-syntax-ns#type> ?v1 . <http://dbpedia.org/resource/!Action_Pact!> <http://www.w3.org/1999/02/22-rdf-syntax-ns#type> ?v1 . }";
                }
                if (example === "q2")
                {
                    document.getElementById(query).value =    "select ?v0 ?v1 where { ?v0 <http://dbpedia.org/ontology/associatedBand> <http://dbpedia.org/resource/LCD_Soundsystem> . ?v0 <http://dbpedia.org/ontology/associatedBand> <http://dbpedia.org/resource/Six_Finger_Satellite> .  ?v0  <http://www.w3.org/1999/02/22-rdf-syntax-ns#type> ?v1 . <http://dbpedia.org/resource/!PAUS3> <http://www.w3.org/1999/02/22-rdf-syntax-ns#type> ?v1 . <http://dbpedia.org/resource/07_Vestur> <http://www.w3.org/1999/02/22-rdf-syntax-ns#type> ?v1 . }";
				} 
}

function handle13(example, query)
{
                if (example === "q1")
                {
                    document.getElementById(query).value =    "dog";
                }
                if (example === "q2")
                {
                    document.getElementById(query).value =    "US";
				} 
}

function handle14(example, query)
{
                if (example === "q1")
                {
                    document.getElementById(query).value =    "delete data {         <http://dbpedia.org/resource/The_Juan_MacLean> <http://dbpedia.org/ontology/associatedBand> <http://dbpedia.org/resource/LCD_Soundsystem> .  }";
                }
                if (example === "q2")
                {
                    //document.getElementById(query).value =    "delete where {         ?v0 <http://dbpedia.org/ontology/associatedBand> <http://dbpedia.org/resource/Illa> .  }";
                    document.getElementById(query).value =    "delete { ?v0 <http://dbpedia.org/ontology/type> <http://dbpedia.org/class/entity> . } where { ?v0 <http://dbpedia.org/ontology/associatedBand>  <http://dbpedia.org/resource/Illa> . }";
				} 
}

function handle15(example, query)
{
                if (example === "q1")
                {
                    document.getElementById(query).value =    "insert data { <http://dbpedia.org/resource/The_Juan_MacLean> <http://dbpedia.org/ontology/associatedBand> <http://dbpedia.org/resource/LCD_Soundsystem> .   }";
				}
                if (example === "q2")
                {
                    document.getElementById(query).value =    "insert { ?v0 <http://dbpedia.org/ontology/type> <http://dbpedia.org/class/entity> . } where { ?v0 <http://dbpedia.org/ontology/associatedBand>  <http://dbpedia.org/resource/Illa> . }";
				} 
}

function handle16(example, query)
{
                if (example === "q1")
                {
                    document.getElementById(query).value =    "select ?v0 where { ?v0 <http://dbpedia.org/ontology/associatedBand> <http://dbpedia.org/resource/LCD_Soundsystem> .   }";
				}
                if (example === "q2")
                {
                    document.getElementById(query).value =    "select ?v0 where { ?v0 <http://dbpedia.org/ontology/type> <http://dbpedia.org/class/entity> .  }";
				} 
}

function handleQueryExample(num)
{
		//alert(num);
		if(num==undefined)
		{
			handle1(document.getElementById("example1").value, "query_text1");
			handle2(document.getElementById("example2").value, "query_text2");
			handle3(document.getElementById("example3").value, "query_text3");
			handle4(document.getElementById("example4").value, "query_text4");
			handle5(document.getElementById("example5").value, "query_text5");
			handle6(document.getElementById("example6").value, "query_text6");
			handle7(document.getElementById("example7").value, "query_text7");
			handle8(document.getElementById("example8").value, "query_text8");
			handle9(document.getElementById("example9").value, "query_text9");
			handle10(document.getElementById("example10").value, "query_text10");
			handle11(document.getElementById("example11").value, "query_text11");
			handle12(document.getElementById("example12").value, "query_text12");
			handle13(document.getElementById("example13").value, "query_text13");
			handle14(document.getElementById("example14").value, "query_text14");
			handle15(document.getElementById("example15").value, "query_text15");
			handle16(document.getElementById("example16").value, "query_text16");
			return;
		}
	examp = "example"+num; query="query_text"+num;
	var example = document.getElementById(examp).value;
	if(num==1) handle1(example, query);
	if(num==2) handle2(example, query);
	if(num==3) handle3(example, query);
	if(num==4) handle4(example, query);
	if(num==5) handle5(example, query);
	if(num==6) handle6(example, query);
	if(num==7) handle7(example, query);
	if(num==8) handle8(example, query);
	if(num==9) handle9(example, query);
	if(num==10) handle10(example, query);
	if(num==11) handle11(example, query);
	if(num==12) handle12(example, query);
	if(num==13) handle13(example, query);
	if(num==14) handle14(example, query);
	if(num==15) handle15(example, query);
	if(num==16) handle16(example, query);
}
