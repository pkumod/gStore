select distinct ?x where
{
?x	<rdf:type>	<ub:GraduateStudent>.
?y	<rdf:type>	<ub:University>.
?z	<rdf:type>	<ub:Department>.
?x	<ub:memberOf>	?z.
?z	<ub:subOrganizationOf>	?y.
?x	<ub:undergraduateDegreeFrom>	?y.
}
