
// Generated from ./SPARQL.g4 by ANTLR 4.7.2

#pragma once


#include "antlr4-runtime.h"
#include "SPARQLParser.h"


/**
 * This interface defines an abstract listener for a parse tree produced by SPARQLParser.
 */
class  SPARQLListener : public antlr4::tree::ParseTreeListener {
public:

  virtual void enterEntry(SPARQLParser::EntryContext *ctx) = 0;
  virtual void exitEntry(SPARQLParser::EntryContext *ctx) = 0;

  virtual void enterQueryUnit(SPARQLParser::QueryUnitContext *ctx) = 0;
  virtual void exitQueryUnit(SPARQLParser::QueryUnitContext *ctx) = 0;

  virtual void enterQuery(SPARQLParser::QueryContext *ctx) = 0;
  virtual void exitQuery(SPARQLParser::QueryContext *ctx) = 0;

  virtual void enterUpdateUnit(SPARQLParser::UpdateUnitContext *ctx) = 0;
  virtual void exitUpdateUnit(SPARQLParser::UpdateUnitContext *ctx) = 0;

  virtual void enterPrologue(SPARQLParser::PrologueContext *ctx) = 0;
  virtual void exitPrologue(SPARQLParser::PrologueContext *ctx) = 0;

  virtual void enterBaseDecl(SPARQLParser::BaseDeclContext *ctx) = 0;
  virtual void exitBaseDecl(SPARQLParser::BaseDeclContext *ctx) = 0;

  virtual void enterPrefixDecl(SPARQLParser::PrefixDeclContext *ctx) = 0;
  virtual void exitPrefixDecl(SPARQLParser::PrefixDeclContext *ctx) = 0;

  virtual void enterSelectquery(SPARQLParser::SelectqueryContext *ctx) = 0;
  virtual void exitSelectquery(SPARQLParser::SelectqueryContext *ctx) = 0;

  virtual void enterSubSelect(SPARQLParser::SubSelectContext *ctx) = 0;
  virtual void exitSubSelect(SPARQLParser::SubSelectContext *ctx) = 0;

  virtual void enterSelectClause(SPARQLParser::SelectClauseContext *ctx) = 0;
  virtual void exitSelectClause(SPARQLParser::SelectClauseContext *ctx) = 0;

  virtual void enterExpressionAsVar(SPARQLParser::ExpressionAsVarContext *ctx) = 0;
  virtual void exitExpressionAsVar(SPARQLParser::ExpressionAsVarContext *ctx) = 0;

  virtual void enterConstructquery(SPARQLParser::ConstructqueryContext *ctx) = 0;
  virtual void exitConstructquery(SPARQLParser::ConstructqueryContext *ctx) = 0;

  virtual void enterDescribequery(SPARQLParser::DescribequeryContext *ctx) = 0;
  virtual void exitDescribequery(SPARQLParser::DescribequeryContext *ctx) = 0;

  virtual void enterAskquery(SPARQLParser::AskqueryContext *ctx) = 0;
  virtual void exitAskquery(SPARQLParser::AskqueryContext *ctx) = 0;

  virtual void enterDatasetClause(SPARQLParser::DatasetClauseContext *ctx) = 0;
  virtual void exitDatasetClause(SPARQLParser::DatasetClauseContext *ctx) = 0;

  virtual void enterDefaultGraphClause(SPARQLParser::DefaultGraphClauseContext *ctx) = 0;
  virtual void exitDefaultGraphClause(SPARQLParser::DefaultGraphClauseContext *ctx) = 0;

  virtual void enterNamedGraphClause(SPARQLParser::NamedGraphClauseContext *ctx) = 0;
  virtual void exitNamedGraphClause(SPARQLParser::NamedGraphClauseContext *ctx) = 0;

  virtual void enterSourceSelector(SPARQLParser::SourceSelectorContext *ctx) = 0;
  virtual void exitSourceSelector(SPARQLParser::SourceSelectorContext *ctx) = 0;

  virtual void enterWhereClause(SPARQLParser::WhereClauseContext *ctx) = 0;
  virtual void exitWhereClause(SPARQLParser::WhereClauseContext *ctx) = 0;

  virtual void enterSolutionModifier(SPARQLParser::SolutionModifierContext *ctx) = 0;
  virtual void exitSolutionModifier(SPARQLParser::SolutionModifierContext *ctx) = 0;

  virtual void enterGroupClause(SPARQLParser::GroupClauseContext *ctx) = 0;
  virtual void exitGroupClause(SPARQLParser::GroupClauseContext *ctx) = 0;

  virtual void enterGroupCondition(SPARQLParser::GroupConditionContext *ctx) = 0;
  virtual void exitGroupCondition(SPARQLParser::GroupConditionContext *ctx) = 0;

  virtual void enterHavingClause(SPARQLParser::HavingClauseContext *ctx) = 0;
  virtual void exitHavingClause(SPARQLParser::HavingClauseContext *ctx) = 0;

  virtual void enterHavingCondition(SPARQLParser::HavingConditionContext *ctx) = 0;
  virtual void exitHavingCondition(SPARQLParser::HavingConditionContext *ctx) = 0;

  virtual void enterOrderClause(SPARQLParser::OrderClauseContext *ctx) = 0;
  virtual void exitOrderClause(SPARQLParser::OrderClauseContext *ctx) = 0;

  virtual void enterOrderCondition(SPARQLParser::OrderConditionContext *ctx) = 0;
  virtual void exitOrderCondition(SPARQLParser::OrderConditionContext *ctx) = 0;

  virtual void enterLimitoffsetClauses(SPARQLParser::LimitoffsetClausesContext *ctx) = 0;
  virtual void exitLimitoffsetClauses(SPARQLParser::LimitoffsetClausesContext *ctx) = 0;

  virtual void enterLimitClause(SPARQLParser::LimitClauseContext *ctx) = 0;
  virtual void exitLimitClause(SPARQLParser::LimitClauseContext *ctx) = 0;

  virtual void enterOffsetClause(SPARQLParser::OffsetClauseContext *ctx) = 0;
  virtual void exitOffsetClause(SPARQLParser::OffsetClauseContext *ctx) = 0;

  virtual void enterValuesClause(SPARQLParser::ValuesClauseContext *ctx) = 0;
  virtual void exitValuesClause(SPARQLParser::ValuesClauseContext *ctx) = 0;

  virtual void enterUpdate(SPARQLParser::UpdateContext *ctx) = 0;
  virtual void exitUpdate(SPARQLParser::UpdateContext *ctx) = 0;

  virtual void enterUpdate1(SPARQLParser::Update1Context *ctx) = 0;
  virtual void exitUpdate1(SPARQLParser::Update1Context *ctx) = 0;

  virtual void enterLoad(SPARQLParser::LoadContext *ctx) = 0;
  virtual void exitLoad(SPARQLParser::LoadContext *ctx) = 0;

  virtual void enterClear(SPARQLParser::ClearContext *ctx) = 0;
  virtual void exitClear(SPARQLParser::ClearContext *ctx) = 0;

  virtual void enterDrop(SPARQLParser::DropContext *ctx) = 0;
  virtual void exitDrop(SPARQLParser::DropContext *ctx) = 0;

  virtual void enterCreate(SPARQLParser::CreateContext *ctx) = 0;
  virtual void exitCreate(SPARQLParser::CreateContext *ctx) = 0;

  virtual void enterAdd(SPARQLParser::AddContext *ctx) = 0;
  virtual void exitAdd(SPARQLParser::AddContext *ctx) = 0;

  virtual void enterMove(SPARQLParser::MoveContext *ctx) = 0;
  virtual void exitMove(SPARQLParser::MoveContext *ctx) = 0;

  virtual void enterCopy(SPARQLParser::CopyContext *ctx) = 0;
  virtual void exitCopy(SPARQLParser::CopyContext *ctx) = 0;

  virtual void enterInsertData(SPARQLParser::InsertDataContext *ctx) = 0;
  virtual void exitInsertData(SPARQLParser::InsertDataContext *ctx) = 0;

  virtual void enterDeleteData(SPARQLParser::DeleteDataContext *ctx) = 0;
  virtual void exitDeleteData(SPARQLParser::DeleteDataContext *ctx) = 0;

  virtual void enterDeleteWhere(SPARQLParser::DeleteWhereContext *ctx) = 0;
  virtual void exitDeleteWhere(SPARQLParser::DeleteWhereContext *ctx) = 0;

  virtual void enterModify(SPARQLParser::ModifyContext *ctx) = 0;
  virtual void exitModify(SPARQLParser::ModifyContext *ctx) = 0;

  virtual void enterDeleteClause(SPARQLParser::DeleteClauseContext *ctx) = 0;
  virtual void exitDeleteClause(SPARQLParser::DeleteClauseContext *ctx) = 0;

  virtual void enterInsertClause(SPARQLParser::InsertClauseContext *ctx) = 0;
  virtual void exitInsertClause(SPARQLParser::InsertClauseContext *ctx) = 0;

  virtual void enterUsingClause(SPARQLParser::UsingClauseContext *ctx) = 0;
  virtual void exitUsingClause(SPARQLParser::UsingClauseContext *ctx) = 0;

  virtual void enterGraphOrDefault(SPARQLParser::GraphOrDefaultContext *ctx) = 0;
  virtual void exitGraphOrDefault(SPARQLParser::GraphOrDefaultContext *ctx) = 0;

  virtual void enterGraphRef(SPARQLParser::GraphRefContext *ctx) = 0;
  virtual void exitGraphRef(SPARQLParser::GraphRefContext *ctx) = 0;

  virtual void enterGraphRefAll(SPARQLParser::GraphRefAllContext *ctx) = 0;
  virtual void exitGraphRefAll(SPARQLParser::GraphRefAllContext *ctx) = 0;

  virtual void enterQuadPattern(SPARQLParser::QuadPatternContext *ctx) = 0;
  virtual void exitQuadPattern(SPARQLParser::QuadPatternContext *ctx) = 0;

  virtual void enterQuadData(SPARQLParser::QuadDataContext *ctx) = 0;
  virtual void exitQuadData(SPARQLParser::QuadDataContext *ctx) = 0;

  virtual void enterQuads(SPARQLParser::QuadsContext *ctx) = 0;
  virtual void exitQuads(SPARQLParser::QuadsContext *ctx) = 0;

  virtual void enterQuadsNotTriples(SPARQLParser::QuadsNotTriplesContext *ctx) = 0;
  virtual void exitQuadsNotTriples(SPARQLParser::QuadsNotTriplesContext *ctx) = 0;

  virtual void enterTriplesTemplate(SPARQLParser::TriplesTemplateContext *ctx) = 0;
  virtual void exitTriplesTemplate(SPARQLParser::TriplesTemplateContext *ctx) = 0;

  virtual void enterGroupGraphPattern(SPARQLParser::GroupGraphPatternContext *ctx) = 0;
  virtual void exitGroupGraphPattern(SPARQLParser::GroupGraphPatternContext *ctx) = 0;

  virtual void enterGroupGraphPatternSub(SPARQLParser::GroupGraphPatternSubContext *ctx) = 0;
  virtual void exitGroupGraphPatternSub(SPARQLParser::GroupGraphPatternSubContext *ctx) = 0;

  virtual void enterGraphPatternTriplesBlock(SPARQLParser::GraphPatternTriplesBlockContext *ctx) = 0;
  virtual void exitGraphPatternTriplesBlock(SPARQLParser::GraphPatternTriplesBlockContext *ctx) = 0;

  virtual void enterTriplesBlock(SPARQLParser::TriplesBlockContext *ctx) = 0;
  virtual void exitTriplesBlock(SPARQLParser::TriplesBlockContext *ctx) = 0;

  virtual void enterGraphPatternNotTriples(SPARQLParser::GraphPatternNotTriplesContext *ctx) = 0;
  virtual void exitGraphPatternNotTriples(SPARQLParser::GraphPatternNotTriplesContext *ctx) = 0;

  virtual void enterOptionalGraphPattern(SPARQLParser::OptionalGraphPatternContext *ctx) = 0;
  virtual void exitOptionalGraphPattern(SPARQLParser::OptionalGraphPatternContext *ctx) = 0;

  virtual void enterGraphGraphPattern(SPARQLParser::GraphGraphPatternContext *ctx) = 0;
  virtual void exitGraphGraphPattern(SPARQLParser::GraphGraphPatternContext *ctx) = 0;

  virtual void enterServiceGraphPattern(SPARQLParser::ServiceGraphPatternContext *ctx) = 0;
  virtual void exitServiceGraphPattern(SPARQLParser::ServiceGraphPatternContext *ctx) = 0;

  virtual void enterBind(SPARQLParser::BindContext *ctx) = 0;
  virtual void exitBind(SPARQLParser::BindContext *ctx) = 0;

  virtual void enterInlineData(SPARQLParser::InlineDataContext *ctx) = 0;
  virtual void exitInlineData(SPARQLParser::InlineDataContext *ctx) = 0;

  virtual void enterDataBlock(SPARQLParser::DataBlockContext *ctx) = 0;
  virtual void exitDataBlock(SPARQLParser::DataBlockContext *ctx) = 0;

  virtual void enterInlineDataOnevar(SPARQLParser::InlineDataOnevarContext *ctx) = 0;
  virtual void exitInlineDataOnevar(SPARQLParser::InlineDataOnevarContext *ctx) = 0;

  virtual void enterInlineDataFull(SPARQLParser::InlineDataFullContext *ctx) = 0;
  virtual void exitInlineDataFull(SPARQLParser::InlineDataFullContext *ctx) = 0;

  virtual void enterDataBlockValue(SPARQLParser::DataBlockValueContext *ctx) = 0;
  virtual void exitDataBlockValue(SPARQLParser::DataBlockValueContext *ctx) = 0;

  virtual void enterMinusGraphPattern(SPARQLParser::MinusGraphPatternContext *ctx) = 0;
  virtual void exitMinusGraphPattern(SPARQLParser::MinusGraphPatternContext *ctx) = 0;

  virtual void enterGroupOrUnionGraphPattern(SPARQLParser::GroupOrUnionGraphPatternContext *ctx) = 0;
  virtual void exitGroupOrUnionGraphPattern(SPARQLParser::GroupOrUnionGraphPatternContext *ctx) = 0;

  virtual void enterFilter(SPARQLParser::FilterContext *ctx) = 0;
  virtual void exitFilter(SPARQLParser::FilterContext *ctx) = 0;

  virtual void enterConstraint(SPARQLParser::ConstraintContext *ctx) = 0;
  virtual void exitConstraint(SPARQLParser::ConstraintContext *ctx) = 0;

  virtual void enterFunctionCall(SPARQLParser::FunctionCallContext *ctx) = 0;
  virtual void exitFunctionCall(SPARQLParser::FunctionCallContext *ctx) = 0;

  virtual void enterArgList(SPARQLParser::ArgListContext *ctx) = 0;
  virtual void exitArgList(SPARQLParser::ArgListContext *ctx) = 0;

  virtual void enterExpressionList(SPARQLParser::ExpressionListContext *ctx) = 0;
  virtual void exitExpressionList(SPARQLParser::ExpressionListContext *ctx) = 0;

  virtual void enterConstructTemplate(SPARQLParser::ConstructTemplateContext *ctx) = 0;
  virtual void exitConstructTemplate(SPARQLParser::ConstructTemplateContext *ctx) = 0;

  virtual void enterConstructTriples(SPARQLParser::ConstructTriplesContext *ctx) = 0;
  virtual void exitConstructTriples(SPARQLParser::ConstructTriplesContext *ctx) = 0;

  virtual void enterTriplesSameSubject(SPARQLParser::TriplesSameSubjectContext *ctx) = 0;
  virtual void exitTriplesSameSubject(SPARQLParser::TriplesSameSubjectContext *ctx) = 0;

  virtual void enterPropertyList(SPARQLParser::PropertyListContext *ctx) = 0;
  virtual void exitPropertyList(SPARQLParser::PropertyListContext *ctx) = 0;

  virtual void enterPropertyListNotEmpty(SPARQLParser::PropertyListNotEmptyContext *ctx) = 0;
  virtual void exitPropertyListNotEmpty(SPARQLParser::PropertyListNotEmptyContext *ctx) = 0;

  virtual void enterVerb(SPARQLParser::VerbContext *ctx) = 0;
  virtual void exitVerb(SPARQLParser::VerbContext *ctx) = 0;

  virtual void enterObjectList(SPARQLParser::ObjectListContext *ctx) = 0;
  virtual void exitObjectList(SPARQLParser::ObjectListContext *ctx) = 0;

  virtual void enterObject(SPARQLParser::ObjectContext *ctx) = 0;
  virtual void exitObject(SPARQLParser::ObjectContext *ctx) = 0;

  virtual void enterTriplesSameSubjectpath(SPARQLParser::TriplesSameSubjectpathContext *ctx) = 0;
  virtual void exitTriplesSameSubjectpath(SPARQLParser::TriplesSameSubjectpathContext *ctx) = 0;

  virtual void enterPropertyListpath(SPARQLParser::PropertyListpathContext *ctx) = 0;
  virtual void exitPropertyListpath(SPARQLParser::PropertyListpathContext *ctx) = 0;

  virtual void enterPropertyListpathNotEmpty(SPARQLParser::PropertyListpathNotEmptyContext *ctx) = 0;
  virtual void exitPropertyListpathNotEmpty(SPARQLParser::PropertyListpathNotEmptyContext *ctx) = 0;

  virtual void enterVerbpathOrSimple(SPARQLParser::VerbpathOrSimpleContext *ctx) = 0;
  virtual void exitVerbpathOrSimple(SPARQLParser::VerbpathOrSimpleContext *ctx) = 0;

  virtual void enterVerbpath(SPARQLParser::VerbpathContext *ctx) = 0;
  virtual void exitVerbpath(SPARQLParser::VerbpathContext *ctx) = 0;

  virtual void enterVerbSimple(SPARQLParser::VerbSimpleContext *ctx) = 0;
  virtual void exitVerbSimple(SPARQLParser::VerbSimpleContext *ctx) = 0;

  virtual void enterObjectListpath(SPARQLParser::ObjectListpathContext *ctx) = 0;
  virtual void exitObjectListpath(SPARQLParser::ObjectListpathContext *ctx) = 0;

  virtual void enterObjectpath(SPARQLParser::ObjectpathContext *ctx) = 0;
  virtual void exitObjectpath(SPARQLParser::ObjectpathContext *ctx) = 0;

  virtual void enterPath(SPARQLParser::PathContext *ctx) = 0;
  virtual void exitPath(SPARQLParser::PathContext *ctx) = 0;

  virtual void enterPathAlternative(SPARQLParser::PathAlternativeContext *ctx) = 0;
  virtual void exitPathAlternative(SPARQLParser::PathAlternativeContext *ctx) = 0;

  virtual void enterPathSequence(SPARQLParser::PathSequenceContext *ctx) = 0;
  virtual void exitPathSequence(SPARQLParser::PathSequenceContext *ctx) = 0;

  virtual void enterPathElt(SPARQLParser::PathEltContext *ctx) = 0;
  virtual void exitPathElt(SPARQLParser::PathEltContext *ctx) = 0;

  virtual void enterPathEltOrInverse(SPARQLParser::PathEltOrInverseContext *ctx) = 0;
  virtual void exitPathEltOrInverse(SPARQLParser::PathEltOrInverseContext *ctx) = 0;

  virtual void enterPathMod(SPARQLParser::PathModContext *ctx) = 0;
  virtual void exitPathMod(SPARQLParser::PathModContext *ctx) = 0;

  virtual void enterPathPrimary(SPARQLParser::PathPrimaryContext *ctx) = 0;
  virtual void exitPathPrimary(SPARQLParser::PathPrimaryContext *ctx) = 0;

  virtual void enterPathNegatedPropertySet(SPARQLParser::PathNegatedPropertySetContext *ctx) = 0;
  virtual void exitPathNegatedPropertySet(SPARQLParser::PathNegatedPropertySetContext *ctx) = 0;

  virtual void enterPathOneInPropertySet(SPARQLParser::PathOneInPropertySetContext *ctx) = 0;
  virtual void exitPathOneInPropertySet(SPARQLParser::PathOneInPropertySetContext *ctx) = 0;

  virtual void enterTriplesNode(SPARQLParser::TriplesNodeContext *ctx) = 0;
  virtual void exitTriplesNode(SPARQLParser::TriplesNodeContext *ctx) = 0;

  virtual void enterBlankNodepropertyList(SPARQLParser::BlankNodepropertyListContext *ctx) = 0;
  virtual void exitBlankNodepropertyList(SPARQLParser::BlankNodepropertyListContext *ctx) = 0;

  virtual void enterTriplesNodepath(SPARQLParser::TriplesNodepathContext *ctx) = 0;
  virtual void exitTriplesNodepath(SPARQLParser::TriplesNodepathContext *ctx) = 0;

  virtual void enterBlankNodepropertyListpath(SPARQLParser::BlankNodepropertyListpathContext *ctx) = 0;
  virtual void exitBlankNodepropertyListpath(SPARQLParser::BlankNodepropertyListpathContext *ctx) = 0;

  virtual void enterCollection(SPARQLParser::CollectionContext *ctx) = 0;
  virtual void exitCollection(SPARQLParser::CollectionContext *ctx) = 0;

  virtual void enterCollectionpath(SPARQLParser::CollectionpathContext *ctx) = 0;
  virtual void exitCollectionpath(SPARQLParser::CollectionpathContext *ctx) = 0;

  virtual void enterGraphNode(SPARQLParser::GraphNodeContext *ctx) = 0;
  virtual void exitGraphNode(SPARQLParser::GraphNodeContext *ctx) = 0;

  virtual void enterGraphNodepath(SPARQLParser::GraphNodepathContext *ctx) = 0;
  virtual void exitGraphNodepath(SPARQLParser::GraphNodepathContext *ctx) = 0;

  virtual void enterVarOrTerm(SPARQLParser::VarOrTermContext *ctx) = 0;
  virtual void exitVarOrTerm(SPARQLParser::VarOrTermContext *ctx) = 0;

  virtual void enterVarOrIri(SPARQLParser::VarOrIriContext *ctx) = 0;
  virtual void exitVarOrIri(SPARQLParser::VarOrIriContext *ctx) = 0;

  virtual void enterVarOrIriSet(SPARQLParser::VarOrIriSetContext *ctx) = 0;
  virtual void exitVarOrIriSet(SPARQLParser::VarOrIriSetContext *ctx) = 0;

  virtual void enterVar(SPARQLParser::VarContext *ctx) = 0;
  virtual void exitVar(SPARQLParser::VarContext *ctx) = 0;

  virtual void enterGraphTerm(SPARQLParser::GraphTermContext *ctx) = 0;
  virtual void exitGraphTerm(SPARQLParser::GraphTermContext *ctx) = 0;

  virtual void enterExpression(SPARQLParser::ExpressionContext *ctx) = 0;
  virtual void exitExpression(SPARQLParser::ExpressionContext *ctx) = 0;

  virtual void enterConditionalOrexpression(SPARQLParser::ConditionalOrexpressionContext *ctx) = 0;
  virtual void exitConditionalOrexpression(SPARQLParser::ConditionalOrexpressionContext *ctx) = 0;

  virtual void enterConditionalAndexpression(SPARQLParser::ConditionalAndexpressionContext *ctx) = 0;
  virtual void exitConditionalAndexpression(SPARQLParser::ConditionalAndexpressionContext *ctx) = 0;

  virtual void enterValueLogical(SPARQLParser::ValueLogicalContext *ctx) = 0;
  virtual void exitValueLogical(SPARQLParser::ValueLogicalContext *ctx) = 0;

  virtual void enterRelationalexpression(SPARQLParser::RelationalexpressionContext *ctx) = 0;
  virtual void exitRelationalexpression(SPARQLParser::RelationalexpressionContext *ctx) = 0;

  virtual void enterNumericexpression(SPARQLParser::NumericexpressionContext *ctx) = 0;
  virtual void exitNumericexpression(SPARQLParser::NumericexpressionContext *ctx) = 0;

  virtual void enterAdditiveexpression(SPARQLParser::AdditiveexpressionContext *ctx) = 0;
  virtual void exitAdditiveexpression(SPARQLParser::AdditiveexpressionContext *ctx) = 0;

  virtual void enterMultiplicativeexpression(SPARQLParser::MultiplicativeexpressionContext *ctx) = 0;
  virtual void exitMultiplicativeexpression(SPARQLParser::MultiplicativeexpressionContext *ctx) = 0;

  virtual void enterUnaryexpression(SPARQLParser::UnaryexpressionContext *ctx) = 0;
  virtual void exitUnaryexpression(SPARQLParser::UnaryexpressionContext *ctx) = 0;

  virtual void enterPrimaryexpression(SPARQLParser::PrimaryexpressionContext *ctx) = 0;
  virtual void exitPrimaryexpression(SPARQLParser::PrimaryexpressionContext *ctx) = 0;

  virtual void enterBrackettedexpression(SPARQLParser::BrackettedexpressionContext *ctx) = 0;
  virtual void exitBrackettedexpression(SPARQLParser::BrackettedexpressionContext *ctx) = 0;

  virtual void enterPredSet(SPARQLParser::PredSetContext *ctx) = 0;
  virtual void exitPredSet(SPARQLParser::PredSetContext *ctx) = 0;

  virtual void enterBuiltInCall(SPARQLParser::BuiltInCallContext *ctx) = 0;
  virtual void exitBuiltInCall(SPARQLParser::BuiltInCallContext *ctx) = 0;

  virtual void enterRegexexpression(SPARQLParser::RegexexpressionContext *ctx) = 0;
  virtual void exitRegexexpression(SPARQLParser::RegexexpressionContext *ctx) = 0;

  virtual void enterSubstringexpression(SPARQLParser::SubstringexpressionContext *ctx) = 0;
  virtual void exitSubstringexpression(SPARQLParser::SubstringexpressionContext *ctx) = 0;

  virtual void enterStrReplaceexpression(SPARQLParser::StrReplaceexpressionContext *ctx) = 0;
  virtual void exitStrReplaceexpression(SPARQLParser::StrReplaceexpressionContext *ctx) = 0;

  virtual void enterExistsFunc(SPARQLParser::ExistsFuncContext *ctx) = 0;
  virtual void exitExistsFunc(SPARQLParser::ExistsFuncContext *ctx) = 0;

  virtual void enterNotexistsFunc(SPARQLParser::NotexistsFuncContext *ctx) = 0;
  virtual void exitNotexistsFunc(SPARQLParser::NotexistsFuncContext *ctx) = 0;

  virtual void enterAggregate(SPARQLParser::AggregateContext *ctx) = 0;
  virtual void exitAggregate(SPARQLParser::AggregateContext *ctx) = 0;

  virtual void enterIriOrFunction(SPARQLParser::IriOrFunctionContext *ctx) = 0;
  virtual void exitIriOrFunction(SPARQLParser::IriOrFunctionContext *ctx) = 0;

  virtual void enterRDFLiteral(SPARQLParser::RDFLiteralContext *ctx) = 0;
  virtual void exitRDFLiteral(SPARQLParser::RDFLiteralContext *ctx) = 0;

  virtual void enterNumericLiteral(SPARQLParser::NumericLiteralContext *ctx) = 0;
  virtual void exitNumericLiteral(SPARQLParser::NumericLiteralContext *ctx) = 0;

  virtual void enterNumericLiteralUnsigned(SPARQLParser::NumericLiteralUnsignedContext *ctx) = 0;
  virtual void exitNumericLiteralUnsigned(SPARQLParser::NumericLiteralUnsignedContext *ctx) = 0;

  virtual void enterNumericLiteralPositive(SPARQLParser::NumericLiteralPositiveContext *ctx) = 0;
  virtual void exitNumericLiteralPositive(SPARQLParser::NumericLiteralPositiveContext *ctx) = 0;

  virtual void enterNumericLiteralNegative(SPARQLParser::NumericLiteralNegativeContext *ctx) = 0;
  virtual void exitNumericLiteralNegative(SPARQLParser::NumericLiteralNegativeContext *ctx) = 0;

  virtual void enterNum_integer(SPARQLParser::Num_integerContext *ctx) = 0;
  virtual void exitNum_integer(SPARQLParser::Num_integerContext *ctx) = 0;

  virtual void enterNum_decimal(SPARQLParser::Num_decimalContext *ctx) = 0;
  virtual void exitNum_decimal(SPARQLParser::Num_decimalContext *ctx) = 0;

  virtual void enterNum_double(SPARQLParser::Num_doubleContext *ctx) = 0;
  virtual void exitNum_double(SPARQLParser::Num_doubleContext *ctx) = 0;

  virtual void enterInteger_positive(SPARQLParser::Integer_positiveContext *ctx) = 0;
  virtual void exitInteger_positive(SPARQLParser::Integer_positiveContext *ctx) = 0;

  virtual void enterDecimal_positive(SPARQLParser::Decimal_positiveContext *ctx) = 0;
  virtual void exitDecimal_positive(SPARQLParser::Decimal_positiveContext *ctx) = 0;

  virtual void enterDouble_positive(SPARQLParser::Double_positiveContext *ctx) = 0;
  virtual void exitDouble_positive(SPARQLParser::Double_positiveContext *ctx) = 0;

  virtual void enterInteger_negative(SPARQLParser::Integer_negativeContext *ctx) = 0;
  virtual void exitInteger_negative(SPARQLParser::Integer_negativeContext *ctx) = 0;

  virtual void enterDecimal_negative(SPARQLParser::Decimal_negativeContext *ctx) = 0;
  virtual void exitDecimal_negative(SPARQLParser::Decimal_negativeContext *ctx) = 0;

  virtual void enterDouble_negative(SPARQLParser::Double_negativeContext *ctx) = 0;
  virtual void exitDouble_negative(SPARQLParser::Double_negativeContext *ctx) = 0;

  virtual void enterBooleanLiteral(SPARQLParser::BooleanLiteralContext *ctx) = 0;
  virtual void exitBooleanLiteral(SPARQLParser::BooleanLiteralContext *ctx) = 0;

  virtual void enterString(SPARQLParser::StringContext *ctx) = 0;
  virtual void exitString(SPARQLParser::StringContext *ctx) = 0;

  virtual void enterIri(SPARQLParser::IriContext *ctx) = 0;
  virtual void exitIri(SPARQLParser::IriContext *ctx) = 0;

  virtual void enterPrefixedName(SPARQLParser::PrefixedNameContext *ctx) = 0;
  virtual void exitPrefixedName(SPARQLParser::PrefixedNameContext *ctx) = 0;

  virtual void enterBlankNode(SPARQLParser::BlankNodeContext *ctx) = 0;
  virtual void exitBlankNode(SPARQLParser::BlankNodeContext *ctx) = 0;


};

