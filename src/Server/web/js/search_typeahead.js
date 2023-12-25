var start = 0;
var end = 0;
$(document).ready(function() {
/*
    var typeaheadCate = new Bloodhound({
        datumTokenizer: Bloodhound.tokenizers.obj.whitespace('name'),
        queryTokenizer: Bloodhound.tokenizers.whitespace,
        limit: 3,
        remote: {
			url:'http://172.31.222.72:9000/bdbkDemo/json/getTypeaheadCateNames.php?query=%QUERY&size=4',
            wildcard: '%QUERY',
            filter: function (cates) {
                return $.map(cates.results, function (cate) {
                    return {
                        cateName : cate.cateName,
                        cateId : cate.cateTagId
                    };
                });
            }
        }
    });
*/
    var typeaheadEnt = new Bloodhound({
        datumTokenizer: Bloodhound.tokenizers.obj.whitespace('entityName'),
        queryTokenizer: Bloodhound.tokenizers.whitespace,
       // limit: 10,
        remote: {
			//url:'http://172.31.222.72:9000/bdbkDemo/json/getTypeaheadEntityNames.php?query=%QUERY',
			//url:'http://entity-dbpedia.gstore-pku.com/search?indexPath=/home/lijing/data/index/entity_index&name=%QUERY',
			url: 'http://entity.gstore-pku.com/name/%QUERY',
			wildcard: '%QUERY',

			filter: function (ents) {
				return $.map(ents, function (ent) {
                    return {
                        entityName : ent.entityName
                    };
                });
			
            }

	
		/*
		transform: function(result){
			//alert("success");
			var resultList = result.map(function (item){
				var aItem = {entityName: item.entityName};
				return JSON.stringify(aItem);
			});
			return resultList;
		}
*/
	/*
		transform:function(ents){
		alert(ents);
			return ents;
		}
		*/
		
    }
	});


    $('#multiple-datasets .typeahead').typeahead({
            highlight: true,
            hint: false,
        },
  /*      {
            name: 'category',
            display: 'cateName',
            source: typeaheadCate.ttAdapter(),
            templates: {
                header: '<h3 class="cate-name">类别</h3>',
                suggestion: Handlebars.compile('<p>{{cateName}}</p>'),
                empty: ['<h4 class="empty-message">没有相关类别</h4>'].join('\n')
            }
        },
    */
		{
            name: 'entity',
            display: 'entityName',
            source: typeaheadEnt.ttAdapter(),
			limit: 100,
		//source: promise.ttAdapter(),
		  
		templates: {
              //  header: '<h3 class="cate-name">实体</h3>',
                empty: ['<div class="empty-message">没有相关实体</div>'].join('\n'),
                suggestion: Handlebars.compile('<p>{{entityName}}</p>')
            },
			
		});
   
	$('#multiple-datasets .typeahead')
        .on('typeahead:selected', function(event, selected_object, dataset) {
            $("#multiple-datasets .typeahead").typeahead('close');
			document.getElementById("multiple-datasets").style.display = "none";
			console.log(selected_object);
			/*
            if(selected_object && selected_object.cateName)
                redrawSVG('category', selected_object.cateId, false);
            else
                redrawSVG('entity', selected_object.entityId, false);
			*/
			var tempStr = document.getElementById("element_3").value;
			var part1 = tempStr.substring(0, start);
			var part2 =    $("#multiple-datasets .typeahead").typeahead('val');
			var part3 = tempStr.substring(end + 1, tempStr.len);

			$("#element_3").focus();
			var putIn = part1;
			if(tempStr[start-1] != "<")
				putIn += "\t<";
			putIn += part2;
			if(tempStr[end+1] != ">")
				putIn += ">\t";
			var curP = putIn.length;
			//alert(curP);
			putIn += part3;
	        //document.getElementById("element_3").value = part1 + "\t<" +  part2 + "\t>" + part3;
			document.getElementById("element_3").value = putIn;
			setCursorPosition(document.getElementById("element_3"), curP);
		
			//updatePredColumn();
			//$("#element_3").focus();
		//	document.getElementById("multiple-datasets").style.display = "none";
			getPredicateList(part2);
        })
        .keypress(function(event){
			if(event.keyCode == 13){
                $("#multiple-datasets .typeahead").blur();
                //redrawSVG('entity', $('.typeahead').val(), true);
			document.getElementById("multiple-datasets").style.display = "none";
		
            }
			
        });

});
function getPredicateList(subject_entity)
{
	var dp = "select distinct ?x where { <" + subject_entity + "> ?x ?y. }";
	//var encodeArgu = encodeURIComponent(argu);
	var argu = "?operation=query&username=endpoint&password=123&db_name=" + document.getElementById("element_1").value + "&format=html&sparql=" + dp;
	var encodeArgu = encodeURIComponent(argu);
	$.get(encodeArgu, function(data, status){
			if(status=="success"){
				//toTxt();
				//alert(data);
				var items = new Array();
				var parts = data.results.bindings;
				for(var i = 0; i < parts.length; i++)
				{
					var part = parts[i];
					items[i] = part.x.value;
				}
				updatePredColumn(items);
		}
	});
}

function getCursorPosition(t)
{
	if(document.selection) 
	{
		// IE support
		t.focus();
		var ds = document.selection;
		var range = ds.createRange();
		var stored_range = range.duplicate();
		stored_range.moveToElementText(t);
		stored_range.setEndPoint("EndToEnd", range);
		t.selectionStart = stored_range.text.length - range.text.length;
		t.selectionEnd = t.selectionStart + range.text.length;
		return t.selectionStart;
	}
	else
		return t.selectionStart
	    
}

function setCursorPosition(textDom, pos)
{
if(textDom.setSelectionRange) {
	        // IE Support
			         textDom.focus();
			                 textDom.setSelectionRange(pos, pos);
			                     
}else if (textDom.createTextRange) {
	        // Firefox support
			         var range = textDom.createTextRange();
			                 range.collapse(true);
			                         range.moveEnd('character', pos);
			                                 range.moveStart('character', pos);
			                                         range.select();
			                                             
}
}

function getCandidateStart(pos, str)
{
	var startIndex = 0;
	
	var len = str.length;
	
	for(var ii = pos-1; ii >= 0; ii--)
	{
		var temp = str[ii];
		//console.log("temp: " + temp);
		if(temp == '<' || temp == '{' ||  temp == ' ' || temp == '\t' || temp == '\n')
		{
			startIndex = ii + 1;
			break;
		}
	}
	var stopFlag = str[startIndex];
	if(stopFlag == '<')
		startIndex++;

	return startIndex;

}
function getCandidateEnd(pos, str)
{
	var len = str.length;

	var endIndex = len - 1;

	for(var jj = pos; jj < len; jj++)
	{
		//alert("yessss");
		var tempp = str[jj];
		//console.log("tempp: " + tempp);
		if(tempp == '>' || tempp == '}' || tempp == ' ' || tempp == '\t' || tempp == '\n')
		{
			endIndex = jj - 1;
			break;
		}
	}
	var stopFlag = str[endIndex];
	if(stopFlag == '>')
		endIndex--;
	return endIndex;
}


function updatePredColumn(preds) {
	var innerHTML = "<label class=\"descripiton\" for=\"predicateList\">谓词列表\n" ;
		/*
    var innerHTML =     "<div class=\"panel panel-default\">\n"+
        "    <div class=\"panel-heading\" data-toggle=\"collapse\" href=\"#predicateList\" style='cursor:pointer'>\n"+
        "        <h3 class=\"panel-title\"><a data-toggle=\"collapse\" href=\"#predicateList\">\n"+
        "            谓词列表"+
        "        </a></h3>\n"+
        "    </div>\n"+
		*/
    //    "    <div class=\"panel-body collapse\" id=\"predicateList\">\n";
       innerHTML += "<div class='row'> <div role=\"group\" aria-label=\"Basic example\">\n";
       
		for(var i = 0; i < preds.length; i++){
        
        //if(i % 3 == 1)
          innerHTML += '<button id="predButton' + i + '" type="button" ' +
            'class="btn btn-secondary ' +  '" ' +
            'onclick="buttonClick(\'' + preds[i] + '\')"' +
            '>' + preds[i] + '</button>'
        //if(i % 3 == 0)
          //  innerHTML += '</div></div>\n';
    }
   // if(i % 3 != 0)
        innerHTML += '</div></div>\n';
    innerHTML += '</div></div>';
    $("#buttonlist").html(innerHTML);
  //  $("#buttonlist").show().animate( { "opacity": "1", top:"20"} , 1500); // 开启加载图标
}

function buttonClick(pred)
{
	var tempStr = document.getElementById("element_3").value;
	
	var ele2 = document.getElementById("element_3");
	var cursorPos = getCursorPosition(ele2);
	//alert(cursorPos);
	var part1 = tempStr.substring(0, cursorPos);
			var part2 =    pred;
			var part3 = tempStr.substring(cursorPos, tempStr.len);
			var putIn =  part1 + "\t<" + part2 + ">\t";
			var curP = putIn.length;
			//alert(curP);
	        document.getElementById("element_3").value = putIn + part3;
			
			setCursorPosition(document.getElementById("element_3"), curP);
		
		
}

function handlekeyup(event)
{
	if(event.keyCode == 13)
	{
		document.getElementById("multiple-datasets").style.display = "none";
		return;
	}
	//globalvalue = myvalue;
	if($("#multiple-datasets .typeahead").typeahead('val') != "")
	{
		if(event.keyCode == 40 || event.keyCode == 38){
			$("#multiple-datasets .typeahead").focus();
			return;	
		}
	}
	var ele1 = 	document.getElementById("searchbox");
	var ele2 = document.getElementById("element_3");
	var myvalue = ele2.value;
	var cursorPos = getCursorPosition(ele2);
	//console.log("cursorPos = " + cursorPos);
	var len2 = myvalue.length;
	//console.log("myvalue.length = " + len2);
	//console.log(myvalue.substring(len2-1) == "\n");
	
	start = getCandidateStart(cursorPos, myvalue);
	end = getCandidateEnd(cursorPos, myvalue);
	
	//console.log("start: " + start);
	//console.log("end: " + end);
	var str = myvalue.substring(start, end + 1);
	if(str == "" || str.substring(0, 1) == "?")
		document.getElementById("multiple-datasets").style.display = "none";
	else
	{
		var element = document.getElementById("element_3");
		 var coordinates = getCaretCoordinates(element, element.selectionEnd);
		           console.log('(top, left, height) = (%s, %s, %s)', coordinates.top, coordinates.left, coordinates.height);
	         
		document.getElementById("multiple-datasets").style.left = element.offsetLeft - element.scrollLeft + coordinates.left + 5 + 'px';
	   
		document.getElementById("multiple-datasets").style.top = element.offsetTop - element.scrollTop + coordinates.top + 10 +'px';
	   
		document.getElementById("multiple-datasets").style.display = "";
		$('#multiple-datasets .typeahead').typeahead('val', str);
		$('.typeahead').typeahead('open'); 
	}
	/*
	var myvalue = ele2.value
	//var len1 = globalvalue.length;
	var len2 = myvalue.length;
	var n = myvalue.lastIndexOf("<");
	var inputchar = myvalue.substring(len2-1);
	console.log(inputchar);
	*/
//	if(inputchar == "<")

	/*
	if(inputchar == " " || inputchar == "\r" || inputchar == "\t")
	{
		$('#multiple-datasets .typeahead').typeahead('val', "");
		$('.typeahead').typeahead('close');
		console.log(globalvalue);
		
	}

	console.log(len1);
	var str = myvalue.substring(len1);
	
	$('#multiple-datasets .typeahead').typeahead('val', str);
	$('.typeahead').typeahead('open'); 
*/
	/*setTimeout("check()", 100);
*/

}

function fireKeyEvent(el, evtType, keyCode) {
	alert(el.value);
	var evtObj;
	if (document.createEvent) 
	{
		if (window.KeyEvent) 
		{//firefox 浏览器下模拟事件
			 evtObj = document.createEvent('KeyEvents');
			 evtObj.initKeyEvent(evtType, true, true, window, true, false, false, false, keyCode, 0);
		} 
		else 
		{//chrome 浏览器下模拟事件
			 evtObj = document.createEvent('UIEvents');
			 evtObj.initUIEvent(evtType, true, true, window, 1);
													
			 delete evtObj.keyCode;
			 if (typeof evtObj.keyCode === "undefined") 
			 {//为了模拟keycode
				Object.defineProperty(evtObj, "keyCode", { value: keyCode  });                       
			 } 
			 else 
			 {
				evtObj.key = String.fromCharCode(keyCode);
			 }										
			 if (typeof evtObj.ctrlKey === 'undefined') 
			 {//为了模拟ctrl键
				Object.defineProperty(evtObj, "ctrlKey", { value: true  });
			 }
			 else
			 {
				evtObj.ctrlKey = true;
			}
		}
		el.dispatchEvent(evtObj);
										            
	} 
	else if (document.createEventObject) 
	{//IE 浏览器下模拟事件
		evtObj = document.createEventObject();
		evtObj.keyCode = keyCode
		el.fireEvent('on' + evtType, evtObj);
	}
				        
}
/*
function handlekeyup(event)
{
	var ele = 	document.getElementById("searchbox");

	cCode = event.keyCode;
	//alert(cCode);
	fireKeyEvent(ele, "keydown", cCode);
	fireKeyEvent(ele, "keypress", cCode);
	fireKeyEvent(ele, "keyup", cCode);
}
*/
