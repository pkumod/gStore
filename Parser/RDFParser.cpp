#include "RDFParser.h"
/*	if you want to parse a triple file, you need to create a RDFParser object
*	and run parseFile several times until all the triple is processed
*	for example:
*		RDFParser _RDFParser(filename);
*		triple_array = new Triple_with_objtype[RDFParser::RDFParser::TRIPLE_NUM_PER_GROUP];
*		while (true)
*		{
*			triple_num = 0;
*			_RDFParser.parseFile();
*			if (triple_num == 0)	break;
*			......
*		}		
*/
string RDFParser::parseFile(TripleWithObjType* _triple_array, int& _triple_num)
{
	string _subject, _predicate, _object, _objectSubType;
	Type::Type_ID _objectType;

	while (_triple_num < RDFParser::TRIPLE_NUM_PER_GROUP)
	{
		try
		{
			if (!this->_TurtleParser.parse(_subject, _predicate, _object, _objectType, _objectSubType))		break;
		}
		catch (const TurtleParser::Exception& _e) 
		{
			cerr << _e.message << endl;
			this->_TurtleParser.discardLine();
			continue;
		}

		_subject = "<" + _subject + ">";
		_predicate = "<" + _predicate + ">";

		char _objectTypec;
		if (_objectType == Type::Type_URI)
		{
			_object = "<" + _object + ">";
			_objectTypec = 'e';
		}
		else if (_objectType == Type::Type_CustomLanguage)
		{
			_object = "\"" + _object + "\"@" + _objectSubType;
			_objectTypec = 'l';
		}
		else 
		{
			_object = "\"" + _object + "\"";
			_objectTypec = 'l';
		}

		_triple_array[_triple_num++] = TripleWithObjType(_subject, _predicate, _object, _objectTypec);
	}
	return "";
}

/*	if you want to parse a string, you need to create a RDFParser object with no parameter, if the triple has prefix, you also need to provide it.
*	the whole string must be processed in one time of invoking parseString, pay attention to the triples in the string won't exceed the limit of RDFParser::TRIPLE_NUM_PER_GROUP.
*	for example:
*		RDFParser _RDFParser;
*		_RDFParser.parseString(prefix, ..);
*		_RDFParser.parseString(triple string 1, ..);
*		_RDFParser.parseString(triple string 2, ..);
*
*		triple string 1 & 2 will share the common prefix, if you don't want this, create a new RDFParser object.
*		_RDFParser.parseString(prefix + "\n" + triple string 1 + "\n" + triple string 2, ..); is also acceptable.
*/
string RDFParser::parseString(string _str, TripleWithObjType* _triple_array, int& _triple_num)
{
	//clear in each time invoking
	this->_sin.clear();
	this->_sin << _str;
	
	string _subject, _predicate, _object, _objectSubType;
	Type::Type_ID _objectType;

	while (_triple_num < RDFParser::TRIPLE_NUM_PER_GROUP)
	{
		try
		{
			if (!this->_TurtleParser.parse(_subject, _predicate, _object, _objectType, _objectSubType))		break;
		}
		catch (const TurtleParser::Exception& _e)
		{
			cerr << _e.message << endl;
			this->_TurtleParser.discardLine();
			continue;
		}

		_subject = "<" + _subject + ">";
		_predicate = "<" + _predicate + ">";

		char _objectTypec;
		if (_objectType == Type::Type_URI)
		{
			_object = "<" + _object + ">";
			_objectTypec = 'e';
		}
		else if (_objectType == Type::Type_CustomLanguage)
		{
			_object = "\"" + _object + "\"@" + _objectSubType;
			_objectTypec = 'l';
		}
		else
		{
			_object = "\"" + _object + "\"";
			_objectTypec = 'l';
		}

		_triple_array[_triple_num++] = TripleWithObjType(_subject, _predicate, _object, _objectTypec);
	}
	return "";
}
