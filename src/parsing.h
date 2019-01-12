// Expression parsing
#pragma once

#include <algorithm>
#include <string>
#include <vector>
#include <list>
#include <map>
#include <functional>
#include <unordered_map>

using StringHasherType = std::hash<std::string>;
using StringHashType = std::hash<std::string>::result_type;

class Token
{
public:
    enum ETokenTypes
    {
        COMMA,
        CLOSEPARENS,
        OPENPARENS,
        NUMBER,
        FUNCTION,
        OPERATOR,
        UNARYOPERATOR,
        VAR,
        INVALID,
        NONE,
    };

    Token();
    Token(ETokenTypes t, const std::string token);
    Token(const Token &rhs);
    virtual ~Token();

    const ETokenTypes GetType() const;
    const std::string &GetToken() const;

protected:
    ETokenTypes type_;
    std::string token_;
};

using TokenStream = std::vector<Token>;

// Define a type for a stat mod function
struct ExpressionFunction
{
	unsigned int numargs_;
	double (*func_)(const double *, unsigned int);
};

// Function map
using FunctionMapType = std::unordered_map<std::string, ExpressionFunction>;

class Tokenizer
{
public:
    Tokenizer(const std::string expr, const FunctionMapType &fmap/*, const VarMapType &vars*/);
    bool HasNext();
    Token NextToken();

protected:
    std::string expression_;
    std::string::iterator pos_;
    Token lastToken_;
    //std::map<std::string, int> functions_;
    //std::vector<std::string> vars_;

	const FunctionMapType &functions_;
	//const VarMapType &vars_;

    bool IsValidOperator(char ch);
    bool IsDigit(char ch);
    bool IsAlphabetic(char ch);
    Token ParseNumberToken(char ch);
    Token ParseComma(char ch);
    Token ParseParentheses(bool which);
    Token ParseOperator(char ch);
    Token ParseFunctionOrVariable(char ch);
    bool IsNumeric(char ch, bool lastCharE);
    bool IsSpecialToken(const std::string &t);
    bool IsFunctionName(const std::string &t);
};

class ExpressionToPostfix
{
public:
    ExpressionToPostfix(const std::string &expr, const FunctionMapType &fmap/*, const VarMapType &vars*/);
    std::vector<Token> ToPostfix();

protected:
    std::string expr_;
    const FunctionMapType &f_;
    //const VarMapType &vars_;

    int GetNumOperands(const Token &tk);
    bool IsLeftAssociative(const Token &tk);
    int GetPrecedence(const Token &tk);
};

