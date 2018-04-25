function build(db, ds){
	if(db != "" && ds != "")
	{
		var username1 = getCookie('user');
		var password1 = getCookie('pswd');
		var argu1 = "?operation=build&db_name=" + db + "&ds_path=" + ds + "&username=" + username1 + "&password=" + password1;
		var encodeArgu1 = escape(argu1);
		$.get(encodeArgu1, function(data, status){
			if(status=="success"){
				alert(data);
			}
		});
	}
	else
		alert("input needed.");
}

function load(db) {
	if(db != "")
	{
		var username2 = getCookie('user');
		var password2 = getCookie('pswd');
		var argu2 = "?operation=load&db_name=" + db + "&username=" + username2 + "&password=" + password2;
		var encodeArgu2 = escape(argu2);
		$.get(encodeArgu2, function(data, status){
			if(status=="success"){
				alert(data);
				//load database successfully, change the database name on web page
				//document.getElementById("_db_name_3").value = db;
			}
		});
	}
	else
		alert("input needed.");
}

function query(db, dp) {
	if(db != "")
	{
	var username3 = getCookie('user');
	var password3 = getCookie('pswd');

	var format = document.getElementById("result_format").value;
	if(format == "1")
		format = "html";
	else if(format == "2")
		format = "txt";
	else if(format == "3")
		format = "csv";
	else if(format == "4")
		format = "json";
	var argu3 = "?operation=query&username=" + username3 + "&password=" + password3 + "&db_name=" + db + "&format=" + format +"&sparql=" + dp;
	var encodeArgu3 = encodeURIComponent(argu3);
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
		element.setAttribute("href", encodeArgu3);
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
		$.get(encodeArgu3, function(data, status){
			if(status == "success"){
				if(data.indexOf('+') < 0)
					alert(data);
				else
				{
				//toTxt();
				//alert(data);
				var parts = data.split("+");
				var query_time = parts[1];
				var fileName = parts[3];
			    var lines = Number(parts[2]);
				//alert(lines);
				if(lines > 100)
					lines = 100;
				//alert(lines);
				for(var ip = 5; ip < parts.length; ip++)
				{
					parts[4] = parts[4] + "+" + parts[ip];
				}
				var items = parts[4].split("\n");
				//alert(items[0]);
				var valNum = items[0].split("?");
				var rows = valNum.length - 1;
				//alert(rows);
				var page = '<html><div align="left"><a href="javascript:void(0);" id="back" style="font-size:16px;color:blue">Click to Return</a>';
				page = page + '<a id="download" style="font-size:16px;margin-left:20px;color:blue">Click to Download</a>';
				page = page + '<a id="trick" style="display: none">Click to back</a>';
				page = page + '<p>Total answers: ' + parts[2] + '</p>';
				page = page + '<p>Query time: ' + parts[1] + 'ms</p>';
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
				//setTimeout(afterAns(fileName), 3000);
			
				var tmp1 = "?operation=download&filepath=" + fileName;
				var request1 = escape(tmp1);
				var element1 = document.getElementById("download");
				element1.setAttribute("href", request1);
				element1.setAttribute("download", "sparql.txt");
	
				var tmp2 = "?operation=delete&filepath=" + fileName;
				var request2 = escape(tmp2);	
				var element2 = document.getElementById("back");
				
				var tmp3 = "?operation=login&username=" + username3 + "&password=" + password3;
				var request3 = escape(tmp3);
				var element3 = document.getElementById("trick");
				element3.setAttribute("href", request3);
				//!Notice: element2 is a "<a>" tag, and it has two actions, href and onclick, be careful with the executing order of these two actions.
				//in this case, we use a call-back function to prevent strange things. we return to the origin web page after the request to delete file returns successfully.
				element2.onclick = function(){
					$.get(request2, function(data, status){
						//alert("delete return");
						//var element3 = document.getElementById("trick");
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
	else
		alert("input needed.");
}

/*
function afterAns(file_name){

}
*/
/*
	$.get(encodeArgu3, function(data, status){
		if(status=="success"){
			$("#queryAns").empty();
			var res = $("<p></p>").text(data);
			$("#queryAns").append(res);
			$("#queryAns").scrollTop($("#queryAns").height());
		}
	});
}
*/
function unload(db) {
	if(db != "")
	{
		var username4 = getCookie('user');
		var password4 = getCookie('pswd');
		var argu4 = "?operation=unload&db_name=" + db + "&username=" + username4 + "&password=" + password4;
		var encodeArgu4 = escape(argu4);
		$.get(encodeArgu4, function(data, status){
			if(status=="success"){
				alert(data);
				//unload database successfully, so set the database name on web page to be NULL
				//document.getElementById("_db_name_3").value = "NULL";
	
			}
		});
	}
	else
		alert("input needed.");
}
/*
function getDatabaseName(){
	//alert("getDatabaseName");
	var argu5 = "?operation=show&type=current";
	var encodeArgu5 = escape(argu5);
	$.get(encodeArgu5, function(data, status){
		if(status=="success"){
			document.getElementById("_db_name_3").value = data;
		}
		else{
			document.getElementById("_db_name_3").value = "NULL";
		}
	});
}
*/
function monitor() {
	var db_name6 = document.getElementById("_db_name_monitor").value;
	if(db_name6 != "")
	{
		var argu6 = "?operation=monitor&db_name=" + db_name6;
		var encodeArgu6 = escape(argu6);
		$.get(encodeArgu6, function(data, status){
			if(status=="success"){

			//alert(data);
			var ans = data.split("\n");
			//alert(ans.length);
			$("#monitorAns").empty();
		
			for(var i = 0; i < ans.length - 1; i++)
			{
				var res = $("<p></p>").text(ans[i]);
				$("#monitorAns").append(res);
			}
			
			$("#monitorAns").scrollTop($("#monitorAns").height());
	
			//document.getElementById("monitor_text").value = data;
		}
	});
	}
	else
		alert("input needed.");
}
function monitor_empty(){
	//alert("monitor_empty");
	$("#monitorAns").empty();
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
function addUser(username, password){
	if(username != "" && password != "")
	{
		var username7 = getCookie('user');
		var password7 = getCookie('pswd');
		var argu7 = "?operation=user&type=add_user&username1=" + username7 + "&password1=" + password7 + "&username2=" + username + "&addtion=" + password;
		var encodeArgu7 = escape(argu7);
		$.get(encodeArgu7, function(data, status){
			if(status == "success"){
				if(data != "operation succeeded.")
					alert(data);
				showUsers();
			}
		});
	}
	else
		alert("input needed.");
}
function delUser(username){
	if(username != "")
	{
		var username8 = getCookie('user');
		var password8 = getCookie('pswd');
		var argu8 = "?operation=user&type=delete_user&username1=" + username8 + "&password1=" + password8 + "&username2=" + username + "&addtion=";
		var encodeArgu8 = escape(argu8);
		$.get(encodeArgu8, function(data, status){
			if(status == "success"){
				if(data != "operation succeeded.")
					alert(data);
				showUsers();
			}
		});
	}
	else
		alert("input needed.");
}
function addPrivilege(username, type, db){
	if(username != "" && type != "" && db != "")
	{
		var username9 = getCookie('user');
		var password9 = getCookie('pswd');
		type = "add_" + type;
		var argu9 = "?operation=user&type=" + type + "&username1=" + username9 + "&password1=" + password9 + "&username2=" + username + "&addtion=" + db;
		var encodeArgu9 = escape(argu9);
		$.get(encodeArgu9, function(data, status){
			if(status == "success"){
				if(data != "operation succeeded.")
					alert(data);
				showUsers();
			}
		});
	}
	else
		alert("input needed.");
}
function delPrivilege(username, type, db){
	if(username != "" && type != "" && db != "")
	{
		var username11 = getCookie('user');
		var password11 = getCookie('pswd');
		type = "delete_" + type
		var argu11 = "?operation=user&type=" + type + "&username1=" + username11 + "&password1=" + password11 + "&username2=" + username + "&addtion=" + db;
		var encodeArgu11 = escape(argu11);
		$.get(encodeArgu11, function(data, status){
			if(status == "success"){
				if(data != "operation succeeded.")
					alert(data);
				showUsers();
			}
		});
	}
	else
		alert("input needed.");
}
function showUsers(){
	var argu12 = "?operation=showUsers";
	var encodeArgu12 = escape(argu12);
	$.get(encodeArgu12, function(data, status){
		if(status == "success"){
			/*
			//alert(data);
			var ans = data.split("\n");
			//alert(ans.length);
			$("#userAns").empty();
		
			for(var i = 0; i < ans.length - 1; i++)
			{
				var res = $("<p></p>").text(ans[i]);
				$("#userAns").append(res);
			}
			
			$("#userAns").scrollTop($("#userAns").height());
			*/
			//alert(data);
			$("#userAns").empty();

			var ans = data.split("\n");
			var res = '<table border="1" style="font-size:medium">';
			for(var ii = 0; ii < ans.length; ii++)
			{
				res = res + "<tr>";
				var item = ans[ii].split("\t");
				//alert(item.length);
				for(jj = 0; jj < item.length; jj++)
				{
					res = res + '<td>' + item[jj] + '</td>';
				}
				res = res + "<tr>";
			}
			res = res + '</table>';
			$("#userAns").append(res);
			$("#userAns").scrollTop($("#userAns").height());

		}
	});
}
function showDatabases() {
	//alert("showDatabases");
	var argu10 = "?operation=show";
	var encodeArgu10 = escape(argu10);
	//alert(encodeArgu10);
	$.get(encodeArgu10, function(data, status){
		if(status=="success"){

			//alert(data);
			var ans = data.split("\n");
			//alert(ans.length);
			$("#databasesAns").empty();
		
			for(var i = 0; i < ans.length - 1; i++)
			{
				var res = $("<p></p>").text(ans[i]);
				$("#databasesAns").append(res);
			}
			
			$("#databasesAns").scrollTop($("#databasesAns").height());
	
			//document.getElementById("monitor_text").value = data;
		}
	});
}
function databases_empty(){
	//alert("monitor_empty");
	$("#databasesAns").empty();
}
