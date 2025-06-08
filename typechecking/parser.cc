/*
 * Created by: Joshua Elkins
 * Date: June 24th, 2023
 */

#include <iostream>
#include <string>
#include <set>
#include <map>
#include <vector>
#include "parser.h"
#include "lexer.h"
#include "inputbuf.h"
#include <cassert>
#include <algorithm>

using namespace std;

typedef enum {
	TYPE_INT, TYPE_REAL, TYPE_BOOLEAN, TYPE_UNKNOWN
} Type;

void type_error(int line_num, string constraint) {
	cout << "TYPE MISMATCH " << line_num << " " << constraint << endl;
	exit(0); // TODO
}

class Graph
{
public:
	void insertNode(string node)
	{
		if (graph.find(node) == graph.end()) {
			graph[node] = set <string>();
			typeTable[node] = TYPE_UNKNOWN;
			variableOrder.push_back(node);
		}
	}

	void dfsAllNeighbours(string node, vector <string>& out) {
		if (visited[node]) {
			return;
		}

		visited[node] = true;
		out.push_back(node);

		for (string neigh : graph[node]) {
			dfsAllNeighbours(neigh, out);
		}
	}

	vector<string> getAllNeighbours(string node) {

		visited.clear();
		vector <string> out;
		dfsAllNeighbours(node, out);

		// Sort out vector such that variables are in order they appear		
		vector <string> outInOrder;
		for (string var : variableOrder) {
			if (find(out.begin(), out.end(), var) != out.end()) {
				outInOrder.push_back(var);
			}
		}

		return outInOrder;
	}

	void mergeNodes(string node1, string node2, int line_num, string const& constraint)
	{
		Type v1 = typeTable[node1];
		Type v2 = typeTable[node2];

		if (v1 == v2) {
			graph[node1].insert(node2);
			graph[node2].insert(node1);
		}
		else if (v1 == TYPE_UNKNOWN) {
			setValue(node1, v2, line_num, constraint);
			graph[node1].insert(node2);
			graph[node2].insert(node1);
		}
		else if (v2 == TYPE_UNKNOWN) {
			setValue(node2, v1, line_num, constraint);
			graph[node1].insert(node2);
			graph[node2].insert(node1);
		}
		else {
			type_error(line_num, constraint);
		}
	}

	void setValueDfs(string node, Type type, int line_num, string const &constraint)
	{
		if (visited[node]) {
			return;
		}

		visited[node] = true;


		if (typeTable[node] == type) {
			// pass
		}
		else if (typeTable[node] == TYPE_UNKNOWN) {
			typeTable[node] = type;
		}
		else {
			type_error(line_num, constraint);
		}

		for (string neigh : graph[node]) {
			setValueDfs(neigh, type, line_num, constraint);
		}
	}

	void setValue(string node, Type type, int line_num, string const &constraint)
	{
		visited.clear();
		setValueDfs(node, type, line_num, constraint);
	}

	map <string, bool> visited;
	map <string, Type> typeTable;
	map <string, set<string>> graph;
	vector <string> variableOrder;
};

Graph graph;
vector <string> tmp;
TokenType expressionResult;
string expressionNode;

// Syntax Error Function.
void syntax_error(){
	cout << "Syntax Error\n";
	exit(1);
}

/*
 * Completed Function.
 * Entry point to the program.
 */
int Parser::parse_program(){
	#ifdef DEBUG
		cout << "Entered Parse Program" << endl;
	#endif
	token = lexer.GetToken();
	if(token.token_type == ID){
		lexer.UngetToken(token);
		parse_globalVars();
		parse_body();
	}
	else if(token.token_type == LBRACE){
		lexer.UngetToken(token);
		parse_body();
	}
	else{
		syntax_error();
	}

	return 0;
}

/*
 * Completed Function.
 * Acts as basic entry into the global variable list. 
 */
int Parser::parse_globalVars(){
	#ifdef DEBUG
		cout << "Entered Parse Global Variables" << endl;
	#endif
	parse_vardecllist();

	return 0;
}

/*
 * Completed
 * Loops our variable declaration list. 
 */
int Parser::parse_vardecllist(){
	#ifdef DEBUG
		cout << "Entered Parse Variable Declaration List" << endl;
	#endif
	token = lexer.GetToken();
	while(token.token_type == ID){
		lexer.UngetToken(token);
		parse_vardecl();
		token = lexer.GetToken();
	}
	lexer.UngetToken(token);
	return 0;
}

/*
 * Completed Function
 * Acts as a method to handle the declaration statements.
 */
int Parser::parse_vardecl(){
	#ifdef DEBUG
		cout << "Entered Parse Variable Declaration" << endl;
	#endif
	token = lexer.GetToken();
	if(token.token_type != ID){
		syntax_error();
	}
	lexer.UngetToken(token);
	parse_varlist();
	token = lexer.GetToken();
	if(token.token_type != COLON){
		syntax_error();
	}
	token = lexer.GetToken();
	if(token.token_type == INT || token.token_type == REAL || token.token_type == BOO){
		lexer.UngetToken(token);
		parse_typename();
		token = lexer.GetToken();
		if(token.token_type != SEMICOLON){
			syntax_error();
		}
	}
	else{
		syntax_error();
	}

	return 0;
}

/*
 * Completed Function
 * Acts as the gathering function for our variables
 */
int Parser::parse_varlist(){
	#ifdef DEBUG
		cout << "Entered Parse Variable List" << endl;
	#endif
	token = lexer.GetToken();
	if(token.token_type != ID) {
		syntax_error();
	}
	else{
		// TOKEN
		tmp.push_back(token.lexeme);

		Token t2 = lexer.GetToken();
		if(t2.token_type == COMMA){

			while(token.token_type == ID && t2.token_type == COMMA){
				// Gather ID token info here
				token = lexer.GetToken();
				// TOKEN
				tmp.push_back(token.lexeme);

				if(token.token_type != ID){
					syntax_error();
				}
				t2 = lexer.GetToken();
			}
			lexer.UngetToken(t2);
		}
		else{
			// Gather singular ID token info here
			lexer.UngetToken(t2);
		}
	}

	return 0;
}

/*
 * Completed Function
 * Just consumes the INT, REAL, or BOO tokens
 */
int Parser::parse_typename(){
	#ifdef DEBUG
		cout << "Entered Parse Type Name" << endl;
	#endif
	token = lexer.GetToken();

	Type type = TYPE_UNKNOWN;

	if(token.token_type == INT){
		type = TYPE_INT;
	}
	else if(token.token_type == REAL){
		type = TYPE_REAL;
	}
	else if(token.token_type == BOO){
		type = TYPE_BOOLEAN;
	}
	else{
		syntax_error();
	}

	for (int i = 0; i < (int)tmp.size(); ++i) {
		graph.insertNode(tmp[i]);
		graph.setValue(tmp[i], type, token.line_no, "C1");
	}

	tmp.clear();

	return 0;
}

/*
 * Completed Function
 * Acts as the method to consume braces and enter statement list
 */
int Parser::parse_body(){
	#ifdef DEBUG
		cout << "Entered Parse Body" << endl;
	#endif
	token = lexer.GetToken();
	if(token.token_type == LBRACE){
		parse_stmtlist();
		token = lexer.GetToken();
		if(token.token_type != RBRACE){
			syntax_error();
		}
	}
	else{
		syntax_error();
	}

	return 0;
}

/*
 * Completed Function
 * Acts as our looper to enter all our statements
 */
int Parser::parse_stmtlist(){
	#ifdef DEBUG
		cout << "Entered Parse Statement List" << endl;
	#endif
	token = lexer.GetToken();
	while(token.token_type == ID || token.token_type == IF || token.token_type == WHILE || token.token_type == SWITCH){
		lexer.UngetToken(token);
		parse_stmt();
		token = lexer.GetToken();
	}
	lexer.UngetToken(token);

	return 0;
}

/*
 * Completed Function
 * Acts as our method to enter the specific statements
 */
int Parser::parse_stmt(){
	#ifdef DEBUG
		cout << "Entered Parse Statement" << endl;
	#endif
	token = lexer.GetToken();
	if(token.token_type == ID){
		lexer.UngetToken(token);
		parse_assstmt();
	}
	else if(token.token_type == IF){
		lexer.UngetToken(token);
		parse_ifstmt();
	}
	else if(token.token_type == WHILE){
		lexer.UngetToken(token);
		parse_whilestmt();
	}
	else if(token.token_type == SWITCH){
		lexer.UngetToken(token);
		parse_switchstmt();
	}
	else{
		syntax_error();
	}

	return 0;
}

/*
 * Function Completed
 * Acts as our assignment statement parser
 */
int Parser::parse_assstmt(){
	#ifdef DEBUG
		cout << "Entered Parse Assignment Statement" << endl;
	#endif

	token = lexer.GetToken();
	if(token.token_type != ID){
		syntax_error();
	}

	string lhs = token.lexeme;
	graph.insertNode(lhs);

	token = lexer.GetToken();
	if(token.token_type != EQUAL){
		syntax_error();
	}

	// EXPRESSION
	parse_expression();

	// ID, NUM, REAL, BOOL
	if (expressionResult == ID) {
		graph.mergeNodes(lhs, expressionNode, token.line_no, "C1");
	}
	else if (expressionResult == NUM) {
		graph.setValue(lhs, TYPE_INT, token.line_no, "C1");
	}
	else if (expressionResult == REALNUM) {
		graph.setValue(lhs, TYPE_REAL, token.line_no, "C1");
	}
	else if (expressionResult == TR || expressionResult == FA) {
		graph.setValue(lhs, TYPE_BOOLEAN, token.line_no, "C1");		
	}
	else {
		cout << expressionResult << endl;
		cout << "ASSIGNMENT STATEMENT ERROR\n";
	}
	

	token = lexer.GetToken();
	if(token.token_type != SEMICOLON){
		syntax_error();
	}

	return 0;
}


Type convert(TokenType type) {
	switch (type) {
		case NUM:
			return TYPE_INT;
		case REALNUM:
			return TYPE_REAL;
		case TR:
			return TYPE_BOOLEAN;
		case FA:
			return TYPE_BOOLEAN;
		default:
			assert(0);
	}
}

/*
 * Completed Function
 * Acts as our expression handling.
 */
int Parser::parse_expression(){
	#ifdef DEBUG
		cout << "Entered Parse Expression" << endl;
	#endif
	token = lexer.GetToken();
	if(token.token_type == NOT){
		auto tmpToken = token;

		lexer.UngetToken(token);
		parse_unaryOperator();
		parse_expression();

		if (expressionResult == ID) {
			graph.setValue(expressionNode, TYPE_BOOLEAN, tmpToken.line_no, "C3");

		}
		else {
			if (expressionResult != TR && expressionResult != FA) {
				type_error(tmpToken.line_no, "C3");
			}
		}
		// TYPE BOOL
		// if Node, set it to bool
		// or else check if bool
	}
	else if(token.token_type == PLUS || token.token_type == MINUS || token.token_type == MULT ||  token.token_type == DIV 
			|| token.token_type == GREATER || token.token_type == LESS || token.token_type == GTEQ || token.token_type == LTEQ || token.token_type == EQUAL || token.token_type == NOTEQUAL){
		
		auto tmpToken = token;
		lexer.UngetToken(token);

		parse_binaryOperator();

		parse_expression();
		auto expressionResult1 = expressionResult;
		auto expressionNode1 = expressionNode;

		parse_expression();
		auto expressionResult2 = expressionResult;
		auto expressionNode2 = expressionNode;

		// TYPES MUST BE SAME
		// Possible types = ID (NODE), NUM, REAL, BOOL.
		// NODE, NODE -> Make edge
		// NODE, NUM/REAL/BOOL -> Set it to NUM/REAL/BOOL
		// NUM/NUM, REAL/REAL, BOOL/BOOL -> NUM/REAL/BOOL

		if (expressionResult1 == ID && expressionResult2 == ID) {
			graph.mergeNodes(expressionNode1, expressionNode2, tmpToken.line_no, "C2");
			expressionResult = ID;
			expressionNode = expressionNode1;
		}
		else if (expressionResult1 == ID && (expressionResult2 == NUM || expressionResult2 == REALNUM || expressionResult2 == TR || expressionResult2 == FA)) {
			graph.setValue(expressionNode1, convert(expressionResult2), tmpToken.line_no, "C2");
			expressionResult = ID;
			expressionNode = expressionNode1;
		}
		else if (expressionResult2 == ID && (expressionResult1 == NUM || expressionResult1 == REALNUM || expressionResult1 == TR || expressionResult1 == FA)) {
			graph.setValue(expressionNode2, convert(expressionResult1), tmpToken.line_no, "C2");
			expressionResult = ID;
			expressionNode = expressionNode2;
		}
		else if (expressionResult1 == expressionResult2
					|| (expressionResult1 == TR && expressionResult2 == FA)
					|| (expressionResult1 == FA && expressionResult2 == TR)) {

			expressionResult = expressionResult1;
		}
		else {
			type_error(tmpToken.line_no, "C2");
		}

		if (tmpToken.token_type == GREATER || tmpToken.token_type == LESS || tmpToken.token_type == GTEQ || tmpToken.token_type == LTEQ || tmpToken.token_type == EQUAL || tmpToken.token_type == NOTEQUAL) {
			expressionResult = TR;			
		}
	}
	else if(token.token_type == ID || token.token_type == NUM || token.token_type == REALNUM || token.token_type == TR || token.token_type == FA){
		lexer.UngetToken(token);
		parse_primary();
		// NODE, NUM, REAL, BOOL
	}
	else{
		syntax_error();
	}

	return 0;
}

/*
 * Completed Function
 * Gets our NOT token
 */
int Parser::parse_unaryOperator(){
	#ifdef DEBUG
		cout << "Entered Parse Unary Operator" << endl;
	#endif
	token = lexer.GetToken();
	if(token.token_type != NOT){
		syntax_error();
	}
	//Do something with the NOT

	return 0;
}

/*
 * Completed Function
 * Acts as our binary handler
 */
int Parser::parse_binaryOperator(){
	#ifdef DEBUG
		cout << "Entered Binary Operator" << endl;
	#endif
	token = lexer.GetToken();
	if(token.token_type == PLUS || token.token_type == MINUS || token.token_type == MULT ||  token.token_type == DIV){
		// Do something with these Tokens
	}
	else if(token.token_type == GREATER || token.token_type == LESS || token.token_type == GTEQ || token.token_type == LTEQ || token.token_type == EQUAL || token.token_type == NOTEQUAL){
		// Do something with these Tokens
	}
	else{
		syntax_error();
	}

	return 0;
}




/*
 * Completed Function
 * Acts as our primary handler
 */
int Parser::parse_primary(){
	#ifdef DEBUG
		cout << "Entered Parse Primary" << endl;
	#endif
	token = lexer.GetToken();
	if(token.token_type == ID || token.token_type == NUM || token.token_type == REALNUM || token.token_type == TR || token.token_type == FA){
		// Do something with these Tokens
		if (token.token_type == ID) {
			graph.insertNode(token.lexeme);
			expressionResult = ID;
			expressionNode = token.lexeme;
		}
		else {
			expressionResult = token.token_type;
		}
	}
	else{
		syntax_error();
	}

	return 0;
}

/*
 * Completed Function
 * Acts as our If Statement handler
 */
int Parser::parse_ifstmt(){
	#ifdef DEBUG
		cout << "Entered Parse If Statement" << endl;
	#endif

	
	token = lexer.GetToken();
	Token tmpToken = token;

	if(token.token_type != IF){
		syntax_error();
	}
	token = lexer.GetToken();
	if(token.token_type != LPAREN){
		syntax_error();
	}

	parse_expression();
	// must be bool
	if (expressionResult == ID) {
		graph.setValue(expressionNode, TYPE_BOOLEAN, tmpToken.line_no, "C4");
	}
	else if (expressionResult == TR || expressionResult == FA) {
		// pass
	}
	else {
		type_error(tmpToken.line_no, "C4");
	}

	token = lexer.GetToken();
	if(token.token_type != RPAREN){
		syntax_error();
	}
	parse_body();

	return 0;
}

/*
 * Completed Function
 * Acts as our While Statement handler
 */
int Parser::parse_whilestmt(){
	#ifdef DEBUG
		cout << "Entered Parse While Statement" << endl;
	#endif

	token = lexer.GetToken();
	Token tmpToken = token;

	if(token.token_type != WHILE){
		syntax_error();
	}
	token = lexer.GetToken();
	if(token.token_type != LPAREN){
		syntax_error();
	}
	
	parse_expression();
	// must be bool
	if (expressionResult == ID) {
		graph.setValue(expressionNode, TYPE_BOOLEAN, tmpToken.line_no, "C4");
	}
	else if (expressionResult == TR || expressionResult == FA) {
		// pass
	}
	else {
		type_error(tmpToken.line_no, "C4");
	}

	token = lexer.GetToken();
	if(token.token_type != RPAREN){
		syntax_error();
	}
	parse_body();

	return 0;
}

/*
 * Completed Function
 * Acts as out Switch Statement handler
 */
int Parser::parse_switchstmt(){
	#ifdef DEBUG
		cout << "Entered Switch Statement" << endl;
	#endif
	token = lexer.GetToken();
	Token tmpToken = token;

	if(token.token_type != SWITCH){
		syntax_error();
	}
	token = lexer.GetToken();
	if(token.token_type != LPAREN){
		syntax_error();
	}

	parse_expression();
	if (expressionResult == ID) {
		graph.setValue(expressionNode, TYPE_INT, tmpToken.line_no, "C5");
	}
	else if (expressionResult == INT) {
		// pass 
	}
	else {
		type_error(tmpToken.line_no, "C5");
	}
	
	token = lexer.GetToken();
	if(token.token_type != RPAREN){
		syntax_error();
	}
	token = lexer.GetToken();
	if(token.token_type != LBRACE){
		syntax_error();
	}
	parse_caselist();
	token = lexer.GetToken();
	if(token.token_type != RBRACE){
		syntax_error();
	}

	return 0;
}

/*
 *
 */
int Parser::parse_caselist(){
	#ifdef DEBUG
		cout << "Entered Parse Case List" << endl;
	#endif
	token = lexer.GetToken();
	if(token.token_type == CASE){
		while(token.token_type == CASE){
			lexer.UngetToken(token);
			parse_case();
			token = lexer.GetToken();
		}
		lexer.UngetToken(token);
	}
	else{
		syntax_error();
	}

	return 0;
}

int Parser::parse_case(){
	#ifdef DEBUG
		cout << "Entered Parse Case" << endl;
	#endif
	token = lexer.GetToken();
	if(token.token_type != CASE){
		syntax_error();
	}
	token = lexer.GetToken();
	if(token.token_type != NUM){
		syntax_error();
	}
	// Do something with this
	token = lexer.GetToken();
	if(token.token_type != COLON){
		syntax_error();
	}
	parse_body();

	return 0;
}

void printUnknown(vector <string> const& unknownVariables) {
	for (int i = 0; i < (int)unknownVariables.size(); ++i) {
		if (i) {
			cout << ", ";
		}
		cout << unknownVariables[i];
	}
	cout << ": ? #\n";
}

int main(){
	#ifdef DEBUG
		cout << "Entered Main" << endl;
	#endif

	int i;
    	Parser* parseProgram = new Parser();
    	i = parseProgram->parse_program();
	(void)i;

	map <string, bool> printed;

	for (string variable : graph.variableOrder) {

		if (printed[variable]) {
			continue;
		}

		if (graph.typeTable[variable] == TYPE_UNKNOWN) {
			vector <string> unknown = graph.getAllNeighbours(variable);
			printUnknown(unknown);
			for (string v : unknown) {
				printed[v] = true;
			}
		}
		else {

			printed[variable] = true;

			cout << variable << ": ";
			Type type = graph.typeTable[variable];
			if (type == TYPE_INT) {
				cout << "int";
			}
			else if (type == TYPE_REAL) {
				cout << "real";
			}
			else if (type == TYPE_BOOLEAN) {
				cout << "bool";
			}
			else if (type == TYPE_UNKNOWN) {
				cout << "?";
			}
			else {
				cout << "ERROR";
			}
			cout << " #" << endl;	
		}
	}

	// vector <string> unknown;
	// for (string variable : graph.variableOrder) {
	// 	if (graph.typeTable[variable] == TYPE_UNKNOWN) {
	// 		if (unknown.empty() == false) {
	// 			if (graph.areConnected(unknown.back(), variable)) {
	// 				unknown.push_back(variable);
	// 			}
	// 			else {
	// 				printUnknown(unknown);
	// 				unknown.clear();
	// 				unknown.push_back(variable);
	// 			}
	// 		}
	// 		else {
	// 			unknown.push_back(variable);
	// 		}
	// 	}
	// 	else {

	// 		if (unknown.empty() == false) {
	// 			printUnknown(unknown);
	// 			unknown.clear();
	// 		}

	// 		cout << variable << ": ";
	// 		Type type = graph.typeTable[variable];
	// 		if (type == TYPE_INT) {
	// 			cout << "int";
	// 		}
	// 		else if (type == TYPE_REAL) {
	// 			cout << "real";
	// 		}
	// 		else if (type == TYPE_BOOLEAN) {
	// 			cout << "bool";
	// 		}
	// 		else if (type == TYPE_UNKNOWN) {
	// 			cout << "?";
	// 		}
	// 		else {
	// 			cout << "ERROR";
	// 		}
	// 		cout << " #" << endl;
	// 	}
	// }

	// if (unknown.empty() == false) {
	// 	printUnknown(unknown);
	// }

	return 0; 
}
