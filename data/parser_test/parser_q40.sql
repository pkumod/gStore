select ?x where
{?x <birthday> ?y . filter(month(?y) = 1 || month(?y) = 2)}