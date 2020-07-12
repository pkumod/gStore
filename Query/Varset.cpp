/*=============================================================================
# Filename: Varset.cpp
# Author: Jiaqi, Chen
# Mail: chenjiaqi93@163.com
# Last Modified: 2017-03-10
# Description: implement functions in Varset.h
=============================================================================*/

#include "Varset.h"

using namespace std;

Varset::Varset(const string& _var)
{
  this->addVar(_var);
}

Varset::Varset(const vector<string>& _vars)
{
  for (int i = 0; i < (int)_vars.size(); i++)
    this->addVar(_vars[i]);
}

bool Varset::empty() const
{
  return this->vars.empty();
}

int Varset::getVarsetSize() const
{
  return this->vars.size();
}

bool Varset::findVar(const string& _var) const
{
  if (this->vars.empty())
    return false;

  for (int i = 0; i < (int)this->vars.size(); i++)
    if (this->vars[i] == _var)
      return true;

  return false;
}

void Varset::addVar(const string& _var)
{
  if (!this->findVar(_var))
    this->vars.push_back(_var);
}

Varset Varset::operator+(const Varset& _varset) const
{
  Varset r(*this);

  for (int i = 0; i < (int)_varset.vars.size(); i++)
    r.addVar(_varset.vars[i]);

  return r;
};

Varset& Varset::operator+=(const Varset& _varset)
{
  for (int i = 0; i < (int)_varset.vars.size(); i++)
    this->addVar(_varset.vars[i]);

  return *this;
}

Varset Varset::operator*(const Varset& _varset) const
{
  Varset r;

  for (int i = 0; i < (int)this->vars.size(); i++)
    if (_varset.findVar(this->vars[i]))
      r.addVar(this->vars[i]);

  return r;
}

Varset Varset::operator-(const Varset& _varset) const
{
  Varset r;

  for (int i = 0; i < (int)this->vars.size(); i++)
    if (!_varset.findVar(this->vars[i]))
      r.addVar(this->vars[i]);

  return r;
}

bool Varset::operator==(const Varset& _varset) const
{
  if ((int)this->vars.size() != (int)_varset.vars.size())
    return false;

  for (int i = 0; i < (int)this->vars.size(); i++)
    if (!_varset.findVar(this->vars[i]))
      return false;

  return true;
}

bool Varset::hasCommonVar(const Varset& _varset) const
{
  for (int i = 0; i < (int)this->vars.size(); i++)
    if (_varset.findVar(this->vars[i]))
      return true;

  return false;
}

bool Varset::belongTo(const Varset& _varset) const
{
  for (int i = 0; i < (int)this->vars.size(); i++)
    if (!_varset.findVar(this->vars[i]))
      return false;

  return true;
}

vector<int> Varset::mapTo(const Varset& _varset) const
{
  vector<int> r;

  for (int i = 0; i < (int)this->vars.size(); i++) {
    r.push_back(-1);
    for (int j = 0; j < (int)_varset.vars.size(); j++)
      if (this->vars[i] == _varset.vars[j])
        r[i] = j;
  }

  return r;
}

void Varset::print() const
{
  printf("Varset: ");

  for (int i = 0; i < (int)this->vars.size(); i++) {
    printf("%s ", this->vars[i].c_str());
  }

  printf("\n");
}
