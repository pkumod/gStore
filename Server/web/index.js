var endpoint_username = "endpoint";
var endpoint_password = "123";
var opts = {

	lines: 13, // 花瓣数目
	length: 20, // 花瓣长度
	width: 10, //
	scale: 0.45,
	radius: 30, // 花瓣距中心半径
	corners: 1, // 花瓣圆滑度 (0-1)
	rotate: 0, // 花瓣旋转角度
	direction: 1, // 花瓣旋转方向 1: 顺时针, -1: 逆时针
	color: '#5882FA', // 花瓣颜色
	speed: 1, // 花瓣旋转速度
	trail: 60, // 花瓣旋转时的拖影(百分比)
	shadow: false, // 花瓣是否显示阴影
	hwaccel: false, //spinner 是否启用硬件加速及高速旋转
	className: 'spinner', // spinner css 样式名称
	zIndex: 2e9, // spinner的z轴 (默认是2000000000
	top: '46%', // spinner 相对父容器Top定位 单位 px
	left: '50%',// spinner 相对父容器Left定位 单位 px
	position: 'absolute'
};
var spinner = new Spinner(opts);
function query(dp) {
	//setCookie to record the previous query
	setCookie('sparql', encodeURIComponent(dp), 7);	
	var encodeVal = escape(dp);
	//alert(encodeVal);
	var format = document.getElementById("element_5").value;
	var db_name = document.getElementById("element_1").value;
	if(format == "1")
		format = "html";
	else if(format == "2")
		format = "txt";
	else if(format == "3")
		format = "csv";
	else if(format == "4")
		format = "json";
	var argu = "?operation=query&username=" + endpoint_username + "&password=" + endpoint_password + "&db_name=" + db_name + "&format=" + format +"&sparql=" + dp;

//	var argu = "?operation=query&db_name=" + db_name +"&format=" + format + "&sparql=" + dp;
	var encodeArgu = encodeURIComponent(argu);
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
		$("#myspin").text("");
		var target = $("#myspin").get(0);
		spinner.spin(target);
		$.get(encodeArgu, function(data, status){
		//	setTimeout(function(){
		//		spinner.spin();
		//	}, 300);
			if(status=="success"){
				if(data.StatusCode != 0)
				{	
				//	alert(data.StatusMsg);
						spinner.spin();
						setTimeout(function(){alert(data.StatusMsg);}, 300);
				}
				else
				{
				//toTxt();
				//alert(data);
				var obj = data;
				var query_time = obj.QueryTime;
				var fileName = obj.Filename;
				var lines = obj.AnsNum;
				if(lines > 100)
					lines = 100;
				var keys = obj.head.vars;
				var items = obj.results.bindings;
				var rows = keys.length-1;
				var page = '<html><div id="myspin2"></div><div align="left"><a href="javascript:void(0);" id="back" style="font-size:16px;color:blue">Click to Return</a>';
				page = page + '<a id="download" style="font-size:16px;margin-left:20px">Click to Download</a>';
				page = page + '<a href="/" id="trick" style="display: none">Click to back</a>';
				page = page + '<p>Total answers: ' + obj.AnsNum + '</p>';
				page = page + '<p>Query time: ' + obj.QueryTime + '</p>';
				if(obj.AnsNum > 100)
				{
					
					page = page + '<p>Number of answers is too large, show only 100 of them, click to download the full result!</p>'; 
					
				}
				page = page + '</div><table border="1" style = "font-size:medium">';
				page = page + "<tr>";
		
				for(var ii = 0; ii <= rows; ii++)
				{
					page = page + "<th>" + "?" + keys[ii] + "</th>";
				}
				page = page + "</tr>";
				var i = 1;
				var j = 0;
				for (i = 0; i <= items.length - 1; i++)
				{
					page = page + "<tr>";
					var item = items[i];
					//alert(items[i]);	
					for(var prop in item)
					{	
						page = page + '<td>' + item[prop].value + '</td>';
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
						$("#myspin2").text("");
						var target = $("#myspin2").get(0);
						spinner.spin(target);

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
				};
			
				}

		}
		});
	}
}

function handleQueryExample()
            {
			
		        var example = document.getElementById("example").value;
			
				/*
				if(example === "q1")
				{
						document.getElementById("element_3").value = "select ?x\n" +
						"where\n" +
						"{ \n" +
								"\t?x <http://purl.org/dc/terms/relation> <http://zhonto.org/resource/Giga>.\n" +
						"} \n";
				}
	            if(example === "q2")
				{
						document.getElementById("element_3").value = "select ?x ?y\n" +
						"where\n" +
						"{ \n" +
								"\t?x <http://purl.org/dc/terms/references> ?y.\n" +
						"}\n";
				}

				if(example === "q3")
				{
						document.getElementById("element_3").value = "select ?x\n" + 
						"where\n" +
						"{ \n" +
								"\t?y <http://purl.org/dc/terms/relation> <http://zhonto.org/resource/2004_MD6>.\n" +
								"\t?y <http://www.w3.org/2002/07/owl#sameAs> ?x.\n" +
						"} \n";
				}
				
                if (example === "q1")
                {
                    document.getElementById("element_3").value =    "select distinct ?x where \n" +
																	"{ \n" +
																	"\t?x	<rdf:type>	<ub:GraduateStudent>. \n" +
																	"\t?y	<rdf:type>	<ub:University>. \n" +
																	"\t?z	<rdf:type>	<ub:Department>. \n" +
																	"\t?x	<ub:memberOf>	?z. \n" +
																	"\t?z	<ub:subOrganizationOf>	?y. \n" +
																	"\t?x	<ub:undergraduateDegreeFrom>	?y. \n" +
																	"} \n";
                }
				*/
          /* 
				 if(example === "q1")
				{
					var last_sparql = decodeURIComponent(getCookie('sparql'));
					if(last_sparql != '')
					{
						document.getElementById("element_3").value = last_sparql;
						setCookie('sparql', '', -1);
					}
					else
					{
						document.getElementById("element_3").value = "select ?x ?y \n" +
							"where \n " +
							"{ \n" +
									"\t<北京大学> ?x ?y. \n" +

							"} \n";

					}
				}

				if(example === "q2")
				{
						document.getElementById("element_3").value = "select ?x \n" +
							"where \n" +
							"{ \n" +
									"\t<刘昊然> <代表作品> ?x. \n" +

							"} \n ";
				}
				if(example === "q3")
				{
						document.getElementById("element_3").value = "select ?x ?y \n" +
							"where \n" +
							"{ \n" +
									"\t?x <校庆日> ?y. \n" +

							"} \n";
				}
            */   
              
				if (example === "q1")
                {
					var last_sparql = decodeURIComponent(getCookie('sparql'));
					if(last_sparql != '')
					{
						document.getElementById("element_3").value = last_sparql;
						setCookie('sparql', '', -1);
					}
					else
					{
                    document.getElementById("element_3").value =    "select distinct ?x where \n" +
																	"{ \n" +
																	"\t?x    <rdf:type>    <ub:UndergraduateStudent>. \n" +
																	"\t?y    <ub:name> <Course1>. \n" +
																	"\t?x    <ub:takesCourse>  ?y. \n" +
																	"\t?z    <ub:teacherOf>    ?y. \n" +
																	"\t?z    <ub:name> <FullProfessor1>. \n" +
																	"\t?z    <ub:worksFor>    ?w. \n" +
																	"\t?w    <ub:name>    <Department0>. \n" +
																	"} \n";

					}
				}
                if (example === "q2")
                {
                    document.getElementById("element_3").value =    "select distinct ?x where \n" +
																	"{ \n" +
																	"\t?x	<rdf:type>	<ub:Course>. \n" +
																	"\t?x	<ub:name>	?y. \n" +
																	"} \n";
				} 
                if (example === "q3")
                {
                    document.getElementById("element_3").value =    "select distinct ?x where \n" +
																	"{ \n" +
																	"\t?x    <rdf:type>    <ub:UndergraduateStudent>. \n" +
																	"}\n";
                }
                
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
//设置cookie
function setCookie(name,value,day){
    //alert("setCookie");
    //alert(name);
    //alert(value);
    var date = new Date();
    date.setDate(date.getDate() + day);
    //alert(date);
    document.cookie = name + '=' + value + ';expires='+ date;
    //alert("document.cookie");
    //alert(document.cookie);
  }
//获取cookie
function getCookie(name){
    //alert("getCookie");
    //alert(name);
    var reg = RegExp(name+'=([^;]+)');
    var arr = document.cookie.match(reg);
    if(arr){
      //alert("cookie got");
      return arr[1];
    }else{
      //alert("no cookie");
      return '';
    }
}
