#include "rastuff.h"

#include <QApplication>
#include <QLocale>
#include <QTranslator>

void    test_rcheevos_parse();

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    RAStuff w;
    w.show();
    return a.exec();
    //test_rcheevos_parse();
}

#include "rcheevos/rc_internal.h"

typedef struct rc_condset_memrefs_t
{
    rc_memref_t* memrefs;
    rc_value_t* variables;
} rc_condset_memrefs_t;


int m_rc_operand_is_memref(const rc_operand_t* self) {
    switch (self->type) {
    case RC_OPERAND_CONST:
    case RC_OPERAND_FP:
    case RC_OPERAND_LUA:
    case RC_OPERAND_RECALL:
        return 0;

    default:
        return 1;
    }
}

void    test_rcheevos_parse()
{
    //QString test = "0x 0000a0=188.1._R:0xH00f36d<d0xH00f36d_0xH0004c2=64_R:0xH00008a>0";
    QString test = "0xH00008a>0";

    rc_condset_t* condset;
    rc_condset_memrefs_t* memrefs = (rc_condset_memrefs_t*) malloc(sizeof(rc_condset_memrefs_t));
    char buffer[2048];
    const char* memaddr = "0x 0000a0=188.1._R:0xH00f36d<d0xH00f36d_0xH0004c2=64_R:0xH00008a>0";
    qDebug() << memaddr;
    rc_parse_state_t parse;
    int size;

    qDebug() << "init";
    rc_init_parse_state(&parse, buffer, 0, 0);
    rc_init_parse_state_memrefs(&parse, &memrefs->memrefs);
    rc_init_parse_state_variables(&parse, &memrefs->variables);

    qDebug() << "parse cond";
    condset = rc_parse_condset(&memaddr, &parse, 0);
    size = parse.offset;
    rc_destroy_parse_state(&parse);
    rc_condset_t* next = condset;
    qDebug() << "checking for address";
    rc_memref_t* m_next = memrefs->memrefs;
    while (m_next != nullptr)
    {
        qDebug() << QString::number(m_next->address, 16);
        qDebug() << m_next->value.size;
        m_next = m_next->next;
    }
}
