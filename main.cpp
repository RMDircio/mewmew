// =======================================================
// ====================== INCLUDES =======================
// =======================================================

#include <cstdlib>
#include <cmath>
#include <fstream>
#include <iostream>
#include <string>
#include <map>
#include <vector>
#include "mewlib/mewmewLexer.h"
#include "mewlib/mewmewParser.h"
#include "mewlib/mewmewBaseVisitor.h"

// =================== END INCLUDES ======================

//========================================================
//===================== NAMESPACES =======================
//========================================================

using namespace std;
using namespace antlr4;
using namespace antlrcpp;

//=================== END NAMESPACES =====================

map<string, float> _vartable; // VARTABLE aka. Symbol Table to Hold Variables

//========================================================
//================== UTILITY FUNCTIONS ===================
//========================================================

/*
 * -------------------------------------------------------
 *  Splits a String with given delimiter
 * -------------------------------------------------------
 */
vector<string> stringsplit(string input , string delimiter){

    string token;
    vector<string> result;
    size_t pos = 0;
    
    while((pos = input.find(delimiter)) != string::npos){
        token = input.substr(0 , pos);
        result.push_back(token);
        input.erase(0 , pos + delimiter.length());
    }
    result.push_back(input);

    return result;
}

/*
 * ------------------------------------------------------
 *  Converts MewMew number to traditional number
 * ------------------------------------------------------
 */

float mew_to_float(string m){
    float n;

    if (m.find(".") != string::npos){
        vector<string> sp = stringsplit(m , ".");
        string x = "";
        x+= to_string(sp[0].size()/3);
        x+= ".";
        x+= to_string(sp[1].size()/3);
        n = stof(x);

    }else{
        n = m.size()/3;
    }

    return n;

}

// ================ END UTILITY FUNCTIONS ==================

//==========================================================
//================== ERROR HANDLER CLASS ===================
//==========================================================

class MewErr : public BaseErrorListener{

    public:
        void syntaxError(Recognizer *recognizer, Token *offendingSymbol, size_t line, size_t charPositionInLine, const std::string &msg, std::exception_ptr e) override{
        underlineError(recognizer, offendingSymbol, line, charPositionInLine);
        cout << "/ᐠ–ꞈ–ᐟ\\ Syntax Error! at line " << line << " -> " << msg << endl; 

        }

    protected:
        void underlineError(Recognizer *recognizer , Token *offendingSymbol , size_t line , int charPositionInLine){
                
            CommonTokenStream* tokens = (CommonTokenStream*)recognizer->getInputStream();
            string input = tokens->getTokenSource()->getInputStream()->toString();
            vector<string> lines = stringsplit(input, "\n");
            string errorline = lines[line-1];
            cout << errorline << endl;
            for (int i = 0; i < charPositionInLine; ++i) cout << "\e[31m~";
            int start = offendingSymbol->getStartIndex();
            int stop = offendingSymbol->getStopIndex();
            if (start>=0 && stop >=0){
                    for(int i = start; i<= stop ; ++i) cout << u8"^";
            }
            cout << "\e[39m\n";
        }

};

//=============== END ERROR HANDLER CLASS =================

//=========================================================
//============== EVALUATOR VISITOR CLASS ==================
//=========================================================
class MewMewEval : public mewmewBaseVisitor{

public:
    /*
     * -----------------------------------------------------
     * Handles Assign Expressions
     * 
     * first it checks if variable with given name already 
     * exists [a1] , if yes , then replace the previous 
     * value of the variable with new evaluated value [a2],
     * Otherwise , it creates a new item in symbol table
     * [a3].
     * Then finally returns 0
     *
     * -> m = mewmew;
     *
     * -----------------------------------------------------
     */
    antlrcpp::Any visitAssignExpr(mewmewParser::AssignExprContext *ctx) override{

            string id = ctx->ID()->toString();
            float value = visit(ctx->expr());

            if (_vartable.count(id)){               // <--------- a1
                _vartable[id] = value;              // <--------- a2
            }else{
                _vartable.insert(pair<string , float>(id , value));         // <--------a3
            }
            return 0;
    }
    
    /*
     * ---------------------------------------------------
     *  Handles Print aka. Meow Say Statements
     *  
     *  first it evaluates the given expression and finally
     *  prints it to stdout
     *
     * -> ::mewmew;
     * => 2
     *
     * ---------------------------------------------------
     */

    antlrcpp::Any visitPrintExpr(mewmewParser::PrintExprContext *ctx) override{
        
        float ex = visit(ctx->expr());
        cout << ex << endl;
        return 0;

    }
    
    /*
     * ---------------------------------------------------
     *  Handles Abs Statements
     *
     *  first it evaluates the given expression and then
     *  returns the abs value of it.
     *
     * -> m = -mewmew;
     * -> ::~m;
     * => 2
     * ---------------------------------------------------
     */

   antlrcpp::Any visitAbsExpr(mewmewParser::AbsExprContext *ctx) override{

        float expr = visit(ctx->expr());
        return abs(expr);

   } 

    /*
     * ---------------------------------------------------
     *  Handles Unary Minus Statements
     *
     *  first it evaluates the given expression and then
     *  returns the unary minus value of it.
     *
     * -> m = -mewmew;
     * -> ::m;
     * => -2
     *
     * ---------------------------------------------------
     */

   antlrcpp::Any visitUnaryExpr(mewmewParser::UnaryExprContext *ctx) override{
        
       float exp = visit(ctx->expr());
       return -exp;

   }  
    
    /*
     * ---------------------------------------------------
     *  Handles Modulas (%) Statements
     *  
     *  It returns modulas of two given expression
     *
     * -> m = mewmewmew % mewmew;
     * -> ::m;
     * => 1
     *
     * ---------------------------------------------------
     */

   antlrcpp::Any visitModExpr(mewmewParser::ModExprContext *ctx) override{
    
       float left = visit(ctx->expr(0));
       float right = visit(ctx->expr(1));
       return float((int)left % (int)right);
        
   }

    /*
     * ---------------------------------------------------
     *  Handles Addition (+) or Substraction (-) Statements
     *
     *  first it checks the operator of the statement , if
     *  it is '+' (ADD) [b1] , the returns left expression
     *  + right expression otherwise it returns left
     *  expression - right expression
     *
     * -> m = mew + mewmew - mew ;
     * -> ::m;
     * => 2
     * ---------------------------------------------------
     */

    antlrcpp::Any visitAddSubExpr(mewmewParser::AddSubExprContext *ctx) override{
        
        float left = visit(ctx->expr(0));
        float right = visit(ctx->expr(1));
        
        if (ctx->op->getType() == mewmewParser::ADD){       // <----------------- b1
            
            return left + right;


        }else{
            
            return left - right;

        }

    }

    /*
     * ---------------------------------------------------
     *  Handles Multiplication (*) or Division (/) Statements
     *
     *  first it checks the operator of the statement , if
     *  it is '*' (MUL) [b2] , the returns left expression
     *  x right expression otherwise it returns left
     *  expression ÷ right expression
     *
     * -> m = mewmew * mewmew / mewmew ;
     * -> ::m;
     * => 2 
     * ---------------------------------------------------
     */

    antlrcpp::Any visitMulDivExpr(mewmewParser::MulDivExprContext *ctx) override{
        
        float left = visit(ctx->expr(0));
        float right = visit(ctx->expr(1));

        if (ctx->op->getType() == mewmewParser::MUL){       // <------- b2
                
            return left * right;
            
        }else{
            return left / right;
        }

    }

    /*
     * ---------------------------------------------------
     *  Handles Power (**) Statements
     *
     *  Evaluates left and right expressions , then
     *  returns Left to the power Right using cmath library.
     *
     * -> m = mewmew ** mewmewmew ;
     * -> ::m;
     * => 8
     * ---------------------------------------------------
     */

    antlrcpp::Any visitPowExpr(mewmewParser::PowExprContext *ctx) override{
        
        float left = visit(ctx->expr(0));
        float right = visit(ctx->expr(1));
        
        return pow(left , right);

    }

    /*
     * ---------------------------------------------------
     *  Handles Is Equal to (==) Statements
     *
     *  First evaluates left and right expressions , then
     *  returns 1 (True) if left expression is equal to
     *  right expressions otherwise returns 0 (False).
     *
     * -> m = mewmew == mewew ;
     * -> ::m ;
     * => 1
     * ---------------------------------------------------
     */

    antlrcpp::Any visitEqExpr(mewmewParser::EqExprContext *ctx) override{

        float left = visit(ctx->expr(0));
        float right = visit(ctx->expr(1));
        float result = left == right;


        return result;

    }

    /*
     * ---------------------------------------------------
     *  Handles Is Greater than Equal to (>=) Statements
     *
     *  First evaluates left and right expressions , then
     *  returns 1 (True) if left expression is greater than equal to
     *  right expressions otherwise returns 0 (False).
     *
     * -> m = mewmewmew >= mewew ;
     * -> ::m ;
     * => 1
     * ---------------------------------------------------
     */

    antlrcpp::Any visitGeExpr(mewmewParser::GeExprContext *ctx) override{
        float left = visit(ctx->expr(0));
        float right = visit(ctx->expr(1));
        float result = left >= right;

        return result;

    }

    /*
     * ---------------------------------------------------
     *  Handles Is Greater than (>) Statements
     *
     *  First evaluates left and right expressions , then
     *  returns 1 (True) if left expression is greater than
     *  right expressions otherwise returns 0 (False).
     *
     * -> m = mewmewmew > mewew ;
     * -> ::m ;
     * => 1
     * ---------------------------------------------------
     */

    antlrcpp::Any visitGtExpr(mewmewParser::GtExprContext *ctx) override{
        float left = visit(ctx->expr(0));
        float right = visit(ctx->expr(1));
        float result = left > right;
    
        return result;
    }

    /*
     * ---------------------------------------------------
     *  Handles Is Less than Equal to (<=) Statements
     *
     *  First evaluates left and right expressions , then
     *  returns 1 (True) if left expression is less than
     *  equal to right expressions otherwise returns 0 
     *  (False).
     *
     * -> m = mew <= mewew ;
     * -> ::m ;
     * => 1
     * ---------------------------------------------------
     */

    antlrcpp::Any visitLeExpr(mewmewParser::LeExprContext *ctx) override{
        float left = visit(ctx->expr(0));
        float right = visit(ctx->expr(1));
        float result = left <= right;

        return result;

    }

    /*
     * ---------------------------------------------------
     *  Handles Is Less than (<) Statements
     *
     *  First evaluates left and right expressions , then
     *  returns 1 (True) if left expression is less than
     *  right expressions otherwise returns 0 (False).
     *
     * -> m = mew < mewew ;
     * -> ::m ;
     * => 1
     * ---------------------------------------------------
     */

    antlrcpp::Any visitLtExpr(mewmewParser::LtExprContext *ctx) override{
        float left = visit(ctx->expr(0));
        float right = visit(ctx->expr(1));
        float result = left < right;

        return result;

    }
    
    /*
     * ---------------------------------------------------
     *  Handles Is Not Equal to (!=) Statements
     *
     *  First evaluates left and right expressions , then
     *  returns 1 (True) if left expression is not equal to
     *  right expressions otherwise returns 0 (False).
     *
     * -> m = mew != mewew ;
     * -> ::m ;
     * => 1
     * ---------------------------------------------------
     */

    antlrcpp::Any visitNeExpr(mewmewParser::NeExprContext *ctx) override{
        float left = visit(ctx->expr(0));
        float right = visit(ctx->expr(1));
        float result = left != right;

        return result;

    }

    /*
     * ---------------------------------------------------
     *  Handles Identifier Statements/Expressions
     *
     *  First it checks if any identifier exists in the 
     *  symbol table with the given name [c1] , if yes,
     *  it returns the value associated with the 
     *  identifier from symbol table otherwise it prints an
     *  error message saying the variable is undeclared [c2]
     *
     * -> m = mewmew ;
     * -> ::m ;
     * => 2
     * ---------------------------------------------------
     */

    antlrcpp::Any visitIdExpr(mewmewParser::IdExprContext *ctx) override {

          string id = ctx->ID()->toString();
          if(_vartable.count(id) == 1){         // <------ c1
            return _vartable[id];
          }else{
            cerr << endl << "/ᐠ–ꞈ–ᐟ\\ Undeclared Variable '" << id << "'" << endl; // <--------- c2
            exit(1);
          }
        }

    /*
     * ---------------------------------------------------
     *  Handles Number (mewmew......) Expressions
     *
     *  First it extracts the mewmew... string , with help
     *  of utiliy function `mew_to_float` function , it
     *  converts the string to an equivalent float value.
     *
     *  Mew Numbers can contain decimal (.) to make a 
     *  'real' floating point number.
     *
     *  for example , `mewmew` will be converted to 2 and
     *  `mewmew.mewmewmew` will be converted to 2.3
     *
     * -> m = mewmew.mew ;
     * -> ::m ;
     * => 2.1
     * ---------------------------------------------------
     */

    antlrcpp::Any visitNumExpr(mewmewParser::NumExprContext *ctx) override{
        
        string m_ = ctx->MEWNUMBER()->toString();
        float x = mew_to_float(m_);
        return x;

    }

    /*
     * ---------------------------------------------------
     *  Handles If Statements
     *
     *  First it evaluates condition expression , if the
     *  value is 1 (True) , the statement body is executed
     *
     * -> mewmew > mew ? ::mewmew; ;
     * => 2
     * ---------------------------------------------------
     */

    antlrcpp::Any visitIfExpr(mewmewParser::IfExprContext *ctx) override{
        
        float cond = visit(ctx->expr());
        if (cond == 1){
            visit(ctx->stmts());
        }
        return cond;

    }

    /*
     * ---------------------------------------------------
     *  Handles If Else Statements
     *
     *  First it evaluates condition expression , if the
     *  value is 1 (True) , the truthy statement body is 
     *  executed otherwise else statement body is
     *  executed
     *
     * -> mewmew < mew ? ::mewmew; : ::mew; ;
     * => 1
     * ---------------------------------------------------
     */
    antlrcpp::Any visitIfElseExpr(mewmewParser::IfElseExprContext *ctx) override{
       
       float cond = visit(ctx->expr());

       if (cond == 1) {
            visit(ctx->stmts(0));
       }else{
            visit(ctx->stmts(1));
       }
       return cond;

    }

    /*
     * ---------------------------------------------------
     *  Handles Repeat Loop aka. Meow Loop Statements
     *
     *  First it evaluates condition expression , then it
     *  executes the body N times , where N is value of 
     *  returned by the condition expression
     *
     * -> m = mew;
     * -> @mewmewmew: m = m + mew; ;
     * -> ::m;
     * => 4
     * ---------------------------------------------------
     */

    antlrcpp::Any visitRepeatExpr(mewmewParser::RepeatExprContext *ctx) override{
    
       float con = visit(ctx->expr());
       for (int i = 1 ; i <= con ; ++i){
        visit(ctx->stmts());
        
       }
       return con;

    }

};

//============ END EVALUATOR VISITOR CLASS ================

/*
 * --------------------------------------------------------
 *  Parses and Evaluates Given Filename
 *
 *  Mostly Copy paste from the book
 *
 *  what it does is , open the file , tokenize the file.
 *  Feed the tokens to the parser , then a parse tree
 *  is created, then we tell our visitor class to visit
 *  the parse tree and execute the commands
 *
 * ---------------------------------------------------------
 */

void parse(ANTLRFileStream input){

    mewmewLexer lexer(&input);
    CommonTokenStream tokens(&lexer);
    tokens.fill();
    mewmewParser parser(&tokens);
    parser.removeErrorListeners();
    MewErr errorlistener;
    parser.addErrorListener(&errorlistener);
    tree::ParseTree* tree = parser.prog();
    MewMewEval* mv = new MewMewEval();
    mv->visit(tree);
    
}

/*
 * --------------------------------------------------------
 *  Driver Function
 *
 *  It checks if the user has supplied a filename to 
 *  execute , if not , it prints the usage , else
 *  first it checks if the given filename exits, if
 *  exists , it calls the parse function to parse the
 *  user supplied filename , otherwise prints an error
 *  saying that the filename doesnot exist.
 *
 * --------------------------------------------------------
 */

int main(int argc, char **argv) {

  if (argc < 2) {
    cout << endl << u8"/ᐠ｡ꞈ｡ᐟ\\ MewMew - Program in Cats' Language!" << endl << endl;
    cout << "No Specified filename to execute :(" << endl;
    cerr << "Usage: ./mewmew FILENAME" << endl;
  } else if (argc == 2) {
    
    string path = argv[1];      // filename
    ifstream infile(path);
    
    if(infile.good()){          // Checks if file exits
        infile.close();
        ANTLRFileStream filename(path);
        parse(filename);
    }else{
        cout << "MEOW!! File '" << path << "' " << "Doesn't Exist!" << endl;
    }

    
  }
  return 0;
}