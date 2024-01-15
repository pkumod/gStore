// This source file was originally from:
// Taken from https://github.com/friedmud/variadic_table/blob/master/include/VariadicTable.h
//
// Copyright (c) 2018, Derek Gaston.
// All rights reserved.
#ifndef _UTIL_PRETTY_PRINT_H
#define _UTIL_PRETTY_PRINT_H

#include <iostream>
#include <iomanip>
#include <ios>
#include <vector>
#include <tuple>
#include <type_traits>
#include <cassert>
#include <cmath>
#include <algorithm>
#include <limits>

/**
 * A class for "pretty printing" a table of data.
 *
 * Requries C++11 (and nothing more)
 *
 * It's templated on the types that will be in each column
 * (all values in a column must have the same type)
 *
 * For instance, to use it with data that looks like:  "Fred", "193.4", "35", "Sam"
 * with header names: "Name", "Weight", "Age", "Brother"
 *
 * You would invoke the table like so:
 * PrettyPrint vt({"Name", "Weight", "Age", "Brother"});
 *
 * Then add the data to the table:
 * vt.addRow({"Fred", "193.4", "35", "Sam"});
 *
 * And finally print it:
 * vt.print(std::cout);
 */
class PrettyPrint
{
public:

  /**
   * Construct the table with headers
   *
   * @param headers The names of the columns
   * @param static_column_size The size of columns that can't be found automatically
   */
  PrettyPrint(std::vector<std::string> headers,
                unsigned int static_column_size = 0,
                unsigned int cell_padding = 1,
                unsigned int max_column_size = 100)
    : _headers(headers),
      _num_columns(headers.size()),
      _static_column_size(static_column_size),
      _cell_padding(cell_padding),
      _max_column_size(max_column_size)
  {

  }

  /**
   * Add a row of data
   *
   * Easiest to use like:
   * table.addRow({data1, data2, data3});
   *
   * @param data A Tuple of data to add
   */
  void addRow(std::vector<std::string> row) { _data.emplace_back(row); }

  /**
   * Pretty print the table of data
   */
  template <typename StreamType>
  void print(StreamType & stream)
  {
    size_columns();

    // Start computing the total width
    // First - we will have _num_columns + 1 "|" characters
    unsigned int total_width = _num_columns + 1;

    // Now add in the size of each colum
    for (auto & col_size : _column_sizes)
      total_width += col_size + (2 * _cell_padding);

    // Print out the top line
    stream << std::string(total_width, '-') << "\n";

    // Print out the headers
    stream << "|";
    for (unsigned int i = 0; i < _num_columns; i++)
    {
      // Must find the center of the column
      auto half = _column_sizes[i] / 2;
      half -= _headers[i].size() / 2;

      stream << std::string(_cell_padding, ' ') << std::setw(_column_sizes[i]) << std::left
             << std::string(half, ' ') + _headers[i] << std::string(_cell_padding, ' ') << "|";
    }

    stream << "\n";

    // Print out the line below the header
    stream << std::string(total_width, '-') << "\n";

    // Now print the rows of the table
    for (auto & row : _data)
    {
      stream << "|";
      print_each(std::forward<std::vector<std::string>>(row), stream);
      stream << "\n";
    }

    // Print out the line below the header
    stream << std::string(total_width, '-') << "\n";
  }

protected:
   /**
   * These three functions print out each item in a Tuple into the table
   *
   * Original Idea From From https://stackoverflow.com/a/26908596
   *
   * BTW: This would all be a lot easier with generic lambdas
   * there would only need to be one of this sequence and then
   * you could pass in a generic lambda.  Unfortunately, that's C++14
   */

  /**
   * This gets called on each item
   */
  template <typename StreamType>
  void print_each(std::vector<std::string> && t, StreamType & stream)
  {
    for (size_t i = 0; i < t.size(); i++)
    {
      auto & val = t[i];
      stream << std::string(_cell_padding, ' ') << std::setw(_column_sizes[i])
            << std::left << val << std::string(_cell_padding, ' ') << "|";
    }
  }


  /**
   * The function that is actually called that starts the recursion
   */
  void size_each(std::vector<std::string> && t, std::vector<size_t> & sizes)
  {
    for (size_t i = 0; i < t.size(); i++)
    {
      sizes[i] = std::min(t[i].size(), _max_column_size);
    }
  }

  /**
   * Finds the size each column should be and set it in _column_sizes
   */
  void size_columns()
  {
    _column_sizes.resize(_num_columns);

    // Temporary for querying each row
    std::vector<size_t> column_sizes(_num_columns);

    // Start with the size of the headers
    for (unsigned int i = 0; i < _num_columns; i++)
      _column_sizes[i] = _headers[i].size();

    // Grab the size of each entry of each row and see if it's bigger
    for (std::vector<std::string> & row : _data)
    {
      size_each(std::forward<std::vector<std::string>>(row), column_sizes);

      for (unsigned int i = 0; i < _num_columns; i++)
        _column_sizes[i] = std::max(_column_sizes[i], column_sizes[i]);
    }
  }

  /// The column headers
  std::vector<std::string> _headers;

  /// Number of columns in the table
  unsigned int _num_columns;

  /// Size of columns that we can't get the size of
  unsigned int _static_column_size;

  /// Size of the cell padding
  unsigned int _cell_padding;

  /// Max size of columns
  long unsigned int _max_column_size;

  /// The actual data
  std::vector<std::vector<std::string>> _data;

  /// Holds the printable width of each column
  std::vector<size_t> _column_sizes;
};
#endif // _UTIL_PRETTY_PRINT_H