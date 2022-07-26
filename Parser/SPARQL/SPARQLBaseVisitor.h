
// Generated from ./SPARQL.g4 by ANTLR 4.7.2

#pragma once


#include "antlr4-runtime.h"
#include "SPARQLVisitor.h"


/**
 * This class provides an empty implementation of SPARQLVisitor, which can be
 * extended to create a visitor which only needs to handle a subset of the available methods.
 */
class  SPARQLBaseVisitor : public SPARQLVisitor {
public:

  virtual antlrcpp::Any visitEntry(SPARQLParser::EntryContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitQueryUnit(SPARQLParser::QueryUnitContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitQuery(SPARQLParser::QueryContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitUpdateUnit(SPARQLParser::UpdateUnitContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitPrologue(SPARQLParser::PrologueContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitBaseDecl(SPARQLParser::BaseDeclContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitPrefixDecl(SPARQLParser::PrefixDeclContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitSelectquery(SPARQLParser::SelectqueryContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitSubSelect(SPARQLParser::SubSelectContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitSelectClause(SPARQLParser::SelectClauseContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitExpressionAsVar(SPARQLParser::ExpressionAsVarContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitConstructquery(SPARQLParser::ConstructqueryContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitDescribequery(SPARQLParser::DescribequeryContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitAskquery(SPARQLParser::AskqueryContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitDatasetClause(SPARQLParser::DatasetClauseContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitDefaultGraphClause(SPARQLParser::DefaultGraphClauseContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitNamedGraphClause(SPARQLParser::NamedGraphClauseContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitSourceSelector(SPARQLParser::SourceSelectorContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitWhereClause(SPARQLParser::WhereClauseContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitSolutionModifier(SPARQLParser::SolutionModifierContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitGroupClause(SPARQLParser::GroupClauseContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitGroupCondition(SPARQLParser::GroupConditionContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitHavingClause(SPARQLParser::HavingClauseContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitHavingCondition(SPARQLParser::HavingConditionContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitOrderClause(SPARQLParser::OrderClauseContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitOrderCondition(SPARQLParser::OrderConditionContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitLimitoffsetClauses(SPARQLParser::LimitoffsetClausesContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitLimitClause(SPARQLParser::LimitClauseContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitOffsetClause(SPARQLParser::OffsetClauseContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitValuesClause(SPARQLParser::ValuesClauseContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitUpdate(SPARQLParser::UpdateContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitUpdate1(SPARQLParser::Update1Context *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitLoad(SPARQLParser::LoadContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitClear(SPARQLParser::ClearContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitDrop(SPARQLParser::DropContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitCreate(SPARQLParser::CreateContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitAdd(SPARQLParser::AddContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitMove(SPARQLParser::MoveContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitCopy(SPARQLParser::CopyContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitInsertData(SPARQLParser::InsertDataContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitDeleteData(SPARQLParser::DeleteDataContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitDeleteWhere(SPARQLParser::DeleteWhereContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitModify(SPARQLParser::ModifyContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitDeleteClause(SPARQLParser::DeleteClauseContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitInsertClause(SPARQLParser::InsertClauseContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitUsingClause(SPARQLParser::UsingClauseContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitGraphOrDefault(SPARQLParser::GraphOrDefaultContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitGraphRef(SPARQLParser::GraphRefContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitGraphRefAll(SPARQLParser::GraphRefAllContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitQuadPattern(SPARQLParser::QuadPatternContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitQuadData(SPARQLParser::QuadDataContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitQuads(SPARQLParser::QuadsContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitQuadsNotTriples(SPARQLParser::QuadsNotTriplesContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitTriplesTemplate(SPARQLParser::TriplesTemplateContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitGroupGraphPattern(SPARQLParser::GroupGraphPatternContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitGroupGraphPatternSub(SPARQLParser::GroupGraphPatternSubContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitGraphPatternTriplesBlock(SPARQLParser::GraphPatternTriplesBlockContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitTriplesBlock(SPARQLParser::TriplesBlockContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitGraphPatternNotTriples(SPARQLParser::GraphPatternNotTriplesContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitOptionalGraphPattern(SPARQLParser::OptionalGraphPatternContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitGraphGraphPattern(SPARQLParser::GraphGraphPatternContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitServiceGraphPattern(SPARQLParser::ServiceGraphPatternContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitBind(SPARQLParser::BindContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitInlineData(SPARQLParser::InlineDataContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitDataBlock(SPARQLParser::DataBlockContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitInlineDataOnevar(SPARQLParser::InlineDataOnevarContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitInlineDataFull(SPARQLParser::InlineDataFullContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitDataBlockValue(SPARQLParser::DataBlockValueContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitMinusGraphPattern(SPARQLParser::MinusGraphPatternContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitGroupOrUnionGraphPattern(SPARQLParser::GroupOrUnionGraphPatternContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitFilter(SPARQLParser::FilterContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitConstraint(SPARQLParser::ConstraintContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitFunctionCall(SPARQLParser::FunctionCallContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitArgList(SPARQLParser::ArgListContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitExpressionList(SPARQLParser::ExpressionListContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitConstructTemplate(SPARQLParser::ConstructTemplateContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitConstructTriples(SPARQLParser::ConstructTriplesContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitTriplesSameSubject(SPARQLParser::TriplesSameSubjectContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitPropertyList(SPARQLParser::PropertyListContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitPropertyListNotEmpty(SPARQLParser::PropertyListNotEmptyContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitVerb(SPARQLParser::VerbContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitObjectList(SPARQLParser::ObjectListContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitObject(SPARQLParser::ObjectContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitTriplesSameSubjectpath(SPARQLParser::TriplesSameSubjectpathContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitPropertyListpath(SPARQLParser::PropertyListpathContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitPropertyListpathNotEmpty(SPARQLParser::PropertyListpathNotEmptyContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitVerbpathOrSimple(SPARQLParser::VerbpathOrSimpleContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitVerbpath(SPARQLParser::VerbpathContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitVerbSimple(SPARQLParser::VerbSimpleContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitObjectListpath(SPARQLParser::ObjectListpathContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitObjectpath(SPARQLParser::ObjectpathContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitPath(SPARQLParser::PathContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitPathAlternative(SPARQLParser::PathAlternativeContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitPathSequence(SPARQLParser::PathSequenceContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitPathElt(SPARQLParser::PathEltContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitPathEltOrInverse(SPARQLParser::PathEltOrInverseContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitPathMod(SPARQLParser::PathModContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitPathPrimary(SPARQLParser::PathPrimaryContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitPathNegatedPropertySet(SPARQLParser::PathNegatedPropertySetContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitPathOneInPropertySet(SPARQLParser::PathOneInPropertySetContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitTriplesNode(SPARQLParser::TriplesNodeContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitBlankNodepropertyList(SPARQLParser::BlankNodepropertyListContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitTriplesNodepath(SPARQLParser::TriplesNodepathContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitBlankNodepropertyListpath(SPARQLParser::BlankNodepropertyListpathContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitCollection(SPARQLParser::CollectionContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitCollectionpath(SPARQLParser::CollectionpathContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitGraphNode(SPARQLParser::GraphNodeContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitGraphNodepath(SPARQLParser::GraphNodepathContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitVarOrTerm(SPARQLParser::VarOrTermContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitVarOrIri(SPARQLParser::VarOrIriContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitVarOrIriSet(SPARQLParser::VarOrIriSetContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitVar(SPARQLParser::VarContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitGraphTerm(SPARQLParser::GraphTermContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitExpression(SPARQLParser::ExpressionContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitConditionalOrexpression(SPARQLParser::ConditionalOrexpressionContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitConditionalAndexpression(SPARQLParser::ConditionalAndexpressionContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitValueLogical(SPARQLParser::ValueLogicalContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitRelationalexpression(SPARQLParser::RelationalexpressionContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitNumericexpression(SPARQLParser::NumericexpressionContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitAdditiveexpression(SPARQLParser::AdditiveexpressionContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitMultiplicativeexpression(SPARQLParser::MultiplicativeexpressionContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitUnaryexpression(SPARQLParser::UnaryexpressionContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitPrimaryexpression(SPARQLParser::PrimaryexpressionContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitBrackettedexpression(SPARQLParser::BrackettedexpressionContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitPredSet(SPARQLParser::PredSetContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitBuiltInCall(SPARQLParser::BuiltInCallContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitRegexexpression(SPARQLParser::RegexexpressionContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitSubstringexpression(SPARQLParser::SubstringexpressionContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitStrReplaceexpression(SPARQLParser::StrReplaceexpressionContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitExistsFunc(SPARQLParser::ExistsFuncContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitNotexistsFunc(SPARQLParser::NotexistsFuncContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitAggregate(SPARQLParser::AggregateContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitIriOrFunction(SPARQLParser::IriOrFunctionContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitRDFLiteral(SPARQLParser::RDFLiteralContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitNumericLiteral(SPARQLParser::NumericLiteralContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitNumericLiteralUnsigned(SPARQLParser::NumericLiteralUnsignedContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitNumericLiteralPositive(SPARQLParser::NumericLiteralPositiveContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitNumericLiteralNegative(SPARQLParser::NumericLiteralNegativeContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitNum_integer(SPARQLParser::Num_integerContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitNum_decimal(SPARQLParser::Num_decimalContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitNum_double(SPARQLParser::Num_doubleContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitInteger_positive(SPARQLParser::Integer_positiveContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitDecimal_positive(SPARQLParser::Decimal_positiveContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitDouble_positive(SPARQLParser::Double_positiveContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitInteger_negative(SPARQLParser::Integer_negativeContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitDecimal_negative(SPARQLParser::Decimal_negativeContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitDouble_negative(SPARQLParser::Double_negativeContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitBooleanLiteral(SPARQLParser::BooleanLiteralContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitString(SPARQLParser::StringContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitIri(SPARQLParser::IriContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitPrefixedName(SPARQLParser::PrefixedNameContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitBlankNode(SPARQLParser::BlankNodeContext *ctx) override {
    return visitChildren(ctx);
  }


};

