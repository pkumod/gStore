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

function build(db, ds){
	if(db != "" && ds != "")
	{
		$("#myspin").text("");
		var target = $("#myspin").get(0);
		spinner.spin(target);

		var username1 = getCookie('user');
		var password1 = getCookie('pswd');
		var argu1 = "?operation=build&db_name=" + db + "&ds_path=" + ds + "&username=" + username1 + "&password=" + password1;
		var encodeArgu1 = escape(argu1);
		$.get(encodeArgu1, function(data, status){
		//	setTimeout(function(){spinner.spin();}, 300);
			if(status=="success"){
			spinner.spin();
			setTimeout(function(){alert(data.StatusMsg);}, 300);
			//alert(data.StatusMsg);
			}
		});
	}
	else
		alert("input needed.");
}

function load(db) {
	if(db != "")
	{
			$("#myspin").text("");
		var target = $("#myspin").get(0);
		spinner.spin(target);

		var username2 = getCookie('user');
		var password2 = getCookie('pswd');
		var argu2 = "?operation=load&db_name=" + db + "&username=" + username2 + "&password=" + password2;
		var encodeArgu2 = escape(argu2);
		$.get(encodeArgu2, function(data, status){
			//setTimeout(function(){spinner.spin();}, 300);
		
			if(status=="success"){
			spinner.spin();
			setTimeout(function(){alert(data.StatusMsg);}, 300);
			
				//	alert(data.StatusMsg);
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
		$("#myspin").text("");
		var target = $("#myspin").get(0);
		spinner.spin(target);
		$.get(encodeArgu3, function(data, status){
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
				page = page + '<a id="download" style="font-size:16px;margin-left:20px;color:blue">Click to Download</a>';
				page = page + '<a id="trick" style="display: none">Click to back</a>';
				page = page + '<p>Total answers: ' + data.AnsNum + '</p>';
				page = page + '<p>Query time: ' + data.QueryTime + '</p>';

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
				
				var tmp3 = "?operation=login&username=" + username3 + "&password=" + password3;
				var request3 = escape(tmp3);
				var element3 = document.getElementById("trick");
				element3.setAttribute("href", request3);
				//!Notice: element2 is a "<a>" tag, and it has two actions, href and onclick, be careful with the executing order of these two actions.
				//in this case, we use a call-back function to prevent strange things. we return to the origin web page after the request to delete file returns successfully.
				element2.onclick = function(){
							$("#myspin2").text("");
							var target = $("#myspin2").get(0);
							spinner.spin(target);

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
		$("#myspin").text("");
		var target = $("#myspin").get(0);
		spinner.spin(target);

		var username4 = getCookie('user');
		var password4 = getCookie('pswd');
		var argu4 = "?operation=unload&db_name=" + db + "&username=" + username4 + "&password=" + password4;
		var encodeArgu4 = escape(argu4);
		$.get(encodeArgu4, function(data, status){
			//setTimeout(function(){spinner.spin();}, 300);
		
			if(status=="success"){
			spinner.spin();
			setTimeout(function(){alert(data.StatusMsg);}, 300);
			
				//	alert(data.StatusMsg);
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
		var username = getCookie('user');
		var password = getCookie('pswd');
	
		$("#myspin").text("");
		var target = $("#myspin").get(0);
		spinner.spin(target);

		var argu6 = "?operation=monitor&db_name=" + db_name6 + "&username=" + username + "&password=" + password;
		var encodeArgu6 = escape(argu6);
		$.get(encodeArgu6, function(data, status){
			//setTimeout(function(){spinner.spin();}, 300);
			spinner.spin();
			if(status=="success"){
				var obj = data;
                $("#monitorAns").empty();
				for(var item in obj)
				{
					if(item != "StatusCode" && item != "StatusMsg")
					{
                        var res = $("<p></p>").text(item + ":\t" + obj[item]);
                        $("#monitorAns").append(res);
					}
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

function addUser(username, password){
	if(username != "" && password != "")
	{
			$("#myspin").text("");
		var target = $("#myspin").get(0);
		spinner.spin(target);

		var username7 = getCookie('user');
		var password7 = getCookie('pswd');
		var argu7 = "?operation=user&type=add_user&username1=" + username7 + "&password1=" + password7 + "&username2=" + username + "&addition=" + password;
		var encodeArgu7 = escape(argu7);
		$.get(encodeArgu7, function(data, status){
			//setTimeout(function(){spinner.spin();}, 300);
		
			if(status == "success"){
				//if(data.StatusCode != 906)
			//		alert(data.StatusMsg);
				//showUsers();
			//setTimeout(function(){spinner.spin();}, 300);
				spinner.spin();
			setTimeout(function(){alert(data.StatusMsg);}, 300);
	
			}
		});
	}
	else
		alert("input needed.");
}
function delUser(username){
	if(username != "")
	{
		$("#myspin").text("");
		var target = $("#myspin").get(0);
		spinner.spin(target);

		var username8 = getCookie('user');
		var password8 = getCookie('pswd');
		var argu8 = "?operation=user&type=delete_user&username1=" + username8 + "&password1=" + password8 + "&username2=" + username + "&addition=";
		var encodeArgu8 = escape(argu8);
		$.get(encodeArgu8, function(data, status){
			//setTimeout(function(){spinner.spin();}, 300);
		
			if(status == "success"){
				//if(data.StatusCode != 906)
			//		alert(data.StatusMsg);
				//showUsers();
			//setTimeout(function(){spinner.spin();}, 300);
				spinner.spin();
			setTimeout(function(){alert(data.StatusMsg);}, 300);
	
			}
		});
	}
	else
		alert("input needed.");
}
function changePsw(username, password){
	if(username != "" && password != "")
	{
		$("#myspin").text("");
		var target = $("#myspin").get(0);
		spinner.spin(target);

		var username77 = getCookie('user');
		var password77 = getCookie('pswd');
		var argu77 = "?operation=user&type=change_psw&username1=" + username77 + "&password1=" + password77 + "&username2=" + username + "&addition=" + password;
		var encodeArgu77 = escape(argu77);
		$.get(encodeArgu77, function(data, status){
			//setTimeout(function(){spinner.spin();}, 300);
		
			if(status == "success"){
				//if(data.StatusCode != 906)
		//			alert(data.StatusMsg);
				//showUsers();
				if(username == "root")
					setCookie('pswd',password,7);
		//	setTimeout(function(){spinner.spin();}, 300);
				spinner.spin();
			setTimeout(function(){alert(data.StatusMsg);}, 300);
	
			}
		});
	}
	else
		alert("input needed.");
}
function addPrivilege(username, type, db){
	if(username != "" && type != "" && db != "")
	{
		$("#myspin").text("");
		var target = $("#myspin").get(0);
		spinner.spin(target);

		var username9 = getCookie('user');
		var password9 = getCookie('pswd');
		type = "add_" + type;
		var argu9 = "?operation=user&type=" + type + "&username1=" + username9 + "&password1=" + password9 + "&username2=" + username + "&addition=" + db;
		var encodeArgu9 = escape(argu9);
		$.get(encodeArgu9, function(data, status){
			if(status == "success"){
				//if(data.StatusCode != 906)
		//			alert(data.StatusMsg);
				//showUsers();
		//	setTimeout(function(){spinner.spin();}, 300);
				spinner.spin();
			setTimeout(function(){alert(data.StatusMsg);}, 300);
	
			}
		});
	}
	else
		alert("input needed.");
}
function delPrivilege(username, type, db){
	if(username != "" && type != "" && db != "")
	{
		$("#myspin").text("");
		var target = $("#myspin").get(0);
		spinner.spin(target);

		var username11 = getCookie('user');
		var password11 = getCookie('pswd');
		type = "delete_" + type
		var argu11 = "?operation=user&type=" + type + "&username1=" + username11 + "&password1=" + password11 + "&username2=" + username + "&addition=" + db;
		var encodeArgu11 = escape(argu11);
		$.get(encodeArgu11, function(data, status){
			//setTimeout(function(){spinner.spin();}, 300);
		
			if(status == "success"){
				//if(data.StatusCode != 906)
		//			alert(data.StatusMsg);
				//showUsers();
		//	setTimeout(function(){spinner.spin();}, 300);
				spinner.spin();
			setTimeout(function(){alert(data.StatusMsg);}, 300);
	
			}
		});
	}
	else
		alert("input needed.");
}
function showUsers(){
		$("#myspin").text("");
		var target = $("#myspin").get(0);
		spinner.spin(target);

		var username = getCookie('user');
		var password = getCookie('pswd');
	
	//alert("showDatabases");

	var argu12 = "?operation=showUser&username=" + username + "&password=" + password;
	var encodeArgu12 = escape(argu12);
	$.get(encodeArgu12, function(data, status){
		//setTimeout(function(){spinner.spin();}, 300);
		spinner.spin();
		if(status == "success"){
			$("#userAns").empty();
			var obj = data.ResponseBody;
            var res = '<table border="1" style="font-size:medium">';
			res = res + "<tr>";
            for(var item in obj[0])
            {
                res = res + "<th>" + item + "</th>";
            }
            res = res + "</tr>";

            for (var i = 0; i < obj.length; i++)
            {
            	var one_user = obj[i];
                res = res + "<tr>";
                for(var one_user_key in one_user)
                {
                    res = res + '<td>' + one_user[one_user_key] + '</td>';
                }
                res = res + "</tr>";
            }
			res = res + '</table>';

			$("#userAns").append(res);
			$("#userAns").scrollTop($("#userAns").height());

		}
	});
}
function showDatabases() {
		$("#myspin").text("");
		var target = $("#myspin").get(0);
		spinner.spin(target);
		
		var username = getCookie('user');
		var password = getCookie('pswd');
	
	//alert("showDatabases");
	var argu10 = "?operation=show&username=" + username + "&password=" + password;
	var encodeArgu10 = escape(argu10);
	//alert(encodeArgu10);
	$.get(encodeArgu10, function(data, status){
		//setTimeout(function(){spinner.spin();}, 300);
		
		if(status=="success"){
			spinner.spin();
            var obj = data;
            var body = obj.ResponseBody;
            $("#databaseAns").empty();
            for(var i = 0; i < body.length; i++)
			{
				var info = body[i];
				//alert(info);
                for(var item in info)
                {
                	//alert(item);
					var res = $("<p></p>").text(item + ":\t" + info[item]);
					$("#databasesAns").append(res);

                }
                var space_line = $("<p></p>").text("-------------------------");
                $("#databasesAns").append(space_line);
			}

            $("#databasesAns").scrollTop($("#databasesAns").height());


			//setTimeout(function(){spinner.spin();}, 300);
		
			//document.getElementById("monitor_text").value = data;
		}
	});
}
function databases_empty(){
	//alert("monitor_empty");
	$("#databasesAns").empty();
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
