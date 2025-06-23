#include "SparqlUtil.h"

using namespace std;

namespace gs
{
    void SyntaxErrorListener::syntaxError(antlr4::Recognizer *recognizer, antlr4::Token *offendingSymbol,
                                          size_t line, size_t charPositionInLine, const std::string &msg, std::exception_ptr e)
    {
        throw runtime_error("[SyntaxError]:line " + to_string(line) + ":" + to_string(charPositionInLine) + " " + msg);
    }

    bool SparqlUtil::validate(const std::string &sparql, std::string& msg)
    {
        try
        {
            SyntaxErrorListener lstnr;

            antlr4::ANTLRInputStream input(sparql);
            SPARQLLexer lexer(&input);
            lexer.removeErrorListeners();
            lexer.addErrorListener(&lstnr);

            antlr4::CommonTokenStream tokens(&lexer);
            SPARQLParser parser(&tokens);
            parser.removeErrorListeners();
            parser.addErrorListener(&lstnr);

            // Parse the query and get the parse tree
            SPARQLBaseVisitor vistor;

            SPARQLParser::EntryContext *tree = parser.entry();
            vistor.visitEntry(tree);
            SLOG_DEBUG("sparql is valid: " << sparql);
            return true;
        }
        catch (const runtime_error &e1)
        {
            msg = e1.what();
            SLOG_DEBUG("sparql is invalid: " << sparql);
            SLOG_DEBUG(msg);
            return false;
        }
    }

    bool SparqlUtil::validateQuery(const std::string &sparql, std::string& msg)
    {
        try
        {
            SyntaxErrorListener lstnr;

            antlr4::ANTLRInputStream input(sparql);
            SPARQLLexer lexer(&input);
            lexer.removeErrorListeners();
            lexer.addErrorListener(&lstnr);

            antlr4::CommonTokenStream tokens(&lexer);
            SPARQLParser parser(&tokens);
            parser.removeErrorListeners();
            parser.addErrorListener(&lstnr);

            // Parse the query and get the parse tree
            SPARQLBaseVisitor vistor;

            SPARQLParser::QueryContext *tree = parser.query();
            vistor.visitQuery(tree);
            SLOG_DEBUG("query is valid: " << sparql);
            return true;
        }
        catch (const runtime_error &e1)
        {
            msg = e1.what();
            SLOG_DEBUG("query is invalid: " << sparql);
            SLOG_DEBUG(msg);
            return false;
        }
    }

    bool SparqlUtil::validateInsertData(const std::string &sparql, std::string& msg)
    {
        try
        {
            SyntaxErrorListener lstnr;

            antlr4::ANTLRInputStream input(sparql);
            SPARQLLexer lexer(&input);
            lexer.removeErrorListeners();
            lexer.addErrorListener(&lstnr);

            antlr4::CommonTokenStream tokens(&lexer);
            SPARQLParser parser(&tokens);
            parser.removeErrorListeners();
            parser.addErrorListener(&lstnr);

            // Parse the query and get the parse tree
            SPARQLBaseVisitor vistor;

            SPARQLParser::InsertDataContext *tree = parser.insertData();
            vistor.visitInsertData(tree);
            SLOG_DEBUG("insertData is valid: " << sparql);
            return true;
        }
        catch (const runtime_error &e1)
        {
            msg = e1.what();
            SLOG_DEBUG("insertData is invalid: " << sparql);
            SLOG_DEBUG(msg);
            return false;
        }
    }

    bool SparqlUtil::validateInsertClause(const std::string &sparql, std::string& msg)
    {
        try
        {
            SyntaxErrorListener lstnr;

            antlr4::ANTLRInputStream input(sparql);
            SPARQLLexer lexer(&input);
            lexer.removeErrorListeners();
            lexer.addErrorListener(&lstnr);

            antlr4::CommonTokenStream tokens(&lexer);
            SPARQLParser parser(&tokens);
            parser.removeErrorListeners();
            parser.addErrorListener(&lstnr);

            // Parse the query and get the parse tree
            SPARQLBaseVisitor vistor;

            SPARQLParser::InsertClauseContext *tree = parser.insertClause();
            vistor.visitInsertClause(tree);
            SLOG_DEBUG("insertClause is valid: " << sparql);
            return true;
        }
        catch (const runtime_error &e1)
        {
            msg = e1.what();
            SLOG_DEBUG("insertClause is invalid: " << sparql);
            SLOG_DEBUG(msg);
            return false;
        }
    }

    bool SparqlUtil::validateDeleteData(const std::string &sparql, std::string& msg)
    {
        try
        {
            SyntaxErrorListener lstnr;

            antlr4::ANTLRInputStream input(sparql);
            SPARQLLexer lexer(&input);
            lexer.removeErrorListeners();
            lexer.addErrorListener(&lstnr);

            antlr4::CommonTokenStream tokens(&lexer);
            SPARQLParser parser(&tokens);
            parser.removeErrorListeners();
            parser.addErrorListener(&lstnr);

            // Parse the query and get the parse tree
            SPARQLBaseVisitor vistor;

            SPARQLParser::DeleteDataContext *tree = parser.deleteData();
            vistor.visitDeleteData(tree);
            SLOG_DEBUG("deleteData is valid: " << sparql);
            return true;
        }
        catch (const runtime_error &e1)
        {
            msg = e1.what();
            SLOG_DEBUG("deleteData is invalid: " << sparql);
            SLOG_DEBUG(msg);
            return false;
        }
    }

    bool SparqlUtil::validateDeleteClause(const std::string &sparql, std::string& msg)
    {
        try
        {
            SyntaxErrorListener lstnr;

            antlr4::ANTLRInputStream input(sparql);
            SPARQLLexer lexer(&input);
            lexer.removeErrorListeners();
            lexer.addErrorListener(&lstnr);

            antlr4::CommonTokenStream tokens(&lexer);
            SPARQLParser parser(&tokens);
            parser.removeErrorListeners();
            parser.addErrorListener(&lstnr);

            // Parse the query and get the parse tree
            SPARQLBaseVisitor vistor;

            SPARQLParser::DeleteClauseContext *tree = parser.deleteClause();
            vistor.visitDeleteClause(tree);
            SLOG_DEBUG("deleteClause is valid: " << sparql);
            return true;
        }
        catch (const runtime_error &e1)
        {
            msg = e1.what();
            SLOG_DEBUG("deleteClause is invalid: " << sparql);
            SLOG_DEBUG(msg);
            return false;
        }
    }

    

    bool SparqlUtil::validateDeleteWhere(const std::string &sparql, std::string& msg)
    {
        try
        {
            SyntaxErrorListener lstnr;

            antlr4::ANTLRInputStream input(sparql);
            SPARQLLexer lexer(&input);
            lexer.removeErrorListeners();
            lexer.addErrorListener(&lstnr);

            antlr4::CommonTokenStream tokens(&lexer);
            SPARQLParser parser(&tokens);
            parser.removeErrorListeners();
            parser.addErrorListener(&lstnr);

            // Parse the query and get the parse tree
            SPARQLBaseVisitor vistor;

            SPARQLParser::DeleteWhereContext *tree = parser.deleteWhere();
            vistor.visitDeleteWhere(tree);
            SLOG_DEBUG("deleteWhere is valid: " << sparql);
            return true;
        }
        catch (const runtime_error &e1)
        {
            msg = e1.what();
            SLOG_DEBUG("deleteWhere is invalid: " << sparql);
            SLOG_DEBUG(msg);
            return false;
        }
    }
}