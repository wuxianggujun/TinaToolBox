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
    ;

openStatement: 'open' STRING;
selectSheetStatement: 'select' 'sheet' (STRING | NUMBER);
readCellStatement: 'read' cell;
writeCellStatement: 'write' value 'to' cell;
forEachStatement: 'for' 'each' 'row' 'in' range block;
ifStatement: 'if' condition block;
printStatement: 'print' value;

cell: CELL_REF;
range: CELL_REF '..' CELL_REF;
block: '{' statement* '}';
condition: value COMPARE_OP value;
value: STRING | NUMBER | cell;

// Lexer Rules
CELL_REF: [A-Z]+[0-9]+;
STRING: '"' .*? '"';
NUMBER: [0-9]+('.'[0-9]+)?;
COMPARE_OP: '==' | '!=' | '>' | '<' | '>=' | '<=';
WS: [ \t\r\n]+ -> skip;
COMMENT: '//' .*? '\r'? '\n' -> skip; 