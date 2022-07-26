
// Generated from ./SPARQL.g4 by ANTLR 4.7.2

#pragma once


#include "antlr4-runtime.h"
#include "SPARQLListener.h"


/**
 * This class provides an empty implementation of SPARQLListener,
 * which can be extended to create a listener which only needs to handle a subset
 * of the available methods.
 */
class  SPARQLBaseListener : public SPARQLListener {
public:

  virtual void enterEntry(SPARQLParser::EntryContext * /*ctx*/) override { }
  virtual void exitEntry(SPARQLParser::EntryContext * /*ctx*/) override { }

  virtual void enterQueryUnit(SPARQLParser::QueryUnitContext * /*ctx*/) override { }
  virtual void exitQueryUnit(SPARQLParser::QueryUnitContext * /*ctx*/) override { }

  virtual void enterQuery(SPARQLParser::QueryContext * /*ctx*/) override { }
  virtual void exitQuery(SPARQLParser::QueryContext * /*ctx*/) override { }

  virtual void enterUpdateUnit(SPARQLParser::UpdateUnitContext * /*ctx*/) override { }
  virtual void exitUpdateUnit(SPARQLParser::UpdateUnitContext * /*ctx*/) override { }

  virtual void enterPrologue(SPARQLParser::PrologueContext * /*ctx*/) override { }
  virtual void exitPrologue(SPARQLParser::PrologueContext * /*ctx*/) override { }

  virtual void enterBaseDecl(SPARQLParser::BaseDeclContext * /*ctx*/) override { }
  virtual void exitBaseDecl(SPARQLParser::BaseDeclContext * /*ctx*/) override { }

  virtual void enterPrefixDecl(SPARQLParser::PrefixDeclContext * /*ctx*/) override { }
  virtual void exitPrefixDecl(SPARQLParser::PrefixDeclContext * /*ctx*/) override { }

  virtual void enterSelectquery(SPARQLParser::SelectqueryContext * /*ctx*/) override { }
  virtual void exitSelectquery(SPARQLParser::SelectqueryContext * /*ctx*/) override { }

  virtual void enterSubSelect(SPARQLParser::SubSelectContext * /*ctx*/) override { }
  virtual void exitSubSelect(SPARQLParser::SubSelectContext * /*ctx*/) override { }

  virtual void enterSelectClause(SPARQLParser::SelectClauseContext * /*ctx*/) override { }
  virtual void exitSelectClause(SPARQLParser::SelectClauseContext * /*ctx*/) override { }

  virtual void enterExpressionAsVar(SPARQLParser::ExpressionAsVarContext * /*ctx*/) override { }
  virtual void exitExpressionAsVar(SPARQLParser::ExpressionAsVarContext * /*ctx*/) override { }

  virtual void enterConstructquery(SPARQLParser::ConstructqueryContext * /*ctx*/) override { }
  virtual void exitConstructquery(SPARQLParser::ConstructqueryContext * /*ctx*/) override { }

  virtual void enterDescribequery(SPARQLParser::DescribequeryContext * /*ctx*/) override { }
  virtual void exitDescribequery(SPARQLParser::DescribequeryContext * /*ctx*/) override { }

  virtual void enterAskquery(SPARQLParser::AskqueryContext * /*ctx*/) override { }
  virtual void exitAskquery(SPARQLParser::AskqueryContext * /*ctx*/) override { }

  virtual void enterDatasetClause(SPARQLParser::DatasetClauseContext * /*ctx*/) override { }
  virtual void exitDatasetClause(SPARQLParser::DatasetClauseContext * /*ctx*/) override { }

  virtual void enterDefaultGraphClause(SPARQLParser::DefaultGraphClauseContext * /*ctx*/) override { }
  virtual void exitDefaultGraphClause(SPARQLParser::DefaultGraphClauseContext * /*ctx*/) override { }

  virtual void enterNamedGraphClause(SPARQLParser::NamedGraphClauseContext * /*ctx*/) override { }
  virtual void exitNamedGraphClause(SPARQLParser::NamedGraphClauseContext * /*ctx*/) override { }

  virtual void enterSourceSelector(SPARQLParser::SourceSelectorContext * /*ctx*/) override { }
  virtual void exitSourceSelector(SPARQLParser::SourceSelectorContext * /*ctx*/) override { }

  virtual void enterWhereClause(SPARQLParser::WhereClauseContext * /*ctx*/) override { }
  virtual void exitWhereClause(SPARQLParser::WhereClauseContext * /*ctx*/) override { }

  virtual void enterSolutionModifier(SPARQLParser::SolutionModifierContext * /*ctx*/) override { }
  virtual void exitSolutionModifier(SPARQLParser::SolutionModifierContext * /*ctx*/) override { }

  virtual void enterGroupClause(SPARQLParser::GroupClauseContext * /*ctx*/) override { }
  virtual void exitGroupClause(SPARQLParser::GroupClauseContext * /*ctx*/) override { }

  virtual void enterGroupCondition(SPARQLParser::GroupConditionContext * /*ctx*/) override { }
  virtual void exitGroupCondition(SPARQLParser::GroupConditionContext * /*ctx*/) override { }

  virtual void enterHavingClause(SPARQLParser::HavingClauseContext * /*ctx*/) override { }
  virtual void exitHavingClause(SPARQLParser::HavingClauseContext * /*ctx*/) override { }

  virtual void enterHavingCondition(SPARQLParser::HavingConditionContext * /*ctx*/) override { }
  virtual void exitHavingCondition(SPARQLParser::HavingConditionContext * /*ctx*/) override { }

  virtual void enterOrderClause(SPARQLParser::OrderClauseContext * /*ctx*/) override { }
  virtual void exitOrderClause(SPARQLParser::OrderClauseContext * /*ctx*/) override { }

  virtual void enterOrderCondition(SPARQLParser::OrderConditionContext * /*ctx*/) override { }
  virtual void exitOrderCondition(SPARQLParser::OrderConditionContext * /*ctx*/) override { }

  virtual void enterLimitoffsetClauses(SPARQLParser::LimitoffsetClausesContext * /*ctx*/) override { }
  virtual void exitLimitoffsetClauses(SPARQLParser::LimitoffsetClausesContext * /*ctx*/) override { }

  virtual void enterLimitClause(SPARQLParser::LimitClauseContext * /*ctx*/) override { }
  virtual void exitLimitClause(SPARQLParser::LimitClauseContext * /*ctx*/) override { }

  virtual void enterOffsetClause(SPARQLParser::OffsetClauseContext * /*ctx*/) override { }
  virtual void exitOffsetClause(SPARQLParser::OffsetClauseContext * /*ctx*/) override { }

  virtual void enterValuesClause(SPARQLParser::ValuesClauseContext * /*ctx*/) override { }
  virtual void exitValuesClause(SPARQLParser::ValuesClauseContext * /*ctx*/) override { }

  virtual void enterUpdate(SPARQLParser::UpdateContext * /*ctx*/) override { }
  virtual void exitUpdate(SPARQLParser::UpdateContext * /*ctx*/) override { }

  virtual void enterUpdate1(SPARQLParser::Update1Context * /*ctx*/) override { }
  virtual void exitUpdate1(SPARQLParser::Update1Context * /*ctx*/) override { }

  virtual void enterLoad(SPARQLParser::LoadContext * /*ctx*/) override { }
  virtual void exitLoad(SPARQLParser::LoadContext * /*ctx*/) override { }

  virtual void enterClear(SPARQLParser::ClearContext * /*ctx*/) override { }
  virtual void exitClear(SPARQLParser::ClearContext * /*ctx*/) override { }

  virtual void enterDrop(SPARQLParser::DropContext * /*ctx*/) override { }
  virtual void exitDrop(SPARQLParser::DropContext * /*ctx*/) override { }

  virtual void enterCreate(SPARQLParser::CreateContext * /*ctx*/) override { }
  virtual void exitCreate(SPARQLParser::CreateContext * /*ctx*/) override { }

  virtual void enterAdd(SPARQLParser::AddContext * /*ctx*/) override { }
  virtual void exitAdd(SPARQLParser::AddContext * /*ctx*/) override { }

  virtual void enterMove(SPARQLParser::MoveContext * /*ctx*/) override { }
  virtual void exitMove(SPARQLParser::MoveContext * /*ctx*/) override { }

  virtual void enterCopy(SPARQLParser::CopyContext * /*ctx*/) override { }
  virtual void exitCopy(SPARQLParser::CopyContext * /*ctx*/) override { }

  virtual void enterInsertData(SPARQLParser::InsertDataContext * /*ctx*/) override { }
  virtual void exitInsertData(SPARQLParser::InsertDataContext * /*ctx*/) override { }

  virtual void enterDeleteData(SPARQLParser::DeleteDataContext * /*ctx*/) override { }
  virtual void exitDeleteData(SPARQLParser::DeleteDataContext * /*ctx*/) override { }

  virtual void enterDeleteWhere(SPARQLParser::DeleteWhereContext * /*ctx*/) override { }
  virtual void exitDeleteWhere(SPARQLParser::DeleteWhereContext * /*ctx*/) override { }

  virtual void enterModify(SPARQLParser::ModifyContext * /*ctx*/) override { }
  virtual void exitModify(SPARQLParser::ModifyContext * /*ctx*/) override { }

  virtual void enterDeleteClause(SPARQLParser::DeleteClauseContext * /*ctx*/) override { }
  virtual void exitDeleteClause(SPARQLParser::DeleteClauseContext * /*ctx*/) override { }

  virtual void enterInsertClause(SPARQLParser::InsertClauseContext * /*ctx*/) override { }
  virtual void exitInsertClause(SPARQLParser::InsertClauseContext * /*ctx*/) override { }

  virtual void enterUsingClause(SPARQLParser::UsingClauseContext * /*ctx*/) override { }
  virtual void exitUsingClause(SPARQLParser::UsingClauseContext * /*ctx*/) override { }

  virtual void enterGraphOrDefault(SPARQLParser::GraphOrDefaultContext * /*ctx*/) override { }
  virtual void exitGraphOrDefault(SPARQLParser::GraphOrDefaultContext * /*ctx*/) override { }

  virtual void enterGraphRef(SPARQLParser::GraphRefContext * /*ctx*/) override { }
  virtual void exitGraphRef(SPARQLParser::GraphRefContext * /*ctx*/) override { }

  virtual void enterGraphRefAll(SPARQLParser::GraphRefAllContext * /*ctx*/) override { }
  virtual void exitGraphRefAll(SPARQLParser::GraphRefAllContext * /*ctx*/) override { }

  virtual void enterQuadPattern(SPARQLParser::QuadPatternContext * /*ctx*/) override { }
  virtual void exitQuadPattern(SPARQLParser::QuadPatternContext * /*ctx*/) override { }

  virtual void enterQuadData(SPARQLParser::QuadDataContext * /*ctx*/) override { }
  virtual void exitQuadData(SPARQLParser::QuadDataContext * /*ctx*/) override { }

  virtual void enterQuads(SPARQLParser::QuadsContext * /*ctx*/) override { }
  virtual void exitQuads(SPARQLParser::QuadsContext * /*ctx*/) override { }

  virtual void enterQuadsNotTriples(SPARQLParser::QuadsNotTriplesContext * /*ctx*/) override { }
  virtual void exitQuadsNotTriples(SPARQLParser::QuadsNotTriplesContext * /*ctx*/) override { }

  virtual void enterTriplesTemplate(SPARQLParser::TriplesTemplateContext * /*ctx*/) override { }
  virtual void exitTriplesTemplate(SPARQLParser::TriplesTemplateContext * /*ctx*/) override { }

  virtual void enterGroupGraphPattern(SPARQLParser::GroupGraphPatternContext * /*ctx*/) override { }
  virtual void exitGroupGraphPattern(SPARQLParser::GroupGraphPatternContext * /*ctx*/) override { }

  virtual void enterGroupGraphPatternSub(SPARQLParser::GroupGraphPatternSubContext * /*ctx*/) override { }
  virtual void exitGroupGraphPatternSub(SPARQLParser::GroupGraphPatternSubContext * /*ctx*/) override { }

  virtual void enterGraphPatternTriplesBlock(SPARQLParser::GraphPatternTriplesBlockContext * /*ctx*/) override { }
  virtual void exitGraphPatternTriplesBlock(SPARQLParser::GraphPatternTriplesBlockContext * /*ctx*/) override { }

  virtual void enterTriplesBlock(SPARQLParser::TriplesBlockContext * /*ctx*/) override { }
  virtual void exitTriplesBlock(SPARQLParser::TriplesBlockContext * /*ctx*/) override { }

  virtual void enterGraphPatternNotTriples(SPARQLParser::GraphPatternNotTriplesContext * /*ctx*/) override { }
  virtual void exitGraphPatternNotTriples(SPARQLParser::GraphPatternNotTriplesContext * /*ctx*/) override { }

  virtual void enterOptionalGraphPattern(SPARQLParser::OptionalGraphPatternContext * /*ctx*/) override { }
  virtual void exitOptionalGraphPattern(SPARQLParser::OptionalGraphPatternContext * /*ctx*/) override { }

  virtual void enterGraphGraphPattern(SPARQLParser::GraphGraphPatternContext * /*ctx*/) override { }
  virtual void exitGraphGraphPattern(SPARQLParser::GraphGraphPatternContext * /*ctx*/) override { }

  virtual void enterServiceGraphPattern(SPARQLParser::ServiceGraphPatternContext * /*ctx*/) override { }
  virtual void exitServiceGraphPattern(SPARQLParser::ServiceGraphPatternContext * /*ctx*/) override { }

  virtual void enterBind(SPARQLParser::BindContext * /*ctx*/) override { }
  virtual void exitBind(SPARQLParser::BindContext * /*ctx*/) override { }

  virtual void enterInlineData(SPARQLParser::InlineDataContext * /*ctx*/) override { }
  virtual void exitInlineData(SPARQLParser::InlineDataContext * /*ctx*/) override { }

  virtual void enterDataBlock(SPARQLParser::DataBlockContext * /*ctx*/) override { }
  virtual void exitDataBlock(SPARQLParser::DataBlockContext * /*ctx*/) override { }

  virtual void enterInlineDataOnevar(SPARQLParser::InlineDataOnevarContext * /*ctx*/) override { }
  virtual void exitInlineDataOnevar(SPARQLParser::InlineDataOnevarContext * /*ctx*/) override { }

  virtual void enterInlineDataFull(SPARQLParser::InlineDataFullContext * /*ctx*/) override { }
  virtual void exitInlineDataFull(SPARQLParser::InlineDataFullContext * /*ctx*/) override { }

  virtual void enterDataBlockValue(SPARQLParser::DataBlockValueContext * /*ctx*/) override { }
  virtual void exitDataBlockValue(SPARQLParser::DataBlockValueContext * /*ctx*/) override { }

  virtual void enterMinusGraphPattern(SPARQLParser::MinusGraphPatternContext * /*ctx*/) override { }
  virtual void exitMinusGraphPattern(SPARQLParser::MinusGraphPatternContext * /*ctx*/) override { }

  virtual void enterGroupOrUnionGraphPattern(SPARQLParser::GroupOrUnionGraphPatternContext * /*ctx*/) override { }
  virtual void exitGroupOrUnionGraphPattern(SPARQLParser::GroupOrUnionGraphPatternContext * /*ctx*/) override { }

  virtual void enterFilter(SPARQLParser::FilterContext * /*ctx*/) override { }
  virtual void exitFilter(SPARQLParser::FilterContext * /*ctx*/) override { }

  virtual void enterConstraint(SPARQLParser::ConstraintContext * /*ctx*/) override { }
  virtual void exitConstraint(SPARQLParser::ConstraintContext * /*ctx*/) override { }

  virtual void enterFunctionCall(SPARQLParser::FunctionCallContext * /*ctx*/) override { }
  virtual void exitFunctionCall(SPARQLParser::FunctionCallContext * /*ctx*/) override { }

  virtual void enterArgList(SPARQLParser::ArgListContext * /*ctx*/) override { }
  virtual void exitArgList(SPARQLParser::ArgListContext * /*ctx*/) override { }

  virtual void enterExpressionList(SPARQLParser::ExpressionListContext * /*ctx*/) override { }
  virtual void exitExpressionList(SPARQLParser::ExpressionListContext * /*ctx*/) override { }

  virtual void enterConstructTemplate(SPARQLParser::ConstructTemplateContext * /*ctx*/) override { }
  virtual void exitConstructTemplate(SPARQLParser::ConstructTemplateContext * /*ctx*/) override { }

  virtual void enterConstructTriples(SPARQLParser::ConstructTriplesContext * /*ctx*/) override { }
  virtual void exitConstructTriples(SPARQLParser::ConstructTriplesContext * /*ctx*/) override { }

  virtual void enterTriplesSameSubject(SPARQLParser::TriplesSameSubjectContext * /*ctx*/) override { }
  virtual void exitTriplesSameSubject(SPARQLParser::TriplesSameSubjectContext * /*ctx*/) override { }

  virtual void enterPropertyList(SPARQLParser::PropertyListContext * /*ctx*/) override { }
  virtual void exitPropertyList(SPARQLParser::PropertyListContext * /*ctx*/) override { }

  virtual void enterPropertyListNotEmpty(SPARQLParser::PropertyListNotEmptyContext * /*ctx*/) override { }
  virtual void exitPropertyListNotEmpty(SPARQLParser::PropertyListNotEmptyContext * /*ctx*/) override { }

  virtual void enterVerb(SPARQLParser::VerbContext * /*ctx*/) override { }
  virtual void exitVerb(SPARQLParser::VerbContext * /*ctx*/) override { }

  virtual void enterObjectList(SPARQLParser::ObjectListContext * /*ctx*/) override { }
  virtual void exitObjectList(SPARQLParser::ObjectListContext * /*ctx*/) override { }

  virtual void enterObject(SPARQLParser::ObjectContext * /*ctx*/) override { }
  virtual void exitObject(SPARQLParser::ObjectContext * /*ctx*/) override { }

  virtual void enterTriplesSameSubjectpath(SPARQLParser::TriplesSameSubjectpathContext * /*ctx*/) override { }
  virtual void exitTriplesSameSubjectpath(SPARQLParser::TriplesSameSubjectpathContext * /*ctx*/) override { }

  virtual void enterPropertyListpath(SPARQLParser::PropertyListpathContext * /*ctx*/) override { }
  virtual void exitPropertyListpath(SPARQLParser::PropertyListpathContext * /*ctx*/) override { }

  virtual void enterPropertyListpathNotEmpty(SPARQLParser::PropertyListpathNotEmptyContext * /*ctx*/) override { }
  virtual void exitPropertyListpathNotEmpty(SPARQLParser::PropertyListpathNotEmptyContext * /*ctx*/) override { }

  virtual void enterVerbpathOrSimple(SPARQLParser::VerbpathOrSimpleContext * /*ctx*/) override { }
  virtual void exitVerbpathOrSimple(SPARQLParser::VerbpathOrSimpleContext * /*ctx*/) override { }

  virtual void enterVerbpath(SPARQLParser::VerbpathContext * /*ctx*/) override { }
  virtual void exitVerbpath(SPARQLParser::VerbpathContext * /*ctx*/) override { }

  virtual void enterVerbSimple(SPARQLParser::VerbSimpleContext * /*ctx*/) override { }
  virtual void exitVerbSimple(SPARQLParser::VerbSimpleContext * /*ctx*/) override { }

  virtual void enterObjectListpath(SPARQLParser::ObjectListpathContext * /*ctx*/) override { }
  virtual void exitObjectListpath(SPARQLParser::ObjectListpathContext * /*ctx*/) override { }

  virtual void enterObjectpath(SPARQLParser::ObjectpathContext * /*ctx*/) override { }
  virtual void exitObjectpath(SPARQLParser::ObjectpathContext * /*ctx*/) override { }

  virtual void enterPath(SPARQLParser::PathContext * /*ctx*/) override { }
  virtual void exitPath(SPARQLParser::PathContext * /*ctx*/) override { }

  virtual void enterPathAlternative(SPARQLParser::PathAlternativeContext * /*ctx*/) override { }
  virtual void exitPathAlternative(SPARQLParser::PathAlternativeContext * /*ctx*/) override { }

  virtual void enterPathSequence(SPARQLParser::PathSequenceContext * /*ctx*/) override { }
  virtual void exitPathSequence(SPARQLParser::PathSequenceContext * /*ctx*/) override { }

  virtual void enterPathElt(SPARQLParser::PathEltContext * /*ctx*/) override { }
  virtual void exitPathElt(SPARQLParser::PathEltContext * /*ctx*/) override { }

  virtual void enterPathEltOrInverse(SPARQLParser::PathEltOrInverseContext * /*ctx*/) override { }
  virtual void exitPathEltOrInverse(SPARQLParser::PathEltOrInverseContext * /*ctx*/) override { }

  virtual void enterPathMod(SPARQLParser::PathModContext * /*ctx*/) override { }
  virtual void exitPathMod(SPARQLParser::PathModContext * /*ctx*/) override { }

  virtual void enterPathPrimary(SPARQLParser::PathPrimaryContext * /*ctx*/) override { }
  virtual void exitPathPrimary(SPARQLParser::PathPrimaryContext * /*ctx*/) override { }

  virtual void enterPathNegatedPropertySet(SPARQLParser::PathNegatedPropertySetContext * /*ctx*/) override { }
  virtual void exitPathNegatedPropertySet(SPARQLParser::PathNegatedPropertySetContext * /*ctx*/) override { }

  virtual void enterPathOneInPropertySet(SPARQLParser::PathOneInPropertySetContext * /*ctx*/) override { }
  virtual void exitPathOneInPropertySet(SPARQLParser::PathOneInPropertySetContext * /*ctx*/) override { }

  virtual void enterTriplesNode(SPARQLParser::TriplesNodeContext * /*ctx*/) override { }
  virtual void exitTriplesNode(SPARQLParser::TriplesNodeContext * /*ctx*/) override { }

  virtual void enterBlankNodepropertyList(SPARQLParser::BlankNodepropertyListContext * /*ctx*/) override { }
  virtual void exitBlankNodepropertyList(SPARQLParser::BlankNodepropertyListContext * /*ctx*/) override { }

  virtual void enterTriplesNodepath(SPARQLParser::TriplesNodepathContext * /*ctx*/) override { }
  virtual void exitTriplesNodepath(SPARQLParser::TriplesNodepathContext * /*ctx*/) override { }

  virtual void enterBlankNodepropertyListpath(SPARQLParser::BlankNodepropertyListpathContext * /*ctx*/) override { }
  virtual void exitBlankNodepropertyListpath(SPARQLParser::BlankNodepropertyListpathContext * /*ctx*/) override { }

  virtual void enterCollection(SPARQLParser::CollectionContext * /*ctx*/) override { }
  virtual void exitCollection(SPARQLParser::CollectionContext * /*ctx*/) override { }

  virtual void enterCollectionpath(SPARQLParser::CollectionpathContext * /*ctx*/) override { }
  virtual void exitCollectionpath(SPARQLParser::CollectionpathContext * /*ctx*/) override { }

  virtual void enterGraphNode(SPARQLParser::GraphNodeContext * /*ctx*/) override { }
  virtual void exitGraphNode(SPARQLParser::GraphNodeContext * /*ctx*/) override { }

  virtual void enterGraphNodepath(SPARQLParser::GraphNodepathContext * /*ctx*/) override { }
  virtual void exitGraphNodepath(SPARQLParser::GraphNodepathContext * /*ctx*/) override { }

  virtual void enterVarOrTerm(SPARQLParser::VarOrTermContext * /*ctx*/) override { }
  virtual void exitVarOrTerm(SPARQLParser::VarOrTermContext * /*ctx*/) override { }

  virtual void enterVarOrIri(SPARQLParser::VarOrIriContext * /*ctx*/) override { }
  virtual void exitVarOrIri(SPARQLParser::VarOrIriContext * /*ctx*/) override { }

  virtual void enterVarOrIriSet(SPARQLParser::VarOrIriSetContext * /*ctx*/) override { }
  virtual void exitVarOrIriSet(SPARQLParser::VarOrIriSetContext * /*ctx*/) override { }

  virtual void enterVar(SPARQLParser::VarContext * /*ctx*/) override { }
  virtual void exitVar(SPARQLParser::VarContext * /*ctx*/) override { }

  virtual void enterGraphTerm(SPARQLParser::GraphTermContext * /*ctx*/) override { }
  virtual void exitGraphTerm(SPARQLParser::GraphTermContext * /*ctx*/) override { }

  virtual void enterExpression(SPARQLParser::ExpressionContext * /*ctx*/) override { }
  virtual void exitExpression(SPARQLParser::ExpressionContext * /*ctx*/) override { }

  virtual void enterConditionalOrexpression(SPARQLParser::ConditionalOrexpressionContext * /*ctx*/) override { }
  virtual void exitConditionalOrexpression(SPARQLParser::ConditionalOrexpressionContext * /*ctx*/) override { }

  virtual void enterConditionalAndexpression(SPARQLParser::ConditionalAndexpressionContext * /*ctx*/) override { }
  virtual void exitConditionalAndexpression(SPARQLParser::ConditionalAndexpressionContext * /*ctx*/) override { }

  virtual void enterValueLogical(SPARQLParser::ValueLogicalContext * /*ctx*/) override { }
  virtual void exitValueLogical(SPARQLParser::ValueLogicalContext * /*ctx*/) override { }

  virtual void enterRelationalexpression(SPARQLParser::RelationalexpressionContext * /*ctx*/) override { }
  virtual void exitRelationalexpression(SPARQLParser::RelationalexpressionContext * /*ctx*/) override { }

  virtual void enterNumericexpression(SPARQLParser::NumericexpressionContext * /*ctx*/) override { }
  virtual void exitNumericexpression(SPARQLParser::NumericexpressionContext * /*ctx*/) override { }

  virtual void enterAdditiveexpression(SPARQLParser::AdditiveexpressionContext * /*ctx*/) override { }
  virtual void exitAdditiveexpression(SPARQLParser::AdditiveexpressionContext * /*ctx*/) override { }

  virtual void enterMultiplicativeexpression(SPARQLParser::MultiplicativeexpressionContext * /*ctx*/) override { }
  virtual void exitMultiplicativeexpression(SPARQLParser::MultiplicativeexpressionContext * /*ctx*/) override { }

  virtual void enterUnaryexpression(SPARQLParser::UnaryexpressionContext * /*ctx*/) override { }
  virtual void exitUnaryexpression(SPARQLParser::UnaryexpressionContext * /*ctx*/) override { }

  virtual void enterPrimaryexpression(SPARQLParser::PrimaryexpressionContext * /*ctx*/) override { }
  virtual void exitPrimaryexpression(SPARQLParser::PrimaryexpressionContext * /*ctx*/) override { }

  virtual void enterBrackettedexpression(SPARQLParser::BrackettedexpressionContext * /*ctx*/) override { }
  virtual void exitBrackettedexpression(SPARQLParser::BrackettedexpressionContext * /*ctx*/) override { }

  virtual void enterPredSet(SPARQLParser::PredSetContext * /*ctx*/) override { }
  virtual void exitPredSet(SPARQLParser::PredSetContext * /*ctx*/) override { }

  virtual void enterBuiltInCall(SPARQLParser::BuiltInCallContext * /*ctx*/) override { }
  virtual void exitBuiltInCall(SPARQLParser::BuiltInCallContext * /*ctx*/) override { }

  virtual void enterRegexexpression(SPARQLParser::RegexexpressionContext * /*ctx*/) override { }
  virtual void exitRegexexpression(SPARQLParser::RegexexpressionContext * /*ctx*/) override { }

  virtual void enterSubstringexpression(SPARQLParser::SubstringexpressionContext * /*ctx*/) override { }
  virtual void exitSubstringexpression(SPARQLParser::SubstringexpressionContext * /*ctx*/) override { }

  virtual void enterStrReplaceexpression(SPARQLParser::StrReplaceexpressionContext * /*ctx*/) override { }
  virtual void exitStrReplaceexpression(SPARQLParser::StrReplaceexpressionContext * /*ctx*/) override { }

  virtual void enterExistsFunc(SPARQLParser::ExistsFuncContext * /*ctx*/) override { }
  virtual void exitExistsFunc(SPARQLParser::ExistsFuncContext * /*ctx*/) override { }

  virtual void enterNotexistsFunc(SPARQLParser::NotexistsFuncContext * /*ctx*/) override { }
  virtual void exitNotexistsFunc(SPARQLParser::NotexistsFuncContext * /*ctx*/) override { }

  virtual void enterAggregate(SPARQLParser::AggregateContext * /*ctx*/) override { }
  virtual void exitAggregate(SPARQLParser::AggregateContext * /*ctx*/) override { }

  virtual void enterIriOrFunction(SPARQLParser::IriOrFunctionContext * /*ctx*/) override { }
  virtual void exitIriOrFunction(SPARQLParser::IriOrFunctionContext * /*ctx*/) override { }

  virtual void enterRDFLiteral(SPARQLParser::RDFLiteralContext * /*ctx*/) override { }
  virtual void exitRDFLiteral(SPARQLParser::RDFLiteralContext * /*ctx*/) override { }

  virtual void enterNumericLiteral(SPARQLParser::NumericLiteralContext * /*ctx*/) override { }
  virtual void exitNumericLiteral(SPARQLParser::NumericLiteralContext * /*ctx*/) override { }

  virtual void enterNumericLiteralUnsigned(SPARQLParser::NumericLiteralUnsignedContext * /*ctx*/) override { }
  virtual void exitNumericLiteralUnsigned(SPARQLParser::NumericLiteralUnsignedContext * /*ctx*/) override { }

  virtual void enterNumericLiteralPositive(SPARQLParser::NumericLiteralPositiveContext * /*ctx*/) override { }
  virtual void exitNumericLiteralPositive(SPARQLParser::NumericLiteralPositiveContext * /*ctx*/) override { }

  virtual void enterNumericLiteralNegative(SPARQLParser::NumericLiteralNegativeContext * /*ctx*/) override { }
  virtual void exitNumericLiteralNegative(SPARQLParser::NumericLiteralNegativeContext * /*ctx*/) override { }

  virtual void enterNum_integer(SPARQLParser::Num_integerContext * /*ctx*/) override { }
  virtual void exitNum_integer(SPARQLParser::Num_integerContext * /*ctx*/) override { }

  virtual void enterNum_decimal(SPARQLParser::Num_decimalContext * /*ctx*/) override { }
  virtual void exitNum_decimal(SPARQLParser::Num_decimalContext * /*ctx*/) override { }

  virtual void enterNum_double(SPARQLParser::Num_doubleContext * /*ctx*/) override { }
  virtual void exitNum_double(SPARQLParser::Num_doubleContext * /*ctx*/) override { }

  virtual void enterInteger_positive(SPARQLParser::Integer_positiveContext * /*ctx*/) override { }
  virtual void exitInteger_positive(SPARQLParser::Integer_positiveContext * /*ctx*/) override { }

  virtual void enterDecimal_positive(SPARQLParser::Decimal_positiveContext * /*ctx*/) override { }
  virtual void exitDecimal_positive(SPARQLParser::Decimal_positiveContext * /*ctx*/) override { }

  virtual void enterDouble_positive(SPARQLParser::Double_positiveContext * /*ctx*/) override { }
  virtual void exitDouble_positive(SPARQLParser::Double_positiveContext * /*ctx*/) override { }

  virtual void enterInteger_negative(SPARQLParser::Integer_negativeContext * /*ctx*/) override { }
  virtual void exitInteger_negative(SPARQLParser::Integer_negativeContext * /*ctx*/) override { }

  virtual void enterDecimal_negative(SPARQLParser::Decimal_negativeContext * /*ctx*/) override { }
  virtual void exitDecimal_negative(SPARQLParser::Decimal_negativeContext * /*ctx*/) override { }

  virtual void enterDouble_negative(SPARQLParser::Double_negativeContext * /*ctx*/) override { }
  virtual void exitDouble_negative(SPARQLParser::Double_negativeContext * /*ctx*/) override { }

  virtual void enterBooleanLiteral(SPARQLParser::BooleanLiteralContext * /*ctx*/) override { }
  virtual void exitBooleanLiteral(SPARQLParser::BooleanLiteralContext * /*ctx*/) override { }

  virtual void enterString(SPARQLParser::StringContext * /*ctx*/) override { }
  virtual void exitString(SPARQLParser::StringContext * /*ctx*/) override { }

  virtual void enterIri(SPARQLParser::IriContext * /*ctx*/) override { }
  virtual void exitIri(SPARQLParser::IriContext * /*ctx*/) override { }

  virtual void enterPrefixedName(SPARQLParser::PrefixedNameContext * /*ctx*/) override { }
  virtual void exitPrefixedName(SPARQLParser::PrefixedNameContext * /*ctx*/) override { }

  virtual void enterBlankNode(SPARQLParser::BlankNodeContext * /*ctx*/) override { }
  virtual void exitBlankNode(SPARQLParser::BlankNodeContext * /*ctx*/) override { }


  virtual void enterEveryRule(antlr4::ParserRuleContext * /*ctx*/) override { }
  virtual void exitEveryRule(antlr4::ParserRuleContext * /*ctx*/) override { }
  virtual void visitTerminal(antlr4::tree::TerminalNode * /*node*/) override { }
  virtual void visitErrorNode(antlr4::tree::ErrorNode * /*node*/) override { }

};

