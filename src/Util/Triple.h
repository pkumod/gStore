/*
 * Triple.h
 *
 *  Created on: 2014-6-20
 *      Author: liyouhuan
 */

#ifndef _UTIL_TRIPLE_H
#define _UTIL_TRIPLE_H

#include "Util.h"

using namespace std;

// helper functions for TripleTerm (RDF 1.2)
// canonical string format: <<( <subject> <predicate> <object> )>>
inline string makeTripleTermString(const string& subject, const string& predicate, const string& object)
{
	return "<<( " + subject + " " + predicate + " " + object + " )>>";
}

inline bool isTripleTermString(const string& str)
{
	return str.length() >= 3 && str[0] == '<' && str[1] == '<' && str[2] == '(';
}

// Parse a canonical triple term string into its three components.
// Format: <<( <subject> <predicate> <object> )>>
// Returns false if the string is not in the expected format.
inline bool parseTripleTermString(const string& termStr, string& subject, string& predicate, string& object)
{
	if (!isTripleTermString(termStr))
		return false;

	// Extract inner content: skip "<<( " and trailing " )>>"
	if (termStr.length() < 10) return false;
	string inner = termStr.substr(4, termStr.length() - 8);

	// Parse first component (subject)
	size_t pos = 0;
	size_t next;

	if (inner[pos] == '<')
	{
		next = inner.find('>', pos);
		if (next == string::npos) return false;
		subject = inner.substr(pos, next - pos + 1);
		pos = next + 1;
	}
	else if (inner[pos] == '"')
	{
		next = inner.find("\"", pos + 1);
		if (next == string::npos) return false;
		subject = inner.substr(pos, next - pos + 1);
		pos = next + 1;
	}
	else if (inner[pos] == '_')
	{
		next = inner.find(' ', pos);
		if (next == string::npos) next = inner.length();
		subject = inner.substr(pos, next - pos);
		pos = next;
	}
	else
	{
		return false;
	}

	// Skip whitespace
	while (pos < inner.length() && inner[pos] == ' ') pos++;

	// Parse second component (predicate) - must be IRI
	if (inner[pos] == '<')
	{
		next = inner.find('>', pos);
		if (next == string::npos) return false;
		predicate = inner.substr(pos, next - pos + 1);
		pos = next + 1;
	}
	else
	{
		return false;
	}

	// Skip whitespace
	while (pos < inner.length() && inner[pos] == ' ') pos++;

	// Parse third component (object)
	if (inner[pos] == '<')
	{
		// Could be nested triple term or regular IRI
		if (pos + 2 < inner.length() && inner[pos+1] == '<' && inner[pos+2] == '(')
		{
			// Nested triple term - find matching )>>
			size_t depth = 1;
			next = pos + 3;
			while (next < inner.length() && depth > 0)
			{
				if (next + 2 < inner.length() && inner[next] == ')' && inner[next+1] == '>' && inner[next+2] == '>')
				{
					depth--;
					if (depth == 0) { next += 3; break; }
					next += 3;
				}
				else if (next + 2 < inner.length() && inner[next] == '<' && inner[next+1] == '<' && inner[next+2] == '(')
				{
					depth++;
					next += 3;
				}
				else
				{
					next++;
				}
			}
			if (depth != 0) return false;
			object = inner.substr(pos, next - pos);
		}
		else
		{
			next = inner.find('>', pos);
			if (next == string::npos) return false;
			object = inner.substr(pos, next - pos + 1);
		}
	}
	else if (inner[pos] == '"')
	{
		next = inner.find("\"", pos + 1);
		if (next == string::npos) return false;
		size_t after = next + 1;
		if (after < inner.length() && inner[after] == '^')
		{
			after += 2;
			if (inner[after] == '<')
			{
				after = inner.find('>', after);
				if (after == string::npos) return false;
				after++;
			}
			else
			{
				while (after < inner.length() && inner[after] != ' ' && inner[after] != ')')
					after++;
			}
		}
		else if (after < inner.length() && inner[after] == '@')
		{
			after++;
			while (after < inner.length() && inner[after] != ' ' && inner[after] != ')')
				after++;
		}
		object = inner.substr(pos, after - pos);
	}
	else if (inner[pos] == '_')
	{
		next = inner.find(' ', pos);
		if (next == string::npos) next = inner.length();
		object = inner.substr(pos, next - pos);
	}
	else
	{
		return false;
	}

	return true;
}

class Triple{
public:
	string subject;
	string predicate;
	string object;

	Triple();

	/* _line responds to a triple
	 * It's OK if _line ends up with '.' or not
	 * tuples separated by '\t'
	 *  */
	Triple(string _line);
	Triple(const string& _s, const string& _p, const string& _o);
	Triple(const Triple& _triple);
	Triple& operator=(const Triple& _triple);

	void setSubject(const string& _s);
	void setPredicate(const string& _p);
	void setObject(const string& _o);

	const string& getSubject()const;
	const string& getPredicate()const;
	const string& getObject()const;

	const string toString()const;
};

class TripleWithObjType : public Triple
{
public:
	enum ObjectType {None, Entity, Literal};
	ObjectType object_type;

    TripleWithObjType();
    TripleWithObjType(const string& _s, const string& _p, const string& _o, const ObjectType _o_type = None);
    TripleWithObjType(const TripleWithObjType& _triple_with_objtype);

    TripleWithObjType& operator=(const TripleWithObjType& _triple_with_objtype);

    void setObjType(const ObjectType _o_type);

    bool isObjEntity()const;
    bool isObjLiteral()const;

    const string toString()const;
};

#endif //_UTIL_TRIPLE_H
