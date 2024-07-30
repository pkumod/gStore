/*=============================================================================
# Filename: RDFParser.cpp
# Author: Yue Pang
# Mail: michelle.py@pku.edu.cn
# Last Modified:    2021-08-03 15:28 CST
# Description: implements the class for parsing RDF data during build based on 
RDF-3X's TurtleParser
=============================================================================*/
#include "RDFParser.h"

/** Constructor for parseString. Passes the member stringstream to 
 * TurtleParser, which can later be fed other strings containing RDF
 * data in parseString.
 */
RDFParser::RDFParser():_TurtleParser(_sin)
{}

/** Constructor for parseFile. Takes in an ifstream of an RDF file 
 * and passes to TurtleParser.
 * @param _fin the ifstream of the input RDF file.
 */
RDFParser::RDFParser(ifstream& _fin):_TurtleParser(_fin)
{}

/** Parses an RDF file. More rigorous numeric checks on top of TurtleParser.
 * If you want to parse a triple file, you need to create a RDFParser object
 * and run parseFile several times until all the triple is processed. For each
 * run, the number of parsed triples won't exceed RDFParser::TRIPLE_NUM_PER_GROUP.
 * For example:
 * RDFParser _RDFParser(filename);
 * triple_array = new Triple_with_objtype[RDFParser::RDFParser::TRIPLE_NUM_PER_GROUP];
 * while (true)
 * {
 * 	triple_num = 0;
 * 	_RDFParser.parseFile(triple_array, triple_num);
 * 	if (triple_num == 0)    break;
 * 	......
 * }       
 * 
 * @param _triple_array stores the parsed triples.
 * @param _triple_num the number of successfully parsed triples.
 * @param _error_log the file name for outputting parse error log; if left
 * blank, output will be redirected to stdout. Each line of output is in the
 * format of: Line x (subject predicate object): error message.
 */
int RDFParser::parseFile(TripleWithObjType* _triple_array, int& _triple_num, string _error_log, int init_line)
{
	string rawSubject, rawPredicate, rawObject;
	string _subject, _predicate, _object, _objectSubType;
	string errorMsg;
	Type::ID _objectType;
	ofstream ofile;

	if (!_error_log.empty() && _error_log != "NULL" )
	{
		ofile.open(_error_log,ios::app);
		if (!ofile)
			cout << "Error log file cannot be opened." << endl;

	}

	int numLines = init_line;
	int prev_line = -1;	// For recognizing errors thrown by TurtleParser in the same line of the input file
	while (_triple_num < RDFParser::TRIPLE_NUM_PER_GROUP)
	{
		numLines++;
		try
		{
			if (!this->_TurtleParser.parse(rawSubject, rawPredicate, rawObject, _objectType, _objectSubType))		break;
		}
		catch (const TurtleParser::Exception& _e)
		{
			errorMsg = _e.message;
			// Sync the line number in errorMsg, then get rid of the part before ':'
			// format: "lexer error in line 14276: unexpected character ?"
			int num_l = errorMsg.find("line") + 5;
			int num_r = errorMsg.find(':');
			int curr_line = stoi(errorMsg.substr(num_l, num_r - num_l));
			if (curr_line == prev_line)
			{
				numLines--;
				continue;
			}

			errorMsg = errorMsg.substr(0, num_l - 9) + errorMsg.substr(num_r) + '\n';

			// TODO: get the actual corresponding line
			if (ofile)
				ofile << "Line " << numLines << " (<" << rawSubject << "> <" << rawPredicate << "> <"<< rawObject << ">): " << errorMsg;
			else
				cout << "Line " << numLines << " (<" << rawSubject << "> <" << rawPredicate << "> <"<< rawObject << ">): " << errorMsg << endl;
			this->_TurtleParser.discardLine();
			continue;
		}

		_subject = "<" + rawSubject + ">";
		_predicate = "<" + rawPredicate + ">";

		TripleWithObjType::ObjectType _object_type = TripleWithObjType::None;
		if (_objectType == Type::URI)
		{
			_object = "<" + rawObject + ">";
			_object_type = TripleWithObjType::Entity;
		}
		else
		{
			if (_objectType == Type::Literal)
				_object = "\"" + rawObject + "\"";
			else if (_objectType == Type::CustomLanguage)
				_object = "\"" + rawObject + "\"@" + _objectSubType;
			else if (_objectType == Type::String)
				_object = "\"" + rawObject + "\"^^<http://www.w3.org/2001/XMLSchema#string>";
			else if (_objectType == Type::Integer)
			{
				try
				{
					stoll(rawObject);
				}
				catch (invalid_argument& e)
				{
					errorMsg = "Object integer value invalid";
					if (ofile)
						ofile << "Line " << numLines << " (<" << rawSubject << "> <" << rawPredicate \
							<< "> <" << rawObject << ">): " << errorMsg;
					else
						cout << "Line " << numLines << " (<" << rawSubject << "> <" << rawPredicate \
							<< "> <" << rawObject << ">): " << errorMsg << endl;
					continue;
				}
				catch (out_of_range& e)
				{
					errorMsg = "Object integer out of range";
					if (ofile)
						ofile << "Line " << numLines << " (<" << rawSubject << "> <" << rawPredicate \
							<< "> <" << rawObject << ">): " << errorMsg;
					else
						cout << "Line " << numLines << " (<" << rawSubject << "> <" << rawPredicate \
							<< "> <" << rawObject << ">): " << errorMsg << endl;
					continue;
				}
				_object = "\"" + rawObject + "\"^^<http://www.w3.org/2001/XMLSchema#integer>";
			}
			else if (_objectType == Type::Decimal)
				_object = "\"" + rawObject + "\"^^<http://www.w3.org/2001/XMLSchema#decimal>";
			else if (_objectType == Type::Double)
			{
				try
				{
					stod(rawObject);
				}
				catch (invalid_argument& e)
				{
					errorMsg = "Object double value invalid";
					if (ofile)
						ofile << "Line " << numLines << " (<" << rawSubject << "> <" << rawPredicate \
							<< "> <" << rawObject << ">): " << errorMsg;
					else
						cout << "Line " << numLines << " (<" << rawSubject << "> <" << rawPredicate \
							<< "> <" << rawObject << ">): " << errorMsg << endl;
					continue;
				}
				if (rawObject.length() == 3 && toupper(rawObject[0]) == 'N' && toupper(rawObject[1]) == 'A'
					&& toupper(rawObject[2]) == 'N')
				{
					errorMsg = "Object double value is NaN";
					if (ofile)
						ofile << "Line " << numLines << " (<" << rawSubject << "> <" << rawPredicate \
							<< "> <" << rawObject << ">): " << errorMsg;
					else
						cout << "Line " << numLines << " (<" << rawSubject << "> <" << rawPredicate \
							<< "> <" << rawObject << ">): " << errorMsg << endl;
					continue;
				}
				_object = "\"" + rawObject + "\"^^<http://www.w3.org/2001/XMLSchema#double>";
			}
			else if (_objectType == Type::Boolean)
				_object = "\"" + rawObject + "\"^^<http://www.w3.org/2001/XMLSchema#boolean>";
			else if (_objectType == Type::CustomType)
			{
				if (_objectSubType == "http://www.w3.org/2001/XMLSchema#long")
				{
					try
					{
						stoll(rawObject);
					}
					catch (invalid_argument& e)
					{
						errorMsg = "Object long value invalid";
						if (ofile)
							ofile << "Line " << numLines << " (<" << rawSubject << "> <" << rawPredicate \
								<< "> <" << rawObject << ">): " << errorMsg;
						else
							cout << "Line " << numLines << " (<" << rawSubject << "> <" << rawPredicate \
								<< "> <" << rawObject << ">): " << errorMsg << endl;
						continue;
					}
					catch (out_of_range& e)
					{
						errorMsg = "Object long value out of range";
						if (ofile)
							ofile << "Line " << numLines << " (<" << rawSubject << "> <" << rawPredicate \
								<< "> <" << rawObject << ">): " << errorMsg;
						else
							cout << "Line " << numLines << " (<" << rawSubject << "> <" << rawPredicate \
								<< "> <" << rawObject << ">): " << errorMsg << endl;
						continue;
					}
				}
				else if (_objectSubType == "http://www.w3.org/2001/XMLSchema#int")
				{
					long long ll;
					try
					{
						ll = stoll(rawObject);
					}
					catch (invalid_argument& e)
					{
						errorMsg = "Object int value invalid";
						if (ofile)
							ofile << "Line " << numLines << " (<" << rawSubject << "> <" << rawPredicate \
								<< "> <" << rawObject << ">): " << errorMsg;
						else
							cout << "Line " << numLines << " (<" << rawSubject << "> <" << rawPredicate \
							<< "> <" << rawObject << ">): " << errorMsg << endl;
						continue;
					}
					catch (out_of_range& e)
					{
						errorMsg = "Object int value out of range";
						if (ofile)
							ofile << "Line " << numLines << " (<" << rawSubject << "> <" << rawPredicate \
								<< "> <" << rawObject << ">): " << errorMsg;
						else
							cout << "Line " << numLines << " (<" << rawSubject << "> <" << rawPredicate \
								<< "> <" << rawObject << ">): " << errorMsg << endl;
						continue;
					}
					if (ll < (long long)INT_MIN || ll >(long long)INT_MAX)
					{
						errorMsg = "Object int value out of range";
						if (ofile)
							ofile << "Line " << numLines << " (<" << rawSubject << "> <" << rawPredicate \
								<< "> <" << rawObject << ">): " << errorMsg;
						else
							cout << "Line " << numLines << " (<" << rawSubject << "> <" << rawPredicate \
								<< "> <" << rawObject << ">): " << errorMsg << endl;
						continue;
					}
				}
				else if (_objectSubType == "http://www.w3.org/2001/XMLSchema#short")
				{
					long long ll;
					try
					{
						ll = stoll(rawObject);
					}
					catch (invalid_argument& e)
					{
						errorMsg = "Object short value invalid";
						if (ofile)
							ofile << "Line " << numLines << " (<" << rawSubject << "> <" << rawPredicate \
								<< "> <" << rawObject << ">): " << errorMsg;
						else
							cout << "Line " << numLines << " (<" << rawSubject << "> <" << rawPredicate \
								<< "> <" << rawObject << ">): " << errorMsg << endl;
						continue;
					}
					catch (out_of_range& e)
					{
						errorMsg = "Object short value out of range";
						if (ofile)
							ofile << "Line " << numLines << " (<" << rawSubject << "> <" << rawPredicate \
								<< "> <" << rawObject << ">): " << errorMsg;
						else
							cout << "Line " << numLines << " (<" << rawSubject << "> <" << rawPredicate \
								<< "> <" << rawObject << ">): " << errorMsg << endl;
						continue;
					}
					if (ll < (long long)SHRT_MIN || ll >(long long)SHRT_MAX)
					{
						errorMsg = "Object short value out of range";
						if (ofile)
							ofile << "Line " << numLines << " (<" << rawSubject << "> <" << rawPredicate \
								<< "> <" << rawObject << ">): " << errorMsg;
						else
							cout << "Line " << numLines << " (<" << rawSubject << "> <" << rawPredicate \
								<< "> <" << rawObject << ">): " << errorMsg << endl;
						continue;
					}
				}
				else if (_objectSubType == "http://www.w3.org/2001/XMLSchema#byte")
				{
					long long ll;
					try
					{
						ll = stoll(rawObject);
					}
					catch (invalid_argument& e)
					{
						errorMsg = "Object byte value invalid";
						if (ofile)
							ofile << "Line " << numLines << " (<" << rawSubject << "> <" << rawPredicate \
								<< "> <" << rawObject << ">): " << errorMsg;
						else
							cout << "Line " << numLines << " (<" << rawSubject << "> <" << rawPredicate \
								<< "> <" << rawObject << ">): " << errorMsg << endl;
						continue;
					}
					catch (out_of_range& e)
					{
						errorMsg = "Object byte value out of range";
						if (ofile)
							ofile << "Line " << numLines << " (<" << rawSubject << "> <" << rawPredicate \
								<< "> <" << rawObject << ">): " << errorMsg;
						else
							cout << "Line " << numLines << " (<" << rawSubject << "> <" << rawPredicate \
								<< "> <" << rawObject << ">): " << errorMsg << endl;
						continue;
					}
					if (ll < (long long)SCHAR_MIN || ll >(long long)SCHAR_MAX)
					{
						errorMsg = "Object byte value out of range";
						if (ofile)
							ofile << "Line " << numLines << " (<" << rawSubject << "> <" << rawPredicate \
								<< "> <" << rawObject << ">): " << errorMsg;
						else
							cout << "Line " << numLines << " (<" << rawSubject << "> <" << rawPredicate \
								<< "> <" << rawObject << ">): " << errorMsg << endl;
						continue;
					}
				}
				else if (_objectSubType == "http://www.w3.org/2001/XMLSchema#float")
				{
					try
					{
						stof(rawObject);
					}
					catch (invalid_argument& e)
					{
						errorMsg = "Object float value invalid";
						if (ofile)
							ofile << "Line " << numLines << " (<" << rawSubject << "> <" << rawPredicate \
								<< "> <" << rawObject << ">): " << errorMsg;
						else
							cout << "Line " << numLines << " (<" << rawSubject << "> <" << rawPredicate \
								<< "> <" << rawObject << ">): " << errorMsg << endl;
						continue;
					}
					if (rawObject.length() == 3 && toupper(rawObject[0]) == 'N' && toupper(rawObject[1]) == 'A'
						&& toupper(rawObject[2]) == 'N')
					{
						errorMsg = "Object float value is NaN";
						if (ofile)
							ofile << "Line " << numLines << " (<" << rawSubject << "> <" << rawPredicate \
								<< "> <" << rawObject << ">): " << errorMsg;
						else
							cout << "Line " << numLines << " (<" << rawSubject << "> <" << rawPredicate \
								<< "> <" << rawObject << ">): " << errorMsg << endl;
						continue;
					}
				}
				_object = "\"" + rawObject + "\"^^<" + _objectSubType + ">";
			}
			_object_type = TripleWithObjType::Literal;
		}

		_triple_array[_triple_num++] = TripleWithObjType(_subject, _predicate, _object, _object_type);

	}
	if (ofile)
	{
		ofile.close();
	}
	SLOG_CORE("RDFParser parseFile done!");

	return numLines;
}

/** Parses a string containing RDF data. Based on parseFile.
 * if you want to parse a string, you need to create a RDFParser object with 
 * no parameter, if the triple has prefix, you also need to provide it.
 * The whole string must be processed in one time of invoking parseString.
 * For example:
 * RDFParser _RDFParser;
 * _RDFParser.parseString(prefix, ..);
 * _RDFParser.parseString(triple string 1, ..);
 * _RDFParser.parseString(triple string 2, ..);
 * Note: triple string 1 & 2 will share the common prefix. If you don't 
 * want this, create a new RDFParser object.
 * _RDFParser.parseString(prefix + "\n" + triple string 1 + "\n" + \
 * triple string 2, ..); is also acceptable.
 * 
 * @param _str the input string containing RDF data.
 * @param _triple_array stores the parsed triples.
 * @param _triple_num the number of successfully parsed triples.
 */
/*	
*/
void RDFParser::parseString(string _str, TripleWithObjType* _triple_array, int& _triple_num)
{
	//clear in each time invoking
	// The same thing: just a string stream instead of a file stream.
	this->_sin.clear();
	this->_sin << _str;
	
	parseFile(_triple_array, _triple_num);
}
