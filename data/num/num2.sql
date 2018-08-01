PREFIX	:	<http://example/>
PREFIX	foaf:	<http://xmlns.com/foaf/0.1/>
PREFIX	xsd:	<http://www.w3.org/2001/XMLSchema#>

SELECT ?nx	?ny	?sx	?sy ?hx ?hy
{
	?x	foaf:knows	?y.
	?x	foaf:givenName	?nx.
	?y	foaf:givenName	?ny.
	?x	foaf:salary	?sx.
	?y	foaf:salary ?sy.
	?x	foaf:height	?hx.
	?y	foaf:height	?hy.
	FILTER(?sx > ?sy && (?hx > ?hy || ?hx >= "170.0"^^xsd:float))
}
