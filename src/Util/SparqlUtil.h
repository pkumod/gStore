#pragma once

#include "GlobalTypedef.h"
#include "antlr4-runtime.h"
#include "../Parser/SPARQL/SPARQLLexer.h"
#include "../Parser/SPARQL/SPARQLParser.h"
#include "../Parser/SPARQL/SPARQLBaseVisitor.h"

namespace gs {
    class SyntaxErrorListener: public antlr4::BaseErrorListener
    {
    public:
        void syntaxError(antlr4::Recognizer *recognizer, antlr4::Token * offendingSymbol, \
            size_t line, size_t charPositionInLine, const std::string &msg, std::exception_ptr e);
    };


    class SparqlUtil {
    public:
        static bool validate(const std::string &sparql, std::string& msg);
        static bool validateQuery(const std::string &sparql, std::string& msg);
        static bool validateInsertData(const std::string &sparql, std::string& msg);
        static bool validateInsertClause(const std::string &sparql, std::string& msg);
        static bool validateDeleteData(const std::string &sparql, std::string& msg);
        static bool validateDeleteClause(const std::string &sparql, std::string& msg);
        static bool validateDeleteWhere(const std::string &sparql, std::string& msg);
    };
}