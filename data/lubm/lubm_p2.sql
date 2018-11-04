select distinct ?x ?p1 ?p2 where
{
?x	<rdf:type>	<ub:GraduateStudent>.
?y	<rdf:type>	<ub:University>.
?z	<rdf:type>	<ub:Department>.
?x	?p1	?z.
?z	?p2	?y.
?x	<ub:undergraduateDegreeFrom>	?y.
}
