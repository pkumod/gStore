
function query(dp) {
	//alert(dp);
	var encodeVal = escape(dp);
	//alert(encodeVal);
	var format = document.getElementById("element_5").value;
	if(format == "1")
		format = "html";
	else if(format == "2")
		format = "txt";
	else if(format == "3")
		format = "csv";
	else if(format == "4")
		format = "json";
	var argu = "?operation=query&format=" + format + "&sparql=" + dp;
	var encodeArgu = escape(argu);
	$.get(encodeArgu, function(data, status){

		if(status=="success"){
			//toTxt();
			//alert(data);
			if(format == "html")
			{
				var parts = data.split("+");
				var fileName = parts[2];
			    var lines = Number(parts[1]);
				//alert(lines);
				if(lines > 100)
					lines = 100;
				//alert(lines);
				var items = parts[3].split("\n");
				//alert(items[86]);
				var valNum = items[0].split("?");
				var rows = valNum.length - 1;

				var page = '<html><div align="left"><a href="/" id="back" style="font-size:16px;color:blue">Click to Return</a>';
				page = page + '<a id="download" style="font-size:16px;margin-left:20px">Click to Download</a>';
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
					var tmpItem = items[i].replace(/"([^"]*)"/g, "<$1>");
					//alert(tmpItem);
					var item = tmpItem.split(">");
					//alert(item.length);
					for(j = 0; j < rows; j++)
					{
						page = page + '<td>' + item[j].replace("<", "") + '</td>';
					}
					page = page + "</tr>";
				}
				page = page + '</table></div></html>';
				$("#main_body").empty();
				$("#main_body").append(page);

				var tmp1 = "?operation=delete&download=true&filepath=" + fileName;
				var request1 = escape(tmp1);
				var element1 = document.getElementById("download");
				element1.setAttribute("href", request1);
				element1.setAttribute("download", "sparql.txt");
				element1.onclick = function(){
					element1.setAttribute("style", "display: none");
				}
				var element2 = document.getElementById("back");
				element2.onclick = function(){
					if($(element1).css("display") != "none")
					{
						var tmp2 = "?operation=delete&download=false&filepath=" + fileName;
						var request2 = escape(tmp2);
						$.get(request2, function(data, status){});
					}
				}
			}
			else
			{
				var parts = data.split("+");
				var fileName = parts[2];
				var tmp = "?operation=delete&download=true&filepath=" + fileName;
				var request = escape(tmp);
				var element = document.getElementById("hideLink");
				element.setAttribute("href", request);
				if(format == "txt")
					element.setAttribute("download", "sparql.txt");
				else if(format == "csv")
					element.setAttribute("download", "sparql.csv");
				else if(format == "json")
					element.setAttribute("download", "sparql.json");
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
			/*
		
		alert(data);
		var vl = document.getElementById("queryAns").value;
		alert(vl);
		//$("#queryAns").value=data;
		document.getElementById("queryAns").value = data;
		vl = document.getElementById("queryAns").value;
		alert(vl);
		document.getElementById("myForm").submit();
		*/
		}
	});
	
}

function handleQueryExample()
            {
                var example = document.getElementById("example").value;
                if (example === "q1")
                {
                    document.getElementById("element_3").value =    "SELECT DISTINCT ?v0 ?v1 ?v2 \n" +
                    												"WHERE \n" +
																	"{ \n" +
																	"\t?v0 <http://www.w3.org/1999/02/22-rdf-syntax-ns#type> <http://dbpedia.org/class/yago/LanguagesOfBotswana> . \n" +
																	"\t?v0 <http://www.w3.org/1999/02/22-rdf-syntax-ns#type> <http://dbpedia.org/class/yago/LanguagesOfNamibia> . \n" +
																	"\t?v0 <http://www.w3.org/1999/02/22-rdf-syntax-ns#type> <http://dbpedia.org/ontology/Language> . \n" +
																	"} \n";
                }

                if (example === "q2")
                {
                    document.getElementById("element_3").value =    "SELECT ?v0 ?v1 WHERE \n" +
																	"{ \n" +
																	"\t?v0 <http://purl.org/dc/terms/subject> ?v1 .\n" +
																	"} \n";
                }

                if (example === "q3")
                {
                    document.getElementById("element_3").value =    "SELECT ?v0 ?v1 ?v2 ?v3 WHERE \n" +
																	"{ \n" +
																	"\t?v0 <http://dbpedia.org/ontology/wikiPageWikiLink> <http://dbpedia.org/resource/Alcohol> .\n" +
																	"\tOPTIONAL{?v0 <http://dbpedia.org/ontology/wikiPageWikiLink> ?v2 .}\n" +
																	"} \n";
                }

                if (example === "q4")
                {
                    document.getElementById("element_3").value =    "SELECT DISTINCT ?v0 WHERE \n" +
																	"{ \n" +
																	"\t?v0 <http://purl.org/dc/terms/subject> <http://dbpedia.org/resource/Category:956_births> .\n" +
																	"\t{?v0 <http://dbpedia.org/property/wikiPageUsesTemplate> ?v1 .}\n" + "UNION\n" +
																	"\t{?v0 <http://dbpedia.org/prpperty/hasPhotoCollection> ?v2 .}\n" +
																	"} \n";
                }
                if (example === "q5")
                {
                    document.getElementById("element_3").value =    "SELECT ?v0 ?v2 ?v3 WHERE \n" +
																	"{ \n" +
																	"\t?v0 <http://dbpedia.org/ontology/wikiPageWikiLink> <http://dbpedia.org/resource/Autism> .\n" +
																	"\t?v2 <http://dbpedia.org/property/candidate> ?v0 .\n" +
																	"\t?v3 <http://dbpedia.org/property/constituencyWestminster> ?v2 .\n" +
																	"} \n";
                }
                if (example === "q6")
                {
                    document.getElementById("element_3").value =    "SELECT ?V1 ?V2 WHERE \n" +
																	"{ \n" +
																	"\t?v1 <http://www.w3.org/2002/07/owl#sameAs> <http://it.dbpedia.org/resource/Category:Filosofi_del_IV_secolo_a.C.> .\n" +
																	"\t?v2 <dbpedia.org/ontology/wikiPageWikiLink> ?v1 .\n" +
																	"\t<http://dbpedia.org/resource/Chinese_classics> <http://dbpedia.org/ontology/wikiPageWikiLink> ?v2  .\n" +
																	"} \n";
                }
                if (example === "q7")
                {
                    document.getElementById("element_3").value =    "SELECT ?v0 ?v1 WHERE \n" +
																	"{ \n" +
																	"\t<http://dbpedia.org/resource/Albedo> <http://dbpedia.org/ontology/wikiPageWikiLink> ?v0 .\n" +
																	"\t?v1 <http://dbpedia.org/ontology/wikiPageWikiLink> ?v0 .\n" +
																	"}\n";
                }
                if (example === "q8")
                {
                    document.getElementById("element_3").value =    "SELECT ?v0 ?v1 ?v2 ?v3 ?v4 ?v5 ?v6 WHERE \n" +
																	"{\n" +
																	"\t?v0 <http://www.w3.org/2004/02/skos/core#broader> ?v1 .\n" +
																	"\t?v0 <http://dbpedia.org/ontology/child> ?v2 .\n" +
																	"\t?v0 <http://dbpedia.org/ontology/spouse> ?v3 .\n" +
																	"\t?v0 <http://dbpedia.org/ontology/profession> ?v4 .\n" +
																	"\t?v0 <http://dbpedia.org/ontology/party> ?v5 .\n" +
																	"\t?v0 <http://dbpedia.prg/ontology/militaryRank> ?v6 .\n" + 
																	"}\n";
                }
                if (example === "q9")
                {
                    document.getElementById("element_3").value =    "SELECT ?v0 ?v1 ?v2 ?v3 WHERE\n" +
																	"{\n" +
																	"\t?V0 <http://dbpedia.org/ontology/location> ?v1 .\n" +
																	"\t?v0 <http://dbpedia.org/ontology/typeOfElectrification> ?v2 .\n" +
																	"\t?v0 <http://dbpedia.org/ontology/wikiPageWikiLink> <http://dbpedia.org/resource/Koltsevaya_Line> .\n" +
																	"\t?v0 <http://dbpedia.org/property/wikiPageUsesTemplate> ?v3 .\n" +
																	"}\n";
                }
                if (example === "q10")
                {
                    document.getElementById("element_3").value =    "SELECT ?v0 ?v1 ?v2 ?v3 WHERE\n" +
																	"{\n" +
																	"\t?v0 <http://dbpedia.org/property/hasPhotoCollection> ?v1 .\n" +
																	"\t?v0 <xmlns.com/foaf/0.1/depiction> ?v2 .\n" +
																	"\t?v0 <http://dbpedia.org/ontology/abstract> ?v3 .\n" +
																	"\t?v0 <http://dbpedia.org/ontology/wikiPageWikiLink> <http://dbpedia.org/resource/Combination> .\n" +
																	"}\n";
                }
            }
