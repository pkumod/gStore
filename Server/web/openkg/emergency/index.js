
function query(dp) {
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
		var element = document.getElementById("hideLink");
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
					var tmpItem = items[i].replace(/"([^"]*)"/g, "<$1>");
					//alert(tmpItem);
					var item = tmpItem.split(">");
					//alert(item.length);
					for(j = 0; j < rows; j++)
					{
						//alert(item[j]);
						page = page + '<td>' + item[j].replace("<", "") + '</td>';
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

function handleQueryExample()
            {
                var example = document.getElementById("example").value;
				if(example === "q1")
				{
					document.getElementById("element_3").value = "select ?x ?y\n" +
					"where\n" +
					"{\n" +
							"\t?x <http://www.wust.edu.cn/emergency/property#description> ?y.\n" +
					"}\n";
				}

				if(example === "q2")
				{
					document.getElementById("element_3").value = "select ?x ?y\n" +
					"where\n" +
					"{ \n" +
							"\t?s <http://www.w3.org/1999/02/22-rdf-syntax-ns#type> ?x.\n" +
							"\t?s <http://www.wust.edu.cn/emergency/property#location> ?y.\n" +
					"} \n";
				}
				if(example === "q3")
				{
					document.getElementById("element_3").value = "select ?x ?y ?z ?w\n" + 
					"where\n" +
					"{ \n" +
							"\t?x <http://www.wust.edu.cn/emergency/property#begindate> ?y./n" +
							"\t?x <http://www.wust.edu.cn/emergency/property#enddate> ?z.\n" +
							"\t?x <http://www.wust.edu.cn/emergency/property#casualties> ?w.\n" +
					"} \n";
				}
                //if (example === "q4")
                //{
                    //document.getElementById("element_3").value =    "select distinct ?x where \n" +
																	//"{ \n" +
																	//"\t?x	<rdf:type>	<ub:GraduateStudent>. \n" +
																	//"\t?y	<rdf:type>	<ub:University>. \n" +
																	//"\t?z	<rdf:type>	<ub:Department>. \n" +
																	//"\t?x	<ub:memberOf>	?z. \n" +
																	//"\t?z	<ub:subOrganizationOf>	?y. \n" +
																	//"\t?x	<ub:undergraduateDegreeFrom>	?y. \n" +
																	//"} \n";
                //}
                //if (example === "q5")
                //{
                    //document.getElementById("element_3").value =    "select distinct ?x where \n" +
																	//"{ \n" +
																	//"\t?x	<rdf:type>	<ub:Course>. \n" +
																	//"\t?x	<ub:name>	?y. \n" +
																	//"} \n";
                //}
                //if (example === "q6")
                //{
                    //document.getElementById("element_3").value =    "select distinct ?x where \n" +
																	//"{ \n" +
																	//"\t?x    <rdf:type>    <ub:UndergraduateStudent>. \n" +
																	//"\t?y    <ub:name> <Course1>. \n" +
																	//"\t?x    <ub:takesCourse>  ?y. \n" +
																	//"\t?z    <ub:teacherOf>    ?y. \n" +
																	//"\t?z    <ub:name> <FullProfessor1>. \n" +
																	//"\t?z    <ub:worksFor>    ?w. \n" +
																	//"\t?w    <ub:name>    <Department0>. \n" +
																	//"} \n";
                //}
                //if (example === "q7")
                //{
                    //document.getElementById("element_3").value =    "select distinct ?x where \n" +
																	//"{ \n" +
																	//"\t?x    <rdf:type>    <ub:UndergraduateStudent>. \n" +
																	//"}\n";
                //}
                //if (example === "q8")
                //{
                    //document.getElementById("element_3").value =    "SELECT ?v0 ?v1 ?v2 ?v3 ?v4 ?v5 ?v6 WHERE \n" +
																	//"{\n" +
																	//"\t?v0 <http://www.w3.org/2004/02/skos/core#broader> ?v1 .\n" +
																	//"\t?v0 <http://dbpedia.org/ontology/child> ?v2 .\n" +
																	//"\t?v0 <http://dbpedia.org/ontology/spouse> ?v3 .\n" +
																	//"\t?v0 <http://dbpedia.org/ontology/profession> ?v4 .\n" +
																	//"\t?v0 <http://dbpedia.org/ontology/party> ?v5 .\n" +
																	//"\t?v0 <http://dbpedia.prg/ontology/militaryRank> ?v6 .\n" + 
																	//"}\n";
                //}
                //if (example === "q9")
                //{
                    //document.getElementById("element_3").value =    "SELECT ?v0 ?v1 ?v2 ?v3 WHERE\n" +
																	//"{\n" +
																	//"\t?V0 <http://dbpedia.org/ontology/location> ?v1 .\n" +
																	//"\t?v0 <http://dbpedia.org/ontology/typeOfElectrification> ?v2 .\n" +
																	//"\t?v0 <http://dbpedia.org/ontology/wikiPageWikiLink> <http://dbpedia.org/resource/Koltsevaya_Line> .\n" +
																	//"\t?v0 <http://dbpedia.org/property/wikiPageUsesTemplate> ?v3 .\n" +
																	//"}\n";
                //}
                //if (example === "q10")
                //{
                    //document.getElementById("element_3").value =    "SELECT ?v0 ?v1 ?v2 ?v3 WHERE\n" +
																	//"{\n" +
																	//"\t?v0 <http://dbpedia.org/property/hasPhotoCollection> ?v1 .\n" +
																	//"\t?v0 <xmlns.com/foaf/0.1/depiction> ?v2 .\n" +
																	//"\t?v0 <http://dbpedia.org/ontology/abstract> ?v3 .\n" +
																	//"\t?v0 <http://dbpedia.org/ontology/wikiPageWikiLink> <http://dbpedia.org/resource/Combination> .\n" +
																	//"}\n";
                //}
			}
