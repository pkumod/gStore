/**
 * Created by hanzhe on 16-12-5.
 */

//画力导图
function DisplayTriple() {
    // 中心点固定居中
    json.nodes[0].fixed = !json.nodes[0].fixed;
    json.nodes[0].x = width/2 - 100;
    json.nodes[0].y = height/2;

    force = d3.layout.force()
        .nodes(json.nodes)
        .links(json.edges)
        .size([width, height])
        .linkDistance(function (d) {return (100 + Math.random()*100)*3;})
        .charge(-300)
        .on("tick", tick)
        .start();

    link = svg.selectAll(".link")
        .data(json.edges)
        .enter().append("line")
        .attr("class", "link")
        .style("stroke", function (d) {return getColorHexMDC(d.color, 400);})
        .style("stroke-width", defaultEdgeWidth);


    node = svg.selectAll(".node")
        .data(json.nodes)
        .enter().append("g")
        .attr("class", "node")
        .call(force.drag);
    entNodes = node.filter(function (d){ return d.type == 'entity';});
    AppendMouseBlurEffect(entNodes, 0.6);

    // 用文本框显示文字
    // TODO: 现在只能mouseover识别字， 鼠标停留在文本框的空白部分无效。。。
    var strNodes = node.filter(function (d){ return d.type != 'entity';});
    strNodes.append("foreignObject")
        .attr({
            'id' : function (d) { return "textNode" + d.id;},
            'width' : function (d) {return getTextWindowWidth_leave(d.name, fontsize, 10);},
            'x' : function (d) {return -0.5*getTextWindowWidth_leave(d.name, fontsize, 10);},
            'height' : function (d) { return getTextWindowHeight_leave(d.name, fontsize, 10);},
            'y' : function (d) { return -0.5*getTextWindowHeight_leave(d.name, fontsize, 10);}
            ,'class': 'textbox'
        })
        .html(function (d) { return "<div class='textbox'><p>" + getTitle_leave(d.name) + "</p></div>"; })
        .on("mouseover", function (d, i) {
            // 显示全部信息
            var curTxtNode = d3.select("#textNode" + d.id);
            //console.error("in :" + d.id + " " + curTxtNode);
            curTxtNode.attr("width", function (d) {return getTextWindowWidth_enter(d.name, fontsize, 10);});
            curTxtNode.attr("height", function (d) {return getTextWindowHeight_enter(d.name, fontsize, 10);});
            curTxtNode.attr("class", "textbox");
            curTxtNode.html(d.name);
        })
        .on("mouseout", function (d, i) {
            //var curTxtNode = d3.selectAll(this.getElementsByTagName("foreignObject"))
            var curTxtNode = d3.select("#textNode" + d.id);
            curTxtNode.attr("width", function (d) {return getTextWindowWidth_leave(d.name, fontsize, 10);});
            curTxtNode.attr("height", function (d) {return getTextWindowHeight_leave(d.name, fontsize, 10);});
            curTxtNode.attr("class", "textbox");
            curTxtNode.html(function (d) { return getTitle_leave(d.name); });
        });

    //添加连线上的文字
    var edges_text;
    if(DEBUGMODE == 1) {
        edges_text = svg.selectAll(".linetext")
            .data(json.edges)
            .enter()
            .append("text")
            .attr("class", "linetext")
            .style("fill", function (d, i) {
                return getColorHexMDC(d.color, 900);
            })
            .text(function (d) {
                return d.id + "-" + d.description;
            })
            .style("font-size", "24px")
            .style("fill-opacity", function (edge) {
                edge.opacity = 1;
                return 1;
            });
    }

    force
        .nodes(json.nodes)
        .links(json.edges)
        .on("tick", tick)
        .start();

    var time = 1;

    function tick() {
        node.attr("transform", function (d) {
            if(isNaN(d.x)) d.x = 0;
            if(isNaN(d.y)) d.y = 0;
            return "translate(" + d.x + "," + d.y + ")";
        });
        link.attr("x1", function(d) { return d.source.x; })
            .attr("y1", function(d) { return d.source.y; })
            .attr("x2", function(d) { return d.target.x; })
            .attr("y2", function(d) { return d.target.y; });
        if(DEBUGMODE == 1) {
            edges_text
                .attr("x", function (d) {
                    return (d.source.x + d.target.x) / 2;
                })
                .attr("y", function (d) {
                    return (d.source.y + d.target.y) / 2;
                });
        }
    }
}


//加载 node->{linked edges}, edge->{linked nodes}, {edge status}, {node status} 表
function loadEdgeMaps() {
    edge2linkedNodes = new Map();
    edgeVisStatus = new Map();
    edgeColor2Edges = new Map();
    edgeColor2Name = new Map();
    $.each(json.edges, function (i, d){
        json.edges[i].source = parseInt(d.source);
        json.edges[i].target = parseInt(d.target);
        json.edges[i].value = 5;
        json.edges[i].id = i;
        var souceEntityId = nodeSeqId2nodeId[json.edges[i].source];
        var targetEntityId = nodeSeqId2nodeId[json.edges[i].target];
        var edgeColor = json.edges[i].color;
        var edgeName = json.edges[i].description;
        edge2linkedNodes[i] = new Set();
        edge2linkedNodes[i].add(souceEntityId);
        edge2linkedNodes[i].add(targetEntityId);
        node2linkedEdges[souceEntityId].add(i);
        node2linkedEdges[targetEntityId].add(i);
        edgeColorVisStatus[i] = true;
        if(edgeColor2Edges[edgeColor] == undefined)
            edgeColor2Edges[edgeColor] = new Set();
        edgeColor2Edges[edgeColor].add(i);
        edge2Color[i] = edgeColor;
        edgeVisStatus[i] = true;
        edgeColor2Name[edgeColor] = edgeName;
    })
}

//改变谓词的可见性
function changeNodeAndEdgeStatus(edgeColorId) {
    if(edgeColorVisStatus[edgeColorId] == false){ // 原来不可见,改成可见
        $("#predButton" + edgeColorId).removeClass(getColorClassMDC(edgeColorId, 100)).addClass(getColorClassMDC(edgeColorId, 400)); // 改变图标的状态
        edgeColorVisStatus[edgeColorId] = true;
        // 将所有和该类谓词相连的node节点全部置为'可见'
        edgeColor2Edges[edgeColorId].forEach(function (edgeId) {
            edgeVisStatus[edgeId] = true;
            edge2linkedNodes[edgeId].forEach(function (nodeEntityId) {nodeVisStatus[nodeEntityId] = true;})
        })
    }
    else { // 原来可见, 改成不可见
        $("#predButton" + edgeColorId).addClass(getColorClassMDC(edgeColorId, 100)).removeClass(getColorClassMDC(edgeColorId, 400)); // 改变图标状态
        edgeColorVisStatus[edgeColorId] = false;
        edgeColor2Edges[edgeColorId].forEach(function (edgeSeqId) {
            edgeVisStatus[edgeSeqId] = false;
            edge2linkedNodes[edgeSeqId].forEach(function (nodeEntityId) {
                // 1. 检查每个点是否需要删除 (已经 没有边与之相连了)
                var hasOtherEdge = false;
                node2linkedEdges[nodeEntityId].forEach(function (edgeId) {
                    if(edgeVisStatus[edgeId])
                        hasOtherEdge = true;
                })
                if(!hasOtherEdge && nodeEntityId != rootId) {
                    nodeVisStatus[nodeEntityId] = false;
                }
            })
        })
    }
}


//将谓词对应的边加粗
function boldEdges(EdgeColorId) {
    link.style("stroke-width", function(d){return d.color == EdgeColorId ? boldEdgeWidth : defaultEdgeWidth;});
}

// 将谓词边的宽度变为一样宽
function undoBoldEdges() {
    link.style("stroke-width", defaultEdgeWidth);
}

//当谓词按钮状态改变时控制边和点的显示/消失
function buttonStatusChanged(buttonId) {
    // 谓词被显示/取消, 需要改变按钮的状态
    changeNodeAndEdgeStatus(buttonId);
    updateSVGTriple();
}

// 谓词的可见性状态被改变, 需要SVG改变
function updateSVGTriple() {
    svg.selectAll(".node").style("visibility", function(d) {
        return !nodeVisStatus[d.id] ? "hidden" : "visible";
    })
    svg.selectAll(".link").style("opacity", function(d) {
        return !edgeVisStatus[d.id] ? 0 : 1;
    })
}
/*
//画出页面的谓词按钮列表
function updatePredColumn() {
    var innerHTML =     "<div class=\"panel panel-default\">\n"+
        "    <div class=\"panel-heading\" data-toggle=\"collapse\" href=\"#predicateList\" style='cursor:pointer'>\n"+
        "        <h3 class=\"panel-title\"><a data-toggle=\"collapse\" href=\"#predicateList\">\n"+
        "            谓词列表"+
        "        </a></h3>\n"+
        "    </div>\n"+
        "    <div class=\"panel-body collapse\" id=\"predicateList\">\n";
    var cnt = 0;
    for(var i in edgeColor2Name){
        cnt ++;
        if(cnt % 3 == 1)
            innerHTML += "<div class='row'> <div class=\"btn-group\" role=\"group\" aria-label=\"Basic example\">\n";
        innerHTML += '<button id="predButton' + i + '" type="button" ' +
            'class="btn btn-secondary ' + getColorClassMDC(i, 400) + '" ' +
            'onmouseover="boldEdges(' + i + ')" ' +
            'onmouseleave="undoBoldEdges(' + i + ')" ' +
            'onclick="buttonStatusChanged(\'' + i + '\')"' +
            '>' + edgeColor2Name[i] + '</button>'
        if(cnt % 3 == 0)
            innerHTML += '</div></div>\n';
    }
    if(cnt % 3 != 0)
        innerHTML += '</div></div>\n';
    innerHTML += '</div></div>'
    $("#buttonlist").html(innerHTML);
    $("#buttonlist").show().animate( { "opacity": "1", top:"20"} , 1500); // 开启加载图标
}
*/

