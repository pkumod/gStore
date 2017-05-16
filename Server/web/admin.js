function build(db, ds){
	$.get("build/" + db + "/" + ds, function(data, status){
		if(status=="success"){
			alert(data);
		}
	});
}
  1
function load(db) {
	$.get("load/" + db, function(data, status){
		if(status=="success"){
			alert(data);
		}
	});
}

function query(dp) {
	$.get("query/" + dp, function(data, status){
		if(status=="success"){
			$("#queryAns").empty();
			var res = $("<p></p>").text(data);
			$("#queryAns").append(res);
			$("#queryAns").scrollTop($("#queryAns").height());
		}
	});
}

function unload(db) {
	$.get("unload", function(data, status){
		if(status=="success"){
			alert(data);
		}
	});
}
