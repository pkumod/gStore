/**
 * Created by hanzhe on 16-12-5.
 */


//画力导图
function DisplayCateIns() {
    //重画力导图
    force = d3.layout.force()
        .gravity(0.02)
        // .distance(30)
        .charge(-2)
        .size([width, height]);
    force.nodes(json.nodes).start();

    var node = svg.selectAll(".node")
        .data(json.nodes)
        .enter().append("g")
        .attr("class", "node")
        .call(force.drag);

    //鼠标悬停毛玻璃
    AppendMouseBlurEffect(node, 1.0);
    // console.log(json.nodes);

    //实现实体之间"排斥"的动画效果
    force.on("tick", function() {
        var q = d3.geom.quadtree(json.nodes),
            i = 0,
            n = json.nodes.length;
        while (++i < n) {
            q.visit(collide(json.nodes[i]));
        }
        node.attr("transform", function(d) {
            var radius = getDefaultR(d);
            d.x = Math.max(0, Math.min(width - 0, d.x));
            d.y = Math.max(0, Math.min(height - 0, d.y));
            return "translate("+d.x+","+d.y+")";
        });
    });
}

function collide(node) {
    var r = getDefaultR(node) + 1,
        nx1 = node.x - r, nx2 = node.x + r,
        ny1 = node.y - r, ny2 = node.y + r;
    return function(quad, x1, y1, x2, y2) {
        if (quad.point && (quad.point !== node)) {
            var x = node.x - quad.point.x,
                y = node.y - quad.point.y,
                l = Math.sqrt(x * x + y * y);
            var r = getDefaultR(node) + getDefaultR(quad.point) + 20;
            if (l < r ) {
                l = (l - r) / l * .5;
                node.x -= x *= l;
                node.y -= y *= l;
                quad.point.x += x;
                quad.point.y += y;
            }
        }
        return x1 > nx2 || x2 < nx1 || y1 > ny2 || y2 < ny1;
    };
}

