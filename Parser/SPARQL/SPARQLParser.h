
// Generated from ./SPARQL.g4 by ANTLR 4.7.2

#pragma once


#include "antlr4-runtime.h"




class  SPARQLParser : public antlr4::Parser {
public:
  enum {
    T__0 = 1, T__1 = 2, T__2 = 3, T__3 = 4, T__4 = 5, T__5 = 6, T__6 = 7, 
    T__7 = 8, T__8 = 9, T__9 = 10, T__10 = 11, T__11 = 12, T__12 = 13, T__13 = 14, 
    T__14 = 15, T__15 = 16, T__16 = 17, T__17 = 18, T__18 = 19, T__19 = 20, 
    T__20 = 21, T__21 = 22, T__22 = 23, T__23 = 24, T__24 = 25, T__25 = 26, 
    T__26 = 27, T__27 = 28, T__28 = 29, K_NOW = 30, K_YEAR = 31, K_UNION = 32, 
    K_IF = 33, K_ASK = 34, K_ASC = 35, K_CONCAT = 36, K_IN = 37, K_UNDEF = 38, 
    K_INSERT = 39, K_MONTH = 40, K_DEFAULT = 41, K_SELECT = 42, K_FLOOR = 43, 
    K_TZ = 44, K_COPY = 45, K_CEIL = 46, K_HOURS = 47, K_DATATYPE = 48, 
    K_ISNUMERIC = 49, K_STRUUID = 50, K_CONSTRUCT = 51, K_ADD = 52, K_BOUND = 53, 
    K_NAMED = 54, K_TIMEZONE = 55, K_MIN = 56, K_ISBLANK = 57, K_UUID = 58, 
    K_BIND = 59, K_CLEAR = 60, K_INTO = 61, K_AS = 62, K_ALL = 63, K_IRI = 64, 
    K_BASE = 65, K_BY = 66, K_DROP = 67, K_LOAD = 68, K_WITH = 69, K_BNODE = 70, 
    K_WHERE = 71, K_AVG = 72, K_SAMPLE = 73, K_UCASE = 74, K_SERVICE = 75, 
    K_MINUS = 76, K_SAMETERM = 77, K_STRSTARTS = 78, K_STR = 79, K_MOVE = 80, 
    K_HAVING = 81, K_COALESCE = 82, K_STRBEFORE = 83, K_ABS = 84, K_ISLITERAL = 85, 
    K_STRAFTER = 86, K_STRLEN = 87, K_LANG = 88, K_CREATE = 89, K_DESC = 90, 
    K_MAX = 91, K_FILTER = 92, K_USING = 93, K_NOT = 94, K_STRENDS = 95, 
    K_OFFSET = 96, K_CONTAINS = 97, K_PREFIX = 98, K_MINUTES = 99, K_REPLACE = 100, 
    K_REGEX = 101, K_DELETE = 102, K_SEPARATOR = 103, K_DAY = 104, K_SILENT = 105, 
    K_STRLANG = 106, K_ORDER = 107, K_ROUND = 108, K_GRAPH = 109, K_SECONDS = 110, 
    K_URI = 111, K_DISTINCT = 112, K_EXISTS = 113, K_GROUP = 114, K_SUM = 115, 
    K_REDUCED = 116, K_FROM = 117, K_LANGMATCHES = 118, K_ISURI = 119, K_TO = 120, 
    K_ISIRI = 121, K_RAND = 122, K_STRDT = 123, K_COUNT = 124, K_DESCRIBE = 125, 
    K_VALUES = 126, K_LCASE = 127, K_OPTIONAL = 128, K_LIMIT = 129, K_SUBSTR = 130, 
    K_SIMPLECYCLEPATH = 131, K_SIMPLECYCLEBOOLEAN = 132, K_CYCLEPATH = 133, 
    K_CYCLEBOOLEAN = 134, K_SHORTESTPATH = 135, K_SHORTESTPATHLEN = 136, 
    K_KHOPREACHABLE = 137, K_KHOPENUMERATE = 138, K_KHOPREACHABLEPATH = 139, 
    K_PPR = 140, K_TRIANGLECOUNTING = 141, K_CLOSENESSCENTRALITY = 142, 
    K_BFSCOUNT = 143, K_PR = 144, K_ALPHA = 145, K_MAXITER = 146, K_TOL = 147, 
    K_SSSP = 148, K_SSSPLEN = 149, K_LABELPROP = 150, K_WCC = 151, K_CLUSTERCOEFF = 152, 
    K_MAXIMUMCLIQUE = 153, K_PFN = 154, KK_INSERTDATA = 155, KK_DELETEDATA = 156, 
    KK_DELETEWHERE = 157, KK_ENCODE_FOR_URI = 158, KK_MD5 = 159, KK_SHA1 = 160, 
    KK_SHA256 = 161, KK_SHA384 = 162, KK_SHA512 = 163, KK_GROUP_CONCAT = 164, 
    IRIREF = 165, PNAME_NS = 166, PNAME_LN = 167, BLANK_NODE_LABEL = 168, 
    VAR1 = 169, VAR2 = 170, LANGTAG = 171, INTEGER = 172, DECIMAL = 173, 
    DOUBLE = 174, INTEGER_POSITIVE = 175, DECIMAL_POSITIVE = 176, DOUBLE_POSITIVE = 177, 
    INTEGER_NEGATIVE = 178, DECIMAL_NEGATIVE = 179, DOUBLE_NEGATIVE = 180, 
    EXPONENT = 181, STRING_LITERAL1 = 182, STRING_LITERAL2 = 183, STRING_LITERAL_LONG1 = 184, 
    STRING_LITERAL_LONG2 = 185, ECHAR = 186, NIL = 187, WS = 188, ANON = 189, 
    PN_CHARS_BASE = 190, PN_CHARS_U = 191, VARNAME = 192, PN_CHARS = 193, 
    PN_PREFIX = 194, PN_LOCAL = 195, PLX = 196, PERCENT = 197, HEX = 198, 
    PN_LOCAL_ESC = 199, COMMENT = 200
  };

  enum {
    RuleEntry = 0, RuleQueryUnit = 1, RuleQuery = 2, RuleUpdateUnit = 3, 
    RulePrologue = 4, RuleBaseDecl = 5, RulePrefixDecl = 6, RuleSelectquery = 7, 
    RuleSubSelect = 8, RuleSelectClause = 9, RuleExpressionAsVar = 10, RuleConstructquery = 11, 
    RuleDescribequery = 12, RuleAskquery = 13, RuleDatasetClause = 14, RuleDefaultGraphClause = 15, 
    RuleNamedGraphClause = 16, RuleSourceSelector = 17, RuleWhereClause = 18, 
    RuleSolutionModifier = 19, RuleGroupClause = 20, RuleGroupCondition = 21, 
    RuleHavingClause = 22, RuleHavingCondition = 23, RuleOrderClause = 24, 
    RuleOrderCondition = 25, RuleLimitoffsetClauses = 26, RuleLimitClause = 27, 
    RuleOffsetClause = 28, RuleValuesClause = 29, RuleUpdate = 30, RuleUpdate1 = 31, 
    RuleLoad = 32, RuleClear = 33, RuleDrop = 34, RuleCreate = 35, RuleAdd = 36, 
    RuleMove = 37, RuleCopy = 38, RuleInsertData = 39, RuleDeleteData = 40, 
    RuleDeleteWhere = 41, RuleModify = 42, RuleDeleteClause = 43, RuleInsertClause = 44, 
    RuleUsingClause = 45, RuleGraphOrDefault = 46, RuleGraphRef = 47, RuleGraphRefAll = 48, 
    RuleQuadPattern = 49, RuleQuadData = 50, RuleQuads = 51, RuleQuadsNotTriples = 52, 
    RuleTriplesTemplate = 53, RuleGroupGraphPattern = 54, RuleGroupGraphPatternSub = 55, 
    RuleGraphPatternTriplesBlock = 56, RuleTriplesBlock = 57, RuleGraphPatternNotTriples = 58, 
    RuleOptionalGraphPattern = 59, RuleGraphGraphPattern = 60, RuleServiceGraphPattern = 61, 
    RuleBind = 62, RuleInlineData = 63, RuleDataBlock = 64, RuleInlineDataOnevar = 65, 
    RuleInlineDataFull = 66, RuleDataBlockValue = 67, RuleMinusGraphPattern = 68, 
    RuleGroupOrUnionGraphPattern = 69, RuleFilter = 70, RuleConstraint = 71, 
    RuleFunctionCall = 72, RuleArgList = 73, RuleExpressionList = 74, RuleConstructTemplate = 75, 
    RuleConstructTriples = 76, RuleTriplesSameSubject = 77, RulePropertyList = 78, 
    RulePropertyListNotEmpty = 79, RuleVerb = 80, RuleObjectList = 81, RuleObject = 82, 
    RuleTriplesSameSubjectpath = 83, RulePropertyListpath = 84, RulePropertyListpathNotEmpty = 85, 
    RuleVerbpathOrSimple = 86, RuleVerbpath = 87, RuleVerbSimple = 88, RuleObjectListpath = 89, 
    RuleObjectpath = 90, RulePath = 91, RulePathAlternative = 92, RulePathSequence = 93, 
    RulePathElt = 94, RulePathEltOrInverse = 95, RulePathMod = 96, RulePathPrimary = 97, 
    RulePathNegatedPropertySet = 98, RulePathOneInPropertySet = 99, RuleTriplesNode = 100, 
    RuleBlankNodepropertyList = 101, RuleTriplesNodepath = 102, RuleBlankNodepropertyListpath = 103, 
    RuleCollection = 104, RuleCollectionpath = 105, RuleGraphNode = 106, 
    RuleGraphNodepath = 107, RuleVarOrTerm = 108, RuleVarOrIri = 109, RuleVarOrIriSet = 110, 
    RuleVar = 111, RuleGraphTerm = 112, RuleExpression = 113, RuleConditionalOrexpression = 114, 
    RuleConditionalAndexpression = 115, RuleValueLogical = 116, RuleRelationalexpression = 117, 
    RuleNumericexpression = 118, RuleAdditiveexpression = 119, RuleMultiplicativeexpression = 120, 
    RuleUnaryexpression = 121, RulePrimaryexpression = 122, RuleBrackettedexpression = 123, 
    RulePredSet = 124, RuleBuiltInCall = 125, RuleRegexexpression = 126, 
    RuleSubstringexpression = 127, RuleStrReplaceexpression = 128, RuleExistsFunc = 129, 
    RuleNotexistsFunc = 130, RuleAggregate = 131, RuleIriOrFunction = 132, 
    RuleRDFLiteral = 133, RuleNumericLiteral = 134, RuleNumericLiteralUnsigned = 135, 
    RuleNumericLiteralPositive = 136, RuleNumericLiteralNegative = 137, 
    RuleNum_integer = 138, RuleNum_decimal = 139, RuleNum_double = 140, 
    RuleInteger_positive = 141, RuleDecimal_positive = 142, RuleDouble_positive = 143, 
    RuleInteger_negative = 144, RuleDecimal_negative = 145, RuleDouble_negative = 146, 
    RuleBooleanLiteral = 147, RuleString = 148, RuleIri = 149, RulePrefixedName = 150, 
    RuleBlankNode = 151
  };

  SPARQLParser(antlr4::TokenStream *input);
  ~SPARQLParser();

  virtual std::string getGrammarFileName() const override;
  virtual const antlr4::atn::ATN& getATN() const override { return _atn; };
  virtual const std::vector<std::string>& getTokenNames() const override { return _tokenNames; }; // deprecated: use vocabulary instead.
  virtual const std::vector<std::string>& getRuleNames() const override;
  virtual antlr4::dfa::Vocabulary& getVocabulary() const override;


  class EntryContext;
  class QueryUnitContext;
  class QueryContext;
  class UpdateUnitContext;
  class PrologueContext;
  class BaseDeclContext;
  class PrefixDeclContext;
  class SelectqueryContext;
  class SubSelectContext;
  class SelectClauseContext;
  class ExpressionAsVarContext;
  class ConstructqueryContext;
  class DescribequeryContext;
  class AskqueryContext;
  class DatasetClauseContext;
  class DefaultGraphClauseContext;
  class NamedGraphClauseContext;
  class SourceSelectorContext;
  class WhereClauseContext;
  class SolutionModifierContext;
  class GroupClauseContext;
  class GroupConditionContext;
  class HavingClauseContext;
  class HavingConditionContext;
  class OrderClauseContext;
  class OrderConditionContext;
  class LimitoffsetClausesContext;
  class LimitClauseContext;
  class OffsetClauseContext;
  class ValuesClauseContext;
  class UpdateContext;
  class Update1Context;
  class LoadContext;
  class ClearContext;
  class DropContext;
  class CreateContext;
  class AddContext;
  class MoveContext;
  class CopyContext;
  class InsertDataContext;
  class DeleteDataContext;
  class DeleteWhereContext;
  class ModifyContext;
  class DeleteClauseContext;
  class InsertClauseContext;
  class UsingClauseContext;
  class GraphOrDefaultContext;
  class GraphRefContext;
  class GraphRefAllContext;
  class QuadPatternContext;
  class QuadDataContext;
  class QuadsContext;
  class QuadsNotTriplesContext;
  class TriplesTemplateContext;
  class GroupGraphPatternContext;
  class GroupGraphPatternSubContext;
  class GraphPatternTriplesBlockContext;
  class TriplesBlockContext;
  class GraphPatternNotTriplesContext;
  class OptionalGraphPatternContext;
  class GraphGraphPatternContext;
  class ServiceGraphPatternContext;
  class BindContext;
  class InlineDataContext;
  class DataBlockContext;
  class InlineDataOnevarContext;
  class InlineDataFullContext;
  class DataBlockValueContext;
  class MinusGraphPatternContext;
  class GroupOrUnionGraphPatternContext;
  class FilterContext;
  class ConstraintContext;
  class FunctionCallContext;
  class ArgListContext;
  class ExpressionListContext;
  class ConstructTemplateContext;
  class ConstructTriplesContext;
  class TriplesSameSubjectContext;
  class PropertyListContext;
  class PropertyListNotEmptyContext;
  class VerbContext;
  class ObjectListContext;
  class ObjectContext;
  class TriplesSameSubjectpathContext;
  class PropertyListpathContext;
  class PropertyListpathNotEmptyContext;
  class VerbpathOrSimpleContext;
  class VerbpathContext;
  class VerbSimpleContext;
  class ObjectListpathContext;
  class ObjectpathContext;
  class PathContext;
  class PathAlternativeContext;
  class PathSequenceContext;
  class PathEltContext;
  class PathEltOrInverseContext;
  class PathModContext;
  class PathPrimaryContext;
  class PathNegatedPropertySetContext;
  class PathOneInPropertySetContext;
  class TriplesNodeContext;
  class BlankNodepropertyListContext;
  class TriplesNodepathContext;
  class BlankNodepropertyListpathContext;
  class CollectionContext;
  class CollectionpathContext;
  class GraphNodeContext;
  class GraphNodepathContext;
  class VarOrTermContext;
  class VarOrIriContext;
  class VarOrIriSetContext;
  class VarContext;
  class GraphTermContext;
  class ExpressionContext;
  class ConditionalOrexpressionContext;
  class ConditionalAndexpressionContext;
  class ValueLogicalContext;
  class RelationalexpressionContext;
  class NumericexpressionContext;
  class AdditiveexpressionContext;
  class MultiplicativeexpressionContext;
  class UnaryexpressionContext;
  class PrimaryexpressionContext;
  class BrackettedexpressionContext;
  class PredSetContext;
  class BuiltInCallContext;
  class RegexexpressionContext;
  class SubstringexpressionContext;
  class StrReplaceexpressionContext;
  class ExistsFuncContext;
  class NotexistsFuncContext;
  class AggregateContext;
  class IriOrFunctionContext;
  class RDFLiteralContext;
  class NumericLiteralContext;
  class NumericLiteralUnsignedContext;
  class NumericLiteralPositiveContext;
  class NumericLiteralNegativeContext;
  class Num_integerContext;
  class Num_decimalContext;
  class Num_doubleContext;
  class Integer_positiveContext;
  class Decimal_positiveContext;
  class Double_positiveContext;
  class Integer_negativeContext;
  class Decimal_negativeContext;
  class Double_negativeContext;
  class BooleanLiteralContext;
  class StringContext;
  class IriContext;
  class PrefixedNameContext;
  class BlankNodeContext; 

  class  EntryContext : public antlr4::ParserRuleContext {
  public:
    EntryContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    QueryUnitContext *queryUnit();
    UpdateUnitContext *updateUnit();
    antlr4::tree::TerminalNode *EOF();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  EntryContext* entry();

  class  QueryUnitContext : public antlr4::ParserRuleContext {
  public:
    QueryUnitContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    QueryContext *query();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  QueryUnitContext* queryUnit();

  class  QueryContext : public antlr4::ParserRuleContext {
  public:
    QueryContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    PrologueContext *prologue();
    ValuesClauseContext *valuesClause();
    SelectqueryContext *selectquery();
    ConstructqueryContext *constructquery();
    DescribequeryContext *describequery();
    AskqueryContext *askquery();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  QueryContext* query();

  class  UpdateUnitContext : public antlr4::ParserRuleContext {
  public:
    UpdateUnitContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    UpdateContext *update();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  UpdateUnitContext* updateUnit();

  class  PrologueContext : public antlr4::ParserRuleContext {
  public:
    PrologueContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    std::vector<BaseDeclContext *> baseDecl();
    BaseDeclContext* baseDecl(size_t i);
    std::vector<PrefixDeclContext *> prefixDecl();
    PrefixDeclContext* prefixDecl(size_t i);

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  PrologueContext* prologue();

  class  BaseDeclContext : public antlr4::ParserRuleContext {
  public:
    BaseDeclContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *K_BASE();
    antlr4::tree::TerminalNode *IRIREF();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  BaseDeclContext* baseDecl();

  class  PrefixDeclContext : public antlr4::ParserRuleContext {
  public:
    PrefixDeclContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *K_PREFIX();
    antlr4::tree::TerminalNode *PNAME_NS();
    antlr4::tree::TerminalNode *IRIREF();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  PrefixDeclContext* prefixDecl();

  class  SelectqueryContext : public antlr4::ParserRuleContext {
  public:
    SelectqueryContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    SelectClauseContext *selectClause();
    WhereClauseContext *whereClause();
    SolutionModifierContext *solutionModifier();
    std::vector<DatasetClauseContext *> datasetClause();
    DatasetClauseContext* datasetClause(size_t i);

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  SelectqueryContext* selectquery();

  class  SubSelectContext : public antlr4::ParserRuleContext {
  public:
    SubSelectContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    SelectClauseContext *selectClause();
    WhereClauseContext *whereClause();
    SolutionModifierContext *solutionModifier();
    ValuesClauseContext *valuesClause();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  SubSelectContext* subSelect();

  class  SelectClauseContext : public antlr4::ParserRuleContext {
  public:
    SelectClauseContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *K_SELECT();
    antlr4::tree::TerminalNode *K_DISTINCT();
    antlr4::tree::TerminalNode *K_REDUCED();
    std::vector<VarContext *> var();
    VarContext* var(size_t i);
    std::vector<ExpressionAsVarContext *> expressionAsVar();
    ExpressionAsVarContext* expressionAsVar(size_t i);

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  SelectClauseContext* selectClause();

  class  ExpressionAsVarContext : public antlr4::ParserRuleContext {
  public:
    ExpressionAsVarContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    ExpressionContext *expression();
    antlr4::tree::TerminalNode *K_AS();
    VarContext *var();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  ExpressionAsVarContext* expressionAsVar();

  class  ConstructqueryContext : public antlr4::ParserRuleContext {
  public:
    ConstructqueryContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *K_CONSTRUCT();
    ConstructTemplateContext *constructTemplate();
    WhereClauseContext *whereClause();
    SolutionModifierContext *solutionModifier();
    antlr4::tree::TerminalNode *K_WHERE();
    std::vector<DatasetClauseContext *> datasetClause();
    DatasetClauseContext* datasetClause(size_t i);
    TriplesTemplateContext *triplesTemplate();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  ConstructqueryContext* constructquery();

  class  DescribequeryContext : public antlr4::ParserRuleContext {
  public:
    DescribequeryContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *K_DESCRIBE();
    SolutionModifierContext *solutionModifier();
    std::vector<DatasetClauseContext *> datasetClause();
    DatasetClauseContext* datasetClause(size_t i);
    WhereClauseContext *whereClause();
    std::vector<VarOrIriContext *> varOrIri();
    VarOrIriContext* varOrIri(size_t i);

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  DescribequeryContext* describequery();

  class  AskqueryContext : public antlr4::ParserRuleContext {
  public:
    AskqueryContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *K_ASK();
    WhereClauseContext *whereClause();
    SolutionModifierContext *solutionModifier();
    std::vector<DatasetClauseContext *> datasetClause();
    DatasetClauseContext* datasetClause(size_t i);

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  AskqueryContext* askquery();

  class  DatasetClauseContext : public antlr4::ParserRuleContext {
  public:
    DatasetClauseContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *K_FROM();
    DefaultGraphClauseContext *defaultGraphClause();
    NamedGraphClauseContext *namedGraphClause();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  DatasetClauseContext* datasetClause();

  class  DefaultGraphClauseContext : public antlr4::ParserRuleContext {
  public:
    DefaultGraphClauseContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    SourceSelectorContext *sourceSelector();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  DefaultGraphClauseContext* defaultGraphClause();

  class  NamedGraphClauseContext : public antlr4::ParserRuleContext {
  public:
    NamedGraphClauseContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *K_NAMED();
    SourceSelectorContext *sourceSelector();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  NamedGraphClauseContext* namedGraphClause();

  class  SourceSelectorContext : public antlr4::ParserRuleContext {
  public:
    SourceSelectorContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    IriContext *iri();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  SourceSelectorContext* sourceSelector();

  class  WhereClauseContext : public antlr4::ParserRuleContext {
  public:
    WhereClauseContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    GroupGraphPatternContext *groupGraphPattern();
    antlr4::tree::TerminalNode *K_WHERE();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  WhereClauseContext* whereClause();

  class  SolutionModifierContext : public antlr4::ParserRuleContext {
  public:
    SolutionModifierContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    GroupClauseContext *groupClause();
    HavingClauseContext *havingClause();
    OrderClauseContext *orderClause();
    LimitoffsetClausesContext *limitoffsetClauses();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  SolutionModifierContext* solutionModifier();

  class  GroupClauseContext : public antlr4::ParserRuleContext {
  public:
    GroupClauseContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *K_GROUP();
    antlr4::tree::TerminalNode *K_BY();
    std::vector<GroupConditionContext *> groupCondition();
    GroupConditionContext* groupCondition(size_t i);

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  GroupClauseContext* groupClause();

  class  GroupConditionContext : public antlr4::ParserRuleContext {
  public:
    GroupConditionContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    BuiltInCallContext *builtInCall();
    FunctionCallContext *functionCall();
    ExpressionContext *expression();
    antlr4::tree::TerminalNode *K_AS();
    VarContext *var();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  GroupConditionContext* groupCondition();

  class  HavingClauseContext : public antlr4::ParserRuleContext {
  public:
    HavingClauseContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *K_HAVING();
    std::vector<HavingConditionContext *> havingCondition();
    HavingConditionContext* havingCondition(size_t i);

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  HavingClauseContext* havingClause();

  class  HavingConditionContext : public antlr4::ParserRuleContext {
  public:
    HavingConditionContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    ConstraintContext *constraint();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  HavingConditionContext* havingCondition();

  class  OrderClauseContext : public antlr4::ParserRuleContext {
  public:
    OrderClauseContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *K_ORDER();
    antlr4::tree::TerminalNode *K_BY();
    std::vector<OrderConditionContext *> orderCondition();
    OrderConditionContext* orderCondition(size_t i);

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  OrderClauseContext* orderClause();

  class  OrderConditionContext : public antlr4::ParserRuleContext {
  public:
    OrderConditionContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    BrackettedexpressionContext *brackettedexpression();
    antlr4::tree::TerminalNode *K_ASC();
    antlr4::tree::TerminalNode *K_DESC();
    ConstraintContext *constraint();
    VarContext *var();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  OrderConditionContext* orderCondition();

  class  LimitoffsetClausesContext : public antlr4::ParserRuleContext {
  public:
    LimitoffsetClausesContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    LimitClauseContext *limitClause();
    OffsetClauseContext *offsetClause();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  LimitoffsetClausesContext* limitoffsetClauses();

  class  LimitClauseContext : public antlr4::ParserRuleContext {
  public:
    LimitClauseContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *K_LIMIT();
    antlr4::tree::TerminalNode *INTEGER();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  LimitClauseContext* limitClause();

  class  OffsetClauseContext : public antlr4::ParserRuleContext {
  public:
    OffsetClauseContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *K_OFFSET();
    antlr4::tree::TerminalNode *INTEGER();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  OffsetClauseContext* offsetClause();

  class  ValuesClauseContext : public antlr4::ParserRuleContext {
  public:
    ValuesClauseContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *K_VALUES();
    DataBlockContext *dataBlock();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  ValuesClauseContext* valuesClause();

  class  UpdateContext : public antlr4::ParserRuleContext {
  public:
    UpdateContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    PrologueContext *prologue();
    Update1Context *update1();
    UpdateContext *update();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  UpdateContext* update();

  class  Update1Context : public antlr4::ParserRuleContext {
  public:
    Update1Context(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    LoadContext *load();
    ClearContext *clear();
    DropContext *drop();
    AddContext *add();
    MoveContext *move();
    CopyContext *copy();
    CreateContext *create();
    InsertDataContext *insertData();
    DeleteDataContext *deleteData();
    DeleteWhereContext *deleteWhere();
    ModifyContext *modify();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  Update1Context* update1();

  class  LoadContext : public antlr4::ParserRuleContext {
  public:
    LoadContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *K_LOAD();
    IriContext *iri();
    antlr4::tree::TerminalNode *K_SILENT();
    antlr4::tree::TerminalNode *K_INTO();
    GraphRefContext *graphRef();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  LoadContext* load();

  class  ClearContext : public antlr4::ParserRuleContext {
  public:
    ClearContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *K_CLEAR();
    GraphRefAllContext *graphRefAll();
    antlr4::tree::TerminalNode *K_SILENT();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  ClearContext* clear();

  class  DropContext : public antlr4::ParserRuleContext {
  public:
    DropContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *K_DROP();
    GraphRefAllContext *graphRefAll();
    antlr4::tree::TerminalNode *K_SILENT();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  DropContext* drop();

  class  CreateContext : public antlr4::ParserRuleContext {
  public:
    CreateContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *K_CREATE();
    GraphRefContext *graphRef();
    antlr4::tree::TerminalNode *K_SILENT();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  CreateContext* create();

  class  AddContext : public antlr4::ParserRuleContext {
  public:
    AddContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *K_ADD();
    std::vector<GraphOrDefaultContext *> graphOrDefault();
    GraphOrDefaultContext* graphOrDefault(size_t i);
    antlr4::tree::TerminalNode *K_TO();
    antlr4::tree::TerminalNode *K_SILENT();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  AddContext* add();

  class  MoveContext : public antlr4::ParserRuleContext {
  public:
    MoveContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *K_MOVE();
    std::vector<GraphOrDefaultContext *> graphOrDefault();
    GraphOrDefaultContext* graphOrDefault(size_t i);
    antlr4::tree::TerminalNode *K_TO();
    antlr4::tree::TerminalNode *K_SILENT();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  MoveContext* move();

  class  CopyContext : public antlr4::ParserRuleContext {
  public:
    CopyContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *K_COPY();
    std::vector<GraphOrDefaultContext *> graphOrDefault();
    GraphOrDefaultContext* graphOrDefault(size_t i);
    antlr4::tree::TerminalNode *K_TO();
    antlr4::tree::TerminalNode *K_SILENT();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  CopyContext* copy();

  class  InsertDataContext : public antlr4::ParserRuleContext {
  public:
    InsertDataContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *KK_INSERTDATA();
    QuadDataContext *quadData();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  InsertDataContext* insertData();

  class  DeleteDataContext : public antlr4::ParserRuleContext {
  public:
    DeleteDataContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *KK_DELETEDATA();
    QuadDataContext *quadData();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  DeleteDataContext* deleteData();

  class  DeleteWhereContext : public antlr4::ParserRuleContext {
  public:
    DeleteWhereContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *KK_DELETEWHERE();
    QuadPatternContext *quadPattern();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  DeleteWhereContext* deleteWhere();

  class  ModifyContext : public antlr4::ParserRuleContext {
  public:
    ModifyContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *K_WHERE();
    GroupGraphPatternContext *groupGraphPattern();
    DeleteClauseContext *deleteClause();
    InsertClauseContext *insertClause();
    antlr4::tree::TerminalNode *K_WITH();
    IriContext *iri();
    std::vector<UsingClauseContext *> usingClause();
    UsingClauseContext* usingClause(size_t i);

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  ModifyContext* modify();

  class  DeleteClauseContext : public antlr4::ParserRuleContext {
  public:
    DeleteClauseContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *K_DELETE();
    QuadPatternContext *quadPattern();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  DeleteClauseContext* deleteClause();

  class  InsertClauseContext : public antlr4::ParserRuleContext {
  public:
    InsertClauseContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *K_INSERT();
    QuadPatternContext *quadPattern();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  InsertClauseContext* insertClause();

  class  UsingClauseContext : public antlr4::ParserRuleContext {
  public:
    UsingClauseContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *K_USING();
    IriContext *iri();
    antlr4::tree::TerminalNode *K_NAMED();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  UsingClauseContext* usingClause();

  class  GraphOrDefaultContext : public antlr4::ParserRuleContext {
  public:
    GraphOrDefaultContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *K_DEFAULT();
    IriContext *iri();
    antlr4::tree::TerminalNode *K_GRAPH();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  GraphOrDefaultContext* graphOrDefault();

  class  GraphRefContext : public antlr4::ParserRuleContext {
  public:
    GraphRefContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *K_GRAPH();
    IriContext *iri();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  GraphRefContext* graphRef();

  class  GraphRefAllContext : public antlr4::ParserRuleContext {
  public:
    GraphRefAllContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    GraphRefContext *graphRef();
    antlr4::tree::TerminalNode *K_DEFAULT();
    antlr4::tree::TerminalNode *K_NAMED();
    antlr4::tree::TerminalNode *K_ALL();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  GraphRefAllContext* graphRefAll();

  class  QuadPatternContext : public antlr4::ParserRuleContext {
  public:
    QuadPatternContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    QuadsContext *quads();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  QuadPatternContext* quadPattern();

  class  QuadDataContext : public antlr4::ParserRuleContext {
  public:
    QuadDataContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    QuadsContext *quads();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  QuadDataContext* quadData();

  class  QuadsContext : public antlr4::ParserRuleContext {
  public:
    QuadsContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    std::vector<TriplesTemplateContext *> triplesTemplate();
    TriplesTemplateContext* triplesTemplate(size_t i);
    std::vector<QuadsNotTriplesContext *> quadsNotTriples();
    QuadsNotTriplesContext* quadsNotTriples(size_t i);

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  QuadsContext* quads();

  class  QuadsNotTriplesContext : public antlr4::ParserRuleContext {
  public:
    QuadsNotTriplesContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *K_GRAPH();
    VarOrIriContext *varOrIri();
    TriplesTemplateContext *triplesTemplate();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  QuadsNotTriplesContext* quadsNotTriples();

  class  TriplesTemplateContext : public antlr4::ParserRuleContext {
  public:
    TriplesTemplateContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    TriplesSameSubjectContext *triplesSameSubject();
    TriplesTemplateContext *triplesTemplate();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  TriplesTemplateContext* triplesTemplate();

  class  GroupGraphPatternContext : public antlr4::ParserRuleContext {
  public:
    GroupGraphPatternContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    SubSelectContext *subSelect();
    GroupGraphPatternSubContext *groupGraphPatternSub();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  GroupGraphPatternContext* groupGraphPattern();

  class  GroupGraphPatternSubContext : public antlr4::ParserRuleContext {
  public:
    GroupGraphPatternSubContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    TriplesBlockContext *triplesBlock();
    std::vector<GraphPatternTriplesBlockContext *> graphPatternTriplesBlock();
    GraphPatternTriplesBlockContext* graphPatternTriplesBlock(size_t i);

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  GroupGraphPatternSubContext* groupGraphPatternSub();

  class  GraphPatternTriplesBlockContext : public antlr4::ParserRuleContext {
  public:
    GraphPatternTriplesBlockContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    GraphPatternNotTriplesContext *graphPatternNotTriples();
    TriplesBlockContext *triplesBlock();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  GraphPatternTriplesBlockContext* graphPatternTriplesBlock();

  class  TriplesBlockContext : public antlr4::ParserRuleContext {
  public:
    TriplesBlockContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    TriplesSameSubjectpathContext *triplesSameSubjectpath();
    TriplesBlockContext *triplesBlock();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  TriplesBlockContext* triplesBlock();

  class  GraphPatternNotTriplesContext : public antlr4::ParserRuleContext {
  public:
    GraphPatternNotTriplesContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    GroupOrUnionGraphPatternContext *groupOrUnionGraphPattern();
    OptionalGraphPatternContext *optionalGraphPattern();
    MinusGraphPatternContext *minusGraphPattern();
    GraphGraphPatternContext *graphGraphPattern();
    ServiceGraphPatternContext *serviceGraphPattern();
    FilterContext *filter();
    BindContext *bind();
    InlineDataContext *inlineData();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  GraphPatternNotTriplesContext* graphPatternNotTriples();

  class  OptionalGraphPatternContext : public antlr4::ParserRuleContext {
  public:
    OptionalGraphPatternContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *K_OPTIONAL();
    GroupGraphPatternContext *groupGraphPattern();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  OptionalGraphPatternContext* optionalGraphPattern();

  class  GraphGraphPatternContext : public antlr4::ParserRuleContext {
  public:
    GraphGraphPatternContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *K_GRAPH();
    VarOrIriContext *varOrIri();
    GroupGraphPatternContext *groupGraphPattern();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  GraphGraphPatternContext* graphGraphPattern();

  class  ServiceGraphPatternContext : public antlr4::ParserRuleContext {
  public:
    ServiceGraphPatternContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *K_SERVICE();
    VarOrIriContext *varOrIri();
    GroupGraphPatternContext *groupGraphPattern();
    antlr4::tree::TerminalNode *K_SILENT();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  ServiceGraphPatternContext* serviceGraphPattern();

  class  BindContext : public antlr4::ParserRuleContext {
  public:
    BindContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *K_BIND();
    ExpressionContext *expression();
    antlr4::tree::TerminalNode *K_AS();
    VarContext *var();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  BindContext* bind();

  class  InlineDataContext : public antlr4::ParserRuleContext {
  public:
    InlineDataContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *K_VALUES();
    DataBlockContext *dataBlock();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  InlineDataContext* inlineData();

  class  DataBlockContext : public antlr4::ParserRuleContext {
  public:
    DataBlockContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    InlineDataOnevarContext *inlineDataOnevar();
    InlineDataFullContext *inlineDataFull();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  DataBlockContext* dataBlock();

  class  InlineDataOnevarContext : public antlr4::ParserRuleContext {
  public:
    InlineDataOnevarContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    VarContext *var();
    std::vector<DataBlockValueContext *> dataBlockValue();
    DataBlockValueContext* dataBlockValue(size_t i);

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  InlineDataOnevarContext* inlineDataOnevar();

  class  InlineDataFullContext : public antlr4::ParserRuleContext {
  public:
    InlineDataFullContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    std::vector<antlr4::tree::TerminalNode *> NIL();
    antlr4::tree::TerminalNode* NIL(size_t i);
    std::vector<VarContext *> var();
    VarContext* var(size_t i);
    std::vector<DataBlockValueContext *> dataBlockValue();
    DataBlockValueContext* dataBlockValue(size_t i);

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  InlineDataFullContext* inlineDataFull();

  class  DataBlockValueContext : public antlr4::ParserRuleContext {
  public:
    DataBlockValueContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    IriContext *iri();
    RDFLiteralContext *rDFLiteral();
    NumericLiteralContext *numericLiteral();
    BooleanLiteralContext *booleanLiteral();
    antlr4::tree::TerminalNode *K_UNDEF();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  DataBlockValueContext* dataBlockValue();

  class  MinusGraphPatternContext : public antlr4::ParserRuleContext {
  public:
    MinusGraphPatternContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *K_MINUS();
    GroupGraphPatternContext *groupGraphPattern();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  MinusGraphPatternContext* minusGraphPattern();

  class  GroupOrUnionGraphPatternContext : public antlr4::ParserRuleContext {
  public:
    GroupOrUnionGraphPatternContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    std::vector<GroupGraphPatternContext *> groupGraphPattern();
    GroupGraphPatternContext* groupGraphPattern(size_t i);
    std::vector<antlr4::tree::TerminalNode *> K_UNION();
    antlr4::tree::TerminalNode* K_UNION(size_t i);

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  GroupOrUnionGraphPatternContext* groupOrUnionGraphPattern();

  class  FilterContext : public antlr4::ParserRuleContext {
  public:
    FilterContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *K_FILTER();
    ConstraintContext *constraint();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  FilterContext* filter();

  class  ConstraintContext : public antlr4::ParserRuleContext {
  public:
    ConstraintContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    BrackettedexpressionContext *brackettedexpression();
    BuiltInCallContext *builtInCall();
    FunctionCallContext *functionCall();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  ConstraintContext* constraint();

  class  FunctionCallContext : public antlr4::ParserRuleContext {
  public:
    FunctionCallContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    IriContext *iri();
    ArgListContext *argList();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  FunctionCallContext* functionCall();

  class  ArgListContext : public antlr4::ParserRuleContext {
  public:
    ArgListContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *NIL();
    std::vector<ExpressionContext *> expression();
    ExpressionContext* expression(size_t i);
    antlr4::tree::TerminalNode *K_DISTINCT();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  ArgListContext* argList();

  class  ExpressionListContext : public antlr4::ParserRuleContext {
  public:
    ExpressionListContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *NIL();
    std::vector<ExpressionContext *> expression();
    ExpressionContext* expression(size_t i);

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  ExpressionListContext* expressionList();

  class  ConstructTemplateContext : public antlr4::ParserRuleContext {
  public:
    ConstructTemplateContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    ConstructTriplesContext *constructTriples();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  ConstructTemplateContext* constructTemplate();

  class  ConstructTriplesContext : public antlr4::ParserRuleContext {
  public:
    ConstructTriplesContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    TriplesSameSubjectContext *triplesSameSubject();
    ConstructTriplesContext *constructTriples();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  ConstructTriplesContext* constructTriples();

  class  TriplesSameSubjectContext : public antlr4::ParserRuleContext {
  public:
    TriplesSameSubjectContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    VarOrTermContext *varOrTerm();
    PropertyListNotEmptyContext *propertyListNotEmpty();
    TriplesNodeContext *triplesNode();
    PropertyListContext *propertyList();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  TriplesSameSubjectContext* triplesSameSubject();

  class  PropertyListContext : public antlr4::ParserRuleContext {
  public:
    PropertyListContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    PropertyListNotEmptyContext *propertyListNotEmpty();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  PropertyListContext* propertyList();

  class  PropertyListNotEmptyContext : public antlr4::ParserRuleContext {
  public:
    PropertyListNotEmptyContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    std::vector<VerbContext *> verb();
    VerbContext* verb(size_t i);
    std::vector<ObjectListContext *> objectList();
    ObjectListContext* objectList(size_t i);

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  PropertyListNotEmptyContext* propertyListNotEmpty();

  class  VerbContext : public antlr4::ParserRuleContext {
  public:
    VerbContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    VarOrIriContext *varOrIri();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  VerbContext* verb();

  class  ObjectListContext : public antlr4::ParserRuleContext {
  public:
    ObjectListContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    std::vector<ObjectContext *> object();
    ObjectContext* object(size_t i);

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  ObjectListContext* objectList();

  class  ObjectContext : public antlr4::ParserRuleContext {
  public:
    ObjectContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    GraphNodeContext *graphNode();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  ObjectContext* object();

  class  TriplesSameSubjectpathContext : public antlr4::ParserRuleContext {
  public:
    TriplesSameSubjectpathContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    VarOrTermContext *varOrTerm();
    PropertyListpathNotEmptyContext *propertyListpathNotEmpty();
    TriplesNodepathContext *triplesNodepath();
    PropertyListpathContext *propertyListpath();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  TriplesSameSubjectpathContext* triplesSameSubjectpath();

  class  PropertyListpathContext : public antlr4::ParserRuleContext {
  public:
    PropertyListpathContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    PropertyListpathNotEmptyContext *propertyListpathNotEmpty();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  PropertyListpathContext* propertyListpath();

  class  PropertyListpathNotEmptyContext : public antlr4::ParserRuleContext {
  public:
    PropertyListpathNotEmptyContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    std::vector<VerbpathOrSimpleContext *> verbpathOrSimple();
    VerbpathOrSimpleContext* verbpathOrSimple(size_t i);
    ObjectListpathContext *objectListpath();
    std::vector<ObjectListContext *> objectList();
    ObjectListContext* objectList(size_t i);

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  PropertyListpathNotEmptyContext* propertyListpathNotEmpty();

  class  VerbpathOrSimpleContext : public antlr4::ParserRuleContext {
  public:
    VerbpathOrSimpleContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    VerbpathContext *verbpath();
    VerbSimpleContext *verbSimple();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  VerbpathOrSimpleContext* verbpathOrSimple();

  class  VerbpathContext : public antlr4::ParserRuleContext {
  public:
    VerbpathContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    PathContext *path();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  VerbpathContext* verbpath();

  class  VerbSimpleContext : public antlr4::ParserRuleContext {
  public:
    VerbSimpleContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    VarContext *var();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  VerbSimpleContext* verbSimple();

  class  ObjectListpathContext : public antlr4::ParserRuleContext {
  public:
    ObjectListpathContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    std::vector<ObjectpathContext *> objectpath();
    ObjectpathContext* objectpath(size_t i);

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  ObjectListpathContext* objectListpath();

  class  ObjectpathContext : public antlr4::ParserRuleContext {
  public:
    ObjectpathContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    GraphNodepathContext *graphNodepath();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  ObjectpathContext* objectpath();

  class  PathContext : public antlr4::ParserRuleContext {
  public:
    PathContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    PathAlternativeContext *pathAlternative();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  PathContext* path();

  class  PathAlternativeContext : public antlr4::ParserRuleContext {
  public:
    PathAlternativeContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    std::vector<PathSequenceContext *> pathSequence();
    PathSequenceContext* pathSequence(size_t i);

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  PathAlternativeContext* pathAlternative();

  class  PathSequenceContext : public antlr4::ParserRuleContext {
  public:
    PathSequenceContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    std::vector<PathEltOrInverseContext *> pathEltOrInverse();
    PathEltOrInverseContext* pathEltOrInverse(size_t i);

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  PathSequenceContext* pathSequence();

  class  PathEltContext : public antlr4::ParserRuleContext {
  public:
    PathEltContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    PathPrimaryContext *pathPrimary();
    PathModContext *pathMod();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  PathEltContext* pathElt();

  class  PathEltOrInverseContext : public antlr4::ParserRuleContext {
  public:
    PathEltOrInverseContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    PathEltContext *pathElt();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  PathEltOrInverseContext* pathEltOrInverse();

  class  PathModContext : public antlr4::ParserRuleContext {
  public:
    PathModContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  PathModContext* pathMod();

  class  PathPrimaryContext : public antlr4::ParserRuleContext {
  public:
    PathPrimaryContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    IriContext *iri();
    PathNegatedPropertySetContext *pathNegatedPropertySet();
    PathContext *path();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  PathPrimaryContext* pathPrimary();

  class  PathNegatedPropertySetContext : public antlr4::ParserRuleContext {
  public:
    PathNegatedPropertySetContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    std::vector<PathOneInPropertySetContext *> pathOneInPropertySet();
    PathOneInPropertySetContext* pathOneInPropertySet(size_t i);

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  PathNegatedPropertySetContext* pathNegatedPropertySet();

  class  PathOneInPropertySetContext : public antlr4::ParserRuleContext {
  public:
    PathOneInPropertySetContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    IriContext *iri();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  PathOneInPropertySetContext* pathOneInPropertySet();

  class  TriplesNodeContext : public antlr4::ParserRuleContext {
  public:
    TriplesNodeContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    CollectionContext *collection();
    BlankNodepropertyListContext *blankNodepropertyList();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  TriplesNodeContext* triplesNode();

  class  BlankNodepropertyListContext : public antlr4::ParserRuleContext {
  public:
    BlankNodepropertyListContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    PropertyListNotEmptyContext *propertyListNotEmpty();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  BlankNodepropertyListContext* blankNodepropertyList();

  class  TriplesNodepathContext : public antlr4::ParserRuleContext {
  public:
    TriplesNodepathContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    CollectionpathContext *collectionpath();
    BlankNodepropertyListpathContext *blankNodepropertyListpath();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  TriplesNodepathContext* triplesNodepath();

  class  BlankNodepropertyListpathContext : public antlr4::ParserRuleContext {
  public:
    BlankNodepropertyListpathContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    PropertyListpathNotEmptyContext *propertyListpathNotEmpty();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  BlankNodepropertyListpathContext* blankNodepropertyListpath();

  class  CollectionContext : public antlr4::ParserRuleContext {
  public:
    CollectionContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    std::vector<GraphNodeContext *> graphNode();
    GraphNodeContext* graphNode(size_t i);

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  CollectionContext* collection();

  class  CollectionpathContext : public antlr4::ParserRuleContext {
  public:
    CollectionpathContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    std::vector<GraphNodepathContext *> graphNodepath();
    GraphNodepathContext* graphNodepath(size_t i);

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  CollectionpathContext* collectionpath();

  class  GraphNodeContext : public antlr4::ParserRuleContext {
  public:
    GraphNodeContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    VarOrTermContext *varOrTerm();
    TriplesNodeContext *triplesNode();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  GraphNodeContext* graphNode();

  class  GraphNodepathContext : public antlr4::ParserRuleContext {
  public:
    GraphNodepathContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    VarOrTermContext *varOrTerm();
    TriplesNodepathContext *triplesNodepath();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  GraphNodepathContext* graphNodepath();

  class  VarOrTermContext : public antlr4::ParserRuleContext {
  public:
    VarOrTermContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    VarContext *var();
    GraphTermContext *graphTerm();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  VarOrTermContext* varOrTerm();

  class  VarOrIriContext : public antlr4::ParserRuleContext {
  public:
    VarOrIriContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    VarContext *var();
    IriContext *iri();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  VarOrIriContext* varOrIri();

  class  VarOrIriSetContext : public antlr4::ParserRuleContext {
  public:
    VarOrIriSetContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    std::vector<VarOrIriContext *> varOrIri();
    VarOrIriContext* varOrIri(size_t i);

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  VarOrIriSetContext* varOrIriSet();

  class  VarContext : public antlr4::ParserRuleContext {
  public:
    VarContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *VAR1();
    antlr4::tree::TerminalNode *VAR2();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  VarContext* var();

  class  GraphTermContext : public antlr4::ParserRuleContext {
  public:
    GraphTermContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    IriContext *iri();
    RDFLiteralContext *rDFLiteral();
    NumericLiteralContext *numericLiteral();
    BooleanLiteralContext *booleanLiteral();
    BlankNodeContext *blankNode();
    antlr4::tree::TerminalNode *NIL();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  GraphTermContext* graphTerm();

  class  ExpressionContext : public antlr4::ParserRuleContext {
  public:
    ExpressionContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    ConditionalOrexpressionContext *conditionalOrexpression();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  ExpressionContext* expression();

  class  ConditionalOrexpressionContext : public antlr4::ParserRuleContext {
  public:
    ConditionalOrexpressionContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    std::vector<ConditionalAndexpressionContext *> conditionalAndexpression();
    ConditionalAndexpressionContext* conditionalAndexpression(size_t i);

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  ConditionalOrexpressionContext* conditionalOrexpression();

  class  ConditionalAndexpressionContext : public antlr4::ParserRuleContext {
  public:
    ConditionalAndexpressionContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    std::vector<ValueLogicalContext *> valueLogical();
    ValueLogicalContext* valueLogical(size_t i);

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  ConditionalAndexpressionContext* conditionalAndexpression();

  class  ValueLogicalContext : public antlr4::ParserRuleContext {
  public:
    ValueLogicalContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    RelationalexpressionContext *relationalexpression();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  ValueLogicalContext* valueLogical();

  class  RelationalexpressionContext : public antlr4::ParserRuleContext {
  public:
    RelationalexpressionContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    std::vector<NumericexpressionContext *> numericexpression();
    NumericexpressionContext* numericexpression(size_t i);
    antlr4::tree::TerminalNode *K_IN();
    ExpressionListContext *expressionList();
    antlr4::tree::TerminalNode *K_NOT();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  RelationalexpressionContext* relationalexpression();

  class  NumericexpressionContext : public antlr4::ParserRuleContext {
  public:
    NumericexpressionContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    AdditiveexpressionContext *additiveexpression();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  NumericexpressionContext* numericexpression();

  class  AdditiveexpressionContext : public antlr4::ParserRuleContext {
  public:
    AdditiveexpressionContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    std::vector<MultiplicativeexpressionContext *> multiplicativeexpression();
    MultiplicativeexpressionContext* multiplicativeexpression(size_t i);
    std::vector<NumericLiteralPositiveContext *> numericLiteralPositive();
    NumericLiteralPositiveContext* numericLiteralPositive(size_t i);
    std::vector<NumericLiteralNegativeContext *> numericLiteralNegative();
    NumericLiteralNegativeContext* numericLiteralNegative(size_t i);
    std::vector<UnaryexpressionContext *> unaryexpression();
    UnaryexpressionContext* unaryexpression(size_t i);

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  AdditiveexpressionContext* additiveexpression();

  class  MultiplicativeexpressionContext : public antlr4::ParserRuleContext {
  public:
    MultiplicativeexpressionContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    std::vector<UnaryexpressionContext *> unaryexpression();
    UnaryexpressionContext* unaryexpression(size_t i);

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  MultiplicativeexpressionContext* multiplicativeexpression();

  class  UnaryexpressionContext : public antlr4::ParserRuleContext {
  public:
    UnaryexpressionContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    PrimaryexpressionContext *primaryexpression();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  UnaryexpressionContext* unaryexpression();

  class  PrimaryexpressionContext : public antlr4::ParserRuleContext {
  public:
    PrimaryexpressionContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    BrackettedexpressionContext *brackettedexpression();
    BuiltInCallContext *builtInCall();
    IriOrFunctionContext *iriOrFunction();
    RDFLiteralContext *rDFLiteral();
    NumericLiteralContext *numericLiteral();
    BooleanLiteralContext *booleanLiteral();
    VarContext *var();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  PrimaryexpressionContext* primaryexpression();

  class  BrackettedexpressionContext : public antlr4::ParserRuleContext {
  public:
    BrackettedexpressionContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    ExpressionContext *expression();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  BrackettedexpressionContext* brackettedexpression();

  class  PredSetContext : public antlr4::ParserRuleContext {
  public:
    PredSetContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    std::vector<IriContext *> iri();
    IriContext* iri(size_t i);

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  PredSetContext* predSet();

  class  BuiltInCallContext : public antlr4::ParserRuleContext {
  public:
    BuiltInCallContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    AggregateContext *aggregate();
    antlr4::tree::TerminalNode *K_STR();
    std::vector<ExpressionContext *> expression();
    ExpressionContext* expression(size_t i);
    antlr4::tree::TerminalNode *K_LANG();
    antlr4::tree::TerminalNode *K_LANGMATCHES();
    antlr4::tree::TerminalNode *K_DATATYPE();
    antlr4::tree::TerminalNode *K_BOUND();
    VarContext *var();
    antlr4::tree::TerminalNode *K_IRI();
    antlr4::tree::TerminalNode *K_URI();
    antlr4::tree::TerminalNode *K_BNODE();
    antlr4::tree::TerminalNode *NIL();
    antlr4::tree::TerminalNode *K_RAND();
    antlr4::tree::TerminalNode *K_ABS();
    antlr4::tree::TerminalNode *K_CEIL();
    antlr4::tree::TerminalNode *K_FLOOR();
    antlr4::tree::TerminalNode *K_ROUND();
    antlr4::tree::TerminalNode *K_CONCAT();
    ExpressionListContext *expressionList();
    SubstringexpressionContext *substringexpression();
    antlr4::tree::TerminalNode *K_STRLEN();
    StrReplaceexpressionContext *strReplaceexpression();
    antlr4::tree::TerminalNode *K_UCASE();
    antlr4::tree::TerminalNode *K_LCASE();
    antlr4::tree::TerminalNode *KK_ENCODE_FOR_URI();
    antlr4::tree::TerminalNode *K_CONTAINS();
    antlr4::tree::TerminalNode *K_STRSTARTS();
    antlr4::tree::TerminalNode *K_STRENDS();
    antlr4::tree::TerminalNode *K_STRBEFORE();
    antlr4::tree::TerminalNode *K_STRAFTER();
    antlr4::tree::TerminalNode *K_YEAR();
    antlr4::tree::TerminalNode *K_MONTH();
    antlr4::tree::TerminalNode *K_DAY();
    antlr4::tree::TerminalNode *K_HOURS();
    antlr4::tree::TerminalNode *K_MINUTES();
    antlr4::tree::TerminalNode *K_SECONDS();
    antlr4::tree::TerminalNode *K_TIMEZONE();
    antlr4::tree::TerminalNode *K_TZ();
    antlr4::tree::TerminalNode *K_NOW();
    antlr4::tree::TerminalNode *K_UUID();
    antlr4::tree::TerminalNode *K_STRUUID();
    antlr4::tree::TerminalNode *KK_MD5();
    antlr4::tree::TerminalNode *KK_SHA1();
    antlr4::tree::TerminalNode *KK_SHA256();
    antlr4::tree::TerminalNode *KK_SHA384();
    antlr4::tree::TerminalNode *KK_SHA512();
    antlr4::tree::TerminalNode *K_COALESCE();
    antlr4::tree::TerminalNode *K_IF();
    antlr4::tree::TerminalNode *K_STRLANG();
    antlr4::tree::TerminalNode *K_STRDT();
    antlr4::tree::TerminalNode *K_SAMETERM();
    antlr4::tree::TerminalNode *K_ISIRI();
    antlr4::tree::TerminalNode *K_ISURI();
    antlr4::tree::TerminalNode *K_ISBLANK();
    antlr4::tree::TerminalNode *K_ISLITERAL();
    antlr4::tree::TerminalNode *K_ISNUMERIC();
    RegexexpressionContext *regexexpression();
    ExistsFuncContext *existsFunc();
    NotexistsFuncContext *notexistsFunc();
    antlr4::tree::TerminalNode *K_SIMPLECYCLEPATH();
    std::vector<VarOrIriContext *> varOrIri();
    VarOrIriContext* varOrIri(size_t i);
    BooleanLiteralContext *booleanLiteral();
    PredSetContext *predSet();
    antlr4::tree::TerminalNode *K_SIMPLECYCLEBOOLEAN();
    antlr4::tree::TerminalNode *K_CYCLEPATH();
    antlr4::tree::TerminalNode *K_CYCLEBOOLEAN();
    antlr4::tree::TerminalNode *K_SHORTESTPATH();
    antlr4::tree::TerminalNode *K_SHORTESTPATHLEN();
    antlr4::tree::TerminalNode *K_KHOPREACHABLE();
    std::vector<Num_integerContext *> num_integer();
    Num_integerContext* num_integer(size_t i);
    Integer_positiveContext *integer_positive();
    Integer_negativeContext *integer_negative();
    antlr4::tree::TerminalNode *K_KHOPENUMERATE();
    antlr4::tree::TerminalNode *K_KHOPREACHABLEPATH();
    antlr4::tree::TerminalNode *K_PPR();
    antlr4::tree::TerminalNode *K_TRIANGLECOUNTING();
    antlr4::tree::TerminalNode *K_CLOSENESSCENTRALITY();
    antlr4::tree::TerminalNode *K_BFSCOUNT();
    antlr4::tree::TerminalNode *K_PR();
    antlr4::tree::TerminalNode *K_ALPHA();
    std::vector<NumericLiteralContext *> numericLiteral();
    NumericLiteralContext* numericLiteral(size_t i);
    antlr4::tree::TerminalNode *K_MAXITER();
    antlr4::tree::TerminalNode *K_TOL();
    antlr4::tree::TerminalNode *K_SSSP();
    antlr4::tree::TerminalNode *K_SSSPLEN();
    antlr4::tree::TerminalNode *K_LABELPROP();
    antlr4::tree::TerminalNode *K_WCC();
    antlr4::tree::TerminalNode *K_CLUSTERCOEFF();
    antlr4::tree::TerminalNode *K_MAXIMUMCLIQUE();
    antlr4::tree::TerminalNode *K_PFN();
    VarOrIriSetContext *varOrIriSet();
    StringContext *string();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  BuiltInCallContext* builtInCall();

  class  RegexexpressionContext : public antlr4::ParserRuleContext {
  public:
    RegexexpressionContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *K_REGEX();
    std::vector<ExpressionContext *> expression();
    ExpressionContext* expression(size_t i);

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  RegexexpressionContext* regexexpression();

  class  SubstringexpressionContext : public antlr4::ParserRuleContext {
  public:
    SubstringexpressionContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *K_SUBSTR();
    std::vector<ExpressionContext *> expression();
    ExpressionContext* expression(size_t i);

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  SubstringexpressionContext* substringexpression();

  class  StrReplaceexpressionContext : public antlr4::ParserRuleContext {
  public:
    StrReplaceexpressionContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *K_REPLACE();
    std::vector<ExpressionContext *> expression();
    ExpressionContext* expression(size_t i);

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  StrReplaceexpressionContext* strReplaceexpression();

  class  ExistsFuncContext : public antlr4::ParserRuleContext {
  public:
    ExistsFuncContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *K_EXISTS();
    GroupGraphPatternContext *groupGraphPattern();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  ExistsFuncContext* existsFunc();

  class  NotexistsFuncContext : public antlr4::ParserRuleContext {
  public:
    NotexistsFuncContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *K_NOT();
    antlr4::tree::TerminalNode *K_EXISTS();
    GroupGraphPatternContext *groupGraphPattern();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  NotexistsFuncContext* notexistsFunc();

  class  AggregateContext : public antlr4::ParserRuleContext {
  public:
    AggregateContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *K_COUNT();
    ExpressionContext *expression();
    antlr4::tree::TerminalNode *K_DISTINCT();
    antlr4::tree::TerminalNode *K_SUM();
    antlr4::tree::TerminalNode *K_MIN();
    antlr4::tree::TerminalNode *K_MAX();
    antlr4::tree::TerminalNode *K_AVG();
    antlr4::tree::TerminalNode *K_SAMPLE();
    antlr4::tree::TerminalNode *KK_GROUP_CONCAT();
    antlr4::tree::TerminalNode *K_SEPARATOR();
    StringContext *string();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  AggregateContext* aggregate();

  class  IriOrFunctionContext : public antlr4::ParserRuleContext {
  public:
    IriOrFunctionContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    IriContext *iri();
    ArgListContext *argList();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  IriOrFunctionContext* iriOrFunction();

  class  RDFLiteralContext : public antlr4::ParserRuleContext {
  public:
    RDFLiteralContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    StringContext *string();
    antlr4::tree::TerminalNode *LANGTAG();
    IriContext *iri();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  RDFLiteralContext* rDFLiteral();

  class  NumericLiteralContext : public antlr4::ParserRuleContext {
  public:
    NumericLiteralContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    NumericLiteralUnsignedContext *numericLiteralUnsigned();
    NumericLiteralPositiveContext *numericLiteralPositive();
    NumericLiteralNegativeContext *numericLiteralNegative();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  NumericLiteralContext* numericLiteral();

  class  NumericLiteralUnsignedContext : public antlr4::ParserRuleContext {
  public:
    NumericLiteralUnsignedContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    Num_integerContext *num_integer();
    Num_decimalContext *num_decimal();
    Num_doubleContext *num_double();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  NumericLiteralUnsignedContext* numericLiteralUnsigned();

  class  NumericLiteralPositiveContext : public antlr4::ParserRuleContext {
  public:
    NumericLiteralPositiveContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    Integer_positiveContext *integer_positive();
    Decimal_positiveContext *decimal_positive();
    Double_positiveContext *double_positive();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  NumericLiteralPositiveContext* numericLiteralPositive();

  class  NumericLiteralNegativeContext : public antlr4::ParserRuleContext {
  public:
    NumericLiteralNegativeContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    Integer_negativeContext *integer_negative();
    Decimal_negativeContext *decimal_negative();
    Double_negativeContext *double_negative();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  NumericLiteralNegativeContext* numericLiteralNegative();

  class  Num_integerContext : public antlr4::ParserRuleContext {
  public:
    Num_integerContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *INTEGER();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  Num_integerContext* num_integer();

  class  Num_decimalContext : public antlr4::ParserRuleContext {
  public:
    Num_decimalContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *DECIMAL();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  Num_decimalContext* num_decimal();

  class  Num_doubleContext : public antlr4::ParserRuleContext {
  public:
    Num_doubleContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *DOUBLE();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  Num_doubleContext* num_double();

  class  Integer_positiveContext : public antlr4::ParserRuleContext {
  public:
    Integer_positiveContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *INTEGER_POSITIVE();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  Integer_positiveContext* integer_positive();

  class  Decimal_positiveContext : public antlr4::ParserRuleContext {
  public:
    Decimal_positiveContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *DECIMAL_POSITIVE();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  Decimal_positiveContext* decimal_positive();

  class  Double_positiveContext : public antlr4::ParserRuleContext {
  public:
    Double_positiveContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *DOUBLE_POSITIVE();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  Double_positiveContext* double_positive();

  class  Integer_negativeContext : public antlr4::ParserRuleContext {
  public:
    Integer_negativeContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *INTEGER_NEGATIVE();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  Integer_negativeContext* integer_negative();

  class  Decimal_negativeContext : public antlr4::ParserRuleContext {
  public:
    Decimal_negativeContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *DECIMAL_NEGATIVE();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  Decimal_negativeContext* decimal_negative();

  class  Double_negativeContext : public antlr4::ParserRuleContext {
  public:
    Double_negativeContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *DOUBLE_NEGATIVE();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  Double_negativeContext* double_negative();

  class  BooleanLiteralContext : public antlr4::ParserRuleContext {
  public:
    BooleanLiteralContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  BooleanLiteralContext* booleanLiteral();

  class  StringContext : public antlr4::ParserRuleContext {
  public:
    StringContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *STRING_LITERAL1();
    antlr4::tree::TerminalNode *STRING_LITERAL2();
    antlr4::tree::TerminalNode *STRING_LITERAL_LONG1();
    antlr4::tree::TerminalNode *STRING_LITERAL_LONG2();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  StringContext* string();

  class  IriContext : public antlr4::ParserRuleContext {
  public:
    IriContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *IRIREF();
    PrefixedNameContext *prefixedName();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  IriContext* iri();

  class  PrefixedNameContext : public antlr4::ParserRuleContext {
  public:
    PrefixedNameContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *PNAME_LN();
    antlr4::tree::TerminalNode *PNAME_NS();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  PrefixedNameContext* prefixedName();

  class  BlankNodeContext : public antlr4::ParserRuleContext {
  public:
    BlankNodeContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *BLANK_NODE_LABEL();
    antlr4::tree::TerminalNode *ANON();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  BlankNodeContext* blankNode();


private:
  static std::vector<antlr4::dfa::DFA> _decisionToDFA;
  static antlr4::atn::PredictionContextCache _sharedContextCache;
  static std::vector<std::string> _ruleNames;
  static std::vector<std::string> _tokenNames;

  static std::vector<std::string> _literalNames;
  static std::vector<std::string> _symbolicNames;
  static antlr4::dfa::Vocabulary _vocabulary;
  static antlr4::atn::ATN _atn;
  static std::vector<uint16_t> _serializedATN;


  struct Initializer {
    Initializer();
  };
  static Initializer _init;
};

