/*
  proc_io.cpp
  S. Edward Dolan
  Wednesday, June 21 2023
*/

// ... proc string]
// ... proc string nil]
case vasm::LOAD_1: {
    rt::loadFile(cpString(ppeek())->val());
    ppush(NIL);
    break;
}
// ... proc file-name list-or-nil]
// ... proc file-name list-or-nil fstream]
case vasm::FSTREAM_1N: {
    ISeq* s = pISeq(ppeek());
    if (s == NIL)
        ppush(FStream::create(cpString(ppeek(1))->val(), std::ios_base::in));
    else {
        openMode_t mode = openMode_t();
        for (; s; s=s->next())
            mode |= kwToOpenMode(cpKeyword(s->first()));
        ppush(FStream::create(cpString(ppeek(1))->val(), mode));
    }
    break;
}
// ... proc]
// ... proc sstream]
case vasm::SSTREAM_0: {
    ppush(SStream::create(""));
    break;
}
// ... proc init-str list-or-nil]
// ... proc init-str list-or-nil sstream]
case vasm::SSTREAM_1N: {
    ISeq* modes = pISeq(ppeek());
    if (modes == NIL)
        ppush(SStream::create(cpString(ppeek(1))->val()));
    else {
        openMode_t mode = openMode_t();
        for (; modes; modes=modes->next())
            mode |= kwToOpenMode(cpKeyword(modes->first()));
        ppush(SStream::create(cpString(ppeek(1))->val(), mode));
    }
    break;
}
// ... proc]
// ... proc result]
case vasm::READ_0: {
    IInStream* s = cpIInStream(rt::currentIN());
    ppush(reader::readOne(s, true, NIL));
    break;
}
// ... proc stream]
// ... proc stream result]
case vasm::READ_1: {
    ppush(reader::readOne(cpIInStream(ppeek()), true, NIL));
    break;
}
// ... proc stream bool obj]
// ... proc stream bool obj result]
case vasm::READ_3: {
    ppush(reader::readOne(cpIInStream(ppeek(2)), rt::toBool(ppeek(1)),
                          ppeek()));
    break;
}
// ... proc stream]
// ... proc stream char]
case vasm::READ_CHAR_1: {
    IInStream* s = cpIInStream(ppeek());
    int c = s->get();
    if (s->eof())
        ppush(NIL);;
    ppush(Character::fetch(c));
    break;
}
// ... proc list-or-nil]
// ... proc list_or-nil nil]
case vasm::PR_0N: {
    for (ISeq* s=rt::seq(ppeek()); s; s=s->next()) {
        cpIOutStream(rt::currentOUT())->print(s->first());
        if (s->next())
            cpIOutStream(rt::currentOUT())->put(' ');
    }
    ppush(NIL);
    break;
}
// ... proc]
// ... proc nil]
case vasm::NEWLINE_0: {
    IOutStream* s = cpIOutStream(rt::currentOUT());
    s->put('\n');
    if (rt::flushOnNewline())
        s->flush();
    ppush(NIL);
    break;
}
// ... proc stream
// ... proc stream nil
case vasm::FCLOSE_1: {
    cpFStream(ppeek())->close();
    ppush(NIL);
    break;
}
// ... proc file-name
// ... proc file-name text
case vasm::SLURP_1: {
    std::string fname = cpString(ppeek())->val();
    std::ifstream s(fname);
    if (!s.is_open())
        throw SxIOError("failed to open: " + fname);
    std::stringstream ss;
    ss << s.rdbuf();
    s.close();
    ppush(String::create(ss.str()));
    break;
}
// ... proc]
// ... proc string]
case vasm::READ_LINE_0: {
    IInStream* s = cpIInStream(rt::currentIN());
    if (s->eof())
        ppush(NIL);
    else {
        std::string buf;
        std::getline(s->istream(), buf);
        ppush(String::fetch(buf));
    }
    break;
}
