grammar SPARQL;

// export CLASSPATH="PATH-TO-antlr-4.7.2-complete.jar:$CLASSPATH"
// alias antlr4='java -Xmx500M -cp "PATH-TO-antlr-4.7.2-complete.jar:$CLASSPATH" org.antlr.v4.Tool'
// antlr4 -Dlanguage=Cpp ./SPARQL.g4 -visitor

// Parser rules

entry : queryUnit | updateUnit EOF ;
queryUnit : query ;
query : prologue( selectquery | constructquery | describequery | askquery )
valuesClause ;
updateUnit : update ;
prologue : ( baseDecl | prefixDecl )* ;
baseDecl : K_BASE IRIREF ;
prefixDecl : K_PREFIX PNAME_NS IRIREF ;
selectquery : selectClause datasetClause* whereClause solutionModifier ;
subSelect : selectClause whereClause solutionModifier valuesClause ;
selectClause : K_SELECT ( K_DISTINCT | K_REDUCED )? ( ( var | expressionAsVar )+ | '*' ) ;
expressionAsVar : '(' expression K_AS var ')' ;
constructquery : K_CONSTRUCT ( constructTemplate datasetClause* whereClause solutionModifier | datasetClause* K_WHERE '{' triplesTemplate? '}' solutionModifier ) ;
describequery : K_DESCRIBE ( varOrIri+ | '*' ) datasetClause* whereClause? solutionModifier ;
askquery : K_ASK datasetClause* whereClause solutionModifier ;
datasetClause : K_FROM ( defaultGraphClause | namedGraphClause ) ;
defaultGraphClause : sourceSelector ;
namedGraphClause : K_NAMED sourceSelector ;
sourceSelector : iri ;
whereClause : K_WHERE? groupGraphPattern ;
solutionModifier : groupClause? havingClause? orderClause? limitoffsetClauses? ;
groupClause : K_GROUP K_BY groupCondition+ ;
groupCondition : builtInCall | functionCall | '(' expression ( K_AS var )? ')' | var ;
havingClause : K_HAVING havingCondition+ ;
havingCondition : constraint ;
orderClause : K_ORDER K_BY orderCondition+ ;
orderCondition : ( ( K_ASC | K_DESC ) brackettedexpression )| ( constraint | var )
 ;
limitoffsetClauses : limitClause offsetClause? | offsetClause limitClause? ;
limitClause : K_LIMIT INTEGER ;
offsetClause : K_OFFSET INTEGER ;
valuesClause : ( K_VALUES dataBlock )? ;
update : prologue ( update1 ( ';' update )? )? ;
update1 : load | clear | drop | add | move | copy | create | insertData | deleteData | deleteWhere | modify ;
load : K_LOAD K_SILENT? iri ( K_INTO graphRef )? ;
clear : K_CLEAR K_SILENT? graphRefAll ;
drop : K_DROP K_SILENT? graphRefAll ;
create : K_CREATE K_SILENT? graphRef ;
add : K_ADD K_SILENT? graphOrDefault K_TO graphOrDefault ;
move : K_MOVE K_SILENT? graphOrDefault K_TO graphOrDefault ;
copy : K_COPY K_SILENT? graphOrDefault K_TO graphOrDefault ;
insertData : KK_INSERTDATA quadData ;
deleteData : KK_DELETEDATA quadData ;
deleteWhere : KK_DELETEWHERE quadPattern ;
modify : ( K_WITH iri )? ( deleteClause insertClause? | insertClause ) usingClause* K_WHERE groupGraphPattern ;
deleteClause : K_DELETE quadPattern ;
insertClause : K_INSERT quadPattern ;
usingClause : K_USING ( iri | K_NAMED iri ) ;
graphOrDefault : K_DEFAULT | K_GRAPH? iri ;
graphRef : K_GRAPH iri ;
graphRefAll : graphRef | K_DEFAULT | K_NAMED | K_ALL ;
quadPattern : '{' quads '}' ;
quadData : '{' quads '}' ;
quads : triplesTemplate? ( quadsNotTriples '.'? triplesTemplate? )* ;
quadsNotTriples : K_GRAPH varOrIri '{' triplesTemplate? '}' ;
triplesTemplate : triplesSameSubject ( '.' triplesTemplate? )? ;
groupGraphPattern : '{' ( subSelect | groupGraphPatternSub ) '}' ;
groupGraphPatternSub : triplesBlock? graphPatternTriplesBlock* ;
graphPatternTriplesBlock : graphPatternNotTriples '.'? triplesBlock? ;
triplesBlock : triplesSameSubjectpath ( '.' triplesBlock? )? ;
graphPatternNotTriples : groupOrUnionGraphPattern | optionalGraphPattern | minusGraphPattern | graphGraphPattern | serviceGraphPattern | filter | bind | inlineData ;
optionalGraphPattern : K_OPTIONAL groupGraphPattern ;
graphGraphPattern : K_GRAPH varOrIri groupGraphPattern ;
serviceGraphPattern : K_SERVICE K_SILENT? varOrIri groupGraphPattern ;
bind : K_BIND '(' expression K_AS var ')' ;
inlineData : K_VALUES dataBlock ;
dataBlock : inlineDataOnevar | inlineDataFull ;
inlineDataOnevar : var '{' dataBlockValue* '}' ;
inlineDataFull : ( NIL | '(' var* ')' ) '{' ( '(' dataBlockValue* ')' | NIL )* '}' ;
dataBlockValue : iri | rDFLiteral | numericLiteral | booleanLiteral | K_UNDEF ;
minusGraphPattern : K_MINUS groupGraphPattern ;
groupOrUnionGraphPattern : groupGraphPattern ( K_UNION groupGraphPattern )* ;
filter : K_FILTER constraint ;
constraint : brackettedexpression | builtInCall | functionCall ;
functionCall : iri argList ;
argList : NIL | '(' K_DISTINCT? expression ( ',' expression )* ')' ;
expressionList : NIL | '(' expression ( ',' expression )* ')' ;
constructTemplate : '{' constructTriples? '}' ;
constructTriples : triplesSameSubject ( '.' constructTriples? )? ;
triplesSameSubject : varOrTerm propertyListNotEmpty | triplesNode propertyList ;
propertyList : propertyListNotEmpty? ;
propertyListNotEmpty : verb objectList ( ';' ( verb objectList )? )* ;
verb : varOrIri | 'a' ;
objectList : object ( ',' object )* ;
object : graphNode ;
triplesSameSubjectpath : varOrTerm propertyListpathNotEmpty | triplesNodepath propertyListpath ;
propertyListpath : propertyListpathNotEmpty? ;
propertyListpathNotEmpty : verbpathOrSimple objectListpath ( ';' ( verbpathOrSimple objectList )? )* ;
verbpathOrSimple : verbpath | verbSimple ;
verbpath : path ;
verbSimple : var ;
objectListpath : objectpath ( ',' objectpath )* ;
objectpath : graphNodepath ;
path : pathAlternative ;
pathAlternative : pathSequence ( '|' pathSequence )* ;
pathSequence : pathEltOrInverse ( '/' pathEltOrInverse )* ;
pathElt : pathPrimary pathMod? ;
pathEltOrInverse : pathElt | '^' pathElt ;
pathMod : '?' | '*' | '+' ;
pathPrimary : iri | 'a' | '!' pathNegatedPropertySet | '(' path ')' ;
pathNegatedPropertySet : pathOneInPropertySet | '(' ( pathOneInPropertySet ( '|' pathOneInPropertySet )* )? ')' ;
pathOneInPropertySet : iri | 'a' | '^' ( iri | 'a' ) ;
triplesNode : collection | blankNodepropertyList ;
blankNodepropertyList : '[' propertyListNotEmpty ']' ;
triplesNodepath : collectionpath | blankNodepropertyListpath ;
blankNodepropertyListpath : '[' propertyListpathNotEmpty ']' ;
collection : '(' graphNode+ ')' ;
collectionpath : '(' graphNodepath+ ')' ;
graphNode : varOrTerm | triplesNode ;
graphNodepath : varOrTerm | triplesNodepath ;
varOrTerm : var | graphTerm ;
varOrIri : var | iri ;
varOrIriSet : '{' varOrIri ',' varOrIri ( ',' varOrIri )* '}' ;
var : VAR1 | VAR2 ;
graphTerm : iri | rDFLiteral | numericLiteral | booleanLiteral | blankNode | NIL ;
expression : conditionalOrexpression ;
conditionalOrexpression : conditionalAndexpression ( '||' conditionalAndexpression )* ;
conditionalAndexpression : valueLogical ( '&&' valueLogical )* ;
valueLogical : relationalexpression ;
relationalexpression : numericexpression ( '=' numericexpression | '!=' numericexpression | '<' numericexpression | '>' numericexpression | '<=' numericexpression | '>=' numericexpression | K_IN expressionList | K_NOT K_IN expressionList )? ;
numericexpression : additiveexpression ;
additiveexpression : multiplicativeexpression ( '+' multiplicativeexpression | '-' multiplicativeexpression | ( numericLiteralPositive | numericLiteralNegative ) ( ( '*' unaryexpression ) | ( '/' unaryexpression ) )* )* ;
multiplicativeexpression : unaryexpression ( '*' unaryexpression | '/' unaryexpression )* ;
unaryexpression : '!' primaryexpression|	'+' primaryexpression 
|	'-' primaryexpression 
|	primaryexpression
 ;
primaryexpression : brackettedexpression | builtInCall | iriOrFunction | rDFLiteral | numericLiteral | booleanLiteral | var ;
brackettedexpression : '(' expression ')' ;
predSet : '{' iri ( ',' iri )* '}' | '{' '}' ;
builtInCall : aggregate|	K_STR '(' expression ')' 
|	K_LANG '(' expression ')' 
|	K_LANGMATCHES '(' expression ',' expression ')' 
|	K_DATATYPE '(' expression ')' 
|	K_BOUND '(' var ')' 
|	K_IRI '(' expression ')' 
|	K_URI '(' expression ')' 
|	K_BNODE ( '(' expression ')' | NIL ) 
|	K_RAND NIL 
|	K_ABS '(' expression ')' 
|	K_CEIL '(' expression ')' 
|	K_FLOOR '(' expression ')' 
|	K_ROUND '(' expression ')' 
|	K_CONCAT expressionList 
|	substringexpression 
|	K_STRLEN '(' expression ')' 
|	strReplaceexpression 
|	K_UCASE '(' expression ')' 
|	K_LCASE '(' expression ')' 
|	KK_ENCODE_FOR_URI '(' expression ')' 
|	K_CONTAINS '(' expression ',' expression ')' 
|	K_STRSTARTS '(' expression ',' expression ')' 
|	K_STRENDS '(' expression ',' expression ')' 
|	K_STRBEFORE '(' expression ',' expression ')' 
|	K_STRAFTER '(' expression ',' expression ')' 
|	K_YEAR '(' expression ')' 
|	K_MONTH '(' expression ')' 
|	K_DAY '(' expression ')' 
|	K_HOURS '(' expression ')' 
|	K_MINUTES '(' expression ')' 
|	K_SECONDS '(' expression ')' 
|	K_TIMEZONE '(' expression ')' 
|	K_TZ '(' expression ')' 
|	K_NOW NIL 
|	K_UUID NIL 
|	K_STRUUID NIL 
|	KK_MD5 '(' expression ')' 
|	KK_SHA1 '(' expression ')' 
|	KK_SHA256 '(' expression ')' 
|	KK_SHA384 '(' expression ')' 
|	KK_SHA512 '(' expression ')' 
|	K_COALESCE expressionList 
|	K_IF '(' expression ',' expression ',' expression ')' 
|	K_STRLANG '(' expression ',' expression ')' 
|	K_STRDT '(' expression ',' expression ')' 
|	K_SAMETERM '(' expression ',' expression ')' 
|	K_ISIRI '(' expression ')' 
|	K_ISURI '(' expression ')' 
|	K_ISBLANK '(' expression ')' 
|	K_ISLITERAL '(' expression ')' 
|	K_ISNUMERIC '(' expression ')' 
|	regexexpression 
|	existsFunc 
|	notexistsFunc
|   K_SIMPLECYCLEPATH '(' varOrIri ',' varOrIri ',' booleanLiteral ',' predSet ')'
|   K_SIMPLECYCLEBOOLEAN '(' varOrIri ',' varOrIri ',' booleanLiteral ',' predSet ')'
|   K_CYCLEPATH '(' varOrIri ',' varOrIri ',' booleanLiteral ',' predSet ')'
|   K_CYCLEBOOLEAN '(' varOrIri ',' varOrIri ',' booleanLiteral ',' predSet ')'
|   K_SHORTESTPATH '(' varOrIri ',' varOrIri ',' booleanLiteral ',' predSet ')'
|   K_SHORTESTPATHLEN '(' varOrIri ',' varOrIri ',' booleanLiteral ',' predSet ')'
|   K_KHOPREACHABLE '(' varOrIri ',' varOrIri ',' booleanLiteral ',' (num_integer | integer_positive | integer_negative) ',' predSet ')'
|   K_KHOPENUMERATE '(' varOrIri ',' varOrIri ',' booleanLiteral ',' (num_integer | integer_positive | integer_negative) ',' predSet ')'
|   K_KHOPREACHABLEPATH '(' varOrIri ',' varOrIri ',' booleanLiteral ',' (num_integer | integer_positive | integer_negative) ',' predSet ')'
|   K_PPR '(' varOrIri ',' (num_integer | integer_positive | integer_negative) ',' predSet ',' num_integer ')'
|   K_TRIANGLECOUNTING '(' booleanLiteral ',' predSet ')'
|   K_CLOSENESSCENTRALITY '(' varOrIri ',' booleanLiteral ',' predSet ')'
|   K_BFSCOUNT '(' varOrIri ',' booleanLiteral ',' predSet ')'
|   K_PR '(' booleanLiteral ',' predSet ',' K_ALPHA '=' numericLiteral ',' K_MAXITER '=' num_integer ',' K_TOL '=' numericLiteral ')'
|   K_SSSP '(' varOrIri ',' booleanLiteral ',' predSet ')'
|   K_SSSPLEN '(' varOrIri ',' booleanLiteral ',' predSet ')'
|   K_LABELPROP '(' booleanLiteral ',' predSet ')'
|   K_WCC '(' predSet ')'
|   K_CLUSTERCOEFF '(' (varOrIri ',')? booleanLiteral ',' predSet ')'
|   K_MAXIMUMCLIQUE '(' '{' varOrIri (',' varOrIri)* '}' ',' predSet ')'
|   K_PFN '(' varOrIriSet ',' booleanLiteral ',' (num_integer | integer_positive | integer_negative) ',' predSet ',' string ')'
 ;
regexexpression : K_REGEX '(' expression ',' expression ( ',' expression )? ')' ;
substringexpression : K_SUBSTR '(' expression ',' expression ( ',' expression )? ')' ;
strReplaceexpression : K_REPLACE '(' expression ',' expression ',' expression ( ',' expression )? ')' ;
existsFunc : K_EXISTS groupGraphPattern ;
notexistsFunc : K_NOT K_EXISTS groupGraphPattern ;
aggregate : K_COUNT '(' K_DISTINCT? ( '*' | expression ) ')'| K_SUM '(' K_DISTINCT? expression ')' 
| K_MIN '(' K_DISTINCT? expression ')' 
| K_MAX '(' K_DISTINCT? expression ')' 
| K_AVG '(' K_DISTINCT? expression ')' 
| K_SAMPLE '(' K_DISTINCT? expression ')' 
| KK_GROUP_CONCAT '(' K_DISTINCT? expression ( ';' K_SEPARATOR '=' string )? ')'
 ;
iriOrFunction : iri argList? ;
rDFLiteral : string ( LANGTAG | ( '^^' iri ) )? ;
numericLiteral : numericLiteralUnsigned | numericLiteralPositive | numericLiteralNegative ;
numericLiteralUnsigned : num_integer | num_decimal | num_double ;
numericLiteralPositive : integer_positive | decimal_positive | double_positive ;
numericLiteralNegative : integer_negative | decimal_negative | double_negative ;
num_integer : INTEGER ;
num_decimal : DECIMAL ;
num_double : DOUBLE ;
integer_positive : INTEGER_POSITIVE ;
decimal_positive : DECIMAL_POSITIVE ;
double_positive : DOUBLE_POSITIVE ;
integer_negative : INTEGER_NEGATIVE ;
decimal_negative : DECIMAL_NEGATIVE ;
double_negative : DOUBLE_NEGATIVE ;
booleanLiteral : 'true' | 'false' ;
string : STRING_LITERAL1 | STRING_LITERAL2 | STRING_LITERAL_LONG1 | STRING_LITERAL_LONG2 ;
iri : IRIREF | prefixedName ;
prefixedName : PNAME_LN | PNAME_NS ;
blankNode : BLANK_NODE_LABEL | ANON
 ;


// Lexer rules

// Case-insensitive keywords
K_NOW : N O W ;
K_YEAR : Y E A R ;
K_UNION : U N I O N ;
K_IF : I F ;
K_ASK : A S K ;
K_ASC : A S C ;
K_CONCAT : C O N C A T ;
K_IN : I N ;
K_UNDEF : U N D E F ;
K_INSERT : I N S E R T ;
K_MONTH : M O N T H ;
K_DEFAULT : D E F A U L T ;
K_SELECT : S E L E C T ;
K_FLOOR : F L O O R ;
K_TZ : T Z ;
K_COPY : C O P Y ;
K_CEIL : C E I L ;
K_HOURS : H O U R S ;
K_DATATYPE : D A T A T Y P E ;
K_ISNUMERIC : I S N U M E R I C ;
K_STRUUID : S T R U U I D ;
K_CONSTRUCT : C O N S T R U C T ;
K_ADD : A D D ;
K_BOUND : B O U N D ;
K_NAMED : N A M E D ;
K_TIMEZONE : T I M E Z O N E ;
K_MIN : M I N ;
K_ISBLANK : I S B L A N K ;
K_UUID : U U I D ;
K_BIND : B I N D ;
K_CLEAR : C L E A R ;
K_INTO : I N T O ;
K_AS : A S ;
K_ALL : A L L ;
K_IRI : I R I ;
K_BASE : B A S E ;
K_BY : B Y ;
K_DROP : D R O P ;
K_LOAD : L O A D ;
K_WITH : W I T H ;
K_BNODE : B N O D E ;
K_WHERE : W H E R E ;
K_AVG : A V G ;
K_SAMPLE : S A M P L E ;
K_UCASE : U C A S E ;
K_SERVICE : S E R V I C E ;
K_MINUS : M I N U S ;
K_SAMETERM : S A M E T E R M ;
K_STRSTARTS : S T R S T A R T S ;
K_STR : S T R ;
K_MOVE : M O V E ;
K_HAVING : H A V I N G ;
K_COALESCE : C O A L E S C E ;
K_STRBEFORE : S T R B E F O R E ;
K_ABS : A B S ;
K_ISLITERAL : I S L I T E R A L ;
K_STRAFTER : S T R A F T E R ;
K_STRLEN : S T R L E N ;
K_LANG : L A N G ;
K_CREATE : C R E A T E ;
K_DESC : D E S C ;
K_MAX : M A X ;
K_FILTER : F I L T E R ;
K_USING : U S I N G ;
K_NOT : N O T ;
K_STRENDS : S T R E N D S ;
K_OFFSET : O F F S E T ;
K_CONTAINS : C O N T A I N S ;
K_PREFIX : P R E F I X ;
K_MINUTES : M I N U T E S ;
K_REPLACE : R E P L A C E ;
K_REGEX : R E G E X ;
K_DELETE : D E L E T E ;
K_SEPARATOR : S E P A R A T O R ;
K_DAY : D A Y ;
K_SILENT : S I L E N T ;
K_STRLANG : S T R L A N G ;
K_ORDER : O R D E R ;
K_ROUND : R O U N D ;
K_GRAPH : G R A P H ;
K_SECONDS : S E C O N D S ;
K_URI : U R I ;
K_DISTINCT : D I S T I N C T ;
K_EXISTS : E X I S T S ;
K_GROUP : G R O U P ;
K_SUM : S U M ;
K_REDUCED : R E D U C E D ;
K_FROM : F R O M ;
K_LANGMATCHES : L A N G M A T C H E S ;
K_ISURI : I S U R I ;
K_TO : T O ;
K_ISIRI : I S I R I ;
K_RAND : R A N D ;
K_STRDT : S T R D T ;
K_COUNT : C O U N T ;
K_DESCRIBE : D E S C R I B E ;
K_VALUES : V A L U E S ;
K_LCASE : L C A S E ;
K_OPTIONAL : O P T I O N A L ;
K_LIMIT : L I M I T ;
K_SUBSTR : S U B S T R ;
K_SIMPLECYCLEPATH : S I M P L E C Y C L E P A T H ;
K_SIMPLECYCLEBOOLEAN : S I M P L E C Y C L E B O O L E A N ;
K_CYCLEPATH : C Y C L E P A T H ;
K_CYCLEBOOLEAN : C Y C L E B O O L E A N ;
K_SHORTESTPATH : S H O R T E S T P A T H ;
K_SHORTESTPATHLEN : S H O R T E S T P A T H L E N ;
K_KHOPREACHABLE : K H O P R E A C H A B L E ;
K_KHOPENUMERATE : K H O P E N U M E R A T E ;
K_KHOPREACHABLEPATH : K H O P R E A C H A B L E P A T H ;
K_PPR : P P R ;
K_TRIANGLECOUNTING : T R I A N G L E C O U N T I N G ;
K_CLOSENESSCENTRALITY : C L O S E N E S S C E N T R A L I T Y ;
K_BFSCOUNT : B F S C O U N T ;
K_PR : P R ;
K_ALPHA : A L P H A ;
K_MAXITER : M A X I T E R ;
K_TOL : T O L ;
K_SSSP : S S S P ;
K_SSSPLEN : S S S P L E N ;
K_LABELPROP : L A B E L P R O P ;
K_WCC : W C C ;
K_CLUSTERCOEFF : C L U S T E R C O E F F ;
K_MAXIMUMCLIQUE : M A X I M U M C L I Q U E ;
K_PFN : P F N ;
KK_INSERTDATA : I N S E R T ' ' D A T A ;
KK_DELETEDATA : D E L E T E ' ' D A T A ;
KK_DELETEWHERE : D E L E T E ' ' W H E R E ;
KK_ENCODE_FOR_URI : E N C O D E '_' F O R '_' U R I ;
KK_MD5 : M D '5' ;
KK_SHA1 : S H A '1' ;
KK_SHA256 : S H A '256' ;
KK_SHA384 : S H A '384' ;
KK_SHA512 : S H A '512' ;
KK_GROUP_CONCAT : G R O U P '_' C O N C A T ;

IRIREF
: '<' ( ~('<' | '>' | '"' | '{' | '}' | '|' | '^' | '\\' | '`' | '\u0000'..'\u0020' ) )* '>'
;
PNAME_NS : PN_PREFIX? ':' ;
PNAME_LN : PNAME_NS PN_LOCAL ;
BLANK_NODE_LABEL : '_:' ( PN_CHARS_U | [0-9] ) ((PN_CHARS|'.')* PN_CHARS)? ;
VAR1 : '?' VARNAME ;
VAR2 : '$' VARNAME ;
LANGTAG : '@' [a-zA-Z]+ ('-' [a-zA-Z0-9]+)* ;
INTEGER : [0-9]+ ;
DECIMAL : [0-9]* '.' [0-9]+ ;
DOUBLE : [0-9]+ '.' [0-9]* EXPONENT | '.' ([0-9])+ EXPONENT | ([0-9])+ EXPONENT ;
INTEGER_POSITIVE : '+' INTEGER ;
DECIMAL_POSITIVE : '+' DECIMAL ;
DOUBLE_POSITIVE : '+' DOUBLE ;
INTEGER_NEGATIVE : '-' INTEGER ;
DECIMAL_NEGATIVE : '-' DECIMAL ;
DOUBLE_NEGATIVE : '-' DOUBLE ;
EXPONENT : [eE] [+-]? [0-9]+ ;
STRING_LITERAL1 : '\'' ( ~('\u0027' | '\u005C' | '\u000A' | '\u000D') | ECHAR )* '\'' ;
STRING_LITERAL2 : '"'  ( ~('\u0022' | '\u005C' | '\u000A' | '\u000D') | ECHAR )* '"' ;
STRING_LITERAL_LONG1 : '\'\'\'' ( ( '\'' | '\'\'' )? ( [^'\\] | ECHAR ) )* '\'\'\'' ;
STRING_LITERAL_LONG2 : '"""' ( ( '"' | '""' )? ( [^"\\] | ECHAR ) )* '"""' ;
ECHAR : '\\' ('t' | 'b' | 'n' | 'r' | 'f' | '"' | '\'') ;
NIL : '(' WS* ')' ;
WS : (' ' | '\t' | '\r' | '\n') ->skip ;
ANON : '[' WS* ']' ;
PN_CHARS_BASE : [A-Z] | [a-z] | '\u00C0'..'\u00D6' | '\u00D8'..'\u00F6' | '\u00F8'..'\u02FF' | '\u0370'..'\u037D' | '\u037F'..'\u1FFF' | '\u200C'..'\u200D' | '\u2070'..'\u218F' | '\u2C00'..'\u2FEF' | '\u3001'..'\uD7FF' | '\uF900'..'\uFDCF' | '\uFDF0'..'\uFFFD' | '\u{10000}'..'\u{EFFFF}' ;
PN_CHARS_U : PN_CHARS_BASE | '_' ;
VARNAME : ( PN_CHARS_U | [0-9] ) ( PN_CHARS_U | [0-9] | '\u00B7' | '\u0300'..'\u036F' | '\u203F'..'\u2040' )* ;
PN_CHARS : PN_CHARS_U | '-' | [0-9] | '\u00B7' | '\u0300'..'\u036F' | '\u203F'..'\u2040' ;
PN_PREFIX : PN_CHARS_BASE ((PN_CHARS|'.')* PN_CHARS)? ;
PN_LOCAL : (PN_CHARS_U | ':' | [0-9] | PLX ) ((PN_CHARS | '.' | ':' | PLX)* (PN_CHARS | ':' | PLX) )? ;
PLX : PERCENT | PN_LOCAL_ESC ;
PERCENT : '%' HEX HEX ;
HEX : [0-9] | [A-F] | [a-f] ;
PN_LOCAL_ESC : '\\' ( '_' | '~' | '.' | '-' | '!' | '$' | '&' | '\'' | '(' | ')' | '*' | '+' | ',' | ';' | '=' | '/' | '?' | '#' | '@' | '%' ) ;
COMMENT : '#' ~[\n\r]* -> channel(HIDDEN);

fragment A : [aA];
fragment B : [bB];
fragment C : [cC];
fragment D : [dD];
fragment E : [eE];
fragment F : [fF];
fragment G : [gG];
fragment H : [hH];
fragment I : [iI];
fragment J : [jJ];
fragment K : [kK];
fragment L : [lL];
fragment M : [mM];
fragment N : [nN];
fragment O : [oO];
fragment P : [pP];
fragment Q : [qQ];
fragment R : [rR];
fragment S : [sS];
fragment T : [tT];
fragment U : [uU];
fragment V : [vV];
fragment W : [wW];
fragment X : [xX];
fragment Y : [yY];
fragment Z : [zZ];