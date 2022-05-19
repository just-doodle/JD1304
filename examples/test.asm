MEMORY_BEGIN: #9000h
`LABEL3`
LDA [8036]
RET

MEMORY_BEGIN: #8000h
LDA 'A'
// 0x8004
`LABEL`
DD #12h
LDA %LABEL%
`LABEL2`
DD #8036h
JSR %LABEL3%
LDA $30
PUSHA
TSX

// if __WARN__ is defined then use the warn preprocessor
@DEFINE __WARN__ 0
@IFDEF __WARN__
@WARN Hello this is a warning preprocessor
@ENDIF

// if __ERROR__ not defined then use the error preprocessor
@DEFINE __ERROR__ 0
@IFNDEF __ERROR__
@ERROR Hello this is a error preprocessor
@ENDIF

// To print the definition of a macro
@MCMP __ERROR__ 0
@INFO EQUAL WARN = *__WARN__* ERROR = *__ERROR__*
@ENDIF

@INFO This is a info preprocessor

TXA
PUSHA
// 0x8035
MEMORY_BEGIN: #8035h
DD #44h