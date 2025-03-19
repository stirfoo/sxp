/*
  proc_numbers.cpp
  S. Edward Dolan
  Wednesday, June 21 2023
*/

// =========================================================================
// Number Procedures

// ... proc number]
// ... proc number integer]
case vasm::INT_1: {
    ppush(Integer::fetch(cpINumber(ppeek(0))->toInt()));
    break;
}
// ... proc number]
// ... proc number float]
case vasm::FLOAT_1: {
    ppush(Float::create(cpINumber(ppeek(0))->toFloat()));
    break;
}

// ... proc]
// ... proc 0]
case vasm::ADD_0: {
    ppush(Integer::fetch(0));
    break;
}
// ... proc x]
// ... proc x x]
case vasm::ADD_1: {
    ppush(cpINumber(ppeek())); // throw if not a number, else dup it
    break;
}
// ... proc x y]
// ... proc x y sum]
case vasm::ADD_2: {
    ppush(cpINumber(ppeek(1))->add(cpINumber(ppeek())));
    break;
}
// ... proc x y list-or-nil]
// ... proc x y list-or-nil sum]
case vasm::ADD_2N: {
    INumber* x = cpINumber(ppeek(2)), *y = cpINumber(ppeek(1));
    x = x->add(y);
    for (ISeq* s=rt::seq(ppeek()); s; s=s->next())
        x = x->add(cpINumber(s->first()));
    ppush(x);
    break;
}
// ... proc x]
// ... proc x -x]
case vasm::SUB_1: {
    ppush(cpINumber(ppeek())->neg());
    break;
}
// ... proc x y]
// ... proc x y dif]
case vasm::SUB_2: {
    ppush(cpINumber(ppeek(1))->sub(cpINumber(ppeek())));
    break;
}
// ... proc x y list-or-nil]
// ... proc x y list-or-nil dif]
case vasm::SUB_2N: {
    INumber* x = cpINumber(ppeek(2)), *y = cpINumber(ppeek(1));
    x = x->sub(y);
    for (ISeq* s=rt::seq(ppeek()); s; s=s->next())
        x = x->sub(cpINumber(s->first()));
    ppush(x);
    break;
}
// ... proc]
// ... proc 0]
case vasm::MUL_0: {
    ppush(Integer::fetch(1));
    break;
}
// ... proc x]
// ... proc x x]
case vasm::MUL_1: {
    ppush(cpINumber(ppeek())); // throw if not a number, else dup it
    break;
}
// ... proc x y]
// ... proc x y prod]
case vasm::MUL_2: {
    ppush(cpINumber(ppeek(1))->mul(cpINumber(ppeek())));
    break;
}
// ... proc x y list-or-nil]
// ... proc x y list-or-nil prod]
case vasm::MUL_2N: {
    INumber* x = cpINumber(ppeek(2)), *y = cpINumber(ppeek(1));
    x = x->mul(y);
    for (ISeq* s=rt::seq(ppeek()); s; s=s->next())
        x = x->mul(cpINumber(s->first()));
    ppush(x);
    break;
}
// ... proc x]
// ... proc x quot]
case vasm::DIV_1: {
    ppush(rt::INT_ONE->div(cpINumber(ppeek())));
    break;
}
// ... proc x y]
// ... proc x y quot]
case vasm::DIV_2: {
    ppush(cpINumber(ppeek(1))->div(cpINumber(ppeek())));
    break;
}
// ... proc x y list-or-nil]
// ... proc x y list-or-nil sum]
case vasm::DIV_2N: {
    INumber* x = cpINumber(ppeek(2)), *y = cpINumber(ppeek(1));
    x = x->div(y);
    for (ISeq* s=rt::seq(ppeek()); s; s=s->next())
        x = x->div(cpINumber(s->first()));
    ppush(x);
    break;
}
// ... proc x]
// ... proc x true]
case vasm::EQEQ_1: {
    ppush(rt::T);
    break;
}
// ... proc x y]
// ... proc x y bool]
case vasm::EQEQ_2: {
    ppush(cpINumber(ppeek(1))->eq(cpINumber(ppeek())) ? rt::T : rt::F);
    break;
}
// ... proc x y list-or-nil]
// ... proc x y list-or-nil bool]
case vasm::EQEQ_2N: {
    INumber* x = cpINumber(ppeek(2)), *y = cpINumber(ppeek(1));
    if (!x->eq(y)) {
        ppush(rt::F);
        goto EQEQ_FAIL;
    }
    x = y;
    for (ISeq* s=rt::seq(ppeek()); s; s=s->next()) {
        y = cpINumber(s->first());
        if (!x->eq(y)) {
            ppush(rt::F);
            goto EQEQ_FAIL;
        }
        x = y;
    }
    ppush(rt::T);
 EQEQ_FAIL:
    break;
}
// ... proc x]
// ... proc x true]
case vasm::LT_1: {
    ppush(rt::T);
    break;
}
// ... proc x y]
// ... proc x y bool]
case vasm::LT_2: {
    ppush(cpINumber(ppeek(1))->lt(cpINumber(ppeek())) ? rt::T : rt::F);
    break;
}
// ... proc x y list-or-nil]
// ... proc x y list-or-nil bool]
case vasm::LT_2N: {
    INumber* x = cpINumber(ppeek(2)), *y = cpINumber(ppeek(1));
    if (!x->lt(y)) {
        ppush(rt::F);
        goto LT_FAIL;
    }
    x = y;
    for (ISeq* s=rt::seq(ppeek()); s; s=s->next()) {
        y = cpINumber(s->first());
        if (!x->lt(y)) {
            ppush(rt::F);
            goto LT_FAIL;
        }
        x = y;
    }
    ppush(rt::T);
 LT_FAIL:
    break;
}

