grammar ExcelScript;

options {
    language=Cpp;
}

// Parser Rules
program: statement+ EOF;

statement
    : openStatement
    | selectSheetStatement
    | readCellStatement
    | writeCellStatement
    | forEachStatement
    | ifStatement
    | printStatement
    | getConfigStatement
    | setConfigStatement
    ;

openStatement: 'open' value;
selectSheetStatement: 'select' 'sheet' value;
readCellStatement: 'read' cell;
writeCellStatement: 'write' value 'to' cell;
forEachStatement: 'for' 'each' 'row' 'in' range block;
ifStatement: 'if' condition block;
printStatement: 'print' value;
getConfigStatement: 'get' 'config' STRING;
setConfigStatement: 'set' 'config' STRING value;

cell: CELL_REF;
range: CELL_REF '..' CELL_REF;
block: '{' statement* '}';
condition: value COMPARE_OP value;
value: STRING | NUMBER | cell | configValue;
configValue: 'config' STRING;

// Lexer Rules
CELL_REF: [A-Z]+[0-9]+;
STRING: '"' .*? '"';
NUMBER: [0-9]+('.'[0-9]+)?;
COMPARE_OP: '==' | '!=' | '>' | '<' | '>=' | '<=';
WS: [ \t\r\n]+ -> skip;
COMMENT: '//' .*? '\r'? '\n' -> skip; 