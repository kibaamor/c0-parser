# c0-parser
C0语言的词法分析和抽象语法树(AST)分析器


## C0语言

C0是简化了C语言语法与编译过程得到的小型编程语言。在文档的最后有C0的完整文法定义。

保留了C中的：
- 变量类型：int, char, double
- 条件判断：if
- 循环语句：for, while, do
- 循环控制：break, continue
- 输入输出：scan, print
- 函数相关：return

## 如何编译

### 需要的软件

- Visual Studio 2015及以上(Windows)
- Gcc 4.9.1 (Linux)
- CMake 3.1及以上

### 编译步骤

打开命令行

```
# 下载源码
git clone https://github.com/KibaAmor/c0-parser.git

# 进入源码根目录
cd c-parser

# 创建存放编译中间文件的目录
mkdir build

# 进入存放编译中间文件的目录
cd build

# 生成工程
cmake ..

# 编译工程
# Windows下直接用Visual Studio打开c0-parser.sln
c0-parser.sln
# Linux下使用下面的命令编译
make -j3
```

生成的可执行文件在`存放编译中间文件的目录`下面的bin文件夹中。

> Windows下也可以运行源码根目录scripts目录下的`gen_vs2015project.bat`或者`gen_vs2017project.bat`来生成sln工程文件。

## 如何测试

### Windows

在Visual Studio中编译工程中名为`RUN_TESTS`的项目，即可在Visual Studio的编译输出控制台中看到测试结果，也可以直接运行项目中`test`文件夹下的工程。

### Linux

打开命令行

```
进入源码根目录
cd c-parser

# 进入存放编译中间文件的目录
cd build

# 运行所有测试
make test

```

## 附：完整的C0文法

```text
<digit> ::= 
    '0'|<nonzero-digit>
<nonzero-digit> ::= 
    '1'|'2'|'3'|'4'|'5'|'6'|'7'|'8'|'9'
<hexadecimal-digit> ::=
    <digit>|'a'|'b'|'c'|'d'|'e'|'f'|'A'|'B'|'C'|'D'|'E'|'F'

<integer-literal> ::= 
    <decimal-literal>|<hexadecimal-literal>
<decimal-literal> ::= 
    '0'|<nonzero-digit>{<digit>}
<hexadecimal-literal> ::= 
    ('0x'|'0X')<hexadecimal-digit>{<hexadecimal-digit>}


<nondigit> ::=    'a'|'b'|'c'|'d'|'e'|'f'|'g'|'h'|'i'|'j'|'k'|'l'|'m'|'n'|'o'|'p'|'q'|'r'|'s'|'t'|'u'|'v'|'w'|'x'|'y'|'z'|'A'|'B'|'C'|'D'|'E'|'F'|'G'|'H'|'I'|'J'|'K'|'L'|'M'|'N'|'O'|'P'|'Q'|'R'|'S'|'T'|'U'|'V'|'W'|'X'|'Y'|'Z'

<identifier> ::= 
    <nondigit>{<nondigit>|<digit>}
<reserved-word> ::= 
     'const'
    |'void'   |'int'    |'char'   |'double'
    |'struct'
    |'if'     |'else'   
    |'switch' |'case'   |'default'
    |'while'  |'for'    |'do'
    |'return' |'break'  |'continue' 
    |'print'  |'scan'


<char-literal> ::= 
    "'" (<c-char>|<escape-seq>) "'" 
<string-literal> ::= 
    '"' {<s-char>|<escape-seq>} '"'
<escape-seq> ::=  
      '\\' | "\'" | '\"' | '\n' | '\r' | '\t'
    | '\x'<hexadecimal-digit><hexadecimal-digit>

    
<sign> ::= 
    '+'|'-'
<digit-seq> ::=
    <digit>{<digit>}
<floating-literal> ::= 
     [<digit-seq>]'.'<digit-seq>[<exponent>]
    |<digit-seq>'.'[<exponent>]
    |<digit-seq><exponent>
<exponent> ::= 
    ('e'|'E')[<sign>]<digit-seq>
   
    
<unary-operator>          ::= '+' | '-'
<additive-operator>       ::= '+' | '-'
<multiplicative-operator> ::= '*' | '/'
<relational-operator>     ::= '<' | '<=' | '>' | '>=' | '!=' | '=='
<assignment-operator>     ::= '='   

    
<single-line-comment> ::=
    '//'{<any-char>}<LF>
<multi-line-comment> ::= 
    '/*'{<any-char>}'*/'  
    
    
<type-specifier>         ::= <simple-type-specifier>
<simple-type-specifier>  ::= 'void'|'int'|'char'|'double'
<const-qualifier>        ::= 'const'
    
    
<C0-program> ::= 
    {<variable-declaration>}{<function-definition>}


<variable-declaration> ::= 
    [<const-qualifier>]<type-specifier><init-declarator-list>';'
<init-declarator-list> ::= 
    <init-declarator>{','<init-declarator>}
<init-declarator> ::= 
    <identifier>[<initializer>]
<initializer> ::= 
    '='<expression>    

    
<function-definition> ::= 
    <type-specifier><identifier><parameter-clause><compound-statement>

<parameter-clause> ::= 
    '(' [<parameter-declaration-list>] ')'
<parameter-declaration-list> ::= 
    <parameter-declaration>{','<parameter-declaration>}
<parameter-declaration> ::= 
    [<const-qualifier>]<type-specifier><identifier>

    
<compound-statement> ::= 
    '{' {<variable-declaration>} <statement-seq> '}'
<statement-seq> ::= 
    {<statement>}
<statement> ::= 
     <compound-statement>
    |<condition-statement>
    |<loop-statement>
    |<jump-statement>
    |<print-statement>
    |<scan-statement>
    |<assignment-expression>';'
    |<function-call>';'
    |';'   
    
    
<condition> ::= 
     <expression>[<relational-operator><expression>] 
   
<condition-statement> ::= 
     'if' '(' <condition> ')' <statement> ['else' <statement>]
    |'switch' '(' <expression> ')' '{' {<labeled-statement>} '}'

<labeled-statement> ::= 
     'case' (<integer-literal>|<char-literal>) ':' <statement>
    |'default' ':' <statement>

    
<loop-statement> ::= 
    'while' '(' <condition> ')' <statement>
   |'do' <statement> 'while' '(' <condition> ')' ';'
   |'for' '('<for-init-statement> [<condition>]';' [<for-update-expression>]')' <statement>

<for-init-statement> ::= 
    [<assignment-expression>{','<assignment-expression>}]';'
<for-update-expression> ::=
    (<assignment-expression>|<function-call>){','(<assignment-expression>|<function-call>)}


<jump-statement> ::= 
     'break' ';'
    |'continue' ';'
    |<return-statement>
<return-statement> ::= 'return' [<expression>] ';'
    
    
<scan-statement> ::= 
    'scan' '(' <identifier> ')' ';'
<print-statement> ::= 
    'print' '(' [<printable-list>] ')' ';'
<printable-list>  ::= 
    <printable> {',' <printable>}
<printable> ::= 
    <expression> | <string-literal>

<assignment-expression> ::= 
    <identifier><assignment-operator><expression>
    
   
  
<expression> ::= 
    <additive-expression>
<additive-expression> ::= 
     <multiplicative-expression>{<additive-operator><multiplicative-expression>}
<multiplicative-expression> ::= 
     <cast-expression>{<multiplicative-operator><cast-expression>}
<cast-expression> ::=
    {'('<type-specifier>')'}<unary-expression>
<unary-expression> ::=
    [<unary-operator>]<primary-expression>
<primary-expression> ::=  
     '('<expression>')' 
    |<identifier>
    |<integer-literal>
    |<char-literal>
    |<floating-literal>
    |<function-call>

<function-call> ::= 
    <identifier> '(' [<expression-list>] ')'
<expression-list> ::= 
    <expression>{','<expression>}
```