select (shortestPathLen(<Francis>, ?x, true, {<喜欢>, <关注>}) as ?y)
where
{
	<Bob> ?pred ?x .
	MINUS { <Francis> <不喜欢> ?x . }
}