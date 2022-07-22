
// Generated from ./SPARQL.g4 by ANTLR 4.7.2

#pragma once


#include "antlr4-runtime.h"
#include "SPARQLParser.h"



/**
 * This class defines an abstract visitor for a parse tree
 * produced by SPARQLParser.
 */
class  SPARQLVisitor : public antlr4::tree::AbstractParseTreeVisitor {
public:

  /**
   * Visit parse trees produced by SPARQLParser.
   */
    virtual antlrcpp::Any visitEntry(SPARQLParser::EntryContext *context) = 0;

    virtual antlrcpp::Any visitQueryUnit(SPARQLParser::QueryUnitContext *context) = 0;

    virtual antlrcpp::Any visitQuery(SPARQLParser::QueryContext *context) = 0;

    virtual antlrcpp::Any visitUpdateUnit(SPARQLParser::UpdateUnitContext *context) = 0;

    virtual antlrcpp::Any visitPrologue(SPARQLParser::PrologueContext *context) = 0;

    virtual antlrcpp::Any visitBaseDecl(SPARQLParser::BaseDeclContext *context) = 0;

    virtual antlrcpp::Any visitPrefixDecl(SPARQLParser::PrefixDeclContext *context) = 0;

    virtual antlrcpp::Any visitSelectquery(SPARQLParser::SelectqueryContext *context) = 0;

    virtual antlrcpp::Any visitSubSelect(SPARQLParser::SubSelectContext *context) = 0;

    virtual antlrcpp::Any visitSelectClause(SPARQLParser::SelectClauseContext *context) = 0;

    virtual antlrcpp::Any visitExpressionAsVar(SPARQLParser::ExpressionAsVarContext *context) = 0;

    virtual antlrcpp::Any visitConstructquery(SPARQLParser::ConstructqueryContext *context) = 0;

    virtual antlrcpp::Any visitDescribequery(SPARQLParser::DescribequeryContext *context) = 0;

    virtual antlrcpp::Any visitAskquery(SPARQLParser::AskqueryContext *context) = 0;

    virtual antlrcpp::Any visitDatasetClause(SPARQLParser::DatasetClauseContext *context) = 0;

    virtual antlrcpp::Any visitDefaultGraphClause(SPARQLParser::DefaultGraphClauseContext *context) = 0;

    virtual antlrcpp::Any visitNamedGraphClause(SPARQLParser::NamedGraphClauseContext *context) = 0;

    virtual antlrcpp::Any visitSourceSelector(SPARQLParser::SourceSelectorContext *context) = 0;

    virtual antlrcpp::Any visitWhereClause(SPARQLParser::WhereClauseContext *context) = 0;

    virtual antlrcpp::Any visitSolutionModifier(SPARQLParser::SolutionModifierContext *context) = 0;

    virtual antlrcpp::Any visitGroupClause(SPARQLParser::GroupClauseContext *context) = 0;

    virtual antlrcpp::Any visitGroupCondition(SPARQLParser::GroupConditionContext *context) = 0;

    virtual antlrcpp::Any visitHavingClause(SPARQLParser::HavingClauseContext *context) = 0;

    virtual antlrcpp::Any visitHavingCondition(SPARQLParser::HavingConditionContext *context) = 0;

    virtual antlrcpp::Any visitOrderClause(SPARQLParser::OrderClauseContext *context) = 0;

    virtual antlrcpp::Any visitOrderCondition(SPARQLParser::OrderConditionContext *context) = 0;

    virtual antlrcpp::Any visitLimitoffsetClauses(SPARQLParser::LimitoffsetClausesContext *context) = 0;

    virtual antlrcpp::Any visitLimitClause(SPARQLParser::LimitClauseContext *context) = 0;

    virtual antlrcpp::Any visitOffsetClause(SPARQLParser::OffsetClauseContext *context) = 0;

    virtual antlrcpp::Any visitValuesClause(SPARQLParser::ValuesClauseContext *context) = 0;

    virtual antlrcpp::Any visitUpdate(SPARQLParser::UpdateContext *context) = 0;

    virtual antlrcpp::Any visitUpdate1(SPARQLParser::Update1Context *context) = 0;

    virtual antlrcpp::Any visitLoad(SPARQLParser::LoadContext *context) = 0;

    virtual antlrcpp::Any visitClear(SPARQLParser::ClearContext *context) = 0;

    virtual antlrcpp::Any visitDrop(SPARQLParser::DropContext *context) = 0;

    virtual antlrcpp::Any visitCreate(SPARQLParser::CreateContext *context) = 0;

    virtual antlrcpp::Any visitAdd(SPARQLParser::AddContext *context) = 0;

    virtual antlrcpp::Any visitMove(SPARQLParser::MoveContext *context) = 0;

    virtual antlrcpp::Any visitCopy(SPARQLParser::CopyContext *context) = 0;

    virtual antlrcpp::Any visitInsertData(SPARQLParser::InsertDataContext *context) = 0;

    virtual antlrcpp::Any visitDeleteData(SPARQLParser::DeleteDataContext *context) = 0;

    virtual antlrcpp::Any visitDeleteWhere(SPARQLParser::DeleteWhereContext *context) = 0;

    virtual antlrcpp::Any visitModify(SPARQLParser::ModifyContext *context) = 0;

    virtual antlrcpp::Any visitDeleteClause(SPARQLParser::DeleteClauseContext *context) = 0;

    virtual antlrcpp::Any visitInsertClause(SPARQLParser::InsertClauseContext *context) = 0;

    virtual antlrcpp::Any visitUsingClause(SPARQLParser::UsingClauseContext *context) = 0;

    virtual antlrcpp::Any visitGraphOrDefault(SPARQLParser::GraphOrDefaultContext *context) = 0;

    virtual antlrcpp::Any visitGraphRef(SPARQLParser::GraphRefContext *context) = 0;

    virtual antlrcpp::Any visitGraphRefAll(SPARQLParser::GraphRefAllContext *context) = 0;

    virtual antlrcpp::Any visitQuadPattern(SPARQLParser::QuadPatternContext *context) = 0;

    virtual antlrcpp::Any visitQuadData(SPARQLParser::QuadDataContext *context) = 0;

    virtual antlrcpp::Any visitQuads(SPARQLParser::QuadsContext *context) = 0;

    virtual antlrcpp::Any visitQuadsNotTriples(SPARQLParser::QuadsNotTriplesContext *context) = 0;

    virtual antlrcpp::Any visitTriplesTemplate(SPARQLParser::TriplesTemplateContext *context) = 0;

    virtual antlrcpp::Any visitGroupGraphPattern(SPARQLParser::GroupGraphPatternContext *context) = 0;

    virtual antlrcpp::Any visitGroupGraphPatternSub(SPARQLParser::GroupGraphPatternSubContext *context) = 0;

    virtual antlrcpp::Any visitGraphPatternTriplesBlock(SPARQLParser::GraphPatternTriplesBlockContext *context) = 0;

    virtual antlrcpp::Any visitTriplesBlock(SPARQLParser::TriplesBlockContext *context) = 0;

    virtual antlrcpp::Any visitGraphPatternNotTriples(SPARQLParser::GraphPatternNotTriplesContext *context) = 0;

    virtual antlrcpp::Any visitOptionalGraphPattern(SPARQLParser::OptionalGraphPatternContext *context) = 0;

    virtual antlrcpp::Any visitGraphGraphPattern(SPARQLParser::GraphGraphPatternContext *context) = 0;

    virtual antlrcpp::Any visitServiceGraphPattern(SPARQLParser::ServiceGraphPatternContext *context) = 0;

    virtual antlrcpp::Any visitBind(SPARQLParser::BindContext *context) = 0;

    virtual antlrcpp::Any visitInlineData(SPARQLParser::InlineDataContext *context) = 0;

    virtual antlrcpp::Any visitDataBlock(SPARQLParser::DataBlockContext *context) = 0;

    virtual antlrcpp::Any visitInlineDataOnevar(SPARQLParser::InlineDataOnevarContext *context) = 0;

    virtual antlrcpp::Any visitInlineDataFull(SPARQLParser::InlineDataFullContext *context) = 0;

    virtual antlrcpp::Any visitDataBlockValue(SPARQLParser::DataBlockValueContext *context) = 0;

    virtual antlrcpp::Any visitMinusGraphPattern(SPARQLParser::MinusGraphPatternContext *context) = 0;

    virtual antlrcpp::Any visitGroupOrUnionGraphPattern(SPARQLParser::GroupOrUnionGraphPatternContext *context) = 0;

    virtual antlrcpp::Any visitFilter(SPARQLParser::FilterContext *context) = 0;

    virtual antlrcpp::Any visitConstraint(SPARQLParser::ConstraintContext *context) = 0;

    virtual antlrcpp::Any visitFunctionCall(SPARQLParser::FunctionCallContext *context) = 0;

    virtual antlrcpp::Any visitArgList(SPARQLParser::ArgListContext *context) = 0;

    virtual antlrcpp::Any visitExpressionList(SPARQLParser::ExpressionListContext *context) = 0;

    virtual antlrcpp::Any visitConstructTemplate(SPARQLParser::ConstructTemplateContext *context) = 0;

    virtual antlrcpp::Any visitConstructTriples(SPARQLParser::ConstructTriplesContext *context) = 0;

    virtual antlrcpp::Any visitTriplesSameSubject(SPARQLParser::TriplesSameSubjectContext *context) = 0;

    virtual antlrcpp::Any visitPropertyList(SPARQLParser::PropertyListContext *context) = 0;

    virtual antlrcpp::Any visitPropertyListNotEmpty(SPARQLParser::PropertyListNotEmptyContext *context) = 0;

    virtual antlrcpp::Any visitVerb(SPARQLParser::VerbContext *context) = 0;

    virtual antlrcpp::Any visitObjectList(SPARQLParser::ObjectListContext *context) = 0;

    virtual antlrcpp::Any visitObject(SPARQLParser::ObjectContext *context) = 0;

    virtual antlrcpp::Any visitTriplesSameSubjectpath(SPARQLParser::TriplesSameSubjectpathContext *context) = 0;

    virtual antlrcpp::Any visitPropertyListpath(SPARQLParser::PropertyListpathContext *context) = 0;

    virtual antlrcpp::Any visitPropertyListpathNotEmpty(SPARQLParser::PropertyListpathNotEmptyContext *context) = 0;

    virtual antlrcpp::Any visitVerbpathOrSimple(SPARQLParser::VerbpathOrSimpleContext *context) = 0;

    virtual antlrcpp::Any visitVerbpath(SPARQLParser::VerbpathContext *context) = 0;

    virtual antlrcpp::Any visitVerbSimple(SPARQLParser::VerbSimpleContext *context) = 0;

    virtual antlrcpp::Any visitObjectListpath(SPARQLParser::ObjectListpathContext *context) = 0;

    virtual antlrcpp::Any visitObjectpath(SPARQLParser::ObjectpathContext *context) = 0;

    virtual antlrcpp::Any visitPath(SPARQLParser::PathContext *context) = 0;

    virtual antlrcpp::Any visitPathAlternative(SPARQLParser::PathAlternativeContext *context) = 0;

    virtual antlrcpp::Any visitPathSequence(SPARQLParser::PathSequenceContext *context) = 0;

    virtual antlrcpp::Any visitPathElt(SPARQLParser::PathEltContext *context) = 0;

    virtual antlrcpp::Any visitPathEltOrInverse(SPARQLParser::PathEltOrInverseContext *context) = 0;

    virtual antlrcpp::Any visitPathMod(SPARQLParser::PathModContext *context) = 0;

    virtual antlrcpp::Any visitPathPrimary(SPARQLParser::PathPrimaryContext *context) = 0;

    virtual antlrcpp::Any visitPathNegatedPropertySet(SPARQLParser::PathNegatedPropertySetContext *context) = 0;

    virtual antlrcpp::Any visitPathOneInPropertySet(SPARQLParser::PathOneInPropertySetContext *context) = 0;

    virtual antlrcpp::Any visitTriplesNode(SPARQLParser::TriplesNodeContext *context) = 0;

    virtual antlrcpp::Any visitBlankNodepropertyList(SPARQLParser::BlankNodepropertyListContext *context) = 0;

    virtual antlrcpp::Any visitTriplesNodepath(SPARQLParser::TriplesNodepathContext *context) = 0;

    virtual antlrcpp::Any visitBlankNodepropertyListpath(SPARQLParser::BlankNodepropertyListpathContext *context) = 0;

    virtual antlrcpp::Any visitCollection(SPARQLParser::CollectionContext *context) = 0;

    virtual antlrcpp::Any visitCollectionpath(SPARQLParser::CollectionpathContext *context) = 0;

    virtual antlrcpp::Any visitGraphNode(SPARQLParser::GraphNodeContext *context) = 0;

    virtual antlrcpp::Any visitGraphNodepath(SPARQLParser::GraphNodepathContext *context) = 0;

    virtual antlrcpp::Any visitVarOrTerm(SPARQLParser::VarOrTermContext *context) = 0;

    virtual antlrcpp::Any visitVarOrIri(SPARQLParser::VarOrIriContext *context) = 0;

    virtual antlrcpp::Any visitVarOrIriSet(SPARQLParser::VarOrIriSetContext *context) = 0;

    virtual antlrcpp::Any visitVar(SPARQLParser::VarContext *context) = 0;

    virtual antlrcpp::Any visitGraphTerm(SPARQLParser::GraphTermContext *context) = 0;

    virtual antlrcpp::Any visitExpression(SPARQLParser::ExpressionContext *context) = 0;

    virtual antlrcpp::Any visitConditionalOrexpression(SPARQLParser::ConditionalOrexpressionContext *context) = 0;

    virtual antlrcpp::Any visitConditionalAndexpression(SPARQLParser::ConditionalAndexpressionContext *context) = 0;

    virtual antlrcpp::Any visitValueLogical(SPARQLParser::ValueLogicalContext *context) = 0;

    virtual antlrcpp::Any visitRelationalexpression(SPARQLParser::RelationalexpressionContext *context) = 0;

    virtual antlrcpp::Any visitNumericexpression(SPARQLParser::NumericexpressionContext *context) = 0;

    virtual antlrcpp::Any visitAdditiveexpression(SPARQLParser::AdditiveexpressionContext *context) = 0;

    virtual antlrcpp::Any visitMultiplicativeexpression(SPARQLParser::MultiplicativeexpressionContext *context) = 0;

    virtual antlrcpp::Any visitUnaryexpression(SPARQLParser::UnaryexpressionContext *context) = 0;

    virtual antlrcpp::Any visitPrimaryexpression(SPARQLParser::PrimaryexpressionContext *context) = 0;

    virtual antlrcpp::Any visitBrackettedexpression(SPARQLParser::BrackettedexpressionContext *context) = 0;

    virtual antlrcpp::Any visitPredSet(SPARQLParser::PredSetContext *context) = 0;

    virtual antlrcpp::Any visitBuiltInCall(SPARQLParser::BuiltInCallContext *context) = 0;

    virtual antlrcpp::Any visitRegexexpression(SPARQLParser::RegexexpressionContext *context) = 0;

    virtual antlrcpp::Any visitSubstringexpression(SPARQLParser::SubstringexpressionContext *context) = 0;

    virtual antlrcpp::Any visitStrReplaceexpression(SPARQLParser::StrReplaceexpressionContext *context) = 0;

    virtual antlrcpp::Any visitExistsFunc(SPARQLParser::ExistsFuncContext *context) = 0;

    virtual antlrcpp::Any visitNotexistsFunc(SPARQLParser::NotexistsFuncContext *context) = 0;

    virtual antlrcpp::Any visitAggregate(SPARQLParser::AggregateContext *context) = 0;

    virtual antlrcpp::Any visitIriOrFunction(SPARQLParser::IriOrFunctionContext *context) = 0;

    virtual antlrcpp::Any visitRDFLiteral(SPARQLParser::RDFLiteralContext *context) = 0;

    virtual antlrcpp::Any visitNumericLiteral(SPARQLParser::NumericLiteralContext *context) = 0;

    virtual antlrcpp::Any visitNumericLiteralUnsigned(SPARQLParser::NumericLiteralUnsignedContext *context) = 0;

    virtual antlrcpp::Any visitNumericLiteralPositive(SPARQLParser::NumericLiteralPositiveContext *context) = 0;

    virtual antlrcpp::Any visitNumericLiteralNegative(SPARQLParser::NumericLiteralNegativeContext *context) = 0;

    virtual antlrcpp::Any visitNum_integer(SPARQLParser::Num_integerContext *context) = 0;

    virtual antlrcpp::Any visitNum_decimal(SPARQLParser::Num_decimalContext *context) = 0;

    virtual antlrcpp::Any visitNum_double(SPARQLParser::Num_doubleContext *context) = 0;

    virtual antlrcpp::Any visitInteger_positive(SPARQLParser::Integer_positiveContext *context) = 0;

    virtual antlrcpp::Any visitDecimal_positive(SPARQLParser::Decimal_positiveContext *context) = 0;

    virtual antlrcpp::Any visitDouble_positive(SPARQLParser::Double_positiveContext *context) = 0;

    virtual antlrcpp::Any visitInteger_negative(SPARQLParser::Integer_negativeContext *context) = 0;

    virtual antlrcpp::Any visitDecimal_negative(SPARQLParser::Decimal_negativeContext *context) = 0;

    virtual antlrcpp::Any visitDouble_negative(SPARQLParser::Double_negativeContext *context) = 0;

    virtual antlrcpp::Any visitBooleanLiteral(SPARQLParser::BooleanLiteralContext *context) = 0;

    virtual antlrcpp::Any visitString(SPARQLParser::StringContext *context) = 0;

    virtual antlrcpp::Any visitIri(SPARQLParser::IriContext *context) = 0;

    virtual antlrcpp::Any visitPrefixedName(SPARQLParser::PrefixedNameContext *context) = 0;

    virtual antlrcpp::Any visitBlankNode(SPARQLParser::BlankNodeContext *context) = 0;


};

