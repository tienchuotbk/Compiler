/* 
 * @copyright (c) 2008, Hedspi, Hanoi University of Technology
 * @author Huu-Duc Nguyen
 * @version 1.0
 */
#include <stdio.h>
#include <stdlib.h>

#include "reader.h"
#include "scanner.h"
#include "parser.h"
#include "error.h"
#include "debug.h"

Token *currentToken;
Token *lookAhead;

extern Type* intType;
extern Type* charType;
extern SymTab* symtab;

void scan(void) {
  Token* tmp = currentToken;
  currentToken = lookAhead;
  lookAhead = getValidToken();
  free(tmp);
}

void eat(TokenType tokenType) {
  if (lookAhead->tokenType == tokenType) {
    scan();
  } else missingToken(tokenType, lookAhead->lineNo, lookAhead->colNo);
}

void compileProgram(void) {
  // TODO: create, enter, and exit program block
  Object* program;
  
  eat(KW_PROGRAM);
  eat(TK_IDENT);
  
  program = createProgramObject(currentToken->string);
  enterBlock(program->progAttrs->scope);
  
  eat(SB_SEMICOLON);
  compileBlock();
  eat(SB_PERIOD);
}

void compileBlock(void) {
  
  // TODO: create and declare constant objects
  Object *constObj;
  ConstantValue *constValue;
    
  if (lookAhead->tokenType == KW_CONST) {
    eat(KW_CONST);

    do {
      eat(TK_IDENT);
      
      // create
	  // TODO CHECK
	  constObj = createConstantObject(currentToken->string);
      eat(SB_EQ);
      constValue = compileConstant();
      
      // init const value
	  constObj->constAttrs->value = constValue;
	  declareObject(constObj);
	  
      eat(SB_SEMICOLON);
    } while (lookAhead->tokenType == TK_IDENT);

    compileBlock2();
  } 
  else compileBlock2();
}

void compileBlock2(void) {
  
  // TODO: create and declare type objects
  Object *typeObj;
  Type *actualType;
  if (lookAhead->tokenType == KW_TYPE) {
    eat(KW_TYPE);

    do {
      eat(TK_IDENT);
      
      // create
      // TODO CHECK
      typeObj = createTypeObject(currentToken->string);
	  
      eat(SB_EQ);
      // set type then declare
      actualType = compileType();
      typeObj->typeAttrs->actualType = actualType;
      declareObject(typeObj);
	  
      eat(SB_SEMICOLON);
    } while (lookAhead->tokenType == TK_IDENT);

    compileBlock3();
  } 
  else compileBlock3();
}

void compileBlock3(void) {
  
  // TODO: create and declare variable objects
  Object *varObj;
  Type *varType;
  if (lookAhead->tokenType == KW_VAR) {
    eat(KW_VAR);

    do {
      eat(TK_IDENT);
      
      // create
      // TODO CHECK
      varObj = createVariableObject(currentToken->string);
      eat(SB_COLON);
      
      // set variable type then declare
      varType = compileType();
      varObj->varAttrs->type = varType;
      declareObject(varObj);
	  
      eat(SB_SEMICOLON);
    } while (lookAhead->tokenType == TK_IDENT);

    compileBlock4();
  } 
  else compileBlock4();
}

void compileBlock4(void) {
  compileSubDecls();
  compileBlock5();
}

void compileBlock5(void) {
  eat(KW_BEGIN);
  compileStatements();
  eat(KW_END);
}

void compileSubDecls(void) {
  while ((lookAhead->tokenType == KW_FUNCTION) || (lookAhead->tokenType == KW_PROCEDURE)) {
    if (lookAhead->tokenType == KW_FUNCTION)
      compileFuncDecl();
    else compileProcDecl();
  }
}

void compileFuncDecl(void) {
  
  // TODO: create and declare a function object
  Object *funcObj;
  Type *returnType;
  
  eat(KW_FUNCTION);
  eat(TK_IDENT);
  
  // create and declare
  // TODO CHECK
  funcObj = createFunctionObject(currentToken->string);
  declareObject(funcObj);
  // enter function scope
  enterBlock(funcObj->funcAttrs->scope);
  
  compileParams();
  eat(SB_COLON);
  
  // set return type for function
  returnType = compileBasicType();
  funcObj->funcAttrs->returnType = returnType;
    
  eat(SB_SEMICOLON);
  compileBlock();
  eat(SB_SEMICOLON);
  
  // exit function scope
    exitBlock();
}

void compileProcDecl(void) {
  
  // TODO: create and declare a procedure object
   Object *procObj;
   
  eat(KW_PROCEDURE);
  eat(TK_IDENT);
  
  // create and declare
    // TODO CHECK
    procObj = createProcedureObject(currentToken->string);
    declareObject(procObj);
    // enter procedure scope
    enterBlock(procObj->procAttrs->scope);
    
  compileParams();
  eat(SB_SEMICOLON);
  compileBlock();
  eat(SB_SEMICOLON);
  
   // exit procedure scope
    exitBlock();
    
}

ConstantValue* compileUnsignedConstant(void) {
  // TODO: create and return an unsigned constant value
  ConstantValue* constValue;

  switch (lookAhead->tokenType) {
  case TK_NUMBER:
    eat(TK_NUMBER);
    break;
  case TK_IDENT:
    eat(TK_IDENT);
    //TODO
    break;
  case TK_CHAR:
    eat(TK_CHAR);
    //TODO
    constValue = makeCharConstant(currentToken->string[0]);
    break;
  default:
    error(ERR_INVALID_CONSTANT, lookAhead->lineNo, lookAhead->colNo);
    break;
  }
  return constValue;
}

ConstantValue* compileConstant(void) {
  // TODO: create and return a constant
  ConstantValue* constValue;

  switch (lookAhead->tokenType) {
  case SB_PLUS:
    eat(SB_PLUS);
    // set const int value
    constValue = compileConstant2();
    break;
  case SB_MINUS:
    eat(SB_MINUS);
    // set const int value (negative)
    constValue = compileConstant2();
    constValue->intValue = -constValue->intValue;
    break;
  case TK_CHAR:
    eat(TK_CHAR);
    // set const char value
    constValue = makeCharConstant(currentToken->string[0]);
    break;
  default:
    compileConstant2();
    break;
  }
  return constValue;
}

ConstantValue* compileConstant2(void) {
  // TODO: create and return a constant value
  ConstantValue* constValue;
  Object* obj;

  switch (lookAhead->tokenType) {
  case TK_NUMBER:
    eat(TK_NUMBER);
    //Number
    constValue = makeIntConstant(currentToken->value);
    break;
  case TK_IDENT:
    eat(TK_IDENT);
    // TODO get ident's type then duplicate if int
    
    break;
  default:
    error(ERR_INVALID_CONSTANT, lookAhead->lineNo, lookAhead->colNo);
    break;
  }
  return constValue;
}

Type* compileType(void) {
  // TODO: create and return a type
  Type* type;
  Type* elementType;
  int arraySize;
  Object* obj;

  switch (lookAhead->tokenType) {
  case KW_INTEGER: 
    eat(KW_INTEGER);
    // set type
    type = makeIntType();
    break;
  case KW_CHAR: 
    eat(KW_CHAR);
    // set type
    type = makeCharType();
    break;
  case KW_ARRAY:
    eat(KW_ARRAY);
    eat(SB_LSEL);
    eat(TK_NUMBER);
    // set array size
    arraySize = currentToken->value;
    eat(SB_RSEL);
    eat(KW_OF);
    // set array type and create it
    elementType = compileType();
    type = makeArrayType(arraySize, elementType);
    compileType();
    break;
  case TK_IDENT:
    eat(TK_IDENT);
    // TODO get to the ident's type then duplicate
    
    break;
  default:
    error(ERR_INVALID_TYPE, lookAhead->lineNo, lookAhead->colNo);
    break;
  }
  return type;
}

Type* compileBasicType(void) {
  // TODO: create and return a basic type
  Type* type;

  switch (lookAhead->tokenType) {
  case KW_INTEGER: 
    eat(KW_INTEGER);
	// integer
    type = makeIntType(); 
    break;
  case KW_CHAR: 
    eat(KW_CHAR);
	// char
    type = makeCharType(); 
    break;
  default:
    error(ERR_INVALID_BASICTYPE, lookAhead->lineNo, lookAhead->colNo);
    break;
  }
  return type;
}

void compileParams(void) {
  if (lookAhead->tokenType == SB_LPAR) {
    eat(SB_LPAR);
    compileParam();
    while (lookAhead->tokenType == SB_SEMICOLON) {
      eat(SB_SEMICOLON);
      compileParam();
    }
    eat(SB_RPAR);
  }
}

void compileParam(void) {
  // TODO: create and declare a parameter
  Object* param;
  Type* type;
  enum ParamKind paramKind;

  switch (lookAhead->tokenType) {
  case TK_IDENT:
    // compileBasicType later
    paramKind = PARAM_VALUE;
//    eat(SB_COLON);
//    compileBasicType();
    break;
  case KW_VAR:
    eat(KW_VAR);
    // compileBasicType later
    paramKind = PARAM_REFERENCE;
//    eat(TK_IDENT);
//    eat(SB_COLON);
//    compileBasicType();
    break;
  default:
    error(ERR_INVALID_PARAMETER, lookAhead->lineNo, lookAhead->colNo);
    break;
  }
  eat(TK_IDENT);
    // create
    param = createParameterObject(currentToken->string, paramKind, symtab->currentScope->owner);

    eat(SB_COLON);
    // compileBasicType then declare param
    type = compileBasicType();
    param->paramAttrs->type = type;
    declareObject(param);
  
}

void compileStatements(void) {
  compileStatement();
  while (lookAhead->tokenType == SB_SEMICOLON) {
    eat(SB_SEMICOLON);
    compileStatement();
  }
}

void compileStatement(void) {
  switch (lookAhead->tokenType) {
  case TK_IDENT:
    compileAssignSt();
    break;
  case KW_CALL:
    compileCallSt();
    break;
  case KW_BEGIN:
    compileGroupSt();
    break;
  case KW_IF:
    compileIfSt();
    break;
  case KW_WHILE:
    compileWhileSt();
    break;
  case KW_FOR:
    compileForSt();
    break;
    // EmptySt needs to check FOLLOW tokens
  case SB_SEMICOLON:
  case KW_END:
  case KW_ELSE:
    break;
    // Error occurs
  default:
    error(ERR_INVALID_STATEMENT, lookAhead->lineNo, lookAhead->colNo);
    break;
  }
}

void compileLValue(void) {
  eat(TK_IDENT);
  compileIndexes();
}

void compileAssignSt(void) {
  compileLValue();
  eat(SB_ASSIGN);
  compileExpression();
}

void compileCallSt(void) {
  eat(KW_CALL);
  eat(TK_IDENT);
  compileArguments();
}

void compileGroupSt(void) {
  eat(KW_BEGIN);
  compileStatements();
  eat(KW_END);
}

void compileIfSt(void) {
  eat(KW_IF);
  compileCondition();
  eat(KW_THEN);
  compileStatement();
  if (lookAhead->tokenType == KW_ELSE) 
    compileElseSt();
}

void compileElseSt(void) {
  eat(KW_ELSE);
  compileStatement();
}

void compileWhileSt(void) {
  eat(KW_WHILE);
  compileCondition();
  eat(KW_DO);
  compileStatement();
}

void compileForSt(void) {
  eat(KW_FOR);
  eat(TK_IDENT);
  eat(SB_ASSIGN);
  compileExpression();
  eat(KW_TO);
  compileExpression();
  eat(KW_DO);
  compileStatement();
}

void compileArgument(void) {
  compileExpression();
}

void compileArguments(void) {
  switch (lookAhead->tokenType) {
  case SB_LPAR:
    eat(SB_LPAR);
    compileArgument();

    while (lookAhead->tokenType == SB_COMMA) {
      eat(SB_COMMA);
      compileArgument();
    }

    eat(SB_RPAR);
    break;
    // Check FOLLOW set 
  case SB_TIMES:
  case SB_SLASH:
  case SB_PLUS:
  case SB_MINUS:
  case KW_TO:
  case KW_DO:
  case SB_RPAR:
  case SB_COMMA:
  case SB_EQ:
  case SB_NEQ:
  case SB_LE:
  case SB_LT:
  case SB_GE:
  case SB_GT:
  case SB_RSEL:
  case SB_SEMICOLON:
  case KW_END:
  case KW_ELSE:
  case KW_THEN:
    break;
  default:
    error(ERR_INVALID_ARGUMENTS, lookAhead->lineNo, lookAhead->colNo);
  }
}

void compileCondition(void) {
  compileExpression();
  switch (lookAhead->tokenType) {
  case SB_EQ:
    eat(SB_EQ);
    break;
  case SB_NEQ:
    eat(SB_NEQ);
    break;
  case SB_LE:
    eat(SB_LE);
    break;
  case SB_LT:
    eat(SB_LT);
    break;
  case SB_GE:
    eat(SB_GE);
    break;
  case SB_GT:
    eat(SB_GT);
    break;
  default:
    error(ERR_INVALID_COMPARATOR, lookAhead->lineNo, lookAhead->colNo);
  }

  compileExpression();
}

void compileExpression(void) {
  switch (lookAhead->tokenType) {
  case SB_PLUS:
    eat(SB_PLUS);
    compileExpression2();
    break;
  case SB_MINUS:
    eat(SB_MINUS);
    compileExpression2();
    break;
  default:
    compileExpression2();
  }
}

void compileExpression2(void) {
  compileTerm();
  compileExpression3();
}


void compileExpression3(void) {
  switch (lookAhead->tokenType) {
  case SB_PLUS:
    eat(SB_PLUS);
    compileTerm();
    compileExpression3();
    break;
  case SB_MINUS:
    eat(SB_MINUS);
    compileTerm();
    compileExpression3();
    break;
    // check the FOLLOW set
  case KW_TO:
  case KW_DO:
  case SB_RPAR:
  case SB_COMMA:
  case SB_EQ:
  case SB_NEQ:
  case SB_LE:
  case SB_LT:
  case SB_GE:
  case SB_GT:
  case SB_RSEL:
  case SB_SEMICOLON:
  case KW_END:
  case KW_ELSE:
  case KW_THEN:
    break;
  default:
    error(ERR_INVALID_EXPRESSION, lookAhead->lineNo, lookAhead->colNo);
  }
}

void compileTerm(void) {
  compileFactor();
  compileTerm2();
}

void compileTerm2(void) {
  switch (lookAhead->tokenType) {
  case SB_TIMES:
    eat(SB_TIMES);
    compileFactor();
    compileTerm2();
    break;
  case SB_SLASH:
    eat(SB_SLASH);
    compileFactor();
    compileTerm2();
    break;
    // check the FOLLOW set
  case SB_PLUS:
  case SB_MINUS:
  case KW_TO:
  case KW_DO:
  case SB_RPAR:
  case SB_COMMA:
  case SB_EQ:
  case SB_NEQ:
  case SB_LE:
  case SB_LT:
  case SB_GE:
  case SB_GT:
  case SB_RSEL:
  case SB_SEMICOLON:
  case KW_END:
  case KW_ELSE:
  case KW_THEN:
    break;
  default:
    error(ERR_INVALID_TERM, lookAhead->lineNo, lookAhead->colNo);
  }
}

void compileFactor(void) {
  switch (lookAhead->tokenType) {
  case TK_NUMBER:
    eat(TK_NUMBER);
    break;
  case TK_CHAR:
    eat(TK_CHAR);
    break;
  case TK_IDENT:
    eat(TK_IDENT);
    switch (lookAhead->tokenType) {
    case SB_LPAR:
      compileArguments();
      break;
    case SB_LSEL:
      compileIndexes();
      break;
    default:
      break;
    }
    break;
  default:
    error(ERR_INVALID_FACTOR, lookAhead->lineNo, lookAhead->colNo);
  }
}

void compileIndexes(void) {
  while (lookAhead->tokenType == SB_LSEL) {
    eat(SB_LSEL);
    compileExpression();
    eat(SB_RSEL);
  }
}

int compile(char *fileName) {
  if (openInputStream(fileName) == IO_ERROR)
    return IO_ERROR;

  currentToken = NULL;
  lookAhead = getValidToken();

  initSymTab();

  compileProgram();

  printObject(symtab->program,0);

  cleanSymTab();

  free(currentToken);
  free(lookAhead);
  closeInputStream();
  return IO_SUCCESS;

}
