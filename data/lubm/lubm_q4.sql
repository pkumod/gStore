select distinct ?x where 
{ 
	?x    <rdf:type>    <ub:UndergraduateStudent>. 
	?y    <ub:name> <Course1>. 
	?x    <ub:takesCourse>  ?y. 
	?z    <ub:teacherOf>    ?y. 
	?z    <ub:name> <FullProfessor1>. 
	?z    <ub:worksFor>    ?w. 
	?w    <ub:name>    <Department0>. 
}
