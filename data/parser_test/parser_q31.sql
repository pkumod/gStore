PREFIX ns:   <http://example.org/ns#>
select ?y ?x
where
{
	?y ns:p ?x .
	filter (?x > 9)
}