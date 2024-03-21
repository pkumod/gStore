var DEBUGMODE = 0;

// cateIns的css文件
$cateInsSheet = $('link[href="../css/cateIns.css"]')[0].sheet;

var MODE = {
    "RELATION" : 1,
    "LIST" : 2,
    "NULL" : 0,
    properties: {
        0 : {name : 'null', value : 0, code : 'n'},
        1 : {name : 'relation', value : 1, code : 'r'},
        2 : {name : 'list', value : 2, code : 'l'},
    }
};
var CurMode = MODE.NULL;

////////////////////////////////////////////////
// 共用的node\edge状态, 网页选中button来激活buttonStatusChanged函数来更新
// node 有2种id:
//     1. 实体id: 对应json文件中的`id`属性, 如果node是实体节点, id为实体id;如果是文本节点,id为-1,-2,...
//     2. nodeSeqId: 实体节点在json文件中的排列序号, 从0,1,...开始编号;是d3力导图的画图方式
// edge 只有一种id: edge id: edge边在json文件中的排列序号, 从0,1,...开始编号;是d3力导图的画图方式
// edgeColor 只有一种id: 不同的edge id 如果是相同的谓词, edgeColor 则一样, 对应json文件中edge的`color`属性
////////////////////////////////////////////////
// 调用方式 nodeVisStatus[192344] (返回id为192344的实体是否可见), 非实体node的id小于0
var nodeVisStatus = new Map();
// 调用方式 edgeColorVisStatus[1] 返回 color 为 1的边是否可见
var edgeColorVisStatus = new Map();
// 调用方式 edgeColorVisStatus[10] 返回 edge id 为 10的边是否可见
var edgeVisStatus = new Map();
// 中心点的实体id
var rootId;
// node 实体id -> 相连的 edge id
var node2linkedEdges = new Map();
// 力导图的序号(0,1,2,...)到实体id的映射表
var nodeSeqId2nodeId = new Map();
// 实体id到力导图的序号(0,1,2,...)的映射表
var node2SeqId = new Map();
// edge id -> 相连的 node id, 有且仅有2个相连的节点
var edge2linkedNodes = new Map();
// edge color -> edge id
var edgeColor2Edges = new Map();
// edge id -> edge color
var edge2Color = new Map();
// edge color -> predicate name
var edgeColor2Name = new Map();
/////////////////////////////////////////////////////////////


var json; // 存储从 getCateInsJson.php 获取的信息
var width = 1000;
var height = 800;
var svg ;
var force;
var mouseEntity = 0; // 当前鼠标所在的实体id
var fontsize = 14;
var link; //所有边的集合
var node; //所有点的集合
var entNodes; // 所有是实体的点
var imgEntNodes; // 有图片的实体的点
var noImgEntNodes; // 没有图片的实体点


var defaultEdgeWidth = 5;
var boldEdgeWidth = 15;
var minRadius = 30;

$(document).ready(function() {
});

// svg图加载之前做的工作，检查图片是否404，
function preloadCheckAndShow() {
    curNodeImageParsed = 0;
    preloadBar.set(0);
    $.each(json.nodes, function (i, d) {
        UrlExists((d.img == undefined ? '' : d.img));
    })
}

// 获得d3 node的半径，半径不能过小
function getDefaultR(d) {return 0.8*Math.max(d.hotness * (d.id == mouseEntity ? 1.5:1), minRadius);}

// 加载动画：喵喵+进度百分比
var preloadBar = new ProgressBar.Path('#heart-path', {
    easing: 'easeInOut',
    from: { color: '#E57373', width: 1 },
    to: { color: '#cf1a32', width: 8 },
    step: function(state, circle) {
        circle.path.setAttribute('stroke', state.color);
        circle.path.setAttribute('stroke-width', state.width);
        document.getElementById("preloadtext").innerHTML=parseInt(100*circle.value());
    },
    duration: 1400
});

var curNodeImageParsed = 0;
var nodeSize = 100;
// 显示加载动画, 加载完毕后进入显示页面
function preloadAnimation() {
    startPer = (curNodeImageParsed / nodeSize);
    endPer = (curNodeImageParsed+1) / nodeSize;
    preloadBar.animate(endPer);
    curNodeImageParsed ++;
    if(curNodeImageParsed == nodeSize){
        if(endPer != 1.0)
            preloadBar.animate(1.0);
        window.setTimeout(function () {
            $("#preloadspinner").animate( { "opacity": "0", top:"-50"} , 300 ); // 开启加载图标
            window.setTimeout(function () {
                $("#svg-meowth-container").animate( { "opacity": "0", top:"0"} , 200 ); // 开启加载图标
                window.setTimeout(function () {
                    $("#svg-meowth-container").css('visibility', 'hidden');
                    switch(CurMode) {
                        case MODE.LIST:
                            DisplayCateIns();
                            break;
                        case MODE.RELATION:
                            updatePredColumn();
                            DisplayTriple();
                            break;
                    }
                }, 500)
            }, 500);
        },2000);
    }
}

// 将json节点的name/title显示在node上
function appendText2Node(d, scale) {
   d.append("text")
        .text(function (d) {return d.name;})
        .style("font-size", function (d) { return getFontSizeEntityTitle(getDefaultR(d) * scale, d.name, this); })
        .attr("text-anchor", "middle")
        .attr("dy", ".3em")
}

// 给节点添加鼠标悬停毛玻璃效果、悬停显示实体名字效果
function AppendMouseBlurEffect(entNodes, scaleRate) {
    imgEntNodes = entNodes.filter(function (d){ return nodeIndex2ImgUrl(d).startsWith("http");});
    noImgEntNodes = entNodes.filter(function (d){ return !nodeIndex2ImgUrl(d).startsWith("http");});

    //鼠标悬停毛玻璃
    entNodes
        .on("mouseenter",function(d){
            mouseEntity = parseInt(d.id);
            d3.select(this).select("image").attr("filter", "url(#blur)");
            d3.select(this).select("circle").attr("r", getDefaultR(d));
            d3.select(this).select("text").remove();
            appendText2Node(d3.select(this), 1.1);
        })
        .on("mouseleave",function(d){
            if(d.id == mouseEntity)
                mouseEntity = 0;
            d3.select(this).select("text").remove();
            if(!nodeIndex2ImgUrl(d).startsWith("http"))
                appendText2Node(d3.select(this), 1.0);
            d3.select(this).select("image").attr("filter", null);
            d3.select(this).select("circle").attr("r", getDefaultR(d));
            force.resume();
        })

    // 设置实体的图片
    entNodes.append('svg:defs').append('pattern')
        .attr("id", function (d) {return "bg" + d.id;})
        .attr('width', 1)
        .attr('height', 1)
        .attr('patternContentUnits', 'objectBoundingBox')
        .append("svg:image")
        .attr("href", function (d) {return nodeIndex2ImgUrl(d);})
        // .attr("xlink:href", function (d) {return nodeIndex2ImgUrl(d);})
        // .attr("xlink:href", function (d) {return defaultEntityIMG;})
        .attr("height", 1)
        .attr("width", 1)
        .attr("preserveAspectRatio", "xMidYMin slice");
    entNodes.append("circle")
        .attr("r", function (d) {return getDefaultR(d);})
        .style("fill", function (d) { return "url(#bg" + d.id + ")";})
    ;
    // TODO 右击固定node
    var menu = [
        {title: function (d) {d.tmpFix = d.fixed; return d.name}},
        {
            title: function (d) { // 没有第二个参数
                var stat = json.nodes[d.index].tmpFix;
                // console.log(d.index + "找title时候fix状态:" + stat);
                return (stat == 0 || stat == 6) ? '固定' : '取消固定';
            },
            action: function(ele, d) {
                // console.log(ele);
                // console.log(d);
                var stat = json.nodes[d.index].tmpFix;
                // console.error(d.index + "点击之前nodes.fix:" + json.nodes[d.index].tmpFix);
                json.nodes[d.index].tmpFix = (stat >= 6 ? 7 - stat : 1 - stat);
                // console.error(d.index + "点击之后nodes.fix:" + json.nodes[d.index].tmpFix);
                json.nodes[d.index].fixed = json.nodes[d.index].tmpFix;
            }
        },
        {divider: true},
    ]

    entNodes.on('contextmenu', d3.contextMenu(menu, {
        onOpen: function(d, i) {
            // console.log('open! ' + d.id);
            // console.log(d.id + " onOpen之前状态::" + d.fixed);
            // d.fixed = 1; // 能从浮动->固定
            // console.log(d.id + " onOpen之后状态::" + d.fixed);
        },
        onClose: function(d, i) {
            if(d == undefined || i == undefined) return;
            // console.log(d);
            // console.log(d.id);
            // console.error("张德江的状态:");
            // console.error(json.nodes[26]);
            var idx = node2SeqId[d.id];
            json.nodes[idx].fixed = json.nodes[idx].tmpFix;
        }
    }));


    entNodes.on("dblclick", function(d){redrawSVG('entity', d.id, false);})

    // 没有图片的实体点
    noImgEntNodes.selectAll("image").attr("filter", "url(#blur)");
    appendText2Node(noImgEntNodes, 1);
}

var defaultEntityIMG = '../img/grey-square.png';

//加载 node->{linked edges}, edge->{linked nodes}, {edge status}, {node status} 表
function loadNodeMaps() {
    node2linkedEdges = new Map();
    // nodeVisStatus = new Map();
    nodeVisStatus = new Map();
    nodeSeqId2nodeId = new Map();
    node2SeqId.clear();
    rootId = json.nodes[0].id;
    // 1. 初始化4个map
    $.each(json.nodes, function (i, d) {
        var entityId = json.nodes[i].id;
        json.nodes[i].fixed = 0;  // 实体点是否固定在屏幕上
        json.nodes[i].tmpFix = 0; // 存储实体点操作完成后是否固定在屏幕上(中间状态)
        node2linkedEdges[entityId] = new Set();
        nodeVisStatus[entityId] = true;
        nodeSeqId2nodeId[i] = entityId;
        node2SeqId[entityId] = i;
    })
    nodeSize = Object.keys(nodeVisStatus).length
}

function getColorClassMDC(color_name_idx, color_value_idx) {
    var names = ["amber", "blue", "blue-grey", "brown", "cyan", "deep-orange", "deep-purple", "green", "grey", "indigo", "light-blue", "light-green", "lime", "orange", "pink", "purple", "red", "teal", "yellow"];
    var re = "mdc-bg-" + names[color_name_idx % names.length] + "-" + color_value_idx;
    return re;
}

// 颜色的css文件
$colorSheet = $('link[href="../css/material-design-color-palette.min.css"]')[0].sheet;

function getColorHexMDC(color_name_idx, color_value_idx) {
    var name = "." + getColorClassMDC(color_name_idx, color_value_idx);
    for (var j = 0, k = $colorSheet.cssRules.length; j < k; j++) {
        var rule = $colorSheet.cssRules[j];
        if (rule.selectorText && rule.selectorText.split(',').indexOf(name) !== -1) {
            return rule.style['background-color'];
        }
    }
    return null;
}

//获取节点的IMG url
var imgLinkExistMap = new Map();
function nodeIndex2ImgUrl(d) {
    var id = (d.entityId ? d.entityId : d.id);
    if(json.nodes[node2SeqId[id]].img == undefined || json.nodes[node2SeqId[id]].img == null || json.nodes[node2SeqId[id]].img == '')
        return defaultEntityIMG;
    // console.log(json.nodes[node2SeqId[d.id]]);
    // console.log(json.nodes[node2SeqId[d.id]].img);
    // var url = json.nodes[node2SeqId[d.id]].img ;
    return defaultEntityIMG;
    return UrlReplace(url);
}

function UrlReplace(url) {
    if(imgLinkExistMap[url] != undefined)
        return imgLinkExistMap[url];
    imgLinkExistMap[url] = UrlExists(url) ? url : defaultEntityIMG;
    // preloadAnimation();
    return imgLinkExistMap[url];
}

// 这个是异步的，不可取！！
function UrlExists(u){
    imgLinkExistMap[u] = defaultEntityIMG;
    preloadAnimation();
    // $.ajax({
    //     url:u,
    //     type:'HEAD',
    //     error: function() {;;imgLinkExistMap[u] = defaultEntityIMG},
    //     success: function() {preloadAnimation();; imgLinkExistMap[u] = u}
    // });
}



//计算entity title， 使得title全部包含在半径为r的圆内
function getFontSizeEntityTitle(r, title, element) {
    var x = Math.min(r, (r- 2) / element.getComputedTextLength() * 24) + "px";
    return x;
}

function getTextWindowWidth_leave(text, fontsize, maxWid) {
    return text.length * 15 > 100 ? 100 : text.length * 15;
}

function getTextWindowWidth_enter(text, fontsize, maxWid) {
    return text.length * 15 > 200 ? 200 : text.length * 15;
}

function getTextWindowHeight_leave(text, fontsize, maxWid) {
    return 20;
}

function getTextWindowHeight_enter(text, fontsize, maxWid) {
    return ((text.length + 1) /20 + 1 ) * 20;
}

function getTitle_leave(name) {
    return name.length > 6 ? name.substring(0, 6) : name;
}
