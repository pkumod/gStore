select distinct ?x ?p1 ?p2 ?p3 where 
{ 
	?x    <rdf:type>    <ub:UndergraduateStudent>. 
	?y    <ub:name> <Course1>. 
	?x    ?p1  ?y. 
	?z    <ub:teacherOf>    ?y. 
	?z    <ub:name> <FullProfessor1>. 
	?z    ?p2    ?w. 
	?w    ?p3    <Department0>. 
}
