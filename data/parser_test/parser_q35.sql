SELECT (kHopReachable(<Francis>, ?x, true, -1, {<喜欢>, <关注>}) AS ?y)
WHERE
{
	<Bob> ?pred ?x .
	MINUS { <Francis> <不喜欢> ?x . }
}